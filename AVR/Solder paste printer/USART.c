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
uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;
uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
uint8_t rx_buffer_data[RX_BUFFERSIZE];
uint8_t tx_buffer_data[TX_BUFFERSIZE];




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
    
    if (rx_head == tail)
    {
        return RX_NO_DATA;
    }
    else
    {
        data = rx_buffer_data[tail++]
    
        if(tail == RX_BUFFERSIZE)
        {
            tail = 0;
        }
        
        return data;
        rx_tail = tail;
        
    }
}




void RX_buffer()
{
    uint8_t rx_data;
    uint8_t head;
    
    rx_data = USARTn.RXDATAL;
    
    switch (data);
    {
        case CMD_RESET:     /*insert reset routine break*/ break;
        case CMD_STATUS_REPORT: /* insert status report routine*/ break; // Set as true
        case CMD_CYCLE_START:   /* insert system exucuiton status*/ break; // Set as true
        default:
            if(data > 0x7F)
            {
                switch(data); // pick up realtime commands
                {
                    
                }
            }
            else // data that does not contain realtime or system commands
            {
                
                head = rx_head + 1;
    
                if(rx_head == RX_BUFFERSIZE)
                {
                    head = 0;
                }
    
                if(head != rx_tail)
                {
                    rx_buffer_data[rx_head] = rx_data;
                    rx_head = head;
                }
            }
        }
    }
}

void TX_receive(uint8_t data)
{
    uint8_t head;
    head = tx_head;
    
    if(head == TX_BUFFERSIZE)
    {  
        head = 0;
    }
     //if (tx_head == tx_tail)
    //{
         //if tx is busy writing
    //}
    
    tx_buffer_data[head++] = data;
    tx_head = head;
}


void TX_buffer()
{
    uint8_t tail;
    tail = tx_tail;
	
    USARTn.TXDATAL = tx_buffer_data[tail++];
    
    if (tx_tail == TX_BUFFERSIZE)
    {
        tail = 0;
    }
  
    tail = tx_tail;
}