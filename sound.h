/*
 * sound.h
 *
 *  Created on: 2011/03/12
 *      Author: Tonsuke
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "stm32f4xx_conf.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "fat.h"
#include "usart.h"
#include "icon.h"

#define MUSIC_INFO_POS_Y 24

/*
typedef struct {
	char headStrRIFF[4];
	uint32_t fileSize;
	char headStrWAVE[4], chunkfmt[4];
	uint32_t fmtChunkSize;
	uint16_t formatID, numChannel;
	uint32_t sampleRate, dataSpeed;
	uint16_t blockSize, bitPerSample;
	char chunkdata[4];
	uint32_t bytesWaveData;
}WAVEFormatStruct;
*/

typedef struct {
	uint16_t formatID, numChannel;
	uint32_t sampleRate, dataSpeed;
	uint16_t blockSize, bitPerSample;
}WAVEFormatStruct;


typedef struct {
	char headStrRIFF[4];
	uint32_t fileSize;
	char headStrWAVE[4];
}WAVEFormatHeaderStruct;


typedef struct {
	char chunkfmt[4];
	uint32_t chunkSize;
}WAVEFormatChunkStruct;


volatile struct music_src_p {
	int *prevX, *curX, \
		*totalSec, *media_data_totalBytes, \
		offset, done;
	drawBuff_typedef *drawBuff;
	MY_FILE *fp;
} music_src_p;

volatile struct {
	uint8_t *buff, comp;
	uint32_t bufferSize,\
			 sound_reads, \
			 contentSize;
	void (*func)(void);
	MY_FILE *fp;
} dac_intr;

volatile struct{
	int8_t flag_make_rand, play_continuous, mode_changed, initial_mode;
	uint8_t *pRandIdEntry;
} shuffle_play;

typedef struct{
	int currentTotalSec;
	uint8_t update, \
			comp;
}music_play_typedef;

typedef struct {
	int enable, type, tic, cnt, shifts;
	int16_t sLeft, sRight;
}sound_fade_typedef;


#define RET_PLAY_NORM  0
#define RET_PLAY_STOP -1
#define RET_PLAY_NEXT -2
#define RET_PLAY_PREV -3
#define RET_PLAY_INVALID -99

extern music_play_typedef music_play;

extern uint8_t navigation_loop_mode;
extern uint8_t bass_boost_mode;
extern uint8_t reverb_effect_mode;
extern uint8_t vocal_cancel_mode;

//uint8_t SOUND_BUFFER[9216]; // interruption occurs 50msec each

#define UPPER_OF_WORD(x) (x >> 16)
#define LOWER_OF_WORD(x) (x & 0x0000ffff)


#define NAV_ONE_PLAY_EXIT         0
#define NAV_PLAY_ENTIRE           1
#define NAV_INFINITE_PLAY_ENTIRE  2
#define NAV_INFINITE_ONE_PLAY     3
#define NAV_SHUFFLE_PLAY          4


extern void DAC_Buffer_Process_Stereo_S16bit();
extern void DAC_Buffer_Process_Mono_U8bit();
extern void setStrSec(char *timeStr, int totalSec);
extern int PlaySound(int id);
extern int PlayMusic(int id);
extern void SOUNDDMAConf(void *dacOutputReg, size_t blockSize, size_t periphSize);
extern void SOUNDInitDAC(uint32_t sampleRate);

extern void musicTouch();
extern int musicPause();



#endif /* SOUND_H_ */
