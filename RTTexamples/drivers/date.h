/***************************2012-2013, NJUT, Edu.****************************** 
* File Name          : date.h     Version :  2.0      Date: 2013.07.30
* Author             : 孙冬梅转自：www.armjishu.com Team
* Description        : 日期相关函数
*History:         
*      <author>  <time>   <version >   <desc> 
*      Sundm    12/09/30     1.0     文件创建   
*
*******************************************************************************/
#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
};
    
void GregorianDay(struct rtc_time * tm);
uint32_t mktimev(struct rtc_time *tm);
void to_tm(uint32_t tim, struct rtc_time * tm);

#endif
