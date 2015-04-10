/*******************************2012-2013, NJUT, Edu.************************** 
FileName: touch_spi.c 
Author:  王晓荣       Version :  2.0       Date: 2014.06.05
Description:    触摸屏驱动  采用SPI总线编写    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      王晓荣    14/05/01     1.0     文件创建   
      Sundm     14/06/06     2.0     添加中断处理   
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
#include "touch.h"
#include "usart.h"
#include "stm32f10x.h"
#include "rtthread.h"

/*静态函数定义 */
static uint8_t  spi_white_read_byte(uint8_t byte);
static uint16_t touch_read_ad(uint8_t cmd);
static uint16_t touch_read_xy(uint8_t cmd);

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

#define CMD_READ_X                  0X90 
#define CMD_READ_Y                  0XD0 

/*******************************************************************************
*  Function Name  : 　TP_INT_config （内部函数）
*  Description    : 触摸屏INT管脚配置
*  Input          : None
*  Output         : None
*  Return         : None
*******************************************************************************/
static void TOUCH_INT_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  
  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(TOUCH_GPIO_RCC , ENABLE);  
  
  /* INT pins configuration */
  GPIO_InitStructure.GPIO_Pin = TOUCH_INT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   /*上拉输入，默认是1*/
  GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6); 

  /* Configure Button EXTI line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
  
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable the EXTI5 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TOUCH_EXTI_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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
   rt_kprintf("x:%d , y: %d\n\r",touch_read_x(),touch_read_y()); 
    /* Clear the EXTI Line 6 */  
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
}

/*******************************************************************************
* Function Name  : touch_config
* Description    : 触摸屏配置，端口初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void touch_config(void)
{ 
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB1PeriphClockCmd(TOUCH_SPI_RCC, ENABLE);			
  RCC_APB2PeriphClockCmd(TOUCH_GPIO_RCC | RCC_APB2Periph_AFIO, ENABLE);

  GPIO_PinRemapConfig(GPIO_Remap_SPI3,ENABLE);

  GPIO_InitStructure.GPIO_Pin   = TOUCH_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;   
  GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin   = TOUCH_MISO_PIN;
  GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin   = TOUCH_MOSI_PIN;
  GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);

  GPIO_SetBits(TOUCH_PORT, TOUCH_CS_PIN);
  GPIO_InitStructure.GPIO_Pin   = TOUCH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 
  GPIO_Init(TOUCH_PORT, &GPIO_InitStructure);    

  SPI_Cmd(TOUCH_SPI_X, DISABLE);                                            /**< 先禁能,才能改变MODE */ 
  SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;/**< 两线全双工 */
  SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;                /**< 主机 */                                
  SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;                /**< 8位 */  
  SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;                  /**< 空闲为高电平 */
  SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;                 /**< 上升沿捕获（空闲为高电平时，第1个边沿为下降沿） */      
  SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;                   /**< NSS信号由软件管理 */ 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;       /**< 72M / 16 */               
  SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;               /**< 数据传输从MSB位开始 */
  SPI_InitStructure.SPI_CRCPolynomial     = 7;                              /**< CRC7 */  
  SPI_Init(TOUCH_SPI_X, &SPI_InitStructure);
  SPI_Cmd(TOUCH_SPI_X, ENABLE); 
  
  /*打开触摸屏中断后，按下屏幕将读数通过串口打印*/
  //TOUCH_INT_config();
}

/*******************************************************************************
* Function Name  : touch_read_x
* Description    : 读取X方向AD采样值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t touch_read_x(void)
{
  return touch_read_xy(CMD_READ_X);
} 

/*******************************************************************************
* Function Name  : touch_read_y
* Description    : 读取Y方向AD采样值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t touch_read_y(void)
{
  return touch_read_xy(CMD_READ_Y);
}

/*******************************************************************************
* Function Name  : touch_read_xy(内部函数)
* Description    : 读取X或者Y方向AD采样值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static uint16_t touch_read_xy(uint8_t cmd)
{
  uint16_t i, j;
  uint16_t buf[4];
  uint16_t temp;
  
  /** 采4个点 */   
  for(i = 0; i < 4; i++)
  {
      buf[i]=touch_read_ad(cmd);
  }	    
  /** 升序排列 */    
  for(i = 0; i < 3; i++)
  {
    for(j = i + 1; j < 4; j++)
    {
      if(buf[i] > buf[j])
      {
        temp   = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }	
  /** 如果偏差太大，舍弃 */
  if((buf[2] - buf[1]) > 5)   
  {
    return 0;
  }   
  /** 取中间2个数的平均值 */
  else  
  {
    return ((buf[2] + buf[1]) / 2);
  } 
} 

/*******************************************************************************
* Function Name  : spi_white_read_byte(内部函数)
* Description    : 利用SPI读写1个字节
* Input          : 写入的1个字节
* Output         : None
* Return         : 读出的1个字节
*******************************************************************************/
static uint8_t spi_white_read_byte(uint8_t byte)
{
  /** 等待写完成 */
  while(SPI_I2S_GetFlagStatus(TOUCH_SPI_X, SPI_I2S_FLAG_TXE) == RESET);
  /** 写1个字节到MOSI, 同时通过MISO接收1个字节 */
  SPI_I2S_SendData(TOUCH_SPI_X, byte);
  /** 等待MISO接收完成 */
  while(SPI_I2S_GetFlagStatus(TOUCH_SPI_X, SPI_I2S_FLAG_RXNE) == RESET);
  /** 返回MISO接收值 */
  return SPI_I2S_ReceiveData(TOUCH_SPI_X);  
}

/*******************************************************************************
* Function Name  : touch_read_ad(内部函数)
* Description    : 读ad值
* Input          : cmd 读取命令
* Output         : None
* Return         : None
*******************************************************************************/
static uint16_t touch_read_ad(uint8_t cmd)	  
{ 
  uint16_t ad;

  GPIO_ResetBits(TOUCH_PORT, TOUCH_CS_PIN);
  spi_white_read_byte(cmd);
  ad = spi_white_read_byte(0);
  ad <<= 8;
  ad |= spi_white_read_byte(0);
  ad >>= 3;
  GPIO_SetBits(TOUCH_PORT, TOUCH_CS_PIN);
  return ad;     
}

