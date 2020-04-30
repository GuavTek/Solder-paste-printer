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
bool prevError = false;			//Helps keep track when entering/exiting error mode
bool prevMotor = false;			//Keeps motor state before error

int main(void)
{
	USART_INIT(3, 9600);
	PORTC.DIRSET = PIN3_bm;	//Motor enable pin
	PORTC.OUTSET = PIN3_bm;	//Disable motor
	InitEndSensors();
	InitClock();
	InitDispenser();
	stepper_TCB_init();
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
	StepperInit();
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
			//Normal state
			if (prevError)
			{
				prevError = false;
				
				//Restart dispenser
				Dispense(theCurrentBlock.dispenseEnable);
				
				//Restart motor stepping
				TCA0.SINGLE.CTRLA |= prevMotor;
			}
			
			ParseStream();
			GetNewBlock();
			
		} 
		else
		{
			//Error state
			if (!prevError)
			{
				prevError = true;
				
				//Save motor state
				prevMotor = TCA0.SINGLE.CTRLA & TCA_SINGLE_ENABLE_bm;
				
				//Stop dispenser
				Dispense(false);
				
				//Stop motor stepping
				TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
			}
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

