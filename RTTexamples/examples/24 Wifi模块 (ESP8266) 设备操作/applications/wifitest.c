/*****************************2012-2016, NJUT, Edu.****************************** 
FileName: wifitest.c 
Author:  孙冬梅       Version :  1.0        Date: 2016.11.30
Description:   WIFI模块试程序 post 当前温度和 GET LED1
当前温度从11-99  
注意：以下内容 TEMPDATA LEDDATA必须更换成自己注册的YEELINK设备的ID号，否则 将发送到作者的设备上
device：350393
sensor：393199 397707
U-ApiKey: 91bd74f5f8b8a809aaa6037911f7c382

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    16/11/30     1.0     文件创建   
********************************************************************************/ 
#include "esp8266.h"
#include "led.h"

#define TEMPDATA "POST /v1.1/device/350393/sensor/393199/datapoints HTTP/1.1\r\nHost: api.yeelink.net\r\nU-ApiKey: 91bd74f5f8b8a809aaa6037911f7c382\r\nContent-Length:12\r\n\r\n{\"value\":23}"
#define LEDDATA "GET /v1.1/device/350393/sensor/397707/datapoints HTTP/1.1\r\nHost: api.yeelink.net\r\nU-ApiKey: 91bd74f5f8b8a809aaa6037911f7c382\r\n\r\n}"

char netdata[] = TEMPDATA;
uint8_t i = 11;

void test_wifi(void)
{
  rt_bool_t res = RT_TRUE;
  LED_Init();
  res = wificonfig();
  res = wifiinit(); 
  res = wifijap();  
  res = wificonnect(); 
  
  /*更换POST的数据内容*/
  i++; if(i>=99)i=11;
  netdata[rt_strlen(netdata)-3]= 0x30+ i/10;
  netdata[rt_strlen(netdata)-2]= 0x30+ i%10;
  /*更换POST的数据内容结束*/
  
  res = wifisend(netdata);//发送温度值 采用POST方法
  res = wifisend(LEDDATA); //获取灯值 采用GET方法
  
  res = wificloseconnect();
  res = wifiexit();  
}

#include "finsh.h"
FINSH_FUNCTION_EXPORT(test_wifi, wifi module test e.g. test_wifi());

