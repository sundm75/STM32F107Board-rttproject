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
* Function Name  : Usart2Config
* Description    : COM1初始化,8数据，1停止，中断接收,校验位和波特率可配置
* Input          : baudrate parity 0-无校验 1-奇校验 2-偶校验 3-无校验2个停止位
* Output         : None
* Return         : None
*******************************************************************************/
void Usart2Config(uint32_t baudrate,uint8_t parity)
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
  USART_InitStructure.USART_BaudRate   = baudrate;
  USART_InitStructure.USART_StopBits   = USART_StopBits_1;
  //USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  // USART_InitStructure.USART_Parity     = USART_Parity_No;
  switch(parity)
          {  case 0:
              USART_InitStructure.USART_WordLength = USART_WordLength_8b;
              USART_InitStructure.USART_Parity = USART_Parity_No;
              break;
          case 1:
              USART_InitStructure.USART_WordLength = USART_WordLength_9b;
              USART_InitStructure.USART_Parity = USART_Parity_Odd;
            break;
          case 2:
              USART_InitStructure.USART_WordLength = USART_WordLength_9b;
              USART_InitStructure.USART_Parity = USART_Parity_Even;
            break;
          case 3:
              USART_InitStructure.USART_WordLength = USART_WordLength_8b;
              USART_InitStructure.USART_Parity = USART_Parity_No;
              USART_InitStructure.USART_StopBits = USART_StopBits_2;
            break;
          default:
            break;
          }
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  
  /*接收中断使能 RXNE接收中断 TC传输完成中断 TXE发送中断 PE奇偶错误中断 IDLE空闲中断*/
  USART_ITConfig(USART2, USART_IT_RXNE , ENABLE);
  
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

