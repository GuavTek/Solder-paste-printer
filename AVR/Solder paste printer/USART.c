/*
 * USART.c
 *
 * Created: 05-Feb-20 11:44:00
 *  Author: mikda
 */ 



#include "Header.h"


volatile uint8_t rx_head = 0;
uint8_t rx_tail = 0;
uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
uint8_t rx_buffer_data[RX_BUFFERSIZE];
uint8_t tx_buffer_data[TX_BUFFERSIZE];

bool RX_Full = false;
bool prevRX_Full = false;

void USART_INIT(uint8_t portnum, uint32_t baudrate){
	
	//Division needed to get desired baudrate
	uint16_t baudDiv = (4 * fCLK_PER / baudrate);
	
	//Enable interrupts RX/TX complete
	uint8_t RA = USART_DREIE_bm | USART_RXCIE_bm;
	
	//Enable tx and rx
	uint8_t RB = USART_RXEN_bm|USART_TXEN_bm;
	
	//Set no parity, 8 data-bits
	uint8_t RC = USART_PMODE_DISABLED_gc|(0x3);
	
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

uint8_t RX_read()
{
    uint8_t data;
    uint8_t tail;
    tail = rx_tail + 1;
	
	//No data
	if (rx_head == rx_tail)
	{
		return 0;
	}
	
	if(tail == RX_BUFFERSIZE)
	{
		tail = 0;
	}
    
    data = rx_buffer_data[tail];
    rx_tail = tail;
		
	// Signal that there is space in buffer
	if (RX_Count() > 20)
	{
		RX_Full = false;
	}
        
	return data;
}

void RX_write()
{
    uint8_t rx_data;
    uint8_t head;
    
    rx_data = USARTn.RXDATAL;
    
    switch (rx_data)
    {
        case CMD_RESET: {
			//Performs a hard software-reset
			ccp_write_io((void*)&RSTCTRL.SWRR, RSTCTRL_SWRE_bm);
			return;
		}
        case CMD_STATUS_REPORT: {
			currentState.statusDump = true;
			return;
		}
        case CMD_CYCLE_START: {
			currentState.noError = true;
			break;
		}
		case CMD_ABORT: {
			currentState.abortPrint = true;
			return;
		}
        default:
            if(rx_data > 0x7F)
            {
                switch(rx_data); // pick up realtime commands
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
        			rx_buffer_data[head] = rx_data;
        			rx_head = head;
    			} else {
					ReportEvent(BUFFER_OVERFLOW, 'R');
				}
	
				// Signal that buffer is full (soon)
				if (RX_Count() < 10)
				{
					RX_Full = true;
				}
            }
    }

    
}

// Returns available buffer space
uint8_t RX_Count(){
	int16_t cnt = rx_tail - rx_head;
	
	if (cnt < 0)
	{
		cnt += RX_BUFFERSIZE;
	}
	return cnt;
}

ReturnCodes RX_available(){
	if (rx_head == rx_tail)
	{
		return BUFFER_EMPTY;
	}
	
	uint8_t head = rx_head + 1;
	if (head == RX_BUFFERSIZE)
	{
		head = 0;
	}
	
	if (head == rx_tail)
	{
		return BUFFER_FULL;
	}
	return BUFFER_AVAILABLE;
}

void TX_write(uint8_t data)
{
    uint8_t head = tx_head + 1;
    
    if(head == TX_BUFFERSIZE)
    {  
        head = 0;
    }
    
    tx_buffer_data[head] = data;
    tx_head = head;
	USARTn.CTRLA |= USART_DREIE_bm;
}

void TX_read()
{
	
	//Return if transmit is not actually ready
	if (!(USARTn.STATUS & USART_DREIF_bm))
	{
		return;
	}
	
	// The state of the RX buffer has high priority
	if (RX_Full != prevRX_Full)
	{
		if (RX_Full)
		{
			USARTn.TXDATAL = 'f';
		} 
		else
		{
			USARTn.TXDATAL = 'a';
		}
		
		prevRX_Full = RX_Full;
	} else {
	    uint8_t tail = tx_tail + 1;
	 
	    if (tail >= TX_BUFFERSIZE)
		{
			tail = 0;
	    }
		
		USARTn.TXDATAL = tx_buffer_data[tail];
		tx_tail = tail;
	}
}

ReturnCodes TX_available(){
	
	if (RX_Full != prevRX_Full)
	{
		return BUFFER_AVAILABLE;
	}
	
	if (tx_head == tx_tail)
	{
		return BUFFER_EMPTY;
	}
	
	uint8_t head = tx_head + 1;
	if (head >= TX_BUFFERSIZE)
	{
		head = 0;
	}
	
	if (head == tx_tail)
	{
		return BUFFER_FULL;
	}
	return BUFFER_AVAILABLE;
}

void RTX_FLUSH(){
	rx_head = 0;
	tx_head = 0;
	rx_tail = 0;
	tx_tail = 0;
	USARTn.RXDATAL;
	USARTn.RXDATAL;
}

void TX_Jumpstart(){
	if ((TX_available() != BUFFER_EMPTY) && !(USARTn.CTRLA & USART_DREIE_bm))
	{
		//TX_read();
		USARTn.CTRLA |= USART_DREIE_bm;
	}
}

ISR(USART3_RXC_vect){
	RX_write();
	
}

ISR(USART3_DRE_vect){
	if (TX_available() != BUFFER_EMPTY)
	{
		TX_read();
	} else {
		//Disable interrupt if there is no data to send
		USARTn.CTRLA &= ~USART_DREIE_bm;
	}
	//USARTn.STATUS 
}

