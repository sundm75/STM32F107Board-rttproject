/******************************2012-2013, NJTECH, Edu.************************** 
FileName: adc.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    adc驱动      
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/03/30     1.0     文件创建   
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

#define ADC1_DR_Address    ((uint32_t)0x4001244C)  //DR_Address寄存器基地址

static u16  ADCConvertedValue;

/*******************************************************************************
* Function Name  : ADC_Config
* Description    : ADC 采用DMA方式　初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_Config(void)
{
  ADC_InitTypeDef ADC_InitStructure;      
  DMA_InitTypeDef DMA_InitStructure;      
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Configure PC.00 (ADC Channel10) as analog input*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;    //DMA对应的外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;   //内存存储基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//DMA的转换模式为SRC模式，由外设搬移到内存
  DMA_InitStructure.DMA_BufferSize = 1;		   //DMA缓存大小，1个,单位为DMA_MemoryDataSize
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//接收一次数据后，设备地址禁止后移
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	//关闭接收一次数据后，目标内存地址后移
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA搬数据尺寸，HalfWord就是为16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //转换模式，循环缓存模式。
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;	//DMA优先级高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  //M2M模式禁用
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);   

  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  

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
  *advalue = ADCConvertedValue;
  
}


