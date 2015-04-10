/******************************2012-2013, NJUT, Edu.*************************** 
FileName: adc_temprature.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    使用片内的温度传感器测量温度,并通过串口输出温度值    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/03/30     1.0     文件创建   
*******************************************************************************/ 

#include "ADC_Temperature.h"

uint16_t ADCCov[16]; 
volatile uint8_t ADC_TX_Finish = 0; 
static DMA_InitTypeDef DMA_InitStructure; 

/*******************************************************************************
* Function Name  : ADCTEMP_Config
* Description    : ADC 内部温度传感器配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADCTEMP_Config(void) 
{ 
  ADC_InitTypeDef ADC_InitStructure; 

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE); 
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;      //独立模式 
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //单通道模式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;      //连续扫描 
  ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_None;//软件启动转换 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;     //数据右对齐 
  ADC_InitStructure.ADC_NbrOfChannel = 1;         //1个通道 
  ADC_Init(ADC1, &ADC_InitStructure); 
  
  /* 配置通道16的采样速度,这里因为是测温,不需要很快,配置为最慢*/  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5); 
  
  /* 使能内部温度传感器和内部的参考电压 */  
  ADC_TempSensorVrefintCmd(ENABLE);  
  
  /* 允许ADC1的DMA模式 */ 
  ADC_DMACmd(ADC1, ENABLE); 
  
  /* 允许ADC1*/ 
  ADC_Cmd(ADC1, ENABLE); 
  
  /*重置校准寄存器 */    
  ADC_ResetCalibration(ADC1); 
  while(ADC_GetResetCalibrationStatus(ADC1)); 
  
  /*开始校准状态*/ 
  ADC_StartCalibration(ADC1); 
  while(ADC_GetCalibrationStatus(ADC1)); 
  
  /* 人工打开ADC转换.*/  
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
} 
 
/*******************************************************************************
* Function Name  : DMA_Config
* Description    : ADC DMA 配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA_Config(void) 
{ 
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 

  DMA_DeInit(DMA1_Channel1); 
  
  DMA_InitStructure.DMA_PeripheralBaseAddr  =(u32)( &(ADC1->DR));  //ADC1数据寄存器 
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ADCCov;     //获取ADC的数组 
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;     //片内外设作源头 
  DMA_InitStructure.DMA_BufferSize = 16;        //每次DMA16个数据 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不增加 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    // 内存地址增加 
  DMA_InitStructure.DMA_PeripheralDataSize =  DMA_PeripheralDataSize_HalfWord; //半字 
  DMA_InitStructure.DMA_MemoryDataSize =  DMA_MemoryDataSize_HalfWord;   //半字 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;        //普通模式 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;       //高优先级 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;        //非内存到内存 
  DMA_Init(DMA1_Channel1, &DMA_InitStructure); 
  
  /*DMA发送完成中断开*/
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);       
  
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
    ADC_TX_Finish = 1; 
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
* Function Name  : getadctemp
* Description    : 使用片内的温度传感器测量温度,并通过串口输出温度值 
* Input          : None
* Output         : None
* Return         : 滤波后数据
*******************************************************************************/
int GetTemperature(void) 
{ 
 uint16_t adcvalue = 0; 
 DMAReConfig();//重新启动DMA 
   
  if( ADC_TX_Finish == 1) 
  { 
    ADC_TX_Finish = 0; 
    
    adcvalue = DigitFilter(ADCCov,16);  
    adcvalue = (1430 - adcvalue * 3300/4096)/4.3 + 25;//转换为温度值 
  }
  rt_kprintf("\r\n 当前CPU温度为：：%d度 \n\r",adcvalue);
  return adcvalue;
} 
 
/*******************************************************************************
* Function Name  : TemperatureConfig
* Description    : 片内的温度传感器参数配置
* Input          : None
* Output         : None
* Return         : 滤波后数据
*******************************************************************************/
void TemperatureConfig(void)
{
  NVIC_Configuration();
  ADCTEMP_Config();
  DMA_Config();
}

