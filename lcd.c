/*
 * lcd.c
 *
 *  Created on: 2011/02/19
 *      Author: masayuki
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lcd.h"
#include "pcf_font.h"
#include "xpt2046.h"
#include "fat.h"
#include "usart.h"
#include "sound.h"
#include "delay.h"
#include "mpool.h"

#include "board_config.h"

#include "jerror.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */

#include "sjis2utf16.h"

#include "settings.h"

#include "sd.h"


#undef ENABLE_BGIMG
#ifdef ENABLE_BGIMG
#include "bgimage.h"
#endif

volatile time_typedef time;

volatile cursor_typedef cursor;
volatile LCDStatusStruct_typedef LCDStatusStruct;

LCD_FUNC_typedef LCD_FUNC;

void DMA_ProgressBar_Conf(){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_DeInit(TIM1);
	TIM_TimeBaseInitStructure.TIM_Period = 5 - 1; // 50ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 10000 - 1; // 10ms
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1; // 1us
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);

	TIM_SetCompare3(TIM1, 1);
	TIM_Cmd(TIM1, ENABLE);
	TIM_DMACmd(TIM1, TIM_DMA_CC3, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;

	DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6);

	/*!< DMA2 Channel6 disable */
	DMA_Cmd(DMA2_Stream6, DISABLE);

	DMA_DeInit(DMA2_Stream6);

	/*!< DMA2 Channel6 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_6;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&LCD->RAM;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&progress_circular_bar_16x16x12_buff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = sizeof(progress_circular_bar_16x16x12_buff) / sizeof(progress_circular_bar_16x16x12_buff[0]);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);
	//DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);

	/*!< DMA2 Channel7 enable */
	DMA_Cmd(DMA2_Stream6, ENABLE);
}

void LCDBackLightInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	/* LCD_BKPWM */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

	TIM_DeInit(TIM4);
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1; // 100KHz
	TIM_TimeBaseInitStructure.TIM_Prescaler = ((SystemCoreClock / 4) * 2) / 1000000 - 1; // 1MHz
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_BDTRInitTypeDef bdtr;
	TIM_BDTRStructInit(&bdtr);
	bdtr.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
	TIM_BDTRConfig(TIM1, &bdtr);

	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

	TIM_SetCompare2(TIM4, (int)(1000 * (float)settings_group.disp_conf.brightness / 100.0f) - 1);
}

void LCDBackLightDisable()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_DeInit(TIM4);

	/* LCD_BKPWM */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_7);
}

void LCDBackLightTimerInit(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_DeInit(TIM8);
    /* 1s = 1 / F_CPU(168MHz) * (167 + 1) * (99 + 1) * (9999 + 1)  TIM1フレームレート計測用  */
	TIM_TimeBaseInitStructure.TIM_Period = 9999;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStructure);
	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM8, 0);
	TIM_Cmd(TIM8, ENABLE);
}

void LCDBackLightTimerDeInit(){
	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, DISABLE);
	TIM_DeInit(TIM8);
}


void LCDPeripheralInit(void)
{
	/*-- GPIOs Configuration -----------------------------------------------------*/
	/*
	 +-------------------+--------------------+------------------+------------------+
	 +                       LCD pins assignment                                  +
	 +-------------------+--------------------+------------------+------------------+
	 | PD0  <-> FSMC_D2  |                    | PD11 <-> FSMC_A16| PD7 <-> FSMC_NE1 |
	 | PD1  <-> FSMC_D3  |                    |                  |                  |
	 | PD4  <-> FSMC_NOE | PE7  <-> FSMC_D4   |                  |                  |
	 | PD5  <-> FSMC_NWE | PE8  <-> FSMC_D5   |                  |                  |
	 | PD8  <-> FSMC_D13 | PE9  <-> FSMC_D6   |                  |                  |
	 | PD9  <-> FSMC_D14 | PE10 <-> FSMC_D7   |                  |                  |
	 | PD10 <-> FSMC_D15 | PE11 <-> FSMC_D8   |                  |                  |
	 |                   | PE12 <-> FSMC_D9   |                  |------------------+
	 |                   | PE13 <-> FSMC_D10  |                  |
	 | PD14 <-> FSMC_D0  | PE14 <-> FSMC_D11  |                  |
	 | PD15 <-> FSMC_D1  | PE15 <-> FSMC_D12  |------------------+
	 +-------------------+--------------------+
	   PD3  <-> RESET
	*/

	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	 */

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);


	/* RESET */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* GPIOD Configuration */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  | \
								  GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | \
								  GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* GPIOE Configuration */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | \
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | \
                                  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*-- FSMC Configuration ----------------------------------------------------*/
	p.FSMC_AddressSetupTime = 10; // 8 // 6 // OC 10
	p.FSMC_AddressHoldTime = 0; // 0 // 0 // OC 0
	p.FSMC_DataSetupTime = 10; // 5  // 5 // OC 10
	p.FSMC_BusTurnAroundDuration = 0;  // 0
	p.FSMC_CLKDivision = 0;
	p.FSMC_DataLatency = 0;
	p.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank1_NOR Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);

	time.curTime = 0;
	time.prevTime = 0;
	time.flags.dimLight = 0;
	time.flags.stop_mode = 0;
	time.flags.enable = 1;
}

void MergeCircularProgressBar(int8_t menubar){
	int i, j, k, l;
	float alpha_ratio;
	uint16_t progress_circular_bar_temp[16 * 16];
	uint16_t alpha, data, bgdata, Ra, Ga, Ba, Rb, Gb, Bb, \
	*d = (uint16_t*)progress_circular_bar_16x16x12, \
	*a = (uint16_t*)progress_circular_bar_16x16_alpha;

	k = 0;
	for(j = PROGRESS_CIRCULAR_POS_Y;j < (PROGRESS_CIRCULAR_POS_Y + PROGRESS_CIRCULAR_HEIGHT);j++){
		for(i = PROGRESS_CIRCULAR_POS_X;i < (PROGRESS_CIRCULAR_POS_X + PROGRESS_CIRCULAR_WIDTH);i++){
			progress_circular_bar_temp[k++] = menubar ? menubar_320x22[i + j * 320] : colorc[BLACK];
		}
	}

	k = 0;
	for(l = 0;l < PROGRESS_CIRCULAR_FRAMES;l++){
		a = (uint16_t*)progress_circular_bar_16x16_alpha;
		for(i = 0;i < PROGRESS_CIRCULAR_HEIGHT;i++){
			for(j = 0;j < PROGRESS_CIRCULAR_WIDTH;j++){
				bgdata = progress_circular_bar_temp[k % (PROGRESS_CIRCULAR_WIDTH * PROGRESS_CIRCULAR_HEIGHT)];
				data = *d++, alpha = *a++;
				Ga = (alpha & 0x07e0) >> 5;

				alpha_ratio = (float)((float)Ga / 63.0f);

				// Foreground Image
				Ra = (data & 0xf800) >> 11;
				Ga = (data & 0x07e0) >> 5;
				Ba = (data & 0x001f);

				Ra *= alpha_ratio;
				Ga *= alpha_ratio;
				Ba *= alpha_ratio;

				// Background Image
				Rb = (bgdata & 0xf800) >> 11;
				Gb = (bgdata & 0x07e0) >> 5;
				Bb = (bgdata & 0x001f);

				Rb *= (1.0f - alpha_ratio);
				Gb *= (1.0f - alpha_ratio);
				Bb *= (1.0f - alpha_ratio);

				// Add colors
				Ra += Rb;
				Ga += Gb;
				Ba += Bb;

				progress_circular_bar_16x16x12_buff[k++] = (Ra << 11) | (Ga << 5) | Ba;
			}
		}
	}
}

void LCDCheckPattern(){
	int i, j, k;
	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);
	for(k = 0;k < 6;k++){
		for(j = 0;j < 8 * 20;j++){
			for(i = 0;i < 20;i++){
				LCDPutData(0x0000); // 0xa800
			}
			for(i = 0;i < 20;i++){
				LCDPutData(0xa800);
			}
		}
		for(j = 0;j < 8 * 20;j++){
			for(i = 0;i < 20;i++){
				LCDPutData(0xa800);
			}
			for(i = 0;i < 20;i++){
				LCDPutData(0x0000);
			}
		}
	}
}

void LCDInit(void)
{
	LCDPeripheralInit();

	clx = cly = 0;
	LCD_CNT_PORT->ODR |=  _BV(LCD_RESET);
	Delayms(10);
	LCD_CNT_PORT->ODR &= ~_BV(LCD_RESET);
	Delayms(10);
	LCD_CNT_PORT->ODR |=  _BV(LCD_RESET);
	Delayms(10);

	LCDPutCmd(0x0000);LCDPutData(0x0001);
    LCDPutCmd(0x0003);LCDPutData(0xA8A8);
    LCDPutCmd(0x000C);LCDPutData(0x0000);
    LCDPutCmd(0x000D);LCDPutData(0x000a);
    LCDPutCmd(0x000E);LCDPutData(0x2B00);
    LCDPutCmd(0x001E);LCDPutData(0x00B8);
    LCDPutCmd(0x0001);LCDPutData(0x2B3F);
    LCDPutCmd(0x0002);LCDPutData(0x0600);
    LCDPutCmd(0x0010);LCDPutData(0x0000);
    LCDPutCmd(0x0011);LCDPutData(0x6058); // AM:Vertical　0x6068
    LCDPutCmd(0x0005);LCDPutData(0x0000);
    LCDPutCmd(0x0006);LCDPutData(0x0000);
    LCDPutCmd(0x0016);LCDPutData(0xEF1C);
    LCDPutCmd(0x0017);LCDPutData(0x0003);
    LCDPutCmd(0x0007);LCDPutData(0x0233);
    LCDPutCmd(0x000B);LCDPutData(0x0000);
    LCDPutCmd(0x000F);LCDPutData(0x0000);
    LCDPutCmd(0x0041);LCDPutData(0x0000);
    LCDPutCmd(0x0042);LCDPutData(0x0000);
    LCDPutCmd(0x0048);LCDPutData(0x0000);
    LCDPutCmd(0x0049);LCDPutData(0x013F);
    LCDPutCmd(0x004A);LCDPutData(0x0000);
    LCDPutCmd(0x004B);LCDPutData(0x0000);
    LCDPutCmd(0x0044);LCDPutData(0xEF00);
    LCDPutCmd(0x0045);LCDPutData(0x0000);
    LCDPutCmd(0x0046);LCDPutData(0x013F);
    LCDPutCmd(0x0030);LCDPutData(0x0707);
    LCDPutCmd(0x0031);LCDPutData(0x0204);
    LCDPutCmd(0x0032);LCDPutData(0x0204);
    LCDPutCmd(0x0033);LCDPutData(0x0502);
    LCDPutCmd(0x0034);LCDPutData(0x0507);
    LCDPutCmd(0x0035);LCDPutData(0x0204);
    LCDPutCmd(0x0036);LCDPutData(0x0204);
    LCDPutCmd(0x0037);LCDPutData(0x0502);
    LCDPutCmd(0x003A);LCDPutData(0x0302);
    LCDPutCmd(0x003B);LCDPutData(0x0302);
    LCDPutCmd(0x0023);LCDPutData(0x0000);
    LCDPutCmd(0x0024);LCDPutData(0x0000);
    LCDPutCmd(0x0025);LCDPutData(0x8000);
//    LCDPutCmd(0x0025);LCDPutData(0xE000);
    LCDPutCmd(0x004e);LCDPutData(0);
    LCDPutCmd(0x004f);LCDPutData(0);
/*
    LCDPutCmd(0x0028);LCDPutData(0x0006);
    LCDPutCmd(0x002F);LCDPutData(0x12BE);
    LCDPutCmd(0x0012);LCDPutData(0x6CEB);
*/

	LCDGotoXY(0, 0);
	LCDClear(240, 320, BLACK);

	MergeCircularProgressBar(1);
	jpeg_read.buf_type = 0;

	LCDPutCmd(0x0000);
	LCD->RAM;
	debug.printf("\r\nLCD_ID:%04x", LCD->RAM);

	LCDBackLightInit();
}

void LCDSetWindowArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
	if(1){
		LCDPutCmd(0x0044);LCDPutData((height - 1 + y) << 8 | y);
		LCDPutCmd(0x0045);LCDPutData(LCD_WIDTH - width - x);
		LCDPutCmd(0x0046);LCDPutData(LCD_WIDTH - 1 - x);
	} else {
		LCDPutCmd(0x0044);LCDPutData(LCD_HEIGHT - height - 1);
		LCDPutCmd(0x0045);LCDPutData(0x0000);
		LCDPutCmd(0x0046);LCDPutData(width - 1);
	}
}

__attribute__( ( always_inline ) ) __INLINE void LCDSetGramAddr(int x, int y)
{
//	if(1){
		LCDPutCmd(0x004e);
		LCDPutData(y);
		LCDPutCmd(0x004f);
		LCDPutData(LCD_WIDTH - 1 - x);
//	} else {
//		LCDPutCmd(0x004e);
//		LCDPutData(LCD_HEIGHT - 1 - y);
//		LCDPutCmd(0x004f);
//		LCDPutData(y);
//	}
}

void LCDPutPos(uint16_t x, uint16_t y, colors color)
{
	LCDSetGramAddr(x, y);
	LCDPutCmd(0x0022);
	LCDPutData(colorc[color]);
}

inline void LCDGotoXY(int x, int y)
{
	clx = x;
	cly = y;
}

void LCDPutWideCharDefault(uint16_t code, colors color)
{
	if(code >= 0x0020 && code <= 0x00DF){
		LCDPutAscii(code, color);
	} else {
		LCDPutAscii(0x00e0, color);
	}
}

static uint16_t sjis_utf16_conv(uint16_t code)
{
	return sjis2utf16_table[code - 0x8140];
}

void LCDPutStringSJIS(uint8_t *s, uint8_t color)
{
	uint16_t tc;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			LCD_FUNC.putChar(*s++, color);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.c_loaded){
			LCD_FUNC.putWideChar(sjis_utf16_conv(tc), color);
		} else {
			LCD_FUNC.putWideChar(C_FONT_UNDEF_CODE, color);
		}
	}
}

uint16_t LCDPutStringSJISN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, uint8_t color)
{
	uint16_t tc, yPos = 0;

	while(*s != '\0'){
		/*
		if(clx >= endPosX){
			LCDPutString("..", color);
			return;
		}
		*/
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if((LCD_FUNC.putChar == PCFPutChar16px) || (LCD_FUNC.putChar == C_PCFPutChar16px)){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", color);
				return yPos;
			}
		}

		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			LCD_FUNC.putChar(*s++, color);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.c_loaded){
			LCD_FUNC.putWideChar(sjis_utf16_conv(tc), color);
		} else {
			LCD_FUNC.putWideChar(C_FONT_UNDEF_CODE, color);
		}
	}

	return yPos;
}

uint16_t LCDGetStringSJISPixelLength(uint8_t *s, uint16_t font_width)
{
	uint16_t tc, len = 0;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			len += LCD_FUNC.getCharLength(*s++, font_width);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.c_loaded){
			len += LCD_FUNC.getCharLength(sjis_utf16_conv(tc), font_width);
		} else {
			len += LCD_FUNC.getCharLength(C_FONT_UNDEF_CODE, font_width);
		}
	}

	return len;
}

uint16_t LCDPutStringLFN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, colors color)
{
	uint16_t tc, yPos = 0;

	while(1){
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if((LCD_FUNC.putChar == PCFPutChar16px) || (LCD_FUNC.putChar == C_PCFPutChar16px)){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", color);
				return yPos;
			}
		}

		tc  = *(uint8_t*)s++;
		tc |= *(uint8_t*)s++ << 8;

		if(tc == 0x0000){
			return yPos;
		}
		if(tc <= 0x007F){
			LCD_FUNC.putChar(tc, color);
			continue;
		}
		if(!pcf_font.c_loaded){
			LCD_FUNC.putWideChar(tc, color);
		} else {
			LCD_FUNC.putWideChar(C_FONT_UNDEF_CODE, color);
		}
	}

	return yPos;
}

uint16_t LCDGetStringLFNPixelLength(void *s, uint16_t font_width){
	uint16_t tc, len = 0;

	while(1){
		tc  = *(uint8_t*)s++;
		tc |= *(uint8_t*)s++ << 8;

		if(tc == 0x0000) return len;
		if(tc <= 0x007F){
			len += LCD_FUNC.getCharLength(tc, font_width);
			continue;
		}
		if(!pcf_font.c_loaded){
			len += LCD_FUNC.getCharLength(tc, font_width);
		} else {
			len += LCD_FUNC.getCharLength(C_FONT_UNDEF_CODE, font_width);
		}
	}

	return len;
}


void LCDPutAscii(uint16_t asc, colors color)
{
	int i, j, x, y = cly + 2;
	uint8_t fontbuf[12];

	*((uint32_t*)&fontbuf[0]) = *((uint32_t*)&font_ascii_table[asc * sizeof(fontbuf) + 0]);
	*((uint32_t*)&fontbuf[4]) = *((uint32_t*)&font_ascii_table[asc * sizeof(fontbuf) + 4]);
	*((uint32_t*)&fontbuf[8]) = *((uint32_t*)&font_ascii_table[asc * sizeof(fontbuf) + 8]);

	for(i = 0;i < 11;i++){
		x = clx;
		for(j = 0;j < 6;j++){
			LCDSetGramAddr(x++, y);
			LCDPutCmd(0x0022);
			if((fontbuf[i] << j) & 0x80){
				LCDPutData(colorc[color]);
			}
		}
		y++;
	}
	clx += 6;
}


void LCDPutString(const char *str, colors color)
{
	while(*str != '\0'){
		if((0x20 <= *str) && (*str <= 0xDF)){
			LCD_FUNC.putChar(*str++, color);
		} else if(*str++ == '\n') {
			clx = 0;
			cly += 15;
			continue;
		}
	}
}

void LCDPutStringN(const char *str, uint16_t endPosX, colors color)
{
	while(*str != '\0'){
		if(clx >= endPosX){
			LCDPutString("..", color);
			return;
		}
		if((0x20 <= *str) && (*str <= 0xDF)){
			LCD_FUNC.putChar(*str++, color);
		} else if(*str++ == '\n') {
			clx = 0;
			cly += 13;
			continue;
		}
	}
}

uint16_t LCDPutStringUTF8(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, colors color)
{
	uint16_t tc, yPos = 0;

	while(*s != '\0'){
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if((LCD_FUNC.putChar == PCFPutChar16px) || (LCD_FUNC.putChar == C_PCFPutChar16px)){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", color);
				return yPos;
			}
		}

		if((*s >= 0x20 && *s <= 0x7F)){
			LCD_FUNC.putChar(*s++, color);
			continue;
		}

		if(*s >= 0xE0){
			tc  = ((uint16_t)*s++ << 12) & 0xF000;
			tc |= ((uint16_t)*s++ << 6) & 0x0FC0;
			tc |=  (uint16_t)*s++ & 0x003F;

			if(!pcf_font.c_loaded){
				LCD_FUNC.putChar(tc, color);
			} else {
				LCD_FUNC.putChar(C_FONT_UNDEF_CODE, color);
			}
		} else {
			s++;
		}
	}

	return yPos;
}

uint16_t LCDGetStringUTF8PixelLength(uint8_t *s, uint16_t font_width)
{
	uint16_t tc, len = 0;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F)){
			len += LCD_FUNC.getCharLength(*s++, font_width);
			continue;
		}

		if(*s >= 0xE0){
			tc  = ((uint16_t)*s++ << 12) & 0xF000;
			tc |= ((uint16_t)*s++ << 6) & 0x0FC0;
			tc |=  (uint16_t)*s++ & 0x003F;

			if(!pcf_font.c_loaded){
				len += LCD_FUNC.getCharLength(tc, font_width);
			} else {
				len += LCD_FUNC.getCharLength(C_FONT_UNDEF_CODE, font_width);
			}
		} else {
			s++;
		}
	}

	return len;
}



void LCDClear(int x,int y, colors color)
{
	int i, j;

	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);
	for(i = 0;i < y;i++){
		for(j = 0;j < x;j++){
			LCDPutData(colorc[color]);
		}
	}
}

void LCDClearWithBgImg()
{
	int i;
	int R, G, B;
	uint16_t temp;

	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);

	for(i = 0;i < LCD_WIDTH * LCD_HEIGHT;i++){
#ifdef ENABLE_BGIMG
		temp = bgImage[i];

		R = (temp & 0xf800) >> 11;
		G = (temp & 0x07e0) >> 5;
		B = (temp & 0x001f);

		if((R -= 4) <= 0) R = 0;
		if((G -= 8) <= 0) G = 0;
		if((B -= 4) <= 0) B = 0;

		if(R > 31){R = 31;}
		if(G > 63){G = 63;}
		if(B > 31){B = 31;}

		temp = (R << 11) | (G << 5) | B;

//		LCDPutData(bgImage[i]);
		LCDPutData(temp);
#else
		LCDPutData(black);
#endif
	}
 }

void LCDPutBgImg(const uint16_t *p)
{
	int i;
	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);
	for(i = 0;i < LCD_WIDTH * LCD_HEIGHT;i++){
		LCDPutData(*p++);
	}
 }

uint16_t addValRGB(uint16_t srcColor, int16_t add)
{
	pixel_fmt_typedef pixel;

	pixel.color.d16 = srcColor;
	pixel.color.R = __USAT(pixel.color.R + add, 5);
	pixel.color.G = __USAT(pixel.color.G + add * 2, 6);
	pixel.color.B = __USAT(pixel.color.B + add, 5);

	return (pixel.color.d16);
}

void LCDPutBgImgFiler(){
	uint32_t i;
	uint16_t *p;

	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);

	p = (uint16_t*)(FLASH_BASE + 16 * 1024); // Sector #1
	for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){ // 1st 6KB
		LCD->RAM = addValRGB(*p++, -4);
	}
	p = (uint16_t*)(FLASH_BASE + (16 + 16) * 1024); // Sector #2
	for(i = 0;i < (16 * 1024 / sizeof(uint16_t));i++){ // 2nd 16KB
		LCD->RAM = addValRGB(*p++, -4);
	}
	p = (uint16_t*)(0x080C0000); // Sector #10
	for(i = 0;i < (128 * 1024 / sizeof(uint16_t));i++){ // 3rd 128KB
		LCD->RAM = addValRGB(*p++, -4);
	}
}

void LCDPutBgImgMusic(){
	uint32_t  i;
	uint16_t *p;

	LCDSetGramAddr(0, 0);
	LCDPutCmd(0x0022);

	p = (uint16_t*)(FLASH_BASE + (16 + 6) * 1024); // Sector #1 + 6KB offset
	for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){ // 1st 6KB
		LCD->RAM = *p++;
	}
	p = (uint16_t*)(FLASH_BASE + (16 + 16 + 16) * 1024); // Sector #3
	for(i = 0;i < (16 * 1024 / sizeof(uint16_t));i++){ // 2nd 16KB
		LCD->RAM = *p++;
	}
	p = (uint16_t*)(0x080E0000); // Sector #11
	for(i = 0;i < (128 * 1024 / sizeof(uint16_t));i++){ // 3rd 128KB
		LCD->RAM = *p++;
	}
}

void DrawLine(int x1, int y1, int x2, int y2, colors color)
{
    int W = x2 - x1;
    int H = y2 - y1;
    int dx = 0;
    int dy = 0;
    int Wy = 0;
    int Hx = 0;
    if (W >= H)
    {
        for (dx = 0; dx <= W; dx ++)
        {
        	LCDSetGramAddr(x1+dx, y1+dy);
			LCDPutCmd(0x0022);
			LCDPutData(colorc[color]);
            Hx += H;
            if (Wy < Hx)
            {
                Wy += W;
                dy += 1;
            }
        }
    }
    else
    {
        for (dy = 0; dy <= H; dy ++)
        {
        	LCDSetGramAddr(x1+dx, y1+dy);
			LCDPutCmd(0x0022);
			LCDPutData(colorc[color]);

            Wy += W;
            if (Hx < Wy)
            {
                Hx += H;
                dx += 1;
            }
        }
    }
}

__attribute__( ( always_inline ) ) __INLINE void LCDPset(uint16_t x, uint16_t y, colors color){
	LCDSetGramAddr(x, y);
	LCDPutCmd(0x0022);
	LCDPutData(colorc[color]);
}

__attribute__( ( always_inline ) ) __INLINE void LCDPset2(uint16_t x, uint16_t y, uint16_t data){
	LCDSetGramAddr(x, y);
	LCDPutCmd(0x0022);
	LCDPutData(data);
}


__attribute__( ( always_inline ) ) __INLINE uint16_t LCDGetPos(uint16_t x, uint16_t y){
	LCDSetGramAddr(x, y);
	LCDPutCmd(0x0022);
	LCD->RAM;
	return LCD->RAM;
}

void LCDDrawLine2(int16_t sPosX, int16_t sPosY, int16_t ePosX, int16_t ePosY, colors color){
	int dw, dh, e, x, y;

	dw = ePosX - sPosX;
	dh = ePosY - sPosY;

	e = 0;

	if(dw >= dh){
		y = sPosY;
		for(x = sPosX;x <= ePosX;x++){
			LCDPset(x, y, color);

			e += 2 * dh;
			if(e >= (2 * dw)){
				y++;
				e -= 2 * dw;
			}
		}
	} else{

	}
}

void LCDDrawLine(uint16_t sPosX, uint16_t sPosY, uint16_t ePosX, uint16_t ePosY, colors color){
	int x, y, w, h, offsetY;
	float delta, alpha;
	pixel_fmt_typedef pixel_fg, pixel_bg, pixel;

	w = ePosX - sPosX;
	h = ePosY - sPosY;

//	debug.printf("\r\n\nPlot");
//	debug.printf("\r\ndelta:%f", delta);
//	debug.printf("\r\noffsetY:%d", offsetY);

	if(abs(w) >= abs(h)){
		if(ePosX == sPosX){
			offsetY = sPosY + 0.5f;
			for(x = sPosX;x <= ePosX;x++){
				y = offsetY;
				LCDPset(x, y, color);
			}
			return;
		}

		delta = (float)h / (float)w;
		offsetY = -delta * sPosX + sPosY + 0.5f;

		if(ePosX >= sPosX){
			for(x = sPosX;x <= ePosX;x++){
				y = delta * x + offsetY;
				alpha = delta * x + offsetY;
				alpha = 1.0f - (alpha - y);
				pixel_bg.color.d16 = LCDGetPos(x, y);
				pixel_fg.color.d16 = colorc[color];
				pixel.color.R = pixel_fg.color.R * alpha + pixel_bg.color.R * (1.0f - alpha);
				pixel.color.G = pixel_fg.color.G * alpha + pixel_bg.color.G * (1.0f - alpha);
				pixel.color.B = pixel_fg.color.B * alpha + pixel_bg.color.B * (1.0f - alpha);
				LCDPset2(x, y, pixel.color.d16);
				debug.printf("\r\nA x:%d y:%d delta:%f", x, y, delta);
//				LCDPset(x, y, color);
			}
		} else {
			for(x = sPosX;x >= ePosX;x--){
				y = delta * x + offsetY;

				alpha = delta * x + offsetY;
				alpha = 1.0f - (alpha - y);
				pixel_bg.color.d16 = LCDGetPos(x, y);
				pixel_fg.color.d16 = colorc[color];
				pixel.color.R = pixel_fg.color.R * alpha + pixel_bg.color.R * (1.0f - alpha);
				pixel.color.G = pixel_fg.color.G * alpha + pixel_bg.color.G * (1.0f - alpha);
				pixel.color.B = pixel_fg.color.B * alpha + pixel_bg.color.B * (1.0f - alpha);
				LCDPset2(x, y, pixel.color.d16);

				debug.printf("\r\nB x:%d y:%d delta:%f", x, y, delta);
//				LCDPset(x, y, color);
			}
		}
	} else {
		if(ePosY >= sPosY){
			if(sPosX == ePosX){
				for(y = sPosY;y <= ePosY;y++){
					x = sPosX;
//					debug.printf("\r\nC0 x:%d y:%d delta:%f", x, y, delta);
					LCDPset(x, y, color);
				}
			} else {
				delta = (float)h / (float)w;
				offsetY = -delta * sPosX + sPosY + 0.5f;
				delta = 1.0f / delta;

				for(y = sPosY;y <= ePosY;y++){
					x = (y - offsetY) * delta;
					debug.printf("\r\nC x:%d y:%d delta:%f", x, y, delta);
					LCDPset(x, y, color);

					alpha = (y - offsetY) * delta;
					alpha = 1.0f - (alpha - x);
					pixel_bg.color.d16 = LCDGetPos(x - 1, y);
					pixel_fg.color.d16 = colorc[color];
					pixel.color.R = pixel_fg.color.R * alpha + pixel_bg.color.R * (1.0f - alpha);
					pixel.color.G = pixel_fg.color.G * alpha + pixel_bg.color.G * (1.0f - alpha);
					pixel.color.B = pixel_fg.color.B * alpha + pixel_bg.color.B * (1.0f - alpha);
					LCDPset2(x - 1, y, pixel.color.d16);

					alpha = 1.0f - alpha;
					pixel_bg.color.d16 = LCDGetPos(x + 1, y);
					pixel_fg.color.d16 = colorc[color];
					pixel.color.R = pixel_fg.color.R * alpha + pixel_bg.color.R * (1.0f - alpha);
					pixel.color.G = pixel_fg.color.G * alpha + pixel_bg.color.G * (1.0f - alpha);
					pixel.color.B = pixel_fg.color.B * alpha + pixel_bg.color.B * (1.0f - alpha);
					LCDPset2(x + 1, y, pixel.color.d16);
				}
			}
		} else {
			if(sPosX == ePosX){
				for(y = sPosY;y >= ePosY;y--){
					x = sPosX;
//					debug.printf("\r\nD0 x:%d y:%d delta:%f", x, y, delta);
					LCDPset(x, y, color);
				}
			} else {
				delta = (float)h / (float)w;
				offsetY = -delta * sPosX + sPosY + 0.5f;
				delta = 1.0f / delta;

				for(y = sPosY;y >= ePosY;y--){
					x = (y - offsetY) * delta;
					alpha = (y - offsetY) * delta;
					alpha = 1.0f - (alpha - x);
					pixel_bg.color.d16 = LCDGetPos(x, y);
					pixel_fg.color.d16 = colorc[color];
					pixel.color.R = pixel_fg.color.R * alpha + pixel_bg.color.R * (1.0f - alpha);
					pixel.color.G = pixel_fg.color.G * alpha + pixel_bg.color.G * (1.0f - alpha);
					pixel.color.B = pixel_fg.color.B * alpha + pixel_bg.color.B * (1.0f - alpha);
					LCDPset2(x, y, pixel.color.d16);

					debug.printf("\r\nD x:%d y:%d delta:%f", x, y, delta);
//					LCDPset(x, y, color);
				}
			}
		}
	}

}

void LCDDrawSquare(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colors color)
{
	uint32_t ix, iy;

	for(iy = 0;iy < height;iy++){
		ix = 0;
		LCDSetGramAddr(x + ix, y + iy);
		for(ix = 0;ix < width;ix++){
			LCDPutCmd(0x0022);
			LCDPutData(colorc[color]);
		}
	}
}

void LCDPrintFileList()
{
//	TOUCH_PINIRQ_DISABLE;
	touch.func = LCDTouchPoint;
	USART_IRQ_DISABLE;

	int i;
	uint32_t var32;
	volatile uint16_t idEntry = cursor.pageIdx * PAGE_NUM_ITEMS;
	uint16_t entryPointOffset, color, step;

	uint8_t pLFNname[80];
	char fileNameStr[13], fileSizeStr[13], fileTypeStr[4];

	TIM_Cmd(TIM1, DISABLE); // Stop displaying progress bar
	DMA_Cmd(DMA2_Stream6, DISABLE);

	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);

//	LCDClearWithBgImg();
	LCDPutBgImgFiler();

	LCDPutIcon(0, 0, 320, 22, menubar_320x22, menubar_320x22_alpha);

	LCDGotoXY(5, 2);
	LCDPutStringLFN(5, 185, 1, (uint8_t*)fat.currentDirName, HEADER_COLOR);


	step = fat.fileCnt - idEntry;
	if(step >= PAGE_NUM_ITEMS){
		step = PAGE_NUM_ITEMS;
	}

	cly = HEIGHT_ITEM;

	for(i = idEntry;i < (idEntry + step);i++){
		if((i == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings item
			LCDPutIcon(2, cly - 3, 22, 22, settings_22x22, settings_22x22_alpha);
			clx = 28;
			LCDPutString("Settings", ARCHIVE_COLOR);
			cly += HEIGHT_ITEM;
			continue;
		}

		entryPointOffset = getListEntryPoint(i); // リスト上にあるIDファイルエントリの先頭位置をセット

		memset(fileNameStr, '\0', sizeof(fileNameStr));
		memset(fileSizeStr, '\0', sizeof(fileSizeStr));
		memset(fileTypeStr, '\0', sizeof(fileTypeStr));

		strncpy(fileNameStr, (char*)&fbuf[entryPointOffset], 8); // 8文字ファイル名をコピー
		strtok(fileNameStr, " "); // スペースがあればNULLに置き換える

		if(fbuf[entryPointOffset + NT_Reserved] & NT_U2L_NAME){ // Name Upper to Lower
			fileNameStr[0] = tolower(fileNameStr[0]);
			fileNameStr[1] = tolower(fileNameStr[1]);
			fileNameStr[2] = tolower(fileNameStr[2]);
			fileNameStr[3] = tolower(fileNameStr[3]);
			fileNameStr[4] = tolower(fileNameStr[4]);
			fileNameStr[5] = tolower(fileNameStr[5]);
			fileNameStr[6] = tolower(fileNameStr[6]);
			fileNameStr[7] = tolower(fileNameStr[7]);
		}

		var32 = *((uint32_t*)&fbuf[entryPointOffset + FILESIZE]); // ファイルサイズ取得

		if(var32 >= 1000000){
			var32 /= 1000000;
			SPRINTF(fileSizeStr, "%dMB", var32);
		} else if(var32 >= 1000){
			var32 /= 1000;
			SPRINTF(fileSizeStr, "%dKB", var32);
		} else {
			SPRINTF(fileSizeStr, "%dB", var32);
		}

		if(!(fbuf[entryPointOffset + ATTRIBUTES] & ATTR_DIRECTORY)){
			if(fbuf[entryPointOffset + 8] != 0x20){
				strncpy(fileTypeStr, (char*)&fbuf[entryPointOffset + 8], 3);
				//strcat(fileNameStr, ".");
				//strcat(fileNameStr, fileTypeStr);
			} else {
				strcpy(fileTypeStr, "---");
			}

			color = ARCHIVE_COLOR; // archivecolor

			if(strcmp(fileTypeStr, "MOV") == 0){
				LCDPutIcon(2, cly - 5, 22, 22, movie_22x22, movie_22x22_alpha);
			} else if(strcmp(fileTypeStr, "MP3") == 0 || \
					  strcmp(fileTypeStr, "AAC") == 0 || \
					  strcmp(fileTypeStr, "MP4") == 0 || \
					  strcmp(fileTypeStr, "M4A") == 0 || \
					  strcmp(fileTypeStr, "M4P") == 0 || \
					  strcmp(fileTypeStr, "WAV") == 0
					) {
				LCDPutIcon(2, cly - 5, 22, 22, onpu_22x22, onpu_22x22_alpha);
			} else if(strcmp(fileTypeStr, "PCF") == 0) {
				LCDPutIcon(2, cly - 5, 22, 22, font_22x22, font_22x22_alpha);
			} else if(strcmp(fileTypeStr, "JPG") == 0 || strcmp(fileTypeStr, "JPE") == 0) {
				LCDPutIcon(2, cly - 5, 22, 22, jpeg_22x22, jpeg_22x22_alpha);
			} else {
				LCDPutIcon(2, cly - 5, 22, 22, archive_22x22, archive_22x22_alpha);
			}
		} else {
			strcat(fileTypeStr, "DIR");
			color = DIR_COLOR; // dircolor
			fileSizeStr[0] = '\0';
			if(strncmp(fileNameStr, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				LCDPutIcon(2, cly - 5, 22, 22, folder_22x22, folder_22x22_alpha);
			}
		}

		clx = 28;
		if(!setLFNname(pLFNname, i, LFN_WITHOUT_EXTENSION, sizeof(pLFNname))){ // エントリがSFNの場合
			LCDPutString(fileNameStr, color);
		} else { // LFNの場合
			LCDPutStringLFN(clx, 180, 1, pLFNname, color);
		}

		clx = 210;
		LCDPutString(fileTypeStr, color);

		clx = 250;
		LCDPutString(fileSizeStr, color);

		cly += HEIGHT_ITEM;
	}

	if(cursor.pages > 0){
		// Scroll Bar
		int scrollHeight = 204 / (cursor.pages + 1), \
		    scrollStartPos = scrollHeight * cursor.pageIdx + 25 + 2, \
		    height = scrollHeight - 14 - 4;

		LCDPutIcon(309, 25, 6, 204, scrollbar_6x204, scrollbar_6x204_alpha); // scroll background
		LCDPutIcon(309, scrollStartPos, 6, 7, scrollbar_top_6x7, scrollbar_top_6x7_alpha);
		for(i = scrollStartPos + 7;i < ((scrollStartPos + 7) + height);i++){
			LCDPutIcon(309, i, 6, 1, scrollbar_hline_6x1, scrollbar_hline_6x1_alpha);
		}
		LCDPutIcon(309, i, 6, 7, scrollbar_bottom_6x7, scrollbar_bottom_6x7_alpha);
	}

	LCDStoreCursorBar(cursor.pos);
	LCDSelectCursorBar(cursor.pos);
	LCDBackLightTimerInit();
//	TOUCH_PINIRQ_ENABLE;
//	TouchPenIRQ_Enable();
	USART_IRQ_ENABLE;
	touch.repeat = 1;
}

void LCDPrintSettingsList(char type, int select_id, settings_item_typedef *item)
{
//	TOUCH_PINIRQ_DISABLE;
//	USART_IRQ_DISABLE;
	touch.func = LCDTouchPoint;

	int i, selected_entry;
	volatile uint16_t idEntry = cursor.pageIdx * PAGE_NUM_ITEMS;
	uint16_t step;

	TIM_Cmd(TIM1, DISABLE);
	DMA_Cmd(DMA2_Stream6, DISABLE);

	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);

//	LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);
	LCDCheckPattern();
//	LCDPutBgImgFiler();

	LCDPutIcon(0, 0, 320, 22, menubar_320x22, menubar_320x22_alpha);

	LCDGotoXY(5, 2);
	LCDPutString((char*)fat.currentDirName, HEADER_COLOR);

	step = fat.fileCnt - idEntry;
	if(step >= PAGE_NUM_ITEMS){
		step = PAGE_NUM_ITEMS;
	}

	cly = HEIGHT_ITEM;

	if(type == SETTING_TYPE_ITEM){
		if(select_id == 0){
			selected_entry = item->selected_id + 1;
		} else {
			selected_entry = select_id;
		}

		for(i = idEntry;i < (idEntry + step);i++){
			if(strncmp(settings_p[i].name, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				if(i == selected_entry){
					LCDPutIcon(2, cly - 3, 22, 22, radiobutton_checked_22x22, radiobutton_22x22_alpha);
				} else {
					LCDPutIcon(2, cly - 3, 22, 22, radiobutton_unchecked_22x22, radiobutton_22x22_alpha);
				}
			}
			clx = 28;
			LCDPutString(settings_p[i].name, WHITE);

			cly += HEIGHT_ITEM;
		}
		item->selected_id = selected_entry - 1;

		if(item->func != NULL && select_id != 0){
			item->func((void*)item);
		}
	} else {
		for(i = idEntry;i < (idEntry + step);i++){
			if(strncmp(settings_p[i].name, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				if(settings_p[i].icon != NULL){
					LCDPutIcon(2, cly - 5, 22, 22, settings_p[i].icon->data, settings_p[i].icon->alpha);
				} else {
					LCDPutIcon(2, cly - 5, 22, 22, select_22x22, select_22x22_alpha);
				}
			}
			clx = 28;
			LCDPutString(settings_p[i].name, WHITE);

			cly += HEIGHT_ITEM;
		}
	}
/*
	if(cursor.pages > 0){
		// Scroll Bar
		int scrollHeight = 204 / (cursor.pages + 1), \
		    scrollStartPos = scrollHeight * cursor.pageIdx + 25 + 2, \
		    height = scrollHeight - 14 - 4;

		LCDPutIcon(309, 25, 6, 204, scrollbar_6x204, scrollbar_6x204_alpha); // scroll background
		LCDPutIcon(309, scrollStartPos, 6, 7, scrollbar_top_6x7, scrollbar_top_6x7_alpha);
		for(i = scrollStartPos + 7;i < ((scrollStartPos + 7) + height);i++){
			LCDPutIcon(309, i, 6, 1, scrollbar_hline_6x1, scrollbar_hline_6x1_alpha);
		}
		LCDPutIcon(309, i, 6, 7, scrollbar_bottom_6x7, scrollbar_bottom_6x7_alpha);
	}
*/
	LCDStoreCursorBar(cursor.pos);
	LCDSelectCursorBar(cursor.pos);
//	USART_IRQ_ENABLE;
	LCDBackLightTimerInit();
//	TOUCH_PINIRQ_ENABLE;
	touch.repeat = 1;
}

void LCDPutCursorBar(int curPos)
{
	int i;

	LCDSetGramAddr(0, (curPos + 1) * HEIGHT_ITEM);
	LCDPutCmd(0x0022);
	for(i = 0;i < LCD_WIDTH * 13;i++){
		LCDPutData(cursorRAM[i]);
	}
}

void LCDSelectCursorBar(int curPos)
{
	int  i, j;
	uint16_t startPosX = 26, width = 280;
	pixel_fmt_typedef pixel;

	for(j = 0;j < 13;j++){
		LCDSetGramAddr(startPosX, (curPos + 1) * HEIGHT_ITEM + j);
		LCDPutCmd(0x0022);
		for(i = startPosX;i < (startPosX + width);i++){

			pixel.color.d16 = cursorRAM[i + j * LCD_WIDTH];

			pixel.color.R = __USAT(pixel.color.R + 6, 5);
			pixel.color.G = __USAT(pixel.color.G + 12, 6);
			pixel.color.B = __USAT(pixel.color.B + 6, 5);

			LCDPutData(pixel.color.d16);
		}
	}
}

/*
void LCDSelectCursorBar(int curPos)
{
	int  i, j;
	int R, G, B;
	uint16_t startPosX = 26, width = 280, pixel;

	for(j = 0;j < 13;j++){
		LCDSetGramAddr(startPosX, (curPos + 1) * HEIGHT_ITEM + j);
		LCDPutCmd(0x0022);
		for(i = startPosX;i < (startPosX + width);i++){

			pixel = cursorRAM[i + j * LCD_WIDTH];

			R = (pixel & 0xf800) >> 11;
			G = (pixel & 0x07e0) >> 5;
			B = (pixel & 0x001f);

			R = __USAT(R + 6, 5);     // 0 <= R < 32
			G = __USAT(G + 6 * 2, 6); // 0 <= G < 64
			B = __USAT(B + 6, 5);     // 0 <= B < 32

			LCDPutData(R << 11 | G << 5 | B);
		}
	}
}
*/

void LCDStoreCursorBar(int curPos)
{
	int i;

	LCDSetGramAddr(0, (curPos + 1) * HEIGHT_ITEM);
	LCDPutCmd(0x0022);
	LCD->RAM;

	for(i = 0;i < LCD_WIDTH * 13;i++){
		cursorRAM[i] = LCD->RAM;
	}
}


void LCDCursorUp()
{
	if(cursor.pos-- <= 0) {
		if(cursor.pageIdx-- <= 0){
			cursor.pageIdx = cursor.pos = 0;
			return;
		} else {
			cursor.pos = PAGE_NUM_ITEMS - 1;
			if(!settings_mode){
				LCDPrintFileList();
			} else {
				LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
			}
			return;
		}
	}

	LCDPutCursorBar(cursor.pos + 1); // 前項消去
	LCDStoreCursorBar(cursor.pos); // 現在の項目を保存
	LCDSelectCursorBar(cursor.pos);  // 現在の項目を選択
}


void LCDCursorDown()
{
	if( (cursor.pos + cursor.pageIdx * PAGE_NUM_ITEMS + 1) >= fat.fileCnt ){
		return;
	}

	if(cursor.pos++ >= (PAGE_NUM_ITEMS - 1)) {
		cursor.pos = 0;
		if(++cursor.pageIdx > cursor.pages) cursor.pageIdx = cursor.pages - 1;
		if(!settings_mode){
			LCDPrintFileList();
		} else {
			LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
		}
		return;
	}

	LCDPutCursorBar(cursor.pos - 1); // 前項消去
	LCDStoreCursorBar(cursor.pos); // 現在の項目を保存
	LCDSelectCursorBar(cursor.pos);  // 現在の項目を選択表示
}

void LCDCursorEnter()
{
	uint16_t idEntry = cursor.pos + cursor.pageIdx * PAGE_NUM_ITEMS, entryPointOffset;
	char fileType[4];

	memset(fileType, '\0', sizeof(fileType));

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_TimeBaseInitStructure.TIM_Period = 999;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	TIM_SetCounter(TIM1, 0);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	while(!TIM_GetITStatus(TIM1, TIM_IT_Update));
	TIM_SetCounter(TIM1, 0);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

	LCDPutCursorBar(cursor.pos);

	while(!TIM_GetITStatus(TIM1, TIM_IT_Update));
	LCDSelectCursorBar(cursor.pos);

	if(settings_mode){ // settings mode
		if(strncmp(settings_p[idEntry].name, "..", 2) == 0){ // selected item is parent directry
			settings_p = (settings_list_typedef*)settings_p[idEntry].next;
			if(settings_p == NULL){ // back to root
				settings_mode = 0;
				fat.currentDirEntry = fat.rootDirEntry;
				memcpy((char*)fat.currentDirName, (char*)root_str, sizeof(root_str));
				free(fat.pfileList);
				makeFileList();
				LCDPrintFileList();
				return;
			}
			strcpy((char*)fat.currentDirName, (char*)&settings_stack.name[--settings_stack.idx][0]);
			fat.fileCnt = settings_stack.items[settings_stack.idx];
			cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
			cursor.pageIdx = settings_stack.pos[settings_stack.idx] / PAGE_NUM_ITEMS;
			cursor.pos = settings_stack.pos[settings_stack.idx] % PAGE_NUM_ITEMS;
			LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
			return;
		} else {
			settings_list_typedef *settings_cur = &settings_p[idEntry];
			if(settings_cur->next == NULL){
				LCDPrintSettingsList(SETTING_TYPE_ITEM, idEntry, settings_cur->item);
				return;
			}
			settings_stack.items[settings_stack.idx] = fat.fileCnt;
			strcpy((char*)&settings_stack.name[settings_stack.idx][0], (char*)fat.currentDirName);
			strcpy((char*)fat.currentDirName, settings_p[idEntry].name);
			fat.fileCnt = settings_p[idEntry].itemCnt;
			settings_p = settings_p[idEntry].next;
			settings_stack.pos[settings_stack.idx] = idEntry;
			cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
			cursor.pos = cursor.pageIdx = 0;
			settings_stack.idx++;
			LCDPrintSettingsList(settings_cur->type, 0, settings_cur->item);
			if(settings_cur->func){
				settings_cur->func(NULL);
			}
			return;
		}
	}

	if((idEntry == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings item
		settings_mode = 1;
		settings_stack.idx = 1;
		strcpy((char*)fat.currentDirName, "Settings");
//		fat.fileCnt = sizeof(settings_root_list) / sizeof(settings_root_list[0]);
		fat.fileCnt = settings_root_list_fileCnt;
		cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
		cursor.pos = cursor.pageIdx = 0;
		settings_p = (settings_list_typedef*)settings_root_list;
		LCDPrintSettingsList(0, 0, NULL);
		return;
	}

	entryPointOffset = getListEntryPoint(idEntry); // リスト上にあるIDファイルエントリの先頭位置をセット

	if(!(fbuf[entryPointOffset + ATTRIBUTES] & ATTR_DIRECTORY)){ // ファイルの属性をチェック
		strncpy(fileType, (char*)&fbuf[entryPointOffset + 8], 3);
		if(strcmp(fileType, "WAV") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_WAV;
			return;
		}
		if(strcmp(fileType, "MP3") == 0){
			LCDSetGramAddr(0, 0);
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_MP3;
			return;
		}
		if(strcmp(fileType, "MP4") == 0 || \
		   strcmp(fileType, "M4A") == 0 || \
		   strcmp(fileType, "M4P") == 0 || \
		   strcmp(fileType, "AAC") == 0){
			LCDSetGramAddr(0, 0);
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_AAC;
			return;
		}
		if(strcmp(fileType, "JPG") == 0 || strcmp(fileType, "JPE") == 0){
			LCDSetGramAddr(0, 0);
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_JPG;
			return;
		}
		if(strcmp(fileType, "MOV") == 0){
			LCDSetGramAddr(0, 0);
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_MOV;
			return;
		}
		if(strcmp(fileType, "PCF") == 0){
			LCDSetGramAddr(0, 0);
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_PCF;
			return;
		}
		touch.repeat = 1;
	} else { // ファイルの属性がディレクトリ
		// show circular progress bar animation until file list complete
		LCDSetWindowArea(303, 1, 16, 16);
		LCDSetGramAddr(303, 1);
		LCDPutCmd(0x0022);
		DMA_ProgressBar_Conf();

		shuffle_play.flag_make_rand = 0; // clear shuffle flag

		changeDir(idEntry); // idEntryのディレクトリに移動する
		LCDPrintFileList(); // ファイルリスト表示
	}
}

void LCDStoreBgImgToBuff(int startPosX, int startPosY, int width, int height, uint16_t *p)
{
	uint32_t x, y;
	for(y = 0;y < height;y++){
		LCDSetGramAddr(startPosX, startPosY + y);
		LCDPutCmd(0x0022);
		LCD->RAM;
		for(x = 0;x < width;x++){
			*p++ = LCD->RAM;
		}
	}
}

void LCDPutBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p)
{
	uint32_t x, y;
	for(y = 0;y < height;y++){
		LCDSetGramAddr(startPosX, startPosY + y);
		LCDPutCmd(0x0022);
		for(x = 0;x < width;x++){
			LCD->RAM = *p++;
		}
	}
}

/*
void LCDPutIcon(int startPosX, int startPosY, int width, int height, const uint16_t *d, const uint16_t *a)
{
	int i, j, x, y = startPosY;
	float alpha_ratio;
	pixel_fmt_typedef pixel_alpha, pixel_fg, pixel_bg;

	if(a == '\0'){
		for(y = startPosY;y < startPosY + height;y++){
			LCDSetGramAddr(startPosX, y);
			LCDPutCmd(0x0022);
			for(x = 0;x < width;x++){
				LCDPutData(*d++);
			}
		}
		return;
	}

	for(i = 0;i < height;i++){
		x = startPosX;
		for(j = 0;j < width;j++){
			LCDSetGramAddr(x, y);
			LCDPutCmd(0x0022);
			LCD->RAM;
			pixel_bg.color.d16 = LCD->RAM;
			pixel_alpha.color.d16 = *a++;
			pixel_fg.color.d16 = *d++;
			alpha_ratio = (float)pixel_alpha.color.G / 63.0f;

			// Foreground Image
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Image
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDSetGramAddr(x, y);
			LCDPutCmd(0x0022);
			LCDPutData(pixel_fg.color.d16);
			x++;
		}
		y++;
	}
}
*/

void LCDPutIcon(int startPosX, int startPosY, int width, int height, const uint16_t *d, const uint8_t *a)
{
	int i, j, x, y = startPosY;
	float alpha_ratio;
	pixel_fmt_typedef pixel_fg, pixel_bg;
#define F_INV_255 (1.0f / 255.0f)

	if(a == '\0'){
		for(y = startPosY;y < startPosY + height;y++){
			LCDSetGramAddr(startPosX, y);
			LCDPutCmd(0x0022);
			for(x = 0;x < width;x++){
				LCDPutData(*d++);
			}
		}
		return;
	}

	for(i = 0;i < height;i++){
		x = startPosX;
		for(j = 0;j < width;j++){
			LCDSetGramAddr(x, y);
			LCDPutCmd(0x0022);
			LCD->RAM;
			pixel_bg.color.d16 = LCD->RAM;
			pixel_fg.color.d16 = *d++;
			alpha_ratio = *a++ * F_INV_255; // Gray Scale

			// Foreground Color
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Color
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDSetGramAddr(x, y);
			LCDPutCmd(0x0022);
			LCDPutData(pixel_fg.color.d16);
			x++;
		}
		y++;
	}
}



void LCDTouchPos()
{
	LCDDrawSquare(touch.posX - 1, touch.posY - 1, 3, 3, WHITE);
}

int firstTouchedItem;
int currentTouchItem;

void LCDTouchReleased(void){
	if(currentTouchItem == -1){
		return;
//		cursor.pageIdx = 0, cursor.pos = 0;
//		LCDPrintFileList();
	}else if(currentTouchItem == firstTouchedItem){
		touch.repeat = 0;
		LCDCursorEnter();
		currentTouchItem = 0, firstTouchedItem = -999;
	}
}

void LCDTouchPoint(){
	if(!touch.repeat){
		return;
	}

	currentTouchItem = (touch.posY / HEIGHT_ITEM) - 1; // 選択項目のカーソル位置取得
	if(cursor.pageIdx == 0 && currentTouchItem < 0){
		currentTouchItem = 0;
	}

	if(touch.posY > (LCD_HEIGHT - 5)){
		currentTouchItem++;
	}

	if(touch.update == 0){
		firstTouchedItem = currentTouchItem;
	} else if(firstTouchedItem != currentTouchItem){
		firstTouchedItem = -999;
	}

	if(currentTouchItem == cursor.pos) return; // 前項と同じ項目ならリターン
	if( (currentTouchItem + cursor.pageIdx * PAGE_NUM_ITEMS) >= fat.fileCnt ) return; // ファイル数以上ならリターン

	if(touch.posY < HEIGHT_ITEM){ // 上端まできたらスクロールアップ
		LCDCursorUp();
		return;
	}
	if(currentTouchItem >= PAGE_NUM_ITEMS){ // 下端まできたらスクロールダウン
		LCDCursorDown();
		return;
	}

	LCDPutCursorBar(cursor.pos); // 前項消去
	LCDStoreCursorBar(currentTouchItem); // 現在の項目を保存
	LCDSelectCursorBar(currentTouchItem);  // 現在の項目を選択表示
	cursor.pos = currentTouchItem;

	return;
}

typedef struct{
	uint16_t p[100][100];
}artRAM_typedef;

void dispArtWork(MY_FILE *input_file)
{
	int i, j, t, image_type = 0;
	int x, y, width, height;

	float alpha, alpha_ratio;
	pixel_fmt_typedef pixel_fg, pixel_bg;
	uint8_t checkHeader[512];

	struct jpeg_decompress_struct jdinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;

	if(input_file->clusterOrg == 0){
		image_type = 1;
		goto DISPLAY_ARTWORK;
	}

	my_fread(checkHeader, 1, 512, input_file);

	for(i = 0;i < 511;i++){
		if((checkHeader[i] == 0xff) && (checkHeader[i + 1] == 0xd8)){
			break;
		}
	}

	if(i >= 511){
		debug.printf("\r\nArtWork: doesn't start with 0xffd8");
		image_type = 1;
		goto DISPLAY_ARTWORK;
	}

	my_fseek(input_file, -512, SEEK_CUR);

	debug.printf("\r\n");

	create_mpool();

	/* Initialize the JPEG decompression object with default error handling. */
	jdinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jdinfo);

	jdinfo.useMergedUpsampling = FALSE;
	jdinfo.dct_method = JDCT_ISLOW;
	jdinfo.dither_mode = JDITHER_FS;
	jdinfo.two_pass_quantize = TRUE;

	jdinfo.mem->max_memory_to_use = 30000UL;

	/* Specify data source for decompression */
	jpeg_stdio_src(&jdinfo, input_file);

	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&jdinfo, TRUE);

	if(jdinfo.err->msg_code == JERR_NO_IMAGE){
		debug.printf("\r\nnArtWork: JPEG datastream contains no image");
		image_type = 1;
		goto DISPLAY_ARTWORK;
	}

	if(jdinfo.progressive_mode){ // progressivee jpeg not supported
		debug.printf("\r\nnArtWork: progressive jpeg not supported");
		image_type = 1;
		goto DISPLAY_ARTWORK;
	}

	if(jdinfo.image_width > 640 || jdinfo.image_height > 640){
		debug.printf("\r\nnArtWork: too large to display");
		image_type = 1;
		goto DISPLAY_ARTWORK;
	}

	for(i = 8;i >= 1; i--){
		if((jdinfo.image_width * (float)i / 8.0f) <= 80 && \
				(jdinfo.image_height * (float)i / 8.0f) <= 80){
			break;
		}
	}

	jdinfo.scale_denom = 8;
	jdinfo.scale_num = i > 1 ? i : 1;

	dest_mgr = jinit_write_ppm(&jdinfo);

	/* Start decompressor */
	(void) jpeg_start_decompress(&jdinfo);

	typedef struct {
		struct djpeg_dest_struct pub;	/* public fields */

		/* Usually these two pointers point to the same place: */
		char *iobuffer;		/* fwrite's I/O buffer */
		JSAMPROW pixrow;		/* decompressor output buffer */
		size_t buffer_width;		/* width of I/O buffer */
		JDIMENSION samples_per_row;	/* JSAMPLEs per output row */
	} ppm_dest_struct;

	typedef ppm_dest_struct * ppm_dest_ptr;
	ppm_dest_ptr dest = (ppm_dest_ptr) dest_mgr;

#ifdef MY_DEBUG
	debug.printf("\r\nx:%d y:%d width:%d height:%d", x, y, width, height);
#endif

	DISPLAY_ARTWORK:
	if(image_type == 1){
		x = 22, y = 57, width = 74, height = 74;
	} else {
		debug.printf("\r\nsrc_size:%dx%d", jdinfo.image_width, jdinfo.image_height);
		debug.printf("\r\ndst_size:%dx%d", jdinfo.output_width, jdinfo.output_height);
		debug.printf("\r\nscale_num:%d scale_denom:%d", jdinfo.scale_num, jdinfo.scale_denom);
		debug.printf("\r\nscale:%d/%d", jdinfo.scale_num,  jdinfo.scale_denom);

		width = jdinfo.output_width;
		height = jdinfo.output_height;

		x = (80 - width) / 2 - 1;
		y =  (80 - height) / 2 - 1;
		x = (x > 0 ? x : 0) + 20;
		y = (y > 0 ? y : 0) + 55;
	}

	float a, d, g;
	int heightTop, heightBottom;
	heightTop = height * 0.062f;
	heightBottom = height - heightTop;

	g = (float)(heightBottom - heightTop - height) / (float)(width * (heightTop - heightBottom));
	a = 1.0f + (float)width * g;
	d = heightTop * (1.0f / width + g);

	artRAM_typedef *artRAM, *jpegRAM, *jpegRAM_mirror;
	artRAM = (artRAM_typedef*)CCM_BASE;
	jpegRAM = (artRAM_typedef*)(CCM_BASE + sizeof(artRAM_typedef));
	jpegRAM_mirror = (artRAM_typedef*)(CCM_BASE + sizeof(artRAM_typedef) + sizeof(artRAM_typedef));

	for(j = 0;j < height;j++){
		LCDSetGramAddr(x, j + y);
		LCDPutCmd(0x0022);
		LCD->RAM;
		for(i = 0;i < width;i++){
			artRAM->p[i][j] = LCD->RAM;
		}
	}

	uint16_t xx, yy;
	if(image_type == 1){
		for(j = 0;j < height;j++){
			for(i = 0;i < width;i++){
				jpegRAM->p[i][j] = music_art_default_74x74[i + j * width];
			}
		}
	} else {
		yy = 0;
		while(jdinfo.output_scanline < jdinfo.output_height){
			jpeg_read_scanlines(&jdinfo, dest_mgr->buffer, dest_mgr->buffer_height);
			xx = 0;
			for(i = 0;i < dest->buffer_width;i += 3){
				jpegRAM->p[xx][yy] = (dest->pixrow[i + 2] >> 3) << 11 | (dest->pixrow[i + 1] >> 2) << 5 |  dest->pixrow[i] >> 3;
				xx++;
			}
			yy++;
		}
		(void) jpeg_finish_decompress(&jdinfo);
		  /* All done. */
	#ifdef MY_DEBUG
		if(jerr.num_warnings == EXIT_WARNING){
			USARTPutString("\r\nEXIT_WARNING");
		}else if(jerr.num_warnings == EXIT_SUCCESS){
			USARTPutString("\r\nEXIT_SUCCESS");
		}
		debug.printf("\r\nlast msg:%s", jdinfo.err->jpeg_message_table[jdinfo.err->last_jpeg_message]);
	#endif

		jpeg_destroy_decompress(&jdinfo);
	}

	pixel_fmt_typedef pixel, pixelBox[2][2], pixelV1, pixelV2;
	int ii, jj, inverse;
	float ratioX0, ratioX1, ratioY0, ratioY1, fx, fy;

	xx = 0, yy = 0;
	for(j = 0;j < height;j++){
		xx = 0;
		LCDSetGramAddr(xx + x, yy + y);
		LCDPutCmd(0x0022);
		for(i = 0;i < width;i++){
//			fx = (float)(xx * a) / (float)(1.0f + xx * g);
//			fy = (float)(xx * d + yy) / (float)(1.0f + xx * g);

			fx = (float)(xx) / (float)(a - g * xx);
			fy = (float)(1 + g * fx) * yy - d * fx;

			ii = (int)fx;
			jj = (int)fy;

			if(fy < 0.0f){
				inverse = 1;
				fy = -fy;
			} else {
				inverse = 0;
			}

			ratioX0 = 1.0f - (fx - (float)ii);
			ratioX1 = 1.0f - ratioX0;

			ratioY0 = 1.0f - (fy - (float)jj);
			ratioY1 = 1.0f - ratioY0;

			if((ii >= 0 && ii < width) && (jj >= 0 && jj < height)){
				pixelBox[0][0].color.d16 = jpegRAM->p[ii][jj];
				if(!inverse){
					if((ii + 1) < width && (jj + 1) < height){
						pixelBox[1][1].color.d16 = jpegRAM->p[ii + 1][jj + 1];
					} else {
						pixelBox[1][1].color.d16 = artRAM->p[ii][jj];
					}
					if((ii + 1) < width){
						pixelBox[1][0].color.d16 = jpegRAM->p[ii + 1][jj];
					} else {
						pixelBox[1][0].color.d16 = artRAM->p[ii][jj];
					}
					if((jj + 1) < height){
						pixelBox[0][1].color.d16 = jpegRAM->p[ii][jj + 1];
					} else {
						pixelBox[0][1].color.d16 = artRAM->p[ii][jj];
					}
				} else {
					if((ii + 1) < width){
						pixelBox[1][0].color.d16 = jpegRAM->p[ii + 1][jj];
					} else {
						pixelBox[1][0].color.d16 = artRAM->p[ii][jj];
					}
					pixelBox[0][1].color.d16 = artRAM->p[ii][(jj - 1) >= 0 ? (jj - 1) : 0];
					pixelBox[1][1].color.d16 = artRAM->p[ii + 1][(jj - 1) >= 0 ? (jj - 1) : 0];
				}

				pixelV1.color.R = __USAT(pixelBox[0][0].color.R * ratioX0 + pixelBox[1][0].color.R * ratioX1, 5);
				pixelV1.color.G = __USAT(pixelBox[0][0].color.G * ratioX0 + pixelBox[1][0].color.G * ratioX1, 6);
				pixelV1.color.B = __USAT(pixelBox[0][0].color.B * ratioX0 + pixelBox[1][0].color.B * ratioX1, 5);

				pixelV2.color.R = __USAT(pixelBox[0][1].color.R * ratioX0 + pixelBox[1][1].color.R * ratioX1, 5);
				pixelV2.color.G = __USAT(pixelBox[0][1].color.G * ratioX0 + pixelBox[1][1].color.G * ratioX1, 6);
				pixelV2.color.B = __USAT(pixelBox[0][1].color.B * ratioX0 + pixelBox[1][1].color.B * ratioX1, 5);

				pixel.color.R = __USAT(pixelV1.color.R * ratioY0 + pixelV2.color.R * ratioY1, 5);
				pixel.color.G = __USAT(pixelV1.color.G * ratioY0 + pixelV2.color.G * ratioY1, 6);
				pixel.color.B = __USAT(pixelV1.color.B * ratioY0 + pixelV2.color.B * ratioY1, 5);

//				LCDSetGramAddr(xx + x, yy + y);
//				LCDPutCmd(0x0022);
				LCD->RAM = pixel.color.d16;
			}
			xx++;
		}
		yy++;
	}

	DRAW_REFLECTION:
//	y = y + height - 3;
	y = y + height;
	for(j = 0;j < height;j++){
		LCDSetGramAddr(x, j + y);
		LCDPutCmd(0x0022);
		LCD->RAM;
		for(i = 0;i < width;i++){
			artRAM->p[width - 1 - i][height - 1 - j] = LCD->RAM;
//			artRAM->p[i][j] = LCD->RAM;
		}
	}

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			jpegRAM_mirror->p[width - 1 - i][j] = jpegRAM->p[i][j];
		}
	}

	jpegRAM = jpegRAM_mirror;

	xx = 0, yy = 0;
	for(j = 0;j < height;j++){
		alpha = (float)j / (float)(height / 3);
		alpha = alpha <= 1.0f ? alpha : 1.0f;
		alpha_ratio = 1.0f - alpha;
		xx = 0;
		for(i = 0;i < width;i++){
			fx = (float)(-xx + width - 1) / (a - g * (-xx + width - 1));
//			fy = (1.0f + g * fx) * (-yy + height - 1) - d * fx;

//			fx = (float)(xx) / (float)(a - g * xx);
			fy = (1.0f + g * fx) * (-yy + height - 1) - d * fx;


			ii = (int)fx;
			jj = (int)fy;

			if(fy < 0.0f){
				fy = -fy;
				inverse = 1;
			} else {
				inverse = 0;
			}

			ratioX0 = 1.0f - (fx - (float)ii);
			ratioX1 = 1.0f - ratioX0;

			ratioY0 = 1.0f - (fy - (float)jj);
			ratioY1 = 1.0f - ratioY0;

			if((ii >= 0 && ii < width) && (jj >= 0 && jj < height)){
				pixelBox[0][0].color.d16 = jpegRAM->p[ii][jj];
				if(!inverse){
					if((ii + 1) < width && (jj + 1) < height){
						pixelBox[1][1].color.d16 = jpegRAM->p[ii + 1][jj + 1];
					} else {
						pixelBox[1][1].color.d16 = artRAM->p[ii][jj];
					}
					if((ii + 1) < width){
						pixelBox[1][0].color.d16 = jpegRAM->p[ii + 1][jj];
					} else {
						pixelBox[1][0].color.d16 = artRAM->p[ii][jj];
					}
					if((jj + 1) < height){
						pixelBox[0][1].color.d16 = jpegRAM->p[ii][jj + 1];
					} else {
						pixelBox[0][1].color.d16 = artRAM->p[ii][jj];
					}
				} else {
					if((ii + 1) < width){
						pixelBox[1][0].color.d16 = jpegRAM->p[ii + 1][jj];
					} else {
						pixelBox[1][0].color.d16 = artRAM->p[ii][jj];
					}
					pixelBox[0][1].color.d16 = artRAM->p[ii][(jj - 1) >= 0 ? (jj - 1) : 0];
					pixelBox[1][1].color.d16 = artRAM->p[ii + 1][(jj - 1) >= 0 ? (jj - 1) : 0];
				}

				pixelV1.color.R = __USAT(pixelBox[0][0].color.R * ratioX0 + pixelBox[1][0].color.R * ratioX1, 5);
				pixelV1.color.G = __USAT(pixelBox[0][0].color.G * ratioX0 + pixelBox[1][0].color.G * ratioX1, 6);
				pixelV1.color.B = __USAT(pixelBox[0][0].color.B * ratioX0 + pixelBox[1][0].color.B * ratioX1, 5);

				pixelV2.color.R = __USAT(pixelBox[0][1].color.R * ratioX0 + pixelBox[1][1].color.R * ratioX1, 5);
				pixelV2.color.G = __USAT(pixelBox[0][1].color.G * ratioX0 + pixelBox[1][1].color.G * ratioX1, 6);
				pixelV2.color.B = __USAT(pixelBox[0][1].color.B * ratioX0 + pixelBox[1][1].color.B * ratioX1, 5);

				pixel.color.R = __USAT(pixelV1.color.R * ratioY0 + pixelV2.color.R * ratioY1, 5);
				pixel.color.G = __USAT(pixelV1.color.G * ratioY0 + pixelV2.color.G * ratioY1, 6);
				pixel.color.B = __USAT(pixelV1.color.B * ratioY0 + pixelV2.color.B * ratioY1, 5);


				// Foreground Image
				pixel.color.R *= alpha_ratio;
				pixel.color.G *= alpha_ratio;
				pixel.color.B *= alpha_ratio;

				pixel_bg.color.d16 = artRAM->p[width - 1 - xx][height - 1 - yy];

				// Background Image
				pixel_bg.color.R *= (1.0f - alpha_ratio);
				pixel_bg.color.G *= (1.0f - alpha_ratio);
				pixel_bg.color.B *= (1.0f - alpha_ratio);

				// Add colors
				pixel.color.R += pixel_bg.color.R;
				pixel.color.G += pixel_bg.color.G;
				pixel.color.B += pixel_bg.color.B;


//				LCDSetGramAddr(-xx + width - 1 + x, -yy + height - 1 + y);
				LCDSetGramAddr(xx + x, yy + y);
				LCDPutCmd(0x0022);
				LCD->RAM = pixel.color.d16;
			}
			xx++;
		}
		yy++;
	}


//DRAW_REFLECTION:

/*
	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);
	t = 5;
	yy = height - 1;
	for(j = y + height - 1;j >= y;j--){
		alpha = ((float)t * 2.3f) / (float)(height);
		alpha = alpha <= 1.0f ? alpha : 1.0f;
		alpha_ratio = 1.0f - alpha;
		xx = 0;
		for(i = x;i < x + width;i++){
//			LCDSetGramAddr(i, j);
//			LCDPutCmd(0x0022);
//			LCD->RAM;
//			pixel_fg.color.d16 = LCD->RAM;
			pixel_fg.color.d16 = jpegRAM->p[xx][yy];

			// Foreground Image
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			LCDSetGramAddr(i, y + height + t);
			LCDPutCmd(0x0022);
			LCD->RAM;
			pixel_bg.color.d16 = LCD->RAM;

			// Background Image
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDSetGramAddr(i, y + height + t);
			LCDPutCmd(0x0022);
			LCDPutData(pixel_fg.color.d16);
			xx++;
		}
		t++;
		yy--;
	}
	*/
//	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);
}

