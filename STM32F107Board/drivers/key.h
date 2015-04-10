#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

typedef enum 
{  
  KEY1 = 0,
  KEY2 = 1,
  KEYNULL = -1,
} Key_Def;

static GPIO_TypeDef* KEY_PORT[2]={GPIOA, GPIOC};
static const u16 KEY_PIN[2]={GPIO_Pin_0, GPIO_Pin_13};
static const u16 KEY_SRC[2]={ EXTI_Line0, EXTI_Line13};
static const u8  KEY_IRQ[2]={EXTI0_IRQn, EXTI15_10_IRQn};

void Key_Init(void);
void Set_Keyint(void);
Key_Def KEY_Scan(void);

#endif

