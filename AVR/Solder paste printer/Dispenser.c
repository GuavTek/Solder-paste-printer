/*
 * Dispenser.c
 *
 * Created: 10-Mar-20 10:56:51
 *  Author: mikda
 */ 

#include "Header.h"

bool lastDispensing = false;


void Dispense(bool isDispensing){
	
	
	//Return if there is no change
	if (lastDispensing == isDispensing)
	{
		return;
	} else {
		lastDispensing = isDispensing;
	}
	
	if (isDispensing)
	{
		//Start dispensing
	} else {
		//Stop dispensing
	}
	
}

void InitDispenser(){
	
}