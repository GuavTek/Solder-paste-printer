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

//Set all flags false
void ResetBlockState();

//Will sort out modal values as needed, then write block to buffer
inline void PushBlock();

//Write to block buffer
void WriteBlockBuffer();

//Will parse and insert command-values in gc-block
ReturnCodes ParseWord();

//Flags indicating which parameters have been written
struct BlockState {
	bool xPos : 1;
	bool yPos : 1;
	bool zPos : 1;
	bool xArc : 1;
	bool yArc : 1;
	bool zArc : 1;
	bool arcRadius : 1;
	bool mode : 1;
	bool coordMode : 1;
	bool coordUnit : 1;
	bool dispenseEnable : 1;
	bool dispenseRate : 1;
	bool blockNumber : 1;
	bool moveSpeed : 1;
	bool param : 1;
};

struct BlockState blockState;

uint8_t blockBufferHead = 0;
uint8_t blockBufferTail = 0;
gc_block blockBuffer[BLOCK_BUFFER_SIZE];

uint8_t colons;						//For ignoring comments

gc_block inputBlock;				//The block being edited
gc_block modalBlock;				//Saves modal block-values
int parameter = 0;					//P-value of blocks
uint8_t wordIndex = 0;				//How long the word being parsed is now
bool freshBlock = true;				//True if currentblock contains no new info
char currentWord[MAX_WORD_SIZE];	//Buffer for the word being parsed
bool readyBlock = false;			//True when a block is waiting to be put in buffer
bool modal = true;					//Decides which block to place in buffer

int* selectedAxis;					//Used to increment axisOffset
Vector3 axisOffset;					//Current offset
Vector3 localCoordSystem;			//Offset relative to current WCS
Vector3 workCoordSystems[6];		//WCS, set of reference points
uint8_t selWCS = 0;					//Currently selected WCS

ReturnCodes ParseStream(){
	
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
			readyBlock = false;
			WriteBlockBuffer();
			return NONE;
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
		//Parse the last word on line
		if(ParseWord() == NONE){
			freshBlock = false;
		}
		wordIndex = 0;	

		if(freshBlock == false){
			//Process the new block and place it in buffer
			colons = 0;
			freshBlock = true;

			PushBlock();

			ResetBlockState();
			ReportEvent(NEW_BLOCK, inputBlock.blockNumber);
			return NEW_BLOCK;
		}
	}
	
	//Checks if a new word has started
	if (WordEnd(nextChar)){
		if(ParseWord() == NONE){
			//Indicate new info if there are no errors
			freshBlock = false;
		}
		wordIndex = 0;
	}
	
	//Detect word overflow
	if (wordIndex >= MAX_WORD_SIZE)
	{
		ReportEvent(BUFFER_OVERFLOW, 'W');
	} else {
		currentWord[wordIndex] = nextChar;
		wordIndex++;
	}
	return NONE;
}

ReturnCodes ParseWord(){
	char letter = currentWord[0];
	float val = 0;
	int num;
	//uint16_t fraction = 0;
	//uint8_t precision = 0;
	
	
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
				inputBlock.moveSpeed = num;
				blockState.moveSpeed = true;
				break;
			}
			case 'G':{
				//Prep commands
				switch(num){
					case 0: {
						inputBlock.motion = Rapid_position;
						blockState.mode = true;
						break;
					}
					case 1: {
						inputBlock.motion = Linear_interpolation;
						blockState.mode = true;
						break;
					}
					case 2: {
						inputBlock.motion = Arc_CW;
						blockState.mode = true;
						break;
					}
					case 3: {
						inputBlock.motion = Arc_CCW;
						blockState.mode = true;
						break;
					}
					case 4: {
						inputBlock.motion = Dwell;
						blockState.mode = true;
						//inputBlock.dwellTime = parameter;
						break;
					}
					case 10: {
						//Write to current WCS
						inputBlock.motion = Offset_WCS;
						blockState.mode = true;
						break;
					}
					case 17: {
						//XY plane selected
						selectedAxis = &axisOffset.z;
						break;
					}
					case 18: {
						//ZX plane selected
						selectedAxis = &axisOffset.y;
						break;
					}
					case 19: {
						//YZ plane selected
						selectedAxis = &axisOffset.x;
						break;
					}
					case 20: {
						inputBlock.coordinateUnit = Inch;
						blockState.coordUnit = true;
						break;
					}
					case 21: {
						inputBlock.coordinateUnit = millimeter;
						blockState.coordUnit = true;
						break;
					}
					case 28: {
						inputBlock.motion = Home;
						blockState.mode = true;
						break;
					}
					case 45: {
						//Axis offset +1
						*selectedAxis += 1;
						break;
					}
					case 46: {
						//Axis offset -1
						*selectedAxis -= 1;
						break;
					}
					case 47: {
						//Axis offset +2
						*selectedAxis += 2;
						break;
					}
					case 48: {
						//Axis offset -2
						*selectedAxis -= 2;
						break;
					}
					case 50: {
						//Position register offset, currently only works with absolute positions
						inputBlock.motion = Offset_posReg;
						blockState.mode = true;
						break;
					}
					case 52: {
						//Local coordinate system
						inputBlock.motion = Offset_LCS;
						blockState.mode = true;
						break;
					}
					case 54: case 55: case 56: case 57: case 58: case 59: {
						//Work coordinate system
						selWCS = num - 54;
						inputBlock.motion = Offset_Sel;	//Set to LCS to recalculate axisOffset
						blockState.mode = true;
						break;
					}
					case 90: {
						inputBlock.coordinateMode = absolute;
						blockState.coordMode = true;
						break;
					}
					case 91: {
						inputBlock.coordinateMode = incremental;
						blockState.coordMode = true;
						break;
					}
				}
				break;
			}
			
			case 'I':{
				//Arc center X
				inputBlock.arcCentre.x = Length2Step(val, inputBlock.coordinateUnit);
				blockState.xArc = true;
				break;	
			}
			case 'J':{
				//Arc center Y
				inputBlock.arcCentre.y = Length2Step(val, inputBlock.coordinateUnit);
				blockState.yArc = true;
				break;
			}
			case 'K':{
				//Arc center Z
				inputBlock.arcCentre.z = LengthZ2Step(val, inputBlock.coordinateUnit);
				blockState.zArc = true;
				break;	
			}
			case 'M':{
				//Machine commands
				switch(num){
					case 0: {
						//Compulsory stop (Pause)
						inputBlock.motion = Pause;
						blockState.mode = true;
						break;
					}
					case 1: {
						//Optional stop (Pause)
						if (OPTIONAL_STOP)
						{
							inputBlock.motion = Pause;
							blockState.mode = true;
						}
						break;
					}
					case 2: {
						//End of program
						inputBlock.motion = Stop;
						blockState.mode = true;
						break;
					}
					case 3: case 4: {
						//Spindle (dispenser) on
						inputBlock.dispenseEnable = true;
						blockState.dispenseEnable = true;
						break;
					}
					case 5: {
						//Spindle (dispenser) off
						inputBlock.dispenseEnable = false;
						blockState.dispenseEnable = true;
						break;
					}
					case 30: {
						//End of program, return to program top
						inputBlock.motion = Stop;
						blockState.mode = true;
						break;
					}
				}
				break;
			}
			case 'N':{
				//Line number
				inputBlock.blockNumber = num;
				blockState.blockNumber = true;
				return NONE;
			}
			case 'O':{
				//Program name
				
				break;
			}
			case 'P':{
				//Parameter for G and M
				parameter = num;
				blockState.param = true;
				break;
			}
			case 'R':{
				//Arc radius
				inputBlock.arcRadius = 0; //Not implemented
				blockState.arcRadius = true;
				break;
			}
			case 'S':{
				//Spindle speed
				inputBlock.dispenseRate = num;
				blockState.dispenseRate = true;
				break;
			}
			case 'X':{
				//Position X
				inputBlock.pos.x = Length2Step(val, inputBlock.coordinateUnit);
				blockState.xPos = true;
				break;
			}
			case 'Y':{
				//Position Y
				inputBlock.pos.y = Length2Step(val, inputBlock.coordinateUnit);
				blockState.yPos = true;
				break;
			}
			case 'Z':{
				//Position Z
				inputBlock.pos.z = LengthZ2Step(val, inputBlock.coordinateUnit);
				blockState.zPos = true;
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

void ResetBlockState(){
	blockState.xPos = false;
	blockState.yPos = false;
	blockState.zPos = false;
	blockState.xArc = false;
	blockState.yArc = false;
	blockState.zArc = false;
	blockState.arcRadius = false;
	blockState.mode = false;
	blockState.coordMode = false;
	blockState.coordUnit = false;
	blockState.dispenseEnable = false;
	blockState.dispenseRate = false;
	blockState.blockNumber = false;
	blockState.moveSpeed = false;
	blockState.param = false;
}

void InitParser(){
	//Reset buffer indexes
	wordIndex = 0;
	blockBufferHead = 0;
	blockBufferTail = 0;
	
	//Reset offset
	workCoordSystems[0].x = STD_OFFSET_X;
	workCoordSystems[0].y = STD_OFFSET_Y;
	workCoordSystems[0].z = STD_OFFSET_Z;
	selWCS = 0;
	axisOffset = workCoordSystems[0];

	//Reset modal values to default
	modalBlock.pos.x.full = STD_OFFSET_X;
	modalBlock.pos.x.micro = 0;
	modalBlock.pos.y.full = STD_OFFSET_Y;
	modalBlock.pos.y.micro = 0;
	modalBlock.pos.z.full = STD_OFFSET_Z;
	modalBlock.pos.z.micro = 0;
	modalBlock.motion = Rapid_position;
	modalBlock.dispenseRate = 0;
	modalBlock.moveSpeed = 5;
	modalBlock.dispenseEnable = false;
	modalBlock.dwellTime = 0;
	modalBlock.coordinateMode = absolute;
	modalBlock.coordinateUnit = millimeter;

	//Send printer head home
	inputBlock.motion = Home;
	
	PushBlock();
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
	} else if (in == '\n' || in == '\r')
	{
		return false;
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

inline void PushBlock(){
	
	//Check if it is a modal block or not
	if (inputBlock.motion == Offset_WCS)
	{

		//Set work coordinates
		if (blockState.xPos)
		{
			workCoordSystems[selWCS].x = inputBlock.pos.x.full;
		} else {
			workCoordSystems[selWCS].x = 0;
		}
		if (blockState.yPos)
		{
			workCoordSystems[selWCS].y = inputBlock.pos.y.full;
		} else {
			workCoordSystems[selWCS].y = 0;
		}
		if (blockState.zPos)
		{
			workCoordSystems[selWCS].z = inputBlock.pos.z.full;
		} else {
			workCoordSystems[selWCS].z = 0;
		}

		//Reset LCS
		localCoordSystem.x = 0;
		localCoordSystem.y = 0;
		localCoordSystem.z = 0;

		//Set current offset
		axisOffset = workCoordSystems[selWCS];

		//Reset motionMode
		inputBlock.motion = modalBlock.motion;

	} else if (inputBlock.motion == Offset_posReg)
	{
		//Set current offset
		if (blockState.xPos)
		{
			axisOffset.x = inputBlock.pos.x.full;
		} else {
			axisOffset.x = 0;
		}
		if (blockState.yPos)
		{
			axisOffset.y = inputBlock.pos.y.full;
		} else {
			axisOffset.y = 0;
		}
		if (blockState.zPos)
		{
			axisOffset.z = inputBlock.pos.z.full;
		} else {
			axisOffset.z = 0;
		}

		//Reset motionMode
		inputBlock.motion = modalBlock.motion;

	} else if (inputBlock.motion == Offset_LCS)
	{
		//Set local offset
		if (blockState.xPos)
		{
			localCoordSystem.x = inputBlock.pos.x.full;
		} else {
			localCoordSystem.x = 0;
		}
		if (blockState.yPos)
		{
			localCoordSystem.y = inputBlock.pos.y.full;
		} else {
			localCoordSystem.y = 0;
		}
		if (blockState.zPos)
		{
			localCoordSystem.z = inputBlock.pos.z.full;
		} else {
			localCoordSystem.z = 0;
		}

		//Set current offset
		axisOffset.x = workCoordSystems[selWCS].x + localCoordSystem.x;
		axisOffset.y = workCoordSystems[selWCS].y + localCoordSystem.y;
		axisOffset.z = workCoordSystems[selWCS].z + localCoordSystem.z;

		//Reset motionMode
		inputBlock.motion = modalBlock.motion;

	} else if (inputBlock.motion == Offset_Sel) {
		//Reset LCS
		localCoordSystem.x = 0;
		localCoordSystem.y = 0;
		localCoordSystem.z = 0;

		//Set current offset
		axisOffset = workCoordSystems[selWCS];

		//Reset motionMode
		inputBlock.motion = modalBlock.motion;

	} else if (inputBlock.motion == Dwell)
	{
		if (blockState.param)
		{
			inputBlock.dwellTime = parameter;
			modal = false;
			WriteBlockBuffer();
		} else {
			//Error, No time set?
		}

		//Reset motionMode
		inputBlock.motion = modalBlock.motion;

	} else if (inputBlock.motion == Home)
	{
		modal = false;
		WriteBlockBuffer();
	} else {
		//Modal operations

		//Save modal values
		if (blockState.coordMode) {
			modalBlock.coordinateMode = inputBlock.coordinateMode;
		}

		if (blockState.coordUnit) {
			modalBlock.coordinateUnit = inputBlock.coordinateUnit;
		}
		
		if (blockState.xPos) {
			modalBlock.pos.x = inputBlock.pos.x;
			if (modalBlock.coordinateMode == absolute) {
				modalBlock.pos.x.full += axisOffset.x;
			}
		} else if(modalBlock.coordinateMode == incremental) {
			modalBlock.pos.x.full = 0;
			modalBlock.pos.x.micro = 0;
		}

		if (blockState.yPos) {
			modalBlock.pos.y = inputBlock.pos.y;
			if (modalBlock.coordinateMode == absolute) {
				modalBlock.pos.y.full += axisOffset.y;
			}
		} else if(modalBlock.coordinateMode == incremental) {
			modalBlock.pos.y.full = 0;
			modalBlock.pos.y.micro = 0;
		}

		if (blockState.zPos) {
			modalBlock.pos.z = inputBlock.pos.z;
			if(modalBlock.coordinateMode == absolute) {
				modalBlock.pos.z.full += axisOffset.z;
			}
		} else if(modalBlock.coordinateMode == incremental) {
			modalBlock.pos.z.full = 0;
			modalBlock.pos.z.micro = 0;
		}	
		
		if (blockState.mode) {
			modalBlock.motion = inputBlock.motion;
		}

		if (blockState.dispenseEnable)
		{
			modalBlock.dispenseEnable = inputBlock.dispenseEnable;
		} 

		if (blockState.dispenseRate)
		{
			modalBlock.dispenseRate = inputBlock.dispenseRate;
		} 

		if (blockState.blockNumber)
		{
			modalBlock.blockNumber = inputBlock.blockNumber;
		}

		if (blockState.moveSpeed)
		{
			modalBlock.moveSpeed = inputBlock.moveSpeed;
		} 

		//Put the block into buffer
		modal = true;
		WriteBlockBuffer();
	}
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

void WriteBlockBuffer(){
	if(BlockBufferAvailable() == BUFFER_FULL){
		ReportEvent(BUFFER_FULL, 'B');
		readyBlock = true;
		return;
	}
	
	blockBufferHead++;

	if (blockBufferHead >= BLOCK_BUFFER_SIZE)
	{
		blockBufferHead = 0;
	}

	if (modal)
	{
		blockBuffer[blockBufferHead] = modalBlock;
	} else {
		blockBuffer[blockBufferHead] = inputBlock;
	}

	//Set position to local zero if homeing
	if (inputBlock.motion == Home)
	{
		inputBlock.pos.x.full = axisOffset.x;
		inputBlock.pos.y.full = axisOffset.y;
		inputBlock.pos.z.full = axisOffset.z;
		inputBlock.motion = Rapid_position;
		readyBlock = true;
		modal = false;
	} else {
		readyBlock = false;
	}
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