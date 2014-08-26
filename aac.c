/*
 * aac.c
 *
 *  Created on: 2012/03/27
 *      Author: Tonsuke
 */


#include "aac.h"
#include "pcf_font.h"


/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: main.c,v 1.4 2005/07/05 21:08:13 ehyche Exp $
 *
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com)
 * February 2005
 *
 * main.c - sample command-line test wrapper for decoder
 **************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "aacdec.h"
#include "aaccommon.h"

#include "fat.h"
#include "usart.h"
#include "sound.h"
#include "mjpeg.h"
#include "lcd.h"
#include "icon.h"
//#include "bgimage.h"
#include "xpt2046.h"
#include "board_config.h"

#include "settings.h"

#include "arm_math.h"
#include "fft.h"
#include "fx.h"

#include <ctype.h>		/* to declare isprint() */


#define READBUF_SIZE	(2 * AAC_MAINBUF_SIZE * AAC_MAX_NCHANS)	/* pick something big enough to hold a bunch of frames */


#ifdef AAC_ENABLE_SBR
#define SBR_MUL		2
#else
#define SBR_MUL		1
#endif

#define TAG_MAX_CNT 100

typedef struct {
	int numEntry;
	MY_FILE fp_stco, fp;
}aac_stco_Typedef;

aac_stco_Typedef *_aac_stco_struct;

uint8_t *_nameTag, *_artistTag, *_albumTag;
media_info_typedef *_media_info;
MY_FILE *_file_covr;

static inline uint32_t getAtomSize(void* atom){
/*
	ret = *(uint8_t*)(atom) << 24;
	ret |= *(uint8_t*)(atom + 1) << 16;
	ret |= *(uint8_t*)(atom + 2) << 8;
	ret |= *(uint8_t*)(atom + 3);
*/
	return __REV(*(uint32_t*)atom);
}


int collectMediaData(MY_FILE *fp, uint32_t parentAtomSize, uint32_t child)
{
	uint8_t atombuf[512];
	int atomSize, totalAtomSize = 0, index, is, contentLength;
	volatile MY_FILE fp_tmp;

	memset(atombuf, '\0', sizeof(atombuf));

	do{
		my_fread(atombuf, 1, 8, fp);
		atomSize = getAtomSize(atombuf);

		for(index = 0;index < ATOM_ITEMS;index++){
			if(!strncmp((char*)&atombuf[4], (char*)&atomTypeString[index][0], 4)){
				debug.printf("\r\n");
				for(is = child;is > 0;is--) debug.printf(" ");
				debug.printf("%s %d", (char*)&atomTypeString[index][0], atomSize);
				break;
			}
		}

		if(index >= ATOM_ITEMS){
			debug.printf("\r\nunrecognized atom:%s %d", (char*)&atombuf[4], atomSize);
			goto NEXT;
		}

		memcpy((void*)&fp_tmp, (void*)fp, sizeof(MY_FILE));

		switch(index){
		case COVR: // art work image
			my_fseek(fp, 16, SEEK_CUR);
			memcpy((void*)_file_covr, (void*)fp, sizeof(MY_FILE));
			break;
		case CNAM:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_nameTag, 1, contentLength, fp);
			_nameTag[contentLength] = '\0';
			_nameTag[contentLength + 1] = '\0';
			break;
		case CART:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_artistTag, 1, contentLength, fp);
			_artistTag[contentLength] = '\0';
			_artistTag[contentLength + 1] = '\0';
			break;
		case CALB:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_albumTag, 1, contentLength, fp);
			_albumTag[contentLength] = '\0';
			_albumTag[contentLength + 1] = '\0';
			break;
		case META:
			my_fseek((MY_FILE*)&fp_tmp, 4, SEEK_CUR);
			totalAtomSize -= 4;
			atomSize -= 4;
			break;
		case MDHD:
			my_fseek(fp, 12, SEEK_CUR); // skip ver/flag  creationtime modificationtime
			my_fread(atombuf, 1, 4, fp); // time scale
			_media_info->sound.timeScale = getAtomSize(atombuf);

			my_fread(atombuf, 1, 4, fp); // duration
			_media_info->sound.duration = getAtomSize(atombuf);
			break;
		case STSD:
			my_fseek(fp, 8, SEEK_CUR); // skip Reserved(6bytes)/Data Reference Index
			my_fread(atombuf, 1, 4, fp); // next atom size
			my_fread(atombuf, 1, sizeof(sound_format), fp);
			memcpy((void*)&_media_info->format, (void*)atombuf, sizeof(sound_format));
			_media_info->format.version = (uint16_t)b2l((void*)&_media_info->format.version, sizeof(uint16_t));
			_media_info->format.revision = (uint16_t)b2l((void*)&_media_info->format.revision, sizeof(uint16_t));
			_media_info->format.vendor = (uint16_t)b2l((void*)&_media_info->format.vendor, sizeof(uint16_t));
			_media_info->format.numChannel = (uint16_t)b2l((void*)&_media_info->format.numChannel, sizeof(uint16_t));
			_media_info->format.sampleSize = (uint16_t)b2l((void*)&_media_info->format.sampleSize, sizeof(uint16_t));
			_media_info->format.complesionID = (uint16_t)b2l((void*)&_media_info->format.complesionID, sizeof(uint16_t));
			_media_info->format.packetSize = (uint16_t)b2l((void*)&_media_info->format.packetSize, sizeof(uint16_t));
			_media_info->format.sampleRate = (uint16_t)b2l((void*)&_media_info->format.sampleRate, sizeof(uint16_t));
			my_fread(atombuf, 1, 4, fp); // etds atom size
			uint32_t etds_size = (uint32_t)b2l((void*)&atombuf[0], sizeof(uint32_t));
			my_fread(atombuf, 1, 512, fp);

			_media_info->bitrate.maxBitrate = 0;
			_media_info->bitrate.avgBitrate = 0;

			int i;
			for(i = 0;i < etds_size - 4 - 1;i++){
				if(atombuf[i] == 0x40 && atombuf[i + 1] == 0x15){
					_media_info->bitrate.maxBitrate = (uint32_t)b2l((void*)&atombuf[i + 5], sizeof(uint32_t));
					_media_info->bitrate.avgBitrate = (uint32_t)b2l((void*)&atombuf[i + 9], sizeof(uint32_t));
					break;
				}
			}
			break;
		case STCO:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			my_fread(atombuf, 1, 4, fp); // numEntry
			_aac_stco_struct->numEntry = getAtomSize(atombuf);
			memcpy((void*)&_aac_stco_struct->fp_stco, (void*)fp, sizeof(MY_FILE));
			break;
		default:
			break;
		}

		memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));

		if(atomHasChild[index]){
			if(collectMediaData(fp, atomSize - 8, child + 1) != 0){ // Re entrant
				return -1;
			}
			memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));
		}

NEXT:
		my_fseek(fp, atomSize - 8, SEEK_CUR);
		totalAtomSize += atomSize;
//		debug.printf("\r\n***parentAtomSize:%d totalAtomSize:%d", parentAtomSize, totalAtomSize);
	}while(parentAtomSize > (totalAtomSize + 8));

	return 0;
}



static int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, MY_FILE *infile)
{
	int nRead;

	/* move last, small chunk from end of buffer to start, then fill with new data */
	memmove(readBuf, readPtr, bytesLeft);
	nRead = my_fread(readBuf + bytesLeft, 1, bufSize - bytesLeft, infile);

	/* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
	if (nRead < bufSize - bytesLeft)
		memset(readBuf + bytesLeft + nRead, 0, bufSize - bytesLeft - nRead);

	return nRead;
}


int PlayAAC(int id)
{
	time.flags.enable = 0;
	TouchPenIRQ_Disable();
	TOUCH_PINIRQ_DISABLE;
	touch.func = touch_empty_func;

	int i;
	int totalSec, remainTotalSec, mdatOffset, media_data_totalBytes;
	int bytesLeft, nRead, err, eofReached;
	int curX = 0, prevX = 0, ret = 0;
	uint32_t *pabuf;
	unsigned char *readPtr, readBuf[READBUF_SIZE];
//	static short outbuf[AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL];
	short *outbuf;
	void *putCharTmp = '\0', *putWideCharTmp = '\0';
//	drawBuff_typedef *drawBuff = (drawBuff_typedef*)jFrameMem;
	drawBuff_typedef dbuf, *drawBuff;
	drawBuff = &dbuf;
	_drawBuff = drawBuff;
	MY_FILE *infile = '\0', infilecp;
	HAACDecoder *hAACDecoder;
	AACFrameInfo aacFrameInfo;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	char timeStr[16];
	uint8_t nameTag[TAG_MAX_CNT], artistTag[TAG_MAX_CNT], albumTag[TAG_MAX_CNT];

	memset(nameTag, 0, sizeof(nameTag));
	memset(artistTag, 0, sizeof(artistTag));
	memset(albumTag, 0, sizeof(albumTag));

	_nameTag = nameTag, _artistTag = artistTag, _albumTag = albumTag;

	media_info_typedef media_info;
	_media_info = &media_info;

	uint8_t aac_stco_buf[4];
	aac_stco_Typedef aac_stco_struct;
	_aac_stco_struct = &aac_stco_struct;

	MY_FILE file_covr;

	file_covr.clusterOrg = 0;
	_file_covr = &file_covr;

	music_src_p.curX = &curX;
	music_src_p.prevX = &prevX;
	music_src_p.media_data_totalBytes = &media_data_totalBytes;
	music_src_p.totalSec = &totalSec;
	music_src_p.drawBuff = drawBuff;
	music_src_p.fp = infile;

	LCDPutBgImgMusic();

	/* open input file */
	infile = my_fopen(id);
	if(infile == '\0'){
		LCDStatusStruct.waitExitKey = 0;
		return -1;
	}

	memcpy((void*)&infilecp, (void*)infile, sizeof(MY_FILE));

	debug.printf("\r\n\n*** AAC Decode ***");

	hAACDecoder = (HAACDecoder *)AACInitDecoder();

	if (!hAACDecoder) {
		debug.printf(" *** Error initializing decoder ***\n");
		my_fclose(infile);
		LCDStatusStruct.waitExitKey = 0;
		return -1;
	}

	nameTag[0] = artistTag[0] = albumTag[0] = '\0';

	uint8_t atombuf[8];
	my_fread(atombuf, 1, 8, infile);
	mdatOffset = 0;
	if(strncmp((char*)&atombuf[4], "ftyp", 4) == 0){
		my_fseek(infile, 0, SEEK_SET);
		collectMediaData(infile, infile->fileSize, 0); // チャネル数、サンプルレート、デュレーション、メタデータ等を取得する

		memcpy((void*)infile, (void*)&infilecp, sizeof(MY_FILE));

		memcpy((void*)&_aac_stco_struct->fp, (void*)&_aac_stco_struct->fp_stco, sizeof(MY_FILE));
//		my_fread(aac_stco_buf, 4, &aac_stco_struct.fp);
//		my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);

//		do{
			my_fread(aac_stco_buf, 1, 4, (MY_FILE*)&_aac_stco_struct->fp);
			my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);
//			my_fread(aac_stco_buf, 1, 1, infile);
//		}while(aac_stco_buf[0] != 0x20 && aac_stco_buf[0] != 0x21);
//		my_fseek(infile, -1, SEEK_CUR);

		debug.printf("\r\n\ntimeScale:%d", media_info.sound.timeScale);
		debug.printf("\r\nduration:%d", media_info.sound.duration);
//		debug.printf("\r\n\nmedia_info.format.version:%d",media_info.format.version);
//		debug.printf("\r\nmedia_info.format.revision:%d", media_info.format.revision);
//		debug.printf("\r\nmedia_info.format.vendor:%d", media_info.format.vendor);
		debug.printf("\r\nnumChannel:%d", media_info.format.numChannel);
		debug.printf("\r\nsampleSize:%d", media_info.format.sampleSize);
//		debug.printf("\r\nmedia_info.format.complesionID:%d", media_info.format.complesionID);
//		debug.printf("\r\nmedia_info.format.packetSize:%d", media_info.format.packetSize);
		debug.printf("\r\nsampleRate:%d", media_info.format.sampleRate);

		totalSec = (int)((float)media_info.sound.duration / (float)media_info.sound.timeScale + 0.5);
		mdatOffset = infile->seekBytes;
		media_data_totalBytes = infile->fileSize - mdatOffset;

		aacFrameInfo.nChans = media_info.format.numChannel;
//		aacFrameInfo.sampRateCore = media_info.format.sampleRate;
		aacFrameInfo.sampRateCore = media_info.sound.timeScale;
		aacFrameInfo.profile = 0;
		AACSetRawBlockParams(hAACDecoder, 0, &aacFrameInfo);
	} else {
		debug.printf("\r\nnot encapseled data");
		my_fseek(infile, 0, SEEK_SET);
	}


	uint16_t xTag = 110, yTag = 67, disp_limit = 300, strLen, yPos;
	if(!albumTag[0] && !artistTag[0]){
		yTag += 20;
	} else if(!albumTag[0] || !artistTag[0]){
		yTag += 10;
	}

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


	if(nameTag[0] != 0){
		strLen = LCDGetStringUTF8PixelLength(nameTag, 16);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = LCD_WIDTH - 1;
		} else {
			disp_limit = LCD_WIDTH - 20;
			yTag -= 8;
		}

		strLen = LCDGetStringUTF8PixelLength(albumTag, 12);
		if((xTag + strLen) > (LCD_WIDTH - 20)){
			yTag -= 6;
		}
		LCDGotoXY(xTag + 1, yTag + 1);
		LCDPutStringUTF8(xTag + 1, disp_limit, 2, nameTag, BLACK);
		LCDGotoXY(xTag, yTag);
		yPos = LCDPutStringUTF8(xTag, disp_limit - 1, 2, nameTag, WHITE);
		disp_limit = 288;
		yTag += 20 + yPos;
	} else {
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
		yTag += 20;
	}

	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;
	disp_limit = 300;

	if(albumTag[0] != 0){
		LCDGotoXY(xTag + 1, yTag + 1);
		LCDPutStringUTF8(xTag + 1, LCD_WIDTH - 20, 2, albumTag, BLACK);
		LCDGotoXY(xTag, yTag);
		yPos = LCDPutStringUTF8(xTag, LCD_WIDTH - 21, 2, albumTag, WHITE);
		yTag += 20 + yPos;
	}
	if(artistTag[0] != 0){
		LCDGotoXY(xTag + 1, yTag + 1);
		LCDPutStringUTF8(xTag + 1, disp_limit, 1, artistTag, BLACK);
		LCDGotoXY(xTag, yTag);
		LCDPutStringUTF8(xTag, disp_limit - 1, 1, artistTag, WHITE);
	}

	dispArtWork(&file_covr);

	LCDPutIcon(0, 155, 320, 80, music_underbar_320x80, music_underbar_320x80_alpha);

	char s[20];
	SPRINTF((char*)s, "%d/%d", id, fat.fileCnt - 1);
	LCDGotoXY(21, MUSIC_INFO_POS_Y + 1);
	LCDPutString((char*)s, BLACK);
	LCDGotoXY(20, MUSIC_INFO_POS_Y);
	LCDPutString((char*)s, WHITE);

	if(settings_group.music_conf.b.musicinfo){
		LCDGotoXY(71, MUSIC_INFO_POS_Y + 1);
		LCDPutString("AAC", BLACK);
		LCDGotoXY(70, MUSIC_INFO_POS_Y);
		LCDPutString("AAC", WHITE);

		LCDGotoXY(111, MUSIC_INFO_POS_Y + 1);
		LCDPutString(media_info.format.numChannel == 2 ? "Stereo" : "Mono", BLACK);
		LCDGotoXY(110, MUSIC_INFO_POS_Y);
		LCDPutString(media_info.format.numChannel == 2 ? "Stereo" : "Mono", WHITE);

		if(media_info.bitrate.avgBitrate){
			SPRINTF(s, "%dkbps", (int)(media_info.bitrate.avgBitrate / 1000));
		} else {
			SPRINTF(s, "---kbps");
		}
		LCDGotoXY(171, MUSIC_INFO_POS_Y + 1);
		LCDPutString(s, BLACK);
		LCDGotoXY(170, MUSIC_INFO_POS_Y);
		LCDPutString(s, WHITE);


		SPRINTF(s, "%dHz", aacFrameInfo.sampRateCore);
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

	Update_Navigation_Loop_Icon(navigation_loop_mode);

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

	bytesLeft = 0;
	eofReached = 0;
	readPtr = readBuf;
	err = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the TIM1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
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

	uint8_t SOUND_BUFFER[8192];
	dac_intr.buff = SOUND_BUFFER;
    dac_intr.bufferSize = (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL) * sizeof(int16_t) * aacFrameInfo.nChans;
    int SoundDMAHalfBlocks = dac_intr.bufferSize / 2 / (sizeof(uint16_t) * 2);

	float *fabuf, *fbbuf;
	float *float_buf = (float*)mempool;
	int fbuf_len = (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL) * aacFrameInfo.nChans;

	memset(float_buf, '\0', fbuf_len * sizeof(float));

//	int prevWaitExitKey = LCDStatusStruct.waitExitKey;
	int stcoEntry, curXprev, cnt = 0;
	int loop_icon_touched = 0, loop_icon_cnt = 0, boost = 0;
	int delay_buffer_filled = 0, DMA_Half_Filled = 0;


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
	IIR.sbuf_size = (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL) * aacFrameInfo.nChans;
	IIR.num_blocks = SoundDMAHalfBlocks;
	IIR.fs = aacFrameInfo.sampRateCore;
	IIR.number = bass_boost_mode;
	boost = bass_boost_mode;
	IIR_Set_Params(&IIR);

	REVERB_Struct_Typedef RFX;
	RFX.delay_buffer = &delay_buffer;
	RFX.num_blocks = SoundDMAHalfBlocks;
	RFX.fs = aacFrameInfo.sampRateCore;
	RFX.number = reverb_effect_mode;
	REVERB_Set_Prams(&RFX);

	FFT_Struct_Typedef FFT;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 64;
	FFT.samples = dac_intr.bufferSize / (sizeof(int16_t) * aacFrameInfo.nChans) / 2;
	if(aacFrameInfo.nChans < 2){
		FFT.samples >>= 1;
	}
	FFT_Init(&FFT);

	SOUNDInitDAC(aacFrameInfo.sampRateCore);
	SOUNDDMAConf((void*)&DAC->DHR12LD, sizeof(int16_t) * aacFrameInfo.nChans, sizeof(int16_t) * aacFrameInfo.nChans);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);

	music_play.currentTotalSec = 0;
	music_play.comp = 0;
	music_play.update = 1;

	DMA_Cmd(DMA1_Stream1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	while(!eofReached && LCDStatusStruct.waitExitKey){

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
				goto END_AAC;
			}
			if(curX == curXprev){
				goto SKIP_SEEK;
			}

			stcoEntry = _aac_stco_struct->numEntry * (float)music_src_p.offset / (float)media_data_totalBytes;
			memcpy((void*)&_aac_stco_struct->fp, (void*)&_aac_stco_struct->fp_stco, sizeof(MY_FILE));
			my_fseek((MY_FILE*)&_aac_stco_struct->fp, stcoEntry * 4, SEEK_CUR);
//			my_fread(aac_stco_buf, 4, &aac_stco_struct.fp);
//			my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);
			do{
				my_fread(aac_stco_buf, 1, 4, (MY_FILE*)&_aac_stco_struct->fp);
				my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);
				my_fread(aac_stco_buf, 1, 4, infile);
			}while(aac_stco_buf[0] != 0x20 && aac_stco_buf[0] != 0x21);
			my_fseek(infile, -4, SEEK_CUR);

			debug.printf("\r\naac_stco_struct.numEntry:%d", _aac_stco_struct->numEntry);
			debug.printf("\r\nstcoEntry:%d", stcoEntry);

			bytesLeft = 0;
			eofReached = 0;
			readPtr = readBuf;
			err = 0;

			music_play.currentTotalSec = totalSec * (float)((float)(infile->seekBytes - mdatOffset) / (float)media_data_totalBytes);

SKIP_SEEK:
			cnt = 0;
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
			music_play.update = 0;

			DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
			DMA_Cmd(DMA1_Stream1, ENABLE);
		}
		EXIT_TP:


		if(SOUND_DMA_HALF_TRANS_BB){ // Half
 			SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
 			outbuf = (short*)dac_intr.buff;
 			pabuf = (uint32_t*)outbuf;
 			fabuf = (float*)float_buf;
 			fbbuf = (float*)&float_buf[fbuf_len >> 1];
 		} else if(SOUND_DMA_FULL_TRANS_BB){ // Full
 			SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
 			outbuf = (short*)&dac_intr.buff[dac_intr.bufferSize >> 1];
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

		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < AAC_MAX_NCHANS * AAC_MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = readBuf;
			if (nRead == 0){
				eofReached = 1;
				debug.printf("\r\neofReached");
				break;
			}
		}

		/* decode one AAC frame */
 		err = AACDecode(hAACDecoder, &readPtr, &bytesLeft, outbuf);

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

		if (err) {
			// error occurred
			switch (err) {
			case ERR_AAC_INDATA_UNDERFLOW:
				debug.printf("\r\nERR_AAC_INDATA_UNDERFLOW");
				// need to provide more data on next call to AACDecode() (if possible)
				if (eofReached || bytesLeft == READBUF_SIZE){
					debug.printf("\r\noutOfData");
				}
				break;
			default:
				break;
			}
			break;
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
			curX = (LCD_WIDTH - (33)) * (float)((float)(infile->seekBytes - mdatOffset) / (float)media_data_totalBytes) + 8;
			LCDStoreBgImgToBuff(curX, drawBuff->posision.y, \
								drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);
			LCDPutIcon(curX, drawBuff->posision.y, \
					   drawBuff->posision.width, drawBuff->posision.height, seek_circle_16x16, seek_circle_16x16_alpha);

			prevX = curX;
			continue;
		}

		/*
		if(TIM_GetITStatus(TIM1, TIM_IT_Update)){
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
			music_play.currentTotalSec++;
			music_play.update = 1;
			music_play.comp = 0;
			// no error
			AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);

			debug.printf("\r\n\nbitRate:%d", aacFrameInfo.bitRate);
			debug.printf("\r\nnChans:%d", aacFrameInfo.nChans);
			debug.printf("\r\nsampRateCore:%d", aacFrameInfo.sampRateCore);
			debug.printf("\r\nbitsPerSample:%d", aacFrameInfo.bitsPerSample);
			debug.printf("\r\noutputSamps:%d", aacFrameInfo.outputSamps);
			debug.printf("\r\nprofile:%d", aacFrameInfo.profile);
			debug.printf("\r\ntnsUsed:%d", aacFrameInfo.tnsUsed);
			debug.printf("\r\npnsUsed:%d", aacFrameInfo.pnsUsed);
		} */
	}

	if(!eofReached && !LCDStatusStruct.waitExitKey){
		ret = RET_PLAY_STOP;
	} else {
		ret = RET_PLAY_NORM;
	}

	if (err != ERR_AAC_NONE && err != ERR_AAC_INDATA_UNDERFLOW){
		debug.printf("\r\nError - %d", err);
	}
END_AAC:
	AUDIO_OUT_SHUTDOWN;
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, DISABLE);

	AACFreeDecoder(hAACDecoder);

	/* Disable the TIM1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	LCDStatusStruct.waitExitKey = 0;

	TIM_Cmd(TIM1, DISABLE);

	/* close files */
	my_fclose(infile);

	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;

	TOUCH_PINIRQ_ENABLE;
	TouchPenIRQ_Enable();
	time.prevTime = time.curTime;
	time.flags.enable = 1;
	return ret;
}

