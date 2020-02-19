/*
 * GParse.h
 *
 * Created: 17-Feb-20 16:00:59
 *  Author: mikda
 */ 


#ifndef GPARSE_H_
#define GPARSE_H_

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

typedef struct  {
	int x;
	int y;
	int z;
} Vector3;

//Should contain all parameters of a command
typedef struct  {
	Vector3 pos;
	enum MotionModes motion;
	uint8_t feedrate;
} gc_block;


//Will parse the stream of characters
uint8_t ParseLine(const char* line[]);

//Will parse and insert command-values in gc-block
//Returns status code
uint8_t ParseWord(const char* wrd[], gc_block *block);



#endif /* GPARSE_H_ */