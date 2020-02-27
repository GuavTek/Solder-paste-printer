/*
 * USART.h
 *
 * Created: 05-Feb-20 11:44:32
 *  Author: mikda
 */ 


#ifndef USART_H_
#define USART_H_


#define RX_BUFFERSIZE 128
#define TX_BUFFERSIZE 64

// initializes a uart port with the chosen baudrate
void USART_INIT(uint8_t portnum, uint32_t baudrate);

// Handles the received data
void RX_buffer();
uint8_t RX_read();

// Handles the transmitt data
void TX_receive(uint8_t data);
void TX_buffer();


#endif /* USART_H_ */