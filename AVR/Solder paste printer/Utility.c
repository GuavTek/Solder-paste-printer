/*
 * Utility.c
 *
 * Created: 19-Feb-20 12:25:54
 *  Author: mikda
 */ 

#include <stdint.h>
#include "Header.h"

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

