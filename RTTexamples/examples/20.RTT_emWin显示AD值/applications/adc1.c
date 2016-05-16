/******************************2012-2013, NJUT, Edu.*************************** 
FileName: adc.c 
Author:  孙冬梅       Version :  1.0        Date: 2016.02.10
Description:    1.adc驱动,带数字滤波  
                2.B0 B1 C0 使用ADC1 通道8、9、10 
Version:         2.0 
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
  *          |       A1IN                  |         B0                  |
  *          |       A2IN                  |         B1                  |
  *          |       AIN                   |         C0                  |
  *          +-----------------------------+-----------------------------+
*******************************************************************************/ 

#include "adc1.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include <stdio.h>
#include <rtthread.h>

#define ADC1_DR_Address    ((uint32_t)0x4001244C)  //DR_Address寄存器基地址
#define FilterLen 128
#define Sample_Num 128
#define Channel_Num 3
uint16_t ADCConvertedValue[Sample_Num][Channel_Num];
rt_device_t device2;

/*******************************************************************************
* Function Name  : DMA_Config
* Description    : ADC DMA 配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA_Config(void) 
{ 
  DMA_InitTypeDef DMA_InitStructure;     
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;    //DMA对应的外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;   //内存存储基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//DMA的转换模式为SRC模式，由外设搬移到内存
  DMA_InitStructure.DMA_BufferSize = Sample_Num*Channel_Num;		   //DMA缓存大小，单位为DMA_MemoryDataSize
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//接收一次数据后，设备地址禁止后移
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	//关闭接收一次数据后，目标内存地址增加
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA搬数据尺寸，HalfWord就是为16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //转换模式，循环。
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;	//DMA优先级高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  //M2M模式禁用
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);   
  
  /*DMA发送完成中断开*/
  //DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); 
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
} 

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : DMA NVIC配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Enable DMA channel6 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : DMAReConfig
* Description    : ADC DMA 重新配置  内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMAReConfig(void) 
{ 
  DMA_InitTypeDef DMA_InitStructure; 
  DMA_DeInit(DMA1_Channel1); 
  DMA_Init(DMA1_Channel1, &DMA_InitStructure); 
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); 
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
* Function Name  : DigitFilter 内部函数
* Description    : 软件滤波 取NO的2/5作为头尾忽略值,注意N要大于5,否则不会去头尾 
* Input          : buf 数据首地址 len 数据长度
* Output         : None
* Return         : 滤波后数据
*******************************************************************************/
uint16_t DigitFilter(uint16_t * buf,uint8_t len) 
{ 
  uint8_t i,j; 
  uint32_t tmp; 
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
  ADC_InitTypeDef ADC_InitStructure;      
 
  /*NVIC Config*/
  NVIC_Configuration();
  
  /* Configure PC.00 (ADC Channel10) as analog input*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PB0 (ADC Channel8) as analog input*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Configure PB1 (ADC Channel9) as analog input*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);


  /* Enable ADC1  */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
  
  /* ADC1 configuration */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //独立的转换模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;		  //开启扫描模式 多通道
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   //开启连续转换模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC外部开关，关闭状态,软件启动转换 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //对齐方式,ADC为12位中，右对齐方式
  ADC_InitStructure.ADC_NbrOfChannel = Channel_Num;	 //开启通道数，Channel_Num个
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel 8 configuration ADC通道组， 第8个通道 采样顺序1，转换时间 采样时间为55.5 周期  */ 
  /* PB0作为AI 用ADC_Channel_8*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);
  
  /* ADC1 regular channel 9 configuration ADC通道组， 第9个通道 采样顺序2，转换时间 采样时间为55.5 周期 */ 
  /* PB1作为AI 用ADC_Channel_9*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_55Cycles5);

  /*PC10 ADC1 regular channel 10configuration ADC通道组， 第10个通道 采样顺序3，转换时间 采样时间为55.5 周期 */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_55Cycles5);
  
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);	  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  
  
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);	  
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));  
  /* Start ADC1 calibaration */
  /*开始校准状态*/ 
  ADC_StartCalibration(ADC1);		
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));	   
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  
  /* DMA Config */
  DMA_Config();  
  
  /* 查找系统中的串口2设备 */
  device2 = rt_device_find("uart2");
  if (device2!= RT_NULL)
  {
    rt_device_open(device2, RT_DEVICE_OFLAG_RDWR);
  }
  
}

/*******************************************************************************
* Function Name  : getadcvalue
* Description    : 获取当前ADC值
* Input          : None
* Output         : advalue
* Return         : None
*******************************************************************************/
void getadcvalue(uint16_t* adcvalue)
{
  uint16_t Precent[3] = {0};
  uint32_t Voltage[3] = {0};
  uint16_t adcvaluetemp[Sample_Num] = {0};
  uint32_t i,j;
  uint8_t uartbuf[8] = {0};
  
  uartbuf[0]=0x12;
  uartbuf[1]=0x34;
  
  for(i=0;i<Channel_Num;i++ )
  {
    for(j=0;j<Sample_Num;j++)
    {
      adcvaluetemp[j] = ADCConvertedValue[j][i];
    }
    adcvalue[i] = DigitFilter(adcvaluetemp, Sample_Num);
    Precent[i] = (adcvalue[i]*100/0x1000);	
    Voltage[i] = adcvalue[i] * 3300/0x1000;		
    /*以下显示测量值 */      
    rt_kprintf("\r\n 当前AD[%d]转换结果为：0x%X, 百分比为：%d% ，电压值：%d.%dV \n\r", 
          i, adcvalue[i], Precent[i], Voltage[i]/1000, Voltage[i]%1000);
    uartbuf[i*2+2]=adcvalue[i]>>8;
    uartbuf[i*2+3]=adcvalue[i];
  }
    rt_device_write(device2, 0,uartbuf,8);

}

