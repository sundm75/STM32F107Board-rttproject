/****************************2012-2013, NJUT, Edu.****************************** 
FileName: clienttest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.07.30
Description:    网络客户端测试程序 

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/07/30     1.0     文件创建   
********************************************************************************/ 
#include <rtthread.h>
#include <lwip/api.h>
#include <finsh.h> 

static struct netconn* conn = RT_NULL;

#define NW_RX      0x01
#define NW_TX      0x02
#define NW_CLOSED   0x04
#define NW_MASK      (NW_RX | NW_TX | NW_CLOSED)

/* tx session structure */
struct tx_session
{
   rt_uint8_t *data;      /* data to be transmitted */
   rt_uint32_t length;      /* data length */
   rt_sem_t    ack;      /* acknowledge semaphore */
};
struct tx_session tx_data;
struct rt_event nw_event;
struct rt_semaphore nw_sem;

void rx_callback(struct netconn *conn, enum netconn_evt evt, rt_uint16_t len)
{
   if (evt == NETCONN_EVT_RCVPLUS)
   {
      rt_event_send(&nw_event, NW_RX);
   }
}

void process_rx_data(struct netbuf *buffer)
{
   rt_uint8_t *data;
   rt_uint16_t length;
   
   /* get data */
   netbuf_data(buffer, (void**)&data, &length);
   
   rt_kprintf("rx: %s\n", data);
}

void nw_thread(void* parameter)
{
   struct netbuf *buf;
    
   rt_err_t result;
   rt_uint32_t event;
   rt_err_t net_rev_result;

   /* set network rx call back */
   conn->callback = rx_callback;

   while (1)
   {
      /* receive network event */
      result = rt_event_recv(&nw_event, 
         NW_MASK, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
         RT_WAITING_FOREVER, &event);
      if (result == RT_EOK)
      {
         /* get event successfully */
         if (event & NW_RX)
         {
            /* do a rx procedure */
            net_rev_result = netconn_recv(conn, &buf);
            if (buf != RT_NULL)
            {
               process_rx_data(buf);
            }
         }
         
         if (event & NW_TX)
         {
            /* do a tx procedure */
            netconn_write(conn, tx_data.data, tx_data.length, NETCONN_COPY);
            
            /* tx done, notify upper application */
            rt_sem_release(tx_data.ack);
         }
         
         if (event & NW_CLOSED)
         {
            /* connection is closed */
            netconn_close(conn);
         }
      }
   }
}

void test_sendhit(void)
{
   static char hit_data[80];
   static rt_uint32_t hit = 0;
   
   if (conn != RT_NULL)
   {
      tx_data.data   = (rt_uint8_t*)&hit_data[0];
      tx_data.length = rt_sprintf(hit_data, "hit %d", hit ++);
      
      rt_kprintf("send hit: %s\n", tx_data.data);
      
      rt_event_send(&nw_event, NW_TX);
      
      /* wait ack */
      rt_sem_take(&nw_sem, RT_WAITING_FOREVER);
   }
}

void test_client(void)
{
   int err;
   struct ip_addr ip;
   rt_thread_t thread;
   
   /* create a TCP connection */
   conn = netconn_new(NETCONN_TCP);

   /* set ip address */
   IP4_ADDR(&ip, 192, 168, 1, 102);
   
   /* connect to server */
   err = netconn_connect(conn, &ip, 8999);
   
   rt_kprintf("connect error code: %d\n", err);
   
   /* connect OK */
   if (err == 0)
   {
      rt_kprintf("Connect OK, startup rx thread\n");
      
      /* init event */
      rt_event_init(&nw_event, "nw_event", RT_IPC_FLAG_FIFO);
      rt_sem_init(&nw_sem, "nw_sem", 0, RT_IPC_FLAG_FIFO);
      tx_data.ack = &nw_sem;

      /* create a new thread */
      thread = rt_thread_create("rx", nw_thread, RT_NULL,
         1024, 20, 20);
      if (thread != RT_NULL)
         rt_thread_startup(thread);
   }
}
FINSH_FUNCTION_EXPORT(test_client, tcp client demo e.g.test_client())
FINSH_FUNCTION_EXPORT(test_sendhit, send hit on network e.g.test_sendhit())


