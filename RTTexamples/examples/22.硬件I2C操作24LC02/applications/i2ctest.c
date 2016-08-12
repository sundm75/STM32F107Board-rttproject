/****************************2012-2013, NJUT, Edu.****************************** 
FileName: i2ctest.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.07.30
Description:    I2C.C 驱动测试程序 
              1.写入I2C一串数据并打印
              2.读出I2C一串数据并打印
              3. 数据比较并打印比较结果
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/07/30     1.0     文件创建   
********************************************************************************/ 

#include <rtthread.h>
#include "I2C.h"
#include "key.h"
#include  <finsh.h> 

/*******************************************************************************
* Function Name  : i2ctest
* Description    : I2C.C 驱动测试函数
*              1.写入I2C一串数据并打印
*              2.读出I2C一串数据并打印
*              3. 数据比较并打印比较结果
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void test_i2c (void)
{
  unsigned char  i;  
  
  rt_I2C_init();
  
  rt_kprintf("\r\nI2C总线FLASH（24C02） ok\n");
  
  rt_kprintf("\r\n写入的数据   \n");
  for(i=0;i<64;i++) 
  {   
        rt_I2C_write(0x00+i,i);
        rt_kprintf("0x%02X  ",i);
  }
  
  rt_kprintf("\r\n读取的数据   \n");
  for(i=0;i<64;i++)
  {  
      rt_kprintf("0x%02X  ",rt_I2C_read(i));
  }   
}

FINSH_FUNCTION_EXPORT(test_i2c,  startup flash test e.g: test_i2c()); 

