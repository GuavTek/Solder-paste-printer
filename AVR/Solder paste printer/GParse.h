/*
 * GParse.h
 *
 * Created: 17-Feb-20 16:00:59
 *  Author: mikda
 */ 


#ifndef GPARSE_H_
#define GPARSE_H_

#include "Utility.h"


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