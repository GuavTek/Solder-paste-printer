/*
 * Solder paste printer.c
 *
 * Created: 05-Feb-20 11:27:05
 * Author : mikda
 */ 

#include "Header.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MStep 4		//PD4
#define MDir 2		//PC2


int main(void)
{
	USART_INIT(3, 9600);
	//Set clk_per prescaler not working?
//	CLKCTRL.MCLKCTRLB = (PDIV << 1)|(1 << 0);
	PORTF.DIRSET = 1 << 5;	//LED
	PORTC.DIRSET = 1 << MDir;
	PORTD.DIRSET = 1 << MStep;
	
	sei();
    /* Replace with your application code */
    while (1) 
    {
		_delay_ms(1000);
		PORTF.OUTTGL = PIN5_bm;
    }
}

ISR(USART3_RXC_vect){
	ParseStream();
}

/*
ISR(USART3_TXC_vect){
	
}
*/
