/****************************2012-2013, NJUT, Edu.****************************** 
FileName: wdgtest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.07.30
Description:    看门狗测试程序 

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/07/30     1.0     文件创建   
********************************************************************************/ 
#include  <rtthread.h >
#include "stm32f10x.h"
#include "led.h"
#include "key.h"

/*******************************************************************************
* Function Name  : iwdg_init
* Description    : 初始化看门狗
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void iwdg_init()
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); // 关闭 I WDG_PR 和 IWDG_RLR 的写保护
    IWDG_SetReload(0xfff); // 设置重装载值为 0xfff
    IWDG_SetPrescaler(IWDG_Prescaler_32); // 设置预分频系数为 32 间隔时间3S 参考手册P317
    // IWDG_ReloadCounter(); 在按键中断服务程序中添加这一句喂狗
    IWDG_Enable(); // 使能看门狗
}

void test_iwdg(void)
{
    Key_Init();
    Set_Keyint();//在KEY2 中断服务程序中喂狗 
    LED_Init();
    iwdg_init();  
}
 
/*******************************************************************************
* Function Name  : EXTI13_IRQHandler
* Description    : 外部中断按键2服务函数,LED2 3闪烁 喂狗操作 运行此函数，注释掉key.c中的中断函数
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
/* */void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
          LEDTog(LED2);
          LEDTog(LED3);
          IWDG_ReloadCounter(); // 把重装载值写入看门狗中，俗称喂狗
          EXTI_ClearITPendingBit(EXTI_Line13);
          rt_kprintf("\r\n 正在喂狗！");
	} 
}



/*******************************************************************************
* Function Name  : WWDGConfig
* Description    : 窗口看门狗配置
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void WWDGConfig(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;         
   /* 将外设 WWDG寄存器重设为缺省值 初始化关闭窗口看门狗时钟   WWDG_DeInit();  */
  
    /* 开启窗口看门狗时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE);
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	 
   
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn ; //通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //占先中断等级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    //响应中断优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);                        //配置优先级
    
     
    /* Set WWDG prescaler to 8 设置 WWDG 预分频值 即PCLK1(72M)/4096/8=2197 Hz*/ 
    WWDG_SetPrescaler(WWDG_Prescaler_8);
    /* Set WWDG window value to 0x41指定的窗口值，该参数取值必须在 0x40 与 0x7F之间。  */ 
    WWDG_SetWindowValue(0x40); 
    
    /* Enable WWDG and set counter value to 0x7F 使能窗口看门狗，并把看门狗计数器的值设为0x7f*/
    WWDG_Enable(0x7f);
    /* Clear EWI flag 清除早期唤醒中断标志位 */ 
    WWDG_ClearFlag();  
    /* Enable WWDG Early wakeup interrupt 使能 WWDG 早期唤醒中断（EWI）  */ 
    WWDG_EnableIT();  
}


void test_wwdg(void)
{
      LED_Init();
      WWDGConfig();
}

   
/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : 窗口看门狗中断服务 灯4闪烁 喂狗操作
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)//窗口看门狗中断程序，喂狗操作
{
        static uint32_t count=0;
        if(count>=10)//进10次中断LEC4翻转一次 10*0x40(64)/ 2197=291ms  
        {
          LEDTog(LED4);
          count = 0;
        }
        else count++;
        
        WWDG_SetCounter(0x7f); // 设置计数器值为0x7f 
        
        WWDG_ClearFlag(); // 清除早期唤醒中断标志位 
}


   #ifdef  RT_USING_FINSH 
   #include  <finsh.h> 
   /*  输出CRCTest IWDGTest WWDGTest函数到finsh  shell中 */ 
   FINSH_FUNCTION_EXPORT (test_iwdg,  startup  IWDG test); 
   FINSH_FUNCTION_EXPORT (test_wwdg,  startup  WWDG test); 
     
   #endif 
