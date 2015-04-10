
#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f10x.h"

#define I2C_SCL_PIN   GPIO_Pin_6 
#define I2C_SDA_PIN   GPIO_Pin_7 

#define I2C_PORT      GPIOB 

#define I2C_CLK      RCC_APB2Periph_GPIOB 

void rt_I2C_init(void);
unsigned char rt_I2C_read(unsigned char address);
void rt_I2C_write(unsigned char address,unsigned char info);

#endif

