/*
 *		STM32F4-Discovery Motion Player Project
 *		by Tonsuke
 *
 *		v1.5
 *		2013/05/25
 *
 *		http://motionplayer.wiki.fc2.com
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

void TIM8_UP_TIM13_IRQHandler(void) // Back Light & Sleep control timer
{
	if(TIM_GetITStatus(TIM8, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);

		if(settings_group.disp_conf.time2sleep == 0){ // Always On
			return;
		}

		++time.curTime;

		if(abs(time.curTime - time.prevTime) >= settings_group.disp_conf.time2sleep / 2 && !time.flags.dimLight && time.flags.enable){ // Dim Light
			TIM_SetCompare2(TIM4, (int)(500 * (float)settings_group.disp_conf.brightness / 100.0f) - 1);
			time.flags.dimLight = 1;
		}

		if(abs(time.curTime - time.prevTime) >= settings_group.disp_conf.time2sleep && time.flags.dimLight && time.flags.enable){ // Sleep Enable
			time.flags.stop_mode = 1;
		}
	}
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

    SETTINGS_Init();

    USARTInit();

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

	if(PCFFontInit(getIdByName("FONT.PCF")) != -1){
		LCD_FUNC.putChar = PCFPutChar;
		LCD_FUNC.putWideChar = PCFPutChar;
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
    	if(time.flags.stop_mode){
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
						goto PLAY_NEXT;
					default:
						break;
					}
					break;
				case RET_PLAY_NEXT: // 次へ
			PLAY_NEXT:
			prevRet = LCDStatusStruct.idEntry;
					LCDStatusStruct.waitExitKey = 1;
					if(++LCDStatusStruct.idEntry >= fat.fileCnt){
						if(navigation_loop_mode == NAV_INFINITE_PLAY_ENTIRE){
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
				if(LCDStatusStruct.idEntry){
					cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
					cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;
				}
			}while((ret != RET_PLAY_STOP) || LCDStatusStruct.waitExitKey);
			MergeCircularProgressBar();
			LCDPrintFileList();
			touch.click = 0;
			break;
			case FILE_TYPE_JPG: // JPEG
				arrow_clicked = 0;
				do{
					djpeg_arrows = DJPEG_ARROW_LEFT | DJPEG_ARROW_RIGHT;
					if(LCDStatusStruct.idEntry <= 1){
						djpeg_arrows &= ~DJPEG_ARROW_LEFT;
					} else {
						if(setExtensionName(extensionName, LCDStatusStruct.idEntry - 1)){
							if(strncmp(extensionName, "JPG", 3) != 0 && strncmp(extensionName, "JPE", 3) != 0){
								djpeg_arrows &= ~DJPEG_ARROW_LEFT;
							}
						} else {
							djpeg_arrows &= ~DJPEG_ARROW_LEFT;
						}
					}
					if(LCDStatusStruct.idEntry >= (fat.fileCnt - 1)){
						djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
					} else {
						if(setExtensionName(extensionName, LCDStatusStruct.idEntry + 1)){
							if(strncmp(extensionName, "JPG", 3) != 0 && strncmp(extensionName, "JPE", 3) != 0){
								djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
							}
						} else {
							djpeg_arrows &= ~DJPEG_ARROW_RIGHT;
						}
					}
					ret = dojpeg(LCDStatusStruct.idEntry, djpeg_arrows, arrow_clicked);
					if(ret == DJPEG_ARROW_LEFT){
						arrow_clicked = 1;
						LCDStatusStruct.idEntry--;
					} else if(ret == DJPEG_ARROW_RIGHT){
						arrow_clicked = 1;
						LCDStatusStruct.idEntry++;
					}
					if(LCDStatusStruct.idEntry){
						cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
						cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;
					}
				}while(LCDStatusStruct.waitExitKey);
				MergeCircularProgressBar();
				LCDPrintFileList();
    			touch.click = 0;
				break;
			case FILE_TYPE_PCF: // Display Font
				if(PCFFontInit(LCDStatusStruct.idEntry) != -1){
					LCD_FUNC.putChar = PCFPutChar;
					LCD_FUNC.putWideChar = PCFPutChar;
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
