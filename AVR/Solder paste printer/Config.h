/*
 * Config.h
 *
 * Created: 17-Feb-20 15:57:38
 *  Author: mikda
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define VERBOSE_FEEDBACK true

//Define clock frequencies
#define PRESCALE 6
#define PDIV 8
#define F_CPU 20000000/PRESCALE			// Actually just fCLK__PER
#define fCLK_PER F_CPU

//USB com port
#define USARTn USART3

#define BLOCK_BUFFER_SIZE 8
#define RX_BUFFERSIZE 128
#define TX_BUFFERSIZE 64

#define METRIC_STEP_LENGTH 0.01495;	//Length of a step in millimeters

//G-code
#define MAX_G_LINE 0
#define MAX_LINE_SIZE 256
#define STOP_CHAR "\r"
#define MAX_WORD_SIZE 16
#define CMD_X_SIZE 8
#define CMD_Y_SIZE 8
#define CMD_Z_SIZE 4
#define CMD_RESET '@'
#define CMD_STATUS_REPORT '€'
#define CMD_CYCLE_START '¤'

#endif /* CONFIG_H_ */