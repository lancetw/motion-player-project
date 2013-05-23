/*
 * xpt2046.h
 *
 *  Created on: 2012/03/05
 *      Author: Tonsuke
 */

#ifndef XPT2046_H_
#define XPT2046_H_

#include "stm32f4xx_conf.h"
#include "delay.h"

#include "board_config.h"


#define TOUCH_PINIRQ_ENABLE	 (EXTI->IMR |=  _BV(4)) // 外部割込みLine4許可TIM5_IRQn
#define TOUCH_PINIRQ_DISABLE (EXTI->IMR &= ~_BV(4)) // 外部割込みLine4不許可

typedef struct{
	int xStep, yStep;
	uint32_t x[4], y[4];
}touch_calibrate_typedef;

typedef struct{
	touch_calibrate_typedef *cal;
	int cnt, adX, adY, aveX, aveY, posX, posY;
	int8_t calend, click, update, repeat;
	void (*func)(void);
}touch_typedef;

extern volatile touch_typedef touch;

extern void XPT2046Init();
extern void TouchPanel_Calibration();
extern uint16_t GetAxis(uint8_t control);
extern void touch_empty_func();
extern void TouchPenReleaseIRQ_Disable();
extern void TouchPenReleaseIRQ_Enable();
extern void TouchPenIRQ_Disable();
extern void TouchPenIRQ_Enable();

#define GET_X_AXIS() GetAxis(0b10010000)
#define GET_Y_AXIS() GetAxis(0b11010000)


#endif /* XPT2046_H_ */
