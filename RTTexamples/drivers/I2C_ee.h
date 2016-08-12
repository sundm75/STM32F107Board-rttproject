#ifndef __IIC_EE_H
#define __IIC_EE_H
#include"stm32f10x.h"	

#define I2C_PageSize      8

void IIC_Configuration(void);
void IIC_Byte_Write( u8 pBuffer, u8 Word_Address);
void IIC_Wait_EEprom(void);
void IIC_Byte_Read( u8* pRead, u8 Word_Address);
void IIC_EE_WriteOnePage(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
void IIC_Buffer_Write(u8* pBuffer, u8 WriterAddr, u16 NumByteToRead);
void IIC_Buffer_Read(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead);
#endif	 
