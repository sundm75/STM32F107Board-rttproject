/***************************2012-2013, NJUT, Edu.****************************** 
* File Name          : calendar.h     Version :  2.0      Date: 2013.07.30
* Author             : 孙冬梅转自：www.armjishu.com Team
* Description        : 超强的日历，支持农历，24节气几乎所有日历的功能
*                          日历时间以1970年为元年，用32bit的时间寄存器可以运
*                          行到2100年左右
*History:         
*      <author>  <time>   <version >   <desc> 
*      Sundm    12/09/30     1.0     文件创建   
*******************************************************************************/

#ifndef __CALENDAR_H
#define __CALENDAR_H
#include "stm32f10x.h"

u8 GetChinaCalendar(u16  year,u8 month,u8 day,u8 *p);
void GetChinaCalendarStr(u16 year,u8 month,u8 day,u8 *str);
u8 GetJieQiStr(u16 year,u8 month,u8 day,u8 *str);

#endif 
