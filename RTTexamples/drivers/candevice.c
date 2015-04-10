/**********************2012-2014, NJUT, Edu.*********************************** 
File name:      candevice.c
Author:  sundm     Version:  1.0      Date: 2014.10.13 
Description:    can设备接口驱动
Version:         1.0
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/12/10    1.0       文件创建  
*******************************************************************************/ 

#include "candevice.h"

/**
 * @addtogroup STM32
 */
/*STM32 data convert*/
static void dev2stm_can(CAN_msg_t* devptr, CanTxMsg* stmptr )
{
  rt_uint8_t i;
  for(i=0;i<8;i++)
  {
    stmptr->Data[i] = devptr->DATA[i];
  }
  stmptr->DLC = devptr->DLC;
  stmptr->ExtId = devptr->EXT_ID;
  stmptr->IDE = devptr->IDE;
  stmptr->RTR = devptr->RTR;
  stmptr->StdId = devptr->ID;
}

static void stm2dev_can(CanRxMsg* stmptr, CAN_msg_t* devptr)
{
  rt_uint8_t i;
  for(i=0;i<8;i++)
  {
    devptr->DATA[i] = stmptr->Data[i];
  }
  devptr->DLC = stmptr->DLC ;       
  devptr->EXT_ID = stmptr->ExtId ;      
  devptr->IDE = stmptr->IDE ;        
  devptr->RTR = stmptr->RTR ;         
  devptr->ID = stmptr->StdId ;      
}

/* RT-Thread Device Interface */
static rt_err_t rt_can_init (rt_device_t dev)
{
	struct stm32_can_device* can_dev = (struct stm32_can_device*) dev->user_data;

	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
	{
		if (dev->flag & RT_DEVICE_FLAG_INT_RX)
		{
			rt_memset(can_dev->int_rx->rx_message_buffer, 0,
				sizeof(can_dev->int_rx->rx_message_buffer));
			can_dev->int_rx->read_index = 0;
			can_dev->int_rx->save_index = 0;
		}

		/* Enable CAN */
                CAN_ITConfig(can_dev->can_device, CAN_IT_FMP0, ENABLE);
                
                /* Realease CAN FIFO*/
                CAN_FIFORelease(can_dev->can_device, 0);
                CAN_FIFORelease(can_dev->can_device, 1);
               
		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}

	return RT_EOK;
}

static rt_err_t rt_can_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_can_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_can_read (rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	CAN_msg_t* ptr;
	rt_err_t err_code;
        struct stm32_can_device* can_dev = (struct stm32_can_device*) dev->user_data;
        
	ptr = buffer;
	err_code = RT_EOK;

	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* interrupt mode Rx */
		while (size)
		{
			rt_base_t level;

			/* disable interrupt */
			level = rt_hw_interrupt_disable();

			if (can_dev->int_rx->read_index != can_dev->int_rx->save_index)
			{
				/* read a character */
				*ptr++ = can_dev->int_rx->rx_message_buffer[can_dev->int_rx->read_index];
				size--;
                                  
				/* move to next position */
				can_dev->int_rx->read_index ++;
				if (can_dev->int_rx->read_index >= CAN_RX_LEN)
					can_dev->int_rx->read_index = 0;
			}
			else
			{
				/* set error code */
				err_code = -RT_EEMPTY;

				/* enable interrupt */
				rt_hw_interrupt_enable(level);
				break;
			}

			/* enable interrupt */
			rt_hw_interrupt_enable(level);
		}
	}
	/* set error code */
	rt_set_errno(err_code);
	return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
}

static rt_size_t rt_can_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	CAN_msg_t* ptr;
        CanTxMsg stm_ptr;
	rt_err_t err_code;
	struct stm32_can_device* can_dev;
        

	err_code = RT_EOK;
	ptr = (CAN_msg_t*)buffer;
	can_dev = (struct stm32_can_device*)dev->user_data;

	if (dev->flag & RT_DEVICE_FLAG_INT_TX)
	{
		/* interrupt mode Tx, does not support */
		RT_ASSERT(0);
	}
	else if (dev->flag & RT_DEVICE_FLAG_DMA_TX)
	{
		
	}
	else
	{
		/* polling mode */
		if (dev->flag & RT_DEVICE_FLAG_STREAM)
		{
			/* stream mode */
			while (size)
			{
				if (ptr != RT_NULL)
				{
                                        dev2stm_can(ptr, &stm_ptr);
                                  	CAN_Transmit(can_dev->can_device, &stm_ptr);
				}
				++ptr; --size;
			}
		}
	}

	/* set error code */
	rt_set_errno(err_code);

	return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
}

static rt_err_t rt_can_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct stm32_can_device* can_dev;

	RT_ASSERT(dev != RT_NULL);

	can_dev = (struct stm32_can_device*)dev->user_data;
	switch (cmd)
	{
	case RT_DEVICE_CTRL_SUSPEND:
		/* suspend device */
		dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
                CAN_ITConfig(can_dev->can_device, CAN_IT_FMP0, DISABLE);
		break;

	case RT_DEVICE_CTRL_RESUME:
		/* resume device */
		dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
		CAN_ITConfig(can_dev->can_device, CAN_IT_FMP0, ENABLE);
                break;
	}

	return RT_EOK;
}

/*
 * serial register for STM32
 * support STM32F103VB and STM32F103ZE
 */
rt_err_t rt_hw_can_register(rt_device_t device, const char* name, rt_uint32_t flag, struct stm32_can_device *can)
{
	RT_ASSERT(device != RT_NULL);

	device->type 		= RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init 		= rt_can_init;
	device->open		= rt_can_open;
	device->close		= rt_can_close;
	device->read 		= rt_can_read;
	device->write 		= rt_can_write;
	device->control 	= rt_can_control;
	device->user_data	= can;

	/* register a character device */
	return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

/* ISR for can interrupt */
void rt_hw_can_isr(rt_device_t device)
{
	struct stm32_can_device* can_dev = (struct stm32_can_device*)device->user_data;
	CanRxMsg  RxMessage;
	CAN_msg_t  rx_canmessage;
	rt_base_t level;


	{
		/* interrupt mode receive */
		RT_ASSERT(device->flag & RT_DEVICE_FLAG_INT_RX);

		/* save on rx buffer */
		
		/* disable interrupt */
		level = rt_hw_interrupt_disable();

		/* save rx message */  
		CAN_Receive(can_dev->can_device, CAN_FIFO0, &RxMessage);
		stm2dev_can(&RxMessage, &rx_canmessage);
		can_dev->int_rx->rx_message_buffer[can_dev->int_rx->save_index] = rx_canmessage;
		can_dev->int_rx->save_index ++;
		if (can_dev->int_rx->save_index >= CAN_RX_LEN)
			can_dev->int_rx->save_index = 0;

		/* if the next position is read index, discard this 'read char' */
		if (can_dev->int_rx->save_index == can_dev->int_rx->read_index)
		{
			can_dev->int_rx->read_index ++;
			if (can_dev->int_rx->read_index >= CAN_RX_LEN)
				can_dev->int_rx->read_index = 0;
		}

		/* enable interrupt */
		rt_hw_interrupt_enable(level);
		

		/* invoke callback */
		if (device->rx_indicate != RT_NULL)
		{
			rt_size_t rx_length;

			/* get rx length */
			rx_length = can_dev->int_rx->read_index > can_dev->int_rx->save_index ?
				CAN_RX_LEN - can_dev->int_rx->read_index + can_dev->int_rx->save_index :
				can_dev->int_rx->save_index - can_dev->int_rx->read_index;

			device->rx_indicate(device, rx_length);
		}
	}
}




