/****************************2012-2013, NJUT, Edu.****************************** 
FileName: adctest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.07.30
Description:    adc.C ADC_Temperature.c 驱动测试程序 
              1.测量ADC值，并打印
              2.测量CPU温度值，并打印 
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/07/30     1.0     文件创建   
********************************************************************************/ 

#include <rtthread.h>
#include "adc.h"
#include  <finsh.h> 

/*******************************************************************************
* Function Name  : test_adc
* Description    : adc.C ADC_Temperature.c 驱动测试函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void test_adc (void)
{
  ADC_Config();
  getadcvalue();
  ADCTEMP_Config();
  gettemperature(); 
}


#ifdef  RT_USING_FINSH 
  #include  <finsh.h> 
  FINSH_FUNCTION_EXPORT(test_adc, startup adc convert e.g. test_adc());
  FINSH_FUNCTION_EXPORT(getadcvalue, startup adc convert e.g. getadcvalue());
  FINSH_FUNCTION_EXPORT(gettemperature, startup temperature convert e.g. gettemperature());
#endif