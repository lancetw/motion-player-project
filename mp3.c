/*
 * mp3.c
 *
 *  Created on: 2012/03/25
 *      Author: Tonsuke
 */


/* ***** BEGIN LICENSE BLOCK *****
 * Version: RCSL 1.0/RPSL 1.0
 *
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file, are
 * subject to the current version of the RealNetworks Public Source License
 * Version 1.0 (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the RealNetworks Community Source License Version 1.0
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl,
 * in which case the RCSL will apply. You may also obtain the license terms
 * directly from RealNetworks.  You may not use this file except in
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks
 * applicable to this file, the RCSL.  Please see the applicable RPSL or
 * RCSL for the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the portions
 * it created.
 *
 * This file, and the files included with this file, is distributed and made
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point MP3 decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * June 2003
 *
 * main.c - command-line test app that uses C interface to MP3 decoder
 **************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mp3dec.h"
#include "mp3common.h"
#include "mp3.h"
#include "fat.h"
#include "lcd.h"
#include "pcf_font.h"
#include "sound.h"
#include "usart.h"
#include "icon.h"
#include "xpt2046.h"

#include "settings.h"

#include "board_config.h"

#include "mjpeg.h"

#include "mpool.h"
#include "arm_math.h"
#include "fft.h"

#include "fx.h"


#define READBUF_SIZE		(1024 * 10)	/* feel free to change this, but keep big enough for >= one frame at high bitrates */
#define MAX_ARM_FRAMES		100
#define ARMULATE_MUL_FACT	1

typedef struct {
	char head_char[4];
	uint32_t flags;
	uint32_t frames;
}xing_struct_typedef;

typedef struct {
	char id_str[3];
	uint8_t verU16[2]; // must cast to (uint16_t*)
	uint8_t flag;
	uint8_t p_size[4];
}id3tag_head_struct_typedef;

typedef struct {
	char frameID_str[4];
	uint8_t p_size[4];
	uint8_t flagsU16[2];
}id3v34tag_frame_struct_typedef;

typedef struct {
	char frameID_str[3];
	uint8_t p_size[3];
}id3v2tag_frame_struct_typedef;

typedef struct{
	uint8_t encType;
	uint8_t str_p[100];
	uint16_t flags;
	uint8_t bom[2];
	size_t size;
}id3_nameTag_struct_typedef;

id3_nameTag_struct_typedef *_id3_title, *_id3_album, *_id3_artist;

MY_FILE *_file_artwork;

uint32_t unSynchSize(void *p_size){
	uint32_t frameSize;

	frameSize  = ((uint32_t)*(uint8_t*)p_size << 21);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 1) << 14);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 2) << 7);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 3));

	return frameSize;
}

static int ID3_2v34(MY_FILE *fp, uint32_t entireTagSize, uint8_t ver)
{
	uint32_t frameSize;
	int tagPoint;
	id3v34tag_frame_struct_typedef id3tag_frame;
	id3_nameTag_struct_typedef *nameTag;
	uint16_t flags;
	char frameID_str[5];
	uint8_t *str_p;

	tagPoint = sizeof(id3tag_head_struct_typedef);

	debug.printf("\r\nentireTagSize:%d", entireTagSize);

	while(tagPoint < entireTagSize){
		my_fread((void*)&id3tag_frame, 1, sizeof(id3v34tag_frame_struct_typedef), fp);
		frameID_str[4] = '\0';
		strncpy(frameID_str, id3tag_frame.frameID_str, 4);
		if(frameID_str[0] < 0x20 || frameID_str[0] > 0x80){ // not ascii then break
			break;
		}

		debug.printf("\r\n\nframeID_str:%s", frameID_str);
		if(ver == 4){
			frameSize = unSynchSize((void*)&id3tag_frame.p_size);
		} else {
			frameSize = b2l((void*)&id3tag_frame.p_size, sizeof(id3tag_frame.p_size));
		}
		debug.printf("\r\nsize:%d", frameSize);

		flags = *(uint16_t*)&id3tag_frame.flagsU16;
		debug.printf("\r\nflags:%d", flags);

		tagPoint += sizeof(id3v34tag_frame_struct_typedef) + frameSize;

		nameTag = '\0';
		if(strncmp(frameID_str, "TIT2", 4) == 0){
			nameTag = _id3_title;
		} else if(strncmp(frameID_str, "TALB", 4) == 0){
			nameTag = _id3_album;
		} else if(strncmp(frameID_str, "TPE1", 4) == 0){
			nameTag = _id3_artist;
		} else if(strncmp(frameID_str, "APIC", 4) == 0){
			memcpy((void*)_file_artwork, (void*)fp, sizeof(MY_FILE));
			return 0;
		} else {
			my_fseek(fp, frameSize, SEEK_CUR);
			continue;
		}

		if(nameTag != '\0'){
			my_fread((void*)&nameTag->encType, 1, 1, fp);
			frameSize -= sizeof(nameTag->encType);

			if(nameTag->encType){
				my_fread((void*)nameTag->bom, 1, 2, fp);
				frameSize -= sizeof(nameTag->bom);
			}

			str_p = malloc(frameSize);
			my_fread(str_p, 1, frameSize, fp);

			nameTag->flags = flags;
			nameTag->size = (frameSize < sizeof(nameTag->str_p) - 2 ? frameSize : sizeof(nameTag->str_p) - 2);

			memcpy(nameTag->str_p, str_p, nameTag->size);
			nameTag->str_p[nameTag->size] = '\0';
			nameTag->str_p[nameTag->size + 1] = '\0';

			free((void*)str_p);
		}
	}

	return 0;
}

static int ID3_2v2(MY_FILE *fp, uint32_t entireTagSize)
{
	uint32_t frameSize;
	int tagPoint;
	id3v2tag_frame_struct_typedef id3tag_frame;
	id3_nameTag_struct_typedef *nameTag;
	char frameID_str[4];
	uint8_t *str_p;

	tagPoint = sizeof(id3tag_head_struct_typedef);

	while(tagPoint < entireTagSize){
		my_fread((void*)&id3tag_frame, 1, sizeof(id3v2tag_frame_struct_typedef), fp);
		frameID_str[3] = '\0';
		strncpy(frameID_str, id3tag_frame.frameID_str, 3);
		if(frameID_str[0] < 0x20 || frameID_str[0] > 0x80){ // not ascii then break
			break;
		}

		debug.printf("\r\n\nframeID_str:%s", frameID_str);

		frameSize = b2l((void*)&id3tag_frame.p_size, sizeof(id3tag_frame.p_size));
		debug.printf("\r\nsize:%d", frameSize);

		tagPoint += sizeof(id3v2tag_frame_struct_typedef) + frameSize;

		nameTag = '\0';
		if(strncmp(frameID_str, "TT2", 3) == 0){
			nameTag = _id3_title;
		} else if(strncmp(frameID_str, "TAL", 3) == 0){
			nameTag = _id3_album;
		} else if(strncmp(frameID_str, "TP1", 3) == 0){
			nameTag = _id3_artist;
		} else if(strncmp(frameID_str, "PIC", 3) == 0){
			memcpy((void*)_file_artwork, (void*)fp, sizeof(MY_FILE));
			return 0;
		} else {
			my_fseek(fp, frameSize, SEEK_CUR);
			continue;
		}

		if(nameTag != '\0'){
			my_fread((void*)&nameTag->encType, 1, 1, fp);
			frameSize -= sizeof(nameTag->encType);

			if(nameTag->encType){
				my_fread((void*)nameTag->bom, 1, 2, fp);
				frameSize -= sizeof(nameTag->bom);
			}

			str_p = malloc(frameSize);
			my_fread(str_p, 1, frameSize, fp);

			nameTag->size = (frameSize < sizeof(nameTag->str_p) - 2 ? frameSize : sizeof(nameTag->str_p) - 2);

			memcpy(nameTag->str_p, str_p, nameTag->size);
			nameTag->str_p[nameTag->size] = '\0';
			nameTag->str_p[nameTag->size + 1] = '\0';

			free((void*)str_p);
		}
	}

	return 0;
}

void eliminateEndSpace(uint8_t data[], uint8_t len)
{
	int i, chrCnt = 0;

	for(i = len;i > 0;i--){
		if(data[i - 1] != '\0' && data[i - 1] != ' '){
			data[i] ='\0';
			break;
		}
	}

	for(i = 0;i < len;i++){
		if(data[i] != '\0') chrCnt++;
		else break;
	}

	if(chrCnt == 0 || chrCnt == len){
		data[0] = ' ';
	}
}


int ID3_1v01(MY_FILE *fp){
	char tag[4];

	my_fseek(fp, -127, SEEK_END);
	my_fread((uint8_t*)tag, 1, 3, fp);
	if(strncmp(tag, "TAG", 3) == 0){
		debug.printf("\r\nID3v1.x");
		my_fread(_id3_title->str_p, 1, 30, fp);
		my_fread(_id3_artist->str_p, 1, 30, fp);
		my_fread(_id3_album->str_p, 1, 30, fp);

		_id3_title->encType = _id3_artist->encType = _id3_album->encType = 0;

		eliminateEndSpace(_id3_title->str_p, 30);
		eliminateEndSpace(_id3_artist->str_p, 30);
		eliminateEndSpace(_id3_album->str_p, 30);
	}

	return 0;
}

int id3tag(MY_FILE *fp){
	volatile id3tag_head_struct_typedef id3tag;
	uint16_t ver;
	size_t size;

	my_fread((void*)&id3tag, 1, sizeof(id3tag_head_struct_typedef), fp);
	if(strncmp((char*)id3tag.id_str, "ID3", 3) == 0){
		ver = *(uint16_t*)&id3tag.verU16;
		size = unSynchSize((void*)id3tag.p_size);

		debug.printf("\r\nid3tag.version:%d", ver);
		debug.printf("\r\nsize:%d", size);

		switch(ver){
		case 2:
			ID3_2v2(fp, size);
			break;
		case 3:
		case 4:
			ID3_2v34(fp, size, ver);
			break;
		default:
			break;
		}
		my_fseek(fp, size + sizeof(id3tag_head_struct_typedef), SEEK_SET);
	} else {
		ID3_1v01(fp);
		my_fseek(fp, 0, SEEK_SET);
	}

	return 0;
}
void UTF16BigToLittle(uint8_t *str_p, size_t size)
{
	int i;
	uint8_t t_char;

	for(i = 0;i < size;i += sizeof(uint16_t)){
		t_char = str_p[i];
		str_p[i] = str_p[i + 1];
		str_p[i + 1] = t_char;
	}
}

uint16_t MP3PutString(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, id3_nameTag_struct_typedef *id3_nameTag, colors color)
{
	uint16_t len = 0;

	if(!id3_nameTag->encType){
		len = LCDPutStringSJISN(startPosX, endPosX, lineCnt, id3_nameTag->str_p, color);
	} else { // UTF-16
		if(id3_nameTag->bom[0] == 0xfe && id3_nameTag->bom[1] == 0xff){ // UTF-16BE
			UTF16BigToLittle(id3_nameTag->str_p, id3_nameTag->size); // big to little
		}
		len = LCDPutStringLFN(startPosX, endPosX, lineCnt, id3_nameTag->str_p, color);
	}
	return len;
}

uint16_t MP3GetStringPixelLength(id3_nameTag_struct_typedef *id3_nameTag, uint16_t font_width)
{
	uint16_t len = 0;

	if(!id3_nameTag->encType){
		len = LCDGetStringSJISPixelLength(id3_nameTag->str_p, font_width);
	} else { // UTF-16
		if(id3_nameTag->bom[0] == 0xfe && id3_nameTag->bom[1] == 0xff){ // UTF-16BE
			UTF16BigToLittle(id3_nameTag->str_p, id3_nameTag->size); // big to little
		}
		len = LCDGetStringLFNPixelLength(id3_nameTag->str_p, font_width);
	}

	return len;
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

int MP3FindXingWord(unsigned char *buf, int nBytes)
{
	int i;

	for (i = 0; i < nBytes - 3; i++) {
		if ((buf[i+0] == 'X') && (buf[i+1] == 'i') && (buf[i+2] == 'n') && (buf[i+3] == 'g'))
			return i;
	}

	return -1;
}

int MP3FindInfoWord(unsigned char *buf, int nBytes)
{
	int i;

	for (i = 0; i < nBytes - 3; i++) {
		if ((buf[i+0] == 'I') && (buf[i+1] == 'n') && (buf[i+2] == 'f') && (buf[i+3] == 'o'))
			return i;
	}

	return -1;
}


int PlayMP3(int id)
{
	time.flags.enable = 0;
	TouchPenIRQ_Disable();
	TOUCH_PINIRQ_DISABLE;
	touch.func = touch_empty_func;

	int i;
	int bytesLeft, nRead, err, offset, outOfData, eofReached;
	unsigned char *readPtr;
	unsigned char readBuf[READBUF_SIZE];
	short *outbuf;
	uint32_t *pabuf;

	int totalSec, remainTotalSec, media_data_totalBytes, minute, sec;
//	drawBuff_typedef *drawBuff = (drawBuff_typedef*)jFrameMem;
	drawBuff_typedef dbuf, *drawBuff;
	drawBuff = &dbuf;
	_drawBuff = drawBuff;

	int curX = 0, prevX = 0, ret = 0;
	char timeStr[20];

    uint8_t buf[512], hasXing = 0, hasInfo = 0;
    uint32_t seekBytes, count = 0, seekBytesSyncWord = 0;
    int xingSeekBytes = 0, infoSeekBytes = 0, header_offset;
    float duration;
    MP3FrameInfo info;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	void *putCharTmp = '\0', *putWideCharTmp = '\0';

	id3_nameTag_struct_typedef title, album, artist;
	_id3_title = &title, _id3_album = &album, _id3_artist = &artist;
	uint16_t xTag = 110, yTag = 67, disp_limit = 300, strLen = 0, yPos;

	MY_FILE *infile = '\0', file_artwork;

	file_artwork.clusterOrg = 0;
	_file_artwork = &file_artwork;

	music_src_p.curX = &curX;
	music_src_p.prevX = &prevX;
	music_src_p.media_data_totalBytes = &media_data_totalBytes;
	music_src_p.totalSec = &totalSec;
	music_src_p.drawBuff = drawBuff;
	music_src_p.fp = infile;

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

	LCDPutBgImgMusic();

	infile = my_fopen(id);
	if(infile == '\0'){
		LCDStatusStruct.waitExitKey = 0;
		ret = -1;
		goto EXIT_PROCESS;
	}

	MP3FrameInfo mp3FrameInfo;
	HMP3Decoder hMP3Decoder;

	memset(&mp3FrameInfo, '\0', sizeof(mp3FrameInfo));

	if ( (hMP3Decoder = MP3InitDecoder()) == 0 ){
		LCDStatusStruct.waitExitKey = 0;
		ret = -1;
		goto EXIT_PROCESS;
	}


	_id3_title->str_p[0] = _id3_album->str_p[0] = _id3_artist->str_p[0] = '\0';

	id3tag(infile);

/*
	if(_id3_title->str_p[0] != '\0'){
		debug.printf("\r\n\ntitle found\r\nsize:%d\r\nencType:%d", _id3_title->size, _id3_title->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_title->str_p);
		debug.printf("\r\n%04x", *(uint32_t*)&_id3_title->str_p[4]);
	}
	if(_id3_album->str_p[0] != '\0'){
		debug.printf("\r\n\nalbum found\r\nsize:%d\r\nencType:%d", _id3_album->size, _id3_album->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_album->str_p);
	}
	if(_id3_artist->str_p[0] != '\0'){
		debug.printf("\r\n\nartist found\r\nsize:%d\r\nencType:%d", _id3_artist->size, _id3_artist->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_artist->str_p);
	}
	*/

	if(!_id3_album->str_p[0] && !_id3_artist->str_p[0]){
		yTag += 20;
	} else if(!_id3_album->str_p[0] || !_id3_artist->str_p[0]){
		yTag += 10;
	}


	if(_id3_title->str_p[0] != 0){
		strLen = MP3GetStringPixelLength(_id3_title, 16);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = LCD_WIDTH - 1;
		} else {
			disp_limit = LCD_WIDTH - 20;
			yTag -= 8;
		}

		strLen = MP3GetStringPixelLength(_id3_album, 12);
		if((xTag + strLen) > (LCD_WIDTH - 20)){
			yTag -= 6;
		}
		LCDGotoXY(xTag + 1, yTag + 1);
		MP3PutString(xTag + 1, disp_limit, 2, _id3_title, BLACK);
		LCDGotoXY(xTag, yTag);
		yPos = MP3PutString(xTag, disp_limit - 1, 2, _id3_title, WHITE);
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

	if(_id3_album->str_p[0] != 0){
		LCDGotoXY(xTag + 1, yTag + 1);
		MP3PutString(xTag + 1, LCD_WIDTH - 20, 2, _id3_album, BLACK);
		LCDGotoXY(xTag, yTag);
		yPos = MP3PutString(xTag, LCD_WIDTH - 21, 2, _id3_album, WHITE);
		yTag += 20 + yPos;
	}
	if(_id3_artist->str_p[0] != 0){
		LCDGotoXY(xTag + 1, yTag + 1);
		MP3PutString(xTag + 1, disp_limit, 1, _id3_artist, BLACK);
		LCDGotoXY(xTag, yTag);
		MP3PutString(xTag, disp_limit - 1, 1, _id3_artist, WHITE);
	}

	dispArtWork(_file_artwork);
	LCDPutIcon(0, 155, 320, 80, music_underbar_320x80, music_underbar_320x80_alpha);

	bytesLeft = 0;
	outOfData = 0;
	eofReached = 0;
	readPtr = readBuf;
	nRead = 0;

	seekBytes = infile->seekBytes;

	while(!eofReached){
		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = readBuf;
			if (nRead == 0){
				eofReached = 1;
				break;
			}
		}

		if((header_offset = (MP3FindXingWord(readPtr, bytesLeft))) != -1){
			xingSeekBytes = header_offset + seekBytes;
			hasXing = 1;
		}

		if((header_offset = (MP3FindInfoWord(readPtr, bytesLeft))) != -1){
			infoSeekBytes = header_offset + seekBytes;
			hasInfo = 1;
		}

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {
			bytesLeft = 0;
			readPtr = readBuf;
		    seekBytes = infile->seekBytes;
			continue;
		} else {
			my_fseek(infile, seekBytes + offset, SEEK_SET);
			my_fread(buf, 1, 4, infile);

			if(UnpackFrameHeader((MP3DecInfo*)hMP3Decoder, buf) != -1){
				MP3GetLastFrameInfo(hMP3Decoder, &info);
			    if(info.bitrate){
			    	if(!count){
				    	memcpy(&mp3FrameInfo, &info, sizeof(MP3FrameInfo));
				    	debug.printf("\r\n\nmp3FrameInfo");
				    	debug.printf("\r\nbitrate:%d", mp3FrameInfo.bitrate);
				    	debug.printf("\r\nnChans:%d", mp3FrameInfo.nChans);
				    	debug.printf("\r\nsamprate:%d", mp3FrameInfo.samprate);
				    	debug.printf("\r\nbitsPerSample:%d", mp3FrameInfo.bitsPerSample);
				    	debug.printf("\r\noutputSamps:%d", mp3FrameInfo.outputSamps);
				    	debug.printf("\r\nlayer:%d", mp3FrameInfo.layer);
				    	debug.printf("\r\nversion:%d", mp3FrameInfo.version);
				    	seekBytesSyncWord = seekBytes + offset;
			    	}
			    	if(++count >= 2){
			    		break;
			    	}
			    }
			}
			bytesLeft = 0;
			readPtr = readBuf;
		    seekBytes = infile->seekBytes;
			continue;
		}
	}
	debug.printf("\r\nseekBytesSyncWord:%d", seekBytesSyncWord);
	media_data_totalBytes = infile->fileSize - seekBytesSyncWord;
	debug.printf("\r\nmedia_data_totalBytes:%d", media_data_totalBytes);

	if(hasXing){
		debug.printf("\r\nXing");
	}
	if(hasInfo){
		debug.printf("\r\nInfo");
	}

	if(hasInfo){
		hasXing = 1;
		xingSeekBytes = infoSeekBytes;
	}

	if(hasXing){
		debug.printf("\r\nxingSeekBytes:%d", xingSeekBytes);
		xing_struct_typedef xing_struct;
		my_fseek(infile, xingSeekBytes, SEEK_SET);
		my_fread((uint8_t*)&xing_struct, 1, 12, infile);
		if((strncmp(xing_struct.head_char, "Xing", 4) != 0) && (strncmp(xing_struct.head_char, "Info", 4) != 0)){
			goto CALC_CBR_DURATION;
		}
		debug.printf("\r\ncalc VBR duration");
		xing_struct.flags = b2l(&xing_struct.flags, sizeof(xing_struct.flags));
		if(!(xing_struct.flags & 0x0001)){ // has frames?
			goto CALC_CBR_DURATION;
		}
		xing_struct.frames = b2l(&xing_struct.frames, sizeof(xing_struct.frames));
		duration = (float)xing_struct.frames * ((float)(mp3FrameInfo.outputSamps / mp3FrameInfo.nChans) / (float)mp3FrameInfo.samprate) + 0.5f;
		minute = (float)duration / 60.0f;
		sec = (int)((((float)duration / 60.0f) - (float)minute) * 60.0f);
	} else {
		CALC_CBR_DURATION:
		duration = (float)media_data_totalBytes / (float)mp3FrameInfo.bitrate * 8.0f + 0.5f;
		minute = (float)duration / 60.0f;
		sec = (int)((((float)duration / 60.0f) - (float)minute) * 60.0f);
	}

	char s[20];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCDGotoXY(21, MUSIC_INFO_POS_Y + 1);
	LCDPutString(s, BLACK);
	LCDGotoXY(20, MUSIC_INFO_POS_Y);
	LCDPutString(s, WHITE);

	if(settings_group.music_conf.b.musicinfo){
		LCDGotoXY(71, MUSIC_INFO_POS_Y + 1);
		LCDPutString("MP3", BLACK);
		LCDGotoXY(70, MUSIC_INFO_POS_Y);
		LCDPutString("MP3", WHITE);

		LCDGotoXY(111, MUSIC_INFO_POS_Y + 1);
		LCDPutString(mp3FrameInfo.nChans == 2 ? "Stereo" : "Mono", BLACK);
		LCDGotoXY(110, MUSIC_INFO_POS_Y);
		LCDPutString(mp3FrameInfo.nChans == 2 ? "Stereo" : "Mono", WHITE);

		SPRINTF(s, "%dkbps", mp3FrameInfo.bitrate / 1000);
		LCDGotoXY(171, MUSIC_INFO_POS_Y + 1);
		LCDPutString(s, BLACK);
		LCDGotoXY(170, MUSIC_INFO_POS_Y);
		LCDPutString(s, WHITE);

		SPRINTF(s, "%dHz", mp3FrameInfo.samprate);
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

	debug.printf("\r\n%d:%02d", minute, sec);

	totalSec = duration;

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

	debug.printf("\r\nsampleRate:%d", mp3FrameInfo.samprate);

	my_fseek(infile, seekBytesSyncWord, SEEK_SET);

	int curXprev, position_changed = 0, cnt = 0;
	uint32_t  noerror_cnt = 0;

	float *fabuf, *fbbuf;
	float *float_buf = (float*)mempool;
	int fbuf_len = (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP) * mp3FrameInfo.nChans;

	memset(float_buf, '\0', fbuf_len * sizeof(float));

	uint8_t SOUND_BUFFER[9216];
	dac_intr.buff = SOUND_BUFFER;
    dac_intr.bufferSize = (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP) * sizeof(int16_t) * mp3FrameInfo.nChans;
    int SoundDMAHalfBlocks = dac_intr.bufferSize / 2 / (sizeof(uint16_t) * 2);
    int delay_buffer_filled = 0, boost = 0, loop_icon_touched = 0, loop_icon_cnt = 0;

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
	IIR.sbuf_size = (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP) * mp3FrameInfo.nChans;
	IIR.num_blocks = SoundDMAHalfBlocks;
	IIR.fs = mp3FrameInfo.samprate;
	IIR.number = bass_boost_mode;
	boost = bass_boost_mode;
	IIR_Set_Params(&IIR);

	REVERB_Struct_Typedef RFX;
	RFX.delay_buffer = &delay_buffer;
	RFX.num_blocks = SoundDMAHalfBlocks;
	RFX.fs = mp3FrameInfo.samprate;
	RFX.number = reverb_effect_mode;
	REVERB_Set_Prams(&RFX);

	FFT_Struct_Typedef FFT;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 64;
	FFT.samples = dac_intr.bufferSize / (sizeof(int16_t) * mp3FrameInfo.nChans) / 2;
	if(mp3FrameInfo.nChans < 2){
		FFT.samples >>= 1;
	}
	FFT_Init(&FFT);

	bytesLeft = 0;
	outOfData = 0;
	eofReached = 0;
	readPtr = readBuf;
	nRead = 0;

	position_changed = 0;
	noerror_cnt = 0;

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

	SOUNDInitDAC(mp3FrameInfo.samprate);
	SOUNDDMAConf((void*)&DAC->DHR12LD, sizeof(int16_t) * mp3FrameInfo.nChans, sizeof(int16_t) * mp3FrameInfo.nChans);
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
 			continue;
 		}

		if(!position_changed && cnt++ > 10){
			AUDIO_OUT_ENABLE;
		}

		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = readBuf;
			if (nRead == 0){
				eofReached = 1;
				break;
			}
		}

		if(TP_PEN_INPUT_BB == Bit_RESET && !loop_icon_touched){ // タッチパネルが押されたか？
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
				goto END_MP3;
			}
			if(curX == curXprev){
				goto SKIP_SEEK;
			}

			my_fseek(infile, seekBytesSyncWord + music_src_p.offset, SEEK_SET);

			bytesLeft = 0;
			eofReached = 0;
			readPtr = readBuf;
			err = 0;

			music_play.currentTotalSec = totalSec * (float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes;

			position_changed = 1;
			noerror_cnt = 0;
SKIP_SEEK:
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
			music_play.update = 0, cnt = 0;

			DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
			DMA_Cmd(DMA1_Stream1, ENABLE);
			if(music_play.currentTotalSec == 0){
				my_fseek(infile, seekBytesSyncWord, SEEK_SET);
				bytesLeft = 0;
				outOfData = 0;
				eofReached = 0;
				readPtr = readBuf;
				nRead = 0;
				continue;
			}
		}
		EXIT_TP:

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {
			debug.printf("\r\nno sync found");
			bytesLeft = 0;
			readPtr = readBuf;
			continue;
		}
		readPtr += offset;
		bytesLeft -= offset;

 		/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */
		err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, outbuf, 0);

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
		if(delay_buffer_filled && !position_changed && boost){
			IIR_Filter(&IIR, pabuf, fabuf, fbbuf);
		}

		/* Reverb effect */
		if(delay_buffer_filled && !position_changed && reverb_effect_mode){ // Reverb effect
			REVERB(&RFX, pabuf);
		}

		if(!position_changed && settings_group.music_conf.b.fft){
			/* sample audio data for FFT calcuration */
	 		FFT_Sample(&FFT, pabuf);

	 		/* FFT analyzer left */
	 		FFT_Display_Left(&FFT, drawBuff, 0xef7d);

	 		/* FFT analyzer right */
	 		FFT_Display_Right(&FFT, drawBuff, 0xef7d);
		}

		if (err) {
			noerror_cnt = 0;
			/* error occurred */
			switch (err) {
			case ERR_MP3_INDATA_UNDERFLOW:
				debug.printf("\r\nERR_MP3_INDATA_UNDERFLOW");
				break;
			case ERR_MP3_MAINDATA_UNDERFLOW:
				/* do nothing - next call to decode will provide more mainData */
				debug.printf("\r\nERR_MP3_MAINDATA_UNDERFLOW");
				break;
			case ERR_MP3_FREE_BITRATE_SYNC:
				debug.printf("\r\nERR_MP3_FREE_BITRATE_SYNC");
				break;
			case ERR_MP3_INVALID_FRAMEHEADER:
			case ERR_MP3_INVALID_HUFFCODES:
				debug.printf("\r\nERR_MP3_INVALID_(FRAMEHEADER | HUFFCODES)");
				bytesLeft = 0;
				readPtr = readBuf;
				continue;
			default:
				debug.printf("\r\nerr:%d", err);
				break;
			}
		} else {
			noerror_cnt++;
		}

		if(position_changed && noerror_cnt >= 15){
			position_changed = 0;
			AUDIO_OUT_ENABLE;
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
			curX = (LCD_WIDTH - (33)) * (float)((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes) + 8;
			LCDStoreBgImgToBuff(curX, drawBuff->posision.y, \
								drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);
			LCDPutIcon(curX, drawBuff->posision.y, \
					   drawBuff->posision.width, drawBuff->posision.height, seek_circle_16x16, seek_circle_16x16_alpha);

			prevX = curX;
			continue;
		}
	}

	if(!eofReached && !LCDStatusStruct.waitExitKey){
		ret = RET_PLAY_STOP;
	} else {
		ret = RET_PLAY_NORM;
	}

END_MP3:
	MP3FreeDecoder(hMP3Decoder);

EXIT_PROCESS:
	AUDIO_OUT_SHUTDOWN;
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, DISABLE);

	/* Disable the TIM1 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	my_fclose(infile);
	LCDStatusStruct.waitExitKey = 0;

	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;

	TOUCH_PINIRQ_ENABLE;
	TouchPenIRQ_Enable();
	time.prevTime = time.curTime;
	time.flags.enable = 1;

	return ret;
}
