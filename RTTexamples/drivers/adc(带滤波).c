/******************************2012-2013, NJUT, Edu.*************************** 
FileName: adc.c 
Author:  孙冬梅       Version :  1.0        Date: 2015.10.30
Description:    adc驱动,带数字滤波      
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/03/30     1.0     文件创建   
      Sundm    15/10/30     1.0     添加数字滤波部分Filter   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |      Keys                   |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |       AIN                   |         C0                  |
  *          +-----------------------------+-----------------------------+
*******************************************************************************/ 

#include "adc.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include <stdio.h>
#include <rtthread.h>

#define ADC1_DR_Address    ((uint32_t)0x4001244C)  //DR_Address寄存器基地址
#define FilterLen 128

static uint16_t  ADCConvertedValue[FilterLen];

/*******************************************************************************
* Function Name  : DigitFilter 内部函数
* Description    : 软件滤波 取NO的2/5作为头尾忽略值,注意N要大于5,否则不会去头尾 
* Input          : buf 数据首地址 len 数据长度
* Output         : None
* Return         : 滤波后数据
*******************************************************************************/
uint16_t DigitFilter(uint16_t * buf,uint8_t len) 
{ 
  uint8_t i,j; 
  uint16_t tmp; 
  uint8_t cut_no=0; 
  /*排序，将buf[0]到buf[no-1]从大到小排列 */
  for(i=0;i<len;i++) 
  { 
    for(j=0;j<len-i-1;j++) 
    {  
      if(buf[j]>buf[j+1]) 
      {  
        tmp=buf[j]; 
        buf[j]=buf[j+1]; 
        buf[j+1]=tmp; 
      } 
    } 
  } 
  /*len为整形，此处是将len的前2/5丢掉 */
  if(len>5)
  { 
    cut_no=len/5; 
  } 
  /*求平均*/ 
  tmp=0; 
  for(i=cut_no;i<len-cut_no;i++) //求平均 
  tmp+=buf[i]; 
  return(tmp/(len-2*cut_no)); 
} 
 
/*******************************************************************************
* Function Name  : ADC_Config
* Description    : ADC 采用DMA方式　初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;      
  ADC_InitTypeDef ADC_InitStructure;      
 
  /*NVIC Config*/
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

   /* Enable DMA channel6 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Configure PC.00 (ADC Channel10) as analog input*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  /* Enable ADC1  */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
  
  /* ADC1 configuration */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //独立的转换模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;		  //开启扫描模式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   //开启连续转换模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC外部开关，关闭状态
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //对齐方式,ADC为12位中，右对齐方式
  ADC_InitStructure.ADC_NbrOfChannel = 1;	 //开启通道数，1个
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel 10configuration ADC通道组， 第10个通道 采样顺序1，转换时间 */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);	  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  
  
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);	  
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));  
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);		
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));	   
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);	

  
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;    //DMA对应的外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;   //内存存储基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//DMA的转换模式为SRC模式，由外设搬移到内存
  DMA_InitStructure.DMA_BufferSize = FilterLen;		   //DMA缓存大小，1个,单位为DMA_MemoryDataSize
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//接收一次数据后，设备地址禁止后移
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	//关闭接收一次数据后，目标内存地址增加
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA搬数据尺寸，HalfWord就是为16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //转换模式，单次。
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;	//DMA优先级高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  //M2M模式禁用
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);   
  
  /*DMA发送完成中断开*/
  //DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); 
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : DMA1 通道1 中断服务程序 DMA 数据完全完成后产生中断
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void) 
{ 
  if(DMA_GetITStatus(DMA1_IT_TC1))//通道1传输完成中断 
  { 
    DMA_ClearITPendingBit(DMA1_IT_GL1); //清除全部中断标志 
  } 
} 
/*******************************************************************************
* Function Name  : getadcvalue
* Description    : 获取当前ADC值
* Input          : None
* Output         : advalue
* Return         : None
*******************************************************************************/
void getadcvalue(uint16_t *advalue)
{
   uint16_t ADCConvertedValueLocal=0, Precent = 0;
   uint32_t Voltage = 0;
	
  *advalue = DigitFilter(ADCConvertedValue,FilterLen);
  
  /*以下显示测量值*/    
   
  ADCConvertedValueLocal = DigitFilter(ADCConvertedValue,FilterLen);
  Precent = (ADCConvertedValueLocal*100/0x1000);	
  Voltage = ADCConvertedValueLocal * 3300/0x1000;		
  
  rt_kprintf("\r\n 当前AD转换结果为：0x%X, 百分比为：%d% ，电压值：%d.%dV \n\r", 
        ADCConvertedValueLocal, Precent, Voltage/1000, Voltage%1000);
}
#include  <finsh.h> 
FINSH_FUNCTION_EXPORT(getadcvalue, get adc convert e.g. getadcvalue());
