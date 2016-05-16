/*******************************2012-2014, NJUT, Edu.************************** 
FileName: testgui.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.06.04
Description:    触摸屏驱动测试   

    1.test_gui() ：显示外框，及中间字符串。
    2.startgui()：初始化GUI，清屏 ，执行完该数后，才可分别运行以下函数
      2.1 DynamicCali() ：动态校准函数
      2.2 StopCali()：退出动态校准程序
      2.3 TouchCali() ：触摸屏校准函数
      2.4 guidemo() : 运行demo函数

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/06/04     1.0     文件创建   
******************************************************************************/ 
#include "rtthread.h"
#include "timer5.h"
#include "touch.h"
#include "GUI.h"
#include "GUIDEMO.h"
#include "rs485.h"

uint8_t GUI_Initialized   = 0;

/*******************************************************************************
*  Function Name  :   testgui 
*  Description    : GUI测试函数，初化后，显示数据
*  Input          : None
*  Output         : None
*  Return         : None
*******************************************************************************/
void test_gui(void)
{
  int i;  
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    GUI_Init();
    GUI_Initialized = 1;
    GUI_DispChars('/', 53);     
    for( i= 0; i < 240; i += 8)   
    {        
        GUI_DispCharAt('/', 312, i);
        GUI_DispCharAt('/', 1,   i);
    } 
    GUI_DispChars('/', 52); 
    GUI_SetFont(&GUI_Font24B_ASCII);
    GUI_SetColor(GUI_RED); 
    GUI_DispStringAt("This is a test program!",10,110);
}

void rt_gui_thread_entry(void* parameter) 
{
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
  /*设置默认创建标记WM 然后会将 WM_PAINT 消息输出重定向到存储设备中，再复制到显
  示器中。如果整个窗口的内存不够，会自动使用分段*/
  WM_SetCreateFlags(WM_CF_MEMDEV);
  touch_config();
  Timer5Config();
  GUI_Init();
  GUI_Initialized = 1;
  ADC_Config();
  LED_Init();
  RS485GPIOConfig();
  RS485Write();
  
  GUIDEMO_Main();

}

void startgui(void)
{
    rt_thread_t init_thread;
    init_thread = rt_thread_create("GUI_Exec", rt_gui_thread_entry, RT_NULL, 0x0800, 6, 5); 
    if(init_thread != RT_NULL)
    {
            rt_thread_startup(init_thread);
    }
}

void guidemo(void)
{
	GUIDEMO_Main(); 
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(test_gui, startup test gui e.g.test_gui());
FINSH_FUNCTION_EXPORT(startgui, startup startgui);
FINSH_FUNCTION_EXPORT(guidemo, startup guidemo);
#endif

