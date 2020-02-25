/*
 * GParse.c
 *
 * Created: 17-Feb-20 16:01:19
 *  Author: mikda
 */ 

#include "Header.h"

//Will return true when a character should be ignored
bool IgnoreChar(char in);

//Will look for the end of a word, returns true when a new word is detected
bool WordEnd(char in);

//Write to block buffer
void WriteBlockBuffer(gc_block block);

//Will parse and insert command-values in gc-block
ReturnCodes ParseWord(const char wrd[], gc_block *block);

uint8_t blockBufferHead = 0;
uint8_t blockBufferTail = 0;

gc_block blockBuffer[BLOCK_BUFFER_SIZE];

ReturnCodes ParseStream(){
	static gc_block currentBlock;
	static char currentWord[MAX_WORD_SIZE];
	static uint8_t wordIndex = 0;
	static bool readyBlock = false;
	
	if (readyBlock)
	{
		//Check if buffer is full
		if (BlockBufferAvailable() == BUFFER_FULL)
		{
			return BUFFER_FULL;
		} 
		else
		{
			WriteBlockBuffer(currentBlock);
			readyBlock = false;
		}
		
	}
	
	//Load next character from buffer
	char nextChar = RX_read();
	
	//Discard ignored chars
	if (IgnoreChar(nextChar)){
		return NONE;
	}
	
	//Detect line end
	if ((nextChar == '\r' || nextChar == '\n') && wordIndex > 0){
		
		//Erase the unused part of the word buffer
		for (uint8_t i = wordIndex; i < MAX_WORD_SIZE; i++){
			currentWord[i] = 0;
		}
		
		ParseWord(currentWord,&currentBlock);
		wordIndex = 0;	
		
		//Push the block into buffer unless it is full
		readyBlock = true;
		if(BlockBufferAvailable() == BUFFER_FULL){
			return BUFFER_FULL;
		} else {
			WriteBlockBuffer(currentBlock);
			readyBlock = false;
		}
				
		return NEW_BLOCK;
	}
	
	//The two first chars should always be included
	if(wordIndex < 2){
		currentWord[wordIndex] = nextChar;
		wordIndex++;
	}
	else
	{
		//Checks if a new word has started
		if (WordEnd(nextChar)){
			//Erase the unused part of the word buffer
			for (uint8_t i = wordIndex; i < MAX_WORD_SIZE; i++){
				currentWord[i] = 0;
			}
			ParseWord(currentWord,&currentBlock);
			wordIndex = 0;
		}
		currentWord[wordIndex] = nextChar;
		wordIndex++;
	}
	return NONE;
}

ReturnCodes ParseWord(const char wrd[], gc_block *block){
	char letter = wrd[0];
	int num = 0;
	uint8_t fraction = 0;
	uint8_t precision = 0;
	static int parameter = 0;
	
	//Scan the string to see if it is a float
	uint8_t dotPos = ScanWord(wrd, 1, '.');
	
	//If float, convert fraction separately
	if (dotPos)
	{
		num = atoi(Slice(wrd, 1, dotPos - 1));
		precision = StringLength(wrd, dotPos + 1);
		fraction = atoi(Slice(wrd, dotPos + 1, dotPos + precision));
		
	} else {
		num = atoi(Slice(wrd, 1, MAX_WORD_SIZE));
	}
	
	
	//Detect if it is real-time command
	if(letter > 0x7F){
		
	} else {
		switch (letter)
		{
			//case 'A': case 'B': case 'C':{break;} //Ignore rotation
				
			case 'F': {
				//Feedrate
				block->moveSpeed = num;
				break;
			}
			case 'G':{
				//Prep commands
				switch(num){
					case 0: {
						block->motion = Rapid_position;
						break;
					}
					case 1: {
						block->motion = Linear_interpolation;
						break;
					}
					case 2: {
						block->motion = Arc_CW;
						break;
					}
					case 3: {
						block->motion = Arc_CCW;
						break;
					}
					case 4: {
						block->motion = Dwell;
						break;
					}
					case 20: {
						block->coordinateUnit = Inch;
						break;
					}
					case 21: {
						block->coordinateMode = millimeter;
						break;
					}
					case 28: {
						block->motion = Home;
						break;
					}
					case 90: {
						block->coordinateMode = absolute;
						break;
					}
					case 91: {
						block->coordinateMode = incremental;
						break;
					}
				}
				break;
			}
			
			case 'I':{
				//Arc center X
				block->arcCentre.x = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;	
			}
			case 'J':{
				//Arc center Y
				block->arcCentre.y = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;
			}
			case 'K':{
				//Arc center Z
				block->arcCentre.z = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;	
			}
			case 'M':{
				//Machine commands
				switch(num){
					case 0: {
						//Compulsory stop
						ReportStatus(STOP_DETECTED);
						return STOP_DETECTED;
					}
					case 1: {
						//Optional stop
						ReportStatus(STOP_DETECTED);
						return STOP_DETECTED;
					}
					case 2: {
						//End of program
						ReportStatus(STOP_DETECTED);
						return STOP_DETECTED;
					}
					case 3: case 4: {
						//Spindle (dispenser) on
						
						break;
					}
					case 5: {
						//Spindle (dispenser) off
						
						break;
					}
					case 30: {
						//End of program, return to program top
						ReportStatus(STOP_DETECTED);
						return STOP_DETECTED;
					}
				}
				break;
			}
			case 'N':{
				//Line number
				
				return NONE;
			}
			case 'O':{
				//Program name
				
				break;
			}
			case 'P':{
				//Parameter for G and M
				parameter = num;
				break;
			}
			case 'R':{
				//Arc radius
				block->arcRadius = 0; //Not implemented
				break;
			}
			case 'S':{
				//Spindle speed
				block->dispenseRate = num;
				break;
			}
			case 'X':{
				//Position X
				block->pos.x = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;
			}
			case 'Y':{
				//Position Y
				block->pos.y = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;
			}
			case 'Z':{
				//Position Z
				block->pos.z = Micro2Step(num * 10000 + fraction * pow(10, 4-precision));
				break;
			}
			default:{
				ReportStatus(NOT_RECOGNIZED);
				return NOT_RECOGNIZED; //Unrecognized command
			}
		}
	}
	return NONE;
}

bool IgnoreChar(char in){
	if (in == ' ')
	{
		return true;
	} else if (in == '+')
	{
		return true;
	} else if (in == '%')
	{
		return true;
	}
	return false;
}

bool WordEnd(char in){
	if (in == '.')
	{
		return false;
	} else if (in == '-')
	{
		return false;
	} else if ((in <= '9') && (in >= '0'))
	{
		return false;
	}
	return true;
}

ReturnCodes BlockBufferAvailable(){
	if(blockBufferTail == blockBufferHead){
		return BUFFER_EMPTY;
	}
	
	uint8_t tempTail = blockBufferTail - 1;
	if (tempTail >= BLOCK_BUFFER_SIZE)
	{
		tempTail = (BLOCK_BUFFER_SIZE - 1);
	}
	
	if (blockBufferHead == tempTail)
	{
		return BUFFER_FULL;
	}
	return NONE;
}

void WriteBlockBuffer(gc_block block){
	blockBuffer[++blockBufferHead] = block;
}

gc_block ReadBlockBuffer(){
	return blockBuffer[++blockBufferTail];
}