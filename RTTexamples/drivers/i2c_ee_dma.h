#ifndef __I2C_EE_DMA_H
#define __I2C_EE_DMA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>

/* Private define ------------------------------------------------------------*/
#define I2C_Speed                     400000   //I2C的速度
#define I2C1_ADDRESS7    		0x01  
#define EEPROM_ADDRESS 			0xA0  //24C02的地址，由硬件决定

/* eeprom 型号 */
#define AT24Cxx     (2)  //定义AT24CXX的型号：括号内为2、4、8、16、32 如：2表示24C02

#ifndef __cplusplus
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

/* Exported types ------------------------------------------------------------*/
typedef enum i2c_result
{
  NO_ERR  = 0,  
  TIMEOUT = 1,
  BUS_BUSY = 2,
  SEND_START_ERR = 3,
  ADDR_MATCH_ERR = 4,
  ADDR_HEADER_MATCH_ERR = 5,
  DATA_TIMEOUT = 6,
  WAIT_COMM = 7,
  STOP_TIMEOUT = 8

}I2C_Result;

typedef enum i2c_state
{
  COMM_DONE  = 0,  // done successfully
  COMM_PRE = 1,
  COMM_IN_PROCESS = 2,
  CHECK_IN_PROCESS = 3,
  COMM_EXIT = 4 // exit since failure
    
}I2C_STATE;

void I2C_EE_Init(void);
void I2C_EE_WriteBuffer(u8* pBuffer, u16 WriteAddr, u16 NumByteToWrite);
void I2C_EE_ReadBuffer(u8* pBuffer, u16 ReadAddr, u16 NumByteToRead);
void I2C_EE_WaitOperationIsCompleted(void);

#endif
