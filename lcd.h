/*
 * lcd.h
 *
 *  Created on: 2011/02/19
 *      Author: masayuki
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f4xx_conf.h"
#include "fat.h"
#include "font.h"
#include "icon.h"

#define red	        0xf800
#define green		0x07e0
#define blue		0x001f
#define skyblue     0x7e9f
#define white		0xffff
#define gray		0x8c51
#define yellow		0xFFE0
#define cyan	 	0x07FF
#define purple		0xF81F
#define black		0x0000

typedef enum
{
	RED,
	GREEN,
	BLUE,
	SKYBLUE,
	WHITE,
	GRAY,
	YELLOW,
	CYAN,
	PURPLE,
	BLACK
}colors;

typedef enum
{
	FILE_TYPE_WAV = 2,
	FILE_TYPE_MP3,
	FILE_TYPE_AAC,
	FILE_TYPE_JPG,
	FILE_TYPE_MOV,
	FILE_TYPE_PCF,
}selected_file_type;

#define ARCHIVE_COLOR WHITE
#define DIR_COLOR WHITE
#define HEADER_COLOR WHITE

#define LCD_CNT_PORT  GPIOD

#define LCD_WIDTH  320
#define LCD_HEIGHT 240

#define PAGE_NUM_ITEMS 9 //
#define HEIGHT_ITEM 24 // 24px

#define PROGRESS_CIRCULAR_POS_X 	303
#define PROGRESS_CIRCULAR_POS_Y 	  1
#define PROGRESS_CIRCULAR_WIDTH		 16
#define PROGRESS_CIRCULAR_HEIGHT	 16
#define PROGRESS_CIRCULAR_FRAMES     12

#define	LCD_RESET 3

typedef struct
{
	void (*putChar)(uint16_t asc, colors color);
	void (*putWideChar)(uint16_t code, colors color);
	uint16_t (*getCharLength)(uint16_t code, uint16_t font_width);
}LCD_FUNC_typedef;

extern LCD_FUNC_typedef LCD_FUNC;

typedef struct{
	int16_t curTime, prevTime;
	union{
		uint8_t d8;
		struct {
			uint8_t dimLight : 1;
			uint8_t enable : 1;
			uint8_t stop_mode : 1;
			uint8_t reserved : 5;
		};
	}flags;
}time_typedef;

typedef struct{
	union{
		uint16_t d16;
		struct {
			uint16_t B : 5;
			uint16_t G : 6;
			uint16_t R : 5;
		};
	}color;
}pixel_fmt_typedef;


extern volatile time_typedef time;

typedef struct
{
	uint16_t pos, pages, pageIdx;
}cursor_typedef;

typedef struct
{
	int waitExitKey;
	uint16_t idEntry;
}LCDStatusStruct_typedef;

extern volatile cursor_typedef cursor;
extern volatile LCDStatusStruct_typedef LCDStatusStruct;

/*
typedef struct{
	char headStr[4];
	uint32_t audioBlockSize;
	uint32_t videoBlockSize;
}MoviePlayStruct;
*/
typedef struct
{
  __IO uint16_t REG;
  __IO uint16_t RAM;
} LCD_TypeDef;


typedef struct{
	__IO uint32_t DMA_CCR;
	__IO uint32_t DMA_CNDTR;
	__IO uint32_t DMA_CPAR;
	__IO uint32_t DMA_CMAR;
}DMA_TypeConf;


#define LCD_BASE    ((uint32_t)(0x6001fffe)) // 2^16 - 2
#define LCD         ((LCD_TypeDef *)LCD_BASE)

#define LCDPutCmd(x) (LCD->REG = x)
#define LCDPutData(x) (LCD->RAM = x)

static const uint16_t colorc[] = {red, green, blue, skyblue, white, gray, yellow, cyan, purple, black};
volatile uint16_t clx, cly;
uint16_t cursorRAM[LCD_WIDTH * 13];
uint16_t progress_circular_bar_16x16x12_buff[16*16*12];
//extern void put_pixels_row_by_DMA(void* DstBuff, uint32_t dLen);
extern void MergeCircularProgressBar(int8_t menubar);
extern void LCDInit(void);
//inline void LCDPutCmd(uint16_t cmd);
//inline void LCDPutData(uint16_t data);
//static uint16_t LCDGetData(void);
extern void LCDBackLightInit(void);
extern void LCDBackLightTimerInit(void);
extern void LCDBackLightDisable(void);
extern void LCDSetWindowArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
extern inline void LCDSetGramAddr(int x, int y);
extern void LCDPutPos(uint16_t x, uint16_t y, colors color);
extern void LCDGotoXY(int x, int y);
extern void LCDPutWideCharDefault(uint16_t code, colors color);
extern void LCDPutWideChar(uint16_t code, colors color);
extern void LCDPutAscii(uint16_t asc, colors color);
extern void LCDPutString(const char *str, colors color);
extern void LCDPutStringN(const char *str, uint16_t endPosX, colors color);
extern uint16_t LCDPutStringUTF8(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, colors color);
extern uint16_t LCDGetStringUTF8PixelLength(uint8_t *s, uint16_t font_width);
extern uint16_t LCDPutStringSJISN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, uint8_t color);
extern uint16_t LCDGetStringSJISPixelLength(uint8_t *s, uint16_t font_width);
extern uint16_t LCDPutStringLFN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, colors color);
extern uint16_t LCDGetStringLFNPixelLength(void *s, uint16_t font_width);
extern void LCDPutCircularProgressbar(void);
extern void LCDClear(int x,int y, colors color);
extern void LCDClearWithBgImg(void);
extern void LCDPutBgImg(const uint16_t *p);
extern void LCDPrintFileList(void);
extern void LCDStoreCursorBar(int curPos);
extern void LCDPutBgImgFiler(void);
extern void LCDPutBgImgMusic(void);
extern void LCDDrawSquare(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colors color);
void LCDPutCursorBar(int curPos);
void LCDSelectCursorBar(int curPos);
extern void LCDCursorUp(void);
extern void LCDCursorDown(void);
extern void LCDCursorEnter(void);
extern void LCDStoreBgImgToBuff(int startPosX, int startPosY, int width, int height, uint16_t *p);
extern void LCDStoreBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p);
extern void LCDPutBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p);
extern void LCDPutIcon(int startPosX, int startPosY, int width, int height, const uint16_t *d, const uint8_t *a);
extern void LCDTouchPos(void);
extern void LCDTouchReleased(void);
extern void LCDTouchPoint(void);
extern int dojpeg(int id, uint8_t djpeg_arrows, uint8_t arrow_clicked);
extern void dispArtWork(MY_FILE *input_file);

#endif /* LCD_H_ */
