/*
 * Utility.c
 *
 * Created: 19-Feb-20 12:25:54
 *  Author: mikda
 */ 

#include "Header.h"

PrinterState currentState;

gc_block theCurrentBlock;

//The function the RTC calls when triggered
//Is set by StartDwell
void (*RTC_Callback[8])(void);
uint16_t RTC_Times[8];					//The time the function should be called
uint8_t sortedIndex[8];					//The order the functions should be called
volatile int8_t indexS = 0;				//The index of sortedIndex, counts number of elements in buffer
uint8_t bAvail = 0xff;					//Bitmask indicates buffer availability
volatile uint8_t millis = 0;			//Milliseconds since RunDelayedFunctions have been run

uint8_t ScanWord(const char wrd[], uint8_t startIndex, char findChar){
	for (uint8_t i = startIndex; i < MAX_WORD_SIZE; i++)
	{
		//Stop when we run out of digits
		if (wrd[i] == 0)
		{
			return 0;
		}
		else if (wrd[i] == findChar)
		{
			return i;
		}
	}
	
	return 0;	//did not find char
}

void Slice(const char original[], char sliced[], uint8_t startIndex, uint8_t stopIndex){
	int8_t length = stopIndex - startIndex + 1;
	
	//Clear sliced buffer
	for (uint8_t i = 0; i < MAX_WORD_SIZE; i++)
	{
		sliced[i] = 0;
	}

	//Nothing to slice
	if(length < 1){
		return;
	}
	
	//Insert slice
	for(uint8_t i = 0; i < length; i++){
		sliced[i] = original[startIndex + i];
	}
}

uint8_t StringLength(const char strng[], uint8_t startIndex){
	uint8_t counter = 0;
	
	for (uint8_t i = startIndex; i < MAX_WORD_SIZE; i++)
	{
		if (strng[i] == 0)
		{
			return counter;
		} else {
			counter++;
		}
	}
	return counter;
}

StepCount Length2Step(float length, enum CoordUnit unit){
	StepCount newStep;
	float tempLength;

	//Convert to number of steps
	if(unit == millimeter){
		tempLength = length / METRIC_STEP_LENGTH;
	} else {
		tempLength = length / IMPERIAL_STEP_LENGTH;
	}

	//Save full-steps
	newStep.full = round(tempLength);
	
	//Subtract full-steps
	tempLength -= newStep.full;

	//Get micro-steps
	newStep.micro = round(tempLength * 16);
	
	return newStep;
}

StepCount LengthZ2Step(float length, enum CoordUnit unit){
	StepCount newStep;
	float tempLength;

	//Convert to number of steps
	if(unit == millimeter){
		tempLength = length / METRIC_STEP_Z_LENGTH;
		} else {
		tempLength = length / IMPERIAL_STEP_Z_LENGTH;
	}

	//Save full-steps
	newStep.full = round(tempLength);
	
	//Subtract full-steps
	tempLength -= newStep.full;

	//Get micro-steps
	newStep.micro = round(tempLength * 16);
	
	return newStep;
}

void InitClock(){
	
	//Wait for registers to synchronize
	while(RTC.STATUS > 0){}
	
	//Use internal crystal in 32.768kHz mode
	RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
	
	//Set period
	RTC.PER = 32;
	
	//Debug enable
	//RTC.DBGCTRL |= RTC_DBGRUN_bm;

	//Set interrupt for running delayed functions
	PORTE.PIN0CTRL = PORT_ISC_LEVEL_gc;
	PORTE.DIRSET = PIN0_bm;
	
	//Set prescaler and enable RTC
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm ;

}

void StartTimer(uint16_t waitTime, void (*functionToTrigger)(void)){
	
	//Abort if buffer is full
	if (bAvail == 0)
	{
		ReportEvent(BUFFER_OVERFLOW, 'T');
		return;
	}
		
	uint16_t newTime = waitTime;
	uint8_t newIndex = 0;
	
	//Find empty buffer space
	for (uint8_t i = 0; i < 8; i++)
	{
		if (bAvail & (1 << i))
		{
			bAvail &= ~(1<<i);						//Buffer space occupied
			RTC_Callback[i] = functionToTrigger;	//Save function to wake up
			newIndex = i;
			break;	
		}
	}


	//Sort which interrupt should occur first
	//Uses a modified insertion sort
		
	//Disable interrupts
	cli();

	RTC_Times[sortedIndex[indexS - 1]] -= millis;	//Apply time that have passed
	millis = 0;

	int8_t i;		//Index the new element should be placed in

	//Move all the shorter times further out in buffer
	for (i = indexS; i > 0; i--)
	{
		//Compare time in buffer with new time
		uint16_t compTime = RTC_Times[sortedIndex[i-1]];
		if (compTime < newTime)
		{
			//Wait-times are relative to previous wait, it cumulates
			newTime -= compTime;

			//Move item in buffer
			sortedIndex[i] = sortedIndex[i-1];
		} else {
			//Found the correct index, exit loop
			break;
		}
	}

	//Keep wait-times relative
	if (i > 0)
	{
		RTC_Times[sortedIndex[i-1]] -= newTime;
	}

	sortedIndex[i] = newIndex;		//New item has been sorted
	RTC_Times[newIndex] = newTime;	//Save wait-time
	indexS++;						//Increment sortedIndex

	//Re-enable interrupts
	sei();

	//Enable interrupt
	RTC.INTCTRL = RTC_OVF_bm;
}

void RunDelayedFunctions(){
	//Save current time
	uint8_t tempTrig = millis;
	millis -= tempTrig;

	//Apply all of the time
	for (uint8_t i = indexS - 1; i >= 0; i--)
	{
		if (RTC_Times[sortedIndex[i]] > tempTrig)
		{
			//Reduce wait-time
			RTC_Times[sortedIndex[i]] -= tempTrig;
			break;
		} else {
			//Set remaining passed time
			tempTrig -= RTC_Times[sortedIndex[i]];
			
			RTC_Times[sortedIndex[i]] = 0;
		}
	}

	//Run all the queued functions that have been triggered
	for(uint8_t i = indexS - 1; i >= 0; i--){
		
		if (RTC_Times[sortedIndex[i]] > 0)
		{
			//Not triggered
			break;
		}

		//Check if index is valid
		if (indexS > 0)
		{
			indexS--;
		} else {
			millis = 0;
			RTC.INTCTRL &= ~RTC_OVF_bm;
			ReportEvent(BUFFER_EMPTY, 'R');
			return;
		}

		//Set buffer position as available again
		bAvail |= 1<<sortedIndex[indexS];
		
		//Do something
		RTC_Callback[sortedIndex[indexS]]();

		//Stop timer when there are no delays waiting
		if (indexS <= 0)
		{
			//Disable interrupt
			RTC.INTCTRL &= ~RTC_OVF_bm;
			millis = 0;
		}
	}

	
}

ISR(_VECTOR(3)){
	//Clear interrupt flag
	RTC.INTFLAGS = RTC_OVF_bm;

	millis++;

	//Check if delay time is done
	if ((millis > 200) || (millis >= RTC_Times[sortedIndex[indexS - 1]]))
	{
		//Trigger pin interrupt
		PORTE.OUTCLR = PIN0_bm;
	}
}
