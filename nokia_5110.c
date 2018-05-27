

#include "nokia_5110.h"
#include "char_ext_6x8.h"
#include "char_ext_hl_6x8.h"






void delay_1us(void)
{
   unsigned int i;
  for(i=0;i<100;i++);
   
}

void delay_1ms(void)
  {
   unsigned int i;
   for (i=0;i<1140;i++);
}
  
void delay_nms(unsigned int n)
  {
   unsigned int i=0;
   for (i=0;i<n;i++)
   delay_1ms();
  }

void LCD_LED_set(int i){
    LCD_5110_DIR |= 0x01;
    if(i == 1)
    {
        //turn on LED
        LCD_5110_OUT |= 0x01;
    }
    if(i == 0){
        //turn off LED
        LCD_5110_OUT =~ 0x01;
    }
}


void LCD_init(void)
  {

  // LCD_RST = 0;
	
	LCD_5110_DIR |= (0x01 << LCD_RST) + (0x01 << LCD_CE) + (0x01 << LCD_DC) 
	  				+ (0x01 << LCD_DIN) + (0x01<< LCD_CLK);
	
   LCD_5110_OUT &= ~(0x01 << LCD_RST);
    delay_1us();

  // LCD_RST = 1;
   LCD_5110_OUT |= (0x01 << LCD_RST);
    

   //LCD_CE = 0;
    LCD_5110_OUT &= ~(0x01 << LCD_CE);
    delay_1us();

   //LCD_CE = 1;
   LCD_5110_OUT |= (0x01 << LCD_CE);
    delay_1us();

    LCD_write_byte(0x21, 0);
    LCD_write_byte(0xc8, 0);
    LCD_write_byte(0x06, 0);
    LCD_write_byte(0x13, 0);
    LCD_write_byte(0x20, 0);
    LCD_clear();
    LCD_write_byte(0x0c, 0);
        

  // LCD_CE = 0;
	 LCD_5110_OUT &= ~(0x01 << LCD_CE);
  }


void LCD_clear(void)
  {
    unsigned int i;

    LCD_write_byte(0x0c, 0);			
    LCD_write_byte(0x80, 0);			

    for (i=0; i<504; i++)
      LCD_write_byte(0, 1);			
  }


void LCD_set_XY(unsigned char X, unsigned char Y)
  {
    LCD_write_byte(0x40 | Y, 0);		// column
    LCD_write_byte(0x80 | X, 0);          	// row
  }



void LCD_write_char(unsigned char c)
  {
    unsigned char line;

    c -= 32;

    for (line=1; line<7; line++)
      LCD_write_byte(char_mod_6x8[c][line], 1);
  }

void LCD_write_char_hl(unsigned char c)
  {
    unsigned char line;

    c -= 32;

    for (line=1; line<7; line++)
      LCD_write_byte(char_mod_hl_6x8[c][line], 1);
  }

void LCD_write_string(unsigned char X,unsigned char Y,char *s)
  {
    LCD_set_XY(X,Y);
    while (*s)
      {
     LCD_write_char(*s);
     s++;
      }
  }

void LCD_write_string_hl(unsigned char X,unsigned char Y,char *s)
  {
    LCD_set_XY(X,Y);
    while (*s)
      {
     LCD_write_char_hl(*s);
     s++;
      }
  }

void LCD_write_byte(unsigned char dat, unsigned char command)
  {
    unsigned char i;
    //PORTB &= ~LCD_CE ;		        // avr
    //LCD_CE = 0;						// 51
	LCD_5110_OUT &= ~(0x01 << LCD_CE);	// msp430
    
    if (command == 0)
     // PORTB &= ~LCD_DC ;
	//     LCD_DC = 0;
	  LCD_5110_OUT &= ~(0x01 << LCD_DC);
    else
     // PORTB |= LCD_DC ;
    // LCD_DC = 1;
	 LCD_5110_OUT |= (0x01 << LCD_DC);
		for(i=0;i<8;i++)
		{
			if(dat&0x80)
				//SDIN = 1;
			   LCD_5110_OUT |= (0x01 << LCD_DIN);
			    //LCD_5110_OUT &= ~(0x01 << LCD_DIN);

			else
				//SDIN = 0;
			   LCD_5110_OUT &= ~(0x01 << LCD_DIN);
			    //LCD_5110_OUT |= (0x01 << LCD_DIN);

			//SCLK = 0;
			LCD_5110_OUT &= ~(0x01 << LCD_CLK);
			dat = dat << 1;
			//SCLK = 1;
			LCD_5110_OUT |= (0x01 << LCD_CLK);
		}

	 LCD_5110_OUT |= (0x01 << LCD_CE);
  }


