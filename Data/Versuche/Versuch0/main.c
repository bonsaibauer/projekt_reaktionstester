#include <msp430.h>

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR |= 0x01;
	P4DIR |= 0x80;
	while (1){
		P4OUT ^= 0x80;
		P1OUT ^= 0x01;
		int i;for (i = 0; i < 20000; i++);
	}
}