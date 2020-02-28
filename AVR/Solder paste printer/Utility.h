/*
 * Utility.h
 *
 * Created: 19-Feb-20 12:25:26
 *  Author: mikda
 */ 

#ifndef UTILITY_H_
#define UTILITY_H_


typedef enum {
	NONE,
	STOP_DETECTED,
	NEW_BLOCK,
	NOT_RECOGNIZED,
	BUFFER_FULL,
	BUFFER_EMPTY
} ReturnCodes;

typedef struct {
	int full;
	int8_t micro;
} StepCount;

typedef struct {
	StepCount x;
	StepCount y;
	StepCount z;
} StepVector3;

typedef struct  {
	int x;
	int y;
	int z;
} Vector3;

//Will find a characters position in the string
uint8_t ScanWord(const char wrd[], uint8_t startIndex, char findChar);

//Will return a slice of the string
void Slice (const char original[], char sliced[], uint8_t startIndex, uint8_t stopIndex);

//Find number of characters after startIndex
uint8_t StringLength(const char strng[], uint8_t startIndex);

//Sends error statuses to pc
void ReportStatus(ReturnCodes code);

//Converts millimeters to steps
StepCount Metric2Step(float millimeters);

//Imperial not currently supported
//StepCount UInch2Step(int minches);

#endif /* UTILITY_H_ */