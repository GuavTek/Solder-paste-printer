/*
 * Config.h
 *
 * Created: 17-Feb-20 15:57:38
 *  Author: mikda
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_


//Define clock frequencies
#define PRESCALE 6
#define PDIV 8
#define F_CPU 20000000/PRESCALE			// Actually just fCLK__PER
#define fCLK_PER F_CPU


#define BLOCK_BUFFER_SIZE 3

//G-code
#define MAX_G_LINE 0
#define MAX_LINE_SIZE 256
#define STOP_CHAR "\r"
#define MAX_WORD_SIZE 16
#define CMD_X_SIZE 8
#define CMD_Y_SIZE 8
#define CMD_Z_SIZE 4


#endif /* CONFIG_H_ */