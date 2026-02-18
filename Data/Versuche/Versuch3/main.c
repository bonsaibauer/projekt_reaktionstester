#include <MSP430.h> //Register des Prozessors


void main(void){
	WDTCTL = WDTPW + WDTHOLD; /* Watchdog aus! */


	// Globale Interruptfreigabe + Energiesparen
	__bis_SR_register( GIE);
	while(1){
	}
}


__attribute__((interrupt(WDT_VECTOR)))
void WDT_VECTOR_ISR(void){
}
