/******************************2014-2015, NJTECH, Edu.************************** 
FileName: NRF24L01.c 
Author:  孙冬梅       Version :  1.0        Date: 2015.05.30
Description:    NRF24L01驱动   使用SPI1   
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    15/05/30     1.0     文件创建   
  *          STM32Board Key Pin assignment
  *          =============================
 *          +----------------------------------------------------------+
 *          |     RF devices      SPIBUS　      CE       CSN     IRQ   |
 *          +------------------+---------+-----------------------------+
 *          |      RF1         |   SPI1  |    D2    |     A4   |  D7   |
 *          +------------------+---------+-----------------------------+
*******************************************************************************/ 
#include "NRF24l01.h"

//NRF24L01寄存器操作命令
#define SPI_READ_REG    0x00  //读配置寄存器,低5位为寄存器地址
#define SPI_WRITE_REG   0x20  //写配置寄存器,低5位为寄存器地址
#define RD_RX_PLOAD     0x61  //读RX有效数据,1~32字节
#define WR_TX_PLOAD     0xA0  //写TX有效数据,1~32字节
#define FLUSH_TX        0xE1  //清除TX FIFO寄存器.发射模式下用
#define FLUSH_RX        0xE2  //清除RX FIFO寄存器.接收模式下用
#define REUSE_TX_PL     0xE3  //重新使用上一包数据,CE为高,数据包被不断发送.
#define NOP             0xFF  //空操作,可以用来读状态寄存器	 
//SPI(NRF24L01)寄存器地址
#define CONFIG          0x00  //配置寄存器地址;bit0:1接收模式,0发射模式;bit1:电选择;bit2:CRC模式;bit3:CRC使能;
                              //bit4:中断MAX_RT(达到最大重发次数中断)使能;bit5:中断TX_DS使能;bit6:中断RX_DR使能
#define EN_AA           0x01  //使能自动应答功能  bit0~5,对应通道0~5
#define EN_RXADDR       0x02  //接收地址允许,bit0~5,对应通道0~5
#define SETUP_AW        0x03  //设置地址宽度(所有数据通道):bit1,0:00,3字节;01,4字节;02,5字节;
#define SETUP_RETR      0x04  //建立自动重发;bit3:0,自动重发计数器;bit7:4,自动重发延时 250*x+86us
#define RF_CH           0x05  //RF通道,bit6:0,工作通道频率;
#define RF_SETUP        0x06  //RF寄存器;bit3:传输速率(0:1Mbps,1:2Mbps);bit2:1,发射功率;bit0:低噪声放大器增益
#define STATUS          0x07  //状态寄存器;bit0:TX FIFO满标志;bit3:1,接收数据通道号(最大:6);bit4,达到最多次重发
                              //bit5:数据发送完成中断;bit6:接收数据中断;
#define MAX_TX  	    0x10  //达到最大发送次数中断
#define TX_OK       	0x20  //TX发送完成中断
#define RX_OK   	    0x40  //接收到数据中断

#define OBSERVE_TX      0x08  //发送检测寄存器,bit7:4,数据包丢失计数器;bit3:0,重发计数器
#define CD              0x09  //载波检测寄存器,bit0,载波检测;
#define RX_ADDR_P0      0x0A  //数据通道0接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P1      0x0B  //数据通道1接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P2      0x0C  //数据通道2接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P3      0x0D  //数据通道3接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P4      0x0E  //数据通道4接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P5      0x0F  //数据通道5接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define TX_ADDR         0x10  //发送地址(低字节在前),ShockBurstTM模式下,RX_ADDR_P0与此地址相等
#define RX_PW_P0        0x11  //接收数据通道0有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P1        0x12  //接收数据通道1有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P2        0x13  //接收数据通道2有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P3        0x14  //接收数据通道3有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P4        0x15  //接收数据通道4有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P5        0x16  //接收数据通道5有效数据宽度(1~32字节),设置为0则非法
#define FIFO_STATUS     0x17  //FIFO状态寄存器;bit0,RX FIFO寄存器空标志;bit1,RX FIFO满标志;bit2,3,保留
                              //bit4,TX FIFO空标志;bit5,TX FIFO满标志;bit6,1,循环发送上一数据包.0,不循环;
/**********************************************************************************************************/
//NRF24L01控制操作
#define NRF24L01_CE_PIN     GPIO_Pin_4
#define GPIO_NRF24L01_CE  GPIOC
#define RCC_NRF24L01_CE  RCC_APB2Periph_GPIOC

//NRF24L01 SPI接口CS信号
#define NRF24L01_CSN_PIN      GPIO_Pin_4
#define GPIO_NRF24L01_CSN  GPIOA
#define RCC_NRF24L01_CSN  RCC_APB2Periph_GPIOA

#define NRF24L01_IRQ_PIN      GPIO_Pin_14
#define GPIO_NRF24L01_IRQ  GPIOB
#define RCC_NRF24L01_IRQ  RCC_APB2Periph_GPIOB
//NRF2401片选信号
#define Clr_NRF24L01_CE      {GPIO_ResetBits(GPIO_NRF24L01_CE,NRF24L01_CE_PIN);}
#define Set_NRF24L01_CE      {GPIO_SetBits(GPIO_NRF24L01_CE,NRF24L01_CE_PIN);}

//SPI片选信号	
#define Clr_NRF24L01_CSN     {GPIO_ResetBits(GPIO_NRF24L01_CSN,NRF24L01_CSN_PIN);}
#define Set_NRF24L01_CSN     {GPIO_SetBits(GPIO_NRF24L01_CSN,NRF24L01_CSN_PIN);}
    
//NRF2401_IRQ数据输入
#define Clr_NRF24L01_IRQ     {GPIO_ResetBits(GPIO_NRF24L01_IRQ, NRF24L01_IRQ_PIN);} 
#define Set_NRF24L01_IRQ     {GPIO_SetBits(GPIO_NRF24L01_IRQ, NRF24L01_IRQ_PIN);}

#define READ_NRF24L01_IRQ       (GPIO_ReadInputDataBit(GPIO_NRF24L01_IRQ,NRF24L01_IRQ_PIN))

//NRF24L01发送接收数据宽度定义
#define TX_ADR_WIDTH    5                               //5字节的地址宽度
#define RX_ADR_WIDTH    5                               //5字节的地址宽度
#define TX_PLOAD_WIDTH  32                              //20字节的用户数据宽度
#define RX_PLOAD_WIDTH  32                              //20字节的用户数据宽度

const rt_uint8_t static  TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const rt_uint8_t static  RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址	

/*******************************************************************************
* Function Name  : NRF24L01_Write_Reg
* Description    : 通过SPI写寄存器
* Input          : regaddr:要写的寄存器 data:要写的数据
* Output         : None
* Return         : status ,状态值
*******************************************************************************/
static rt_uint8_t NRF24L01_Write_Reg(rt_uint8_t regaddr,rt_uint8_t data)
{
  rt_uint8_t status;	
  Clr_NRF24L01_CSN;                    //使能SPI传输
  status =SPI1_ReadWriteByte(regaddr); //发送寄存器号 
  SPI1_ReadWriteByte(data);            //写入寄存器的值
  Set_NRF24L01_CSN;                    //禁止SPI传输	   
  return(status);       		//返回状态值
}

/*******************************************************************************
* Function Name  : NRF24L01_Read_Reg
* Description    : 通过SPI读寄存器
* Input          : regaddr:要读的寄存器
* Output         : None
* Return         : reg_val ,读出的数据
*******************************************************************************/
static rt_uint8_t NRF24L01_Read_Reg(rt_uint8_t regaddr)
{
  rt_uint8_t reg_val;	    
  Clr_NRF24L01_CSN;                //使能SPI传输		
  SPI1_ReadWriteByte(regaddr);     //发送寄存器号
  reg_val=SPI1_ReadWriteByte(0XFF);//读取寄存器内容
  Set_NRF24L01_CSN;                //禁止SPI传输		    
  return(reg_val);                 //返回状态值
}

/*******************************************************************************
* Function Name  : NRF24L01_Read_Buf
* Description    : 在指定位置读出指定长度的数据
* Input          : regaddr:要读的寄存器  datalen:指定长度
* Output         : *pBuf:读出数据指针
* Return         : status ,读到的状态值
*******************************************************************************/
static rt_uint8_t NRF24L01_Read_Buf(rt_uint8_t regaddr,rt_uint8_t *pBuf,rt_uint8_t datalen)
{
  rt_uint8_t status,rt_uint8_t_ctr;	       
  Clr_NRF24L01_CSN;                     //使能SPI传输
  status=SPI1_ReadWriteByte(regaddr);   //发送寄存器值(位置),并读取状态值   	   
  for(rt_uint8_t_ctr=0;rt_uint8_t_ctr<datalen;rt_uint8_t_ctr++)pBuf[rt_uint8_t_ctr]=SPI1_ReadWriteByte(0XFF);//读出数据
  Set_NRF24L01_CSN;                     //关闭SPI传输
  return status;                        //返回读到的状态值
}

/*******************************************************************************
* Function Name  : NRF24L01_Write_Buf
* Description    : 在指定位置写指定长度的数据
* Input          : regaddr:要写的寄存器  datalen:指定长度
* Output         : *pBuf:要写入的数据指针
* Return         : status ,读到的状态值
*******************************************************************************/
static rt_uint8_t NRF24L01_Write_Buf(rt_uint8_t regaddr, rt_uint8_t *pBuf, rt_uint8_t datalen)
{
  rt_uint8_t status,rt_uint8_t_ctr;	    
  Clr_NRF24L01_CSN;                                    //使能SPI传输
  status = SPI1_ReadWriteByte(regaddr);                //发送寄存器值(位置),并读取状态值
  for(rt_uint8_t_ctr=0; rt_uint8_t_ctr<datalen; rt_uint8_t_ctr++)SPI1_ReadWriteByte(*pBuf++); //写入数据	 
  Set_NRF24L01_CSN;                                    //关闭SPI传输
  return status;                                       //返回读到的状态值
}				   

/*******************************************************************************
* Function Name  : NRF24L01_TxPacket
* Description    : 启动NRF24L01发送一次数据
* Input          : txbuf:待发送数据首地址
* Output         : None
* Return         : 发送完成状况
*******************************************************************************/
static rt_err_t NRF24L01_TxPacket(rt_uint8_t *txbuf)
{
  rt_uint8_t state;   
  Clr_NRF24L01_CE;
  NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
  Set_NRF24L01_CE;                                     //启动发送	   
  while(READ_NRF24L01_IRQ!=0);                              //等待发送完成
  state=NRF24L01_Read_Reg(STATUS);                     //读取状态寄存器的值	   
  NRF24L01_Write_Reg(SPI_WRITE_REG+STATUS,state);      //清除TX_DS或MAX_RT中断标志
  if(state&MAX_TX)                                     //达到最大重发次数
  {
          NRF24L01_Write_Reg(FLUSH_TX,0xff);               //清除TX FIFO寄存器 
          return RT_ETIMEOUT; 
  }
  if(state&TX_OK)                                      //发送完成
  {
          return RT_EOK;
  }
  return RT_ERROR;                                         //其他原因发送失败
}

/*******************************************************************************
* Function Name  : NRF24L01_RxPacket
* Description    : 启动NRF24L01接收一次数据
* Input          : rxbuf:接收数据首地址
* Output         : None
* Return         :  接收完成状况
*******************************************************************************/
static rt_err_t NRF24L01_RxPacket(rt_uint8_t *rxbuf)
{
  rt_uint8_t state;		    							      
  state=NRF24L01_Read_Reg(STATUS);                //读取状态寄存器的值    	 
  NRF24L01_Write_Reg(SPI_WRITE_REG+STATUS,state); //清除TX_DS或MAX_RT中断标志
  if(state&RX_OK)                                 //接收到数据
  {
          NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
          NRF24L01_Write_Reg(FLUSH_RX,0xff);          //清除RX FIFO寄存器 
          return RT_EOK; 
  }	   
  return RT_ERROR;                                      //没收到任何数据
}

/*******************************************************************************
* Function Name  : RX_Mode
* Description    : 该函数初始化NRF24L01到RX模式
*  设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
*  当CE变高后,即进入RX模式,并可以接收数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void RX_Mode(void)
{
	Clr_NRF24L01_CE;	  
  	NRF24L01_Write_Buf(SPI_WRITE_REG+RX_ADDR_P0,(rt_uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	  
  	NRF24L01_Write_Reg(SPI_WRITE_REG+EN_AA,0x01);    //使能通道0的自动应答    
  	NRF24L01_Write_Reg(SPI_WRITE_REG+EN_RXADDR,0x01);//使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(SPI_WRITE_REG+RF_CH,40);	     //设置RF通信频率		  
  	NRF24L01_Write_Reg(SPI_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(SPI_WRITE_REG+RF_SETUP,0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(SPI_WRITE_REG+CONFIG, 0x0f);  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	Set_NRF24L01_CE;                                //CE为高,进入接收模式 
}						 

/*******************************************************************************
* Function Name  : TX_Mode
* Description    : 该函数初始化NRF24L01到TX模式
*  设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,
*  波特率和LNA HCURR PWR_UP,CRC使能,当CE变高后,即进入TX模式,发送数据
*  CE为高大于10us,则启动发送.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void TX_Mode(void)
{														 
  Clr_NRF24L01_CE;	    
  NRF24L01_Write_Buf(SPI_WRITE_REG+TX_ADDR,(rt_uint8_t*)TX_ADDRESS,TX_ADR_WIDTH);    //写TX节点地址 
  NRF24L01_Write_Buf(SPI_WRITE_REG+RX_ADDR_P0,(rt_uint8_t*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  NRF24L01_Write_Reg(SPI_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
  NRF24L01_Write_Reg(SPI_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
  NRF24L01_Write_Reg(SPI_WRITE_REG+SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  NRF24L01_Write_Reg(SPI_WRITE_REG+RF_CH,40);       //设置RF通道为40
  NRF24L01_Write_Reg(SPI_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  NRF24L01_Write_Reg(SPI_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
  Set_NRF24L01_CE;                                  //CE为高,10us后启动发送
}		  


/*******************************************************************************
* Function Name  : NRF24L01_Init
* Description    : 初始化24L01的IO口
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NRF24L01_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /*config CE CSN*/
  RCC_APB2PeriphClockCmd(RCC_NRF24L01_CE, ENABLE);           //使能GPIO的时钟
  GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_PIN;              //NRF24L01 模块片选信号
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_NRF24L01_CE, &GPIO_InitStructure);

  RCC_APB2PeriphClockCmd(RCC_NRF24L01_CSN, ENABLE);          //使能GPIO的时钟
  GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_PIN;      
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_NRF24L01_CSN, &GPIO_InitStructure);

  Set_NRF24L01_CE;                                           //初始化时先拉高
  Set_NRF24L01_CSN;                                   //初始化时先拉高

  /*config irq*/
  GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;     //上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_NRF24L01_IRQ, &GPIO_InitStructure);
  GPIO_SetBits(GPIO_NRF24L01_IRQ,NRF24L01_IRQ_PIN);

  SPI1_Init();                                       //初始化SPI
  Clr_NRF24L01_CE; 	                               //使能24L01
  Set_NRF24L01_CSN;                                  //SPI片选取消
}

/*******************************************************************************
* Function Name  : NRF24L01_Check
* Description    : 上电检测NRF24L01是否在位
* Input          : None
* Output         : None
* Return         : RT_EOK:表示在位;RT_ERROR，表示不在位
*******************************************************************************/
static rt_err_t NRF24L01_Check(void)
{
  rt_uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
  rt_uint8_t buf1[5];
  rt_uint8_t i;   

  NRF24L01_Write_Buf(SPI_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
  NRF24L01_Read_Buf(TX_ADDR,buf1,5);              //读出写入的地址  	
  for(i=0;i<5;i++)if(buf1[i]!=0XA5)break;					   
  if(i!=5)return RT_ERROR;                               //NRF24L01不在位	
  return RT_EOK;		                                //NRF24L01在位
}	 	 

/******************************以下finsh中测试函数*****************************/
static void rt_rf_thread_entry(void* parameter)
{
  rt_uint8_t buf[TX_ADR_WIDTH] = {0x00};

  NRF24L01_Init();  
  if(NRF24L01_Check()==RT_EOK)
  {
    rt_kprintf("\r\n RF1 模块初始化成功！\r\n");
  }
  else  
  {
    rt_kprintf("\r\n RF1 模块不存在！\r\n");
    return;
  }
  RX_Mode();
  while (1)
  {  
    if(NRF24L01_RxPacket(buf) == RT_EOK)
    {
       rt_kprintf("\r\n RF1 接收到数据：\r\n"); 
        {
          rt_kprintf((char const*)buf);
        }
    }
     
    rt_thread_delay(RT_TICK_PER_SECOND/100);        
  }
}

int rf1start(void)
{ 
    rt_thread_t tid;

    tid = rt_thread_create("rf1",
        rt_rf_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX-11, 10);
    if (tid != RT_NULL) rt_thread_startup(tid);

    return 0;
}

void rf1send(rt_uint8_t * str)
{ 
      rt_kprintf("\r\n \r\n"); 
      TX_Mode();
      rt_kprintf("\r\n RF1 发送数据：%s \r\n", str); 
      NRF24L01_TxPacket(str);
      RX_Mode();
}


void rf1read(void)
{ 
  rt_uint8_t data;
  rt_uint8_t buf[32];

  NRF24L01_Read_Buf( CONFIG      , &data , 1);       
  rt_kprintf("\r\n CONFIG = 0x%02x", data);
  NRF24L01_Read_Buf( EN_AA       , &data , 1);       
  rt_kprintf("\r\n EN_AA = 0x%02x", data);
  NRF24L01_Read_Buf( EN_RXADDR   , &data , 1);      
  rt_kprintf("\r\n EN_RXADDR = 0x%02x", data);
  NRF24L01_Read_Buf( SETUP_AW    , &data , 1);       
  rt_kprintf("\r\n SETUP_AW = 0x%02x", data);
  NRF24L01_Read_Buf( SETUP_RETR  , &data , 1);      
  rt_kprintf("\r\n SETUP_RETR = 0x%02x",data);
  NRF24L01_Read_Buf( RF_CH       , &data , 1);       
  rt_kprintf("\r\n RF_CH = 0x%02x",data);
  NRF24L01_Read_Buf( RF_SETUP    , &data , 1);      
  rt_kprintf("\r\n RF_SETUP = 0x%02x",data);
  NRF24L01_Read_Buf( STATUS      , &data , 1);       
  rt_kprintf("\r\n STATUS = 0x%02x",data);
  NRF24L01_Read_Buf( OBSERVE_TX  , &data , 1);       
  rt_kprintf("\r\n OBSERVE_TX = 0x%02x",data);
  NRF24L01_Read_Buf( CD          , &data , 1);      
  rt_kprintf("\r\n CD = 0x%02x",data);
  NRF24L01_Read_Buf( RX_ADDR_P0  , buf , 5);       
  rt_kprintf("\r\n RX_ADDR_P0 = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", buf[0],buf[1],buf[2],buf[3],buf[4]);
  NRF24L01_Read_Buf( RX_ADDR_P1  , buf , 5);      
  rt_kprintf("\r\n RX_ADDR_P1 = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", buf[0],buf[1],buf[2],buf[3],buf[4]);
  NRF24L01_Read_Buf( RX_ADDR_P2  , &data , 1);       
  rt_kprintf("\r\n RX_ADDR_P2 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_ADDR_P3  , &data , 1);      
  rt_kprintf("\r\n RX_ADDR_P3 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_ADDR_P4  , &data , 1);       
  rt_kprintf("\r\n RX_ADDR_P4 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_ADDR_P5  , &data , 1);      
  rt_kprintf("\r\n RX_ADDR_P5 = 0x%02x",data);
  NRF24L01_Read_Buf( TX_ADDR     , buf , 5);       
  rt_kprintf("\r\n TX_ADDR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", buf[0],buf[1],buf[2],buf[3],buf[4]);
  NRF24L01_Read_Buf( RX_PW_P0    , &data , 1);      
  rt_kprintf("\r\n RX_PW_P0 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_PW_P1    , &data , 1);       
  rt_kprintf("\r\n RX_PW_P1 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_PW_P2    , &data , 1);      
  rt_kprintf("\r\n RX_PW_P2 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_PW_P3    , &data , 1);       
  rt_kprintf("\r\n RX_PW_P3 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_PW_P4    , &data , 1);      
  rt_kprintf("\r\n RX_PW_P4 = 0x%02x",data);
  NRF24L01_Read_Buf( RX_PW_P5    , &data , 1);       
  rt_kprintf("\r\n RX_PW_P5 = 0x%02x",data);
  NRF24L01_Read_Buf( FIFO_STATUS , &data , 1);      
  rt_kprintf("\r\n FIFO_STATUS = 0x%02x",data);

}

#include "rtthread.h"
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(rf1start, startup rf1start);
FINSH_FUNCTION_EXPORT(rf1read, startup rf1read);
FINSH_FUNCTION_EXPORT(rf1send, startup rf1send);

#endif
