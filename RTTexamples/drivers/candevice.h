/**********************2012-2014, NJUT, Edu.*********************************** 
File name:      candevice.h
Author:  sundm     Version:  1.0      Date: 2014.10.13 
Description:    can设备接口驱动
Version:         1.0
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/12/10    1.0       文件创建  
*******************************************************************************/ 

#ifndef _CANDEVICE_H_
#define _CANDEVICE_H_

#include <rthw.h>
#include <rtthread.h>

/* STM32F10x library definitions */
#include <stm32f10x.h>
#include "rtthread.h"


#define CAN_RX_LEN		64 

#define CAN_dominant        0
#define CAN_recessive       1

#define CAN_IDE_standard    CAN_dominant
#define CAN_IDE_extended    CAN_recessive

#define CAN_RTR_data        CAN_dominant
#define CAN_RTR_remote      CAN_recessive

typedef struct
{
    rt_uint32_t ID;      /* ID[28:18] */
    rt_uint32_t EXT_ID;  /* ID[17:0] */
    rt_uint8_t  IDE;     /* CAN_dominant(0) - STANDARD, CAN_recessive(1)- EXTENDED IDENTIFIER */
    rt_uint8_t  RTR;     /* CAN_dominant(0) - DATA FRAME, CAN_recessive(1) - REMOTE FRAME */
    rt_uint8_t  DLC;     /* Length of data field in bytes 0~8byte */
    rt_uint8_t  DATA[8]; /* Data field */
} CAN_msg_t;


/* data node for Tx Mode */
struct stm32_can_data_node
{
	CAN_msg_t *data_ptr;
	rt_size_t  data_size;
	struct stm32_can_data_node *next, *prev;
};

struct stm32_can_tx
{

	/* data list head and tail */
	struct stm32_can_data_node *list_head, *list_tail;

};

struct stm32_can_int_rx
{
	CAN_msg_t  rx_message_buffer[CAN_RX_LEN];
	rt_uint32_t read_index, save_index;
};

struct stm32_can_device
{
	CAN_TypeDef* can_device;

	/* rx structure */
	struct stm32_can_int_rx* int_rx;

};

rt_err_t rt_hw_can_register(rt_device_t device, const char* name, rt_uint32_t flag, struct stm32_can_device *can);
void rt_hw_can_isr(rt_device_t device);

#endif
