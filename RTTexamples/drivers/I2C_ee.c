/*
I2C_ee.C 驱动程序 采用硬件I2C查询方式

*/
#include "I2C_ee.h"
#define DEVICE_ADDRESS 0xA0
/*******************************************************************************
* Function Name  : IIC_Configuration
* Description    : 初始化IIC2外设
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IIC_Configuration(void)
{
  GPIO_InitTypeDef	 GPIO_InitStructure;
  I2C_InitTypeDef    I2C_InitStructure;

  /*打开时钟*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);		 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 ,ENABLE);  

  /*定义GPIO口为开漏输出 复用*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; 	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	
  GPIO_Init(GPIOB,&GPIO_InitStructure);			

  /*配置I2C*/
  I2C_InitStructure.I2C_ClockSpeed = 50000;          /*时钟频率400kHz */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;        
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;  /*CCR寄存器占空比 50% */
  I2C_InitStructure.I2C_OwnAddress1 = 0xB0 ;         /*作为从机时的地址*/
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;        /*应答使能 */
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /*7位地址*/
  I2C_Init(I2C1,&I2C_InitStructure);

  /*使能I2C*/
  I2C_Cmd(I2C1, ENABLE);
}


/*******************************************************************************
* Function Name  : IIC_Wait_Eeprom
* Description    : 等待IIC总线空闲，同时等待设备空闲，这里的这个函数完全没有必要加，而且这里的刘凯视频上给的写法是有问题的，
                   那个判断函数while里面的东西是有问题的，他的本意是等ADDR=1但是那样写能跳出来吗？不对啊
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
void IIC_Wait_EEprom(void)
{
//	while((I2C_ReadRegister(I2C1, I2C_Register_SR2) & 0x0002))         /*判断总线是否为闲置，也就是SR2_BUSY是否为0，为0表示总线不忙*/
//	{}                                                               
//  do
//  {
//    I2C_GenerateSTART(I2C1, ENABLE);                                 /* 发送起使条件，该函数让CR1的start位软件置为1，
//																																		 当起使条件发出后，硬件自动清为0 */  
//		
//		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)) /*判断EV5,看下函数定义可以发现，该事件是SB=1，MSL=1，BUSY=1 
//	  {}                                                         意思是起始条件已经发送了，然后是主模式，总线在通讯*/
//   /* while(!(I2C_ReadRegister(I2C2, I2C_Register_SR1) & 0x0001));*//*等待SR1_SB为1表示起始条件已经发出了，如果线忙呢，
//		                                                                 这样是是不是重复发送起始条件，不就影响到线了？第一句加
//		                                                                 了判断总线是否为忙*/
//		
//    I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter);/* 发送器件的地址，是写到DR寄存器里面默认的器件的
//		                                                                     地址的是8位组成的，高7位是地址，bit0保持0，
//		                                                                     而I2C_Direction_Transmitter
//                                                                         会将bit0改成0，这是写的含义；
//		                                                                     如果是I2C_Direction_Receiver则会将bit0变成1，
//		                                                                     这就是收的含义，参加eeprom里面地址位上
//                                                                         的最低位R/W的含义*/
//  }
//	while(!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));/*是看SR1寄存器的ADDR位是否为1，1就退出while，否则就是继续循环
//	                                                             这里是判断器件如eeprom是否为闲置，也就是器件是否发出应答，有应
//	                                                             答说明器件不忙*/
//	
//  I2C_ClearFlag(I2C1, I2C_FLAG_AF);	                          /* 清除SR1的AF标志位，AF为1表示应答失败，0表示成功，要软件清零，
//	                                                             这里退出while就意味着应答成功了 */




vu16 SR1_Tmp = 0;

  do
  {
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    /* Read I2C1 SR1 register */
    SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);
    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter); //问题是这句话只是写入了地址到DR寄存器里面，这离下一句的while
		                                                                      //判断时间很短，要是数据还没发完，设备也没响应，也就是ADDR=0，
		                                                                      //就已经到了while里面的读SR1了，那岂不是出不去while了？
  }while(!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));
  
  /* Clear AF flag */
  I2C_ClearFlag(I2C1, I2C_FLAG_AF);	


}

/*******************************************************************************
* Function Name  : IIC_Byte_Write
* Description    : 对eerpom（at24c02）的写操作，是一个字节的写操作。
* Input          : u8* pbuffer, u8 Word_Address （这里的地址要注意一下规则）
* Output         : None
* Return         : None 
*******************************************************************************//**/
void IIC_Byte_Write( u8 pBuffer, u8 Word_Address)
{
	 //IIC_Wait_EEprom();                 /*等待总线空闲，等待设备器件空闲（其中需要软件清除SR1的AF位，即应答成功）*/
	
	 I2C_GenerateSTART(I2C1, ENABLE);   /*发送一个s，也就是起始信号，因为前面的函数没有终止，这次开始也是需要重新的一个开始信号*/
	 //EV5事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)); /*判断EV5,看下函数定义可以发现，该事件是SB=1，MSL=1，BUSY=1 
	                                                            意思是起始条件已经发送了，然后是主模式，总线在通讯*/
	 
	 I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter);  /*发送器件地址，最后一个参数表示地址bit0为0，意思
	                                                                        是写操作，同时由于写了DR寄存器，故会清除SB位变成0*/
	
	 //EV6 EV8_1（该事件判断同时判断了EV8_1事件）
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));/*判断EV6和EV8_1的条件，此时SB=0，ADDR=1地址发送结束，
	                                                                     TXE=1数据寄存器DR为空，BUSY=1总不空闲,MSL=1主模式,TRA=1
	                                                                        数据已经发送了（因为是写操作，其实是地址数据已经发送了）
	                                                                        如果是主收模式，这里的EV6，TRA=1表示数据还没收到，0表示
	                                                                        收到数据，注意这里TRA=1表示已经发送了，ADDR=1才是发送完成
	                                                                        了,做完该事件，ADDR=0了又*/                                                                           
	 
	 I2C_SendData(I2C1,Word_Address);      /*EV8_1事件，因为这一步时候DR已经为空，该事件是写入data1，对于EEPROM来说这个data1是要写入
	                                      字节的的地址，data2是要写入的内容，data1为8位1k的有效字节是低7位，2kbit的有效字节是8位，32页，
	                                      每页8个字节，一共2k位，16kbit需要11位？怎么送？用硬件的A1，2，3接GPIO来选择存储的页*/
	                         
	 
	 //EV8事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING)); /*为下一步执行EV8事件，判断TXE=1 DR寄存器为空，准备写入DR寄存器
	                                                                 data2，注意此时也判定TRA=1意思是data1已经发送了不表示发送完成，
	                                                                 移位寄存器非空表示数还在发，另外BTF=0说明data1字节发送没完成，
																																	 NOSTRETCH为0时候，BTF=1还表示新数据要被发送了（意味着字节发送）
	                                                                 完成了但是新数据还没有写入到DR里面，这里的EV8事件对于的一段一直
																																	 是有数据发送的，不存在BTF=1的情况*/ 
																																	 																															                                                                  
	 I2C_SendData(I2C1,pBuffer);                                    /*数据寄存器DR为空，这里是写入data2，该步骤隶属于EV8事件*/
	
	/*EV8_2事件（这里就发送两个data，所以就只有一个EV8事件，
	  EV8_1—data1，EV8_1—data2，EV8_2和EV8的区别是检测的
		差个BTF,且EV8_2不写DR而是程序要求停止）*/
	  
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); /*EV8_2的事件判断和EV8事件不一样，TRA=1表示data2已经发送了，
	                                                                  BTF=1字节发送结束，说明这一时刻没有字节在发送，其实表示在
																																		data2正在发送的时候，没有data3写入到DR里面， 然后现在该轮到要
	                                                                  发送data3了，但是DR里面是空的（其实发送data2的时候，中间某时
																																		刻DR就已经空了）*/
	                                                                																																
	 I2C_GenerateSTOP(I2C1,ENABLE);                                  /*EV8_2事件中的程序写停止*/
}
/*向E2PROM写一页，不能跨页*/
void IIC_EE_WriteOnePage(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
  if(NumByteToWrite>0)
  {
	 //IIC_Wait_EEprom();                 /*等待总线空闲，等待设备器件空闲（其中需要软件清除SR1的AF位，即应答成功）*/
	
	 I2C_GenerateSTART(I2C1, ENABLE);   /*发送一个s，也就是起始信号，因为前面的函数没有终止，这次开始也是需要重新的一个开始信号*/
	 //EV5事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)); /*判断EV5,看下函数定义可以发现，该事件是SB=1，MSL=1，BUSY=1 
	                                                            意思是起始条件已经发送了，然后是主模式，总线在通讯*/
	 
	 I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter);  /*发送器件地址，最后一个参数表示地址bit0为0，意思
	                                                                        是写操作，同时由于写了DR寄存器，故会清除SB位变成0*/
	
	 //EV6 EV8_1（该事件判断同时判断了EV8_1事件）
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));/*判断EV6和EV8_1的条件，此时SB=0，ADDR=1地址发送结束，
	                                                                     TXE=1数据寄存器DR为空，BUSY=1总不空闲,MSL=1主模式,TRA=1
	                                                                        数据已经发送了（因为是写操作，其实是地址数据已经发送了）
	                                                                        如果是主收模式，这里的EV6，TRA=1表示数据还没收到，0表示
	                                                                        收到数据，注意这里TRA=1表示已经发送了，ADDR=1才是发送完成
	                                                                        了,做完该事件，ADDR=0了又*/                                                                           
	 
	 I2C_SendData(I2C1,WriteAddr);      /*EV8_1事件，因为这一步时候DR已经为空，该事件是写入data1，对于EEPROM来说这个data1是要写入
	                                      字节的的地址，data2是要写入的内容，data1为8位1k的有效字节是低7位，2kbit的有效字节是8位，32页，
	                                      每页8个字节，一共2k位，16kbit需要11位？怎么送？用硬件的A1，2，3接GPIO来选择存储的页*/
	                         
	 
	 //EV8事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING)); /*为下一步执行EV8事件，判断TXE=1 DR寄存器为空，准备写入DR寄存器
	                                                                 data2，注意此时也判定TRA=1意思是data1已经发送了不表示发送完成，
	                                                                 移位寄存器非空表示数还在发，另外BTF=0说明data1字节发送没完成，
																																	 NOSTRETCH为0时候，BTF=1还表示新数据要被发送了（意味着字节发送）
	                                                                 完成了但是新数据还没有写入到DR里面，这里的EV8事件对于的一段一直
																																	 																															                                                                  
																																 是有数据发送的，不存在BTF=1的情况*/ 
	while(	NumByteToWrite--)
        {
         I2C_SendData(I2C1,*pBuffer++);                                    /*数据寄存器DR为空，这里是写入data2，该步骤隶属于EV8事件*/
	
	/*EV8_2事件（这里就发送两个data，所以就只有一个EV8事件，
	  EV8_1—data1，EV8_1—data2，EV8_2和EV8的区别是检测的
		差个BTF,且EV8_2不写DR而是程序要求停止）*/
	  
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); /*EV8_2的事件判断和EV8事件不一样，TRA=1表示data2已经发送了，
	                                                                  BTF=1字节发送结束，说明这一时刻没有字节在发送，其实表示在
																																		data2正在发送的时候，没有data3写入到DR里面， 然后现在该轮到要
	                                                                  发送data3了，但是DR里面是空的（其实发送data2的时候，中间某时
																																		刻DR就已经空了）*/
	                                                                																																
        }
        
         I2C_GenerateSTOP(I2C1,ENABLE);                                  /*EV8_2事件中的程序写停止*/
  }
}

/*******************************************************************************
* Function Name  : IIC_Buffer_Write
* Description    : 对eerpom（at24c02）的写操作，写多字节
* Input          : u8* pbuffer, u8 WriterAddr，u16 NumByteToWrite
* Output         : None
* Return         : None 
*******************************************************************************//**/
void IIC_Buffer_Write(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
  u8 AlignedStartAddr = 0;
  u8 AlignedEndAddr = 0;
  u8 AlignedCount = 0;
  u8 StartDiffNum = 0;
  u8 EndDiffNum = 0;
  /*WriterAddr StartDiffNum AlignedStartAddr AlignedCount AlignedEndAddr EndDiffNum*/
  if((WriteAddr % I2C_PageSize) ==0)
  {
    AlignedStartAddr = WriteAddr;
    StartDiffNum = 0;
  }
  else
  {
    AlignedStartAddr = (WriteAddr / I2C_PageSize + 1)*I2C_PageSize;
    StartDiffNum = AlignedStartAddr - WriteAddr;
  }
  AlignedCount = (NumByteToWrite - StartDiffNum)/I2C_PageSize;
  AlignedEndAddr = AlignedStartAddr + AlignedCount*I2C_PageSize;
  EndDiffNum = NumByteToWrite - AlignedCount*I2C_PageSize - StartDiffNum;
  rt_kprintf("\r\n%d  %d %d  %d  %d  %d\n",WriteAddr, StartDiffNum,AlignedStartAddr,AlignedCount,AlignedEndAddr,EndDiffNum);
  IIC_EE_WriteOnePage(pBuffer, WriteAddr,StartDiffNum); pBuffer = pBuffer+StartDiffNum;
  rt_thread_delay(2);
  while(AlignedCount--)
  {
    IIC_EE_WriteOnePage(pBuffer, AlignedStartAddr,I2C_PageSize); 
    AlignedStartAddr = AlignedStartAddr + I2C_PageSize;
    pBuffer = pBuffer+I2C_PageSize;
    rt_thread_delay(2);
  }
  IIC_EE_WriteOnePage(pBuffer, AlignedEndAddr,EndDiffNum);  
  rt_thread_delay(2);
}

/*******************************************************************************
* Function Name  : IIC_Byte_Read
* Description    : 对eerpom（at24c02）的读操作，是一个字节的读操作，给定一个地址Word_Address然后读取其值并将值赋给pRead。
* Input          : u8* pRead, u8 Word_Address（这里的地址要注意一下规则1,2kbit的就是8位数就能表示满了，4,8,16k的要用硬件引脚A0，1,2，来由GPIO选择页）
* Output         : None
* Return         : None 
*******************************************************************************//**/
void IIC_Byte_Read( u8* pRead, u8 Word_Address)
{
	/*首先写操作，发送设备地址写操作，找到设备应答了，写入要读数据的地址*/
   //IIC_Wait_EEprom(); 
   I2C_GenerateSTART(I2C1, ENABLE);
	
	 //EV5事件
   while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
   I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter); 
	
	 //EV6 EV8_1事件（该事件判断同时判断了EV8_1事件）
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	 {}
	 I2C_SendData(I2C1,Word_Address); 
		 
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); //采用EV8_2的判断条件反应此时已经收到ACK，Word_Address发送完成。
	 
	/*给Start条件，发送设备地址读操作，找到设备应答了，收数据，主机不应答，终止*/
	 I2C_GenerateSTART(I2C1, ENABLE);
		 
	//EV5事件
   while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
   I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Receiver ); 
	
  //EV6事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		 
	//EV6_1事件，没有标志位，要设置ACK失能和停止位产生
	 I2C1->CR1 &= 0xFBFF ;   //失能ACK
	 I2C1->CR1 |= 0x0020 ;   //使能Stop
	
	//EV7事件，读DR寄存器
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
	 * pRead = I2C1->DR;		 
}



/*******************************************************************************
* Function Name  : IIC_Byte_Read
* Description    : 对eerpom（at24c02）的读操作，是一个字节的读操作，给定一个地址Word_Address然后读取其值并将值赋给pRead。
* Input          : u8* pRead, u8 Word_Address（这里的地址要注意一下规则1,2kbit的就是8位数就能表示满了，4,8,16k的要用硬件引脚A0，1,2，来由GPIO选择页）
* Output         : None
* Return         : None 
*******************************************************************************//**/
void IIC_Buffer_Read(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
	/*首先写操作，发送设备地址写操作，找到设备应答了，写入要读数据的地址*/
   //IIC_Wait_EEprom(); 
   I2C_GenerateSTART(I2C1, ENABLE);
	
	 //EV5事件
   while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
   I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Transmitter); 
	
	 //EV6 EV8_1事件（该事件判断同时判断了EV8_1事件）
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	 {}
	 I2C_SendData(I2C1,ReadAddr); 
		 
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); //采用EV8_2的判断条件反应此时已经收到ACK，Word_Address发送完成。
	 
	/*给Start条件，发送设备地址读操作，找到设备应答了，收数据，主机不应答，终止*/
	 I2C_GenerateSTART(I2C1, ENABLE);
		 
	//EV5事件
   while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
   I2C_Send7bitAddress(I2C1, DEVICE_ADDRESS, I2C_Direction_Receiver ); 
	
  //EV6事件
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		 
	//EV6_1事件，没有标志位，要设置ACK失能和停止位产生
//	 I2C1->CR1 &= 0xFBFF ;   //失能ACK
//	 I2C1->CR1 |= 0x0020 ;   //使能Stop
	while(NumByteToRead--)
        {
	//EV7事件，读DR寄存器
	 while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
	 * pBuffer++ = I2C1->DR;
//         if(NumByteToRead==1)
//         {
//           I2C1->CR1 &= 0xFBFF ;   //失能ACK
//           I2C1->CR1 |= 0x0020 ;   //使能Stop
//         }
        }
        I2C_GenerateSTART(I2C1, ENABLE);
}




