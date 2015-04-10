/*********************2012-2013, NJUT, Edu.************************************ 
FileName: flashtest.c 
Author:  孙冬梅       Version :  1.0        Date: 2012.12.30
Description:    单个字节flash读写测试      
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/08/30     1.0     文件格式高速   
  *          STM32Board Key Pin assignment
******************************************************************************/ 
#include <stm32f10x.h>
#include  <rtthread.h >
#include  <finsh.h> 

#define FLASH_ADR 0x08038000  //要写入数据的地址
#define FLASH_DATA 0xa5a5a5a5  //要写入的数据
void test_flash(void)
{
 u32 tmp;
 
 //判断此FLASH是否为空白
 tmp=*(vu32*)(FLASH_ADR);
 if(tmp==0xffffffff)
 {
  FLASH_Unlock(); //解锁flash编写擦除控制器
  FLASH_ProgramWord(FLASH_ADR,FLASH_DATA);//在指定的地址写一个字
  FLASH_Lock();   //锁定flash编写擦除控制器
  rt_kprintf("已经写入数据\r\n");
 }
 else if(tmp==FLASH_DATA)
 {
  rt_kprintf("读出数据正确\r\n");
 }
 else
 {
  rt_kprintf("读出数据错误\r\n");
  FLASH_Unlock();//解锁flash编写擦除控制器
  FLASH_ErasePage(FLASH_ADR);//擦除flash的一个页面
  FLASH_Lock();   //锁定flash编写擦除控制器
  rt_kprintf("has clear error address!\r\n");
 }
}

/*  输出flashtestbyte 函数到finsh  shell中 
流程：读取该地址数据：
１）	若为０ｘｆｆｆｆｆｆｆｆ，则写入数据，打印"已经写入数据"；
２）	若不为要写入的数据，则擦除该数据，打印"读出数据错误"
３）	若为该数据，则返回，打印"读出数据正确"。
*/ 
FINSH_FUNCTION_EXPORT (test_flash,  test flash write&read e.g.test_flash()); 

