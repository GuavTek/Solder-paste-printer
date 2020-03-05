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
	bool dispenseEnable;
	uint32_t dwellTime;
	enum CoordMode coordinateMode;
	enum CoordUnit coordinateUnit;
} gc_block;


//Checks the status of the block buffer
ReturnCodes BlockBufferAvailable();

//Read from block buffer
gc_block ReadBlockBuffer();

//Reads without incrementing (reads the previous block)
gc_block PeekBlockBuffer();

//Will parse the stream of characters
ReturnCodes ParseStream();

//Initialize, or reset, parser state
void InitParser();


#endif /* GPARSE_H_ */