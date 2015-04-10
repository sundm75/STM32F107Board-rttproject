/***************************2012-2013, NJUT, Edu.****************************** 
* File Name          : calendar.c     Version :  2.0      Date: 2013.07.30
* Author             : 孙冬梅转自：www.armjishu.com Team
* Description        : RTCConfig初始化后，Time_Display("")调用相关函数获得当前时间并显示
*History:         
*      <author>  <time>   <version >   <desc> 
*      Sundm    12/09/30     1.0     文件创建   
*******************************************************************************/
#ifndef __RTC_H
#define __RTC_H

//#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

void RTCConfig(char* timestring, char flag);//  flag: 为0表示由系统决定是否修改时间，为1表示强制修改时间
void Time_Display( char *timestr);//显示当前时间并返回


#endif
