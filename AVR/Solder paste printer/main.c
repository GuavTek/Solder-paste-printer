/*
 * Solder paste printer.c
 *
 * Created: 05-Feb-20 11:27:05
 * Author : mikda
 */ 

#include "Header.h"

//Primary loop when printing
void Print(void);

//Initialize sensors that detect the edge of printer
void InitEndSensors();

//Start excecuting a new block
void GetNewBlock();

//Tells the system that it can continue the process
void EndDwell();

//Blink led
void Blinky(void);

uint16_t timer = 1000;

int main(void)
{
	USART_INIT(3, 9600);
    stepper_TCB_init();
	InitEndSensors();
	InitClock();
	InitDispenser();
	PORTF.DIRSET = PIN5_bm;	//Onboard LED
	sei();
	Blinky();
	USARTn.TXDATAL = 'o';
	currentState.state = idle;
    currentState.noError = true;
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
	currentState.blockFinished = true;
	currentState.noError = true;
	timer = 200;
	PORTF.OUTSET = PIN5_bm;
	
	while(1){
		if (currentState.noError)
		{
			ParseStream();
			GetNewBlock();
		} 
		else
		{
			//Error state
		}
		TX_Jumpstart();
		
		if (currentState.statusDump)
		{
			ReportStatus();
		}
		
		if(currentState.abortPrint){
			//Stops printing and returns to idle mode
			RTX_FLUSH();
			return;
		}
		
	}
}

void GetNewBlock(){
	//Return if previous block is not finished
	if (!currentState.blockFinished)
	{
		return;
	}
	
	if (BlockBufferAvailable() == BUFFER_EMPTY)
	{
		return;
	} else {
		theCurrentBlock = ReadBlockBuffer();
	}
	
	currentState.task = theCurrentBlock.motion;
	
	if (theCurrentBlock.dispenseEnable)
	{
		Dispense(true);
	} else {
		Dispense(false);
	}
	
	switch(theCurrentBlock.motion){
		case Linear_interpolation:
		case Rapid_position:
		case Arc_CW:
		case Arc_CCW:
		case Home: {
			//PrepStep();
			StartTimer(500, EndDwell);
			break;
		}
		case Dwell: {
			StartTimer(theCurrentBlock.dwellTime, EndDwell);
			break;
		}
		case Stop: {
			currentState.abortPrint = true;
			break;
		}
	}
	currentState.blockFinished = false;
}

void EndDwell(){
	//Alls well that ends dwell
	currentState.blockFinished = true;
	Blinky();
}

void InitEndSensors(){
	//To do: find triggered level (PORT_INVEN if high)
	PORTC.PIN5CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN1CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN2CTRL = PORT_ISC_LEVEL_gc;
	PORTD.PIN5CTRL = PORT_ISC_LEVEL_gc;
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
			currentState.noError = false;
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
			currentState.noError = false;
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
			currentState.noError = false;
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
