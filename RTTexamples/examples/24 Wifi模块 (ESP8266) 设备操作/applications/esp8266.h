/************************************************* 
  Copyright (C), 2012, NJUT
  File name:      gprsio.c
  Author:  sundm     Version:  1.0      Date: 2013.1.3 
  Description:    sundm GPRS模块驱动接口函数 
  Others:        采用底层驱动，填充函数  
  Function List:  
*************************************************/ 

#ifndef _ESP8266
#define _ESP8266

#include "rtthread.h"
#include "stm32f10x.h"

rt_bool_t wificonfig();// 查找wifi串口设备并打开，注册回调函数，初始化串口接收事件
rt_bool_t wifiinit(); //wifi接入热点
rt_bool_t wifijap() ;
rt_bool_t wificonnect(); //打开远程连接网络
rt_bool_t wifisend(char * str); //网络数据发送
rt_bool_t wificloseconnect();// 关闭远程连接网络
rt_bool_t wifiexit(); // wifi退出热点

#endif



