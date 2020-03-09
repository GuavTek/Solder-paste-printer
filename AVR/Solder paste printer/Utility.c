/*
 * Utility.c
 *
 * Created: 19-Feb-20 12:25:54
 *  Author: mikda
 */ 

#include "Header.h"

//The function the RTC calls when triggered
//Is set by StartDwell
void (*RTC_Callback)(void);

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
	return 0;
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
	float tempLength = millimeters / METRIC_STEP_LENGTH;
	newStep.full = round(tempLength);
	tempLength -= newStep.full;
	newStep.micro = round(tempLength * 16);
	return newStep;
}

void InitClock(){
	
	//Enable external clock
	//CLKCTRL.XOSC32KCTRLA = CLKCTRL_ENABLE_bm;
	
	//Wait for registers to synchronize
	while(RTC.STATUS > 0){}
	
	//Use internal crystal
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;
	
	
	RTC.PER = 0xFFFF;
	RTC.CMP = 0x10;
	
	//Enable compare match interrupt
	//RTC.INTCTRL = RTC_CMP_bm;
	
	//Debug enable
	RTC.DBGCTRL |= RTC_DBGRUN_bm;
	
	//Set prescaler and enable RTC
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm ;

}

void StartTimer(uint16_t waitTime, void (*functionToTrigger)(void)){
	//Reset timer
	//while(RTC.STATUS & RTC_CNTBUSY_bm){}
	//RTC.CNT = 0;
	
	//Convert from milliseconds to periods of 1.024kHz crystal
	uint32_t tempTime = (waitTime * 24)/1000 + waitTime + RTC.CNT;
	//Set wait-time
	while(RTC.STATUS & RTC_CMPBUSY_bm){}
	RTC.CMP = tempTime;
	
	//Set the function to recall
	RTC_Callback = functionToTrigger;
	
	//Enable interrupt
	RTC.INTCTRL = RTC_CMP_bm;
}

ISR(RTC_CNT_vect){
	//Disable interrupt
	RTC.INTCTRL &= ~RTC_CMP_bm;
	
	//Clear interrupt flag
	RTC.INTFLAGS = RTC_CMP_bm;
	
	//Do something
	RTC_Callback();
}
