#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"

typedef enum 
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
  LED4 = 3,
  LEDALL = 4,
} Led_Def;

static GPIO_TypeDef* LED_PORT[5]={GPIOD, GPIOD, GPIOD, GPIOD, GPIOD};
static const u16 LED_PIN[5]={GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_7,
                          GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7};

void LED_Init(void);
void LEDOn(Led_Def Led);
void LEDOff(Led_Def Led);
void LEDTog(Led_Def Led);

#endif 

