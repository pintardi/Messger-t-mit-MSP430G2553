/*
Nokia5110 pin description
PIN1	RST
PIN2	CE
PIN3	DC
PIN4	DIN
PIN5	CLK
PIN6	VCC
PIN7	BL(backlight)
PIN8	GND

*/
#ifndef __nokia_5110_h_
#define __nokia_5110_h_

#include "msp430.h"



#define LCD_5110_DIR            P2DIR
#define LCD_5110_OUT		P2OUT


#define   LCD_RST    4
#define   LCD_CE    5
#define   LCD_DC    3
#define   LCD_DIN    2
#define   LCD_CLK    1

void LCD_init(void);
void LCD_clear(void);
void LCD_LED_set(int i);
void LCD_write_english_string(unsigned char X,unsigned char Y,char *s);
void LCD_write_english_string_big(unsigned char X,unsigned char Y,char *s);
void LCD_write_english_string_big_even(unsigned char X,unsigned char Y,char *s);
void LCD_write_english_string_big_odd(unsigned char X,unsigned char Y,char *s);
void LCD_write_char(unsigned char c);
void LCD_write_char_big_even(unsigned char c);
void LCD_write_char_big_odd(unsigned char c);
void LCD_draw_bmp_pixel(unsigned char X,unsigned char Y,unsigned char *map,
                  unsigned char Pix_x,unsigned char Pix_y);
void LCD_write_byte(unsigned char dat, unsigned char dc);
void delay_1us(void);                 

#endif
