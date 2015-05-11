/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include "led.h"

#ifdef RT_USING_LWIP
#include <stm32_eth.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif

extern int ui_button(void);
extern int calibration(void);
extern void application_init(void);

void rt_init_thread_entry(void* parameter)
{
    {
        extern void rt_platform_init(void);
        rt_platform_init();
    }

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* initialize the device file system */
	dfs_init();

	/* initialize the elm chan FatFS file system*/
	elm_init();
    
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
    {
        rt_kprintf("File System initialzation failed!\n");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

#ifdef RT_USING_LWIP
	/* initialize lwip stack */
	/* register ethernetif device */
	eth_system_device_init();

	/* initialize lwip system */
	lwip_system_init();
	rt_kprintf("TCP/IP initialized!\n");
#endif

#ifdef RT_USING_FINSH
	/* initialize finsh */
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
        
#ifdef RT_USING_RTGUI
	{
                
		rt_device_t device;
		struct rt_device_rect_info info;
                
                extern int rtgui_system_server_init(void);
                rtgui_system_server_init();
                
		device = rt_device_find("lcd");
		/* re-set graphic device */
		rtgui_graphic_set_device(device);
		
		rtgui_system_server_init();

		/* 触摸屏校准
                calibration();*/
                               
		/* 测试触摸屏按钮 及简易计算器
		ui_button();*/
                               
                /*运行demo */
                //application_init();           
                
	}
#endif        

}

static void rt_thread_entry_led1(void* parameter)
{
  LED_Init(); 
  while (1)
    {  
      LEDTog(LED1);        
      rt_thread_delay(RT_TICK_PER_SECOND/2);  
    }
}
static void rt_thread_entry_led2(void* parameter)
{
  LED_Init();  
  while (1)
  {  
    LEDTog(LED2);        
    rt_thread_delay(RT_TICK_PER_SECOND/3);        
  }
}


int rt_application_init(void)
{ 
    rt_thread_t tid;
    rt_thread_t init_thread;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);
    init_thread = rt_thread_create("led1", rt_thread_entry_led1, RT_NULL, 512, 5, 5);
    if(init_thread != RT_NULL)
	{
		rt_thread_startup(init_thread);
	}
    init_thread = rt_thread_create("led2", rt_thread_entry_led2, RT_NULL, 1024, 6, 5);
    if(init_thread != RT_NULL)
	{
		rt_thread_startup(init_thread);
	}

    return 0;
}

/*@}*/
