/*********************2012-2013, NJUT, Edu.********************* 
FileName: led.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    led驱动      
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/03/30     1.0     文件创建   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |      Leds                   |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |      LED1                   |        D2                   |
  *          |      LED2                   |        D3                   |
  *          |      LED3                   |        D4                   |
  *          |      LED4                   |        D7                   |
  *          +-----------------------------+-----------------------------+
***************************************************************/ 
#include "led.h"

/*******************************************************************************
* Function Name  : LED_Init
* Description    : LEDGPIO初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
        
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); 	
        
	GPIO_InitStructure.GPIO_Pin   = LED_PIN[0] | LED_PIN[1] | LED_PIN[2] | LED_PIN[3];
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);        
}

/*******************************************************************************
* Function Name  : LEDOn
* Description    : LED亮
* Input          : -led:LED值
* Output         : None
* Return         : None
*******************************************************************************/
void LEDOn(Led_Def Led)
{
  	LED_PORT[Led]->BSRR=LED_PIN[Led];
}

/*******************************************************************************
* Function Name  : LEDOff
* Description    : LED灭
* Input          : -led:LED值
* Output         : None
* Return         : None
*******************************************************************************/
void LEDOff(Led_Def Led)
{
  	LED_PORT[Led]->BRR=LED_PIN[Led];  
}

/*******************************************************************************
* Function Name  : LEDTog
* Description    : LED状态翻转
* Input          : -led:LED值
* Output         : None
* Return         : None
*******************************************************************************/
void LEDTog(Led_Def Led)
{
  	LED_PORT[Led]->ODR^=LED_PIN[Led];
}
