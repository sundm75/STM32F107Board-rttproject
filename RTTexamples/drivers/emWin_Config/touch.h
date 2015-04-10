#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32f10x.h"
										    
 
     
uint16_t touch_read_x(void);
uint16_t touch_read_y(void);
void touch_config(void);
																 
		  
#endif


