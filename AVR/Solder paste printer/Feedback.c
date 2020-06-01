/*
 * Feedback.c
 *
 * Created: 02-Mar-20 09:53:19
 *  Author: mikda
 */ 

#include "Header.h"

void ReportEvent(ReturnCodes code, int num){
	if (TX_available() == BUFFER_FULL)
	{
		//Oof
		return;
	}
	
	//Indicate that the print has halted
	if (!currentState.noError)
	{
		TX_write('!');
	}
	
	switch (code){
		case NOT_RECOGNIZED: {
			TX_write('N');
			TX_write(num);	//Should be letter of G-code, is ASCII
			break;
		} 
		case UNEXPECTED_EDGE: {
			TX_write('E');
			TX_write(num);	//The axis which collided
			break;
		}
		case BUFFER_OVERFLOW: {
			TX_write('O');
			TX_write(num);	//The buffer that overflowed
			break;
		}
		case BUFFER_FULL: {
			TX_write('F');
			TX_write(num);	//The buffer which is full
			break;
		} 
		case BUFFER_AVAILABLE: {
			TX_write('A');
			break;
		} 
		case BUFFER_EMPTY: {
			TX_write('G');
			TX_write(num);
		}
		case STOP_DETECTED: {
			TX_write('S');
			break;
		} 
		case NEW_BLOCK:	{
			if(!VERBOSE_FEEDBACK){
				return;
			}
			TX_write('B');
			if (PARSER_LINENUMBER)
			{
				SendInt(num);
			}
			break;
		} 
		case SHORT_WORD: {
			TX_write('H');
			TX_write(num);	//The character which was received
			break;
		}
		case PAUSED: {
			TX_write('P');
			break;
		}
		case DWELL_FINISHED: {
			if (!VERBOSE_FEEDBACK)
			{
				return;
			}
			TX_write('D');
			break;
		}
		case NO_CALLBACK: {
			TX_write('C');
			TX_write(num);
			break;
		}
		default: {
			return;
		}
	}
	TX_write('\n');
	TX_write('\r');
	
}

void SendInt(int num){
	char tempString[8];
	itoa(num, tempString, 10);
	
	for (uint8_t i = 0; i < 8; i++)
	{
		if (tempString[i] == 0)
		{
			break;
		}
		if (TX_available() == BUFFER_FULL)
		{
			break;
		} else {
			TX_write(tempString[i]);
		}
		
	}
	TX_write('\n');
	TX_write('\r');
}

void ReportStatus(){
	static uint8_t parIndex;
	const uint8_t ReportLen = 10;
	static char strBuff[10];
	
	//Generate report
	if (parIndex == 0)
	{
		strBuff[0] = '#';
		strBuff[1] = 'l' + currentState.state;
		strBuff[2] = currentState.noError ? 'n' : 'y';
		strBuff[3] = 'l' + currentState.task;
		for (uint8_t i = 5; i < 8; i++)
		{
			strBuff[i] = ' ';
		}
		itoa(theCurrentBlock.blockNumber, strBuff + 4, 10);
		strBuff[8] = '\n';
		strBuff[9] = '\r';
	}
	
	
	//Send report
	while (TX_available() != BUFFER_FULL && parIndex < ReportLen)
	{
		TX_write(strBuff[parIndex]);
		parIndex++;
	}
	
	//Finished reporting
	if (parIndex >= ReportLen)
	{
		currentState.statusDump = false;
		parIndex = 0;
	}
}

