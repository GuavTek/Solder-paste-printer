/*
 * USART.c
 *
 * Created: 05-Feb-20 11:44:00
 *  Author: mikda
 */ 


#include <stdint.h>
#include <avr/io.h>
#include "Header.h"

// Temporary, counts steps to move
int move = 0;

void USART_INIT(uint8_t portnum, uint32_t baudrate){
	
	//Division needed to get desired baudrate
	uint16_t baudDiv = (4 * fCLK_PER / baudrate);
	
	//Enable interrupts RX/TX complete
	uint8_t RA = /* USART_TXCIE_bm | */ USART_RXCIE_bm;
	
	//Enable tx and rx
	uint8_t RB = USART_RXEN_bm|USART_TXEN_bm;
	
	//Set no parity, 8 data-bits
	uint8_t RC = (0x0 << 4)|(0x3);
	
	switch (portnum)
	{
	case 0:
		USART0.BAUD = baudDiv;
		USART0.CTRLA = RA;
		USART0.CTRLC = RC;
		PORTA.DIRSET = 1 << 0;		//PA0 is output
		USART0.CTRLB = RB;
		break;
	case 1:
		USART1.BAUD = baudDiv;
		USART1.CTRLA = RA;
		USART1.CTRLC = RC;
		PORTC.DIRSET = 1 << 0;		//PC0 is output
		USART1.CTRLB = RB;
		break;
	case 2:
		USART2.BAUD = baudDiv;
		USART2.CTRLA = RA;
		USART2.CTRLC = RC;
		PORTF.DIRSET = 1 << 0;		//PF0 is output
		USART2.CTRLB = RB;
		break;
	case 3:
		USART3.BAUD = baudDiv;
		USART3.CTRLA = RA;
		USART3.CTRLC = RC;
		PORTB.DIRSET = 1 << 0;		//PB0 is output
		USART3.CTRLB = RB;
		break;
	}
}

void USB_RX(){
	char hasGet = USART3.RXDATAL;
	if (hasGet == 'b')
	{
		move += 100;
		PORTF.OUT |= 1 << 5;
	} else if(hasGet == 'a'){
		move -= 100;
		PORTF.OUT &= ~(1 << 5);
	}
	USART3.TXDATAL = hasGet;
}

void USB_TX(){
	
}