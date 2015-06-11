/******************************2014-2015, NJTECH, Edu.************************** 
FileName: rfspi.h
Author:  孙冬梅       Version :  1.0        Date: 2015.05.30
Description:    射频模块SPI驱动接口头文件     
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    15/05/30     1.0     文件创建  
1.SPI端口初始化，SPI1: SDO SDI SCK SPI3:SDO SDI SCK
2.设置速度等SPI参数
3.初始化SPI端口，启动RF传输
*******************************************************************************/
#ifndef __SPI_H
#define __SPI_H
#include "stm32f10x.h"

#define GPIO_SPI3     GPIOC
#define RCC_SPI3      RCC_APB2Periph_GPIOC
#define SPI3_SCK      GPIO_Pin_10
#define SPI3_MISO      GPIO_Pin_11
#define SPI3_MOSI      GPIO_Pin_12

#define GPIO_SPI1     GPIOA
#define RCC_SPI1      RCC_APB2Periph_GPIOA
#define SPI1_SCK      GPIO_Pin_5
#define SPI1_MISO      GPIO_Pin_6
#define SPI1_MOSI      GPIO_Pin_7

#define SPI_SPEED_2         0 
#define SPI_SPEED_8         1 
#define SPI_SPEED_16        2 
#define SPI_SPEED_256       3 
																					  
void SPI3_Init(void);			 //初始化SPI口 
u8 SPI3_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
void SPI3_SetSpeed(u8 SpeedSet); //设置SPI速度

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SpeedSet); //设置SPI速度
u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节		 
#endif

