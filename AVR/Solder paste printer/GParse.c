/*
 * GParse.c
 *
 * Created: 17-Feb-20 16:01:19
 *  Author: mikda
 */ 

#include "Header.h"


uint8_t ParseLine(const char* buff[]){
	static gc_block currentBlock;
	static char currentWord[MAX_WORD_SIZE];
	
	return 0;
}

uint8_t ParseWord(const char* wrd[], gc_block* block){
	char letter = *wrd[0];
	int num = 0;
	uint8_t fraction = 0;
	uint8_t precision = 0;
	
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
				
			case 'F':{
				//Feedrate
				
				break;
			}
			case 'G':{
				//Prep commands
				
				break;
			}
			case 'I':{
				//Arc center X
				
				break;	
			}
			case 'J':{
				//Arc center Y
				
				break;
			}
			case 'K':{
				//Arc center Z
				
				break;	
			}
			case 'M':{
				//Machine commands
				
				break;
			}
			case 'N':{
				//Line number
				
				break;
			}
			case 'O':{
				//Program name
				
				break;
			}
			case 'P':{
				//Parameter for G and M
				
				break;
			}
			case 'R':{
				//Arc radius
				
				break;
			}
			case 'S':{
				//Spindle speed
				
				break;
			}
			case 'X':{
				//Position X
				
				break;
			}
			case 'Y':{
				//Position Y
				
				break;
			}
			case 'Z':{
				//Position Z
				
				break;
			}
			default:{
				return 1; //Unrecognized command
			}
		}
	}
	return 0;	//No problem
}