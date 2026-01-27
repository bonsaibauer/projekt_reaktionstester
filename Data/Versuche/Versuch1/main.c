#include <msp430.h>

void main(void)
{
	/* Init */
	WDTCTL = WDTPW + WDTHOLD; /* Watchdog aus! */

	while(1){
	}
}
