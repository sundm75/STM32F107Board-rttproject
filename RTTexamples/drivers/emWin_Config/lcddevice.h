/*******************************2012-2013, NJUT, Edu.************************** 
FileName: lcddevice.h 
Author:  孙冬梅       Version :  1.0        Date: 2013.12.30
Description:    lcd驱动  头文件    
Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    13/12/30     1.0     文件创建   
******************************************************************************/ 

#ifndef __LCD_H
#define __LCD_H 

#include <stm32f10x.h>
#include "stdio.h"

#define WHITE          0xFFFF
#define BLACK          0x0000
#define GREY           0xF7DE
#define BLUE           0x001F
#define BLUE2          0x051F
#define RED            0xF800
#define MAGENTA        0xF81F		 /**< 品红 */
#define GREEN          0x07E0
#define CYAN           0x7FFF		 /**< 青色 */
#define YELLOW         0xFFE0
#define HYALINE        0x0001		 /**< 透明 */


/*常用LCD操作函数*/
void lcd_test(void);
void lcd_config(void);
void lcd_clr(uint16_t color);
void lcd_set_cursor(uint16_t x,uint16_t y);	 
void lcd_set_windows(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y);
uint16_t lcd_get_point(uint16_t x,uint16_t y);
void lcd_set_point(uint16_t x, uint16_t y, uint16_t value);
void lcd_draw_picture(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint16_t *picture);
void lcd_putchar_8x16(uint16_t x, uint16_t y, uint8_t ch, uint16_t ch_color, uint16_t bk_color);
void lcd_putchar_16x24(uint16_t x, uint16_t y, uint8_t ch, uint16_t ch_color, uint16_t bk_color); 
	
/*emWin 调用函数*/
void lcd_write_reg1(uint16_t dat);
void lcd_write_data(uint16_t dat);
void lcd_read_data_multiple(uint16_t *p_dat, uint16_t num_items);
u16 getLcdIdCode(void);

#endif
 
