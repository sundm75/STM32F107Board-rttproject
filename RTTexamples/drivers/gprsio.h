/************************************************* 
  Copyright (C), 2012, NJUT
  File name:      gprsio.c
  Author:  sundm     Version:  1.0      Date: 2013.1.3 
  Description:    sundm GPRS模块驱动接口函数 
  Others:        采用底层驱动，填充函数  
  Function List:  
*************************************************/ 

#ifndef _GPRSIO
#define _GPRSIO

#include "rtthread.h"
#include "stm32f10x.h"



#define GPRS_PORT GPIOD

#define GPRS_LDO_PIN GPIO_Pin_3
#define GPRS_RST_PIN GPIO_Pin_4
#define GPRS_TREMON_PIN GPIO_Pin_2	


void GPRSPortConfig(void); /*TermOn 端口及LED商品初始化*/
void ComConfig(void);/*串口配置*/
void ModuleReset(void);/*GPRS模块复位*/
void ModuleStart(void);/*GPRS模块开机*/
void ModuleOff(void);/*GPRS模块term on关机*/
void ModulePwrOff(void);/*GPRS模块断电*/
void ComPutString(char *str);/*串口输出字符串*/

#endif



