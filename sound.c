/*
 * sound.c
 *
 *  Created on: 2011/03/12
 *      Author: Tonsuke
 */


#include "sound.h"

#include "lcd.h"
#include "icon.h"
#include "xpt2046.h"
#include "aac.h"
#include "mp3.h"

#include "settings.h"

#include "board_config.h"

#include "pcf_font.h"

#include "mpool.h"

#include "arm_math.h"
#include "fft.h"
#include "fx.h"


uint8_t navigation_loop_mode;
uint8_t bass_boost_mode;
uint8_t reverb_effect_mode;
uint8_t vocal_cancel_mode;

music_play_typedef music_play;

void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		music_play.currentTotalSec++;
		music_play.update = 1;
		music_play.comp = 0;
	}
}

void DAC_Buffer_Process_Stereo_S16bit_PhotoFrame()
{
	int i, halfBufferSize = dac_intr.bufferSize >> 1;
	uint32_t *pabuf;
	uint8_t *outbuf;

	if(SOUND_DMA_HALF_TRANS_BB){ // Half
		SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
		outbuf = (uint8_t*)dac_intr.buff;
		pabuf = (uint32_t*)outbuf;
	} else if(SOUND_DMA_FULL_TRANS_BB) {	// Full
		SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
		outbuf = (uint8_t*)&dac_intr.buff[halfBufferSize];
		pabuf = (uint32_t*)outbuf;
	}

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);
	for(i = 0;i < halfBufferSize >> 2;i += 8){ // signed to unsigned
		pabuf[0] ^= 0x80008000;
		pabuf[1] ^= 0x80008000;
		pabuf[2] ^= 0x80008000;
		pabuf[3] ^= 0x80008000;
		pabuf[4] ^= 0x80008000;
		pabuf[5] ^= 0x80008000;
		pabuf[6] ^= 0x80008000;
		pabuf[7] ^= 0x80008000;
		pabuf += 8;
	}
	dac_intr.sound_reads += halfBufferSize;

	if(dac_intr.sound_reads > (halfBufferSize * 10)){
		AUDIO_OUT_ENABLE;
	}

	if(dac_intr.sound_reads >= dac_intr.contentSize){
		NVIC_InitTypeDef NVIC_InitStructure;

	    AUDIO_OUT_SHUTDOWN;
	    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	    DMA_Cmd(DMA1_Stream1, DISABLE);

		/* Disable DMA1_Stream1 gloabal Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVIC_InitStructure);

		my_fclose(dac_intr.fp);
		dac_intr.fp = '\0';

		dac_intr.comp = 1;
	}
}

void DAC_Buffer_Process_Stereo_S16bit()
{
	int i, halfBufferSize = dac_intr.bufferSize >> 1;
	uint32_t *pabuf;
	uint8_t *outbuf;

	if(SOUND_DMA_HALF_TRANS_BB){ // Half
		SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
		outbuf = (uint8_t*)dac_intr.buff;
		pabuf = (uint32_t*)outbuf;
	} else if(SOUND_DMA_FULL_TRANS_BB) {	// Full
		SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
		outbuf = (uint8_t*)&dac_intr.buff[halfBufferSize];
		pabuf = (uint32_t*)outbuf;
	}

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);
	for(i = 0;i < halfBufferSize >> 2;i += 8){ // signed to unsigned
		pabuf[0] ^= 0x80008000;
		pabuf[1] ^= 0x80008000;
		pabuf[2] ^= 0x80008000;
		pabuf[3] ^= 0x80008000;
		pabuf[4] ^= 0x80008000;
		pabuf[5] ^= 0x80008000;
		pabuf[6] ^= 0x80008000;
		pabuf[7] ^= 0x80008000;
		pabuf += 8;
	}
	dac_intr.sound_reads += halfBufferSize;
}


void DAC_Buffer_Process_Mono_U8bit()
{
	int halfBufferSize = dac_intr.bufferSize >> 1;
	uint8_t *outbuf;

	if(SOUND_DMA_HALF_TRANS_BB){ // Half
		SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
		outbuf = (uint8_t*)dac_intr.buff;
	} else if(SOUND_DMA_FULL_TRANS_BB) { // Full
		SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
		outbuf = (uint8_t*)&dac_intr.buff[halfBufferSize];
	}

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);
	dac_intr.sound_reads += halfBufferSize;
}

void DMA1_Stream1_IRQHandler(void)
{
	dac_intr.func();
}

void SOUNDDMAConf(void *dacOutputReg, size_t blockSize, size_t periphDataSize)
{
	DMA_InitTypeDef DMA_InitStructure;

	/*!< DMA1 Channel7 Stream1 disable */
	DMA_Cmd(DMA1_Stream1, DISABLE);

	DMA_DeInit(DMA1_Stream1);
	/*!< DMA1 Channel7 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)dacOutputReg;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dac_intr.buff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = dac_intr.bufferSize / blockSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = (periphDataSize > 0)?0x400 * periphDataSize:0;
	DMA_InitStructure.DMA_MemoryDataSize = (periphDataSize > 0)?0x400 * periphDataSize:0;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	DMA_Init(DMA1_Stream1, &DMA_InitStructure);
}

void SOUNDInitDAC(uint32_t sampleRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitStructure;

//	CS43L22Init();

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM6, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

	// PC5 MAX4410 Audio Amp Shutdown
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	// 初期化関数を読み出します。

	AUDIO_OUT_SHUTDOWN;

	// PA4 PA5 DAC_OUT1 DAC_OUT2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	// 初期化関数を読み出します。

	DAC_DeInit();
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);

	DAC_DMACmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_2, ENABLE);

	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);

	TIM6->ARR = ((SystemCoreClock / 4) * 2) / sampleRate - 1;
	TIM6->PSC = 0;

	TIM6->CR1 |= _BV(7);
	TIM6->CR2 |= TIM_TRGOSource_Update;
	TIM6->DIER |= TIM_DMA_Update | _BV(0); // Interrupt Enable;
	TIM6->CR1 |= _BV(0);
}

void setStrSec(char *timeStr, int totalSec)
{
	int minute, sec, it = 0, temp;
	const char asciiTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	if(totalSec < 0){
		timeStr[it++] = '-';
		totalSec = abs(totalSec);
	}

	minute = totalSec / 60;
	sec = totalSec % 60;
	if((temp = minute / 100) > 0){
		timeStr[it++] = asciiTable[temp];
		timeStr[it++] = asciiTable[(minute % 100) / 10];
	} else if((temp = minute / 10) > 0) {
		timeStr[it++] = asciiTable[temp];
	}
	timeStr[it++] = asciiTable[minute % 10];
	timeStr[it++] = ':';
	timeStr[it++] = asciiTable[sec / 10];
	timeStr[it++] = asciiTable[sec % 10];
	timeStr[it] = '\0';
}

int PlaySoundPhotoFrame(int id)
{
	int ret;

	WAVEFormatStruct wav;
	WAVEFormatHeaderStruct wavHeader;
	WAVEFormatChunkStruct wavChunk;

	char str[10];

	NVIC_InitTypeDef NVIC_InitStructure;

	MY_FILE *infile;

	if(!(infile = my_fopen(id))){
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	my_fread(&wavHeader, 1, sizeof(WAVEFormatHeaderStruct), infile);

	debug.printf("\r\n\n[WAVE]");

	if(strncmp(wavHeader.headStrRIFF, "RIFF", 4) != 0){
		debug.printf("\r\nNot contain RIFF chunk");
		ret = RET_PLAY_STOP;
		goto END_WAV;
	}

	debug.printf("\r\nFile Size:%d", wavHeader.fileSize);

	if(strncmp(wavHeader.headStrWAVE, "WAVE", 4) != 0){
		debug.printf("\r\nThis is not WAVE file.");
		ret = RET_PLAY_STOP;
		goto END_WAV;
	}

	int restBytes = wavHeader.fileSize;

	while(1){ // loop until format chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "fmt ", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto END_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	my_fread(&wav, 1, sizeof(WAVEFormatStruct), infile);
	my_fseek(infile, wavChunk.chunkSize - sizeof(WAVEFormatStruct), SEEK_CUR);

	restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);

	while(1){ // loop until data chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "data", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto END_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	memset(str, '\0', sizeof(str));
	debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
	debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);

	debug.printf("\r\n\nformatID:%d", wav.formatID);
	debug.printf("\r\nNum Channel:%d", wav.numChannel);
	debug.printf("\r\nSampling Rate:%d", wav.sampleRate);
	debug.printf("\r\nData Speed:%d", wav.dataSpeed);
	debug.printf("\r\nBlock Size:%d", wav.blockSize);
	debug.printf("\r\nBit Per Sample:%d", wav.bitPerSample);
	debug.printf("\r\nBytes Wave Data:%d", wavChunk.chunkSize);


    dac_intr.fp = infile;
    dac_intr.buff = (uint8_t*)cursorRAM;//SOUND_BUFFER;
    dac_intr.bufferSize = sizeof(cursorRAM);
    dac_intr.contentSize = infile->fileSize - infile->seekBytes;
	dac_intr.comp = 0;

    my_fread(dac_intr.buff, 1, dac_intr.bufferSize, dac_intr.fp);
    dac_intr.sound_reads = dac_intr.bufferSize;

    if(wav.bitPerSample == 8){
        dac_intr.func = DAC_Buffer_Process_Mono_U8bit;
    } else {
        dac_intr.func = DAC_Buffer_Process_Stereo_S16bit_PhotoFrame;
    }

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	// Enable DMA1_Stream1 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    SOUNDInitDAC(wav.sampleRate);
    SOUNDDMAConf((void*)&DAC->DHR12LD, wav.blockSize, (wav.bitPerSample / 8) * wav.numChannel);
    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
    DMA_Cmd(DMA1_Stream1, ENABLE);

	END_WAV:
	EXIT_WAV:

	/* Disable DMA1_Stream1 gloabal Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//	NVIC_Init(&NVIC_InitStructure);

//	my_fclose(infile);

	return 0;
}

int PlaySound(int id)
{
	time.flags.enable = 0;
	TouchPenIRQ_Disable();
	TOUCH_PINIRQ_DISABLE;
	touch.func = touch_empty_func;

	int i;
	uint32_t *pabuf;
	uint8_t *outbuf;
	char str[10];

	int totalSec, remainTotalSec, media_data_totalBytes;
	int curX = 0, prevX = 0;
	volatile int ret = RET_PLAY_NORM;

	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	void *putCharTmp = '\0', *putWideCharTmp = '\0';
	drawBuff_typedef dbuf, *drawBuff;
	drawBuff = &dbuf;
	_drawBuff = drawBuff;

	char timeStr[20];

	WAVEFormatStruct wav;
	WAVEFormatHeaderStruct wavHeader;
	WAVEFormatChunkStruct wavChunk;

	MY_FILE *infile, file_covr;


	if(!(infile = my_fopen(id))){
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	my_fread(&wavHeader, 1, sizeof(WAVEFormatHeaderStruct), infile);

	debug.printf("\r\n\n[WAVE]");

	if(strncmp(wavHeader.headStrRIFF, "RIFF", 4) != 0){
		debug.printf("\r\nNot contain RIFF chunk");
		ret = RET_PLAY_STOP;
		goto END_WAV;
	}

	debug.printf("\r\nFile Size:%d", wavHeader.fileSize);

	if(strncmp(wavHeader.headStrWAVE, "WAVE", 4) != 0){
		debug.printf("\r\nThis is not WAVE file.");
		ret = RET_PLAY_STOP;
		goto END_WAV;
	}

	int restBytes = wavHeader.fileSize;

	while(1){ // loop until format chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "fmt ", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto END_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	my_fread(&wav, 1, sizeof(WAVEFormatStruct), infile);
	my_fseek(infile, wavChunk.chunkSize - sizeof(WAVEFormatStruct), SEEK_CUR);

	restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);

	while(1){ // loop until data chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "data", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto END_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	music_src_p.curX = &curX;
	music_src_p.prevX = &prevX;
	music_src_p.media_data_totalBytes = &media_data_totalBytes;
	music_src_p.totalSec = &totalSec;
	music_src_p.drawBuff = drawBuff;
	music_src_p.fp = infile;

	LCDPutBgImgMusic();

	file_covr.clusterOrg = 0;

	dispArtWork(&file_covr);
	LCDPutIcon(0, 155, 320, 80, music_underbar_320x80, music_underbar_320x80_alpha);

	memset(str, '\0', sizeof(str));
	debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
	debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);

	debug.printf("\r\n\nformatID:%d", wav.formatID);
	debug.printf("\r\nNum Channel:%d", wav.numChannel);
	debug.printf("\r\nSampling Rate:%d", wav.sampleRate);
	debug.printf("\r\nData Speed:%d", wav.dataSpeed);
	debug.printf("\r\nBlock Size:%d", wav.blockSize);
	debug.printf("\r\nBit Per Sample:%d", wav.bitPerSample);
	debug.printf("\r\nBytes Wave Data:%d", wavChunk.chunkSize);

	uint32_t data_offset = infile->seekBytes;

	if(wav.bitPerSample != 16){
		debug.printf("\r\n**Bit Per Sample must be 16bit**");
		debug.printf("\r\ndata offset:%d", data_offset);
		ret = RET_PLAY_STOP;
		goto END_WAV;
	}

	int xTag = 110, yTag = 87, disp_limit = 300, strLen;

	putCharTmp = LCD_FUNC.putChar;
	putWideCharTmp = LCD_FUNC.putWideChar;

	if(!pcf_font.c_loaded){
		LCD_FUNC.putChar = PCFPutChar16px;
		LCD_FUNC.putWideChar = PCFPutChar16px;
	} else {
		LCD_FUNC.putChar = C_PCFPutChar16px;
		LCD_FUNC.putWideChar = C_PCFPutChar16px;
	}
	disp_limit = 288;

	uint8_t strNameLFN[80];
	if(setLFNname(strNameLFN, id, LFN_WITHOUT_EXTENSION, sizeof(strNameLFN))){
		strLen = LCDGetStringLFNPixelLength(strNameLFN, 16);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = LCD_WIDTH - 1;
		} else {
			disp_limit = LCD_WIDTH - 20;
			yTag -= 10;
		}
		LCDGotoXY(xTag + 1, yTag + 1);
		LCDPutStringLFN(xTag + 1, disp_limit, 2, strNameLFN, BLACK);
		LCDGotoXY(xTag, yTag);
		LCDPutStringLFN(xTag, disp_limit - 1, 2, strNameLFN, WHITE);
	} else {
		char strNameSFN[9];
		memset(strNameSFN, '\0', sizeof(strNameSFN));
		setSFNname(strNameSFN, id);
		LCDGotoXY(xTag + 1, yTag + 1);
		LCDPutString(strNameSFN, BLACK);
		LCDGotoXY(xTag, yTag);
		LCDPutString(strNameSFN, WHITE);
	}

	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;

	char s[20];
	SPRINTF((char*)s, "%d/%d", id, fat.fileCnt - 1);
	LCDGotoXY(21, MUSIC_INFO_POS_Y + 1);
	LCDPutString((char*)s, BLACK);
	LCDGotoXY(20, MUSIC_INFO_POS_Y);
	LCDPutString((char*)s, WHITE);

	if(settings_group.music_conf.b.musicinfo){
		LCDGotoXY(71, MUSIC_INFO_POS_Y + 1);
		LCDPutString("WAV", BLACK);
		LCDGotoXY(70, MUSIC_INFO_POS_Y);
		LCDPutString("WAV", WHITE);

		LCDGotoXY(111, MUSIC_INFO_POS_Y + 1);
		LCDPutString(wav.numChannel == 2 ? "Stereo" : "Mono", BLACK);
		LCDGotoXY(110, MUSIC_INFO_POS_Y);
		LCDPutString(wav.numChannel == 2 ? "Stereo" : "Mono", WHITE);

		SPRINTF(s, "%1.2fMbps", (float)(wav.dataSpeed * 8) / 1000000.0f);
		LCDGotoXY(171, MUSIC_INFO_POS_Y + 1);
		LCDPutString(s, BLACK);
		LCDGotoXY(170, MUSIC_INFO_POS_Y);
		LCDPutString(s, WHITE);

		SPRINTF(s, "%dHz", (int)wav.sampleRate);
		LCDGotoXY(241, MUSIC_INFO_POS_Y + 1);
		LCDPutString(s, BLACK);
		LCDGotoXY(240, MUSIC_INFO_POS_Y);
		LCDPutString(s, WHITE);
	}


	putCharTmp = LCD_FUNC.putChar;
	putWideCharTmp = LCD_FUNC.putWideChar;

	if(!pcf_font.c_loaded){
		LCD_FUNC.putChar = PCFPutCharCache;
		LCD_FUNC.putWideChar = PCFPutCharCache;

		extern uint16_t cursorRAM[];

		PCFSetGlyphCacheStartAddress((void*)cursorRAM);
		PCFCachePlayTimeGlyphs(12);
	} else {
		LCD_FUNC.putChar = C_PCFPutChar;
		LCD_FUNC.putWideChar = C_PCFPutChar;
	}

	media_data_totalBytes = wavChunk.chunkSize;
	totalSec = wavChunk.chunkSize / wav.dataSpeed;
	setStrSec(timeStr, totalSec);
	debug.printf("\r\nplay time:%s", timeStr);

	// time elapsed
	drawBuff->timeElapsed.x = 14;
	drawBuff->timeElapsed.y = 188;
	drawBuff->timeElapsed.width = 50;
	drawBuff->timeElapsed.height = 13;
	LCDStoreBgImgToBuff(drawBuff->timeElapsed.x, drawBuff->timeElapsed.y, \
			            drawBuff->timeElapsed.width, drawBuff->timeElapsed.height, drawBuff->timeElapsed.p);

	// time remain
	drawBuff->timeRemain.x = totalSec < 6000 ? 268 : 260;
	drawBuff->timeRemain.y = 188;
	drawBuff->timeRemain.width = 50;
	drawBuff->timeRemain.height = 13;
	LCDStoreBgImgToBuff(drawBuff->timeRemain.x, drawBuff->timeRemain.y, \
			            drawBuff->timeRemain.width, drawBuff->timeRemain.height, drawBuff->timeRemain.p);

	drawBuff->posision.x = 0;
	drawBuff->posision.y = 168;
	drawBuff->posision.width = 16;
	drawBuff->posision.height = 16;
	LCDStoreBgImgToBuff(drawBuff->posision.x, drawBuff->posision.y, \
						drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);

	drawBuff->navigation.x = 142;
	drawBuff->navigation.y = 189;
	drawBuff->navigation.width = 32;
	drawBuff->navigation.height = 32;
	LCDStoreBgImgToBuff(drawBuff->navigation.x, drawBuff->navigation.y, \
			            drawBuff->navigation.width, drawBuff->navigation.height, drawBuff->navigation.p);

	drawBuff->fft_analyzer_left.x = FFT_ANALYZER_LEFT_POS_X;
	drawBuff->fft_analyzer_left.y = FFT_ANALYZER_LEFT_POS_Y;
	drawBuff->fft_analyzer_left.width = 32;
	drawBuff->fft_analyzer_left.height = 32;
	LCDStoreBgImgToBuff(drawBuff->fft_analyzer_left.x, drawBuff->fft_analyzer_left.y, \
			            drawBuff->fft_analyzer_left.width, drawBuff->fft_analyzer_left.height, drawBuff->fft_analyzer_left.p);

	drawBuff->fft_analyzer_right.x = FFT_ANALYZER_RIGHT_POS_X;
	drawBuff->fft_analyzer_right.y = FFT_ANALYZER_RIGHT_POS_Y;
	drawBuff->fft_analyzer_right.width = 32;
	drawBuff->fft_analyzer_right.height = 32;
	LCDStoreBgImgToBuff(drawBuff->fft_analyzer_right.x, drawBuff->fft_analyzer_right.y, \
			            drawBuff->fft_analyzer_right.width, drawBuff->fft_analyzer_right.height, drawBuff->fft_analyzer_right.p);

	drawBuff->navigation_loop.x = 277;
	drawBuff->navigation_loop.y = 207;
	drawBuff->navigation_loop.width = 24;
	drawBuff->navigation_loop.height = 18;
	LCDStoreBgImgToBuff(drawBuff->navigation_loop.x, drawBuff->navigation_loop.y, \
			            drawBuff->navigation_loop.width, drawBuff->navigation_loop.height, drawBuff->navigation_loop.p);
	switch(navigation_loop_mode){
	case NAV_ONE_PLAY_EXIT: // 1 play exit
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_bar_24x18, navigation_bar_24x18_alpha);
		break;
	case NAV_PLAY_ENTIRE: // play entire in directry
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_entire_loop_24x18, navigation_entire_loop_24x18_alpha);
		break;
	case NAV_INFINITE_PLAY_ENTIRE: // infinite play entire in directry
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_infinite_entire_loop_24x18, navigation_infinite_entire_loop_24x18_alpha);
		break;
	case NAV_INFINITE_ONE_PLAY: // infinite 1 play
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_one_loop_24x18, navigation_one_loop_24x18_alpha);
		break;
	case NAV_SHUFFLE_PLAY: // shuffle
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_shuffle_24x18, navigation_shuffle_24x18_alpha);
		break;
	default:
		break;
	}

	LCDPutIcon(drawBuff->navigation.x, drawBuff->navigation.y, \
			   drawBuff->navigation.width, drawBuff->navigation.height, \
			   navigation_pause_patch_32x32, navigation_pause_patch_32x32_alpha);


	/* Update Bass Boost Icon */
	drawBuff->bass_boost.x = 10;
	drawBuff->bass_boost.y = 3;
	drawBuff->bass_boost.width = 24;
	drawBuff->bass_boost.height = 18;
	LCDStoreBgImgToBuff(drawBuff->bass_boost.x, drawBuff->bass_boost.y, \
			            drawBuff->bass_boost.width, drawBuff->bass_boost.height, drawBuff->bass_boost.p);
	Update_Bass_Boost_Icon(bass_boost_mode);

	/* Update Reverb Effect Icon */
	drawBuff->reverb_effect.x = 60;
	drawBuff->reverb_effect.y = 2;
	drawBuff->reverb_effect.width = 24;
	drawBuff->reverb_effect.height = 18;
	LCDStoreBgImgToBuff(drawBuff->reverb_effect.x, drawBuff->reverb_effect.y, \
			            drawBuff->reverb_effect.width, drawBuff->reverb_effect.height, drawBuff->reverb_effect.p);
	Update_Reverb_Effect_Icon(reverb_effect_mode);

	/* Update Vocal Canceler Icon */
	drawBuff->vocal_cancel.x = 107;
	drawBuff->vocal_cancel.y = 5;
	drawBuff->vocal_cancel.width = 24;
	drawBuff->vocal_cancel.height = 18;
	LCDStoreBgImgToBuff(drawBuff->vocal_cancel.x, drawBuff->vocal_cancel.y, \
			            drawBuff->vocal_cancel.width, drawBuff->vocal_cancel.height, drawBuff->vocal_cancel.p);
	Update_Vocal_Canceler_Icon(vocal_cancel_mode);


	uint8_t SOUND_BUFFER[9216];
    dac_intr.fp = infile;
    dac_intr.buff = SOUND_BUFFER;
    dac_intr.bufferSize = sizeof(SOUND_BUFFER);
    int SoundDMAHalfBlocks = (dac_intr.bufferSize / (sizeof(int16_t) * 2)) / 2;

	int loop_icon_touched = 0, loop_icon_cnt = 0, boost = 0;
	int delay_buffer_filled = 0, DMA_Half_Filled = 0;


	float *fabuf, *fbbuf;
	float *float_buf = (float*)mempool;
	int fbuf_len = dac_intr.bufferSize / 2;

	memset(float_buf, '\0', fbuf_len * sizeof(float));

	/* variables for reverb effect
	 * delay_buffer is allocated in CCM.(64KB)
	 * Maximum length Stereo 16bit(4bytes/sample)
	 * 0.371s@44100Hz 0.341s@48000Hz */
	delay_buffer_typedef delay_buffer;
	delay_buffer.ptr = (uint32_t*)CCM_BASE;
	delay_buffer.size = 65536 / sizeof(uint32_t);
	delay_buffer.idx = 0;

	IIR_Filter_Struct_Typedef IIR;
	IIR.delay_buffer = &delay_buffer;
	IIR.sbuf_size = dac_intr.bufferSize / 2;
	IIR.num_blocks = SoundDMAHalfBlocks;
	IIR.fs = wav.sampleRate;
	IIR.number = bass_boost_mode;
	boost = bass_boost_mode;
	IIR_Set_Params(&IIR);

	REVERB_Struct_Typedef RFX;
	RFX.delay_buffer = &delay_buffer;
	RFX.num_blocks = SoundDMAHalfBlocks;
	RFX.fs = wav.sampleRate;
	RFX.number = reverb_effect_mode;
	REVERB_Set_Prams(&RFX);

	FFT_Struct_Typedef FFT;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 64;
	FFT.samples = dac_intr.bufferSize / ((wav.bitPerSample / 8) * wav.numChannel) / 2;
	if(wav.numChannel < 2){
		FFT.samples >>= 1;
	}

	FFT_Init(&FFT);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* Enable the TIM1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    /* 1s = 1 / F_CPU(168MHz) * (167 + 1) * (99 + 1) * (9999 + 1)  TIM1フレームレート計測用  */
	TIM_TimeBaseInitStructure.TIM_Period = 9999;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1; // 168 - 1
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	int curXprev, cnt = 0;

	music_play.currentTotalSec = 0;
	music_play.comp = 0;
	music_play.update = 1;

	outbuf = (uint8_t*)&dac_intr.buff[0];
	my_fread(outbuf, 1, dac_intr.bufferSize, dac_intr.fp);

//    dac_intr.sound_reads = 0;
	SOUNDInitDAC(wav.sampleRate);
	SOUNDDMAConf((void*)&DAC->DHR12LD, wav.blockSize, (wav.bitPerSample / 8) * wav.numChannel);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	while((infile->seekBytes < infile->fileSize) && LCDStatusStruct.waitExitKey){

		if(loop_icon_touched && TP_PEN_INPUT_BB == Bit_SET){
			if(--loop_icon_cnt <= 0){
				loop_icon_touched = 0;
			}
		}

		if(TP_PEN_INPUT_BB == Bit_RESET && DMA_Half_Filled && !loop_icon_touched){ // Touch pannel tapped?
			curXprev = curX;
			ret = musicPause();
			if(ret >= 30){
				switch(ret){
				case 31:
					IIR.number = bass_boost_mode;
					boost = 1;
					break;
				case 32:
					RFX.number = reverb_effect_mode;
					break;
				default:
					break;
				}
				loop_icon_touched = 1;
				loop_icon_cnt = 10000;
				goto EXIT_TP;
			}

			if(ret != RET_PLAY_NORM){
				goto EXIT_WAV;
			}
			if(curX == curXprev){
				goto SKIP_SEEK;
			}
//			my_fseek(infile, (music_src_p.offset & 0xfffffff0) + sizeof(WAVEFormatStruct), SEEK_SET);
			my_fseek(infile, (music_src_p.offset & 0xfffffff0) + data_offset, SEEK_SET);
			music_play.currentTotalSec = totalSec * (float)infile->seekBytes / (float)media_data_totalBytes;

SKIP_SEEK:
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
			music_play.update = 0;

//			DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
			DMA_Cmd(DMA1_Stream1, ENABLE);
			AUDIO_OUT_ENABLE;
		}
		EXIT_TP:

		if(SOUND_DMA_HALF_TRANS_BB){ // Half
			SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
			outbuf = (uint8_t*)dac_intr.buff;
 			pabuf = (uint32_t*)outbuf;
 			fabuf = (float*)float_buf;
 			fbbuf = (float*)&float_buf[fbuf_len >> 1];
		} else if(SOUND_DMA_FULL_TRANS_BB){	// Full
			SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
			outbuf = (uint8_t*)&dac_intr.buff[dac_intr.bufferSize >> 1];
 			pabuf = (uint32_t*)outbuf;
 			fabuf = (float*)&float_buf[fbuf_len >> 1];
 			fbbuf = (float*)float_buf;
		} else {
 			DMA_Half_Filled = 0;
			continue;
		}
		DMA_Half_Filled = 1;

		if(cnt++ > 10){
			AUDIO_OUT_ENABLE;
		}

		my_fread(outbuf, 1, dac_intr.bufferSize >> 1, dac_intr.fp);

 		/* signed samples to unsigned and store delay buffer */
		for(i = 0;i < SoundDMAHalfBlocks;i++){
			if(vocal_cancel_mode){ // vocal cancel
				pabuf[i] = __PKHBT(__QASX(pabuf[i], pabuf[i]), __QSAX(pabuf[i], pabuf[i]), 0);
			}

			if(settings_group.music_conf.b.prehalve){ // pre halve
				pabuf[i] = __SHADD16(0, pabuf[i]); // LR right shift 1bit
			}

			pabuf[i] ^= 0x80008000; // signed to unsigned

			delay_buffer.ptr[delay_buffer.idx++] = pabuf[i];

			if(delay_buffer.idx >= delay_buffer.size){
				delay_buffer.idx = 0;
				delay_buffer_filled = 1;
			}
 		}
		dac_intr.sound_reads += dac_intr.bufferSize >> 1;

		/* IIR filtering */
		if(delay_buffer_filled && boost){
			IIR_Filter(&IIR, pabuf, fabuf, fbbuf);
		}

		/* Reverb effect */
		if(delay_buffer_filled && reverb_effect_mode){
			REVERB(&RFX, pabuf);
		}

		if(settings_group.music_conf.b.fft){
	 		/* sample audio data for FFT calcuration */
	 		FFT_Sample(&FFT, pabuf);

	 		/* FFT analyzer left */
	 		FFT_Display_Left(&FFT, drawBuff, 0xef7d);

	 		/* FFT analyzer right */
	 		FFT_Display_Right(&FFT, drawBuff, 0xef7d);
		}

		if(music_play.update && music_play.comp == 0){ // update time remain
			music_play.comp = 1;
			remainTotalSec = -abs(music_play.currentTotalSec - totalSec);
			setStrSec(timeStr, remainTotalSec);
			LCDPutBuffToBgImg(drawBuff->timeRemain.x, drawBuff->timeRemain.y, \
		            drawBuff->timeRemain.width, drawBuff->timeRemain.height, drawBuff->timeRemain.p);

			LCDGotoXY(drawBuff->timeRemain.x + 1, drawBuff->timeRemain.y + 1);
			LCDPutString(timeStr, BLACK);
			LCDGotoXY(drawBuff->timeRemain.x, drawBuff->timeRemain.y);
			LCDPutString(timeStr, WHITE);
			continue;
		}

		if(music_play.update && music_play.comp == 1){ // update time elapsed
			music_play.comp = 2;
			setStrSec(timeStr, music_play.currentTotalSec);
			LCDPutBuffToBgImg(drawBuff->timeElapsed.x, drawBuff->timeElapsed.y, \
		            drawBuff->timeElapsed.width, drawBuff->timeElapsed.height, drawBuff->timeElapsed.p);
			LCDGotoXY(drawBuff->timeElapsed.x + 1, drawBuff->timeElapsed.y + 1);
			LCDPutString(timeStr, BLACK);
			LCDGotoXY(drawBuff->timeElapsed.x, drawBuff->timeElapsed.y);
			LCDPutString(timeStr, WHITE);
			continue;
		}

		if(music_play.update && music_play.comp == 2){ // update seek circle
			music_play.update = 0;

			LCDPutBuffToBgImg(prevX, drawBuff->posision.y, \
							  drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);
			curX = (LCD_WIDTH - (33)) * (float)((float)(infile->seekBytes - sizeof(WAVEFormatStruct)) / (float)media_data_totalBytes) + 8;
			LCDStoreBgImgToBuff(curX, drawBuff->posision.y, \
								drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);
			LCDPutIcon(curX, drawBuff->posision.y, \
					   drawBuff->posision.width, drawBuff->posision.height, seek_circle_16x16, seek_circle_16x16_alpha);

			prevX = curX;
			continue;
		}
	}

	if((infile->seekBytes < infile->fileSize) && !LCDStatusStruct.waitExitKey){
		ret = RET_PLAY_STOP;
	} else {
		ret = RET_PLAY_NORM;
	}

	EXIT_WAV:

	AUDIO_OUT_SHUTDOWN;
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, DISABLE);


	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;

	END_WAV:

	/* Disable the TIM1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Disable DMA1_Stream1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	my_fclose(infile);

	LCDStatusStruct.waitExitKey = 0;

	TOUCH_PINIRQ_ENABLE;
	TouchPenIRQ_Enable();
	time.prevTime = time.curTime;
	time.flags.enable = 1;

	return ret;
}


void musicTouch()
{
	static int prevPosX;

	if((touch.posX > (_drawBuff->navigation.x - 5) && touch.posX < (_drawBuff->navigation.x + _drawBuff->navigation.width + 5)) && \
			(touch.posY > (_drawBuff->navigation.y - 5) && touch.posY < (_drawBuff->navigation.y + _drawBuff->navigation.height + 5))){ //
		LCDPutBuffToBgImg(142, 189, 32, 32, _drawBuff->navigation.p);
		LCDPutIcon(142, 189, 32, 32, navigation_pause_patch_32x32, navigation_pause_patch_32x32_alpha);
		music_src_p.done = 1;
		return;
	}

	if((touch.posX > 10 && touch.posX < 50) && (touch.posY > 193 && touch.posY < 235)){ // 停止アイコン
		debug.printf("\r\nplay abort");
		music_src_p.done = -1;
		return;
	}

	if((touch.posX > 180 && touch.posX < 230) && (touch.posY > 190 && touch.posY < 230)){ // 次へアイコン
		debug.printf("\r\nplay abort & next");
		music_src_p.done = -2;
		return;
	}

	if((touch.posX > 90 && touch.posX < 140) && (touch.posY > 190 && touch.posY < 230)){ // 前へアイコン
		debug.printf("\r\nplay abort & prev");
		music_src_p.done = -3;
		return;
	}

	if((touch.posX > (_drawBuff->navigation_loop.x - 15) && touch.posX < (_drawBuff->navigation_loop.x + _drawBuff->navigation_loop.width + 15)) && \
			(touch.posY > (_drawBuff->navigation_loop.y - 10) && touch.posY < (_drawBuff->navigation_loop.y + _drawBuff->navigation_loop.height + 10))){ //

		Update_Navigation_Loop_Icon(navigation_loop_mode = ++navigation_loop_mode % 5);

		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET);
		return;
	}

	if(prevPosX == touch.posX){
		return;
	}

	prevPosX = touch.posX;

	if(touch.posY > (_drawBuff->posision.y + _drawBuff->posision.height))
	{
		return;
	}

	int remainTotalSec;
	char timeStr[20];

	*music_src_p.curX = touch.posX - 20;
	*music_src_p.curX = *music_src_p.curX < (LCD_WIDTH - 20 - 1) ? *music_src_p.curX : (LCD_WIDTH - 20 - 1);
	*music_src_p.curX = *music_src_p.curX > 6 ? *music_src_p.curX : 6;

	music_play.currentTotalSec = (float)((float)(*music_src_p.curX - 6) / (float)(LCD_WIDTH - (30))) * *music_src_p.totalSec;
	music_src_p.offset = (float)((float)(*music_src_p.curX - 6) / (float)(LCD_WIDTH - (30))) * *music_src_p.media_data_totalBytes;

	// position icon
	LCDPutBuffToBgImg(*music_src_p.prevX, _drawBuff->posision.y, \
			_drawBuff->posision.width, _drawBuff->posision.height, music_src_p.drawBuff->posision.p);
	LCDStoreBgImgToBuff(*music_src_p.curX, _drawBuff->posision.y, \
						_drawBuff->posision.width, _drawBuff->posision.height, music_src_p.drawBuff->posision.p);
	LCDPutIcon(*music_src_p.curX, _drawBuff->posision.y, \
			_drawBuff->posision.width, _drawBuff->posision.height, seek_circle_16x16, seek_circle_16x16_alpha);

	*music_src_p.prevX = *music_src_p.curX;

	// time remain
	remainTotalSec = abs(music_play.currentTotalSec - *music_src_p.totalSec) * -1;
	setStrSec(timeStr, remainTotalSec);

	LCDPutBuffToBgImg(music_src_p.drawBuff->timeRemain.x, music_src_p.drawBuff->timeRemain.y, \
			music_src_p.drawBuff->timeRemain.width, music_src_p.drawBuff->timeRemain.height, \
			music_src_p.drawBuff->timeRemain.p);

	LCDGotoXY(music_src_p.drawBuff->timeRemain.x + 1, music_src_p.drawBuff->timeRemain.y + 1);
	LCDPutString(timeStr, BLACK);
	LCDGotoXY(music_src_p.drawBuff->timeRemain.x, music_src_p.drawBuff->timeRemain.y);
	LCDPutString(timeStr, WHITE);

	// time elapsed
	setStrSec(timeStr, music_play.currentTotalSec);

	LCDPutBuffToBgImg(music_src_p.drawBuff->timeElapsed.x, music_src_p.drawBuff->timeElapsed.y, \
			music_src_p.drawBuff->timeElapsed.width, music_src_p.drawBuff->timeElapsed.height, \
			music_src_p.drawBuff->timeElapsed.p);

	LCDGotoXY(music_src_p.drawBuff->timeElapsed.x + 1, music_src_p.drawBuff->timeElapsed.y + 1);
	LCDPutString(timeStr, BLACK);
	LCDGotoXY(music_src_p.drawBuff->timeElapsed.x, music_src_p.drawBuff->timeElapsed.y);
	LCDPutString(timeStr, WHITE);

	return;
}

int musicPause()
{
	int ret = RET_PLAY_NORM;
    touch.cnt = 0, touch.aveX = 0, touch.aveY = 0;

    extern settings_group_typedef settings_group;
    extern settings_item_typedef settings_item_fft_bar_type;
    extern settings_item_typedef settings_item_fft_color_type;

    while(touch.cnt++ < 600){
    	touch.adX = GET_X_AXIS();
    	touch.adY = GET_Y_AXIS();

    	touch.aveX += ((int)(touch.adX - touch.cal->x[2]) * 10) / touch.cal->xStep + 15;
    	touch.aveY += ((int)(touch.adY - touch.cal->y[0]) * 10) / touch.cal->yStep + 15;
    }

	touch.posX = touch.aveX / 600;
	touch.posY = touch.aveY / 600;

	if(touch.posX < 0) touch.posX = 0;
	if(touch.posX > 319) touch.posX = 319;
	if(touch.posY < 0) touch.posY = 0;
	if(touch.posY > 239) touch.posY = 239;

	if((touch.posX > (_drawBuff->navigation_loop.x - 10) && touch.posX < (_drawBuff->navigation_loop.x + _drawBuff->navigation_loop.width + 10)) && \
			(touch.posY > (_drawBuff->navigation_loop.y - 10) && touch.posY < (_drawBuff->navigation_loop.y + _drawBuff->navigation_loop.height + 10))){ //

		Update_Navigation_Loop_Icon(navigation_loop_mode = ++navigation_loop_mode % 5);

		return 30;
	}

	if((touch.posX > (_drawBuff->bass_boost.x - 10) && touch.posX < (_drawBuff->bass_boost.x + _drawBuff->bass_boost.width + 10)) && \
			(touch.posY > (_drawBuff->bass_boost.y - 10) && touch.posY < (_drawBuff->bass_boost.y + _drawBuff->bass_boost.height + 10))){ //

		Update_Bass_Boost_Icon(bass_boost_mode = ++bass_boost_mode % 4);

		return 31;
	}

	if((touch.posX > (_drawBuff->reverb_effect.x - 10) && touch.posX < (_drawBuff->reverb_effect.x + _drawBuff->reverb_effect.width + 10)) && \
			(touch.posY > (_drawBuff->reverb_effect.y - 10) && touch.posY < (_drawBuff->reverb_effect.y + _drawBuff->reverb_effect.height + 10))){ //

		Update_Reverb_Effect_Icon(reverb_effect_mode = ++reverb_effect_mode % 4);

		return 32;
	}

	if((touch.posX > (_drawBuff->vocal_cancel.x - 5) && touch.posX < (_drawBuff->vocal_cancel.x + _drawBuff->vocal_cancel.width + 5)) && \
			(touch.posY > (_drawBuff->vocal_cancel.y - 10) && touch.posY < (_drawBuff->vocal_cancel.y + _drawBuff->vocal_cancel.height + 10))){ //

		Update_Vocal_Canceler_Icon(vocal_cancel_mode = ++vocal_cancel_mode % 2);

		return 33;
	}

	if((touch.posX > (_drawBuff->fft_analyzer_left.x - 5) && touch.posX < (_drawBuff->fft_analyzer_left.x + _drawBuff->fft_analyzer_left.width + 5)) && \
			(touch.posY > (_drawBuff->fft_analyzer_left.y - 10) && touch.posY < (_drawBuff->fft_analyzer_left.y + _drawBuff->fft_analyzer_left.height + 10))){ //

		if(++settings_group.music_conf.b.fft_bar_color_idx >= settings_item_fft_color_type.item_count){
			settings_group.music_conf.b.fft_bar_color_idx = 0;
		}

		return 34;
	}

	if((touch.posX > (_drawBuff->fft_analyzer_right.x + 5) && touch.posX < (_drawBuff->fft_analyzer_right.x + _drawBuff->fft_analyzer_right.width + 5)) && \
			(touch.posY > (_drawBuff->fft_analyzer_right.y - 10) && touch.posY < (_drawBuff->fft_analyzer_right.y + _drawBuff->fft_analyzer_right.height + 10))){ //

		if(++settings_group.music_conf.b.fft_bar_type >= settings_item_fft_bar_type.item_count){
			settings_group.music_conf.b.fft_bar_type = 0;
		}

		return 35;
	}


	if(touch.posY < (_drawBuff->posision.y - 10)){
		return 30;
	}

	// clear FFT analyzer left
	LCDPutBuffToBgImg(_drawBuff->fft_analyzer_left.x, _drawBuff->fft_analyzer_left.y, \
			_drawBuff->fft_analyzer_left.width, _drawBuff->fft_analyzer_left.height, _drawBuff->fft_analyzer_left.p);

	// clear FFT analyzer right
	LCDPutBuffToBgImg(_drawBuff->fft_analyzer_right.x, _drawBuff->fft_analyzer_right.y, \
			_drawBuff->fft_analyzer_right.width, _drawBuff->fft_analyzer_right.height, _drawBuff->fft_analyzer_right.p);


	if((touch.posX > 10 && touch.posX < 50) && (touch.posY > 193 && touch.posY < 235)){ // 停止アイコン
		debug.printf("\r\nplay abort");
		return RET_PLAY_STOP;
	}

	if((touch.posX > 180 && touch.posX < 230) && (touch.posY > 190 && touch.posY < 230)){ // 次へアイコン
		debug.printf("\r\nplay abort & next");
		return RET_PLAY_NEXT;
	}

	if((touch.posX > 90 && touch.posX < 140) && (touch.posY > 190 && touch.posY < 230)){ // 前へアイコン
		debug.printf("\r\nplay abort & prev");
		return RET_PLAY_PREV;
	}

	AUDIO_OUT_SHUTDOWN;
	DMA_Cmd(DMA1_Stream1, DISABLE);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);

	debug.printf("\r\npaused");
	LCDPutBuffToBgImg(_drawBuff->navigation.x, _drawBuff->navigation.y, \
			          _drawBuff->navigation.width, _drawBuff->navigation.height, _drawBuff->navigation.p);
	LCDPutIcon(_drawBuff->navigation.x, _drawBuff->navigation.y, \
			   _drawBuff->navigation.width, _drawBuff->navigation.height, \
			   navigation_playing_patch_32x32, navigation_playing_patch_32x32_alpha);

	touch.func = musicTouch;
	music_src_p.done = 0;
	music_src_p.offset = 0;

	TOUCH_PINIRQ_ENABLE;
	TouchPenIRQ_Enable();
	while(music_src_p.done == 0){};
	TouchPenIRQ_Disable();
	TOUCH_PINIRQ_DISABLE;

	touch.func = touch_empty_func;

	if(music_src_p.done != 1){
		return music_src_p.done;
	}

	return ret;
}

void make_shuffle_table(unsigned int seed){
	  int i, j;
	  uint8_t rnd;
	  static uint8_t rand_table[256];

	  srand(seed);

	  if(!shuffle_play.flag_make_rand){
		  shuffle_play.pRandIdEntry = rand_table;
		  shuffle_play.flag_make_rand = 1;
	  }

	  shuffle_play.pRandIdEntry[1] = rand() % (fat.fileCnt - 1) + 1;

	  for(j = 2;j <= (fat.fileCnt - 1);j++){
	  LOOP:
	    rnd = rand() % (fat.fileCnt - 1) + 1;
	    for(i = 1;i < j;i++){
	      if(shuffle_play.pRandIdEntry[i] == rnd){
	        goto LOOP;
	      }
	    }
	    shuffle_play.pRandIdEntry[j] = rnd;
	  }
}

int PlayMusic(int id){
	int idNext, idCopy = id;
	unsigned int seed;
	static uint8_t pre_navigation_loop_mode = 0;

	if(navigation_loop_mode == NAV_SHUFFLE_PLAY){
		if(!shuffle_play.mode_changed){
			idNext = id;
			if(++idNext >= fat.fileCnt){
				idNext = 1;
			}
			seed = TIM8->CNT;
			do{
				make_shuffle_table(seed);
				seed += 100;
			}while((id == shuffle_play.pRandIdEntry[idNext]) && (fat.fileCnt > 2));
		}
		if(shuffle_play.play_continuous){
			id = shuffle_play.pRandIdEntry[id];
		}
		shuffle_play.mode_changed = 1;
	}
	shuffle_play.play_continuous = 1;

	if((pre_navigation_loop_mode == NAV_SHUFFLE_PLAY) && (navigation_loop_mode != NAV_SHUFFLE_PLAY)){
		id = idCopy;
	}
	pre_navigation_loop_mode = navigation_loop_mode;

	uint16_t entryPointOffset = getListEntryPoint(id);

	if(fbuf[entryPointOffset + 8] != 0x20){
		char fileTypeStr[4];
		memset(fileTypeStr, '\0', sizeof(fileTypeStr));
		strncpy(fileTypeStr, (char*)&fbuf[entryPointOffset + 8], 3);
		if(strcmp(fileTypeStr, "MP4") == 0 || \
		   strcmp(fileTypeStr, "M4A") == 0 || \
		   strcmp(fileTypeStr, "M4P") == 0)
		{
			return PlayAAC(id);
		} else if(strcmp(fileTypeStr, "MP3") == 0){
			return PlayMP3(id);
		} else if(strcmp(fileTypeStr, "WAV") == 0){
			return PlaySound(id);
		} else if(strcmp(fileTypeStr, "MOV") == 0){
			return PlayMotionJpeg(id);
		}
	}

	id = idCopy;

	if((navigation_loop_mode == NAV_INFINITE_PLAY_ENTIRE) || (navigation_loop_mode == NAV_SHUFFLE_PLAY) || id <= fat.fileCnt){
		LCDStatusStruct.waitExitKey = 1;
		return RET_PLAY_NEXT;
	} else {
		LCDStatusStruct.waitExitKey = 0;
		return RET_PLAY_INVALID;
	}
}
