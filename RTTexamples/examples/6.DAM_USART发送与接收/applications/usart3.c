/***************************2012-2013, NJUT, Edu.******************************* 
FileName: usart3.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.07.10
Description:    usart3驱动  采用DMA方式 进行发送和接收 开发送和接收中断   
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
  *          |      USART3_TX              |        PC10                 |
  *          |      USART3_RX              |        PC11                 |
  *          +-----------------------------+-----------------------------+
*******************************************************************************/ 
#include "stm32f10x.h"
#include "finsh.h"
#include <stdint.h>
#include "stm32f10x.h"
#include "rtthread.h"
#include "string.h"

#define  UART_RX_LEN 512
#define  UART_TX_LEN 512

#define USART3_PORT				GPIOC

#define USART3_PORT_APB2Periph	RCC_APB2Periph_GPIOC
#define USART3_CLK_APB1Periph		RCC_APB1Periph_USART3

#define USART3_TX_PIN				GPIO_Pin_10
#define USART3_RX_PIN				GPIO_Pin_11	

uint8_t USART3_TX_DATA[UART_TX_LEN] = {0x00}; 
uint8_t USART3_RX_DATA[UART_RX_LEN] = {0x00}; 

uint8_t USART3_TX_Finish=1; // USART3发送完成标志位 发送完成为1
uint8_t USART3_RX_Finish=0; // USART3接收一帧完成标志位 接收完成为1
uint16_t rxlen = 0;

/*******************************************************************************
* Function Name  : Uart3Config
* Description    : USART2初始化,115200,无校验，8数据，1停止，中断接收
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Usart3Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(USART3_PORT_APB2Periph | RCC_APB2Periph_AFIO, ENABLE);
  
  /* Enable the USART2 Pins Software Remapping */
  GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
  
  RCC_APB1PeriphClockCmd(USART3_CLK_APB1Periph, ENABLE);    
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = USART3_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(USART3_PORT, &GPIO_InitStructure);
  
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin   = USART3_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART3_PORT, &GPIO_InitStructure); 
  
  /* USART configuration */
  USART_InitStructure.USART_BaudRate   = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits   = USART_StopBits_1;
  USART_InitStructure.USART_Parity     = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);
  
  /*接收中断使能 RXNE接收中断 TC传输完成中断 TXE发送中断 PE奇偶错误中断 IDLE空闲中断*/
  USART_ITConfig(USART3, USART_IT_IDLE , ENABLE);
  
  /* Enable USART */
  USART_Cmd(USART3, ENABLE);	
  
  /*保证发送的第一个字节不会丢失*/
  USART_ClearFlag(USART3,USART_FLAG_TC); 

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 

  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
  
}

/*******************************************************************************
* Function Name  : Usart2PutChar
* Description    : USART2发送一字节
* Input          : ch 要发送的字节
* Output         : None
* Return         : None
*******************************************************************************/
void Usart3PutChar(uint8_t ch)
{
	USART_SendData(USART3, ch);  
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); 	
}

/*******************************************************************************
* Function Name  : Usart2PutString
* Description    : USART2发送字符串
* Input          : str 要发送的字符串
* Output         : None
* Return         : None
*******************************************************************************/
void Usart3PutString(uint8_t *str)
{
  while(*str)
  {
    USART_SendData(USART3, *str++); 
    /*Loop until the end of transmission*/
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); 	
  }
}

/*******************************************************************************
* Function Name  : Usart2DMAPutString
* Description    : USART2通过DMA方式发送字符串
* Input          : str 要发送的字符串
* Output         : None
* Return         : None
*******************************************************************************/
void DMA3PutString(uint8_t *str)
{
  uint16_t len;
  len = strlen((char const*)str);
  memset(USART3_TX_DATA,0x00,UART_TX_LEN);
  
  while(USART3_TX_Finish==0);//等待上一次数据发送完成才下一次
  memcpy(USART3_TX_DATA,str,len);
  DMA_Cmd(DMA1_Channel2, DISABLE); //改变datasize前先要禁止通道工作
  DMA1_Channel2->CNDTR=len; //重装填传输数据量
  USART3_TX_Finish = 0;//DMA传输开始标志量
  DMA_Cmd(DMA1_Channel2, ENABLE);                        
}

/*******************************************************************************
* Function Name  : Usart2GetChar 
* Description    : 串口2接收一个字符
* Input          : None
* Output         : None
* Return         : 接收到的字符
*******************************************************************************/
uint8_t Usart3GetChar(void)
{ 
    uint8_t ch;	
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);    
    ch = USART_ReceiveData(USART3);	   									 
    return ch;   
}

/*******************************************************************************
* Function Name  : USART2_DMA_Configuration
* Description    : 配置DMA通道 USART3->DMA 串口通过DMA的方式接收数据 DMA1通道3
* Input          : USART_ConvertedValue 存放串口发送数据的RAM地址
*                  USART_DR_Address 串口1数据寄存器地址
*                : Len  DMA通道一的DMA缓存大小
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_DMA_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len)
{
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA1时钟
  DMA_DeInit(DMA1_Channel3);//将DMA1的通道3寄存器设置为缺省值
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
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);//根据DMA_InitStructure中的参数初始化DMA1的通道6寄存器
      
  /*DMA发送完成中断开 传输错误中断开*/
  DMA_ITConfig(DMA1_Channel3, DMA_IT_TC , ENABLE);
  DMA_ITConfig(DMA1_Channel3, DMA_IT_TE , ENABLE);
  
  /* 使能USART3的DMA接收请求*/
  USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);
  
  /* Enable DMA1 channel3 */
  DMA_Cmd(DMA1_Channel3, ENABLE);
  
  /* Configure the NVIC Preemption Priority Bits */  
  
  /*DMA中断配置*/
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
  
}

/*******************************************************************************
* Function Name  : USART3_DMA_Configuration
* Description    : 配置DMA通道 DMA->USART3 串口通过DMA的方式发送数据 DMA1通道2
* Input          : USART_ConvertedValue 存放串口发送数据的RAM地址
*                  USART_DR_Address 串口3数据寄存器地址 
*                : Len  DMA通道一的DMA缓存大小
* Output         : None
* Return         : None
*******************************************************************************/
void DMA_USART3_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len)
{
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA1时钟
  
  DMA_DeInit(DMA1_Channel2);//将DMA1的通道12寄存器设置为缺省值
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
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);//根据DMA_InitStructure中的参数初始化DMA1的通道7寄存器
  
  /*DMA发送完成中断开 传输错误中断开*/
  DMA_ITConfig(DMA1_Channel2, DMA_IT_TC , ENABLE);
  DMA_ITConfig(DMA1_Channel2, DMA_IT_TE , ENABLE);
  
  /* Enable the USART Tx DMA request */                
  USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
  
  /* Enable DMA1 channel7 */
  DMA_Cmd(DMA1_Channel2, DISABLE);

  /* Configure the NVIC Preemption Priority Bits */  
  
  /*DMA中断配置*/
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : DMA 中断服务程序 DMA 数据完全完成后产生中断 接收
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{
  DMA_ClearITPendingBit(DMA1_IT_TE3);  
  if (DMA_GetITStatus(DMA1_IT_TC3) != RESET) // 判断为接收中断
    {
      DMA_ClearITPendingBit(DMA1_IT_TC3);
      DMA_Cmd(DMA1_Channel3, DISABLE);//关闭DMA,防止处理其间有数据
      DMA_SetCurrDataCounter(DMA1_Channel3, (uint16_t)UART_RX_LEN); //重装填
      DMA_Cmd(DMA1_Channel3, ENABLE);//处理完,重开DMA
    }
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : DMA 中断服务程序 DMA 数据完全完成后产生中断 发送
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
  DMA_ClearITPendingBit(DMA1_IT_TE2);  
  if (DMA_GetITStatus(DMA1_IT_TC2) != RESET) // 判断为 发送中断
    {
      DMA_ClearITPendingBit(DMA1_IT_TC2);
      DMA_Cmd(DMA1_Channel2, DISABLE);//关闭DMA
      USART3_TX_Finish=1;//置DMA传输完成
    }
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
  uint16_t temp = 0;
  rxlen = 0;
   /*接收中断-设置USART的DMA接收，同时关闭接收中断，使能空闲中断 设置静围默状态；*/
    if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {
      USART_ClearITPendingBit(USART3,USART_IT_RXNE);
      DMA_Cmd(DMA1_Channel3, ENABLE);
      USART_ReceiverWakeUpCmd(USART3,ENABLE);
      USART_ITConfig(USART3, USART_IT_RXNE , DISABLE);
      USART_ITConfig(USART3, USART_IT_IDLE , ENABLE);
   }

   /*溢出中断-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题*/
    if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
    {
      temp = USART3->SR;
      temp = USART3->CR1;
    }

  /*读SR后读DR清除Idle 先读SR，然后读DR才能清除 看USART_ClearFlag注释*/
  /*一帧接收结束 产生IDLE空闲中断 以下语句读状态可清除IDLE中断 */
  if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
  {
    temp = USART3->SR;
    temp = USART3->DR;
    DMA_Cmd(DMA1_Channel3, DISABLE);//关闭DMA,防止处理其间有数据
    rxlen = UART_RX_LEN - DMA_GetCurrDataCounter(DMA1_Channel3);  //获取接收数据长度
    USART3_RX_Finish = 1;//置位接收完成标志
    DMA_ClearFlag(DMA1_FLAG_GL3 | DMA1_FLAG_TC3 | DMA1_FLAG_TE3 | DMA1_FLAG_HT3);//清标志
    DMA_SetCurrDataCounter(DMA1_Channel3, UART_RX_LEN); //重装填
    DMA_Cmd(DMA1_Channel3, ENABLE);
  } 
}

/*******************************************************************************
* Function Name  : clear_buf_uart
* Description    : 检查串口接收字符串打印，并清除接收缓冲
* Input          : std - 输入字符串 index -“OK”的位置
* Output         : None
* Return         : TRUE or FALSE
*******************************************************************************/
void clear_buf_uart(void)
{
  int i;
	if(USART3_RX_Finish!=0)
      {
        rt_kprintf("\r\nUSART3 接收数据长度：%d 字节。内容：",rxlen);
        for(i=0;i<rxlen;i++)
        rt_kprintf("%c",USART3_RX_DATA[i]);
        USART3_RX_Finish = 0;
        rxlen = 0;
        DMA3PutString(USART3_RX_DATA);
        memset(USART3_RX_DATA,0x00,UART_RX_LEN);
      }
}

/*******************************************************************************
* Function Name  : dmarevsend DMA测试函数
* Description    : usart2接收数据后，usart1打印出来,用于测试
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void dmarevsend(void * paramater )
{
  Usart3Config();
  DMA_USART3_Configuration((uint32_t)(&USART3->DR),USART3_TX_DATA,UART_TX_LEN);
  USART3_DMA_Configuration((uint32_t)(&USART3->DR) ,USART3_RX_DATA,UART_RX_LEN);
  
  while(1)
  {
    clear_buf_uart();
    rt_thread_delay(100);
  }
}


int test_usart3()
{
    rt_thread_t init_thread;

    init_thread = rt_thread_create("dmarevsend",
                                   dmarevsend, RT_NULL,
                                   2048, 8, 20);

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}

FINSH_FUNCTION_EXPORT(test_usart3, startup 3 receive send  e.g. test_usart3());
