/*******************************2012-2013, NJUT, Edu.************************** 
FileName: lcd.c 
Author:  孙冬梅       Version :  1.0        Date: 2013.12.30
Description:    lcd驱动  兼容ili9325 ili9320 ssd1289    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/12/30     1.0     文件创建   
  *          STM32Board Key Pin assignment
  *          =============================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+-----------------------------+
  *          |      LCD_Pin                |     Port & Pin              |
  *          +-----------------------------+-----------------------------+
  *          |        CS                   |        C7                   |
  *          |        RD                   |        D13                  |
  *          |        WR                   |        D14                  |
  *          |        RS                   |        D15                  |
  *          |        LCD_D0-D15           |        E0-E15               |
  *          +-----------------------------+-----------------------------+
 ******************************************************************************/ 
#include "lcddevice.h"

/* LCD Control pins */
#define LCD_Pin_WR      GPIO_Pin_14
#define LCD_PORT_WR     GPIOD
#define LCD_CLK_WR      RCC_APB2Periph_GPIOD

#define LCD_Pin_CS      GPIO_Pin_7
#define LCD_PORT_CS     GPIOC
#define LCD_CLK_CS      RCC_APB2Periph_GPIOC

#define LCD_Pin_RS      GPIO_Pin_15
#define LCD_PORT_RS     GPIOD
#define LCD_CLK_RS      RCC_APB2Periph_GPIOD

#define LCD_Pin_RD      GPIO_Pin_13
#define LCD_PORT_RD     GPIOD
#define LCD_CLK_RD      RCC_APB2Periph_GPIOD

#define LCD_DATA_PORT   	GPIOE
#define LCD_DATA_APB2Periph	RCC_APB2Periph_GPIOE 

/** 引脚控制宏定义 */
#define lcd_set_cs()  GPIO_SetBits(LCD_PORT_CS, LCD_Pin_CS)
#define lcd_clr_cs()  GPIO_ResetBits(LCD_PORT_CS, LCD_Pin_CS)

#define lcd_set_rs()  GPIO_SetBits(LCD_PORT_RS, LCD_Pin_RS)
#define lcd_clr_rs()  GPIO_ResetBits(LCD_PORT_RS, LCD_Pin_RS)

#define lcd_set_wr()  GPIO_SetBits(LCD_PORT_WR, LCD_Pin_WR)
#define lcd_clr_wr()  GPIO_ResetBits(LCD_PORT_WR, LCD_Pin_WR)

#define lcd_set_rd()  GPIO_SetBits(LCD_PORT_RD, LCD_Pin_RD)
#define lcd_clr_rd()  GPIO_ResetBits(LCD_PORT_RD, LCD_Pin_RD)

#define LCD_Write(LCD_DATA)  GPIO_Write(LCD_DATA_PORT, LCD_DATA)
#define LCD_Read()  GPIO_ReadInputData(LCD_DATA_PORT)


/*静态函数定义 */
static void lcd_pin_config(void);
static void lcd_write_reg(u16 reg,u16 value);
static u16 lcd_read_reg(u16 reg);
static void lcd_ram_prepare(void);
static u16 lcd_read_ram(void);
static void lcd_write_ram(u16 value);
static u16  lcd_bgr2rgb(u16 c);
static u16 lcd_read_sta(void);
static void lcd_data_as_input(void);
static void lcd_data_as_output(void);

/*行数定义 */
unsigned long const LCD_POW10[10] = 
{
  1 , 10, 100, 1000, 10000,
  100000, 1000000, 10000000, 100000000, 1000000000
};

/*LCD模块类型定义 0x9320：ili9320 0x8989：SSD1289 0x9325：ili9325*/
static u16 DeviceIdCode;

void lcd_delay(int n)
{
  int num;
  num = 100000*n; 
  while(--num);                 
}
/****************************************************************************
* 名    称：   lcd_config
* 功    能：  LCD初始化函数
* 入口参数：无       
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
void lcd_config(void)
{
  int trytimes=0;
	lcd_pin_config();	
	
	/** 控制引脚初始化 */
	lcd_set_cs();	
	lcd_set_wr();
	lcd_set_rd();
	lcd_set_rs();
	
	/** 开启内部振荡器 */
 	lcd_write_reg(0x0000,0x0001);		

        lcd_delay(10); 
        
        /** 读ID */
        DeviceIdCode = lcd_read_reg(0x0000);
        while (((0 == DeviceIdCode)||(1 == DeviceIdCode))&&trytimes<10)
        {
          lcd_delay(1);       
          DeviceIdCode = lcd_read_reg(0x0000);
        }
        //printf("DeviceIdCode = 0X%X \n", DeviceIdCode);
        if(DeviceIdCode==0x8989)//SSD1289
        {
          /** 执行上电流程 */
          lcd_write_reg(0x0003,0xA8A4);		
          lcd_write_reg(0x000C,0x0000);        
          lcd_write_reg(0x000D,0x080C);        
          lcd_write_reg(0x000E,0x2B00);        
          lcd_write_reg(0x001E,0x00B0);  
          
          lcd_delay(1); 
          
          /*
            * .13  0：RGB的值越小越亮；1：RGB的值越大越亮
            * .11  0：RGB；1: BGR
            * .9   0：319-->0；1: 0-->319
            * .14  0：719-->0；1: 0-->719
            * .0 ~.8  设置最大行号 (0x13f = 319)
          */
          lcd_write_reg(0x0001,0x293F);		
          
          lcd_write_reg(0x0002,0x0600);     	/**< LCD Driving Waveform control */
          lcd_write_reg(0x0010,0x0000);     	/**< .0  0: 关闭睡眠模式; 1: 打开睡眠模式 */
          lcd_write_reg(0x0011,0x6070);	        /**< .13-.14  11：16位RGB模式；10：18位RGB模式 */	
          lcd_write_reg(0x0016,0xEF1C);           /**< .15-.8   设置每行的像素数，0xef: 设为240 */
          lcd_write_reg(0x0017,0x0003);           /**< Vertical Porch */
          lcd_write_reg(0x0007,0x0233);           /**< Display Control */  
          lcd_write_reg(0x000B,0x0000);           /**< Frame Cycle Control */  
          lcd_write_reg(0x000F,0x0000);	        /**< Gate Scan Position */  
          lcd_write_reg(0x0041,0x0000);     	/**< Vertical Scroll Control */  
          lcd_write_reg(0x0042,0x0000);     	/**< Vertical Scroll Control */  
          lcd_write_reg(0x0048,0x0000);     	/**< Screen driving position */  
          lcd_write_reg(0x0049,0x013F);     	/**< Screen driving position */  
          lcd_write_reg(0x004A,0x0000);     	/**< Screen driving position */  
          lcd_write_reg(0x004B,0x0000);     	/**< Screen driving position */  
          lcd_write_reg(0x0044,0xEF00);     	/**< Horizontal RAM address position */  
          lcd_write_reg(0x0045,0x0000);           /**< Horizontal RAM address position */   
          lcd_write_reg(0x0046,0x013F);     	/**< Horizontal RAM address position */  
          lcd_write_reg(0x0030,0x0707);     	/**< Gamma Control */  
          lcd_write_reg(0x0031,0x0204);     	/**< Gamma Control */  
          lcd_write_reg(0x0032,0x0204);     	/**< Gamma Control */  
          lcd_write_reg(0x0033,0x0502);     	/**< Gamma Control */  
          lcd_write_reg(0x0034,0x0507);     	/**< Gamma Control */  
          lcd_write_reg(0x0035,0x0204);     	/**< Gamma Control */  
          lcd_write_reg(0x0036,0x0204);           /**< Gamma Control */   
          lcd_write_reg(0x0037,0x0502);           /**< Gamma Control */    
          lcd_write_reg(0x003A,0x0302);           /**< Gamma Control */    
          lcd_write_reg(0x003B,0x0302);           /**< Gamma Control */    
          lcd_write_reg(0x0023,0x0000);           /**< RAM write data mask */   
          lcd_write_reg(0x0024,0x0000);     	/**< RAM write data mask */  
          lcd_write_reg(0x0025,0x8000);     	/**< Frame Frequency Control; 0X8000: 对应屏幕响应频率为65Hz */  
          lcd_write_reg(0x004e,0);                /**< 列(X)首地址设置 */  
          lcd_write_reg(0x004f,0);                /**< 行(Y)首地址设置 */	
        }
        else if(DeviceIdCode==0x9325)//ili9325 
        {
          /* time control*/
          lcd_write_reg(0x00e3,0x3008);   // Set internal timing
          lcd_write_reg(0x00e7,0x0012);
          lcd_write_reg(0x00ef,0x1231);   //Set the internal vcore voltage
          /* display control*/
          lcd_write_reg(0x0001,0x0100);   // set SS and SM bit    
          lcd_write_reg(0x0002,0x0700);   // set 1 line inversion  				                 
          lcd_write_reg(0x0003,0x1030 );  // set GRAM write direction and BGR=1. 
          lcd_write_reg(0x0004,0x0000);   // Resize register                                  
          lcd_write_reg(0x0008,0x0207);   // set the back porch and front porch           
          lcd_write_reg(0x0009,0x0000);   // set non-display area refresh cycle ISC[3:0]       
          lcd_write_reg(0x000a,0x0000);   // FMARK function 
          /*RGB control*/
          lcd_write_reg(0x000c,0x0000);   // RGB interface setting    
          lcd_write_reg(0x000d,0x0000);   // Frame marker Position         
          lcd_write_reg(0x000f,0x0000);   // RGB interface polarity
          /*power control*/
          lcd_write_reg(0x0010,0x0000);   // SAP, BT[3:0], AP, DSTB, SLP, STB
          lcd_write_reg(0x0011,0x0007);   // DC1[2:0], DC0[2:0], VC[2:0] 
          lcd_write_reg(0x0012,0x0000);   // Internal reference voltage= Vci;               
          lcd_write_reg(0x0013,0x0000);   // Set VDV[4:0] for VCOM amplitude
          lcd_write_reg(0x0007,0x0000);   
          
          /*power control*/
          lcd_write_reg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB 
          lcd_write_reg(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0] 
          lcd_write_reg(0x0012, 0x009D); // Internal reference voltage= Vci; 

          lcd_write_reg(0x0013, 0x1900); // Set VDV[4:0] for VCOM amplitude 
          lcd_write_reg(0x0029,0x0025);   // Set VCM[5:0] for VCOMH 
          lcd_write_reg(0x002b,0x000d);   // Set Frame Rate and color
          
          lcd_write_reg(0x0020,0x0000);  // GRAM horizontal Address                                                          
          lcd_write_reg(0x0021,0x0000);  // GRAM Vertical Address          

          /*Adjust the Gamma Curve  */
          lcd_write_reg(0x0030,0x0000);   
          lcd_write_reg(0x0031,0x0506);   
          lcd_write_reg(0x0032,0x0104);   
          lcd_write_reg(0x0035,0x0207);   
          lcd_write_reg(0x0036,0x000f);   
          lcd_write_reg(0x0037,0x0306);   
          lcd_write_reg(0x0038,0x0102);    
          lcd_write_reg(0x0039,0x0707);   
          lcd_write_reg(0x003c,0x0702);   
          lcd_write_reg(0x003d,0x1604);  

          /*Set GRAM area */ 
          lcd_write_reg(0x0050,0x0000);        
          lcd_write_reg(0x0051,0x00ef);   
          lcd_write_reg(0x0052,0x0000);     
          lcd_write_reg(0x0053,0x013f);
          lcd_write_reg(0x0060,0xa700);        
          lcd_write_reg(0x0061,0x0001); 
          lcd_write_reg(0x006a,0x0000);
          /*Partial Display Control*/
          lcd_write_reg(0x0080,0x0000);
          lcd_write_reg(0x0081,0x0000);
          lcd_write_reg(0x0082,0x0000);
          lcd_write_reg(0x0083,0x0000);
          lcd_write_reg(0x0084,0x0000);
          lcd_write_reg(0x0085,0x0000);
          /*Panel Control */
          lcd_write_reg(0x0090,0x0010);     
          lcd_write_reg(0x0092,0x0600);  
          /*262K color and display ON */
          lcd_write_reg(0x0007,0x0133);
        }
        else if(DeviceIdCode==0x9320)//ili9320
        {
		lcd_write_reg(0x00,0x0000);
		lcd_write_reg(0x01,0x0100);	//Driver Output Contral.
		lcd_write_reg(0x02,0x0700);	//LCD Driver Waveform Contral.
		lcd_write_reg(0x03,0x1030);//Entry Mode Set.
		//lcd_write_reg(0x03,0x1018);	//Entry Mode Set.
	
		lcd_write_reg(0x04,0x0000);	//Scalling Contral.
		lcd_write_reg(0x08,0x0202);	//Display Contral 2.(0x0207)
		lcd_write_reg(0x09,0x0000);	//Display Contral 3.(0x0000)
		lcd_write_reg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		lcd_write_reg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		lcd_write_reg(0x0d,0x0000);	//Frame Maker Position.
		lcd_write_reg(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		lcd_delay(5); 
		lcd_write_reg(0x07,0x0101);	//Display Contral.
		lcd_delay(5); 								  
		lcd_write_reg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));//Power Control 1.(0x16b0)
		lcd_write_reg(0x11,0x0007);				//Power Control 2.(0x0001)
		lcd_write_reg(0x12,(1<<8)|(1<<4)|(0<<0));		//Power Control 3.(0x0138)
		lcd_write_reg(0x13,0x0b00);				//Power Control 4.
		lcd_write_reg(0x29,0x0000);				//Power Control 7.
	
		lcd_write_reg(0x2b,(1<<14)|(1<<4));	    
		lcd_write_reg(0x50,0);	//Set X Star
		//水平GRAM终止位置Set X End.
		lcd_write_reg(0x51,239);	//Set Y Star
		lcd_write_reg(0x52,0);	//Set Y End.t.
		lcd_write_reg(0x53,319);	//
	
		lcd_write_reg(0x60,0x2700);	//Driver Output Control.
		lcd_write_reg(0x61,0x0001);	//Driver Output Control.
		lcd_write_reg(0x6a,0x0000);	//Vertical Srcoll Control.
	
		lcd_write_reg(0x80,0x0000);	//Display Position? Partial Display 1.
		lcd_write_reg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		lcd_write_reg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		lcd_write_reg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		lcd_write_reg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		lcd_write_reg(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		lcd_write_reg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		lcd_write_reg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		lcd_write_reg(0x93,0x0001);	//Panel Interface Contral 3.
		lcd_write_reg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		lcd_write_reg(0x97,(0<<8));	//
		lcd_write_reg(0x98,0x0000);	//Frame Cycle Contral.	   
		lcd_write_reg(0x07,0x0173);	//(0x0173)
        }
}

/****************************************************************************
* 名    称：lcd_clr_screen
* 功    能：LCD清屏函数
* 入口参数：背景颜色数据
* 出口参数：无
* 说    明：
* 调用方法：lcd_clr_screen（ GREEN ） 参考头文件中颜色定义
****************************************************************************/
void lcd_clr_screen(u16 color)
{
	u32 index=0;
	lcd_set_cursor(0,0); 
	lcd_ram_prepare(); 	

	lcd_clr_cs();
	lcd_set_rs();

	for(index=0;index<76800;index++)        /** 320*240 = 76800 */
	{
		lcd_clr_wr();
		LCD_Write(color);
		lcd_set_wr();		
	}
	lcd_set_cs();   
}	

/****************************************************************************
* 名    称：lcd_set_cursor
* 功    能：设置鼠标位置
* 入口参数：x 行  y 列
* 出口参数：无
* 说    明：
* 调用方法：lcd_set_cursor（0，0） 设置鼠标位置为（0，0）
****************************************************************************/
void lcd_set_cursor(u16 x,u16 y)
{
  if(DeviceIdCode==0x8989)
  {
 	lcd_write_reg(0x004e,y);		
	lcd_write_reg(0x004f,x);	
  }	
  else if(DeviceIdCode==0x9325)
  {
 	lcd_write_reg(0x0020,y);		
	lcd_write_reg(0x0021,0x13f-x);	
  }	
  else 
  {
 	lcd_write_reg(0x0020,y);		
	lcd_write_reg(0x0021,0x13f-x);	
  }	
}

/****************************************************************************
* 名    称：lcd_set_windows
* 功    能：设置窗口起始点和结束点位置
* 入口参数：[in]窗口起始点和结束点位置
* 出口参数：无
* 说    明：
* 调用方法：lcd_set_windows（0，0，20，20）
****************************************************************************/
void lcd_set_windows(u16 start_x,u16 start_y,u16 end_x,u16 end_y)
{
	lcd_set_cursor(start_x, start_y);
	lcd_write_reg(0x0050, start_x);
	lcd_write_reg(0x0052, start_y);
	lcd_write_reg(0x0051, end_x);
	lcd_write_reg(0x0053, end_y);
}

/****************************************************************************
* 名    称：lcd_set_point
* 功    能：指定像素点赋值
* 入口参数： x 列 y 行 value 像素点处的颜色值
* 出口参数：无
* 说    明：
* 调用方法：lcd_set_point（0，0，GREEN）
****************************************************************************/
void lcd_set_point(u16 x, u16 y, u16 value)
{
	lcd_set_cursor(x, y);
	lcd_ram_prepare();
	lcd_write_ram(value);	
}

/****************************************************************************
* 名    称：u16 lcd_get_point(u16 x,u16 y)
* 功    能：获取指定座标的颜色值
* 入口参数：x      行座标
*           y      列座标
* 出口参数：当前座标颜色值
* 说    明：
* 调用方法：i=lcd_set_point(10,10);
****************************************************************************/
u16 lcd_get_point(u16 x,u16 y)
{
  lcd_set_cursor(x,y);
  return (lcd_bgr2rgb(lcd_read_ram()));
}

/**************************************************************************
* 名    称  : LCD_Pins_Config
* 功    能  : 配置LCD端口 推挽输出模式
* 入口参数    无
* 出口参数  : 无
* 说    明  : 无
* 调用方法  : 无
**************************************************************************/
static void lcd_pin_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(LCD_CLK_RS | LCD_DATA_APB2Periph |
                         LCD_CLK_WR | LCD_CLK_RD |
                         LCD_CLK_CS, ENABLE);

  // DB15--0
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);

  //LCD_Pin_WR
  GPIO_InitStructure.GPIO_Pin = LCD_Pin_WR;
  GPIO_Init(LCD_PORT_WR, &GPIO_InitStructure);

  //LCD_Pin_CS
  GPIO_InitStructure.GPIO_Pin = LCD_Pin_CS;
  GPIO_Init(LCD_PORT_CS, &GPIO_InitStructure);

  //LCD_Pin_RS
  GPIO_InitStructure.GPIO_Pin = LCD_Pin_RS;
  GPIO_Init(LCD_PORT_RS, &GPIO_InitStructure);

  //LCD_Pin_RD
  GPIO_InitStructure.GPIO_Pin = LCD_Pin_RD;
  GPIO_Init(LCD_PORT_RD, &GPIO_InitStructure);
}	

/****************************************************************************
* 名    称：lcd_write_reg（内部函数）
* 功    能：向指定寄存器写数据
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static void lcd_write_reg(u16 reg,u16 value)
{
	lcd_clr_cs();
	lcd_clr_rs();
	lcd_clr_wr();
	LCD_Write(reg);
	lcd_set_wr();

	lcd_set_rs();
	lcd_clr_wr();
	LCD_Write(value);
	lcd_set_wr();
	lcd_set_cs();
}

/****************************************************************************
* 名    称： lcd_read_reg （内部函数）
* 功    能：读寄存器
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
u16 lcd_read_reg(u16 reg)
{
      u16 data;
  /* Write 16-bit Index (then Read Reg) */
	lcd_clr_cs();
	lcd_clr_rs();
	lcd_clr_wr();
	LCD_Write(reg);
	lcd_set_wr();

  /* Read 16-bit Reg */
        lcd_data_as_input();
        lcd_delay(1);
	lcd_set_rs();
	lcd_clr_rd();
	lcd_set_rd();
	data = LCD_Read(); 
	lcd_set_cs();
         lcd_data_as_output();
        lcd_delay(1);
      return    data;
}

/****************************************************************************
* 名    称： lcd_readsta （内部函数）
* 功    能：读状态
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static u16 lcd_read_sta(void)
{
  u16 data;
  /* Write 16-bit Index, then Write Reg */
  lcd_set_rs();
  lcd_clr_rd() ;
  lcd_set_rd() ;
  data = LCD_Read(); 
  lcd_set_cs() ;    
  return    data;
}

/****************************************************************************
* 名    称：lcd_ram_prepare （内部函数）
* 功    能：准备读写RAM
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static void lcd_ram_prepare(void)
{
	lcd_clr_cs();
	lcd_clr_rs();
	lcd_clr_wr();
	LCD_Write(0x0022);
	lcd_set_wr();
	lcd_set_cs();
}

/****************************************************************************
* 名    称：lcd_write_ram （内部函数）
* 功    能：写数据
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static void lcd_write_ram(u16 value)
{
	lcd_clr_cs();
	lcd_set_rs();
	lcd_clr_wr();
	LCD_Write(value);
	lcd_set_wr();
	lcd_set_cs();
}

/****************************************************************************
* 名    称：lcd_data_as_input （内部函数）
* 功    能：data端口设为输入口
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static void lcd_data_as_input(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);
}

/****************************************************************************
* 名    lcd_data_as_output （内部函数）
* 功    能：data端口设为输出口
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static void lcd_data_as_output(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);
}

/****************************************************************************
* 名    称：lcd_read_ram （内部函数）
* 功    能：读RAM
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
static u16 lcd_read_ram(void)
{
	uint16_t dummy;
	uint16_t value;
	lcd_ram_prepare();
	lcd_data_as_input();
        lcd_delay(1);
	dummy = lcd_read_sta();
	dummy ++;
	value = lcd_read_sta();
	lcd_data_as_output();
	lcd_delay(1);
	return value;
}

/****************************************************************************
* 名    lcd_bgr2rgb （内部函数）
* 功    能：将像素点的BGR格式转换为RGB格式(BBBBBGGGGGGRRRRR -> RRRRRGGGGGGBBBBB)
* 入口参数：c BGR格式
* 出口参数：RGB格式
* 说    明：
* 调用方法：
****************************************************************************/
static u16 lcd_bgr2rgb(u16 c)
{
	u16  b, g, r, rgb;
	
	r = (c>>0)  & 0x1f;
	g = (c>>5)  & 0x3f;
	b = (c>>11) & 0x1f;
	
	rgb =  r << 11 + g << 5 + b << 0;
	
	return( rgb );
}


/*-----------------------------emWin 调用函数---------------------------------*/
/****************************************************************************
* 名    称：lcd_write_reg1，lcd_write_data，lcd_read_data_multiple
* 功    能： 在LCDConf.c文件中调用
****************************************************************************/
void lcd_write_reg1(u16 reg)
{
	lcd_clr_cs();
	lcd_clr_rs();
	lcd_clr_wr();
	LCD_Write(reg);
	lcd_set_wr();
	lcd_set_cs();
}

void lcd_write_data(uint16_t dat)
{
    lcd_write_ram(dat);
}

void lcd_read_data_multiple(uint16_t *p_dat, uint16_t num_items)
{
    lcd_data_as_input();
    lcd_clr_cs();
    lcd_set_rs();
    lcd_clr_rd(); 
    lcd_set_rd();
    while(num_items--)
    {
        *p_dat++ = LCD_Read(); 
    }
    lcd_set_cs();
    lcd_data_as_output();
}

u16 getLcdIdCode(void)
{
  return DeviceIdCode;
}

