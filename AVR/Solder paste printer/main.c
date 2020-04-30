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

uint16_t timer = 300;
uint8_t start_pos = (1 << START_POS_X) | (1 << START_POS_Y);

int main(void)
{
	USART_INIT(3, 9600);
	PORTC.DIRSET = PIN3_bm;	//Motor enable pin
	PORTC.OUTSET = PIN3_bm;	//Disable motor
	InitEndSensors();
	InitClock();
	InitDispenser();
	stepper_TCB_init();
	StepperInit();
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
				timer = 300;
				currentState.state = idle;
			}
		}
		RunDelayedFunctions();
    }
}

void Blinky(){
	PORTF.OUTTGL = PIN5_bm;
	StartTimer(timer, Blinky);
}

//Main loop when printing
void Print(void) {
	PORTC.OUTCLR = PIN3_bm; //Enable motors
	InitParser();
	TX_write('k');
	TX_write('\n');
	TX_write('\r');
	currentState.state = printing;
	currentState.abortPrint = false;
	currentState.blockFinished = true;
	currentState.noError = true;
	timer = 1000;
	
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
		
		RunDelayedFunctions();

		if (currentState.statusDump)
		{
			ReportStatus();
		}
		
		if(currentState.abortPrint){
			//Stops printing and returns to idle mode
			RTX_FLUSH();
			PORTC.OUTSET = PIN3_bm;	//Disable motors
			//Stop stepping
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
	
	currentState.blockFinished = false;

	currentState.task = theCurrentBlock.motion;
	
	//Set dispense state
	Dispense(theCurrentBlock.dispenseEnable);
	
	//Execute block command
	switch(theCurrentBlock.motion){
		case Linear_interpolation:
			PrepStep();
			break;
		case Rapid_position:
			PrepStep();
			break;
		case Arc_CW:
		case Arc_CCW:
		case Home: {
			HomingRoutine(theCurrentBlock.motion);
			break;
		}
		case Dwell: {
			StartTimer(theCurrentBlock.dwellTime, EndDwell);
			break;
		}
		case Pause: {
			currentState.noError = false;	//Not actually an error, but procedure is the same
			ReportEvent(PAUSED, 0);
			currentState.blockFinished = true;
			return;
		}
		case Stop: {
			currentState.abortPrint = true;
			ReportEvent(STOP_DETECTED, 0);
			break;
		}
		default: break;
	}
	
}

void EndDwell(){
	//Alls well that ends dwell
	currentState.blockFinished = true;

	ReportEvent(DWELL_FINISHED,0);
}

void InitEndSensors(){
	PORTC.PIN5CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTD.PIN1CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTD.PIN2CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTD.PIN5CTRL = PORT_ISC_BOTHEDGES_gc;
}

ISR(PORTC_PORT_vect){
	if (PORTC.INTFLAGS & PIN5_bm)
	{
		//Note: check if pin is high when home command is started
		//Y-axis end detected
		if (PORTC.IN & PIN5_bm)
		{
			if(start_pos & (1 << START_POS_Y))
			{
				TCB1.CTRLB |= TCB_CCMPEN_bm;
				TCB1.INTCTRL &= ~TCB_CAPT_bm;
			}
			else
			{
				TCB1.CTRLB &= ~TCB_CCMPEN_bm;
				start_pos |= (1 << START_POS_Y);
			}
			//Reverse Y direction
		} 
		else
		{
			//Stop Y Stepping
			start_pos &= ~(1 << START_POS_Y);
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			//Y pos = 0
			//Step to thecurrentblock.pos.y
		}
			
		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'Y');
		}
		PORTC.INTFLAGS |= PIN5_bm;
	}
}

ISR(PORTD_PORT_vect){
	if (PORTD.INTFLAGS & PIN2_bm)
	{
		//X-axis end detected
		if (PORTD.IN & PIN2_bm)
		{
			if(start_pos & (1 << START_POS_X))
			{
				TCB0.CTRLB |= TCB_CCMPEN_bm;
				TCB0.INTCTRL &= ~TCB_CAPT_bm;
			}
			else
			{
				TCB0.CTRLB &= ~TCB_CCMPEN_bm;
				start_pos |= (1 << START_POS_X);
			}
		}
		else
		{
			//Stop X stepping
			//X pos = 0
			start_pos &= ~(1 << START_POS_X);
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
		}

		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'X');
		}
		PORTD.INTFLAGS |= PIN2_bm;
	}
	if (PORTD.INTFLAGS & PIN5_bm)
	{
		//Z-axis end detected
		if (PORTD.IN & PIN5_bm)
		{
			//Reverse Z direction
		}
		else
		{
			//Stop Z stepping
			//Z pos = 0
		}

		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'Z');
		}
		PORTD.INTFLAGS |= PIN5_bm;
	}
	if (PORTD.INTFLAGS & PIN1_bm)
	{
		//Currently unused
		
		PORTD.INTFLAGS |= PIN1_bm;
	}
}
