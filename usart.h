/*
 * usart.h
 *
 *  Created on: 2011/02/22
 *      Author: masayuki
 */

#ifndef USART_H_
#define USART_H_

#include "stm32f4xx_conf.h"

#define CURSOR_LEFT  0x01
#define CURSOR_RIGHT 0x06
#define CURSOR_UP    0x10
#define CURSOR_DOWN  0x0e
#define CURSOR_ENTER 0x0D

#define USART_IRQ_ENABLE USART_ITConfig(USART3, USART_IT_RXNE, ENABLE)
#define USART_IRQ_DISABLE USART_ITConfig(USART3, USART_IT_RXNE, DISABLE)


volatile struct {
	int (*printf)(const char *, ...);
} debug;


extern void USARTInit(void);
extern void USARTPutData(uint16_t x);
extern void USARTPutChar(void *c);
extern void USARTPutString(const char *str);
extern void USARTPutNChar(void *str, uint32_t n);
int USARTPrintf(const char *fmt, ...);


#endif /* USART_H_ */
