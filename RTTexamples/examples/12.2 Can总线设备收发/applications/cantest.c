/****************************2012-2013, NJUT, Edu.****************************** 
FileName: cantest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.12.15
Description:    can设备操作测试程序 

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/12/15     1.0     文件创建   
********************************************************************************/ 

#include <rtthread.h>
#include <candevice.h>

/* can接收消息结构*/
struct rx_msg
{
  rt_device_t dev;
  rt_size_t size;
};


/* 用于接收消息的消息队列*/
static struct rt_messagequeue rx_mq;

/* 接收线程的接收缓冲区*/
/*注意在candevice.h中，设置 CAN_RX_LEN 长度 */
static CAN_msg_t can_msg;

/* 数据到达回调函数*/
rt_err_t candevice_input(rt_device_t dev, rt_size_t size)
{
  struct rx_msg msg;
  msg.dev = dev;
  msg.size = size;
  /* 发送消息到消息队列中*/
  rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
  return RT_EOK;
}

void device_thread_entry(void* parameter)
{

  struct rx_msg msg;
  int count = 0;
  rt_device_t device1;
  rt_err_t result = RT_EOK;
  rt_uint32_t rx_length;
  
  result = rt_mq_init(&rx_mq, "rx_mq", &msg, sizeof(struct rx_msg), sizeof(struct rx_msg)*20, RT_IPC_FLAG_FIFO);
  if (result != RT_EOK) 
  { 
      rt_kprintf("init message queue failed.\n"); 
  } 

  /* 查找系统中的 can1 设备 */
  device1 = rt_device_find("can1");
  if (device1 != RT_NULL)
  {
    /* 设置回调函数及打开设备*/
    rt_device_set_rx_indicate(device1, candevice_input);
    rt_device_open(device1, RT_DEVICE_OFLAG_RDWR);
  }
  
  while (1)
  {
    /* 从消息队列中读取消息*/
    result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 1000);
    if (result == -RT_ETIMEOUT)
    {
      /* 接收超时*/
      rt_kprintf("timeout count:%d\n", ++count);
    }
    /* 成功收到消息*/
    if (result == RT_EOK)
    {
      rx_length = msg.size ;
      /* 读取消息*/
      rx_length = rt_device_read(msg.dev, 0, &can_msg, rx_length);
      rt_device_write(device1, 0, &can_msg, 1);
    }
  }
}

void test_candevice()
{
  /* 创建devt线程*/
  rt_thread_t thread = rt_thread_create("devt",
  device_thread_entry, RT_NULL,
  1024, 25, 7);
  
  /* 创建成功则启动线程*/
  if (thread != RT_NULL)
    rt_thread_startup(thread);
}


#ifdef  RT_USING_FINSH 
 #include  <finsh.h> 
 /*  输出函数到finsh  shell中 */ 
FINSH_FUNCTION_EXPORT(test_candevice, device test e.g.test_candevice());
#endif
