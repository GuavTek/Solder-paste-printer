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

// Handles the received data buffer
void RX_write();
uint8_t RX_read();
ReturnCodes RX_available();
uint8_t RX_Count();

// Handles the transmitt data buffer
void TX_write(uint8_t data);
void TX_read();
ReturnCodes TX_available();

// Flush all values from RX and TX buffers
void RTX_FLUSH();

#endif /* USART_H_ */