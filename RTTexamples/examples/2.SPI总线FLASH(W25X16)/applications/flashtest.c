/*********************2012-2013, NJUT, Edu.********************* 
FileName: flashtest.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.07.30
Description:    spi_flash 驱动测试程序 
              1.读FlashID DeviceID ,与内存存储ID匹配一致后进行读写
              2.先写后读，比较一致后，打印结果
              3.注意，首先要在调试中确定FLASH的ID号
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/07/30     1.0     文件创建   
***************************************************************/ 

#include "spi_flash.h"
#include  <rtthread.h >
#include  <finsh.h> 

  /* Private typedef -----------------------------------------------------------*/
  
/* Private define ------------------------------------------------------------*/
#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t TimingDelay;

#define  sFLASH_ID       0xEF4016
#define  sDevice_ID		0x000015

#define  BufferSize (countof(Tx_Buffer)-1)

/* Private macro -------------------------------------------------------------*/
#define countof(a) (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/
uint8_t  Tx_Buffer[] = "\n\r STM32F107 SPI Flash 测试实验： \n\r ===========================================\r\n";
uint8_t Index, Rx_Buffer[BufferSize];

rt_bool_t   TransferStatus1 = RT_FALSE, TransferStatus2 = RT_TRUE;

__IO uint32_t FlashID = 0;
__IO uint32_t DeviceID = 0;

/*******************************************************************************
* Function Name  : Buffercmp
* Description    : Compares two buffers.
* Input          : - pBuffer1, pBuffer2: buffers to be compared.
*                : - BufferLength: buffer's length
* Output         : None
* Return         : PASSED: pBuffer1 identical to pBuffer2
*                  FAILED: pBuffer1 differs from pBuffer2
*******************************************************************************/
rt_bool_t Buffercmp(u8* pBuffer1, u8* pBuffer2, u16 BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return RT_FALSE;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return RT_TRUE;
}


void test_flash(void)
{

  SPI_FLASH_Init();
   /* Get SPI Flash Device ID */
  DeviceID = SPI_FLASH_ReadDeviceID();
  rt_thread_delay(50);
  
  /* Get SPI Flash ID */
  FlashID = SPI_FLASH_ReadID();

  rt_kprintf("\r\n FlashID is 0x%X", FlashID);
  rt_kprintf("\r\n DeviceID is 0x%X", DeviceID);
  
  if (FlashID == sFLASH_ID)  /* #define  sFLASH_ID       0xEF3015 */
  {

    /* Perform a write in the Flash followed by a read of the written data */
    /* Erase SPI FLASH Sector to write on */
    SPI_FLASH_SectorErase(FLASH_SectorToErase);

    rt_kprintf("%s", Tx_Buffer);
    
    /* Write Tx_Buffer data to SPI FLASH memory */
    SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);

    /* Read data from SPI FLASH memory */
    SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);

    /* Check the corectness of written dada */
    TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
    /* TransferStatus1 = PASSED, if the transmitted and received data by SPI1
       are the same */
    /* TransferStatus1 = FAILED, if the transmitted and received data by SPI1
       are different */

    /* Perform an erase in the Flash followed by a read of the written data */
    /* Erase SPI FLASH Sector to write on */
    SPI_FLASH_SectorErase(FLASH_SectorToErase);

    /* Read data from SPI FLASH memory */
    SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);

    /* Check the corectness of erasing operation dada */
    for (Index = 0; Index < BufferSize; Index++)
    {
      if (Rx_Buffer[Index] != 0xFF)
      {
        TransferStatus2 = RT_FALSE;
      }
    }
    
    /* TransferStatus2 = PASSED, if the specified sector part is erased */
    /* TransferStatus2 = FAILED, if the specified sector part is not well erased */

    if((RT_TRUE == TransferStatus1) && (RT_TRUE == TransferStatus2))
    {
        rt_kprintf("\r\n W25X16 Test Suceed!\n\r");
    }
    else
    {
         rt_kprintf("\r\n -->Failed: W25X16 Test Failed!\n\r");
    }
  }
  else
  {
    rt_kprintf("\r\n W25X16 Test Failed!\n\r");
  }

  SPI_Flash_PowerDown();  
 }


FINSH_FUNCTION_EXPORT(test_flash,  startup flash test e.g: test_flash()); 
 

