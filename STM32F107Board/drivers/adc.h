/******************************2012-2013, NJUT, Edu.*************************** 
FileName: adc.h 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    adc驱动 头文件     
Version:         1.0 
History:         
*******************************************************************************/ 
#ifndef __adc_H
#define __adc_H
#include "stm32f10x.h"

void ADC_Config(void);
void getadcvalue(uint16_t *advalue);

#endif
