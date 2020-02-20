/*
 * Utility.h
 *
 * Created: 19-Feb-20 12:25:26
 *  Author: mikda
 */ 

#ifndef UTILITY_H_
#define UTILITY_H_

//Will find a characters position in the string
uint8_t ScanWord(const char* wrd[], uint8_t startIndex, char findChar);

//Will return a slice of the string
const char* Slice (const char* original[], uint8_t startIndex, uint8_t stopIndex);

//Find number of characters after startIndex
uint8_t StringLength(const char* strng[], uint8_t startIndex);

#endif /* UTILITY_H_ */