/******************************2014-2015, NJTECH, Edu.************************** 
FileName: rfspi.c 
Author:  孙冬梅       Version :  1.0        Date: 2015.05.30
Description:    射频模块SPI1 和 SPI3 驱动接口函数     
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    15/05/30     1.0     文件创建  
1.SPI端口初始化
SPI1: SCK SDI SDO = PA5 PA6 PA7
SPI3:SCK SDI SDO =PC10 PC11 PC12
2.设置速度等SPI参数
3.初始化SPI端口，启动RF传输
*******************************************************************************/
#include "rfspi.h"
#include "stm32f10x.h"


/*******************************************************************************
* Function Name  : SPI3_Init
* Description    : 行外设接口SPI3的初始化，SPI配置成主模式
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_Init(void)
{	 
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE );	
        GPIO_PinRemapConfig(GPIO_Remap_SPI3,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);   
	//SPI3口初始化
	/* Configure SPI1 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI3_MISO| SPI3_MOSI| SPI3_SCK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_SPI3, &GPIO_InitStructure);

    GPIO_SetBits(GPIO_SPI3,SPI3_MISO| SPI3_MOSI| SPI3_SCK);

	/* SPI3 configuration */                                            //初始化SPI1结构体
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI1设置为两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		                //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		            //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		                    //串行时钟在不操作时，时钟为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	                    //第一个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		                    //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  //SPI波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	                //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	                        //CRC值计算的多项式

	SPI_Init(SPI3, &SPI_InitStructure);                                 //根据SPI_InitStruct中指定的参数初始化外设SPI3寄存器
	
	/* Enable SPI3  */
	SPI3_SetSpeed(SPI_SPEED_256);                                           //设置为低速模式
	SPI_Cmd(SPI3, ENABLE);                                              //使能SPI1外设
	SPI3_ReadWriteByte(0xff);                                           //启动传输		 

}  

/*******************************************************************************
* Function Name  : SPI1_Init
* Description    : 行外设接口SPI1的初始化，SPI配置成主模式
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_Init(void)
{	 
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE );	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	

	//SPI1口初始化
	/* Configure SPI1 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO| SPI1_MOSI| SPI1_SCK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_SPI1, &GPIO_InitStructure);

        GPIO_SetBits(GPIO_SPI1,SPI1_MISO| SPI1_MOSI| SPI1_SCK);

	/* SPI1 configuration */                                            //初始化SPI1结构体
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI1设置为两线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		                //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		            //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//Low		            //串行时钟在不操作时，时钟为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//1Edge	                //第一个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		                    //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  //SPI波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	                //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	                        //CRC值计算的多项式

	SPI_Init(SPI1, &SPI_InitStructure);                                 //根据SPI_InitStruct中指定的参数初始化外设SPI1寄存器
	
	/* Enable SPI1  */
	SPI1_SetSpeed(SPI_SPEED_256);                                           //设置为低速模式
	SPI_Cmd(SPI1, ENABLE);                                              //使能SPI1外设
	SPI1_ReadWriteByte(0xff);                                           //启动传输		 
}  
/*******************************************************************************
* Function Name  : SPI3_ReadWriteByte
* Description    : SPI3读写数据函数
* Input          : 要写入的数据
* Output         : None
* Return         : 读出的数据
*******************************************************************************/
u8 SPI3_ReadWriteByte(u8 TxData)                                       
{		
	u8 retry=0;				 	
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET)      //发送缓存标志位为空
		{
		retry++;
		if(retry>200)return 0;
		}			  
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI3, TxData);                                    //通过外设SPI1发送一个数据
	retry=0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);   //接收缓存标志位不为空
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI3);                                 //通过SPI1返回接收数据				    
}

/*******************************************************************************
* Function Name  : SPI1_ReadWriteByte
* Description    : SPI1读写数据函数
* Input          : 要写入的数据
* Output         : None
* Return         : 读出的数据
*******************************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)                                       
{		
	u8 retry=0;				 	
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)      //发送缓存标志位为空
		{
		retry++;
		if(retry>200)return 0;
		}			  
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, TxData);                                    //通过外设SPI1发送一个数据
	retry=0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)   //接收缓存标志位为空循环等待
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);                                 //通过SPI1返回接收数据				    
}

/*******************************************************************************
* Function Name  : SPI1_SetSpeed
* Description    : SPI1设置SPI速率,后SPI 设备使能 
* Input          : 要设置的速度  SPI_SPEED_2   8   16    256
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_SetSpeed(u8 SpeedSet)
  { 
      SPI1->CR1&=0XFFC7;   //Fsck=Fcpu/256 
      if(SpeedSet==SPI_SPEED_2) //二分频 
     { 
       SPI1->CR1|=0<<3; //Fsck=Fpclk/2=36Mhz 
      }
      else if(SpeedSet==SPI_SPEED_8)
          //八分频 
      { 
         SPI1->CR1|=2<<3; //Fsck=Fpclk/8=9Mhz 
       }
      else if(SpeedSet==SPI_SPEED_16)
        //十六分频 
      { 
        SPI1->CR1|=3<<3; //Fsck=Fpclk/16=4.5Mhz 
      }
      else                       //256 分频 
      { 
          SPI1->CR1|=7<<3;  //Fsck=Fpclk/256=281.25Khz             低速模式 
       } 
        SPI1->CR1|=1<<6;    //SPI 设备使能 
} 

/*******************************************************************************
* Function Name  : SPI3_SetSpeed
* Description    : SPI3设置SPI速率,后SPI 设备使能 
* Input          : 要设置的速度  SPI_SPEED_2   8   16    256
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_SetSpeed(u8 SpeedSet)
  { 
      SPI3->CR1&=0XFFC7;   //Fsck=Fcpu/256 
      if(SpeedSet==SPI_SPEED_2) //二分频 
     { 
       SPI3->CR1|=0<<3; //Fsck=Fpclk/2=36Mhz 
      }
      else if(SpeedSet==SPI_SPEED_8)
          //八分频 
      { 
         SPI3->CR1|=2<<3; //Fsck=Fpclk/8=9Mhz 
       }
      else if(SpeedSet==SPI_SPEED_16)
        //十六分频 
      { 
        SPI3->CR1|=3<<3; //Fsck=Fpclk/16=4.5Mhz 
      }
      else                       //256 分频 
      { 
          SPI3->CR1|=7<<3;  //Fsck=Fpclk/256=281.25Khz             低速模式 
       } 
        SPI3->CR1|=1<<6;    //SPI 设备使能 
} 



















