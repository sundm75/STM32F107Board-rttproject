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

static rt_uint8_t thread_stack[1024]; /*  信号量控制块 */ 
static struct rt_semaphore static_sem; 
/*  指向信号量的指针 */ 
static rt_sem_t dynamic_sem = RT_NULL;
  ALIGN(RT_ALIGN_SIZE)         // 设置下一句线程栈数组为对齐地址 
//atic char thread1_stack[1024];  //设置线程堆栈为 1024Bytes 
struct rt_thread thread;        //定义静态线程数据结构 


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


 static void thread_entry(void* parameter) 
{ 
    rt_err_t result; 
    rt_tick_t tick; 
 
/* 1. staic semaphore demo */ 
    /*  获得当前的OS Tick */ 
    tick = rt_tick_get(); 
 
    /*  试图持有信号量，最大等待 10个OS Tick后返回 */ 
    result = rt_sem_take(&static_sem, 10); 
    if (result == -RT_ETIMEOUT) 
    { 
        /*  超时后判断是否刚好是 10个OS Tick */ 
        if (rt_tick_get() - tick != 10) 
        { 
            rt_sem_detach(&static_sem); 
            return; 
        } 
        rt_kprintf("take semaphore timeout\n"); 
    } 
    else 
    {  /*  因为没有其他地方释放信号量，所以不应该成功持有信号量 */ 
        rt_kprintf("take a static semaphore, failed.\n"); 
        rt_sem_detach(&static_sem); 
        return; 
    } 
  /*  释放一次信号量 */ 
    rt_sem_release(&static_sem); 
 
    /*  永久等待方式持有信号量 */ 
    result = rt_sem_take(&static_sem, RT_WAITING_FOREVER); 
    if (result != RT_EOK) 
    { 
        /*  不成功则测试失败 */ 
        rt_kprintf("take a static semaphore, failed.\n"); 
        rt_sem_detach(&static_sem); 
        return; 
    } 
 
    rt_kprintf("take a staic semaphore, done.\n"); 
 
    /*  脱离信号量对象 */ 
    rt_sem_detach(&static_sem);
 

    
   tick = rt_tick_get(); 
 
    /*  试图持有信号量，最大等待 10个OS Tick后返回 */ 
    result = rt_sem_take(dynamic_sem, 10); 
    if (result == -RT_ETIMEOUT) 
    { 
        /*  超时后判断是否刚好是 10个OS Tick */ 
        if (rt_tick_get() - tick != 10) 
        { 
            rt_sem_delete(dynamic_sem); 
            return; 
        } 
        rt_kprintf("take semaphore timeout\n"); 
    } 
    else 
    { 
        /*  因为没有其他地方释放信号量，所以不应该成功持有信号量，否则测试失败 
*/ 
        rt_kprintf("take a dynamic semaphore, failed.\n"); 
        rt_sem_delete(dynamic_sem); 
        return; 
}  /*  释放一次信号量 */ 
    rt_sem_release(dynamic_sem); 
 
    /*  永久等待方式持有信号量 */ 
    result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER); 
    if (result != RT_EOK) 
    { 
        /*  不成功则测试失败 */ 
        rt_kprintf("take a dynamic semaphore, failed.\n"); 
        rt_sem_delete(dynamic_sem); 
        return; 
    } 
 
    rt_kprintf("take a dynamic semaphore, done.\n"); 
    /*  删除信号量对象 */ 
    rt_sem_delete(dynamic_sem); 
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

/*  初始化静态信号量，初始值是 0 */ 
    result = rt_sem_init(&static_sem, "ssem", 0, RT_IPC_FLAG_FIFO); 
    if (result != RT_EOK) 
    { 
        rt_kprintf("init dynamic semaphore failed.\n"); 
        return -1; 
    } 
 
    
  result =rt_thread_init(&thread, 
                   "thread", 
               thread_entry, 
                   RT_NULL,&thread_stack[0], 
                   sizeof(thread_stack),11,5);//线程优先为 11, 时间片为 5 
    rt_thread_startup(&thread);
 /*  创建一个动态信号量，初始值是 0 */ 
    dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO); 
    if (dynamic_sem == RT_NULL) 
    { 
        rt_kprintf("create dynamic semaphore failed.\n"); 
        return -1; 
}
	
    return 0;
}



/*@}*/
