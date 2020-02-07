/*
 * Solder paste printer.c
 *
 * Created: 05-Feb-20 11:27:05
 * Author : mikda
 */ 

#include "GlobalVars.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "USART.h"

#define MStep 4		//PD4
#define MDir 2		//PC2


int main(void)
{
	USART_INIT(3, 19200);
	//Set clk_per prescaler not working?
//	CLKCTRL.MCLKCTRLB = (PDIV << 1)|(1 << 0);
	PORTF.DIRSET = 1 << 5;	//LED
	PORTC.DIRSET = 1 << MDir;
	PORTD.DIRSET = 1 << MStep;
	
	sei();
    /* Replace with your application code */
    while (1) 
    {
		_delay_us(800);
		if(move != 0){
			if(move > 0){
				PORTC.OUT |= 1 << MDir;
				move--;
			} else{
				PORTC.OUT &= ~(1 << MDir);
				move++;
			}
			PORTD.OUT |= 1 << MStep;
		}
		_delay_us(800);
		PORTD.OUT &= ~(1 << MStep);
		
    }
}

ISR(USART3_RXC_vect){
	USB_RX();
}

/*
ISR(USART3_TXC_vect){
	USB_TX();
}
*/
