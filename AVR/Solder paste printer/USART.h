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

//Writes data from usart into RX buffer
void RX_write();

//Read from RX buffer
uint8_t RX_read();

//Check the status of RX buffer
ReturnCodes RX_available();

//Returns available space in RX buffer
uint8_t RX_Count();

//Write data to TX buffer
void TX_write(uint8_t data);

//Read data from TX buffer
void TX_read();

//Check the status of TX buffer
ReturnCodes TX_available();

//Start sending if there is data, and we are not sending
void TX_Jumpstart();

// Flush all values from RX and TX buffers
void RTX_FLUSH();

#endif /* USART_H_ */