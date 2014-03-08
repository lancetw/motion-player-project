#include "stm32f4xx_conf.h"
#include "lcd.h"
#include "usart.h"
#include "delay.h"
#include <string.h>

#include "xmodem.h"

volatile uint32_t timeout;
volatile uint8_t Crcflg, seqno;

void TIM2_IRQHandler(void){
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	timeout++;
}

int16_t usart_polling_timeout(uint32_t t)		/* タイムアウト付きUSARTポーリング */
{
	while(!(USART3->SR & USART_FLAG_RXNE)){
		if(timeout > t) return -1;
	}
	return USART3->DR;
}

void xmodem_init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    /* 1Hz = CK_INT(F_APB1(F_CPU(168MHz) / APB_PSC(4)) * 2) / TIM_PSC(10000)  */
	/* CK_INT = F_CPU(168MHz) / APB_PSC(4) * 2 = 84MHz */
	/* COUNT10ms = 10ms / 1/84MHz = 840000 */
	TIM_TimeBaseInitStructure.TIM_Period = ((SystemCoreClock / 4) * 2) / 10000 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 100 - 1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
}

int xmodem_start_session()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	uint8_t retry = 0;
	
	seqno = 1;
	
	debug.printf("\r\nstarting session...\r\n");

	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); /* USART受信割り込み一時停止 */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); /* TIM2割り込み許可 */
	
WAIT_NACK:
	timeout = 0;
	switch(usart_polling_timeout(WAIT_NACK_TIMEOUT)){		/* 接続要求待ち */
		case NAK:							/* NAK受信 Checksum ブロック送信へ*/
			Crcflg = FALSE;
			return 1;
		case 'C':							/* C文字受信 CRC ブロック送信へ*/
			Crcflg = TRUE;
			return 1;
		case CAN:							/* CANまたはETX受信　XMODEM終了 */
		case ETX:
			break;
		case -1:							/* タイムアウト 接続要求待ちへ */
		default:
			if(++retry <= WAIT_NACK_RETRY) goto WAIT_NACK;
			break;
	}

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); /* USART受信割り込み再開 */
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); /* TIM2割り込み停止 */

	/* Disable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	return (-1);
}

int xmodem_transmit(void* p, uint32_t blocks)	/* XMODEM送信ルーチン */
{
	register uint32_t i, n;
	uint8_t xbuf[133], *buf = (uint8_t*)p;
	uint8_t retry = 0;
	uint16_t crc;

	//mmcBlockRead((uint8_t*)fbuf, sector);
	
	for(n = 0;n < blocks;n++){					/* n個のブロックを送信 */
	REQ_NACK:
		xbuf[HEAD] = SOH;						/* SOH付加 */
		xbuf[SEQ] = seqno;						/* 現在のシーケンス番号付加 */
		xbuf[COM] = seqno ^ 255;				/* シーケンス番号の補数付加 */
		
		memcpy((uint8_t*)&xbuf[DAT], (uint8_t*)&buf[n * 128], 128);

		if(!Crcflg) {							/* Checksum計算 */
			xbuf[CHK] = 0;
			for(i = DAT;i < CHK;i++)
				xbuf[CHK] += xbuf[i];
			
			for(i = HEAD;i <= CHK;i++)			/* ブロック送出 */
				USARTPutData(xbuf[i]);
		}
		else{									/* CRC計算 */
			crc = 0;
			for(i = DAT;i < CHK;i++)
//				crc = _crc_xmodem_update(crc, xbuf[i]);	/* CRCコード生成 */
			
			xbuf[CRCH] = (crc >> 8) & 0x00FF;
			xbuf[CRCL] = crc & 0x00FF;
			
			for(i = HEAD;i <= CRCL;i++)			/* ブロック送出 */
				USARTPutData(xbuf[i]);
		}

	WAIT_ACK:
		timeout = 0;
		switch(usart_polling_timeout(WAIT_ACK_TIMEOUT)){		/* ACK応答待ち */
			case ACK:							/* ACK受信 次のシーケンスへ */
				retry = 0;
				break;
			case NAK:							/* NAK受信 ブロック再送 */
				if(++retry < REQ_NACK_RETRY) goto REQ_NACK;
				goto END;
			case CAN:							/* CAN受信 XMODEM終了 */
				goto END;
			case -1:							/* タイムアウト CAN送信後 XMODEM終了*/
			default:
				if(++retry <= WAIT_ACK_RETRY) goto WAIT_ACK;
				goto CAN_END;
		}
		seqno++;								/* 次のシーケンス番号に更新 */
		retry = 0;
	}
	
	return 0;
	
CAN_END:
	USARTPutData(CAN);						/* CANを送信して終了 */
END:
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); /* USART受信割り込み再開 */
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); /* TIM2割り込み停止 */
	Delayms(100);
	return -1;
}

void xmodem_end_session()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	USARTPutData(EOT);						/* ブロック送信完了 EOT送出 */
	timeout = 0;
	while(usart_polling_timeout(100) != ACK);	/* ACK応答待ち */
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); /* USART受信割り込み再開 */
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); /* TIM2割り込み停止 */

	/* Disable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	Delayms(100);
}

void xput()
{
	const uint8_t bmp_header[] = {
		0x42, 0x4d, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00,
		0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
		0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x1f, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	uint8_t buf[1024];
	int totalPixel = LCD_WIDTH * LCD_HEIGHT + sizeof(bmp_header) / sizeof(uint16_t);
	register int i, n, x, y;
	uint16_t *p_ram;

	debug.printf("\r\nXMODEM");

	xmodem_init();
	if(xmodem_start_session() == -1){
		debug.printf("\r\nsesson timeout...");
		return;
	}

	memcpy((uint8_t*)buf, (uint8_t*)bmp_header, sizeof(bmp_header));
	p_ram = (uint16_t*)&buf[sizeof(bmp_header)];
	n = ( sizeof(buf) -  sizeof(bmp_header) ) / sizeof(uint16_t);

	x = 0, y = LCD_HEIGHT - 1;
	LCDSetWindowArea(0 ,0, LCD_WIDTH, LCD_HEIGHT);
	LCDSetGramAddr(0, LCD_HEIGHT - 1);
	LCDPutCmd(0x0022);
	LCD->RAM;

	do{
		for(i = 0;i < n;i++){
			*p_ram++ = LCD->RAM;
			if(++x >= LCD_WIDTH){
				x = 0;
				if(--y < 0) y = LCD_HEIGHT - 1;
				LCDSetGramAddr(x, y);
				LCDPutCmd(0x0022);
				LCD->RAM;
			}
		}

		xmodem_transmit((uint8_t*)buf, sizeof(buf) / 128);

		totalPixel -= n;
		n = sizeof(buf) / sizeof(uint16_t);
		p_ram = (uint16_t*)buf;
	}while(totalPixel > 0);

	xmodem_end_session();
}
