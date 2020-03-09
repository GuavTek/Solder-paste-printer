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

void print(void);

uint16_t timer = 0;

int main(void)
{
	USART_INIT(3, 9600);
    stepper_TCB_init();
	USARTn.TXDATAL = 'o';
	//Set clk_per prescaler not working?
//	CLKCTRL.MCLKCTRLB = (PDIV << 1)|(1 << 0);
	PORTF.DIRSET = 1 << 5;	//LED
	PORTC.DIRSET = 1 << MDir;
	PORTD.DIRSET = 1 << MStep;
	
	sei();
    
    while (1) 
    {
		if(RX_available() != BUFFER_EMPTY){
			PORTF.OUTTGL = PIN5_bm;
			char tempChar = RX_read();
			if (tempChar == '%')
			{
				print();
			}
		}
		PrepStep();
		timer++;
		
		if (timer == 0)
		{
			PORTF.OUTTGL = PIN5_bm;
		}
		
    }
}


//Main loop when printing
void print(void) {
	InitParser();
	TX_write('k');
	
	PORTF.OUTSET = PIN5_bm;
	
	while(1){
		ParseStream();
		
		//Jumpstart TX if there is data and is not currently sending
		if ((TX_available() != BUFFER_EMPTY) && !(USARTn.CTRLA & USART_TXCIE_bm))
		{
			PORTF.OUTCLR = PIN5_bm;
			TX_read();
			USARTn.CTRLA |= USART_TXCIE_bm;
		}
		
		PORTF.OUTSET = PIN5_bm;
		/*
		if(ABORT){
			flushRX;
			return;
		}
		*/
	}
}

ISR(USART3_RXC_vect){
	RX_write();
	
}


ISR(USART3_TXC_vect){
	if (TX_available() != BUFFER_EMPTY)
	{
		TX_read();
	} else {
		//Disable interrupt if there is no data to send
		USARTn.CTRLA &= ~USART_TXCIE_bm;
	}
}

