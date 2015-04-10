#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32f10x.h"
#include <rtthread.h>
										    
 

#define RT_TOUCH_NORMAL		        0
#define RT_TOUCH_CALIBRATION_DATA	1
#define RT_TOUCH_CALIBRATION 		2

//#define SAVE_CALIBRATION

     
uint16_t touch_read_x(void);
uint16_t touch_read_y(void);
void touch_config(void);
																 

rt_err_t rtgui_touch_hw_init(const char * spi_device_name);
		  
#endif

