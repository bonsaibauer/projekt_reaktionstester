#include <msp430.h>

void main(void){
	WDTCTL = WDTPW + ????;


	while(1){
		for(int i=0; i<200; i++);
	}
}
