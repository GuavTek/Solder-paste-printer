/*
 * GParse.c
 *
 * Created: 17-Feb-20 16:01:19
 *  Author: mikda
 */ 

#include "Header.h"

//Will return true when a character should be ignored
inline bool IgnoreChar(char in);

//Will look for the end of a word, returns true when a new word is detected
inline bool WordEnd(char in);

//Write to block buffer
void WriteBlockBuffer(gc_block block);

//Will parse and insert command-values in gc-block
ReturnCodes ParseWord();

uint8_t blockBufferHead = 0;
uint8_t blockBufferTail = 0;
gc_block blockBuffer[BLOCK_BUFFER_SIZE];

uint8_t colons;	//For ignoring comments

gc_block currentBlock;
uint8_t wordIndex = 0;
bool freshBlock = true;				// True if currentblock contains no new info
char currentWord[MAX_WORD_SIZE];

ReturnCodes ParseStream(){
	static bool readyBlock = false;
	
	//Skips if current block hasn't been placed in buffer yet
	if (readyBlock)
	{
		//Check if buffer is full
		if (BlockBufferAvailable() == BUFFER_FULL)
		{
			return BUFFER_FULL;
		} 
		else
		{
			//ReportStatus(BUFFER_AVAILABLE);
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
	if ((nextChar == '\r' || nextChar == '\n')){
		
		if(ParseWord() == NONE){
			freshBlock = false;
		}
		wordIndex = 0;	
		if(freshBlock == false){
			colons = 0;
			freshBlock = true;
			
			//Push the block into buffer unless it is full
			readyBlock = true;
			if(BlockBufferAvailable() == BUFFER_FULL){
				ReportEvent(BUFFER_FULL, 'B');
				return BUFFER_FULL;
			} else {
				WriteBlockBuffer(currentBlock);
				readyBlock = false;
			}

			ReportEvent(NEW_BLOCK, 0);
			return NEW_BLOCK;
		}
	}
	
	//Detect word overflow
	if (wordIndex >= (MAX_WORD_SIZE - 1))
	{
		ReportEvent(BUFFER_OVERFLOW, 'W');
	}
	
	//Checks if a new word has started
	if (WordEnd(nextChar)){
		if(ParseWord() == NONE){
			freshBlock = false;
		}
		wordIndex = 0;
	}
	
	currentWord[wordIndex] = nextChar;
	wordIndex++;

	return NONE;
}

ReturnCodes ParseWord(){
	char letter = currentWord[0];
	float val = 0;
	int num;
	//uint16_t fraction = 0;
	//uint8_t precision = 0;
	static int parameter = 0;
	
	//Return if word is too short
	if (wordIndex < 2)
	{
		if (wordIndex > 0)
		{
			ReportEvent(SHORT_WORD, currentWord[0]);
		}
		return SHORT_WORD;
	}
	
	//Erase the unused part of the word buffer
	for (uint8_t i = wordIndex; i < MAX_WORD_SIZE; i++){
		currentWord[i] = 0;
	}
	
	
	//Scan the string to see if it is a float
	uint8_t dotPos = ScanWord(currentWord, 1, '.');
	
	//Convert string to float
	val = atof(currentWord + 1);

	
	//If float, convert fraction separately
	if (dotPos)
	{
		char tempSlice[MAX_WORD_SIZE];
		Slice(currentWord, tempSlice, 1, dotPos - 1);
		num = atoi(tempSlice);
	//	precision = StringLength(currentWord, dotPos + 1);
	//	Slice(currentWord, tempSlice, dotPos + 1, dotPos + precision);
	//	fraction = atoi(tempSlice);
		
	} else {
		num = atoi(currentWord + 1);
	}
	

	//Turn characters uppercase
	if (letter <= 'z' && letter >= 'a')
	{
		letter -= ('a' - 'A');
	}

	//Detect if it is real-time command
	if(letter > 0x7F){
		//Should be picked up when received
	} else {
		switch (letter)
		{
			//case 'A': case 'B': case 'C':{break;} //Ignore rotation
				
			case 'F': {
				//Feedrate
				currentBlock.moveSpeed = num;
				break;
			}
			case 'G':{
				//Prep commands
				switch(num){
					case 0: {
						currentBlock.motion = Rapid_position;
						break;
					}
					case 1: {
						currentBlock.motion = Linear_interpolation;
						break;
					}
					case 2: {
						currentBlock.motion = Arc_CW;
						break;
					}
					case 3: {
						currentBlock.motion = Arc_CCW;
						break;
					}
					case 4: {
						currentBlock.motion = Dwell;
						currentBlock.dwellTime = parameter;
						break;
					}
					case 20: {
						currentBlock.coordinateUnit = Inch;
						break;
					}
					case 21: {
						currentBlock.coordinateMode = millimeter;
						break;
					}
					case 28: {
						currentBlock.motion = Home;
						break;
					}
					case 90: {
						currentBlock.coordinateMode = absolute;
						break;
					}
					case 91: {
						currentBlock.coordinateMode = incremental;
						break;
					}
				}
				break;
			}
			
			case 'I':{
				//Arc center X
				currentBlock.arcCentre.x = Metric2Step(val);
				break;	
			}
			case 'J':{
				//Arc center Y
				currentBlock.arcCentre.y = Metric2Step(val);
				break;
			}
			case 'K':{
				//Arc center Z
				currentBlock.arcCentre.z = Metric2Step(val);
				break;	
			}
			case 'M':{
				//Machine commands
				switch(num){
					case 0: {
						//Compulsory stop
						currentBlock.motion = Stop;
						break;
					}
					case 1: {
						//Optional stop
						currentBlock.motion = Stop;
						break;
					}
					case 2: {
						//End of program
						currentBlock.motion = Stop;
						break;
					}
					case 3: case 4: {
						//Spindle (dispenser) on
						currentBlock.dispenseEnable = true;
						break;
					}
					case 5: {
						//Spindle (dispenser) off
						currentBlock.dispenseEnable = false;
						break;
					}
					case 30: {
						//End of program, return to program top
						currentBlock.motion = Stop;
						break;
					}
				}
				break;
			}
			case 'N':{
				//Line number
				currentBlock.blockNumber = num;
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
				currentBlock.arcRadius = 0; //Not implemented
				break;
			}
			case 'S':{
				//Spindle speed
				currentBlock.dispenseRate = num;
				break;
			}
			case 'X':{
				//Position X
				currentBlock.pos.x = Metric2Step(val);
				break;
			}
			case 'Y':{
				//Position Y
				currentBlock.pos.y = Metric2Step(val);
				break;
			}
			case 'Z':{
				//Position Z
				currentBlock.pos.z = Metric2Step(val);
				break;
			}
			default:{
				ReportEvent(NOT_RECOGNIZED, letter);
				return NOT_RECOGNIZED; //Unrecognized command
			}
		}
	}
	return NONE;
}

void InitParser(){
	//Reset buffer indexes
	wordIndex = 0;
	blockBufferHead = 0;
	blockBufferTail = 0;
	
	//Set default block values
	currentBlock.pos.x.full = 0;
	currentBlock.pos.x.micro = 0;
	currentBlock.pos.y.full = 0;
	currentBlock.pos.y.micro = 0;
	currentBlock.pos.z.full = 0;
	currentBlock.pos.z.micro = 0;
	currentBlock.motion = Home;
	currentBlock.dispenseRate = 0;
	currentBlock.moveSpeed = 0;
	currentBlock.dispenseEnable = false;
	currentBlock.dwellTime = 0;
	currentBlock.coordinateMode = absolute;
	currentBlock.coordinateUnit = millimeter;
	
	WriteBlockBuffer(currentBlock);
}

inline bool IgnoreChar(char in){
	
	if (in == ' ')
	{
		return true;
	} else if (in == 0)
	{
		return true;
	} else if (in == '+')
	{
		return true;
	} else if (in == '%')
	{
		return true;
	} else if (in == '/')
	{
		return true;
	} else if ((in == '(') || (in == ';'))
	{
		colons++;
	} else if (in == ')')
	{
		colons--;
	}
	
	//Ignore comments
	if (colons > 0)
	{
		return true;
	}
	return false;
}

inline bool WordEnd(char in){
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
	
	uint8_t tempHead = blockBufferHead + 1;
	if (tempHead >= BLOCK_BUFFER_SIZE)
	{
		tempHead = 0;
	}
	
	if (blockBufferTail == tempHead)
	{
		return BUFFER_FULL;
	}
	return BUFFER_AVAILABLE;
}

void WriteBlockBuffer(gc_block block){
	blockBufferHead++;

	if (blockBufferHead >= BLOCK_BUFFER_SIZE)
	{
		blockBufferHead = 0;
	}

	blockBuffer[blockBufferHead] = block;
}

gc_block ReadBlockBuffer(){
	blockBufferTail++;
	if (blockBufferTail >= BLOCK_BUFFER_SIZE)
	{
		blockBufferTail = 0;
	}
	
	return blockBuffer[blockBufferTail];
}

gc_block PeekBlockBuffer(){
	return blockBuffer[blockBufferTail];
}