/*
 * mjpeg.c
 *
 *  Created on: 2011/07/10
 *      Author: Tonsuke
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "sound.h"
#include "fat.h"
#include "xpt2046.h"
#include "usart.h"
#include "pcf_font.h"


#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */

#include "mjpeg.h"

#define FUNC_VIDEO_BGIMG LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK)

typedef struct{
	int x, y, width, height;
	uint16_t p[30 * 30];
}pic_arrow_buf_typedef;


__attribute__( ( always_inline ) ) static __INLINE size_t getAtomSize(void* atom)
{
/*
	ret =  *(uint8_t*)(atom + 0) << 24;
	ret |= *(uint8_t*)(atom + 1) << 16;
	ret |= *(uint8_t*)(atom + 2) << 8;
	ret |= *(uint8_t*)(atom + 3);
*/
	return __REV(*(uint32_t*)atom);
}

__attribute__( ( always_inline ) ) static __INLINE size_t getSampleSize(void* atombuf, int bytes, MY_FILE *fp)
{
	my_fread(atombuf, 1, bytes, fp);
	return getAtomSize(atombuf);
}


uint32_t b2l(void* val, size_t t){
	uint32_t ret = 0;
	size_t tc = t;

	for(;t > 0;t--){
		ret |= *(uint8_t*)(val + tc - t) << 8 * (t - 1);
	}

	return ret;
}


int collectAtoms(MY_FILE *fp, size_t parentAtomSize, size_t child)
{
	int index, is;
	size_t atomSize, totalAtomSize = 0;
	uint32_t timeScale = 0, duration = 0;
	uint8_t atombuf[512];

	MY_FILE fp_tmp;

	memset(atombuf, '\0', sizeof(atombuf));

	do{
		atomSize = getSampleSize(atombuf, 8, fp);

		for(index = 0;index < ATOM_ITEMS;index++){
			if(!strncmp((char*)&atombuf[4], (char*)&atomTypeString[index][0], 4)){
				debug.printf("\r\n");
				for(is = child;is > 0;is--) debug.printf(" ");
				debug.printf("%s %d", (char*)&atomTypeString[index][0], atomSize);
				break;
			}
		}

//		if(index >= ATOM_ITEMS){
//			debug.printf("\r\nInvalid atom:%s %d", (char*)&atombuf[4], atomSize);
//			return -1;
//		}
		if(index >= ATOM_ITEMS){
			debug.printf("\r\nunrecognized atom:%s %d", (char*)&atombuf[4], atomSize);
			goto NEXT;
		}

		memcpy((void*)&fp_tmp, (void*)fp, sizeof(MY_FILE));

		switch(index){
		case MDHD:
			my_fseek(fp, 12, SEEK_CUR); // skip ver/flag  creationtime modificationtime

			timeScale = getSampleSize(atombuf, 4, fp); // time scale
			duration = getSampleSize(atombuf, 4, fp); // duration

			break;
		case HDLR:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			my_fread(atombuf, 1, 4, fp); // Component type
			my_fread(atombuf, 1, 4, fp); // Component subtype
			if(!strncmp((char*)atombuf, (const char*)"soun", 4)){
				media.sound.flag.process = 1;
				media.sound.timeScale = timeScale;
				media.sound.duration = duration;
			}
			if(!strncmp((char*)atombuf, (const char*)"vide", 4)){
				media.video.flag.process = 1;
				media.video.timeScale = timeScale;
				media.video.duration = duration;
			}
			break;
		case TKHD:
			my_fseek(fp, 74, SEEK_CUR); // skip till width, height data
			my_fread(atombuf, 1, 4, fp); // width
			if(getAtomSize(atombuf)){
				media.video.width = getAtomSize(atombuf);
			}
			my_fread(atombuf, 1, 4, fp); // height
			if(getAtomSize(atombuf)){
				media.video.height = getAtomSize(atombuf);
			}
			break;
		case STSD:
			my_fseek(fp, 8, SEEK_CUR); // skip Reserved(6bytes)/Data Reference Index
			my_fread(atombuf, 1, 4, fp); // next atom size
			my_fread(atombuf, 1, getAtomSize(atombuf) - 4, fp);
			if(media.video.flag.process && !media.video.flag.complete){
				memset((void*)media.video.videoFmtString, '\0', sizeof(media.video.videoFmtString));
				memset((void*)media.video.videoCmpString, '\0', sizeof(media.video.videoCmpString));
				memcpy((void*)media.video.videoFmtString, (void*)atombuf, 4);
				memcpy((void*)media.video.videoCmpString, (void*)&atombuf[47], atombuf[46]);
				if(strncmp((char*)media.video.videoFmtString, "jpeg", 4) == 0){
					media.video.playJpeg = 1; // JPEG
				} else {
					media.video.playJpeg = 0; // Uncompression
				}
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				memcpy((void*)&media.sound.format, (void*)atombuf, sizeof(sound_format));
				media.sound.format.version = (uint16_t)b2l((void*)&media.sound.format.version, sizeof(uint16_t));
				media.sound.format.revision = (uint16_t)b2l((void*)&media.sound.format.revision, sizeof(media.sound.format.revision));
				media.sound.format.vendor = (uint16_t)b2l((void*)&media.sound.format.vendor, sizeof(media.sound.format.vendor));
				media.sound.format.numChannel = (uint16_t)b2l((void*)&media.sound.format.numChannel, sizeof(media.sound.format.numChannel));
				media.sound.format.sampleSize = (uint16_t)b2l((void*)&media.sound.format.sampleSize, sizeof(media.sound.format.sampleSize));
				media.sound.format.complesionID = (uint16_t)b2l((void*)&media.sound.format.complesionID, sizeof(media.sound.format.complesionID));
				media.sound.format.packetSize = (uint16_t)b2l((void*)&media.sound.format.packetSize, sizeof(media.sound.format.packetSize));
				media.sound.format.sampleRate = (uint16_t)b2l((void*)&media.sound.format.sampleRate, sizeof(uint16_t));
			}
			break;
		case STTS:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stts.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stts.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stts.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stts.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STSC:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stsc.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stsc.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stsc.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsc.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STSZ:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stsz.sampleSize = getSampleSize(atombuf, 4, fp);
				video_stsz.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stsz.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stsz.sampleSize = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsz.fp, (void*)fp, sizeof(MY_FILE));
				sound_stsz.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsz.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STCO:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stco.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stco.fp, (void*)fp, sizeof(MY_FILE));

				media.video.flag.process = 0;
				media.video.flag.complete = 1;
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stco.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stco.fp, (void*)fp, sizeof(MY_FILE));

				media.sound.flag.process = 0;
				media.sound.flag.complete = 1;
			}
			break;
		default:
			break;
		}

		memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));

		if(index < ATOM_ITEMS && atomHasChild[index]){
			memcpy((void*)&fp_tmp, (void*)fp, sizeof(MY_FILE));

			if(collectAtoms(fp, atomSize - 8, child + 1) != 0){ // Re entrant
				return -1;
			}

			memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));
		}

NEXT:
		my_fseek(fp, atomSize - 8, SEEK_CUR);
		totalAtomSize += atomSize;
	}while(parentAtomSize > totalAtomSize);

	return 0;
}

static inline uint32_t getVideoSampleTime(uint8_t *atombuf, uint32_t sampleID)
{
	static uint32_t numSamples, count, duration;
	static MY_FILE fp;

	if(sampleID == 0){
		numSamples = count = 0;
		memcpy((void*)&fp, (void*)&video_stts.fp, sizeof(MY_FILE));
	}

	if(numSamples < ++count){
		count = 1;
		numSamples = getSampleSize(atombuf, 8, &fp);
		duration = getAtomSize(&atombuf[4]);
	}

	return duration;
}

void *putCharTmp = '\0', *putWideCharTmp = '\0';


void mjpegTouch(void) // タッチペン割込み処理
{
	static int prev_posX = 0;
	uint32_t i, pointed_chunk, firstChunk, prevChunk, prevSamples, samples, totalSamples = 0, jFrameSize, jFrameOffset;
	struct jpeg_decompress_struct jdinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;
	MY_FILE fp_stsc, fp_stsz, fp_stco, fp_jpeg, fp_frame, fp_frame_cp;
	uint8_t atombuf[12];

	raw_video_typedef raw;

	prepare_range_limit_table2((uint8_t*)progress_circular_bar_16x16x12_buff, &jdinfo);
	build_ycc_rgb_table2((uint8_t*)&progress_circular_bar_16x16x12_buff[1408 / 2], &jdinfo);


	if(mjpeg_touch.resynch){
		goto RESYNCH;
	}

//	debug.printf("\r\nposX:%d posY:%d", touch.posX, touch.posY);

	if((touch.posX > (91 - 10) && touch.posX < (91 + 32 + 10)) && \
			(touch.posY > (197 - 10) && touch.posY < (197 + 17 + 10))) // check if left arrow clicked
	{
		mjpeg_touch.id = -3;
		mjpeg_touch.done = 1;
		return;
	}

	if((touch.posX > (193 - 10) && touch.posX < (193 + 32 + 10)) && \
			(touch.posY > (197 - 10) && touch.posY < (197 + 17 + 10))) // check if right arrow clicked
	{
		mjpeg_touch.id = -2;
		mjpeg_touch.done = 1;
		return;
	}

	if((touch.posX > (_drawBuff->navigation_loop.x - 15) && touch.posX < (_drawBuff->navigation_loop.x + _drawBuff->navigation_loop.width + 15)) && \
			(touch.posY > (_drawBuff->navigation_loop.y - 10) && touch.posY < (_drawBuff->navigation_loop.y + _drawBuff->navigation_loop.height + 10))){ //

		Update_Navigation_Loop_Icon(navigation_loop_mode = ++navigation_loop_mode % 5);

		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET);

		return;
	}

	if((touch.posX > 10 && touch.posX < 50) && (touch.posY > 200 && touch.posY < 230)){ // detect close icon
		mjpeg_touch.id = 1;
		mjpeg_touch.done = 1;
	} else if((touch.posX > 142 && touch.posX < 176) && (touch.posY > 189 && touch.posY < 221)){ // detect play icon
		mjpeg_touch.id = 2;
		mjpeg_touch.done = 1;
	} else if((touch.posX > 270  && touch.posX < 320) && (touch.posY > 10 && touch.posY < 50)){ // detect video info icon
		char s[40], timeStr[10];
		LCDPutIcon(74, 19, 170, 170, video_info_board_170x170, video_info_board_170x170_alpha);

		LCD_FUNC.putChar = putCharTmp;
		LCD_FUNC.putWideChar = putWideCharTmp;

		clx = 85, cly = 28;
		LCDPutString("[Video]\n", WHITE);
		clx = 85,
		sprintf(s, "Type: %s\n", media.video.videoCmpString);
		LCDPutString(s, WHITE);
		clx = 85;
		sprintf(s, "Dimension: %d x %d\n", media.video.width, media.video.height);
		LCDPutString(s, WHITE);
		clx = 85;
		sprintf(s, "Frame Rate: %d fps\n", media.video.frameRate);
		LCDPutString(s, WHITE);
		setStrSec(timeStr, (int)((float)media.video.duration / (float)media.video.timeScale + 0.5f));
		clx = 85;
		sprintf(s, "Duration: %s\n\n", timeStr);
		LCDPutString(s, WHITE);

		clx = 85;
		LCDPutString("[Sound]\n", WHITE);
		clx = 85;
		sprintf(s, "Channels: %s\n", media.sound.format.numChannel <= 1 ? "Mono" : "Stereo");
		LCDPutString(s, WHITE);
		clx = 85;
		sprintf(s, "Sample Size: %d bit\n", media.sound.format.sampleSize);
		LCDPutString(s, WHITE);
		clx = 85;
		sprintf(s, "Sample Rate: %d Hz\n", media.sound.format.sampleRate);
		LCDPutString(s, WHITE);

		while(TP_PEN_INPUT_BB != Bit_RESET){};
	} else if(prev_posX != touch.posX) { // 動画シーク先のプレビューを表示
		if(touch.posY > 190){
			return;
		}
		RESYNCH:
		if(!mjpeg_touch.resynch){
			pointed_chunk = video_stco.numEntry * (float)((float)touch.posX / (float)LCD_WIDTH);
			if(touch.posX <= 7){
				pointed_chunk = 2;
			}
		} else {
			pointed_chunk = mjpeg_touch.resynch_entry;
		}

		/* Initialize the JPEG decompression object with default error handling. */
		jdinfo.useMergedUpsampling = 1;
		jdinfo.err = jpeg_std_error(&jerr);

		memcpy((void*)&fp_stsc, (void*)&video_stsc.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_stsz, (void*)&video_stsz.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_frame, (void*)&fp_global, sizeof(MY_FILE));

//		debug.printf("\r\npointed_chunk:%d video_stco.numEntry:%d", pointed_chunk, video_stco.numEntry);

		prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
		prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk

		while(1){
			if(prevChunk <= pointed_chunk && firstChunk >= pointed_chunk){
				samples = (firstChunk - pointed_chunk) * prevSamples;
				totalSamples += (pointed_chunk - prevChunk) * prevSamples;
				break;
			}
			samples = (firstChunk - prevChunk) * prevSamples;
			totalSamples += samples;

			prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
			prevSamples = getSampleSize(atombuf, 8, &fp_stsc); // samplesPerChunk sampleDescriptionID
			firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 次のfirstChunk
		}
		if(media.video.playJpeg){
			my_fseek(&fp_stsz, totalSamples * 4, SEEK_CUR);
			jFrameSize = getSampleSize(atombuf, 4, &fp_stsz);

			memcpy((void*)&fp_stsz, (void*)&video_stsz.fp, sizeof(MY_FILE));
			my_fseek(&fp_stsz, totalSamples * 4, SEEK_CUR);
		}

		my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR);
		jFrameOffset = getSampleSize(atombuf, 4, &fp_stco);

		memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
		my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR); //

		my_fseek(&fp_frame, jFrameOffset, SEEK_SET);

		if(media.video.playJpeg){
			fp_jpeg.fileSize = jFrameSize;
			fp_jpeg.seekBytes = 0;

			my_fread((void*)CCM_BASE, 1, jFrameSize, &fp_frame);
			jpeg_read.frame_size = jFrameSize;
		} else {
			raw.output_scanline = 0;
			raw.frame_size = media.video.width * media.video.height * sizeof(uint16_t);
			raw.rasters = 50;
			raw.buf_size = raw.rasters * media.video.width * sizeof(uint16_t);
			memcpy((void*)&fp_frame_cp, (void*)&fp_frame, sizeof(MY_FILE));
			my_fseek(&fp_frame, raw.frame_size, SEEK_CUR);
		}

		if(media.video.width != LCD_WIDTH || media.video.height != LCD_HEIGHT){
			LCDDrawSquare(0, 0, LCD_WIDTH, media.video.startPosY, BLACK);
			LCDDrawSquare(0, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
			LCDDrawSquare(media.video.startPosX + media.video.width, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
			LCDDrawSquare(0, media.video.startPosY + media.video.height, LCD_WIDTH, LCD_HEIGHT - (media.video.startPosY + media.video.height), BLACK);
		}

		LCDSetWindowArea(media.video.startPosX, media.video.startPosY, media.video.width, media.video.height);
		LCDSetGramAddr(media.video.startPosX, media.video.startPosY);
		LCDPutCmd(0x0022);

		if(media.video.playJpeg){
			create_mpool();
			jpeg_create_decompress(&jdinfo);

			/* Specify data source for decompression */
			jpeg_stdio_src(&jdinfo, &fp_jpeg);

			/* Read file header, set default decompression parameters */
			(void) jpeg_read_header(&jdinfo, TRUE);

			dest_mgr = jinit_write_ppm(&jdinfo);

			/* Start decompressor */
			(void) jpeg_start_decompress(&jdinfo);

			/* Process data */
			while (jdinfo.output_scanline < jdinfo.output_height) {
			  jpeg_read_scanlines(&jdinfo, dest_mgr->buffer, dest_mgr->buffer_height);
			}
		} else {
			while((raw.output_scanline < media.video.height)){
				if(raw.frame_size < raw.buf_size){
					raw.buf_size = raw.frame_size;
				}
				my_fread((void*)mempool, 1, raw.buf_size, &fp_frame_cp);
				raw.p_raster = (uint16_t*)mempool;
				for(i = 0;i < (raw.buf_size >> 1);i++){
					LCD->RAM = *raw.p_raster++;
				}
				raw.frame_size = raw.frame_size - raw.buf_size;
				raw.output_scanline = raw.output_scanline + raw.rasters;
			}
		}
//		debug.printf("\r\npointed_chunk:%d jFrameSize:%d JFrameOffset:%d totalSamples:%d", pointed_chunk, jFrameSize, JFrameOffset, totalSamples);
		mjpeg_touch.id = 99;
		mjpeg_touch.draw_icon = 0;

		*pv_src.firstChunk = firstChunk;
		*pv_src.prevChunk = prevChunk;
		*pv_src.prevSamples = prevSamples;
		*pv_src.samples = samples;
		*pv_src.totalSamples = totalSamples;
		*pv_src.videoStcoCount = pointed_chunk + 1;

		memcpy((void*)pv_src.fp_video_stsc, (void*)&fp_stsc, sizeof(MY_FILE));
		memcpy((void*)pv_src.fp_video_stsz, (void*)&fp_stsz, sizeof(MY_FILE));
		memcpy((void*)pv_src.fp_video_stco, (void*)&fp_stco, sizeof(MY_FILE));
		memcpy((void*)pv_src.fp_frame, (void*)&fp_frame, sizeof(MY_FILE));

		if(media.video.playJpeg){
			(void) jpeg_finish_decompress(&jdinfo);
			jpeg_destroy_decompress(&jdinfo);
		}

		// Sound
		memcpy((void*)&fp_stsc, (void*)&sound_stsc.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_stsz, (void*)&sound_stsz.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_stco, (void*)&sound_stco.fp, sizeof(MY_FILE));
		memcpy((void*)&fp_frame, (void*)&fp_global, sizeof(MY_FILE));

//		debug.printf("\r\npointed_chunk:%d video_stco.numEntry:%d", pointed_chunk, video_stco.numEntry);

		prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
		prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk

		totalSamples = 0;
		while(1){
			if(prevChunk <= pointed_chunk && firstChunk >= pointed_chunk){
				samples = (firstChunk - pointed_chunk) * prevSamples;
				totalSamples += (pointed_chunk - prevChunk) * prevSamples;
				break;
			}
			samples = (firstChunk - prevChunk) * prevSamples;
			totalSamples += samples;

			prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
			prevSamples = getSampleSize(atombuf, 8, &fp_stsc); // samplesPerChunk sampleDescriptionID
			firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 次のfirstChunk
		}
		my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR);

		*ps_src.firstChunk = firstChunk;
		*ps_src.prevChunk = prevChunk;
		*ps_src.prevSamples = prevSamples;
		*ps_src.samples = samples;
		*ps_src.soundStcoCount = pointed_chunk;

		memcpy((void*)ps_src.fp_sound_stsc, (void*)&fp_stsc, sizeof(MY_FILE));
		memcpy((void*)ps_src.fp_sound_stsz, (void*)&fp_stsz, sizeof(MY_FILE));
		memcpy((void*)ps_src.fp_sound_stco, (void*)&fp_stco, sizeof(MY_FILE));

		if(mjpeg_touch.resynch){
			return;
		}

		LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);
		LCDSetGramAddr(0, 0);
		LCDPutCmd(0x0022);

		int totalSec = (int)((float)media.video.duration / (float)media.video.timeScale + 0.5f);
		int currentTotalSec = totalSec * (float)(touch.posX>7?touch.posX:0) / (float)(LCD_WIDTH - 1);
		int remainTotalSec = abs(currentTotalSec - totalSec);
		char timeStr[20];

		setStrSec(timeStr, -remainTotalSec);
		LCDGotoXY(totalSec < 6000 ? 269 : 261, 189);
		LCDPutString(timeStr, BLACK);
		LCDGotoXY(totalSec < 6000 ? 268 : 260, 188);
		LCDPutString(timeStr, WHITE);

		setStrSec(timeStr, currentTotalSec);
		LCDGotoXY(15, 189);
		LCDPutString(timeStr, BLACK);
		LCDGotoXY(14, 188);
		LCDPutString(timeStr, WHITE);

		int position = touch.posX - 10;

		if(position < 0){
			position = 0;
		}

		if(position > 300){
			position = 300;
		}

		LCDDrawSquare(10, 176, 300, 1, WHITE);
		LCDDrawSquare(11, 177, 300, 1, GRAY);
		LCDPutIcon(position, 168, 16, 16, seek_circle_16x16, seek_circle_16x16_alpha);

	}
	prev_posX = touch.posX;
}


int mjpegPause()
{
	AUDIO_OUT_SHUTDOWN;
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, DISABLE);

	debug.printf("\r\npause");

	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);

	int ret = RET_PLAY_NORM, position;
	int totalSec = media.video.duration / media.video.timeScale;
	int currentTotalSec = totalSec * (float)((float)*pv_src.videoStcoCount / (float)video_stco.numEntry);
	int remainTotalSec = abs(currentTotalSec - totalSec) * -1;
	char timeStr[20];

	// time remain
	setStrSec(timeStr, remainTotalSec);
	LCDGotoXY(totalSec < 6000 ? 269 : 261, 189);
	LCDPutString(timeStr, BLACK);
	LCDGotoXY(totalSec < 6000 ? 268 : 260, 188);
	LCDPutString(timeStr, WHITE);

	// time elapsed
	setStrSec(timeStr, currentTotalSec);
	LCDGotoXY(15, 189);
	LCDPutString(timeStr, BLACK);
	LCDGotoXY(14, 188);
	LCDPutString(timeStr, WHITE);


	_drawBuff->navigation_loop.x = 277;
	_drawBuff->navigation_loop.y = 207;
	_drawBuff->navigation_loop.width = 24;
	_drawBuff->navigation_loop.height = 18;
	LCDStoreBgImgToBuff(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, \
			            _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, _drawBuff->navigation_loop.p);

	Update_Navigation_Loop_Icon(navigation_loop_mode);

	LCDPutIcon(280, 18, 26, 24, video_info_26x24, video_info_26x24_alpha);
	LCDPutIcon(142, 189, 32, 32, navigation_playing_patch_32x32, navigation_playing_patch_32x32_alpha);
	LCDPutIcon(193, 197, 32, 17, next_right_32x17, next_right_32x17_alpha);
	LCDPutIcon(91, 197, 32, 17, next_left_32x17, next_left_32x17_alpha);
	LCDPutIcon(18, 210, 20, 13, exit_play_20x13, exit_play_20x13_alpha);

	position = LCD_WIDTH * (float)((float)*pv_src.videoStcoCount / (float)video_stco.numEntry) - 10;

	if(position < 0){
		position = 0;
	}
	if(position > 300){
		position = 300;
	}

	LCDDrawSquare(10, 176, 300, 1, WHITE);
	LCDDrawSquare(11, 177, 300, 1, GRAY);
	LCDPutIcon(position, 168, 16, 16, seek_circle_16x16, seek_circle_16x16_alpha);

	mjpeg_touch.id = 0;
	mjpeg_touch.done = 0;
	touch.func = mjpegTouch;

	TOUCH_PINIRQ_ENABLE;
	while(!mjpeg_touch.done){
		if(mjpeg_touch.id == 99){
			if((GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_SET && !mjpeg_touch.draw_icon)){

				LCDPutIcon(280, 18, 26, 24, video_info_26x24, video_info_26x24_alpha);
				LCDPutIcon(142, 189, 32, 32, navigation_playing_patch_32x32, navigation_playing_patch_32x32_alpha);
				LCDPutIcon(193, 197, 32, 17, next_right_32x17, next_right_32x17_alpha);
				LCDPutIcon(91, 197, 32, 17, next_left_32x17, next_left_32x17_alpha);
				LCDPutIcon(18, 210, 20, 13, exit_play_20x13, exit_play_20x13_alpha);

				LCDStoreBgImgToBuff(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, \
						            _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, _drawBuff->navigation_loop.p);

				Update_Navigation_Loop_Icon(navigation_loop_mode);

				mjpeg_touch.draw_icon = 1;
			}
			mjpeg_touch.id = 0;
			ret = 1;
		}
	}
	TOUCH_PINIRQ_DISABLE;

	switch(mjpeg_touch.id){
	case 0:
		break;
	case 1: // 閉じるアイコン選択
		LCDStatusStruct.waitExitKey = 0;
		ret = RET_PLAY_STOP;
		break;
	case 2: // 再生アイコン選択
		break;
	case -2: // 右矢印アイコン
		ret = RET_PLAY_NEXT;
		break;
	case -3: // 左矢印アイコン
		ret = RET_PLAY_PREV;
		break;
	default:
		break;
	}

	touch.func = touch_empty_func;

	memset(dac_intr.buff, '\0', dac_intr.bufferSize);

	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
	DMA_Cmd(DMA1_Stream1, ENABLE);

	if(media.video.width != LCD_WIDTH || media.video.height != LCD_HEIGHT){
		LCDDrawSquare(0, 0, LCD_WIDTH, media.video.startPosY, BLACK);
		LCDDrawSquare(0, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCDDrawSquare(media.video.startPosX + media.video.width, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCDDrawSquare(0, media.video.startPosY + media.video.height, LCD_WIDTH, LCD_HEIGHT - (media.video.startPosY + media.video.height), BLACK);
	}

	LCDSetWindowArea(media.video.startPosX, media.video.startPosY, media.video.width, media.video.height);
	LCDSetGramAddr(media.video.startPosX, media.video.startPosY);
	LCDPutCmd(0x0022);

    return ret;
}

int PlayMotionJpeg(int id){
	LCDBackLightTimerDeInit();
	TOUCH_PINIRQ_DISABLE;
	touch.func = touch_empty_func;

	register uint32_t i, j, k;
	int ret = 0;

	struct jpeg_decompress_struct jdinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;

	uint32_t fps, frames, prevFrames, sample_time_limit;
	uint32_t samples, frameDuration, numEntry;
	uint32_t prevChunkSound, prevSamplesSound, firstChunkSound, samplesSound;
	uint32_t firstChunk = 0, totalSamples = 0, prevChunk = 0, prevSamples = 0, totalBytes = 0;
	uint32_t videoStcoCount, soundStcoCount, stco_reads;
	uint32_t prevSamplesBuff[60];

	uint8_t atombuf[512];
	uint8_t fpsCnt = 0;
	const char fps1Hz[] = "|/-\\";
	char timeStr[20];

	raw_video_typedef raw;

	prepare_range_limit_table2((uint8_t*)progress_circular_bar_16x16x12_buff, &jdinfo);
	build_ycc_rgb_table2((uint8_t*)&progress_circular_bar_16x16x12_buff[1408 / 2], &jdinfo);

	drawBuff_typedef drawBuff;
	_drawBuff = &drawBuff;

	MY_FILE fp_sound, fp_frame, fp_frame_cp, \
			fp_stsc, fp_stsz, fp_stco, \
			fp_sound_stsc, fp_sound_stsz, fp_sound_stco, \
			fp_jpeg, \
			*fp = '\0';

	media.sound.flag.process = 0;
	media.sound.flag.complete = 0;
	media.video.flag.process = 0;
	media.video.flag.complete = 0;

	debug.printf("\r\n\n*** MotionJPEG Debug ***");

	fp = my_fopen(id);
	memcpy((void*)&fp_global, (void*)fp, sizeof(MY_FILE));

	int hasChild = atomHasChild[UDTA];
	atomHasChild[UDTA] = 0;

	debug.printf("\r\n[Atoms]");
	if(collectAtoms(fp, fp->fileSize, 0) != 0){
		debug.printf("\r\nread error file contents.");
		my_fclose(fp);
		LCDStatusStruct.waitExitKey = 0;
		TOUCH_PINIRQ_ENABLE;
		atomHasChild[UDTA] = hasChild;
		return -99;
	}

	atomHasChild[UDTA] = hasChild;

	debug.printf("\r\n\n[Video Sample Tables]");
	debug.printf("\r\nstts:%d", video_stts.numEntry);
	debug.printf("\r\nstsc:%d", video_stsc.numEntry);
	debug.printf("\r\nstsz:%d %d", video_stsz.sampleSize, video_stsz.numEntry);
	debug.printf("\r\nstco:%d", video_stco.numEntry);

	debug.printf("\r\n\n[Sound Sample Tables]");
	debug.printf("\r\nstts:%d", sound_stts.numEntry);
	debug.printf("\r\nstsc:%d", sound_stsc.numEntry);
	debug.printf("\r\nstsz:%d %d", sound_stsz.sampleSize, sound_stsz.numEntry);
	debug.printf("\r\nstco:%d", sound_stco.numEntry);

	debug.printf("\r\n\n[Video Track]");
	debug.printf("\r\nformat:%s", media.video.videoFmtString);
	debug.printf("\r\ncompression:%s", media.video.videoCmpString);
	debug.printf("\r\nwidth:%d", media.video.width);
	debug.printf("\r\nheight:%d", media.video.height);
	debug.printf("\r\ntimeScale:%d", media.video.timeScale);
	debug.printf("\r\nduration:%d", media.video.duration);
	setStrSec(timeStr, (int)((float)media.video.duration / (float)media.video.timeScale + 0.5f));
	media.video.frameRate = (int16_t)((float)(media.video.timeScale * video_stsz.numEntry) / media.video.duration + 0.5f);
	debug.printf("\r\nframe rate:%d", media.video.frameRate);
	debug.printf("\r\ntime:%s", timeStr);

	debug.printf("\r\n\n[Sound Track]");
	char s[5];
	s[4] = '\0';
	memcpy(s, (void*)media.sound.format.audioFmtString, 4);
	debug.printf("\r\ntype:%s", s);
	debug.printf("\r\nnumChannel:%d", media.sound.format.numChannel);
	debug.printf("\r\nsampleSize:%d", media.sound.format.sampleSize);
	debug.printf("\r\nsampleRate:%d", media.sound.format.sampleRate);
	debug.printf("\r\ntimeScale:%d", media.sound.timeScale);
	debug.printf("\r\nduration:%d", media.sound.duration);
	setStrSec(timeStr, (int)((float)media.sound.duration / (float)media.sound.timeScale + 0.5f));
	debug.printf("\r\ntime:%s", timeStr);

	if(media.video.width > LCD_WIDTH || media.video.height > LCD_HEIGHT){
		debug.printf("\r\ntoo large video dimension size.");
		my_fclose(fp);
		LCDStatusStruct.waitExitKey = 0;
		TOUCH_PINIRQ_ENABLE;
		atomHasChild[UDTA] = hasChild;
		return RET_PLAY_STOP;
	}

	FUNC_VIDEO_BGIMG;
	media.video.startPosX = (LCD_WIDTH - media.video.width) / 2 - 1;
	media.video.startPosY = (LCD_HEIGHT - media.video.height) / 2 - 1;
	media.video.startPosX = media.video.startPosX > 0 ? media.video.startPosX : 0;
	media.video.startPosY = media.video.startPosY > 0 ? media.video.startPosY : 0;
	media.video.height += (media.video.height % 2); // if value is odd number, convert to even

/*
 	// DEBUG SAMPLE TABLES >>
	uint32_t sampleCount, sampleDuration, totalSampleCount, totalSampleDuration, samplesPerChunk, sampleDscId, chunkOffset;
 	MY_FILE fp_tmp;
	memcpy((void*)&fp_tmp, (void*)&video_stts.fp, sizeof(MY_FILE));

	debug.printf("\r\n\n*** Video Info ***");

	// Time to  Sample (stts)

	debug.printf("\r\n\nTimeToSample(stts)");

	for(i = 0;i < video_stts.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleCount = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDuration = getAtomSize(atombuf);


		debug.printf("\r\n%04d:%04x %04x", i, sampleCount, sampleDuration);


		totalSampleCount += sampleCount;
		totalSampleDuration += sampleDuration * sampleCount;
	}
	debug.printf("\r\ntotalSamplesCount:%d totalSampleDuration:%d", totalSampleCount, totalSampleDuration);




	// Samples To Chunk (stsc)

	debug.printf("\r\n\nSamplesToChunk(stsc)");

	memcpy((void*)&fp_tmp, (void*)&video_stsc.fp, sizeof(MY_FILE));

	for(i = 0;i < video_stsc.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		firstChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		samplesPerChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDscId = getAtomSize(atombuf);

		debug.printf("\r\n%02d:%04x %04x %04x", i, firstChunk, samplesPerChunk, sampleDscId);


		totalSamples += (firstChunk - prevChunk) * prevSamples;

		prevChunk = firstChunk;
		prevSamples  = samplesPerChunk;
	}
	totalSamples += prevSamples;

	debug.printf("\r\ntotalSamples:%d", totalSamples);

	// Chunk To Offset (stco)

	debug.printf("\r\n\nChunkToOffset(stco)");

	memcpy((void*)&fp_tmp, (void*)&video_stco.fp, sizeof(MY_FILE));


	for(i = 0;i < video_stco.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		chunkOffset = getAtomSize(atombuf);

		debug.printf("\r\n%04d:%08x", i, chunkOffset);
	}



	debug.printf("\r\n\n*** Sound Info ***");

	totalSampleCount = 0, totalSampleDuration = 0;
	firstChunk = 0, samplesPerChunk = 0, sampleDscId, totalSamples = 0, \
	prevChunk = 0, prevSamples = 0;
	// Time to  Sample (stts)

	memcpy((void*)&fp_tmp, (void*)&sound_stts.fp, sizeof(MY_FILE));

	debug.printf("\r\n\nTimeToSample(stts)");

	for(i = 0;i < sound_stts.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleCount = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDuration = getAtomSize(atombuf);


		debug.printf("\r\n%04d:%04x %04x", i, sampleCount, sampleDuration);


		totalSampleCount += sampleCount;
		totalSampleDuration += sampleDuration * sampleCount;
	}
	debug.printf("\r\ntotalSamplesCount:%d totalSampleDuration:%d", totalSampleCount, totalSampleDuration);




	// Samples To Chunk (stsc)

	debug.printf("\r\n\nSamplesToChunk(stsc)");

	memcpy((void*)&fp_tmp, (void*)&sound_stsc.fp, sizeof(MY_FILE));

	for(i = 0;i < sound_stsc.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		firstChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		samplesPerChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDscId = getAtomSize(atombuf);

		debug.printf("\r\n%02d:%04x %04x %04x", i, firstChunk, samplesPerChunk, sampleDscId);


		totalSamples += (firstChunk - prevChunk) * prevSamples;

		prevChunk = firstChunk;
		prevSamples  = samplesPerChunk;
	}
	totalSamples += prevSamples;

	debug.printf("\r\ntotalSamples:%d", totalSamples);

	// Chunk To Offset (stco)

	debug.printf("\r\n\nChunkToOffset(stco)");

	memcpy((void*)&fp_tmp, (void*)&sound_stco.fp, sizeof(MY_FILE));


	for(i = 0;i < sound_stco.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		chunkOffset = getAtomSize(atombuf);

		debug.printf("\r\n%04d:%08x", i, chunkOffset);
	}
	// << DEBUG SAMPLE TABLES
*/
	/*** MotionJPEG Play Process ***/
	debug.printf("\r\n\n[Play]\n");

	my_fseek(fp, 0, SEEK_SET);

	memcpy((void*)&fp_frame, (void*)fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsz, (void*)&video_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsc, (void*)&video_stsc.fp, sizeof(MY_FILE));
	numEntry = video_stsc.numEntry;

	/* Initialize the JPEG decompression object with default error handling. */
	jpeg_read.buf_type = 1; // jpeg file stream as internal sram
	jdinfo.useMergedUpsampling = 1;
	jdinfo.err = jpeg_std_error(&jerr);

	fps = frames = prevFrames = 0;
	totalSamples = firstChunk = prevChunk = prevSamples = 0;

	putCharTmp = LCD_FUNC.putChar;
	putWideCharTmp = LCD_FUNC.putWideChar;

	if(!pcf_font.c_loaded){
		LCD_FUNC.putChar = PCFPutCharCache;
		LCD_FUNC.putWideChar = PCFPutCharCache;

		PCFSetGlyphCacheStartAddress((void*)cursorRAM);
		PCFCachePlayTimeGlyphs(12);
	} else {
		LCD_FUNC.putChar = C_PCFPutChar;
		LCD_FUNC.putWideChar = C_PCFPutChar;
	}

	if(abs(video_stco.numEntry - sound_stco.numEntry) > 50){ // not interleaved correctly
		debug.printf("\r\nError!! this is not an interleaved media.");
		debug.printf("\r\nTry command:");
		debug.printf("\r\nMP4Box -inter 500 this.mov");

		LCD_FUNC.putChar = putCharTmp;
		LCD_FUNC.putWideChar = putWideCharTmp;

		LCDDrawSquare(0, 0, 320, 13, BLACK);
		LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);
		LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);
		LCDGotoXY(0, 0);
		LCDPutString("Error!!  this  is  not  an  interleaved  media.\n", RED);
		LCDPutString("Try  command:\n", RED);
		LCDPutString("MP4Box  -inter  500  this.mov", RED);
		Delay(5000);
		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) != Bit_RESET); // タッチパネルが押されたか？

		goto EXIT_PROCESS;
	} else {
		prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
		prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk
		samples = firstChunk - prevChunk;
	}

	frameDuration = getVideoSampleTime(atombuf, totalSamples);

	// SOUND
	memcpy((void*)&fp_sound_stsz, (void*)&sound_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound_stco, (void*)&sound_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound_stsc, (void*)&sound_stsc.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound, (void*)fp, sizeof(MY_FILE));

	prevChunkSound = getSampleSize(atombuf, 12, &fp_sound_stsc); // firstChunk samplesPerChunk sampleDescriptionID　一つ目のfirstChunkをprevChunkに
	prevSamplesSound = (getAtomSize(&atombuf[4]) / 100) * 100; // 一つ目のsamplesPerChunkをprevSamplesに サウンドバッファの半端がでないようにする
	firstChunkSound = getSampleSize(atombuf, 4, &fp_sound_stsc); // 二つ目のfirstChunk

	samplesSound = (firstChunkSound - prevChunkSound) * prevSamplesSound;

	uint8_t SOUND_BUFFER[38400];
	uint16_t soundSampleByte = media.sound.format.sampleSize / 8;
	uint32_t soundSampleBlocks = soundSampleByte * media.sound.format.numChannel;

	float timeScaleCoeff = (1.0f / media.video.timeScale) * 100000;

	dac_intr.fp = &fp_sound;
	dac_intr.buff = SOUND_BUFFER;
	dac_intr.bufferSize = ((media.sound.format.sampleRate / 10) * 2) * soundSampleByte * media.sound.format.numChannel;
	if(media.sound.format.sampleSize == 16){
		dac_intr.func = DAC_Buffer_Process_Stereo_S16bit;
	} else {
		dac_intr.func = DAC_Buffer_Process_Mono_U8bit;
	}

	memset(dac_intr.buff, 0, sizeof(SOUND_BUFFER));

	my_fseek(&fp_sound, getSampleSize(atombuf, 4, &fp_sound_stco), SEEK_SET);

	dac_intr.sound_reads = 0;
	stco_reads = 1;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* TIM3 サンプルタイム用 0.01ms秒単位で指定 */
	TIM_TimeBaseInitStructure.TIM_Period = (100000 * frameDuration) / media.video.timeScale - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 2) / 100000 - 1; // 0.01ms
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	TIM_Cmd(TIM3, ENABLE);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* 1s = 1 / F_CPU(168MHz) * (167 + 1) * (99 + 1) * (9999 + 1)  TIM1フレームレート計測用  */
	TIM_TimeBaseInitStructure.TIM_Period = 9999;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	TIM_Cmd(TIM1, ENABLE);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	// Video
	pv_src.firstChunk = &firstChunk;
	pv_src.prevChunk = &prevChunk;
	pv_src.prevSamples = &prevSamples;
	pv_src.samples = &samples;
	pv_src.totalSamples = &totalSamples;
	pv_src.videoStcoCount = &videoStcoCount;

	pv_src.fp_video_stsc = &fp_stsc;
	pv_src.fp_video_stsz = &fp_stsz;
	pv_src.fp_video_stco = &fp_stco;
	pv_src.fp_frame = &fp_frame;

	// Sound
	ps_src.firstChunk = &firstChunkSound;
	ps_src.prevChunk = &prevChunkSound;
	ps_src.prevSamples = &prevSamplesSound;
	ps_src.samples = &samplesSound;
	ps_src.soundStcoCount = &soundStcoCount;

	ps_src.fp_sound_stsc = &fp_sound_stsc;
	ps_src.fp_sound_stsz = &fp_sound_stsz;
	ps_src.fp_sound_stco = &fp_sound_stco;

	mjpeg_touch.resynch = 0;
	LCDSetWindowArea(media.video.startPosX, media.video.startPosY, media.video.width, media.video.height);

	float limitter;

	switch(SystemCoreClock){
	case 168000000:
		limitter = 0.91f;
		break;
	case 200000000:
		limitter = 0.93f;
		break;
	case 240000000:
		limitter = 0.96f;
		break;
	case 250000000:
		limitter = 0.98f;
		break;
	default:
		limitter = 0.8f;
		break;
	}

	videoStcoCount = 0, soundStcoCount = 0;
	int soundEndFlag = 0;

	// Enable DMA1_Stream1 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	SOUNDInitDAC(media.sound.format.sampleRate);
	if(media.sound.format.sampleSize == 8){
		SOUNDDMAConf((void*)&DAC->DHR8RD, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel);
	} else {
		SOUNDDMAConf((void*)&DAC->DHR12LD, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel);
	}

	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
	DMA_Cmd(DMA1_Stream1, ENABLE);

	while(1){
		CHUNK_OFFSET_HEAD:
		for(j = 0;j < samples;j++){

			my_fseek(&fp_frame, getSampleSize(atombuf, 4, &fp_stco), SEEK_SET);
			if(media.video.playJpeg){
				my_fread(prevSamplesBuff, 1, prevSamples * 4, &fp_stsz);
			}

			for(i = 0;i < prevSamples;i++){

				sample_time_limit = TIM3->ARR * limitter;

				frameDuration = getVideoSampleTime(atombuf, ++totalSamples); // get next frame duration

				LCDSetGramAddr(media.video.startPosX, media.video.startPosY);
				LCDPutCmd(0x0022);

				if(media.video.playJpeg){ // Jpeg compressed media
					fp_jpeg.seekBytes = 0;
					fp_jpeg.fileSize = getAtomSize(&prevSamplesBuff[i]);

					my_fread((void*)CCM_BASE, 1, fp_jpeg.fileSize, &fp_frame);
					jpeg_read.frame_size = fp_jpeg.fileSize;

					totalBytes += fp_jpeg.fileSize;

					mpool_struct.mem_seek = 0;
					jpeg_create_decompress(&jdinfo);

					/* Specify data source for decompression */
					jpeg_stdio_src(&jdinfo, &fp_jpeg);

					/* Read file header, set default decompression parameters */
					(void) jpeg_read_header(&jdinfo, TRUE);

					dest_mgr = jinit_write_ppm(&jdinfo);

					/* Start decompressor */
					(void) jpeg_start_decompress(&jdinfo);
				} else { // Uncompress media
					raw.output_scanline = 0;
					raw.frame_size = media.video.width * media.video.height * sizeof(uint16_t);
					raw.rasters = 50;
					raw.buf_size = raw.rasters * media.video.width * sizeof(uint16_t);
					memcpy((void*)&fp_frame_cp, (void*)&fp_frame, sizeof(MY_FILE));
					my_fseek(&fp_frame, raw.frame_size, SEEK_CUR);
					totalBytes += raw.frame_size;
				}


				DMA_SOUND_IT_ENABLE; // Enable DAC interrupt
				while(!TIM3_SR_UIF_BB){ // while TIM3->SR Update Flag is unset
					if(media.video.playJpeg){
						if((jdinfo.output_scanline < jdinfo.output_height) && (TIM3->CNT < sample_time_limit)){ // JPEG draw rasters
							jpeg_read_scanlines(&jdinfo, dest_mgr->buffer, dest_mgr->buffer_height);
						}
					} else if ((raw.output_scanline < media.video.height) && (TIM3->CNT < sample_time_limit)){ // Uncompress draw rasters
						if(raw.frame_size < raw.buf_size){
							raw.buf_size = raw.frame_size;
						}
						DMA_SOUND_IT_DISABLE;
						my_fread((void*)mempool, 1, raw.buf_size, &fp_frame_cp);
						DMA_SOUND_IT_ENABLE;
						raw.p_raster = (uint16_t*)mempool;
						for(k = 0;k < (raw.buf_size >> 1);k++){
							LCD->RAM = *raw.p_raster++;
						}
						raw.frame_size = raw.frame_size - raw.buf_size;
						raw.output_scanline = raw.output_scanline + raw.rasters;
					}
					if((abs(soundStcoCount - videoStcoCount) > 1) && !soundEndFlag){ //　correct synch unmatch
						if(soundStcoCount >= (sound_stco.numEntry - 2) || videoStcoCount >= (video_stco.numEntry - 2)){
							goto END_PROCESS;
						}
						mjpeg_touch.resynch = 1;
						mjpeg_touch.resynch_entry = soundStcoCount > videoStcoCount ? videoStcoCount : soundStcoCount;
						debug.printf("\r\n*synch unmatch at video_stco:%d sound_stco:%d\n", videoStcoCount, soundStcoCount);
						DMA_SOUND_IT_DISABLE; // Disable DAC interrupt
						mjpegTouch();
						samples /= prevSamples;
						mjpeg_touch.resynch = 0;
						getVideoSampleTime(atombuf, 0); // reset sample time
						getVideoSampleTime(atombuf, totalSamples); // get next sample time
						dac_intr.sound_reads = prevSamplesSound * soundSampleBlocks; // fill DAC buffer
						videoStcoCount -= 2, soundStcoCount -= 2;
						goto CHUNK_OFFSET_HEAD;
					}
					if(dac_intr.sound_reads >= (prevSamplesSound * soundSampleBlocks)){
						if(++soundStcoCount < sound_stco.numEntry){
							soundEndFlag = 0;

							totalBytes += dac_intr.sound_reads;

							my_fseek(dac_intr.fp, getSampleSize(atombuf, 4, &fp_sound_stco), SEEK_SET);

							dac_intr.sound_reads = 0;
							if(++stco_reads > samplesSound){
								stco_reads = 0;
								prevChunkSound = firstChunkSound; // 今回のfirstChunkをprevChunkに
								prevSamplesSound = getSampleSize(atombuf, 12, &fp_sound_stsc); // samplesPerChunk sampleDescriptionID
								firstChunkSound = getAtomSize(&atombuf[8]); // 次のfirstChunk
								samplesSound = firstChunkSound - prevChunkSound; // 次回再生チャンクのサンプル数
							}
						} else {
							soundEndFlag = 1;
							dac_intr.sound_reads = 0;
							DMA_SOUND_IT_DISABLE;
							DMA_Cmd(DMA1_Stream1, DISABLE);
						}
					}
					if(TP_PEN_INPUT_BB == Bit_RESET){ // タッチパネルが押されたか？
						ret = mjpegPause();
						LCD_FUNC.putChar = PCFPutCharCache;
						LCD_FUNC.putWideChar = PCFPutCharCache;
						if(ret == RET_PLAY_STOP || ret == RET_PLAY_NEXT || ret == RET_PLAY_PREV){
							goto END_PROCESS;
						}
						if(ret == 1){ // 一時停止処理へ飛ぶ 返り値:0 そのまま復帰 :1 移動先のサンプルタイムを取得
							samples /= prevSamples;
							getVideoSampleTime(atombuf, 0); // サンプルタイム初期化
							getVideoSampleTime(atombuf, totalSamples); // 移動先のサンプルタイム取得
							dac_intr.sound_reads = prevSamplesSound * soundSampleBlocks; // DAC読み込みバッファ数を埋めておく
							videoStcoCount -= 2, soundStcoCount -= 2;
							ret = 0;
							goto CHUNK_OFFSET_HEAD;
						}
					}
				}
				DMA_SOUND_IT_DISABLE; // DAC割込み不許可

				// フレームあたりのタイムデュレーションタイマ(1/100ms単位で指定)
				TIM3->ARR = frameDuration * timeScaleCoeff - 1;
				TIM3->CR1 = 0;
				TIM3->CNT = 0; // clear counter
				TIM3_SR_UIF_BB = 0; // clear update flag
				TIM3_DIER_UIE_BB = 1; // set update interrupt
				TIM3_CR1_CEN_BB = 1; // enable tim3

				frames++;

				if(TIM1_SR_UIF_BB){ // フレームレート表示用
					TIM1_SR_UIF_BB = 0;
					fps = frames - prevFrames;
					debug.printf("\r%c%dfps %dkbps v:%d s:%d  ", fps1Hz[fpsCnt++ & 3], fps, (int)((float)(totalBytes * 8) * 0.001f), videoStcoCount, soundStcoCount);
					prevFrames = frames;
					totalBytes = 0;
				}
				if(!LCDStatusStruct.waitExitKey){
					ret = RET_PLAY_STOP;
					goto END_PROCESS; // play abort
				}
			}
			AUDIO_OUT_ENABLE;
			if(++videoStcoCount >= video_stco.numEntry){// || soundStcoCount >= (sound_stco.numEntry)){
				goto END_PROCESS; // ビデオチャンクカウントが最後までいったら再生終了
			}
		}
		prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
		prevSamples = getSampleSize(atombuf, 12, &fp_stsc); // samplesPerChunk sampleDescriptionID
		firstChunk = getAtomSize(&atombuf[8]); // 次のfirstChunk
		samples = firstChunk - prevChunk; // 次回再生チャンクのサンプル数
	}

	END_PROCESS: // 再生終了処理
	AUDIO_OUT_SHUTDOWN;
	debug.printf("\r\ntotal_samples:%d video_stco_count:%d sound_stco_count:%d", totalSamples, videoStcoCount, soundStcoCount);
//	debug.printf("\r\ntotalRasters:%d", totalRasters);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
	DMA_Cmd(DMA1_Stream1, DISABLE);
	DMA_DeInit(DMA1_Stream1);

	if(media.video.playJpeg){
		(void) jpeg_finish_decompress(&jdinfo);
		jpeg_destroy_decompress(&jdinfo);
	}

	EXIT_PROCESS: //
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	jpeg_read.buf_type = 0;
	dac_intr.func = '\0';
	my_fclose(fp);

	LCD_FUNC.putChar = putCharTmp;
	LCD_FUNC.putWideChar = putWideCharTmp;

	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);

	LCDStatusStruct.waitExitKey = 0;
	TOUCH_PINIRQ_ENABLE;
	LCDBackLightTimerInit();

	return ret;
}

