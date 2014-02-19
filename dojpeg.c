#include "stm32f4xx_conf.h"

#include "dojpeg.h"
#include "lcd.h"
#include "fat.h"
#include "usart.h"
#include "xpt2046.h"
#include "settings.h"

#include "sound.h"

#include "jerror.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */

#include <ctype.h>		/* to declare isprint() */

#include "mpool.h"

typedef struct{
	int x, y, width, height;
	uint16_t p[30 * 30];
}pic_arrow_buf_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[30 * 30];
}pic_pref_buf_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[40 * 40];
}pic_play_icon_buf;

typedef struct{
	int x, y, width, height;
	uint16_t p[100 * 24];
}copy_image_icon_buf;

int dojpeg(int id, uint8_t djpeg_arrows, uint8_t arrow_clicked)
{
	TouchPenIRQ_Disable();
	touch.func = touch_empty_func;
	register int i, t;
	struct jpeg_decompress_struct jdinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;
	int touch_flag = 0, ret, copy_image_to;
	extern settings_group_typedef settings_group;
	uint8_t time2sleep_cp = settings_group.disp_conf.time2sleep;

	if(time2sleep_cp){ // prevent to sleep during photo frame process
		settings_group.disp_conf.time2sleep = 0;
	}

	pic_arrow_buf_typedef pic_left_arrow, pic_right_arrow;
	pic_pref_buf_typedef pic_pref;
	pic_play_icon_buf pic_abort_icon, pic_play_icon;
	copy_image_icon_buf copy_image_icon, copy_image_filer, copy_image_music;

	MY_FILE *input_file = '\0';

	debug.printf("\r\n");

	create_mpool();

	/* Initialize the JPEG decompression object with default error handling. */
	jdinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jdinfo);

	jdinfo.useMergedUpsampling = FALSE;
	jdinfo.dct_method = JDCT_ISLOW;
//	jdinfo.do_block_smoothing = TRUE;
	jdinfo.dither_mode = JDITHER_FS;
	jdinfo.two_pass_quantize = TRUE;

	input_file = my_fopen(id);

	jdinfo.mem->max_memory_to_use = 30000L;

	/* Specify data source for decompression */
	jpeg_stdio_src(&jdinfo, input_file);

	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&jdinfo, TRUE);

	if(jdinfo.err->msg_code == JERR_NO_IMAGE){
		debug.printf("\r\nJPEG datastream contains no image");
		ret = -1;
		LCDStatusStruct.waitExitKey = 0;
		goto END_PROCESS;
	}
	if(jdinfo.progressive_mode){ // progressivee jpeg not supported
		debug.printf("\r\nprogressive jpeg not supported");
		ret = -1;
		LCDStatusStruct.waitExitKey = 0;
		goto END_PROCESS;
	}

	for(i = 8;i >= 1; i--){
		if((jdinfo.image_width * (float)i / 8.0f) <= LCD_WIDTH && \
				(jdinfo.image_height * (float)i / 8.0f) <= LCD_HEIGHT){
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

	int down_scale = 1, x, y, width, height;

	width = jdinfo.output_width;
	height = jdinfo.output_height;

	while(1){
		if(width <= LCD_WIDTH && height <= LCD_HEIGHT){
			break;
		}
		down_scale *= 2;
		width = (int)((float)jdinfo.output_width / (float)down_scale + 0.5f);
		height = (int)((float)jdinfo.output_height / (float)down_scale + 0.5f);
	}

	x = (LCD_WIDTH - width) / 2 - 1;
	y =  (LCD_HEIGHT - height) / 2 - 1;
	x = x > 0 ? x : 0;
	y = y > 0 ? y : 0;

#ifdef MY_DEBUG
	debug.printf("\r\nx:%d y:%d width:%d height:%d", x, y, width, height);
#endif

	debug.printf("\r\nsrc_size:%dx%d", jdinfo.image_width, jdinfo.image_height);
	debug.printf("\r\ndst_size:%dx%d", width, height);
	debug.printf("\r\nscale_num:%d scale_denom:%d extra_down_scale:%d", jdinfo.scale_num, jdinfo.scale_denom, down_scale);
	debug.printf("\r\nscale:%d/%d", jdinfo.scale_num,  jdinfo.scale_denom * down_scale);

//	LCDCheckPattern();
	LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);

	LCDSetWindowArea(x, y, width, height);
	LCDSetGramAddr(x, y);
	LCDPutCmd(0x0022);

	/* Process data */
	t = 0;
	while(jdinfo.output_scanline < jdinfo.output_height){
		jpeg_read_scanlines(&jdinfo, dest_mgr->buffer, dest_mgr->buffer_height);
		if(!(t++ & (down_scale - 1))){
			for(i = 0;i < dest->buffer_width;i += 3 * down_scale){
				LCD->RAM = (dest->pixrow[i + 2] >> 3) << 11 | (dest->pixrow[i + 1] >> 2) << 5 |  dest->pixrow[i] >> 3;
			}
		}
	}

	/* Finish decompression and release memory.
	 * I must do it in this order because output module has allocated memory
	 * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
	 */
//	if(!jpeg_finish_output(&jdinfo)){	/* terminate output pass */
//		USARTPutString("\r\njpeg_finish_output:suspend");
//	}

	(void) jpeg_finish_decompress(&jdinfo);
	ret = 0;

END_PROCESS:
	jpeg_destroy_decompress(&jdinfo);

	LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);

	if(djpeg_arrows & DJPEG_ARROW_LEFT){
		pic_left_arrow.x = 20;
		pic_left_arrow.y = 106;
		pic_left_arrow.width = 30;
		pic_left_arrow.height = 30;
		LCDStoreBgImgToBuff(pic_left_arrow.x, pic_left_arrow.y, pic_left_arrow.width, pic_left_arrow.height, pic_left_arrow.p);
	}

	if(djpeg_arrows & DJPEG_ARROW_RIGHT){
		pic_right_arrow.x = 269;
		pic_right_arrow.y = 106;
		pic_right_arrow.width = 30;
		pic_right_arrow.height = 30;
		LCDStoreBgImgToBuff(pic_right_arrow.x, pic_right_arrow.y, pic_right_arrow.width, pic_right_arrow.height, pic_right_arrow.p);
	}

	pic_pref.x = 267;
	pic_pref.y = 25;
	pic_pref.width = 30;
	pic_pref.height = 30;
	LCDStoreBgImgToBuff(pic_pref.x, pic_pref.y, pic_pref.width, pic_pref.height, pic_pref.p);

	pic_abort_icon.x = 20;
	pic_abort_icon.y = 20;
	pic_abort_icon.width = 40;
	pic_abort_icon.height = 40;
	LCDStoreBgImgToBuff(pic_abort_icon.x, pic_abort_icon.y, pic_abort_icon.width, pic_abort_icon.height, pic_abort_icon.p);

	pic_play_icon.x = 65;
	pic_play_icon.y = 20;
	pic_play_icon.width = 40;
	pic_play_icon.height = 40;
	LCDStoreBgImgToBuff(pic_play_icon.x, pic_play_icon.y, pic_play_icon.width, pic_play_icon.height, pic_play_icon.p);

	copy_image_icon.x = 109;
	copy_image_icon.y = 20;
	copy_image_icon.width = 100;
	copy_image_icon.height = 24;
	LCDStoreBgImgToBuff(copy_image_icon.x, copy_image_icon.y, copy_image_icon.width, copy_image_icon.height, copy_image_icon.p);

	copy_image_filer.x = 25;
	copy_image_filer.y = 105;
	copy_image_filer.width = 100;
	copy_image_filer.height = 24;
	LCDStoreBgImgToBuff(copy_image_filer.x, copy_image_filer.y, copy_image_filer.width, copy_image_filer.height, copy_image_filer.p);

	copy_image_music.x = 194;
	copy_image_music.y = 105;
	copy_image_music.width = 100;
	copy_image_music.height = 24;
	LCDStoreBgImgToBuff(copy_image_music.x, copy_image_music.y, copy_image_music.width, copy_image_music.height, copy_image_music.p);



  /* All done. */
#ifdef MY_DEBUG
	if(jerr.num_warnings == EXIT_WARNING){
		USARTPutString("\r\nEXIT_WARNING");
	}else if(jerr.num_warnings == EXIT_SUCCESS){
		USARTPutString("\r\nEXIT_SUCCESS");
	}
	debug.printf("\r\nlast msg:%s", jdinfo.err->jpeg_message_table[jdinfo.err->last_jpeg_message]);
#endif

	my_fclose(input_file);

	TouchPenIRQ_Enable();
	touch.click = 0;
	if(arrow_clicked){
		goto ARROW_CLIKED;
	}

	uint32_t flash_addr;
	uint16_t bgImageBuf[3 * 1024], confBuf[2 * 1024], *p16;
	int flash_status;

	if((djpeg_arrows & DJPEG_PLAY)){
		if(ret == -1){
			ret = DJPEG_PLAY;
			LCDStatusStruct.waitExitKey = 1;
			goto SKIP_PLAY_DELAY;
		}
		ret = DJPEG_PLAY;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

		TIM_DeInit(TIM1);
		TIM_TimeBaseInitStructure.TIM_Period = 9999;
		TIM_TimeBaseInitStructure.TIM_Prescaler = (100 * settings_group.filer_conf.photo_frame_td - 1);
		TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
		TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
		TIM_SetCounter(TIM1, 0);
		TIM_Cmd(TIM1, ENABLE);
		TIM_ClearFlag(TIM1, TIM_FLAG_Update);

		while(!TIM_GetFlagStatus(TIM1, TIM_FLAG_Update)){
			if(touch.click){
				extern volatile int8_t photo_frame_flag;
				photo_frame_flag = 0;
				ret = 0;
				break;
			}
		}
	} else if(time2sleep_cp){
		settings_group.disp_conf.time2sleep = time2sleep_cp;
	}
SKIP_PLAY_DELAY:

	while(!ret && LCDStatusStruct.waitExitKey){
		while(!touch.click){
			if(!LCDStatusStruct.waitExitKey){
				ret = -1;
				goto EXIT_JPEG;
			}
			if(time.flags.stop_mode){
				LCDStatusStruct.waitExitKey = 0;
				break;
			}
		}
		ARROW_CLIKED:
		if(touch_flag ^= 1){
			if(time2sleep_cp){
				settings_group.disp_conf.time2sleep = time2sleep_cp;
			}

			/* Experimental photo frame music
		    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
		    DMA_Cmd(DMA1_Stream1, DISABLE);
		    AUDIO_OUT_SHUTDOWN;
		    */

			LCDPutIcon(pic_abort_icon.x, pic_abort_icon.y, pic_abort_icon.width, pic_abort_icon.height, abort_icon_40x40, play_icon_40x40_alpha); // display abort icon
			LCDPutIcon(pic_play_icon.x, pic_play_icon.y, pic_play_icon.width, pic_play_icon.height, play_icon_40x40, play_icon_40x40_alpha); // display play icon
			LCDPutIcon(pic_pref.x, pic_pref.y, pic_pref.width, pic_pref.height, pic_pref_30x30, pic_pref_30x30_alpha); // display play icon
			if(djpeg_arrows & DJPEG_ARROW_LEFT){
				LCDPutIcon(pic_left_arrow.x, pic_left_arrow.y, pic_left_arrow.width, pic_left_arrow.height, pic_left_arrow_30x30, pic_left_arrow_30x30_alpha);
			}
			if(djpeg_arrows & DJPEG_ARROW_RIGHT){
				LCDPutIcon(pic_right_arrow.x, pic_right_arrow.y, pic_right_arrow.width, pic_right_arrow.height, pic_right_arrow_30x30, pic_right_arrow_30x30_alpha);
			}
		} else {
			LCDPutBuffToBgImg(pic_abort_icon.x, pic_abort_icon.y, pic_abort_icon.width, pic_abort_icon.height, pic_abort_icon.p); // disapear abort icon
			LCDPutBuffToBgImg(pic_play_icon.x, pic_play_icon.y, pic_play_icon.width, pic_play_icon.height, pic_play_icon.p); // disapear play icon
			LCDPutBuffToBgImg(pic_pref.x, pic_pref.y, pic_pref.width, pic_pref.height, pic_pref.p); // disapper pic pref icon

			if(djpeg_arrows & DJPEG_ARROW_LEFT){
				LCDPutBuffToBgImg(pic_left_arrow.x, pic_left_arrow.y, pic_left_arrow.width, pic_left_arrow.height, pic_left_arrow.p);
				if((touch.posX > (pic_left_arrow.x - 10) && touch.posX < (pic_left_arrow.x + pic_left_arrow.width + 10)) && \
						(touch.posY > (pic_left_arrow.y - 10) && touch.posY < (pic_left_arrow.y + pic_left_arrow.height + 10))) // check if left arrow clicked
				{
					ret = DJPEG_ARROW_LEFT;
				}
			}
			if(djpeg_arrows & DJPEG_ARROW_RIGHT){
				LCDPutBuffToBgImg(pic_right_arrow.x, pic_right_arrow.y, pic_right_arrow.width, pic_right_arrow.height, pic_right_arrow.p);
				if((touch.posX > (pic_right_arrow.x - 10) && touch.posX < (pic_right_arrow.x + pic_right_arrow.width + 10)) && \
						(touch.posY > (pic_right_arrow.y - 10) && touch.posY < (pic_right_arrow.y + pic_right_arrow.height + 10))) // check if right arrow clicked
				{
					ret = DJPEG_ARROW_RIGHT;
				}
			}
			if((touch.posX > (pic_play_icon.x - 10) && touch.posX < (pic_play_icon.x + pic_play_icon.width + 10)) && \
					(touch.posY > (pic_play_icon.y - 10) && touch.posY < (pic_play_icon.y + pic_play_icon.height + 10))) // check if play icon clicked
			{
				/* Experimental photo frame music
				if(dac_intr.sound_reads > 0){
				    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, ENABLE);
				    DMA_Cmd(DMA1_Stream1, ENABLE);
				}
				*/
				ret = DJPEG_PLAY;
			}

			if((touch.posX > (pic_pref.x - 10) && touch.posX < (pic_pref.x + pic_pref.width + 10)) && \
					(touch.posY > (pic_pref.y - 10) && touch.posY < (pic_pref.y + pic_pref.height + 10))) // check if pref icon clicked
			{
				LCDPutIcon(pic_abort_icon.x, pic_abort_icon.y, pic_abort_icon.width, pic_abort_icon.height, abort_icon_40x40, play_icon_40x40_alpha); // display abort icon
				LCDPutIcon(copy_image_icon.x, copy_image_icon.y, copy_image_icon.width, copy_image_icon.height, copy_image_to_100x24, copy_image_to_100x24_alpha);
				LCDPutIcon(copy_image_filer.x, copy_image_filer.y, copy_image_filer.width, copy_image_filer.height, copy_image_to_filer_100x24, copy_image_to_100x24_alpha);
				LCDPutIcon(copy_image_music.x, copy_image_music.y, copy_image_music.width, copy_image_music.height, copy_image_to_music_100x24, copy_image_to_100x24_alpha);

				while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET);
				touch.click = 0;

				while(!touch.click);
				LCDPutBuffToBgImg(pic_abort_icon.x, pic_abort_icon.y, pic_abort_icon.width, pic_abort_icon.height, pic_abort_icon.p); // disapear abort icon
				LCDPutBuffToBgImg(copy_image_icon.x, copy_image_icon.y, copy_image_icon.width, copy_image_icon.height, copy_image_icon.p);
				LCDPutBuffToBgImg(copy_image_filer.x, copy_image_filer.y, copy_image_filer.width, copy_image_filer.height, copy_image_filer.p);
				LCDPutBuffToBgImg(copy_image_music.x, copy_image_music.y, copy_image_music.width, copy_image_music.height, copy_image_music.p);

				copy_image_to = 0;
				if((touch.posX > (copy_image_filer.x - 10) && touch.posX < (copy_image_filer.x + copy_image_filer.width + 10)) && \
						(touch.posY > (copy_image_filer.y - 10) && touch.posY < (copy_image_filer.y + copy_image_filer.height + 10))) // check if filer icon clicked
				{
					copy_image_to = 1;
				}
				if((touch.posX > (copy_image_music.x - 10) && touch.posX < (copy_image_music.x + copy_image_music.width + 10)) && \
						(touch.posY > (copy_image_music.y - 10) && touch.posY < (copy_image_music.y + copy_image_music.height + 10))) // check if music icon clicked
				{
					copy_image_to = 2;
				}
				if(copy_image_to == 1){

					p16 = (uint16_t*)(FLASH_BASE + (16 + 6) * 1024); // Backup Sector #1 + 6KB offset
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){
						bgImageBuf[i] = *p16++;
					}

					p16 = (uint16_t*)(FLASH_BASE + (16 + 12) * 1024); // Backup Sector #1 configure offset
					for(i = 0;i < (4 * 1024 / sizeof(uint16_t));i++){
						confBuf[i] = *p16++;
					}

					debug.printf("\r\nstart flash");
					FLASH_Unlock();
					FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

					flash_status = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_1:%d", flash_status);
					flash_status = FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_2:%d", flash_status);
					flash_status = FLASH_EraseSector(FLASH_Sector_10, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_10:%d", flash_status);

					flash_addr = (FLASH_BASE + (16 + 6) * 1024); // Backup Sector #1 + 6KB offset
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){
						flash_status = FLASH_ProgramHalfWord(flash_addr, bgImageBuf[i]);
						flash_addr += sizeof(uint16_t);
					}

					flash_addr = (FLASH_BASE + (16 + 12) * 1024); // Backto Sector #1 configure offset
					for(i = 0;i < (4 * 1024 / sizeof(uint16_t));i++){
						flash_status = FLASH_ProgramHalfWord(flash_addr, confBuf[i]);
						flash_addr += sizeof(uint16_t);
					}


					LCDSetGramAddr(0, 0);
					LCDPutCmd(0x0022);
					LCD->RAM;
					flash_addr = (FLASH_BASE + 16 * 1024); // Sector #1
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){ // 1st 6KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					flash_addr = (FLASH_BASE + (16 + 16) * 1024); // Sector #2
					for(i = 0;i < (16 * 1024 / sizeof(uint16_t));i++){ // 2nd 16KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					flash_addr = (0x080C0000); // Sector #10
					for(i = 0;i < (128 * 1024 / sizeof(uint16_t));i++){ // 3rd 128KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					debug.printf("\r\nend flash");
					touch_flag = 0;
					goto ARROW_CLIKED;
				} else if(copy_image_to == 2){

					p16 = (uint16_t*)(FLASH_BASE + (16 * 1024)); // Backup Sector #1 offset
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){
						bgImageBuf[i] = *p16++;
					}

					p16 = (uint16_t*)(FLASH_BASE + (16 + 12) * 1024); // Backup Sector #1 configure offset
					for(i = 0;i < (4 * 1024 / sizeof(uint16_t));i++){
						confBuf[i] = *p16++;
					}

					debug.printf("\r\nstart flash");
					FLASH_Unlock();
					FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

					flash_status = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_1:%d", flash_status);
					flash_status = FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_3:%d", flash_status);
					flash_status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
					debug.printf("\r\nFLASH_Sector_11:%d", flash_status);

					flash_addr = (FLASH_BASE + (16 * 1024)); // Backto Sector #1 offset
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){
						flash_status = FLASH_ProgramHalfWord(flash_addr, bgImageBuf[i]);
						flash_addr += sizeof(uint16_t);
					}

					flash_addr = (FLASH_BASE + (16 + 12) * 1024); // Backto Sector #1 configure offset
					for(i = 0;i < (4 * 1024 / sizeof(uint16_t));i++){
						flash_status = FLASH_ProgramHalfWord(flash_addr, confBuf[i]);
						flash_addr += sizeof(uint16_t);
					}


					LCDSetGramAddr(0, 0);
					LCDPutCmd(0x0022);
					LCD->RAM;
					flash_addr = (FLASH_BASE + (16 + 6) * 1024); // Sector #1 + 6KB offset
					for(i = 0;i < (6 * 1024 / sizeof(uint16_t));i++){ // 1st 6KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					flash_addr = (FLASH_BASE + (16 + 16 + 16) * 1024); // Sector #3
					for(i = 0;i < (16 * 1024 / sizeof(uint16_t));i++){ // 2nd 16KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					flash_addr = (0x080E0000); // Sector #11
					for(i = 0;i < (128 * 1024 / sizeof(uint16_t));i++){ // 3rd 128KB
						flash_status = FLASH_ProgramHalfWord(flash_addr, LCD->RAM);
						flash_addr += sizeof(uint16_t);
					}
					debug.printf("\r\nend flash");
					touch_flag = 0;
					goto ARROW_CLIKED;
				}
			}
			if((touch.posX > pic_abort_icon.x && touch.posX < (pic_abort_icon.x + pic_abort_icon.width)) && \
					(touch.posY > pic_abort_icon.y && touch.posY < (pic_abort_icon.y + pic_abort_icon.height))) // check if abort icon clicked
			{
				ret = -1;
				LCDStatusStruct.waitExitKey = 0;
			}
		}
		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET);
		touch.click = 0;
	}
	EXIT_JPEG:
	touch.func = LCDTouchPoint;

	if(time2sleep_cp){
		settings_group.disp_conf.time2sleep = time2sleep_cp;
	}

	return ret;			/* suppress no-return-value warnings */
}
