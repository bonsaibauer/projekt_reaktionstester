/*
 * ST7735.h
 * Author: R.Heintz
 */

#ifndef ST7735_H_
#define ST7735_H_

#include "font.h"

//P7.4 J2.17 LCD reset pin
//P2.6 J2.13 LCD SPI chip select
//P8.2 J4.31 LCD register select pin (Command oder Data)
//P3.2 J1.7  LCD SPI clock
//P3.0 J2.15 LCD SPI MOSI (UCB0SIMO/UCB0SDA=>USCI B0)

//P2.4 J4.39 LCD backlight

#define RESET_DIR   P7DIR |= BIT4
#define RESET_L     P7OUT &= ~BIT4
#define RUN         P7OUT |= BIT4

#define CS_DIR   	P2DIR |= BIT6
#define SELECT_L    P2OUT &= ~BIT6
#define DESELECT    P2OUT |= BIT6

#define A0_DIR  	P8DIR |= BIT2
#define DATA        P8OUT |= BIT2
#define COMMAND     P8OUT &= ~BIT2

#define LIGHT_DIR   P2DIR|= BIT4
#define LIGHT_ON  	P2OUT |= BIT4

#define UCB0_SELECT  P3SEL
#define UCB0_DIR P3DIR
#define SCLK_BIT_MASK BIT2
#define SDA_BIT_MASK  BIT0

#define NOP         0x00
#define SW_RESET    0x01
#define SLEEP_OUT   0x11
#define DISPLAY_ON  0x29
#define CA_SET      0x2A
#define RA_SET      0x2B
#define RAM_WRITE   0x2C

void pause( void );
void ST7735_interface_init( void );
void ST7735_display_init( void );
void draw( unsigned char x, unsigned char y,
           unsigned char width, unsigned char height,
           unsigned long Color);

void setText( unsigned char x, unsigned char y,
		char String[],unsigned long ColorFront,long ColorBack);

void drawTextLine(unsigned char line,unsigned char pos,char text[],
		unsigned long ColorFront, long ColorBack);
#endif /* ST7735_H_ */
