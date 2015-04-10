/***************************2012-2013, NJUT, Edu.****************************** 
* File Name          : calendar.c     Version :  2.0      Date: 2013.07.30
* Author             : 孙冬梅转自：www.armjishu.com Team
* Description        : RTCConfig初始化后，Time_Display("")调用相关函数获得当前时间并显示
*History:         
*      <author>  <time>   <version >   <desc> 
*      Sundm    12/09/30     1.0     文件创建   
*******************************************************************************/
#include "stm32f10x.h"
#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include "RTC.h"
#include "date.h"
#include "calendar.h"
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/
static struct rtc_time systemtime;
static u8 const *WEEK_STR[] = {"日", "一", "二", "三", "四", "五", "六"};

/* Private function prototypes -----------------------------------------------*/
static void RTC_Configuration(void);
static void NVIC_Configuration(void);
uint32_t Time_Regulate(char* timestring);
void Time_Adjust(char* timestring);
uint8_t USART_Scanf(uint32_t value);

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : NVIC配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable the RTC Interrupt 使能RTC中断 */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : RTC配置 内部函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration(void)
{
  /* Enable PWR and BKP clocks */
  /* PWR时钟（电源控制）与BKP时钟（RTC后备寄存器）使能 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  /*使能RTC和后备寄存器访问 */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  /* 将外设BKP的全部寄存器重设为缺省值 */
  BKP_DeInit();

  /* Enable LSE */
  /* 使能LSE（外部32.768KHz低速晶振）*/
  RCC_LSEConfig(RCC_LSE_ON);
  
  /* Wait till LSE is ready */
  /* 等待外部晶振震荡稳定输出 */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}

  /* Select LSE as RTC Clock Source */
  /*使用外部32.768KHz晶振作为RTC时钟 */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  /* 使能 RTC 的时钟供给 */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  /*等待RTC寄存器同步 */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();

  /* Enable the RTC Second */
  /* 使能RTC的秒中断 */
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();
  
  /* Set RTC prescaler: set RTC period to 1sec */
  /* 32.768KHz晶振预分频值是32767,如果对精度要求很高可以修改此分频值来校准晶振 */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();
}

/*******************************************************************************
* Function Name  : mid
* Description    : 从字符串的中间截取n字符 内部函数
* Input          : dst为目的字符串 src 为源字符串 n为长度，m为位置
* Output         : None
* Return         : None
*******************************************************************************/
char* mid(char *dst,char *src, int n,int m) /**/
{
    char *p = src;
    char *q = dst;
    int len = strlen(src);
    if(n>len) n = len-m;    /*从第m个到最后*/
    if(m<0) m=0;    /*从第一个开始*/
    p += m;
    while(n--) *(q++) = *(p++);
    *(q++)='\0'; /*有必要吗？很有必要*/
    return dst;
}

/*******************************************************************************
* Function Name  : Time_Regulate
* Description    : 将输入时间字符串转换成标准tm格式时间 内部函数
* Input          : timestring为源字符串 
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t Time_Regulate(char* timestring)
{
  char *ch;  
  uint32_t k;
  struct rtc_time *tm;
  ch = rt_malloc(4); 
  tm = rt_malloc(sizeof(struct rtc_time));
  tm->tm_year = atoi(mid(ch,timestring,4,0));
  tm->tm_mon = atoi(mid(ch,timestring,2,4));
  tm->tm_mday = atoi(mid(ch,timestring,2,6));
  tm->tm_hour = atoi(mid(ch,timestring,2,8));
  tm->tm_min = atoi(mid(ch,timestring,2,10));
  tm->tm_sec = atoi(mid(ch,timestring,2,12));
  k = mktimev(tm);
  return k;
}

/*******************************************************************************
* Function Name  : Time_Adjust
* Description    : 把时间转化为RTC计数值写入RTC寄存器 内部函数
* Input          : timestring为源字符串 
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Adjust(char* timestring)
{
  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();
  
  /* Change the current time */
  /* 把时间转化为RTC计数值写入RTC寄存器 */
  RTC_SetCounter(Time_Regulate(timestring));
  
  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();
}

/*******************************************************************************
* Function Name  : Time_Display
* Description    : 显示当前时间及农历、节气， 外部函数
* Input          :  None
* Output         : timestr 返回“20130730081200”格式字符串
* Return         : None
*******************************************************************************/
void Time_Display( char *timestr)
{
  char  time[40] = {0x00};
  char  str[40] = {0x00};
  
  to_tm(RTC_GetCounter(), &systemtime);
  
  sprintf(time, "%04d%02d%02d%02d%02d%02d", systemtime.tm_year,systemtime.tm_mon, 
          systemtime.tm_mday, systemtime.tm_hour, systemtime.tm_min, systemtime.tm_sec);
  /*返回当前时间字符串*/
  strcpy(timestr,time); 
  
  /*农历显示*/
  {
      /* 计算农历 并打印显示*/
      GetChinaCalendar((u16)systemtime.tm_year, (u8)systemtime.tm_mon, (u8)systemtime.tm_mday, (u8*)str);
      rt_kprintf("\n\r\n\r  今天农历：%d%d,%d,%d", str[0], str[1], str[2],  str[3]);
      /* 计算农历年份 并打印显示*/
      GetChinaCalendarStr((u16)systemtime.tm_year,(u8)systemtime.tm_mon,(u8)systemtime.tm_mday,(u8*)str);
      rt_kprintf("  %s", str);
      /* 计算农历节气 */
     if(GetJieQiStr((u16)systemtime.tm_year, (u8)systemtime.tm_mon, (u8)systemtime.tm_mday, (u8*)str))
       rt_kprintf("  %s\n\r", str);

  }
  /* 输出公历时间 */
  rt_kprintf("\r  当前时间为: %d年 %d月 %d日 (星期%s)  %d:%d:%d \r\n", 
                    systemtime.tm_year, systemtime.tm_mon, systemtime.tm_mday, 
                    WEEK_STR[systemtime.tm_wday], systemtime.tm_hour, 
                    systemtime.tm_min, systemtime.tm_sec);
}

/*******************************************************************************
* Function Name  : RTCConfig
* Description    : 时钟配置 外部函数
* Input          : flag: 为0表示由系统决定是否修改时间，为1表示强制修改时间
*                   timestring 要写入的时间
* Output         : timestr 返回“20130730081200”格式字符串
* Return         : None
*******************************************************************************/
void RTCConfig(char* timestring, char flag)
{
	NVIC_Configuration();//使能RTC中断 
  
  /* 以下if...else.... if判断系统时间是否已经设置，判断RTC后备寄存器1的值
     是否为事先写入的0XA5A5，如果不是，则说明RTC是第一次上电，需要配置RTC，
     提示用户通过串口更改系统时间，把实际时间转化为RTC计数值写入RTC寄存器,
     并修改后备寄存器1的值为0XA5A5。
     else表示已经设置了系统时间，打印上次系统复位的原因，并使能RTC秒中断
        flag: 为0表示由系统决定是否修改时间，为1表示强制修改时间
  */
  if ((BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) || (flag==1))
  {
    /* Backup data register value is not correct or not yet programmed (when
       the first time the program is executed) */
    rt_kprintf("\r\n\n RTC not yet configured....");

    /* RTC Configuration */
    RTC_Configuration();

    rt_kprintf("\r\n RTC configured....");

    /* Adjust time by values entred by the user on the hyperterminal */
    Time_Adjust(timestring);
    /* 修改后备寄存器1的值为0XA5A5 */
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  }
  else
  {
    /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
      rt_kprintf("\r\n\n Power On Reset occurred....");
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
      rt_kprintf("\r\n\n External Reset occurred....");
    }

    rt_kprintf("\r\n No need to configure RTC....");
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
  }

#ifdef RTCClockOutput_Enable
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
      
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Disable the Tamper Pin */
  BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
                                 functionality must be disabled */

  /* Enable RTC Clock Output on Tamper Pin */
  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif

  /* Clear reset flags */
  RCC_ClearFlag();
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : RTC秒中断函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
  /*  判断RTC是否发生了秒中断（也有可能是溢出或者闹钟中断) */
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    /* Clear the RTC Second interrupt */
    /* 清除秒中断标志 */
    RTC_ClearITPendingBit(RTC_IT_SEC);

    /* Enable time update */
    /* 如果时间已经设置好，则打印时间 */
//    if(TimeSeted) Time_Display(RTC_GetCounter());      

    /* Wait until last write operation on RTC registers has finished */
    /* 等待上一次对RTC寄存器的写操作完成 */
    RTC_WaitForLastTask();
    
    /* Reset RTC Counter when Time is 23:59:59 */
    /* 如果时间达到23:59:59则下一刻时间为00:00:00 */
    if (RTC_GetCounter() == 0x00015180)
    {
      RTC_SetCounter(0x0);
      
      /* Wait until last write operation on RTC registers has finished */
      /* 等待上一次对RTC寄存器的写操作完成 */
      RTC_WaitForLastTask();
    }
  }
}


