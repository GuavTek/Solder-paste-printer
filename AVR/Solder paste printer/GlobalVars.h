/*
 * GlobalVars.h
 *
 * Created: 05-Feb-20 12:21:58
 *  Author: mikda
 */ 

#include <avr/io.h>

#ifndef GLOBALVARS_H_
#define GLOBALVARS_H_

//Define clock frequencies
#define F_CPU 20000000
#define PRESCALE 6
#define PDIV 8

/*
#if PRESCALE==2
	#define PDIV 0
#elif PRESCALE==4
	#define PDIV 1
#elif PRESCALE==8
	#define PDIV 2
#elif PRESCALE==16
	#define PDIV 3
#elif PRESCALE==32
	#define PDIV 4
#elif PRESCALE==64
	#define PDIV 5
#elif PRESCALE==6
	#define PDIV 8
#elif PRESCALE==10
	#define PDIV 9
#elif PRESCALE==12
	#define PDIV 10
#elif PRESCALE==24
	#define PDIV 11
#elif PRESCALE==48
	#define PDIV 12
#endif
*/

#define fCLK_PER (F_CPU / PRESCALE)

#endif /* GLOBALVARS_H_ */