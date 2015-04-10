/******************************2012-2013, NJUT, Edu.*************************** 
FileName: adc_temprature.h 
Author:  孙冬梅       Version :  1.0        Date: 2013.03.30
Description:    使用片内的温度传感器测量温度,并通过串口输出温度值    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/03/30     1.0     文件创建   
*******************************************************************************/ 
#ifndef __ADC_TEMPERATURE
#define __ADC_TEMPERATURE

  #include "stm32f10x.h"
  #include "stm32f10x_adc.h"
  #include "stm32f10x_dma.h"
  #include <rtthread.h>
  
  void TemperatureConfig(void);
  int GetTemperature(void) ;

#endif 
