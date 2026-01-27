#include <MSP430.h> //Register des Prozessors
#include "ST7735.h"


volatile unsigned int __attribute__((section(".infoD"))) flashData;

int main(void) {
//	unsigned char * flashB=(unsigned char*)0x1800; //Use information B
//	unsigned char flashDataCopy=*flashB;	//Read flash value
	  WDTCTL = WDTPW | WDTHOLD;

	char txt[]="000";
    txt[0]+=2;

	ST7735_interface_init();
	ST7735_display_init();
    drawTextLine(0, 0, txt, 0xFF00FFL,0x000000L);
    while(1){}
}