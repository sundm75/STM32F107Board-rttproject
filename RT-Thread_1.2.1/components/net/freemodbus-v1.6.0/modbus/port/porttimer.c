/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "stm32f10x.h"

/* ----------------------- Start implementation -----------------------------*/
/*******************************************************************************
* Function Name  : xMBPortTimersInit
* Description    : 按配置50us 時鐘
* Input          : usTim1Timerout50us 定时时间
* Output         : None
* Return         : None
*******************************************************************************/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us ) 
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uint16_t PrescalerValue = 0;
	
	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1; // 1/20000=50us 
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) usTim1Timerout50us - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //两级占先 八级副优先
	/* Enable the TIM4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
	
	/* TIM IT DISABLE */
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	/* TIM4 DISABLE counter */
	TIM_Cmd(TIM4,  DISABLE);
	return TRUE;;
}

/*******************************************************************************
* Function Name  : vMBPortTimersEnable
* Description    : 打开时钟
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void vMBPortTimersEnable( void ) 
{
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4,0x0000); 
	TIM_Cmd(TIM4, ENABLE);
}

/*******************************************************************************
* Function Name  : vMBPortTimersDisable
* Description    : 关闭时钟
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void vMBPortTimersDisable( void ) 
{
	TIM_Cmd(TIM4, DISABLE);
	TIM_SetCounter(TIM4,0x0000); 
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
void prvvTIMERExpiredISR( void ) 
{
    ( void )pxMBPortCBTimerExpired(  );
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : TIM4中断函数 清中断，调用prvvTIMERExpiredISR
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    /* Clear TIM4 Capture Compare1 interrupt pending bit*/
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    prvvTIMERExpiredISR( );
  }
}

