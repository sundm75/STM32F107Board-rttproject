/***************************2012-2016, NJUT, Edu.******************************* 
FileName: esp8266.c 
Author:  孙冬梅       Version :  1.0       Date: 2016.11.26
Description:   wifi模块通信
Version:         1.0
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    16/11/10    1.0       文件创建  

Description：设备初始化，查找wifi串口(UART4)设备打开，注册回调函数，初始化串口接收事件。
开启线程，监视串口数据。
发送AT指令时，关闭串口监视线程。
串口收到数据后，运行数据到达回调函数,发送事件到wifi_send_data_package。
wifi_send_data_package中接收数据，并检查数据。
使用  PC10-USART4Tx PC11-USART4Rx CS-PC9
Others:   串口接收数据后，检测 关键字{"value":**}来获取 GET方法得到的数据
  Function List:  
   1. wificonfig() 查找wifi串口设备并打开，注册回调函数，初始化串口接收事件
   2. wifiinit(); wifi设置
   3. wifijap() ；加入AP
   4. wificonnect(); 打开远程连接网络
   5. wifisend("abc"); 网络数据发送
   6. wificloseconnect(); 关闭远程连接网络
   7. wifiexit();  wifi退出AP 并关闭
*******************************************************************************/ 
#include  <rtthread.h >
#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "led.h"


//定义wifi使用串口 和 控制口（CS）
#define WIFI_PORT GPIOC
#define WIFI_PORT_RCC        RCC_APB2Periph_GPIOC
#define WIFI_CS_PIN GPIO_Pin_9

//定义AT指令
#define ESP8266_ATCMD "AT\x00D\x00A"               // AT查询
#define ESP8266_RESET "AT+RST\x00D\x00A"           // 模块复位

#define ESP8266_CWMODE_STA "AT+CWMODE=1\x00D\x00A"      // 选择WiFi应用模式  Station模式
#define ESP8266_CWMODE_AP "AT+CWMODE=2\x00D\x00A"      // 选择WiFi应用模式  AP模式
#define ESP8266_CWMODE_APSTA "AT+CWMODE=3\x00D\x00A"      // 选择WiFi应用模式  Station+AP模式

#define ESP8266_CWLAP "AT+CWLAP\x00D\x00A"      // 列出当前接入点
#define ESP8266_CWQAP "AT+CWQAP\x00D\x00A"      // 退出
#define ESP8266_CIFSR "AT+CIFSR\x00D\x00A"      // 获取本地IP地址

#define ESP8266_CWJAP "AT+CWJAP=\"sundm75\",\"121111215\"\x00D\x00A"      // 加入接入点 TP-LINK_sundm
#define ESP8266_CIPMUX "AT+CIPMUX=0\x00D\x00A"      // 设置单连接
#define ESP8266_CIPMODE "AT+CIPMODE=1\x00D\x00A"      // 设置透传模式

#define ESP8266_CIPSTART "AT+CIPSTART=\"TCP\",\"42.96.164.52\",80\x00D\x00A"      // 建立TCP/UDP连接
#define ESP8266_CIPSTATUS "AT+CIPSTATUS\x00D\x00A"      // 获得TCP/UDP连接状态
#define ESP8266_CIPSEND "AT+CIPSEND="      // 发送数据

#define ESP8266_CIPCLOSE "AT+CIPCLOSE\x00D\x00A"      // 关闭TCP/UDP连接

/***************************WIFI模块串口接收數據事件******************************/
#define REV_DATA      0x01
#define REV_WATCH      0x02
#define REV_STOPWATCH      0x04

#define REV_MASK      ( REV_DATA | REV_WATCH | REV_STOPWATCH )


static struct rt_event rev_event;
static rt_device_t wifi_device;


/* 监视WIFI串口线程入口*/
void wifiwatch_entry(void* parameter)
{
  rt_err_t result = RT_EOK;
  rt_uint32_t event;
  char wifi_rx_buffer[512]={0x00};
  rt_size_t  readnum;
  char * charaddr;
  char * charstartaddr;
 char *  charendaddr;
  uint8_t valuestr[3] = {0x00};
  uint8_t value = 0;
  
  while(1)
  {
      result = rt_event_recv(&rev_event, 
         REV_MASK, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
         RT_WAITING_FOREVER, &event);
      if (result == RT_EOK)
      {
        if (event & REV_DATA)
        {
          rt_memset(wifi_rx_buffer,0x00,sizeof(wifi_rx_buffer));
          rt_thread_delay(RT_TICK_PER_SECOND/2);
          readnum = rt_device_read(wifi_device, 0, wifi_rx_buffer, 512);
          rt_kprintf(wifi_rx_buffer);
          /*以下获取串口接收数据 中的 value值 */
          charaddr = rt_strstr(wifi_rx_buffer,"\"value\":");
          if(charaddr!=RT_NULL)
          {
            rt_kprintf(charaddr);    
            charstartaddr = charaddr + 8;
            charendaddr = rt_strstr(charaddr,"}") ;
            if(charendaddr!=RT_NULL)
            {
              int i=0;
              while (charstartaddr!=charendaddr)
              {
                valuestr[i] = *charstartaddr;
                charstartaddr++;i++;
              }
              value = atoi((char const*)valuestr);
              rt_kprintf("\r\n Wifi Device receive value = %d \r\n",value );
              if(value==1)
              {
                LEDOn(LED1);
              }
              else if(value==0)
              {
                LEDOff(LED1);
              }              
            }
          }
          /*获取串口接收数据 中的 value值 结束 */
        }
        if (event & REV_STOPWATCH)
        {
          return;
        }
      }
    }
}

void wifiwatch(void)
{
  /* 创建wifi watch线程*/
  rt_thread_t thread = rt_thread_create("wifiwatch",
  wifiwatch_entry, RT_NULL,
  1024, 25, 7);
  
  /* 创建成功则启动线程*/
  if (thread != RT_NULL)
    rt_thread_startup(thread);
}
void wifistopwatch(void)
{
  rt_event_send(&rev_event, REV_STOPWATCH);
}

/*数据到达回调函数,发送事件到wifi_send_data_package*/
static rt_err_t wifi_uart_input(rt_device_t dev, rt_size_t size)
{
  rt_event_send(&rev_event, REV_DATA);
  return RT_EOK;
}

/*WIFI串口发送和接收*/
rt_bool_t wifi_send_data_package(char *cmd,char *ack,uint16_t waittime, uint8_t retrytime)
{
  rt_bool_t res = RT_FALSE; 
  rt_err_t result = RT_EOK;
  rt_uint32_t event;
  char wifi_rx_buffer[512]={0x00};
  rt_thread_t thread;
  
  thread = rt_thread_find("wifiwatch");
  if( thread != RT_NULL)
    rt_thread_delete(thread);
  
  do 
  {
    rt_device_write(wifi_device, 0, cmd, rt_strlen(cmd));   
    result = rt_event_recv(&rev_event, 
       REV_MASK, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
       waittime*RT_TICK_PER_SECOND, &event);
    if (result == RT_EOK)
    {
      if (event & REV_DATA)
      {
        rt_memset(wifi_rx_buffer,0x00,sizeof(wifi_rx_buffer));
        rt_device_read(wifi_device, 0, wifi_rx_buffer, 512);
        rt_kprintf(wifi_rx_buffer);
        if((rt_strstr(wifi_rx_buffer,ack))||(rt_strstr(wifi_rx_buffer,"OK")))
          res = RT_TRUE;
        else
          res = RT_FALSE;
      }
    }
    retrytime--;
  }while((!res)&&(retrytime>=1));
  wifiwatch();
  return res;
} 


/*WIFI端口初始化，打开设备，注册回调函数*/
rt_bool_t wificonfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
  RCC_APB2PeriphClockCmd(WIFI_PORT_RCC, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = WIFI_CS_PIN; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(WIFI_PORT, &GPIO_InitStructure);
  //GPIO_SetBits(WIFI_PORT, WIFI_CS_PIN);
  
  wifi_device = rt_device_find("uart4");
  
  if (wifi_device != RT_NULL)    
  {
    rt_kprintf("\r\n Wifi port initialized!\r\n");
    /* 设置回调函数及打开设备*/
    rt_device_set_rx_indicate(wifi_device, wifi_uart_input);
    rt_device_open(wifi_device, RT_DEVICE_OFLAG_RDWR);  
  }
  else
  {
    rt_kprintf("\r\n Wifi port not find !\r\n");
    return RT_FALSE;
  }
  /*WIFI串口打开后，初始化串口接收事件*/
  rt_event_init(&rev_event, "rev_ev", RT_IPC_FLAG_FIFO);
  return RT_TRUE;
}

rt_bool_t wifiinit() //wifi接入AP
{
  if(wifi_send_data_package(ESP8266_ATCMD,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi AT OK !\r\n");
  }
  
  if(wifi_send_data_package(ESP8266_CWMODE_STA,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi AT OK !\r\n");
  }

  rt_kprintf("\r\n Wifi Reset! Display information from module:\r\n");
  if(wifi_send_data_package(ESP8266_RESET,"ready",4,1))
  {
    rt_kprintf("\r\n Wifi Reset OK !\r\n");
  }
  rt_thread_delay(RT_TICK_PER_SECOND*5);
 
  rt_thread_delay(RT_TICK_PER_SECOND*5);
  
  //rt_kprintf("\r\n Wifi  当前AP：\r\n");
  //if(wifi_send_data_package(ESP8266_CWLAP,"OK",1,1))//列出当前接入点
 // {
 //   rt_device_write(wifi_device, 0, ESP8266_CWLAP, rt_strlen(ESP8266_CWLAP)); 
 // }
 // rt_thread_delay(RT_TICK_PER_SECOND*10);
//  else
//    return RT_FALSE; 
  
  
//  if(wifi_send_data_package(ESP8266_CIPSTATUS,"OK",2,1))// 获得TCP/UDP连接状态
//  {
//  }
//  rt_kprintf("\r\n Wifi 获得TCP/UDP连接状态");
//  rt_thread_delay(RT_TICK_PER_SECOND*1);
//  else
//    return RT_FALSE; 
    return RT_TRUE; 
}
rt_bool_t wifijap() //加入AP
{
  rt_kprintf("\r\n Wifi  准备 接入AP sundm75\r\n");

  if(wifi_send_data_package(ESP8266_CWJAP,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi Join AP OK  !\r\n");
  }
  rt_thread_delay(RT_TICK_PER_SECOND*5);
  return RT_TRUE; 
}

rt_bool_t wificonnect() //打开远程连接网络
{
  rt_kprintf("\r\n Wifi Connect %s \r\n",ESP8266_CIPSTART);

  if(wifi_send_data_package(ESP8266_CIPSTART,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi connect OK !\r\n");
  }

    rt_thread_delay(RT_TICK_PER_SECOND*5);
    return RT_TRUE; 
}

rt_bool_t wifisend(char * str) //网络数据发送
{
  uint8_t lenstr[64]={0x00};
  rt_kprintf("\r\n Wifi send data! Display information from module:\r\n");
  sprintf((char*)lenstr,"%s%d\r\n",ESP8266_CIPSEND,strlen(str));
  if(wifi_send_data_package((char*)lenstr,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi send %d data  OK !\r\n",strlen(str));
  }

  if(wifi_send_data_package(str,"OK",1,1))
  {
  }
   rt_thread_delay(RT_TICK_PER_SECOND*5);
  
    return RT_TRUE; 
}


rt_bool_t wificloseconnect()// 关闭远程连接网络
{
  rt_kprintf("\r\n Wifi closeconnect! Display information from module:\r\n");

  if(wifi_send_data_package(ESP8266_CIPCLOSE,"OK",3,1))
  {
    rt_kprintf("\r\n Wifi closeconnect OK !\r\n");
  }
    rt_thread_delay(RT_TICK_PER_SECOND*5);

    return RT_TRUE; 
}


rt_bool_t wifiexit() // wifi退出AP
{
  rt_kprintf("\r\n Wifi wifi exit! Display information from module:\r\n");

  if(wifi_send_data_package(ESP8266_CWQAP,"OK",2,1))
  {
    rt_kprintf("\r\n Wifi exit OK !\r\n");
  }
  else
    return RT_FALSE; 
    return RT_TRUE; 
}



