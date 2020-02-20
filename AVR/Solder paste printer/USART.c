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

int USARTn;
uint8_t rx_head;
uint8_t rx_tail;
volatile uint8_t buffer_data[BUFFERSIZE + 1];





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
        USARTn = USART0
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


uint8_t RX_read()
{
    uint8_t data;
    uint8_t tail;
    tail = rx_tail;
    
    if (rx_head == rx_tail)
    {
        //no data
    }
    
    else
    {
        data = buffer_data[tail++]
    
        if(tail == BUFFERSIZE)
        {
            tail = 0;
        }
        
        else
        {
            return data;
            rx_tail = tail;
        }
    }
}




void RX_buffer()
{
    uint8_t rx_data;
    uint8_t head;
    
    //her skal realtime kommandoer plukkes opp
    
    rx_data = USARTn.RXDATAL;
    head = rx_head;
    
    if(rx_head == BUFFERSIZE)
    {
        head = 0;
    }
    
    if(rx_head != rx_tail)
    {
        buffer_data[head++] = rx_data;
        rx_head = head;
    }
}
void USB_TX(){
	
}