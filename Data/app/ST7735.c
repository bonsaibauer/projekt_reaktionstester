/*
 * ST7735_primitives.c
 *
 *      Author: JJ Hastings WG0Z
 *      Modified to use UCB0 instead of Bit-Banging
 */

#include "msp430.h"

#include "ST7735.h"


// data[] is used by draw()
unsigned char data[4];
unsigned char data2[4];

static const unsigned char WIDTH = 7;
static const unsigned char HEIGHT = 12;

void sendByte(unsigned char item) {
	while (!(UCB0IFG & UCTXIFG))
		;
	UCB0TXBUF = item;
	while (UCB0STAT & UCBUSY)
		;
}

void ST7735_command(unsigned char cmd) {
	COMMAND;
	SELECT_L;
	sendByte(cmd);
	DESELECT;
}

void ST7735_send(unsigned char * pValues, unsigned char cnt) {
	do {
		sendByte(*pValues);
		pValues++;
		cnt--;
	} while (cnt);
}

void ST7735_data(unsigned char * pValues, unsigned char cnt) {
	DATA;
	SELECT_L;
	do {
		sendByte(*pValues);
		pValues++;
		cnt--;
	} while (cnt);
	DESELECT;
}

// FUNCTIONS BELOW ARE 'PUBLIC' and
// defined in ST7735_prototypes.h

void pause(void) {
	// for MSP430 devices running at teh 'default' of about 1 Mhz,
	// 30000 for ix yields about a quarter-second of delay.
	volatile unsigned ix = 30000;
	do {
		ix--;
	} while (0 != ix);
}

void ST7735_interface_init(void) {
	UCSCTL1 =DCORSEL_7;//Speedup SMCLK
	RESET_DIR;
	CS_DIR;
	A0_DIR;
	LIGHT_DIR;

	UCB0_DIR |= SCLK_BIT_MASK | SDA_BIT_MASK;
	UCB0_SELECT |= SCLK_BIT_MASK | SDA_BIT_MASK;
	UCB0CTL1 = UCSWRST;  // Hold USCI in SW reset mode while configuring
	UCB0CTL0 = UCMST + UCSYNC + UCCKPL + UCMSB;    // 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2;                          // SMCLK
	UCB0BR0 = 0;
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;                          // Release USCI state machine
	UCB0IFG &= ~UCRXIFG;

	RESET_L;
	DESELECT;
	COMMAND;
	LIGHT_ON;
	pause();
	RUN;
	pause();

}

void ST7735_display_init(void) {
	ST7735_command( SW_RESET);
	pause();
	ST7735_command( SLEEP_OUT);
	pause();
	ST7735_command( DISPLAY_ON);
}

void setXYChar(unsigned char x, unsigned char y) {
	unsigned char dataXY[4];
	dataXY[0] = 0;
	dataXY[2] = 0;

	dataXY[1] = 129-x - WIDTH + 1;  //
	dataXY[3] = 129-x;

	ST7735_command( CA_SET);
	ST7735_data(dataXY, 4);

	dataXY[1] = 128-y-HEIGHT + 1;
	dataXY[3] = 128-y;
	ST7735_command( RA_SET);
	ST7735_data(dataXY, 4);
}

void writeChar(unsigned char c) {
	const unsigned char *fon = font7x12[c];

	ST7735_command( RAM_WRITE);
	DATA;
	SELECT_L;
	for (unsigned char j = HEIGHT - 1; j != 0xFF; j--) {
		unsigned char l = fon[j];
		for (unsigned char i = WIDTH; i > 0; i--) {
			if (l & (1 << i))
				ST7735_send(data, 3);
			else
				ST7735_send(data2, 3);
		}
	}
	DESELECT;
}

void drawTextLine(unsigned char line,unsigned char pos,char text[],
		unsigned long ColorFront, long ColorBack){

	if(pos>0)
		draw(0, line*HEIGHT, pos*WIDTH,HEIGHT, ColorBack);

	data[0] = (unsigned char) (ColorFront >> 16);
	data[1] = (unsigned char) (ColorFront >> 8);
	data[2] = (unsigned char) ColorFront;

	data2[0] = (unsigned char) (ColorBack >> 16);
	data2[1] = (unsigned char) (ColorBack >> 8);
	data2[2] = (unsigned char) ColorBack;


	unsigned char i;
	for(i=0;text[i] != '\0';i++){
		setXYChar((i+pos)*WIDTH, line*HEIGHT);
		writeChar(text[i]);
	}

	draw((i+pos)*WIDTH, line*HEIGHT, 129-(i+pos)*WIDTH,HEIGHT, ColorBack);
}

void setText(unsigned char x, unsigned char y, char text[],
		unsigned long ColorFront, long ColorBack) {

	data[0] = (unsigned char) (ColorFront >> 16);
	data[1] = (unsigned char) (ColorFront >> 8);
	data[2] = (unsigned char) ColorFront;

	data2[0] = (unsigned char) (ColorBack >> 16);
	data2[1] = (unsigned char) (ColorBack >> 8);
	data2[2] = (unsigned char) ColorBack;

	for(int i=0;text[i] != '\0';i++){
		setXYChar(x+i*WIDTH, y);
		writeChar(text[i]);
	}
	ST7735_command( NOP);
}

void draw(unsigned char x, unsigned char y, unsigned char width,
		unsigned char height, unsigned long Color) {
	unsigned num_pixels = width * height;

	data[0] = 0;
	data[2] = 0;

	data[1] = 129-x - width + 1;  //
	data[3] = 129-x;
	ST7735_command( CA_SET);
	ST7735_data(data, 4);

	data[1] = 128-y-height + 1;
	data[3] = 128-y;
	ST7735_command( RA_SET);
	ST7735_data(data, 4);

	ST7735_command( RAM_WRITE);
	data[0] = (unsigned char) (Color >> 16);
	data[1] = (unsigned char) (Color >> 8);
	data[2] = (unsigned char) Color;

	DATA;
	SELECT_L;
	do {
		ST7735_send(data, 3);
		num_pixels--;
	} while (0 != num_pixels);
	DESELECT;
	ST7735_command( NOP);
}

