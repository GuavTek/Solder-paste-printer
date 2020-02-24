/*
 * GParse.h
 *
 * Created: 17-Feb-20 16:00:59
 *  Author: mikda
 */ 


#ifndef GPARSE_H_
#define GPARSE_H_

#include "Utility.h"

//Types of motion
enum MotionModes {
	Rapid_position,
	Linear_interpolation,
	Arc_CW,
	Arc_CCW,
	Dwell,
	Home
};

enum BlockType {
	Move,
	Command
};

enum CoordUnit {
	millimeter,
	Inch
};

enum CoordMode {
	absolute,
	incremental
};

//Should contain all parameters of a command
typedef struct {
	StepVector3 pos;
	StepVector3 arcCentre;
	uint32_t arcRadius;
	enum MotionModes motion;
	uint8_t dispenseRate;
	uint8_t moveSpeed;
	enum CoordMode coordinateMode;
	enum CoordUnit coordinateUnit;
} gc_block;


//Will return 0 when a character should be ignored
uint8_t IgnoreChar(char in);

//Will look for the end of a word, returns 1 when a new word is detected
uint8_t WordEnd(char in);

//Will parse the stream of characters
ReturnCodes ParseStream(const char buff[]);

//Will parse and insert command-values in gc-block
ReturnCodes ParseWord(const char wrd[], gc_block *block);



#endif /* GPARSE_H_ */