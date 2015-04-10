/************************************************* 
  Copyright (C), 2012, NJUT
  File name:      gprs.h
  Author:  sundm     Version:  1.0      Date: 2013.1.3 
  Description:    GPRS模块api接口函数 
  Others:        
  Function List:  
*************************************************/ 

#ifndef _GPRS
#define _GPRS

#include "rtthread.h"
#include "stm32f10x.h"
#include "gprsio.h"

rt_bool_t  sysconfig(void);// 查找GPRS串口设备并打开，注册回调函数，初始化串口接收事件
rt_bool_t  poweron(void); // 开机
rt_bool_t  poweroff(void);// 关机
rt_bool_t  gprsinit(void);// AT指令、查卡、查信号强度
rt_bool_t  gprsconnect(void);// 打开远程连接网络
rt_bool_t  closeconnect(void);// 关闭远程连接网络
rt_bool_t  gprssend(char * str);// 网络数据发送
rt_bool_t  gprsread(void);//  网络数据接收
rt_bool_t  gprstp(void);//  打开透传
rt_bool_t  gprstpoff(void);// 关闭透传
rt_bool_t  msgreaddata(void);// 读取短信
rt_bool_t  msgsend(char * str);// 发送短信

#endif



