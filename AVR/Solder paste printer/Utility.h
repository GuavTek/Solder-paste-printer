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
	BUFFER_AVAILABLE,
	BUFFER_EMPTY,
	BUFFER_OVERFLOW,
	SHORT_WORD,
	UNEXPECTED_EDGE
} ReturnCodes;

//Types of motion
enum MotionModes {
	Rapid_position,
	Linear_interpolation,
	Arc_CW,
	Arc_CCW,
	Dwell,
	Home,
	Stop
};

enum CoordUnit {
	millimeter,
	Inch
};

enum CoordMode {
	absolute,
	incremental
};

typedef enum {
	idle,
	printing
} Status;

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

typedef struct {
	Status state;
	enum MotionModes task;
	bool abortPrint;
	bool noError;
	bool blockFinished;
	bool statusDump;
} PrinterState;

//Should contain all parameters of a command
typedef struct {
	StepVector3 pos;
	StepVector3 arcCentre;
	uint32_t arcRadius;
	enum MotionModes motion;
	uint8_t dispenseRate;
	uint16_t moveSpeed;
	bool dispenseEnable;
	uint16_t dwellTime;
	uint16_t blockNumber;
	enum CoordMode coordinateMode;
	enum CoordUnit coordinateUnit;
} gc_block;

extern PrinterState currentState;
extern gc_block theCurrentBlock;

//Will find a characters position in the string
uint8_t ScanWord(const char wrd[], uint8_t startIndex, char findChar);

//Will return a slice of the string
void Slice (const char original[], char sliced[], uint8_t startIndex, uint8_t stopIndex);

//Find number of characters after startIndex
uint8_t StringLength(const char strng[], uint8_t startIndex);

//Converts millimeters to steps
StepCount Metric2Step(float millimeters);

//Imperial not currently supported
//StepCount UInch2Step(int minches);

//Sets up the clock for StartTimer function
void InitClock();

//Runs a function after waittime, waittime in millisecond
//Triggers RTC_CMP interrupt when done
//Maximum 64 seconds
void StartTimer(uint16_t waitTime, void (*functionToTrigger)(void));

#endif /* UTILITY_H_ */