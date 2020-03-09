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

uint16_t timer = 1000;

int main(void)
{
	USART_INIT(3, 9600);
    stepper_TCB_init();
	InitEndSensors();
	InitClock();
	PORTF.DIRSET = PIN5_bm;	//Onboard LED
	sei();
	Blinky();
	USARTn.TXDATAL = 'o';
	currentState.state = idle;
    currentState.running = true;
    while (1) 
    {
		//Wait for start-character
		if(RX_available() != BUFFER_EMPTY){
			PORTF.OUTTGL = PIN5_bm;
			if (RX_read() == START_CHAR)
			{
				Print();
				timer = 1000;
				currentState.state = idle;
			}
		}
		
		TX_Jumpstart();

    }
}

void Blinky(){
	PORTF.OUTTGL = PIN5_bm;
	StartTimer(timer, Blinky);
}


//Main loop when printing
void Print(void) {
	InitParser();
	TX_write('k');
	currentState.state = printing;
	currentState.abortPrint = false;
	timer = 200;
	PORTF.OUTSET = PIN5_bm;
	
	while(1){
		if (currentState.running)
		{
			ParseStream();
			PrepStep();
		} 
		else
		{
			//Error state
			
		}
		
		TX_Jumpstart();
		
		
		if(currentState.abortPrint){
			break;
		}
		
	}
	
	//Stops printing and returns to idle mode
	currentState.running = true;
	RTX_FLUSH();
	return;
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
		
		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.running = false;
			ReportStatus(UNEXPECTED_EDGE, 'Y');
		}
		PORTC.INTFLAGS |= PIN5_bm;
	}
}

ISR(PORTD_PORT_vect){
	if (PORTD.INTFLAGS & PIN2_bm)
	{
		//X-axis end detected
		
		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.running = false;
			ReportStatus(UNEXPECTED_EDGE, 'X');
		}
		PORTD.INTFLAGS |= PIN2_bm;
	}
	if (PORTD.INTFLAGS & PIN5_bm)
	{
		//Z-axis end detected
		
		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.running = false;
			ReportStatus(UNEXPECTED_EDGE, 'Z');
		}
		PORTD.INTFLAGS |= PIN5_bm;
	}
	if (PORTD.INTFLAGS & PIN1_bm)
	{
		//idk
		
		PORTD.INTFLAGS |= PIN1_bm;
	}
}
