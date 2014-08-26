/*
 *		STM32F4-Discovery Motion Player Project
 *		by Tonsuke
 *
 *		v1.12
 *		2014/03/07
 *
 *		WIKI
 *		http://motionplayer.wiki.fc2.com
 *
 *		Git Source
 *		https://code.google.com/p/motion-player-project/
 */

#include "stm32f4xx_conf.h"
#include "fat.h"
#include "sd.h"
#include "lcd.h"
#include "dojpeg.h"
#include "mjpeg.h"
#include "aac.h"
#include "mp3.h"
#include "sound.h"
#include "pcf_font.h"
#include "usart.h"
#include "xmodem.h"
#include "xpt2046.h"
#include "icon.h"
#include "settings.h"
#include "mpool.h"

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"

USB_OTG_CORE_HANDLE *pUSB_OTG_dev;
volatile int8_t usb_msc_enable = 0, usb_msc_card_accessing = 0, usb_msc_progressbar_enable = 0, photo_frame_flag = 0;

void TIM8_UP_TIM13_IRQHandler(void) // Back Light & Sleep control timer
{
	if(TIM_GetITStatus(TIM8, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
		if(usb_msc_enable){
			if(usb_msc_card_accessing){
				if(usb_msc_progressbar_enable){
					TIM_Cmd(TIM1, ENABLE); // Start displaying progress bar
				}
	    		usb_msc_progressbar_enable = 1;
			} else {
				TIM_Cmd(TIM1, DISABLE); // Stop displaying progress bar
				int i;
				for(i = 0;i < 16 * 16;i++){ // Clear progress bar drawings
					LCDPutData(colorc[BLACK]);
				}
				usb_msc_progressbar_enable = 0;
			}
			usb_msc_card_accessing = 0;
			return;
		}

		if(settings_group.disp_conf.time2sleep == 0){ // Always On
			return;
		}

		++time.curTime;

		if((abs(time.curTime - time.prevTime) >= (settings_group.disp_conf.time2sleep / 2)) && !time.flags.dimLight && time.flags.enable){ // Dim Light
			TIM_SetCompare2(TIM4, (int)(500 * (float)settings_group.disp_conf.brightness / 100.0f) - 1);
			time.flags.dimLight = 1;
		}

		if(abs(time.curTime - time.prevTime) >= settings_group.disp_conf.time2sleep && time.flags.dimLight && time.flags.enable){ // Sleep Enable
			time.flags.stop_mode = 1;
		}
	}
}

void MassStorage()
{
	settings_group.disp_conf.time2sleep = 0;
	TOUCH_PINIRQ_DISABLE;
	touch.func = touch_empty_func;

	LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);
	LCDPutIcon(75, 95, 22, 22, usb_22x22, usb_22x22_alpha);
	LCDGotoXY(100, 100);
	LCDPutString("USB Connect to HOST", WHITE);

	// show circular progress bar animation until file list complete
	MergeCircularProgressBar(0);
	LCDSetWindowArea(147, 150, 16, 16);
	LCDSetGramAddr(147, 150);
	LCDPutCmd(0x0022);
	DMA_ProgressBar_Conf();

	SystemInit2(168);
	SystemCoreClockUpdate();
	USARTInit();

	USB_OTG_CORE_HANDLE USB_OTG_dev;
	pUSB_OTG_dev = &USB_OTG_dev;

    USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_MSC_cb,
              &USR_cb);

    while(1){};
}

void mem_clean(){
	void *p;
	p = malloc(sizeof(mempool));
	memset(p, 0, sizeof(mempool));
	free(p);
}

int main(void)
{
	char extensionName[4];
	int8_t ret, prevRet = 0, djpeg_arrows, arrow_clicked;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	do{
		SYSCFG_CompensationCellCmd(ENABLE);
	}while(SYSCFG_GetCompensationCellStatus() == RESET);

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    SOUNDInitDAC(0);

	mem_clean();

    SETTINGS_Init();

    USARTInit();

    if(C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font) != -1){
    	debug.printf("\r\ninternal flash font loaded.");
    	LCD_FUNC.putChar = C_PCFPutChar;
    	LCD_FUNC.putWideChar = C_PCFPutChar;
    	LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;
    } else {
    	debug.printf("\r\ninternal flash font load failed.");
    }


	debug.printf("\r\nMCU_IDCODE:%08x", *(uint32_t*)0xE0042000);
	uint32_t mcu_revision = *(uint32_t*)0xE0042000 & 0xffff0000;
	debug.printf("\r\nRevision: ");
	switch(mcu_revision){
	case 0x10000000:
		debug.printf("A");
		break;
	case 0x20000000:
		debug.printf("B");
		break;
	case 0x10010000:
		debug.printf("Z");
		break;
	default:
		break;
	}

    LCDInit();
    XPT2046Init();
    SDInit();

	if(!initFat()){
		const char fat_init_succeeded_str[] = "\r\nFat initialization succeeded.";
		debug.printf(fat_init_succeeded_str);
		LCDPutString(fat_init_succeeded_str, WHITE);
	} else {
		const char fat_init_failed_str[] = "\r\nFat initialization failed.";
		debug.printf(fat_init_failed_str);
		LCDPutString(fat_init_failed_str, WHITE);

		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)){
			debug.printf("\r\nno card inserted.");
			LCDPutString("\r\nno card inserted.", WHITE);
		}

		TouchPenIRQ_Enable();
		TouchPenReleaseIRQ_Enable();

		settings_group.disp_conf.time2sleep = 10;

		LCDBackLightTimerInit();

		while(1){
			if(time.flags.stop_mode){
				LCDBackLightDisable();
				PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
				SystemInit();
				SystemCoreClockUpdate();
				LCDBackLightInit();
				time.flags.stop_mode = 0;
			}
		}
	}
	if(fat.fsType == FS_TYPE_FAT16){
		debug.printf("\r\nFileSystem:FAT16");
	} else if (fat.fsType == FS_TYPE_FAT32){
		debug.printf("\r\nFileSystem:FAT32");
	}

	debug.printf("\r\nCluster Size:%dKB", (fat.sectorsPerCluster * 512) / 1024);

	USARTPutString("\r\nStarting...");

	if(settings_group.filer_conf.fontEnabled){
		if(PCFFontInit(getIdByName("FONT.PCF")) != -1){
	    	debug.printf("\r\nexternal font loaded.");
			LCD_FUNC.putChar = PCFPutChar;
			LCD_FUNC.putWideChar = PCFPutChar;
			LCD_FUNC.getCharLength = PCFGetCharPixelLength;
		} else {
	    	debug.printf("\r\nexternal font load failed.");
		}
	}

	LCDPrintFileList();
	debug.printf("\r\nSystemCoreClock:%dMHz", SystemCoreClock / 1000000);

	TouchPenIRQ_Enable();
	TouchPenReleaseIRQ_Enable();
	navigation_loop_mode = NAV_PLAY_ENTIRE;
	bass_boost_mode = 0;
	reverb_effect_mode = 0;
	vocal_cancel_mode = 0;
	touch.click = 0;

	LCDBackLightTimerInit();

    while(1){
    	if(usb_msc_enable){ // Start Mass Storage Mode
    		MassStorage();
    	}

    	if(time.flags.stop_mode){ // Enter Stop Mode
    		LCDBackLightDisable();
    		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    		SystemInit();
    		SystemCoreClockUpdate();

    		do{
    			SYSCFG_CompensationCellCmd(ENABLE);
    		}while(SYSCFG_GetCompensationCellStatus() == RESET);

    		LCDBackLightInit();
    		time.flags.stop_mode = 0;
    	}
    	switch (LCDStatusStruct.waitExitKey) {
		case FILE_TYPE_WAV: // WAV
		case FILE_TYPE_MP3: // MP3
		case FILE_TYPE_AAC: // AAC
		case FILE_TYPE_MOV: // Motion Jpeg
			shuffle_play.play_continuous = 0;
			shuffle_play.mode_changed = 0;
			shuffle_play.initial_mode = navigation_loop_mode;
			do{
				ret = PlayMusic(LCDStatusStruct.idEntry);
				debug.printf("\r\nret:%d LCDStatusStruct.idEntry:%d fat.fileCnt:%d navigation_loop.mode:%d", ret, LCDStatusStruct.idEntry, fat.fileCnt, navigation_loop_mode);
				switch(ret){
				case RET_PLAY_NORM:
					switch(navigation_loop_mode){
					case NAV_ONE_PLAY_EXIT:
						LCDStatusStruct.waitExitKey = 0;
						ret = RET_PLAY_STOP;
						break;
					case NAV_INFINITE_ONE_PLAY:
						debug.printf("\r\nNAV_INFINITE_ONE_PLAY:%d", LCDStatusStruct.idEntry);
						LCDStatusStruct.waitExitKey = 1;
						continue;
					case NAV_PLAY_ENTIRE:
					case NAV_INFINITE_PLAY_ENTIRE:
					case NAV_SHUFFLE_PLAY:
						goto PLAY_NEXT;
					default:
						break;
					}
					break;
				case RET_PLAY_NEXT: // 次へ
			PLAY_NEXT:
			if(shuffle_play.flag_make_rand && shuffle_play.mode_changed && (navigation_loop_mode != NAV_SHUFFLE_PLAY)){
				if(shuffle_play.initial_mode != NAV_SHUFFLE_PLAY){
					LCDStatusStruct.idEntry = shuffle_play.pRandIdEntry[LCDStatusStruct.idEntry];
				}
				shuffle_play.mode_changed = 0;
			}
			prevRet = LCDStatusStruct.idEntry;
					LCDStatusStruct.waitExitKey = 1;
					if(++LCDStatusStruct.idEntry >= fat.fileCnt){
						if(navigation_loop_mode == NAV_INFINITE_PLAY_ENTIRE || navigation_loop_mode == NAV_SHUFFLE_PLAY){
							LCDStatusStruct.idEntry = 1;
							LCDStatusStruct.waitExitKey = 1;
							continue;
						} else {
							LCDStatusStruct.idEntry--;
							LCDStatusStruct.waitExitKey = 0;
							ret = RET_PLAY_STOP;
						}
					}
					break;
				case RET_PLAY_PREV: // 前へ
					prevRet = RET_PLAY_PREV;
					LCDStatusStruct.waitExitKey = 1;
					if(--LCDStatusStruct.idEntry <= 0){
						LCDStatusStruct.idEntry = fat.fileCnt - 1;
					}
					continue;
				case RET_PLAY_INVALID: // invalid type
					ret = RET_PLAY_STOP;
					LCDStatusStruct.waitExitKey = 0;
					if(prevRet == RET_PLAY_PREV){
						LCDStatusStruct.idEntry++;
					} else {
						LCDStatusStruct.idEntry--;
					}
					break;
				default:
					break;
				}
			}while((ret != RET_PLAY_STOP) || LCDStatusStruct.waitExitKey);
			if(shuffle_play.flag_make_rand && (navigation_loop_mode == NAV_SHUFFLE_PLAY)){
				LCDStatusStruct.idEntry = shuffle_play.pRandIdEntry[LCDStatusStruct.idEntry];
			}
			cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
			cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;

			MergeCircularProgressBar(1);
			LCDPrintFileList();
			touch.click = 0;
			break;
			case FILE_TYPE_JPG: // JPEG
				arrow_clicked = 0;
				ret = 0;
				photo_frame_flag = 0;
//				uint8_t play_flag = 0;
//				int WAV_idEntry;
				do{
					djpeg_arrows = DJPEG_ARROW_LEFT | DJPEG_ARROW_RIGHT;
					if((ret & DJPEG_PLAY)){
						djpeg_arrows |= DJPEG_PLAY;
						if(LCDStatusStruct.idEntry >= fat.fileCnt){
							LCDStatusStruct.idEntry = 1;
						}
					}
					if(LCDStatusStruct.idEntry <= 1){
						djpeg_arrows &= ~DJPEG_ARROW_LEFT;
					} else {
						if(setExtensionName(extensionName, LCDStatusStruct.idEntry - 1)){ // check if current - 1 entry is JPEG
							if(strncmp(extensionName, "JPG", 3) != 0 && strncmp(extensionName, "JPE", 3) != 0){
								djpeg_arrows &= ~DJPEG_ARROW_LEFT;
							}
						} else {
							djpeg_arrows &= ~DJPEG_ARROW_LEFT;
						}
					}
					if(LCDStatusStruct.idEntry >= (fat.fileCnt - 1)){ // if id entry reaches end of files
						djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
					} else {
						if(setExtensionName(extensionName, LCDStatusStruct.idEntry + 1)){ // check if current - 1 entry is JPEG
							if(strncmp(extensionName, "JPG", 3) != 0 && strncmp(extensionName, "JPE", 3) != 0){
								djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
							}
						} else {
							djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
						}
					}
					if(setExtensionName(extensionName, LCDStatusStruct.idEntry)){ // check if current entry is JPEG
						if(strncmp(extensionName, "JPG", 3) == 0 || strncmp(extensionName, "JPE", 3) == 0){
							ret = dojpeg(LCDStatusStruct.idEntry, djpeg_arrows, arrow_clicked);
						}
					}
					if(ret == DJPEG_ARROW_LEFT){
						arrow_clicked = 1;
						LCDStatusStruct.idEntry--;
					} else if(ret == DJPEG_ARROW_RIGHT){
						arrow_clicked = 1;
						LCDStatusStruct.idEntry++;
					} else if(ret == DJPEG_PLAY){
						arrow_clicked = 0;
						if(!photo_frame_flag){
							photo_frame_flag = 1;
						} else {
							LCDStatusStruct.idEntry++;
						}
						/* Experimental photo frame music
						if(dac_intr.comp == 1){
							play_flag = 0;
						}
						if(!play_flag){
							play_flag = 1;
							WAV_idEntry = 0;
							while(++WAV_idEntry <= (fat.fileCnt - 1)){
								if(setExtensionName(extensionName, WAV_idEntry)){
									if(strncmp(extensionName, "WAV", 3) == 0){
										debug.printf("\r\nFound WAV file at:%d", WAV_idEntry);
										PlaySoundPhotoFrame(WAV_idEntry);
										break;
									}
								}
							}
						}
						*/
					}
					if(LCDStatusStruct.idEntry){
						cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
						cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;
					}
				}while(LCDStatusStruct.waitExitKey);
				/* Experimental photo frame music
				NVIC_InitTypeDef NVIC_InitStructure;

			    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC | DMA_IT_HT, DISABLE);
			    DMA_Cmd(DMA1_Stream1, DISABLE);
			    AUDIO_OUT_SHUTDOWN;

				dac_intr.sound_reads = 0;

				// Disable DMA1_Stream1 gloabal Interrupt
				NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
				NVIC_Init(&NVIC_InitStructure);

				my_fclose(dac_intr.fp);
				 */
				MergeCircularProgressBar(1);
				LCDPrintFileList();
    			touch.click = 0;
				break;
			case FILE_TYPE_PCF: // Display Font
				if(PCFFontInit(LCDStatusStruct.idEntry) != -1){
			    	debug.printf("\r\nexternal font loaded.");
					LCD_FUNC.putChar = PCFPutChar;
					LCD_FUNC.putWideChar = PCFPutChar;
					LCD_FUNC.getCharLength = PCFGetCharPixelLength;
				} else {
			    	debug.printf("\r\nexternal font load failed.");
				}
				LCDPrintFileList();
				LCDStatusStruct.waitExitKey = 0;
    			touch.click = 0;
				break;
			default:
				break;
		}
    }

    return 0;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	debug.printf("\r\nWrong parameters value: file %s on line %d\r\n", file, line);

    /* Infinite loop */
    while (1)
    {
    }
}
#endif
