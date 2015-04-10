/************************************************* 
  Copyright (C), 2012, NJUT
  File name:      gprsio.c
  Author:  sundm     Version:  1.0      Date: 2013.1.3 
  Description:    sundm GPRS模块驱动接口函数 
  Others:     所有的函数均需要修改     
  Function List:  １）GPRSPortConfig(void);//端口初始化
                  ２）ModuleStart(void);//GPRS模块开机
                  ３）ModuleReset(void);//GPRS模块复位
                  ４）ComPutString(uint8_t *str);//串口输出字符串
*************************************************/ 
#include "GPRSIO.h"

extern void Delay_10ms(uint16_t ms);
/*******************************************************************************
* Function Name  : GPRSPortConfig
* Description    : TermOn  and RST Port  Config
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPRSPortConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); 	
  
  GPIO_InitStructure.GPIO_Pin   = GPRS_TREMON_PIN | GPRS_RST_PIN| GPRS_LDO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPRS_PORT, &GPIO_InitStructure); 
}

/*******************************************************************************
* Function Name  : TermonOn
* Description    : TermOn Port On 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TermonOn(void)
{
  GPIO_SetBits(GPRS_PORT,GPRS_TREMON_PIN);
}

/*******************************************************************************
* Function Name  : TermonOff
* Description    : TermOn Port Off 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TermonOff(void)
{
  GPIO_ResetBits(GPRS_PORT,GPRS_TREMON_PIN);
}

/*******************************************************************************
* Function Name  : ModuleReset
* Description    : 模块复位 RESET 置低50ms 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ModuleReset(void)
{
  GPIO_SetBits(GPRS_PORT,GPRS_RST_PIN);
  Delay_10ms(50);
  GPIO_ResetBits(GPRS_PORT,GPRS_RST_PIN);
}

/*******************************************************************************
* Function Name  : ModuleStart
* Description    : GPRS模块开机 复位
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ModuleStart(void)
{
  GPIO_SetBits(GPRS_PORT,GPRS_LDO_PIN);
  Delay_10ms(100);
  GPIO_SetBits(GPRS_PORT,GPRS_TREMON_PIN);
  Delay_10ms(150);
  GPIO_ResetBits(GPRS_PORT,GPRS_TREMON_PIN);
  Delay_10ms(100);
}

/*******************************************************************************
* Function Name  : ModuleOff
* Description    : GPRS模块开机 复位
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ModuleOff(void)
{
  GPIO_SetBits(GPRS_PORT,GPRS_TREMON_PIN);
  Delay_10ms(100);
  GPIO_ResetBits(GPRS_PORT,GPRS_TREMON_PIN);
}

/*******************************************************************************
* Function Name  : ModulePwrOff
* Description    : GPRS模块断电
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ModulePwrOff(void)
{
  GPIO_ResetBits(GPRS_PORT,GPRS_LDO_PIN);
  Delay_10ms(60);
}



