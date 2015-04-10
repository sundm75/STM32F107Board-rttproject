/****************************2012-2014, NJUT, Edu.****************************** 
FileName: porttcp.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.10.30
Description:   Modbus TCP的网络驱动代码 ，基于LWIP编写。
1.初始化,监听502端口，如果有连接到来，则建立连接，并监视接收数据.
2.接收数据在函数xMBTCPPortGetRequest中搬迁入FreeModbus.
3.要发送的数据在xMBTCPPortSendResponse中从FreeModbus搬迁出来，并从TCP/IP中发送出去。

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/10/30     1.0     文件创建   
********************************************************************************/ 
/* -------------------------- LWIP includes ---------------------------------*/
#include <lwip/api.h>
#include <finsh.h> 

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines  -----------------------------------------*/
#define MB_TCP_DEFAULT_PORT  502          /* TCP listening port. */
#define MB_TCP_BUF_SIZE     ( 256 + 7 )   /* Must hold a complete Modbus TCP frame. */

/* ----------------------- Prototypes ---------------------------------------*/
static  UCHAR    ucTCPRequestFrame[MB_TCP_BUF_SIZE];
static  USHORT   ucTCPRequestLen;

static  UCHAR    ucTCPResponseFrame[MB_TCP_BUF_SIZE];
static  USHORT   ucTCPResponseLen;

/* ----------------------- Static variables ---------------------------------*/
static  USHORT usPort;

/* ----------------- Net Server  Defines & variables-------------------------*/
static struct netconn* newconn = RT_NULL;

#define NW_RX      0x01
#define NW_TX      0x02
#define NW_CLOSED   0x04
#define NW_MASK      (NW_RX | NW_TX | NW_CLOSED)

#define ETH_LINK_ERR -1;
#define ETH_LINK_OK 0;

struct rt_event nw_event;

/* ----------------------- Static functions ---------------------------------*/

/*******************************************************************************
* Functions Name : rx_callback; process_rx_data; nw_thread
* Description    : 网络数据接收回调函数; 处理接收到数据; 网络连接线程函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*  网络数据接收回调函数 */
void rx_callback(struct netconn *conn, enum netconn_evt evt, rt_uint16_t len)
{
   if (evt == NETCONN_EVT_RCVPLUS)
   {
      rt_event_send(&nw_event, NW_RX);
   }
}

/*  处理接收到数据 */
void process_rx_data(struct netbuf *buffer)
{
    rt_uint8_t *data;
    rt_uint16_t length;
    int i;    

    /* get data */
    netbuf_data(buffer, (void**)&data, &length);
    rt_kprintf("\r\n Modbus TCP receive len = %d, rx data:\n",length);
    for( i=0; i<length; i++)
    {
      rt_kprintf("0x%02X ",data[i]);
    }
    rt_kprintf("\r\n ");
    
    /* 获得modbus请求 */
    memcpy(ucTCPRequestFrame, data, length );
    ucTCPRequestLen = length;
    
    /* 向 modbus poll发送消息 */
    xMBPortEventPost( EV_FRAME_RECEIVED );
}

/*  网络数据发送,接收,关闭线程 */
void nw_thread(void* parameter)
{
   struct netbuf *buf;
   int i;
   rt_err_t result;
   rt_uint32_t event;
   rt_err_t net_rev_result;

   /* set network rx call back */
   newconn->callback = rx_callback;

  /*  循环处理，直到线程关闭*/
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
            /* 接收数据并处理 */
            net_rev_result = netconn_recv(newconn, &buf);
            if (buf != RT_NULL)
            {
               process_rx_data(buf);
            }
            netbuf_delete(buf);
         }
         
         if (event & NW_TX)
         {
            /* LWIP发送Modbus应答数据包 */
            netconn_write(newconn,  ucTCPResponseFrame , ucTCPResponseLen, NETCONN_NOCOPY );
            rt_kprintf("\r\n Modbus TCP send len = %d, send data:\n",ucTCPResponseLen);
            for( i=0; i<ucTCPResponseLen; i++)
            {
              rt_kprintf("0x%02X ",ucTCPResponseFrame[i]);
            }
            rt_kprintf("\r\n ");
         }
         
         if (event & NW_CLOSED)
         {
            /* connection is closed */
            netconn_close(newconn);
            netconn_delete(newconn);
            rt_event_detach(&nw_event);
            return;
         }
      }
   }
}


/*******************************************************************************
* Functions Name : checkethlink; mbtcp_srv_thread
* Description    : 网络物理层查询连接状态; modbus tcp 服务器线程
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*网络物理层查询连接状态*/
static int checkethlink(void)
{
    int temp;
    extern uint16_t ETH_ReadPHYRegister(uint16_t PHYAddress, uint16_t PHYReg);
    int eth_state = ETH_LINK_ERR;
    temp = ETH_ReadPHYRegister(1, 1);
    temp = ETH_ReadPHYRegister(1, 1);
    if(temp&0x04)//检测BMSR的第2位 link状态，为1时联接，为0时断开
    {
      eth_state = ETH_LINK_OK;
    }
    return eth_state;
}

/* modbus tcp 服务器线程 */
void mbtcp_srv_thread(void* paramter)
{
  rt_err_t net_acp_result;
  struct netconn *conn;
  rt_thread_t thread;

  /*  建立一个新的TCP连接句柄 */
  conn = netconn_new(NETCONN_TCP);

  /*  将连接绑定在任意的本地IP地址的502端口上 */
  netconn_bind(conn, NULL, usPort);

  /*  连接进入监听状态 */
  netconn_listen(conn);

  /*  循环处理 */
  while(1)
  {
    /*  接受新的连接请求 阻塞函数 */
    net_acp_result = netconn_accept(conn,&newconn);

    rt_kprintf("connect error code: %d\n", net_acp_result);

   if (net_acp_result == 0)
   {
      rt_kprintf("Modbus Tcp server OK, startup rx thread\n");
      
      /* init event */
      rt_event_init(&nw_event, "nw_event", RT_IPC_FLAG_FIFO);

      /* create a new thread */
      thread = rt_thread_create("rx", nw_thread, RT_NULL,
         0x800, 20, 20);
      if (thread != RT_NULL)
         rt_thread_startup(thread);
   }

     /*网络正常（非检测到网络连接断开或者网线拔掉），在此循环  */
    while( !((netconn_err(newconn) != ERR_OK )||((checkethlink()!= ERR_OK))))
    {
      rt_thread_delay(RT_TICK_PER_SECOND);
    }

    /*断开网络 关闭并删除连接 准备接受新的连接*/
    rt_event_send(&nw_event, NW_CLOSED); 
   
    rt_kprintf("Modbus Tcp server closeed! Wait for new connect !\n");
  }
}

/* ----------------------- Begin implementation -----------------------------*/
BOOL
xMBTCPPortInit( USHORT usTCPPort )
{
  BOOL bOkay = FALSE;
  rt_thread_t tid;

  if( usTCPPort == 0 )
  {
      usPort = MB_TCP_DEFAULT_PORT;
  }
  else
  {
      usPort = (USHORT)usTCPPort;
  }

  tid = rt_thread_create("mbtcpsrv", mbtcp_srv_thread, RT_NULL,
          0x800, 25, 5);
  if (tid != RT_NULL) 
  {
    rt_thread_startup(tid);
    bOkay = TRUE;
  }
  return bOkay;    
}

void 
vMBTCPPortClose(  )
{
    
}

void
vMBTCPPortDisable( void )
{
    
}

BOOL
xMBTCPPortGetRequest( UCHAR ** ppucMBTCPFrame, USHORT * usTCPLength )
{
    *ppucMBTCPFrame = &ucTCPRequestFrame[0];
    *usTCPLength = ucTCPRequestLen;
    
    /* Reset the buffer. */
    ucTCPRequestLen = 0;
    return TRUE;
}

BOOL
xMBTCPPortSendResponse( const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
    memcpy( ucTCPResponseFrame , pucMBTCPFrame , usTCPLength);
    ucTCPResponseLen = usTCPLength;
    
    /* Send data via nw_thread */
    rt_event_send(&nw_event, NW_TX);      
    return TRUE;
}

