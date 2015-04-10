//-------------------------------------------------
//功能: 107板Com2驱动程序映射到PD5 PD6
//版本号：v0.2.0
//编写： 孙冬梅
//日期： 20121213
//-------------------------------------------------

	   							  
#ifndef __USART_H
#define __USART_H

#include "rtthread.h"
#include "stm32f10x.h"
#include "string.h"

void Usart2Config(uint32_t baudrate,uint8_t channelparity);
void Usart2PutChar(uint8_t ch);
uint8_t Usart2GetChar(void);
void Usart2PutString(uint8_t *str);


/*DMA操作*/
void USART2_DMA_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len);
void DMA_USART2_Configuration(uint32_t USART_DR_Address,uint8_t* USART_ConvertedValue,uint32_t Len);
void Usart2DMAPutString(uint8_t *str,int len);
void clear_buf_uart2(void);
uint8_t checkusart2(uint8_t* rxbuffer, uint16_t* len );
void usart2dmainit(void  );
#endif
