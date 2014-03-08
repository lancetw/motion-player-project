/*
 * usart.c
 *
 *  Created on: 2011/02/22
 *      Author: masayuki
 */

#include "stm32f4xx_conf.h"
#include <stdio.h>
#include <stdarg.h>
#include "lcd.h"
#include "usart.h"

#include "settings.h"

#include "xmodem.h"

void USART3_IRQHandler(void)
{
    USART_Cmd(USART3, DISABLE);
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	__IO uint16_t recv = USART3->DR;

	resetDimLightTimer();

	if(recv == 'p') {
	    USART_Cmd(USART3, ENABLE);
		xput();
		return;
	}

	if(LCDStatusStruct.waitExitKey != 0){
		if(recv == 'e'){
			LCDStatusStruct.waitExitKey = 0;
		} else {
			LCDStatusStruct.waitExitKey = recv;
		}
	    USART_Cmd(USART3, ENABLE);
		return;
	}

	switch(recv){
		case CURSOR_LEFT:
			LCDPutCursorBar(cursor.pos);
			LCDStoreCursorBar(0);
			cursor.pos = 0, cursor.pageIdx = 0;
		    USART_Cmd(USART3, ENABLE);
			LCDCursorEnter();
			break;
		case CURSOR_UP:
			LCDCursorUp();
			break;
		case CURSOR_DOWN:
			LCDCursorDown();
			break;
		case CURSOR_ENTER:
		    USART_Cmd(USART3, ENABLE);
			LCDCursorEnter();
			break;
		case CURSOR_RIGHT:
			break;
		default:
			break;
	}
    USART_Cmd(USART3, ENABLE);

}

void USARTInit()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the USART3 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// USART3を有効にします
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	// PB10ポート(USART3 TX)
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	// 初期化関数を読み出します。

	// PB11ポート(USART3 RX)
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	// 初期化関数を読み出します。

	// USART設定
	USART_DeInit(USART3);
	USART_InitStructure.USART_BaudRate = settings_group.debug_conf.baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);

    debug.printf = USARTPrintf;
}

void USARTPutData(uint16_t x)
{
	USART_SendData(USART3, x);
	while(!USART_GetFlagStatus(USART3, USART_FLAG_TXE));
}

void USARTPutChar(void *c)
{
	USART_SendData(USART3, *(uint16_t *)c);
	while(!USART_GetFlagStatus(USART3, USART_FLAG_TXE));
}

void USARTPutString(const char *str)
{
	do{
		USARTPutChar((void*)str);
	}while(*(uint8_t *)str++ != '\0');
}

void USARTPutNChar(void *str, uint32_t n)
{
	while(n--) USARTPutChar((uint8_t *)str++);
}

int USARTPrintf(const char *fmt, ...)
{
	static char s[100];
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsprintf(s, fmt, ap);
	USARTPutString(s);
	va_end(ap);

	return ret;
}


