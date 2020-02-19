/*
 * Utility.c
 *
 * Created: 19-Feb-20 12:25:54
 *  Author: mikda
 */ 

#include <stdint.h>
#include "Header.h"

uint8_t ScanWord(const char* wrd[], uint8_t startIndex, char findChar){
	for (uint8_t i = startIndex; i < MAX_WORD_SIZE; i++)
	{
		//Stop when we run out of digits
		if (*wrd[i] == 0)
		{
			return 0;
		}
		else if (*wrd[i] == findChar)
		{
			return i;
		}
	}
	return 0;
}

const char* Slice(const char* original[], uint8_t startIndex, uint8_t stopIndex){
	int8_t length = stopIndex - startIndex + 1;
	
	//Nothing to slice
	if(length < 1){
		return "0";
	}
	
	char newSlice[length];
	
	for(uint8_t i = 0; i < length; i++){
		newSlice[i] = *original[startIndex + i];
	}
	return newSlice;
}

uint8_t StringLength(const char* strng[], uint8_t startIndex){
	uint8_t counter = 0;
	
	for (uint8_t i = startIndex; i < MAX_WORD_SIZE; i++)
	{
		if (*strng[i] == 0)
		{
			return counter;
		} else {
			counter++;
		}
	}
	return counter;
}