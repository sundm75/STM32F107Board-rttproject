/*******************************2012-2013, NJUT, Edu.************************** 
FileName: touch_spi.c 
Author:  王晓荣       Version :  2.0       Date: 2014.06.05
Description:    触摸屏驱动  采用SPI总线编写    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      王晓荣    14/05/01     1.0     文件创建   
      Sundm     14/06/06     2.0     添加中断处理   
      Sundm     14/06/30     3.0     进行RTGUI移植修改   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |       TOUCH_Pin             |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |        CS                   |        C8                   |
  *          |        INT                  |        C6                   |
  *          |        MISO                 |        C11                  |
  *          |        MOSI                 |        C12                  |
  *          |        SCLK                 |        C10                  |
  *          +-----------------------------+-----------------------------+
 ******************************************************************************/ 
#include <stdbool.h>
#include "board.h"
#include "touch.h"
#include <rtgui/calibration.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <rtgui/event.h>
#include <rtgui/kbddef.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

//竖屏幕 不需要  _ILI_HORIZONTAL_DIRECTION_
//横屏幕 需要  _ILI_HORIZONTAL_DIRECTION_

#define _ILI_HORIZONTAL_DIRECTION_

#if defined(_ILI_HORIZONTAL_DIRECTION_)
#define X_WIDTH 320
#define Y_WIDTH 240
#else
#define X_WIDTH 240
#define Y_WIDTH 320
#endif
/*
TOUCH INT: PC6
*/
#define IS_TOUCH_UP()     GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)


/*
7  6 - 4  3      2     1-0
s  A2-A0 MODE SER/DFR PD1-PD0
*/
/* bit[1:0] power-down */
#define POWER_MODE0     (0) /* Power-Down Between Conversions. When */
                            /* each conversion is finished, the converter */
                            /* enters a low-power mode. At the start of the */
                            /* next conversion, the device instantly powers up */
                            /* to full power. There is no need for additional */
                            /* delays to ensure full operation, and the very first */
                            /* conversion is valid. The Y? switch is on when in */
                            /* power-down.*/
#define POWER_MODE1     (1) /* Reference is off and ADC is on. */
#define POWER_MODE2     (2) /* Reference is on and ADC is off. */
#define POWER_MODE3     (3) /* Device is always powered. Reference is on and */
                            /* ADC is on. */
/* bit[2] SER/DFR */
#define DIFFERENTIAL    (0<<2)
#define SINGLE_ENDED    (1<<2)
/* bit[3] mode */
#define MODE_12BIT      (0<<3)
#define MODE_8BIT       (1<<3)
/* bit[6:4] differential mode */
#define MEASURE_X       (((1<<2) | (0<<1) | (1<<0))<<4)
#define MEASURE_Y       (((0<<2) | (0<<1) | (1<<0))<<4)
#define MEASURE_Z1      (((0<<2) | (1<<1) | (1<<0))<<4)
#define MEASURE_Z2      (((1<<2) | (0<<1) | (0<<0))<<4)
/* bit[7] start */
#define START           (1<<7)

/* X Y change. */
#define TOUCH_MSR_X     (START | MEASURE_X | MODE_12BIT | DIFFERENTIAL | POWER_MODE0)
#define TOUCH_MSR_Y     (START | MEASURE_Y | MODE_12BIT | DIFFERENTIAL | POWER_MODE0)

#if defined(_ILI_HORIZONTAL_DIRECTION_)
#define MIN_X_DEFAULT   1868
#define MAX_X_DEFAULT   205
#define MIN_Y_DEFAULT   176
#define MAX_Y_DEFAULT   1836
#else
#define MIN_X_DEFAULT   176
#define MAX_X_DEFAULT   1836
#define MIN_Y_DEFAULT   1868
#define MAX_Y_DEFAULT   205
#endif

#define SAMP_CNT 8                              //the adc array size
#define SAMP_CNT_DIV2 4                         //the middle of the adc array
#define SH   10                                 // Valve value


/*宏定义 */
#define TOUCH_SPI_X                 SPI3
#define TOUCH_SPI_RCC               RCC_APB1Periph_SPI3

#define TOUCH_PORT                  GPIOC                   
#define TOUCH_GPIO_RCC              RCC_APB2Periph_GPIOC

#define TOUCH_INT_PIN               GPIO_Pin_6 
#define TOUCH_CS_PIN                GPIO_Pin_8 
#define TOUCH_SCK_PIN               GPIO_Pin_10
#define TOUCH_MISO_PIN              GPIO_Pin_11
#define TOUCH_MOSI_PIN              GPIO_Pin_12     

#define TOUCH_EXTI_IRQn             EXTI9_5_IRQn 

/*创建结构体将需要用到的东西进行打包*/
struct rtgui_touch_device 
{
    struct rt_device parent;                    /* 用于注册设备*/

    rt_uint16_t x, y;                           /* 记录读取到的位置值  */ 

    rt_bool_t calibrating;                      /* 触摸校准标志 */ 
    rt_touch_calibration_func_t calibration_func;/* 触摸函数 函数指针 */       

    rt_uint16_t min_x, max_x;                   /* 校准后 X 方向最小 最大值 */ 
    rt_uint16_t min_y, max_y;                   /* 校准后 Y 方向最小 最大值 */

    struct rt_spi_device * spi_device;          /* SPI 设备 用于通信 */ 
    struct rt_event event;                       /* 事件同步，用于“笔中断” */ 
};
static struct rtgui_touch_device *touch = RT_NULL;

rt_inline void touch_int_cmd(FunctionalState NewState);

static void rtgui_touch_calculate(void)
{
    if (touch != RT_NULL)
    {
        /* read touch */
        {
            uint8_t i, j, k, min;
	        uint16_t temp;
            rt_uint16_t tmpxy[2][SAMP_CNT];
            uint8_t send_buffer[1];
            uint8_t recv_buffer[2];
            for(i=0; i<SAMP_CNT; i++)
            {
                send_buffer[0] = TOUCH_MSR_X;
                rt_spi_send_then_recv(touch->spi_device,
                                      send_buffer,
                                      1,
                                      recv_buffer,
                                      2);
#if defined(_ILI_HORIZONTAL_DIRECTION_)
                tmpxy[1][i]  = (recv_buffer[0]<<8)|recv_buffer[1] ;
                tmpxy[1][i] >>= 4;
#else
                tmpxy[0][i]  = (recv_buffer[0]<<8)|recv_buffer[1] ;
                tmpxy[0][i] >>=4;

#endif
                send_buffer[0] = TOUCH_MSR_Y;
                rt_spi_send_then_recv(touch->spi_device,
                                      send_buffer,
                                      1,
                                      recv_buffer,
                                      2);
#if defined(_ILI_HORIZONTAL_DIRECTION_)
                tmpxy[0][i]  = (recv_buffer[0]<<8)|recv_buffer[1] ;
                tmpxy[0][i] >>= 4;
#else
                tmpxy[1][i]  = (recv_buffer[0]<<8)|recv_buffer[1] ;
                tmpxy[1][i] >>= 4;
#endif
            }
            /*再次打开触摸中断*/
            send_buffer[0] = 1 << 7;
            rt_spi_send(touch->spi_device, send_buffer, 1);

            /* calculate average */
			{
				rt_uint32_t total_x = 0;
				rt_uint32_t total_y = 0;
				for(k=0; k<2; k++)
				{ 
					// sorting the ADC value
					for(i=0; i<SAMP_CNT-1; i++)
					{
						min=i;
						for (j=i+1; j<SAMP_CNT; j++)
						{
							if (tmpxy[k][min] > tmpxy[k][j]) 
								min=j;
						}
						temp = tmpxy[k][i];
						tmpxy[k][i] = tmpxy[k][min];
						tmpxy[k][min] = temp;
				    }
				    //check value for Valve value
					if((tmpxy[k][SAMP_CNT_DIV2+1]-tmpxy[k][SAMP_CNT_DIV2-2]) > SH)
					{
						return;
					}
				}
				total_x=tmpxy[0][SAMP_CNT_DIV2-2]+tmpxy[0][SAMP_CNT_DIV2-1]+tmpxy[0][SAMP_CNT_DIV2]+tmpxy[0][SAMP_CNT_DIV2+1];
				total_y=tmpxy[1][SAMP_CNT_DIV2-2]+tmpxy[1][SAMP_CNT_DIV2-1]+tmpxy[1][SAMP_CNT_DIV2]+tmpxy[1][SAMP_CNT_DIV2+1];
				//calculate average value
				touch->x=total_x>>2;
				touch->y=total_y>>2;
                rt_kprintf("touch->x:%d touch->y:%d\r\n", touch->x, touch->y);
           } /* calculate average */
        } /* read touch */

        /* if it's not in calibration status  */
        /*触摸值缩放*/
        if (touch->calibrating != RT_TRUE)
        {
            if (touch->max_x > touch->min_x)
            {
                touch->x = (touch->x - touch->min_x) * X_WIDTH/(touch->max_x - touch->min_x);
            }
            else
            {
                touch->x = (touch->min_x - touch->x) * X_WIDTH/(touch->min_x - touch->max_x);
            }

            if (touch->max_y > touch->min_y)
            {
                touch->y = (touch->y - touch->min_y) * Y_WIDTH /(touch->max_y - touch->min_y);
            }
            else
            {
                touch->y = (touch->min_y - touch->y) * Y_WIDTH /(touch->min_y - touch->max_y);
            }
        }
    }
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

rt_inline void touch_int_cmd(FunctionalState NewState)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure  EXTI  */
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;

    EXTI_InitStructure.EXTI_LineCmd = NewState;

    EXTI_ClearITPendingBit(EXTI_Line6);
    EXTI_Init(&EXTI_InitStructure);
}


static void EXTI_Configuration(void)
{
    /* PC6 touch INT */
    {
      GPIO_InitTypeDef GPIO_InitStructure;  
      /* Enable GPIOB, GPIOC and AFIO clock */
      RCC_APB2PeriphClockCmd(TOUCH_GPIO_RCC , ENABLE);  
      
      /* INT pins configuration */
      GPIO_InitStructure.GPIO_Pin = TOUCH_INT_PIN;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   /*上拉输入，默认是1*/
      GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);
    }

     GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6); 

    /* Configure  EXTI  */
    touch_int_cmd(ENABLE);
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void) /* TouchScreen */
{
  if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
    /* 触摸屏按下后操作 */  
    /* disable interrupt */
    touch_int_cmd(DISABLE);

    rt_event_send(&touch->event, 1);

    EXTI_ClearITPendingBit(EXTI_Line6);
  }
}

/* RT-Thread Device Interface */
static rt_err_t rtgui_touch_init (rt_device_t dev)
{
    uint8_t send;
    struct rtgui_touch_device * touch_device = (struct rtgui_touch_device *)dev;

    NVIC_Configuration();
    EXTI_Configuration();

    send = START | DIFFERENTIAL | POWER_MODE0;
    rt_spi_send(touch_device->spi_device, &send, 1); /* enable touch interrupt */

    return RT_EOK;
}

static rt_err_t rtgui_touch_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RT_TOUCH_CALIBRATION:
        touch->calibrating = RT_TRUE;
        touch->calibration_func = (rt_touch_calibration_func_t)args;
        break;

    case RT_TOUCH_NORMAL:
        touch->calibrating = RT_FALSE;
        break;

    case RT_TOUCH_CALIBRATION_DATA:
    {
        struct calibration_data* data;
        data = (struct calibration_data*) args;

        //update
        touch->min_x = data->min_x;
        touch->max_x = data->max_x;
        touch->min_y = data->min_y;
        touch->max_y = data->max_y;
    }
    break;
    }

    return RT_EOK;
}
/*暂空*/
void _set_mouse_position(uint32_t X, uint32_t Y)
{}
static void touch_thread_entry(void *parameter)
{
    rt_bool_t touch_down = RT_FALSE;
    rt_uint32_t event_value;
    struct rtgui_event_mouse emouse;
    static struct _touch_previous
    {
        rt_uint32_t x;
        rt_uint32_t y;
    } touch_previous;

    RTGUI_EVENT_MOUSE_BUTTON_INIT(&emouse);
    emouse.wid = RT_NULL;

    while(1)
    {
        /* 接收到触摸中断事件 */
        if(rt_event_recv(&touch->event,
                         1,
                         RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                         RT_WAITING_FOREVER,
                         &event_value)
                == RT_EOK)
        {
            while(1)
            {
                if (IS_TOUCH_UP())
                {
                    /* 触摸笔抬起 */
                    /* touch up */
                    emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_UP);

                    /* use old value */
                    emouse.x = touch->x;
                    emouse.y = touch->y;

                    if(touch_down != RT_TRUE) 
                    {                    
                        touch_int_cmd(ENABLE);
                        break;
                    }    

                    if ((touch->calibrating == RT_TRUE) && (touch->calibration_func != RT_NULL))
                    {
                        /* 触摸校准处理 */
                        /* callback function */
                        touch->calibration_func(emouse.x, emouse.y);
											
                    }
                    else
                    {
                        /* 向ui发送触摸坐标 */
                        rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
                    }
                    rt_kprintf("touch up: (%d, %d)\n", emouse.x, emouse.y);

                    /* clean */
                    touch_previous.x = touch_previous.y = 0;
                    touch_down = RT_FALSE;

                    touch_int_cmd(ENABLE);
                    break;
                } /* touch up */
                else /* touch down or move */
                {
                    if(touch_down == RT_FALSE)
                    {
                        rt_thread_delay(RT_TICK_PER_SECOND / 10);
                    }
                    else
                    {
                        rt_thread_delay(5);
                    }

                    if(IS_TOUCH_UP()) continue;

                    /* calculation */
                    rtgui_touch_calculate();
                    
                    /* send mouse event */
                    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
                    emouse.parent.sender = RT_NULL;

                    emouse.x = touch->x;
                    emouse.y = touch->y;
                    _set_mouse_position(emouse.x, emouse.y);
                    /* 光标跟随 */
                    /* init mouse button */
                    emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN);

                    /* send event to server */
                    if (touch->calibrating != RT_TRUE)
                    {
#define previous_keep      8
                        /* filter. */
                        if((touch_previous.x > touch->x + previous_keep)
                            || (touch_previous.x < touch->x - previous_keep)
                            || (touch_previous.y > touch->y + previous_keep)
                            || (touch_previous.y < touch->y - previous_keep))
                        {
                            touch_previous.x = touch->x;
                            touch_previous.y = touch->y;
                            /* 向ui发送触摸坐标 */
                            rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
                            if(touch_down == RT_FALSE)
                            {
                                touch_down = RT_TRUE;
                                rt_kprintf("touch down: (%d, %d)\n", emouse.x, emouse.y);
                            }
                            else
                            {
                                rt_kprintf("touch motion: (%d, %d)\n", emouse.x, emouse.y);
                            }
                        }
                    }
                    else
                    {
                        touch_down = RT_TRUE;
                    }
                } /* touch down or move */
            } /* read touch */
        } /* event recv */
    } /* thread while(1) */
}


rt_err_t rtgui_touch_hw_init(const char * spi_device_name)
{
    struct rt_spi_device * spi_device;
    struct rt_thread * touch_thread;

    spi_device = (struct rt_spi_device *)rt_device_find(spi_device_name);
    if(spi_device == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\r\n", spi_device_name);
        return -RT_ENOSYS;
    }

    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB; /* SPI Compatible Modes 0 */
        cfg.max_hz = 500 * 1000; /* 500K */
        rt_spi_configure(spi_device, &cfg);
    }

    touch = (struct rtgui_touch_device*)rt_malloc (sizeof(struct rtgui_touch_device));
    if (touch == RT_NULL) return RT_ENOMEM; /* no memory yet */

    /* clear device structure */
    rt_memset(&(touch->parent), 0, sizeof(struct rt_device));

    rt_event_init(&touch->event, "touch", RT_IPC_FLAG_FIFO);

    touch->spi_device = spi_device;
    touch->calibrating = false;

    touch->min_x = MIN_X_DEFAULT;
    touch->max_x = MAX_X_DEFAULT;
    touch->min_y = MIN_Y_DEFAULT;
    touch->max_y = MAX_Y_DEFAULT;

    /* init device structure */
    touch->parent.type = RT_Device_Class_Unknown;
    touch->parent.init = rtgui_touch_init;
    touch->parent.control = rtgui_touch_control;
    touch->parent.user_data = RT_NULL;

    /* register touch device to RT-Thread */
    rt_device_register(&(touch->parent), "touch", RT_DEVICE_FLAG_RDWR);

    touch_thread = rt_thread_create("touch",
                                    touch_thread_entry, RT_NULL,
                                    1024, RTGUI_SVR_THREAD_PRIORITY-1, 1);
    if (touch_thread != RT_NULL) rt_thread_startup(touch_thread);

    return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void touch_t( rt_uint16_t x , rt_uint16_t y )
{
    struct rtgui_event_mouse emouse ;
    emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
    emouse.parent.sender = RT_NULL;

    emouse.x = x ;
    emouse.y = y ;
    /* init mouse button */
    emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN );
    rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));

    rt_thread_delay(2) ;
    emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_UP );
    rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
}

FINSH_FUNCTION_EXPORT(touch_t, x & y ) ;
#endif