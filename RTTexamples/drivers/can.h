/******************************2012-2013, NJUT, Edu.*************************** 
FileName: can.h 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    can驱动 头文件     
Version:         1.0 
History:         
*******************************************************************************/ 
#ifndef __CAN_H
#define __CAN_H

#include "stm32f10x.h"
#include <rtthread.h>
void canconfig(void);
void can1send(uint8_t* data,uint8_t number);
void can2send(uint8_t* data,uint8_t number);

#endif
