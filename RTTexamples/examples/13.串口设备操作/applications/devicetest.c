/***************************2012-2014, NJUT, Edu.******************************* 
FileName: devicetest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.05.10
Description:    usart2,3 串口设备操作：
            * 将启动一个devt线程，然后打开串口2和3
            * 当串口2和3有输入时，将读取其中的输入数据然后写入到
            * 串口3和2设备中。
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/05/10     1.0     文件创建   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |      FunctionPin            |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |      USART2_TX              |        PD5                  |
  *          |      USART2_RX              |        PD6                  |
  *          +-----------------------------+-----------------------------+
  *          |      USART3_TX              |        PC10                  |
  *          |      USART3_RX              |        PC11                  |
  *          +-----------------------------+-----------------------------+
*******************************************************************************/ 
#include <rtthread.h>
#include "rs485.h"

/* UART接收消息结构*/
struct rx_msg
{
  rt_device_t dev;
  rt_size_t size;
};


/* 用于接收消息的消息队列*/
static struct rt_messagequeue rx_mq;

/* 接收线程的接收缓冲区*/
/*注意在serial.h中，设置UART_RX_BUFFER_SIZE长度 */
static char uart_rx_buffer[1024];

/* 数据到达回调函数*/
rt_err_t uartdevice_input(rt_device_t dev, rt_size_t size)
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
  rt_device_t device2, device3;
  rt_err_t result = RT_EOK;
  rt_uint32_t rx_length;
  
  RS485GPIOConfig();
  //485转换芯片对PD5PD6的接收有影响
  RS485Write();//uart2连接sp3485,防止输入对PD5PD6干扰，操作前先置写状态
  
  result = rt_mq_init(&rx_mq, "rx_mq", &msg, sizeof(struct rx_msg), sizeof(struct rx_msg)*20, RT_IPC_FLAG_FIFO);
  if (result != RT_EOK) 
  { 
      rt_kprintf("init message queue failed.\n"); 
  } 

  /* 查找系统中的串口2设备 */
  device2 = rt_device_find("uart2");
  if (device2!= RT_NULL)
  {
    /* 设置回调函数及打开设备*/
    rt_device_set_rx_indicate(device2, uartdevice_input);
    rt_device_open(device2, RT_DEVICE_OFLAG_RDWR);
  }
  
  /* 查找系统中的串口3设备 */
  device3 = rt_device_find("uart3");
  if (device3 != RT_NULL)
  {
    /* 设置回调函数及打开设备*/
    rt_device_set_rx_indicate(device3, uartdevice_input);
    rt_device_open(device3, RT_DEVICE_OFLAG_RDWR);
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
      rx_length = (sizeof(uart_rx_buffer) - 1) > msg.size ?
      msg.size : sizeof(uart_rx_buffer) - 1;
      /* 读取消息*/
      rx_length = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length);
      uart_rx_buffer[rx_length] = '\0';
      /* 写到写设备中*/
      rt_device_write(device2, 0, &uart_rx_buffer[0], rx_length);
      rt_device_write(device3, 0, &uart_rx_buffer[0], rx_length);
    }
  }
}

void test_device()
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
FINSH_FUNCTION_EXPORT(test_device, device test e.g.test_device());
#endif