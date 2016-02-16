/****************************2012-2013, NJUT, Edu.****************************** 
FileName: gprstest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.07.30
Description:   GPRS模块试程序 
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/07/30     1.0     文件创建   
********************************************************************************/ 
#include "gprs.h"
void test_gprs(void)
{
  rt_bool_t res = RT_TRUE; 
  res = sysconfig(); if(!res) return;
  
  res = poweron(); if(!res) return;
  rt_thread_delay(2000);
  
  res = gprsinit(); if(!res) return;
  
  res = msgsend("abc"); if(!res) return;
  res = msgreaddata(); //if(!res) return;
 
  res = gprsconnect(); if(!res) return;
  res = gprssend("abc"); if(!res) return;
  res = gprsread();   if(!res) return;
  
  res = gprstp(); if(!res) return;
  res = gprstpoff(); if(!res) return;
  
  res = closeconnect(); if(!res) return;
  
  poweroff(); 
}

#include "finsh.h"
FINSH_FUNCTION_EXPORT(test_gprs, gprs module test e.g. test_gprs());
FINSH_FUNCTION_EXPORT(sysconfig, gprs module test e.g. sysconfig());
FINSH_FUNCTION_EXPORT(poweron, gprs module test e.g. poweron());
FINSH_FUNCTION_EXPORT(gprsinit, gprs module test e.g. gprsinit());
FINSH_FUNCTION_EXPORT(msgsend, gprs module test e.g. msgsend("123"));
FINSH_FUNCTION_EXPORT(msgreaddata, gprs module test e.g. msgreaddata());
FINSH_FUNCTION_EXPORT(gprsconnect, gprs module test e.g. gprsconnect());
FINSH_FUNCTION_EXPORT(gprssend, gprs module test e.g. gprssend("abc"));
FINSH_FUNCTION_EXPORT(gprsread, gprs module test e.g. gprsread());
FINSH_FUNCTION_EXPORT(gprstp, gprs module test e.g. gprstp());
FINSH_FUNCTION_EXPORT(gprstpoff, gprs module test e.g. gprstpoff());
FINSH_FUNCTION_EXPORT(closeconnect, gprs module test e.g. closeconnect());
FINSH_FUNCTION_EXPORT(poweroff, gprs module test e.g. poweroff());
