/*
 * Solder paste printer.c
 *
 * Created: 05-Feb-20 11:27:05
 * Author : mikda
 */ 

#include "Header.h"


void Print(void);

void InitEndSensors();

void Blinky(void);

uint16_t timer = 0;

int main(void)
{
	USART_INIT(3, 9600);
	USARTn.TXDATAL = 'o';
	InitEndSensors();
	InitClock();
	//Set clk_per prescaler not working?
//	CLKCTRL.MCLKCTRLB = (PDIV << 1)|(1 << 0);
	PORTF.DIRSET = 1 << 5;	//Onboard LED
	PORTF.OUTSET = PIN5_bm;
	
	sei();
	Blinky();
	
    
    while (1) 
    {
		if(RX_available() != BUFFER_EMPTY){
			PORTF.OUTTGL = PIN5_bm;
			char tempChar = RX_read();
			if (tempChar == '%')
			{
				Print();
			}
		}
		
		/*
		timer++;
		if (timer == 0)
		{
			PORTF.OUTTGL = PIN5_bm;
		}
		*/
    }
}

void Blinky(){
	PORTF.OUTTGL = PIN5_bm;
	StartTimer(1000, Blinky);
}


//Main loop when printing
void Print(void) {
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

void InitEndSensors(){
	//To do: find triggered level (PORT_INVEN if high)
	PORTC.PIN5CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN1CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN2CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN5CTRL = PORT_ISC_LEVEL_gc;
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

ISR(PORTC_PORT_vect){
	if (PORTC.INTFLAGS & PIN5_bm)
	{
		//Y-axis end detected
		//Stop Y motion
		//Step 1 position back/or until level goes back
		//Log unexpected trigger if current task != home
		PORTC.INTFLAGS |= PIN5_bm;
	}
}

ISR(PORTD_PORT_vect){
	if (PORTD.INTFLAGS & PIN2_bm)
	{
		//X-axis end detected
		
		PORTD.INTFLAGS |= PIN2_bm;
	}
	if (PORTD.INTFLAGS & PIN5_bm)
	{
		//Z-axis end detected
		
		PORTD.INTFLAGS |= PIN5_bm;
	}
	if (PORTD.INTFLAGS & PIN1_bm)
	{
		//idk
		
		PORTD.INTFLAGS |= PIN1_bm;
	}
}
