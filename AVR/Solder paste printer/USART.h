/*
 * USART.h
 *
 * Created: 05-Feb-20 11:44:32
 *  Author: mikda
 */ 


#ifndef USART_H_
#define USART_H_

void USART_INIT(uint8_t portnum, uint32_t baudrate);
void USB_RX();
void USB_TX();

extern int move;

#endif /* USART_H_ */