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

struct rt_thread thread1; 
char thread1_stack[512]; 


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

//入口程序 
void thread1_entry(void* parameter) 
{ 
 int i,j; 
  char *ptr[20]; /* 用于放置20个分配内存块的指针*/ 
 /* 对指针清零*/ 
  for (i = 0; i < 20; i ++)  
    ptr[i] = RT_NULL; 
  for(j = 0; j <2; j ++ )
 { 
    for (i = 0; i <20; i++) 
   { 
   /* 每次分配(1 <<i)大小字节数的内存空间*/ 
      ptr[i] = rt_malloc(1 <<i); 
     /* 如果分配成功*/ 
      if (ptr[i] != RT_NULL) 
     { 
    rt_kprintf("get memory: 0x%x\n", 
ptr[i]); 
    /* 释放内存块*/ 
    rt_free(ptr[i]); 
    ptr[i] = RT_NULL; 
     } 
   } 
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
    rt_err_t result;
    

    
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    
    /* 初始化线程1 */
    result = rt_thread_init(&thread1, "t1", /* 线程名：t1 */
        thread1_entry, RT_NULL, /* 线程的入口是thread1_entry，入口参数是RT_NULL*/
        &thread1_stack[0], sizeof(thread1_stack), /* 线程栈是thread1_stack */
        6, 10);
    if (result == RT_EOK) /* 如果返回正确，启动线程1 */
        rt_thread_startup(&thread1);
    
    return 0;
}



/*@}*/
