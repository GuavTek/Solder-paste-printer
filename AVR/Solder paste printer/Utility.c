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
uint16_t RTC_Times[8];			//The time the function should be called
uint8_t sortedIndex[8];			//The order the functions should be called
uint8_t indexS = 0;				//The index of sortedIndex
uint8_t bAvail = 0xff;			//Bitmask indicates buffer availability

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

StepCount Metric2Step(float millimeters){
	StepCount newStep;

	//Convert to number of steps
	float tempLength = millimeters / METRIC_STEP_LENGTH;
	
	//Save full-steps
	newStep.full = round(tempLength);
	
	//Subtract full-steps
	tempLength -= newStep.full;

	//Get micro-steps
	newStep.micro = round(tempLength * 16);
	
	return newStep;
}

void InitClock(){
	
	//Enable external clock
	//CLKCTRL.XOSC32KCTRLA = CLKCTRL_ENABLE_bm;
	
	//Wait for registers to synchronize
	while(RTC.STATUS > 0){}
	
	//Use internal crystal in 1.024kHz mode
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;
	
	//Set period
	RTC.PER = 0xFFFF;
	
	//Debug enable
	//RTC.DBGCTRL |= RTC_DBGRUN_bm;
	
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
		
	//Convert from milliseconds to periods of 1.024kHz crystal
	uint32_t tempTime = (waitTime * 24)/1000 + waitTime + RTC.CNT;
	uint8_t tempIndex;
	
	//Find empty buffer space
	for (uint8_t i = 0; i < 8; i++)
	{
		if (bAvail & 1<<i)
		{
			bAvail &= ~(1<<i);
			RTC_Times[i] = tempTime;
			RTC_Callback[i] = functionToTrigger;
			tempIndex = i;
			break;	
		}
	}

	//Check if there is another element in buffer
	if (indexS > 0)
	{
		//Sort which interrupt should occur first
		uint16_t currentTime = RTC.CNT;
		tempTime -= currentTime;
		for (int8_t i = indexS; i >= 0; i--)
		{
			//Nothing more to compare
			if (i == 0)
			{
				sortedIndex[0] = tempIndex;
				break;
			}

			//Element 0 in array will be used last
			uint16_t compTime = RTC_Times[sortedIndex[i-1]] - currentTime;
			if (compTime < tempTime)
			{
				sortedIndex[i] = sortedIndex[i-1];
			} else {
				sortedIndex[i] = tempIndex;
				break;
			}
		}
	} 
	else
	{
		sortedIndex[0] = tempIndex;
	}

	//Set wait-time
	while(RTC.STATUS & RTC_CMPBUSY_bm){}
	RTC.CMP = RTC_Times[sortedIndex[indexS]];
	

	indexS++;
	
	//Enable interrupt
	RTC.INTCTRL = RTC_CMP_bm;
}

ISR(RTC_CNT_vect){
	//Clear interrupt flag
	RTC.INTFLAGS = RTC_CMP_bm;

	if (indexS > 0)
	{
		indexS--;
	}
	
	//Check if there are more functions waiting
	if (indexS > 0)
	{
		//Set next interrupt time
		while(RTC.STATUS & RTC_CMPBUSY_bm){}
		RTC.CMP = RTC_Times[sortedIndex[indexS - 1]];
	} 
	else
	{
		//Disable interrupt
		RTC.INTCTRL &= ~RTC_CMP_bm;
	}

	//Set buffer position as available again
	bAvail |= 1<<sortedIndex[indexS];

	//Abort if Callback is unassigned
	if (*RTC_Callback[sortedIndex[indexS]] == 0)
	{
		ReportEvent(NO_CALLBACK,'T');
		return;
	}

	//Do something
	RTC_Callback[sortedIndex[indexS]]();


}
