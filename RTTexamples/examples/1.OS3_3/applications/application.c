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

static rt_uint8_t *ptr[48]; 
static rt_uint8_t mempool[4096]; 
static struct rt_mempool mp; 
 
static rt_thread_t tid1 = RT_NULL; 
static rt_thread_t tid2 = RT_NULL; 

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

static void thread1_entry(void* parameter) 
{ 
    int i,j = 1; 
    char *block; 
 
    while(j--) 
    { 
        for (i = 0; i < 48; i++) 
        { 
            /*  申请内存块 */ 
            rt_kprintf("allocate No.%d\n", i); 
            if (ptr[i] == RT_NULL) 
            { 
                ptr[i] = rt_mp_alloc(&mp, RT_WAITING_FOREVER);
            } 
        } 
 
        /*  继续申请一个内存块，因为已经没有内存块，线程应该被挂起 */ 
        block = rt_mp_alloc(&mp, RT_WAITING_FOREVER); 
        rt_kprintf("allocate the block mem\n"); 
        /*  释放这个内存块 */ 
        rt_mp_free(block); 
        block = RT_NULL; 
    } 
} 
 
/*  线程2 入口，线程2 的优先级比线程1 低，应该线程 1 先获得执行。*/ 
static void thread2_entry(void *parameter) 
{     int i,j = 1; 
 
    while(j--) 
    { 
        rt_kprintf("try to release block\n"); 
 
        for (i = 0 ; i < 48; i ++) 
        { 
            /*  释放所有分配成功的内存块 */ 
            if (ptr[i] != RT_NULL) 
            { 
                rt_kprintf("release block %d\n", i); 
 
                rt_mp_free(ptr[i]); 
                ptr[i] = RT_NULL; 
            } 
        } 
 
        /*  休眠10个OS Tick */ 
        rt_thread_delay(10); 
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
    
    int i; 
    for (i = 0; i < 48; i ++) ptr[i] = RT_NULL; 
 
    /*  初始化内存池对象 ,每块分配的大小为80，但是另外还有大小为4 的控制头，
所以实际大小为84*/ 
    rt_mp_init(&mp, "mp1", &mempool[0], sizeof(mempool), 80); 
    
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    /*  创建线程1 */ 
    tid1 = rt_thread_create("t1", 
    thread1_entry, RT_NULL, 512, 8, 10);     if (tid1 != RT_NULL) 
        rt_thread_startup(tid1); 
 
    /*  创建线程2 */ 
    tid2 = rt_thread_create("t2", 
    thread2_entry, RT_NULL, 512, 9, 10); 
    if (tid2 != RT_NULL) 
        rt_thread_startup(tid2); 
    
    return 0;
}



/*@}*/
