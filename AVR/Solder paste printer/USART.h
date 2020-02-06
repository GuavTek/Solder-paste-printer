/*
 * USART.h
 *
 * Created: 05-Feb-20 11:44:32
 *  Author: mikda
 */ 


#ifndef USART_H_
#define USART_H_

// initializes a uart port with the chosen baudrate
void USART_INIT(uint8_t portnum, uint32_t baudrate);

// Handles the received data
void USB_RX();

// Does nothing (yet)
void USB_TX();

extern int move;

#endif /* USART_H_ */