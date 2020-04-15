/*
 * Config.h
 *
 * Created: 17-Feb-20 15:57:38
 *  Author: mikda
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

//Enables some extra feedback from printer
#define VERBOSE_FEEDBACK true

//Define clock frequencies
#define PRESCALE 6
#define PDIV 8
#define F_CPU 20000000/PRESCALE			// Actually just fCLK__PER
#define fCLK_PER F_CPU
#define fCLK_MMS METRIC_STEP_LENGTH * fCLK_PER

//USB com port
#define USARTn USART3

//Buffer sizes
#define BLOCK_BUFFER_SIZE 64
#define RX_BUFFERSIZE 128
#define TX_BUFFERSIZE 64

#define METRIC_STEP_LENGTH 0.01495	//Length of a step in millimeters
#define IMPERIAL_STEP_LENGTH 0.0005886	//Length of a step in freedom eagles

//axis offset from edge sensors in full-steps
#define STD_OFFSET_X 100
#define STD_OFFSET_Y 100
#define STD_OFFSET_Z 10

//G-code parameters
#define MAX_G_LINE 0
#define MAX_LINE_SIZE 256
#define MAX_WORD_SIZE 16			//Max characters in a command
#define CMD_X_SIZE 8
#define CMD_Y_SIZE 8
#define CMD_Z_SIZE 4
#define OPTIONAL_STOP true

//Defines which ASCII chars have special functions
#define START_CHAR '%'
#define CMD_RESET '@'
#define CMD_STATUS_REPORT '?'
#define CMD_CYCLE_START '~'
#define CMD_ABORT 0x18

#endif /* CONFIG_H_ */