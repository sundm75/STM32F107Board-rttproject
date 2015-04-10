/***************************2012-2013, NJUT, Edu.******************************* 
FileName: usart2.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.07.10
Description:    usart2驱动 (连接485总线) 采用DMA方式 进行发送和接收 开发送和接收中断   
注意：由于 DMA通道冲突，remove i2c_ee_dma.c文件 
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/07/10     1.0     文件创建   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |      FunctionPin            |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |      USART2_TX              |        PD5                  |
  *          |      USART2_RX              |        PD6                  |
  *          +-----------------------------+-----------------------------+
*******************************************************************************/ 
#include "stm32f10x.h"
#include "finsh.h"
#include "rs485.h"
#include <stdint.h>
#include "rtthread.h"
#include "string.h"

#define  UART_RX_LEN 512
#define  UART_TX_LEN 512

#define USART2_PORT				GPIOD

#define USART2_PORT_APB2Periph	RCC_APB2Periph_GPIOD
#define USART2_CLK_APB1Periph		RCC_APB1Periph_USART2

#define USART2_TX_PIN				GPIO_Pin_5
#define USART2_RX_PIN				GPIO_Pin_6	

uint8_t USART2_TX_DATA[UART_TX_LEN] = {0x01}; 
uint8_t USART2_RX_DATA[UART_RX_LEN] = {0x00}; 
uint8_t USART2_TX_Finish=1; // USART2发送完成标志量


/*******************************************************************************
* Function Name  : Uart2Config
* Description    : USART2初始化,115200,无校验，8数据，1停止，中断接收
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Usart2Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(USART2_PORT_APB2Periph | RCC_APB2Periph_AFIO, ENABLE);
  
  /* Enable the USART2 Pins Software Remapping */
  GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
  
  RCC_APB1PeriphClockCmd(USART2_CLK_APB1Periph, ENABLE);    
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = USART2_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(USART2_PORT, &GPIO_InitStructure);
  
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = USART2_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART2_PORT, &GPIO_InitStructure); 
  
  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
  
  /* USART configuration */
  USART_InitStructure.USART_BaudRate   = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits   = USART_StopBits_1;
  USART_InitStructure.USART_Parity     = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  
  /*接收中断使能 RXNE接收中断 TC传输完成中断 TXE发送中断 PE奇偶错误中断 IDLE空闲中断*/
  USART_ITConfig(USART2, USART_IT_IDLE , ENABLE);
  
  /* Enable USART */
  USART_Cmd(USART2, ENABLE);	
  
  /*保证发送的第一个字节不会丢失*/
  USART_ClearFlag(USART2, USART_FLAG_TC);
}

/*******************************************************************************
* Function Name  : Usart2PutChar
* Description    : USART2发送一字节
* Input          : ch 要发送的字节
* Output         : None
* Return         : None
*******************************************************************************/
void Usart2PutChar(uint8_t ch)
{
	USART_SendData(USART2, ch);  
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); 	
}

/*******************************************************************************
* Function Name  : Usart2PutString
* Description    : USART2发送字符串
* Input          : str 要发送的字符串
* Output         : None
* Return         : None
*******************************************************************************/
void Usart2PutString(uint8_t *str)
{
	while(*str)
	{
		USART_SendData(USART2, *str++); 
		/*Loop until the end of transmission*/
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); 	
	}
}

/*******************************************************************************
* Function Name  : Usart2GetChar 
* Description    : 串口2接收一个字符
* Input          : None
* Output         : None
* Return         : 接收到的字符
*******************************************************************************/
uint8_t Usart2GetChar(void)
{ 
    uint8_t ch;	
    while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);    
    ch = USART_ReceiveData(USART2);	   									 
    return ch;   
}

/*******************************************************************************
* Function Name  : USART2_DMA_Configuration
* Description    : 配置DMA通道 USART2->DMA 串口通过DMA的方式接收数据
* Input          : USART_ConvertedValue 存放串口发送数据的RAM地址
*                  USART_DR_Address 串口1数据寄存器地址
*                : Len  DMA通道一的DMA缓存大小
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_DMA_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len)
{
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA1时钟
  DMA_DeInit(DMA1_Channel6);//将DMA1的通道16寄存器设置为缺省值
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_Address;//该参数用以定义外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART_ConvertedValue;//该参数用以定义内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//设置外设作为数据传输的来源
  DMA_InitStructure.DMA_BufferSize = Len;//定义DMA通道一的DMA缓存大小，单位为单个数据大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//定义外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//定义内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//设定外设数据宽度为8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;//设定内存数据宽度为8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA传输模式为Normal，如果为Circular,将会循环传输
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//指定通DMA通道一拥有高优先级
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//将DMA通道一不设置成内存到内存模式
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);//根据DMA_InitStructure中的参数初始化DMA1的通道6寄存器
      
  /*DMA发送完成中断开 传输错误中断开*/
  DMA_ITConfig(DMA1_Channel6, DMA_IT_TC , ENABLE);
  DMA_ITConfig(DMA1_Channel6, DMA_IT_TE , ENABLE);
  
  /* 使能USART1的DMA接收请求*/
  USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);
  
  /* Enable DMA1 channel6 */
  DMA_Cmd(DMA1_Channel6, ENABLE);
  
  /* Configure the NVIC Preemption Priority Bits */  
  
  /*DMA中断配置*/
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
  
}

/*******************************************************************************
* Function Name  : USART2_DMA_Configuration
* Description    : 配置DMA通道 DMA->USART2 串口通过DMA的方式发送数据
* Input          : USART_ConvertedValue 存放串口发送数据的RAM地址
*                  USART_DR_Address 串口1数据寄存器地址 
*                : Len  DMA通道一的DMA缓存大小
* Output         : None
* Return         : None
*******************************************************************************/
void DMA_USART2_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len)
{
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA1时钟
  
  DMA_DeInit(DMA1_Channel7);//将DMA1的通道17寄存器设置为缺省值
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_DR_Address;//该参数用以定义外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART_ConvertedValue;//该参数用以定义内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//设置外设作为数据传输的目的
  DMA_InitStructure.DMA_BufferSize = Len;//定义DMA通道一的DMA缓存大小，单位为单个数据大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//定义外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//定义内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//设定外设数据宽度为8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;//设定内存数据宽度为8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;////DMA传输模式为Normal，如果为Circular,
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//指定通DMA通道一拥有高优先级
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//将DMA通道一不设置成内存到内存模式
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);//根据DMA_InitStructure中的参数初始化DMA1的通道7寄存器
  
  /*DMA发送完成中断开 传输错误中断开*/
  DMA_ITConfig(DMA1_Channel7, DMA_IT_TC , ENABLE);
  DMA_ITConfig(DMA1_Channel7, DMA_IT_TE , ENABLE);
  
  /* Enable the USART Tx DMA request */                
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  
  /* Enable DMA1 channel7 */
  DMA_Cmd(DMA1_Channel7, DISABLE);

  /* Configure the NVIC Preemption Priority Bits */  
  
  /*DMA中断配置*/
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : DMA 中断服务程序 DMA 数据完全完成后产生中断 接收
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{
  DMA_ClearITPendingBit(DMA1_IT_TE6);  
  if (DMA_GetITStatus(DMA1_IT_TC6) != RESET) // 判断为接收中断
    {
      DMA_ClearITPendingBit(DMA1_IT_TC6);
      DMA_Cmd(DMA1_Channel6, DISABLE);//关闭DMA,防止处理其间有数据
      DMA_SetCurrDataCounter(DMA1_Channel6, (uint16_t)UART_RX_LEN); //重装填
      DMA_Cmd(DMA1_Channel6, ENABLE);//处理完,重开DMA
    }
}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : DMA 中断服务程序 DMA 数据完全完成后产生中断 发送
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
  int delaytime;
	DMA_ClearITPendingBit(DMA1_IT_TE7);  
  if (DMA_GetITStatus(DMA1_IT_TC7) != RESET) // 判断为 发送中断
    {
      DMA_ClearITPendingBit(DMA1_IT_TC7);
      DMA_Cmd(DMA1_Channel7, DISABLE);//关闭DMA
      USART2_TX_Finish=1;//置DMA传输完成
      
      for(delaytime = 0;delaytime<5000;delaytime++);
      RS485Read();//置为读状态 串口是慢速设备，变为读之前需要延时，等数据发完
    }
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
  uint32_t DATA_LEN = 0;
  uint16_t i = 0;
  /*读SR后读DR清除Idle 先读SR，然后读DR才能清除 看USART_ClearFlag注释*/
  /*一帧接收结束 产生IDLE空闲中断 以下语句读状态可清除IDLE中断 */
  if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
  {
    i = USART2->SR;
    i = USART2->DR;
    DMA_Cmd(DMA1_Channel6, DISABLE);//关闭DMA,防止处理其间有数据
    
    DATA_LEN = UART_RX_LEN - DMA_GetCurrDataCounter(DMA1_Channel6); 
    
    if(DATA_LEN > 0)
    {                        
      while(USART2_TX_Finish==0);//等待上一次数据发送完成才下一次
      /*将串口2DMA收到的数据 送到 串口2的发送DMA存储地址*/
      for(i=0;i<DATA_LEN;i++)
      {
        USART2_TX_DATA[i]=USART2_RX_DATA[i];
      }
      //USART用DMA传输替代查询方式发送，克服被高优先级中断而产生丢帧现象。
      DMA_Cmd(DMA1_Channel7, DISABLE); //改变datasize前先要禁止通道工作
      DMA1_Channel7->CNDTR=DATA_LEN; //重装填传输数据量
      USART2_TX_Finish=0;//DMA传输开始标志量
      RS485Write();//置为写状态
      DMA_Cmd(DMA1_Channel7, ENABLE);                        
    }
    DMA_ClearFlag(DMA1_FLAG_GL6 | DMA1_FLAG_TC6 | DMA1_FLAG_TE6 | DMA1_FLAG_HT6);//清标志
    DMA_SetCurrDataCounter(DMA1_Channel6, UART_RX_LEN); //重装填
    DMA_Cmd(DMA1_Channel6, ENABLE);//处理完,重开DMA
  } 
}

/*******************************************************************************
* Function Name  : testdma
* Description    : 实现串口收到后，再从自己的串口发出
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void test_usart2(void )
{
  RS485GPIOConfig();
  Usart2Config();
  
  RS485Read() ;
  DMA_USART2_Configuration((uint32_t)(&USART2->DR),USART2_TX_DATA,UART_TX_LEN);
  USART2_DMA_Configuration((uint32_t)(&USART2->DR) ,USART2_RX_DATA,UART_RX_LEN);
}


FINSH_FUNCTION_EXPORT(test_usart2, startup usart2 receive send e.g. test_usart2());
