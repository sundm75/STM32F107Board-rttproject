/******************************2014-2015, NJUT, Edu.*************************** 
FileName: NRF24L01.h 
Author:  孙冬梅       Version :  1.0        Date: 2015.05.30
Description:    NRF24L01驱动 头文件     
Version:         1.0 
History:         
*******************************************************************************/ 
#ifndef __NRF24L01_H
#define __NRF24L01_H	 		    
#include "stm32f10x.h"
#include "rfspi.h"
#include "rtthread.h"

void NRF24L01_Init(void);                                //NRF24l01初始化
void RX_Mode(void);                                      //配置为接收模式
void TX_Mode(void);                                      //配置为发送模式
rt_uint8_t NRF24L01_Write_Buf(rt_uint8_t regaddr, rt_uint8_t *pBuf, rt_uint8_t datalen); //写数据区
rt_uint8_t NRF24L01_Read_Buf(rt_uint8_t regaddr, rt_uint8_t *pBuf, rt_uint8_t datalen);  //读数据区		  
rt_uint8_t NRF24L01_Read_Reg(rt_uint8_t regaddr);		                 //读寄存器
rt_uint8_t NRF24L01_Write_Reg(rt_uint8_t regaddr, rt_uint8_t data);              //写寄存器
rt_err_t NRF24L01_Check(void);                                 //检查NRF24L01是否在位
rt_err_t NRF24L01_TxPacket(rt_uint8_t *txbuf);                         //发送一个包的数据
rt_err_t NRF24L01_RxPacket(rt_uint8_t *rxbuf);                         //接收一个包的数据

#endif











