/*
 * Feedback.c
 *
 * Created: 02-Mar-20 09:53:19
 *  Author: mikda
 */ 

#include "Header.h"

void ReportStatus(ReturnCodes code, int num){
	if (TX_available() == BUFFER_FULL)
	{
		//Oof
		return;
	}
	switch (code){
		case NOT_RECOGNIZED: {
			TX_write('N');
			TX_write(num);	//Should be letter of G-code, is ASCII
			break;
		} 
		case BUFFER_OVERFLOW: {
			TX_write('O');
			TX_write(num);
			break;
		}
		case BUFFER_FULL: {
			TX_write('F');
			TX_write(num);
			break;
		} 
		case BUFFER_AVAILABLE: {
			TX_write('A');
			break;
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
			break;
		} 
		case SHORT_WORD: {
			TX_write('H');
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