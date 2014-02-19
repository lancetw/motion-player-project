/*
 * xpt2046.c
 *
 *  Created on: 2012/03/05
 *      Author: Tonsuke
 */

#include "xpt2046.h"
#include "usart.h"
#include "lcd.h"

#include "board_config.h"

#include "settings.h"

#include <string.h>


volatile touch_typedef touch;

void touch_empty_func(){
	// do nothing
}

uint16_t GetAxis(uint8_t control){
	uint16_t ret;

	XPT2046_CS_ASSERT;
	while(!SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)); // TXが空になるまで待つ

	SPI_I2S_SendData(SPI1, control);
	while(!SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)); // TXが空になるまで待つ

	SPI_I2S_ReceiveData(SPI1);
	SPI_I2S_SendData(SPI1, 0);
	while(!SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)); // TXが空になるまで待つ

	SPI_I2S_ReceiveData(SPI1);
	SPI_I2S_SendData(SPI1, 0);
	while(!SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)); // TXが空になるまで待つ

	ret = SPI_I2S_ReceiveData(SPI1);

	XPT2046_CS_DEASSERT;

	return ret;
}

void resetDimLightTimer()
{
	time.prevTime = time.curTime;
	if(time.flags.dimLight){
		TIM_SetCompare2(TIM4, (int)(1000 * (float)settings_group.disp_conf.brightness / 100.0f) - 1);
		time.flags.dimLight = 0;
	}
}

void EXTI4_IRQHandler(void) // PINIRQピンの立ち下がりエッジ検出割込みハンドラ
{
	TOUCH_PINIRQ_DISABLE; // 外部割込みLine4不許可
	EXTI_ClearFlag(EXTI_Line4); // 外部割込みLine4フラグクリア

	resetDimLightTimer();

	if(time.flags.stop_mode){
		TOUCH_PINIRQ_ENABLE; // 外部割込みLine4許可
		TIM_Cmd(TIM5, DISABLE); // TIM5割込み不許可
		return;
	}

	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	TIM_Cmd(TIM5, ENABLE); // TIM5割込み許可　チャタリング処理
}

void DrawBrush(){
	LCDDrawSquare(touch.posX, touch.posY, 5, 5, WHITE);
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM9, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM9, TIM_IT_Update);

		if(!touch.update){
			TIM_Cmd(TIM9, DISABLE);
			TIM_SetCounter(TIM9, 0);
			TIM_ITConfig(TIM9, TIM_IT_Update, DISABLE);

			LCDTouchReleased();
		}
		touch.update = 0;
	}
}

void TIM5_IRQHandler(void) // チャタリング処理 50msec待ち後
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

	if(TP_PEN_INPUT_BB == Bit_SET){
		goto END_PROCESS; // PINIRQが無効だった場合終了処理
	}

	//debug.printf("\r\nTouched");

	if(touch.calend){
		touch.aveX = touch.aveY = 0;
		touch.cnt = 0;
	}

	TIM_SetCounter(TIM9, 0);
	TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM9, ENABLE);

	while((TP_PEN_INPUT_BB == Bit_RESET) && touch.calend) {
		touch.adX = GET_X_AXIS();
		touch.adY = GET_Y_AXIS();

		touch.aveX += ((int)(touch.adX - touch.cal->x[2]) * 10) / touch.cal->xStep + 15;
		touch.aveY += ((int)(touch.adY - touch.cal->y[0]) * 10) / touch.cal->yStep + 15;

		if(++touch.cnt >= 600){
			touch.posX = touch.aveX / 600;
			touch.posY = touch.aveY / 600;

			if(touch.posX < 0) touch.posX = 0;
			if(touch.posX > 319) touch.posX = 319;
			if(touch.posY < 0) touch.posY = 0;
			if(touch.posY > 239) touch.posY = 239;

			touch.func();
			touch.update = 1;

			touch.aveX = touch.aveY = 0;
			touch.cnt = 0;
		}
	}
	touch.click = 1;

	if(!touch.calend){ // キャリブレート処理
		touch.cal->x[touch.cnt] = GET_X_AXIS();
		touch.cal->y[touch.cnt] = GET_Y_AXIS();

		if(touch.cnt++ >= 3){
			touch.cal->xStep = ((int)(touch.cal->x[3] - touch.cal->x[2]) * 10) / 289;
			touch.cal->yStep = ((int)(touch.cal->y[1] - touch.cal->y[0]) * 10) / 209;

			debug.printf("\r\ncalXstep:%d", touch.cal->xStep);
			debug.printf("\r\ncalYstep:%d", touch.cal->yStep);
			debug.printf("\r\ncalX[0]:%d", touch.cal->x[0]);
			debug.printf("\r\ncalX[1]:%d", touch.cal->x[1]);
			debug.printf("\r\ncalX[2]:%d", touch.cal->x[2]);
			debug.printf("\r\ncalX[3]:%d", touch.cal->x[3]);

			debug.printf("\r\ncalY[0]:%d", touch.cal->y[0]);
			debug.printf("\r\ncalY[1]:%d", touch.cal->y[1]);
			debug.printf("\r\ncalY[2]:%d", touch.cal->y[2]);
			debug.printf("\r\ncalY[3]:%d", touch.cal->y[3]);

			touch.calend = 1;
			touch.aveX = touch.aveY = 0;
			touch.func = touch_empty_func;
		}
	}

END_PROCESS:
	TOUCH_PINIRQ_ENABLE; // 外部割込みLine4許可
	TIM_Cmd(TIM5, DISABLE); // TIM5割込み不許可
}

void TouchPenReleaseIRQ_Disable()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TouchPenReleaseIRQ_Enable()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void TouchPenIRQ_Disable()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TouchPenIRQ_Enable()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TouchPanel_Calibration()
{

	LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);

	touch.calend = 0;
	touch.cnt = 0;

	TIM_Cmd(TIM9, DISABLE);
	TIM_SetCounter(TIM9, 0);
	TIM_ClearITPendingBit(TIM9, TIM_IT_Update);

	LCDDrawSquare(159, 10, 5, 5, RED); // Upper Center
	while(touch.cnt < 1);
	LCDDrawSquare(159, 10, 5, 5, GREEN);

	LCDDrawSquare(159, 225, 5, 5, RED); // Lower Center
	while(touch.cnt < 2);
	LCDDrawSquare(159, 225, 5, 5, GREEN);

	LCDDrawSquare(10, 119, 5, 5, RED); // Left Center
	while(touch.cnt < 3);
	LCDDrawSquare(10, 119, 5, 5, GREEN);

	LCDDrawSquare(305, 119, 5, 5, RED); // Right Center
	while(touch.cnt < 4);
	LCDDrawSquare(305, 119, 5, 5, GREEN);

	touch.cnt = 0;
	touch.click = 0;
	touch.update = 0;

	TIM_Cmd(TIM9, DISABLE);
	TIM_SetCounter(TIM9, 0);
	TIM_ClearITPendingBit(TIM9, TIM_IT_Update);

	SETTINGS_Save();

	LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);
}


void XPT2046Init()
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); // チャタリング対策
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE); // タッチインターバル監視

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

	// GPIOA PA0(USER SWITCH)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// GPIOA PA15(SPI1_NSS)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// GPIOB PB3(SPI1_SCK) PB4(SPI1_MISO) PB5(SPI1_MOSI)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); // SPI1_SCK
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); // SPI1_MISO
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); // SPI1_MOSI


	// GPIOC PC4(XPT2046_PINIRQ)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource4);

	// PINIRQピンの外部割込み要因：立ち下がりエッジ
//	EXTI_DeInit();
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* 外部割込み設定 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM5割込み設定 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM5設定 チャタリング対策 50ms
	TIM_TimeBaseInitStructure.TIM_Period = 50 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = ((SystemCoreClock / 4) * 2) / 1000 - 1; // 1ms
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5, DISABLE);


	/* TIM9割込み設定 */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 100ms
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = SystemCoreClock / 1000 - 1; // 1ms
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM9, &TIM_TimeBaseInitStructure);
	TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
	TIM_SetCounter(TIM9, 0);
	TIM_ITConfig(TIM9, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM9, DISABLE);


	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE, ENABLE);
	SPI_Cmd(SPI1, ENABLE);

	XPT2046_CS_DEASSERT;

	touch.cnt = 0;
	touch.click = 0;
	touch.calend = 0;
	touch.update = 0;

	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
		touch.calend = 0;

		TouchPenIRQ_Enable();
		TouchPenReleaseIRQ_Disable();

		TouchPanel_Calibration();
	} else {
		touch.calend = 1;
	}
/*
	if(!touch.calend){
		TouchPenIRQ_Enable();
		TouchPenReleaseIRQ_Disable();

		LCDDrawSquare(159, 10, 5, 5, RED); // Upper Center
		while(touch.cnt < 1);
		LCDDrawSquare(159, 10, 5, 5, GREEN);

		LCDDrawSquare(159, 225, 5, 5, RED); // Lower Center
		while(touch.cnt < 2);
		LCDDrawSquare(159, 225, 5, 5, GREEN);

		LCDDrawSquare(10, 119, 5, 5, RED); // Left Center
		while(touch.cnt < 3);
		LCDDrawSquare(10, 119, 5, 5, GREEN);

		LCDDrawSquare(305, 119, 5, 5, RED); // Right Center
		while(touch.cnt < 4);
		LCDDrawSquare(305, 119, 5, 5, GREEN);

		touch.cnt = 0;

		SETTINGS_Save();
	}
	*/
}
