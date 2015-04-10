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
#include <led.h>
#include <key.h>


#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <stm32_eth.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif

static void rt_thread_entry_led(void* parameter)
{
  LED_Init();
  Key_Init();
  Set_Keyint();
  while(1)
  {
    LEDTog(LED1);
    LEDTog(LED2);
    LEDTog(LED3);
    LEDTog(LED4);
    rt_thread_delay(RT_TICK_PER_SECOND/2);
  }
}
static int led_tog_init(void)
{
    rt_thread_t init_thread;

    init_thread = rt_thread_create("led_tog",
                                   rt_thread_entry_led, RT_NULL,
                                   0x100, 0x1f, 1000);

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
    
    return 0;

}

  /* 指向线程控制块的指针*/
 static rt_thread_t tid1 = RT_NULL;
 static rt_thread_t tid2 = RT_NULL;
  
 /* 线程1入口*/ 
static void thread1_entry(void* parameter) 
{
  rt_uint32_t count = 0; 
 for(;count<10;count++) 
 { 
   rt_thread_delay(3*RT_TICK_PER_SECOND*10); 
    rt_kprintf("count = %d\n", count); 
 } 
} 

 /* 线程2入口*/
static void thread2_entry(void* parameter) 
{ 
 rt_tick_t tick; 
 rt_uint32_t i; 
 
  for(i=0; ; ++i) 
 { 
    tick = rt_tick_get(); 
   rt_thread_delay(RT_TICK_PER_SECOND*10);
  
  rt_kprintf("tick = %d\n",tick++); 
 } 
} 


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
        led_tog_init();
}

int rt_application_init(void)
{
    rt_thread_t tid;
    

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

/* 创建线程1 */
 tid1 = rt_thread_create("thread",
 thread1_entry, RT_NULL, /* 线程入口是thread1entry, 入口参数是RTNULL */
 512, 5, 5);
 if (tid1 != RT_NULL)
 rt_thread_startup(tid1);
 

 /* 创建线程2 */
 tid2 = rt_thread_create("thread",
 thread2_entry, RT_NULL, /* 线程入口是thread2entry, 入口参数是RTNULL */
512, 7, 5);
 if (tid2 != RT_NULL)
 rt_thread_startup(tid2);
 
	
    return 0;
}



/*@}*/
