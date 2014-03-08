/*
 * settings.c
 *
 *  Created on: 2013/03/18
 *      Author: Tonsuke
 */


#include "settings.h"
#include <stdio.h>

#include "mpool.h"

#include "board_config.h"

#include "xpt2046.h"
#include "usart.h"
#include "fat.h"
#include "lcd.h"
#include "pcf_font.h"
#include "sd.h"

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"


char settings_mode;
unsigned char settings_root_list_fileCnt;

settings_group_typedef settings_group;


settings_list_typedef *settings_p;
settings_stack_typedef settings_stack;

settings_item_typedef settings_item_card;
settings_item_typedef settings_item_cpu;
settings_item_typedef settings_item_usart;
settings_item_typedef settings_item_font;
settings_item_typedef settings_item_sort;
settings_item_typedef settings_item_photo_frame_td;
settings_item_typedef settings_item_brightness;
settings_item_typedef settings_item_sleeptime;
settings_item_typedef settings_item_fft;
settings_item_typedef settings_item_fft_bar_type;
settings_item_typedef settings_item_fft_color_type;
settings_item_typedef settings_item_musicinfo;
settings_item_typedef settings_item_prehalve;

const icon_ptr_typedef music_icon[] = {onpu_22x22, onpu_22x22_alpha};
const icon_ptr_typedef folder_icon[] = {folder_22x22, folder_22x22_alpha};
const icon_ptr_typedef card_icon[] = {card_22x22, card_22x22_alpha};
const icon_ptr_typedef cpu_icon[] = {cpu_22x22, cpu_22x22_alpha};
const icon_ptr_typedef display_icon[] = {display_22x22, display_22x22_alpha};
const icon_ptr_typedef debug_icon[] = {debug_22x22, debug_22x22_alpha};
const icon_ptr_typedef info_icon[] = {info_22x22, info_22x22_alpha};
const icon_ptr_typedef usb_icon[] = {usb_22x22, usb_22x22_alpha};
const icon_ptr_typedef connect_icon[] = {connect_22x22, connect_22x22_alpha};

int my_sprintf(char *a, const char *b, ...)
{
	return 0;
}


const settings_list_typedef settings_root_list[] = {
		{"..", NULL, 0, NULL, },
		{"About Motion Player", info_icon, 1, NEXT_LIST(settings_about_motionplayer_list), SETTING_ABOUT_MOTIONPLAYER},
		{"Card", card_icon, 4, NEXT_LIST(settings_card_list)},
		{"CPU", cpu_icon, 3, NEXT_LIST(settings_cpu_list)},
		{"Debug", debug_icon, 2, NEXT_LIST(settings_debug_list)},
		{"Display", display_icon, 3, NEXT_LIST(settings_display_list)},
		{"Filer", folder_icon, 4, NEXT_LIST(settings_filer_list)},
		{"Music", music_icon, 4, NEXT_LIST(settings_music_list)},
		{"USB Mass Storage", usb_icon, 2, NEXT_LIST(settings_usb_msc_list)},
};

const settings_list_typedef settings_usb_msc_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"Connect to Host", connect_icon, 1, NEXT_LIST(settings_usb_msc_select_list), SETTINS_USB_CONNECT_HOST},
};

const settings_list_typedef settings_usb_msc_select_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_usb_msc_list)},
};

const settings_list_typedef settings_about_motionplayer_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
};

const settings_list_typedef settings_music_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"FFT Settings", NULL, 4, NEXT_LIST(settings_fft_list)},
		{"Display Music Info", NULL, 3, NEXT_LIST(settings_musicinfo_list), NULL, SETTING_TYPE_ITEM, &settings_item_musicinfo},
		{"Pre Halve Sound Sample", NULL, 3, NEXT_LIST(settings_prehalve_list), NULL, SETTING_TYPE_ITEM, &settings_item_prehalve},
};

const settings_list_typedef settings_fft_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"Display Analyzer", NULL, 3, NEXT_LIST(settings_fft_display_list), NULL, SETTING_TYPE_ITEM, &settings_item_fft},
		{"Bar Type", NULL, 5, NEXT_LIST(settings_fft_bar_type_list), NULL, SETTING_TYPE_ITEM, &settings_item_fft_bar_type},
		{"Bar Color", NULL, 8, NEXT_LIST(settings_fft_bar_color_list), NULL, SETTING_TYPE_ITEM, &settings_item_fft_color_type},
};

const settings_list_typedef settings_fft_display_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"ON", NULL, 0, NULL, NULL, 0, &settings_item_fft},
		{"OFF", NULL, 0, NULL, NULL, 0, &settings_item_fft},
};

const settings_list_typedef settings_fft_bar_type_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"Solid", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"V Split", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"H Split", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"Wide", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
};

const settings_list_typedef settings_fft_bar_color_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"White", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Skyblue", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Yellow", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Green", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Red", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Black", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Colorful", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
};

const settings_list_typedef settings_musicinfo_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"ON", NULL, 0, NULL, NULL, 0, &settings_item_musicinfo},
		{"OFF", NULL, 0, NULL, NULL, 0, &settings_item_musicinfo},
};

const settings_list_typedef settings_prehalve_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"Enable", NULL, 0, NULL, NULL, 0, &settings_item_prehalve},
		{"Disable", NULL, 0, NULL, NULL, 0, &settings_item_prehalve},
};

const settings_list_typedef settings_display_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"Brightness", NULL, 5, NEXT_LIST(settings_brightness_list), NULL, SETTING_TYPE_ITEM, &settings_item_brightness},
		{"Time To Sleep", NULL, 6, NEXT_LIST(settings_sleeptime_list), NULL, SETTING_TYPE_ITEM, &settings_item_sleeptime},
};

const settings_list_typedef settings_brightness_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_display_list)},
		{"25%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"50%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"75%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"100%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
};

const settings_list_typedef settings_sleeptime_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_display_list)},
		{"Always ON", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"15s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"30s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"45s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"60s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
};

const settings_list_typedef settings_filer_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"External Font", NULL, 3, NEXT_LIST(settings_font_list), NULL, SETTING_TYPE_ITEM, &settings_item_font},
		{"Sort Items", NULL, 3, NEXT_LIST(settings_sort_list), NULL, SETTING_TYPE_ITEM, &settings_item_sort},
		{"Photo Frame Time Duration", NULL, 5, NEXT_LIST(settings_photo_frame_td_list), NULL, SETTING_TYPE_ITEM, &settings_item_photo_frame_td},
};

const settings_list_typedef settings_font_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_filer_list)},
		{"Enable", NULL, 0, NULL, NULL, 0, &settings_item_font},
		{"Disable", NULL, 0, NULL, NULL, 0, &settings_item_font},
};

const settings_list_typedef settings_sort_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_filer_list)},
		{"Enable", NULL, 0, NULL, NULL, 0, &settings_item_sort},
		{"Disable", NULL, 0, NULL, NULL, 0, &settings_item_sort},
};

const settings_list_typedef settings_photo_frame_td_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_filer_list)},
		{"1s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"3s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"5s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"10s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
};

const settings_list_typedef settings_debug_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"USART Baudrate", NULL, 9, NEXT_LIST(settings_baudrate_list), NULL, SETTING_TYPE_ITEM, &settings_item_usart},
};

const settings_list_typedef settings_baudrate_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_debug_list)},
		{"9600", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"19200", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"38400", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"76800", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"115200", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"230400", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"460800", NULL, 0, NULL, NULL, 0, &settings_item_usart},
		{"921600", NULL, 0, NULL, NULL, 0, &settings_item_usart},
};

const settings_list_typedef settings_cpu_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"Clock Frequency", NULL, 8, NEXT_LIST(settings_cpufreq_list), NULL, SETTING_TYPE_ITEM, &settings_item_cpu},
		{"Core Temperature", NULL, 1, NEXT_LIST(settings_back_to_cpulist), SETTINGS_CORE_TEMPERATURE},
};

const settings_list_typedef settings_cpufreq_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_cpu_list)},
		{"72MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"100MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"120MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"168MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"200MHz OC", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"240MHz OC", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"250MHz OC", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
};

const settings_list_typedef settings_card_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list), },
		{"Bus Width", NULL, 3, NEXT_LIST(settings_card_buswidth_list), NULL, SETTING_TYPE_ITEM, &settings_item_card},
		{"Speed Test", NULL, 1, NEXT_LIST(settings_back_to_cardlist), SETTING_CARD_SPEEDTEST },
		{"Card Info", NULL, 1, NEXT_LIST(settings_back_to_cardlist), SETTING_DISPLAY_CARDINFO},
};

const settings_list_typedef settings_card_buswidth_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_card_list)},
		{"1bit", NULL, 0, NULL, NULL, 0, &settings_item_card},
		{"4bit", NULL, 0, NULL, NULL, 0, &settings_item_card},
};

const settings_list_typedef settings_back_to_cardlist[] = {
		{"..", NULL, 0, NEXT_LIST(settings_card_list)},
};

const settings_list_typedef settings_back_to_cpulist[] = {
		{"..", NULL, 0, NEXT_LIST(settings_cpu_list)},
};


/**
  * @brief  Configures the System clock source, PLL Multiplier and Divider factors,
  *         AHB/APBx prescalers and Flash settings
  * @Note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
static void SetSysClock2(int PLL_N)
{
#define FLASH_SECTOR_1_OFFSET (0x4000)
#define FLASH_SETTING_OFFSET (0x3000)
#define FLASH_SETTING_BASE (FLASH_BASE + FLASH_SECTOR_1_OFFSET + FLASH_SETTING_OFFSET)

	int PLL_M;
	int PLL_P;
	int PLL_Q;

  static const unsigned int cpu_freq_tbl[] = {72, 100, 120, 168, 200, 240, 250};
  PLL_N = validate_val(PLL_N, 168, cpu_freq_tbl, sizeof(cpu_freq_tbl) / sizeof(cpu_freq_tbl[0]));

  /* Determin USB OTG FS, SDIO and RNG Clock from PLL_N */
  uint32_t FLASH_LATENCY;
	switch(PLL_N){
	case 72:
		PLL_M = 8;
		PLL_N = 144;
		PLL_P = 2;
		PLL_Q = 3; // 144 / 3 = 48MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_2WS;
		break;
	case 100:
		PLL_M = 8;
		PLL_N = 200;
		PLL_P = 2;
		PLL_Q = 4; // 200 / 4 = 50MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_3WS;
		break;
	case 120:
		PLL_M = 8;
		PLL_N = 240;
		PLL_P = 2;
		PLL_Q = 5; // 240 / 5 = 48MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_3WS;
		break;
	case 168:
		PLL_M = 8;
		PLL_N = 336;
		PLL_P = 2;
		PLL_Q = 7; // 336 / 7 = 48MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_5WS;
		break;
	case 200:
		PLL_M = 8;
		PLL_N = 400;
		PLL_P = 2;
		PLL_Q = 8; // 400 / 8 = 50MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_5WS;
		break;
	case 240:
		PLL_M = 8;
		PLL_N = 480;
		PLL_P = 2;
		PLL_Q = 10; // 480 / 10 = 48MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_7WS;
		break;
	case 250:
		PLL_M = 8;
		PLL_N = 500;
		PLL_P = 2;
		PLL_Q = 10; // 500 / 10 = 50MHz
		FLASH_LATENCY = FLASH_ACR_LATENCY_7WS;
		break;
	default:
		break;
	}

  SystemCoreClock = (((HSE_VALUE / PLL_M) * PLL_N) / PLL_P);

/******************************************************************************/
/*            PLL (clocked by HSE) used as System clock source                */
/******************************************************************************/
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

  /* Enable HSE */
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }

  if (HSEStatus == (uint32_t)0x01)
  {
    /* Enable high performance mode, System frequency up to 168 MHz */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_PMODE;

    /* HCLK = SYSCLK / 1*/
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    /* PCLK2 = HCLK / 2*/
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

    /* PCLK1 = HCLK / 4*/
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

    /* Configure the main PLL */
    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
                   (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

    /* Enable the main PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till the main PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

	uint32_t mcu_revision = *(uint32_t*)0xE0042000 & 0xffff0000;
	switch(mcu_revision){
	case 0x10000000: // A
	case 0x20000000: // B
	default:
	    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
	    FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN | FLASH_LATENCY;
		break;
	case 0x10010000: // Z
	    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
	    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN | FLASH_LATENCY;
		break;
	}


    /* Select the main PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    /* Wait till the main PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock
         configuration. User can add here some code to deal with this error */
  }
}

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemFrequency variable.
  * @param  None
  * @retval None
  */
void SystemInit2(int PLL_N)
{
#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field.
                                   This value must be a multiple of 0x200. */

  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x24003010;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

#ifdef DATA_IN_ExtSRAM
  SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM */

  /* Configure the System clock source, PLL Multiplier and Divider factors,
     AHB/APBx prescalers and Flash settings ----------------------------------*/
  SetSysClock2(PLL_N);

  /* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}


int validate_saved_val(int val, int default_val, const unsigned int tbl[], size_t tbl_size)
{
	int i, coincide = 0;

	for(i = 0;i < tbl_size;i++){
		if(val == tbl[i]){
			coincide = 1;
			break;
		}
	}

	if(!coincide){
		return default_val;
	}

	return val;
}

int selected_id(int val, const unsigned int tbl[], size_t tbl_size)
{
	int i, ret, coincide  = 0;

	for(i = 0;i < tbl_size;i++){
		if(val == tbl[i]){
			coincide = 1;
			ret = i;
			break;
		}
	}

	if(!coincide){
		return 0;
	}

	return ret;
}

void SETTINGS_Init()
{
	settings_mode = 0;
	settings_root_list_fileCnt = sizeof(settings_root_list) / sizeof(settings_root_list[0]);

	touch.cal = &settings_group.touch_cal;

	/* Load settings */
	memcpy((void*)&settings_group, (void*)(FLASH_SETTING_BASE), sizeof(settings_group));

	/* FFT Item */
	static const unsigned int fft_tbl[] = {1, 0};
#define FFT_TABLE_ITEMS (sizeof(fft_tbl) / sizeof(fft_tbl[0]))
	settings_group.music_conf.b.fft = validate_saved_val(settings_group.music_conf.b.fft, 1, fft_tbl, FFT_TABLE_ITEMS);
	settings_item_fft.selected_id = selected_id(settings_group.music_conf.b.fft, fft_tbl, FFT_TABLE_ITEMS);
	settings_item_fft.item_count = FFT_TABLE_ITEMS;
	settings_item_fft.item_array = fft_tbl;
	settings_item_fft.func = SETTINGS_FFT;

	/* FFT Bar Type Item */
	static const unsigned int fft_bar_type_tbl[] = {0, 1, 2, 3}; // 0:Solid 1:V Split 2:H Split 3:Wide
#define FFT_BAR_TYPE_TABLE_ITEMS (sizeof(fft_bar_type_tbl) / sizeof(fft_bar_type_tbl[0]))
	settings_group.music_conf.b.fft_bar_type = validate_saved_val(settings_group.music_conf.b.fft_bar_type, 0, fft_bar_type_tbl, FFT_BAR_TYPE_TABLE_ITEMS);
	settings_item_fft_bar_type.selected_id = selected_id(settings_group.music_conf.b.fft_bar_type, fft_bar_type_tbl, FFT_BAR_TYPE_TABLE_ITEMS);
	settings_item_fft_bar_type.item_count = FFT_BAR_TYPE_TABLE_ITEMS;
	settings_item_fft_bar_type.item_array = fft_bar_type_tbl;
	settings_item_fft_bar_type.func = SETTINGS_FFT_BAR_TYPE;

	/* FFT Bar Color Item */
	static const unsigned int fft_bar_color_tbl[] = {0, 1, 2, 3, 4, 5, 6}; // 0:White 1:Skyblue 2:Yellow 3:Green 4:Red 5:Black 6:Colorful
#define FFT_BAR_COLOR_TABLE_ITEMS (sizeof(fft_bar_color_tbl) / sizeof(fft_bar_color_tbl[0]))
	settings_group.music_conf.b.fft_bar_color_idx = validate_saved_val(settings_group.music_conf.b.fft_bar_color_idx, 0, fft_bar_color_tbl, FFT_BAR_COLOR_TABLE_ITEMS);
	settings_item_fft_color_type.selected_id = selected_id(settings_group.music_conf.b.fft_bar_color_idx, fft_bar_color_tbl, FFT_BAR_COLOR_TABLE_ITEMS);
	settings_item_fft_color_type.item_count = FFT_BAR_COLOR_TABLE_ITEMS;
	settings_item_fft_color_type.item_array = fft_bar_color_tbl;
	settings_item_fft_color_type.func = SETTINGS_FFT_BAR_COLOR;

	/* Music Info Item */
	static const unsigned int musicinfo_tbl[] = {1, 0};
#define MUSICINFO_TABLE_ITEMS (sizeof(musicinfo_tbl) / sizeof(musicinfo_tbl[0]))
	settings_group.music_conf.b.musicinfo = validate_saved_val(settings_group.music_conf.b.musicinfo, 1, musicinfo_tbl, MUSICINFO_TABLE_ITEMS);
	settings_item_musicinfo.selected_id = selected_id(settings_group.music_conf.b.musicinfo, musicinfo_tbl, MUSICINFO_TABLE_ITEMS);
	settings_item_musicinfo.item_count = MUSICINFO_TABLE_ITEMS;
	settings_item_musicinfo.item_array = musicinfo_tbl;
	settings_item_musicinfo.func = SETTINGS_MUSICINFO;

	/* Pre Halve Item */
	static const unsigned int prehalve_tbl[] = {1, 0};
#define PREHALVE_TABLE_ITEMS (sizeof(prehalve_tbl) / sizeof(prehalve_tbl[0]))
	settings_group.music_conf.b.prehalve = validate_saved_val(settings_group.music_conf.b.prehalve, 1, prehalve_tbl, PREHALVE_TABLE_ITEMS);
	settings_item_prehalve.selected_id = selected_id(settings_group.music_conf.b.prehalve, prehalve_tbl, PREHALVE_TABLE_ITEMS);
	settings_item_prehalve.item_count = PREHALVE_TABLE_ITEMS;
	settings_item_prehalve.item_array = prehalve_tbl;
	settings_item_prehalve.func = SETTINGS_PREHALVE;

	/* Display Brightness Item */
	static const unsigned int brightness_tbl[] = {25, 50, 75, 100};
#define BRIGHTNESS_TABLE_ITEMS (sizeof(brightness_tbl) / sizeof(brightness_tbl[0]))
	settings_group.disp_conf.brightness = validate_saved_val(settings_group.disp_conf.brightness, 100, brightness_tbl, BRIGHTNESS_TABLE_ITEMS);
	settings_item_brightness.selected_id = selected_id(settings_group.disp_conf.brightness, brightness_tbl, BRIGHTNESS_TABLE_ITEMS);
	settings_item_brightness.item_count = BRIGHTNESS_TABLE_ITEMS;
	settings_item_brightness.item_array = brightness_tbl;
	settings_item_brightness.func = SETTINGS_DISPLAY_BRIGHTNESS;

	/* Display Sleep Time Item */
	static const unsigned int sleeptime_tbl[] = {0, 15, 30, 45, 60};
#define SLEEPTIME_TABLE_ITEMS (sizeof(sleeptime_tbl) / sizeof(sleeptime_tbl[0]))
	settings_group.disp_conf.time2sleep = validate_saved_val(settings_group.disp_conf.time2sleep, 15, sleeptime_tbl, SLEEPTIME_TABLE_ITEMS);
	settings_item_sleeptime.selected_id = selected_id(settings_group.disp_conf.time2sleep, sleeptime_tbl, SLEEPTIME_TABLE_ITEMS);
	settings_item_sleeptime.item_count = SLEEPTIME_TABLE_ITEMS;
	settings_item_sleeptime.item_array = sleeptime_tbl;
	settings_item_sleeptime.func = SETTINGS_DISPLAY_SLEEP;

	/* External Font Item */
	static const unsigned int font_tbl[] = {1, 0};
#define FONT_TABLE_ITEMS (sizeof(font_tbl) / sizeof(font_tbl[0]))
	settings_group.filer_conf.fontEnabled = validate_saved_val(settings_group.filer_conf.fontEnabled, 1, font_tbl, FONT_TABLE_ITEMS);
	settings_item_font.selected_id = selected_id(settings_group.filer_conf.fontEnabled, font_tbl, FONT_TABLE_ITEMS);
	settings_item_font.item_count = FONT_TABLE_ITEMS;
	settings_item_font.item_array = font_tbl;
	settings_item_font.func = SETTINGS_FONT_ENABLE;

	/* Filer Sort Item */
	static const unsigned int sort_tbl[] = {1, 0};
#define SORT_TABLE_ITEMS (sizeof(sort_tbl) / sizeof(sort_tbl[0]))
	settings_group.filer_conf.sort = validate_saved_val(settings_group.filer_conf.sort, 1, sort_tbl, SORT_TABLE_ITEMS);
	settings_item_sort.selected_id = selected_id(settings_group.filer_conf.sort, sort_tbl, SORT_TABLE_ITEMS);
	settings_item_sort.item_count = SORT_TABLE_ITEMS;
	settings_item_sort.item_array = sort_tbl;
	settings_item_sort.func = SETTINGS_FILER_SORT;

	/* Photo Frame Time Duration Item */
	static const unsigned int photo_frame_td_tbl[] = {1, 3, 5, 10};
#define PHOTO_FRAME_TD_TABLE_ITEMS (sizeof(photo_frame_td_tbl) / sizeof(photo_frame_td_tbl[0]))
	settings_group.filer_conf.photo_frame_td = validate_saved_val(settings_group.filer_conf.photo_frame_td, 3, photo_frame_td_tbl, PHOTO_FRAME_TD_TABLE_ITEMS);
	settings_item_photo_frame_td.selected_id = selected_id(settings_group.filer_conf.photo_frame_td, photo_frame_td_tbl, PHOTO_FRAME_TD_TABLE_ITEMS);
	settings_item_photo_frame_td.item_count = PHOTO_FRAME_TD_TABLE_ITEMS;
	settings_item_photo_frame_td.item_array = photo_frame_td_tbl;
	settings_item_photo_frame_td.func = SETTINGS_PHOTO_FRAME_TD;

	/* Init USART Baudrate Item */
	static const unsigned int baudrate_tbl[] = {9600, 19200, 38400, 76800, 115200, 230400, 460800, 921600};
#define BAUDRATE_TABLE_ITEMS (sizeof(baudrate_tbl) / sizeof(baudrate_tbl[0]))
	settings_group.debug_conf.baudrate = validate_saved_val(settings_group.debug_conf.baudrate, 115200, baudrate_tbl, BAUDRATE_TABLE_ITEMS);
	settings_item_usart.selected_id = selected_id(settings_group.debug_conf.baudrate, baudrate_tbl, BAUDRATE_TABLE_ITEMS);
	settings_item_usart.item_count = BAUDRATE_TABLE_ITEMS;
	settings_item_usart.item_array = baudrate_tbl;
	settings_item_usart.func = SETTINGS_BAUDRATE;

	/* Init CPU Frequency Item */
	static const unsigned int cpu_freq_tbl[] = {72, 100, 120, 168, 200, 240, 250};
#define CPU_FREQ_TABLE_ITEMS (sizeof(cpu_freq_tbl) / sizeof(cpu_freq_tbl[0]))
	settings_group.cpu_conf.freq = validate_saved_val(settings_group.cpu_conf.freq, 168, cpu_freq_tbl, CPU_FREQ_TABLE_ITEMS);
	settings_item_cpu.selected_id = selected_id(settings_group.cpu_conf.freq, cpu_freq_tbl, CPU_FREQ_TABLE_ITEMS);
	settings_item_cpu.item_count = CPU_FREQ_TABLE_ITEMS;
	settings_item_cpu.item_array = cpu_freq_tbl;
	settings_item_cpu.func = SETTINGS_CPU_FREQ;

	/* Init Card Item */
	static const unsigned int card_wide_bus_tbl[] = {1, 4};
#define CARD_WIDE_BUS_TABLE_ITEMS (sizeof(card_wide_bus_tbl) / sizeof(card_wide_bus_tbl[0]))
	settings_group.card_conf.busWidth = (uint8_t)validate_saved_val(settings_group.card_conf.busWidth, 1, card_wide_bus_tbl, CARD_WIDE_BUS_TABLE_ITEMS);
	settings_item_card.selected_id = selected_id(settings_group.card_conf.busWidth, card_wide_bus_tbl, CARD_WIDE_BUS_TABLE_ITEMS);
	settings_item_card.item_count = CARD_WIDE_BUS_TABLE_ITEMS;
	settings_item_card.item_array = card_wide_bus_tbl;
	settings_item_card.func = SETTING_CARD_BUSWIDTH;
}

void SETTINGS_Save()
{
	int flash_status, i;
	uint32_t flash_addr;
	uint8_t flash_sector1_buf[FLASH_SECTOR_1_SIZE];

	memcpy((void*)flash_sector1_buf, (void*)FLASH_SECOTR_1_ADDR, FLASH_SECTOR_1_SIZE); // Backup Sector #1

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	flash_status = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3); // Pre Erase Sector #1

	memcpy((void*)&flash_sector1_buf[FLASH_SETTING_OFFSET], (void*)&settings_group, sizeof(settings_group));

	flash_addr = FLASH_SECOTR_1_ADDR; // Sector #1
	for(i = 0;i < FLASH_SECTOR_1_SIZE;i += sizeof(uint8_t)){ // Flash Sector #1
		flash_status = FLASH_ProgramByte(flash_addr, flash_sector1_buf[i]);
		flash_addr += sizeof(uint8_t);
	}

}

static void *SETTING_DISPLAY_CARDINFO(void *arg)
{
	char s[100];

	LCDGotoXY(0, 50);

	LCDPutString("File System:", WHITE);
	if(fat.fsType == FS_TYPE_FAT16){
		LCDPutString("FAT16\n", WHITE);
	} else if(fat.fsType == FS_TYPE_FAT32) {
		LCDPutString("FAT32\n", WHITE);
	} else {
		LCDPutString("Unknown\n", WHITE);
	}
	SPRINTF(s, "Cluster Size:%dKB\n", (fat.sectorsPerCluster * 512) / 1024);
	LCDPutString(s, WHITE);

	SPRINTF(s, "\nSpec Version:%s\n", &specVer[cardInfo.specVer][0]);
	LCDPutString(s, WHITE);

	SPRINTF(s, "High Capacity:%s\n", cardInfo.csdVer ? "Yes" : "No");
	LCDPutString(s, WHITE);

	if(cardInfo.speedClass){
		SPRINTF(s, "Speed Class:CLASS%d\n", cardInfo.speedClass);
	} else {
		strcpy(s, "Speed Class:N/A\n");
	}
	LCDPutString(s, WHITE);

	SPRINTF(s, "Supported Bus Widths:%s\n", &busWidth[cardInfo.busWidth][0]);
	LCDPutString(s, WHITE);

	SPRINTF(s, "Max Transfer Speed Per Bus:%dMbit/s\n", cardInfo.tranSpeed / 1000);
	LCDPutString(s, WHITE);

	SPRINTF(s, "Max Clock Frequency:%dMHz\n", cardInfo.maxClkFreq);
	LCDPutString(s, WHITE);

	SPRINTF(s, "Total Blocks:%d\n", cardInfo.totalBlocks);
	LCDPutString(s, WHITE);

	SPRINTF(s, "Card Capacity:%0.2fGB\n", (float)cardInfo.totalBlocks / 1000000000.0f * 512.0f);
	LCDPutString(s, WHITE);

	return NULL;
}

void *SETTING_CARD_SPEEDTEST(void *arg)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	int i, count, blocksize;

	blocksize = settings_group.card_conf.busWidth == 1 ? 32 : 64;

	char s[20];

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	for(i = 1;i <= 5;i++){
		TIM_Cmd(TIM1, DISABLE);
		TIM_TimeBaseInitStructure.TIM_Period = 9999;
		TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
		TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
		TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
		TIM_SetCounter(TIM1, 0);
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM1, ENABLE);

		count = 0;
		while(!TIM_GetFlagStatus(TIM1, TIM_FLAG_Update)){
			SDMultiBlockRead((void*)mempool, count * blocksize + i * 100, blocksize);
			count++;
		}

		LCDGotoXY(0, 50 + (i - 1) * 13);
		SPRINTF(s, "%d:%dKB/s" , i, (count * (blocksize * 512)) / 1000);
		LCDPutString(s, WHITE);
	}

	return NULL;
}

void *SETTING_CARD_BUSWIDTH(void *arg)
{
	settings_item_typedef *card_item = (settings_item_typedef*)arg;

	settings_group.card_conf.busWidth = card_item->item_array[card_item->selected_id];

	debug.printf("\r\nSETTING_CARD_BUSWIDTH:%d", settings_group.card_conf.busWidth);

	SETTINGS_Save();

	SD_Switch_BusWidth(settings_group.card_conf.busWidth);

	return NULL;
}


void *SETTINS_USB_CONNECT_HOST(void *arg)
{
	extern volatile int8_t usb_msc_enable;

	usb_msc_enable = 1;

	return NULL;
}

void *SETTING_ABOUT_MOTIONPLAYER(void *arg)
{
	char s[30];

	LCDGotoXY(0, 50);

	SPRINTF(s, "Version: %d.%d (%s)\n\n", VERSION_MAJOR, VERSION_MINOR, __DATE__);
	LCDPutString(s, WHITE);

	uint32_t mcu_revision = *(uint32_t*)0xE0042000 & 0xffff0000;
	switch(mcu_revision){
	case 0x10000000:
		LCDPutString("MCU Rev: A\n\n", WHITE);
		break;
	case 0x20000000:
		LCDPutString("MCU Rev: B\n\n", WHITE);
		break;
	case 0x10010000:
		LCDPutString("MCU Rev: Z\n\n", WHITE);
		break;
	default:
		LCDPutString("MCU Rev: Unkown\n\n", WHITE);
		break;
	}


	LCDPutString("Website:\nhttp://motionplayer.wiki.fc2.com\n\n", WHITE);

	LCDPutString("Author: Tonsuke\n\n", WHITE);

	return NULL;
}

void *SETTINGS_CPU_FREQ(void *arg)
{
	settings_item_typedef *cpu_item = (settings_item_typedef*)arg;

	settings_group.cpu_conf.freq = cpu_item->item_array[cpu_item->selected_id];

	debug.printf("\r\nsettings_group.cpu_conf.freq:%d", settings_group.cpu_conf.freq);

	SystemInit2(settings_group.cpu_conf.freq);

	SystemCoreClockUpdate();

	USARTInit();

	debug.printf("\r\nSystemCoreClock:%d", SystemCoreClock);

	SETTINGS_Save();


	return NULL;
}

void *SETTINGS_CORE_TEMPERATURE(void *arg)
{
	settings_item_typedef *baudrate_item = (settings_item_typedef*)arg;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;
	ADC_InitTypeDef ADC_InitStruct;

#define ADC_SAMPLE_CNT 10
	int cnt;
	uint16_t bgbuf[80 * 13];
	float temp_f, temp_val;
	char s[30];

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	ADC_DeInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStruct);

	// ADC1 Configuration, ADC_Channel_TempSensor is actual channel 16
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_144Cycles);

	// Enable internal temperature sensor
	ADC_TempSensorVrefintCmd(ENABLE);

	// Enable ADC conversion
	ADC_Cmd(ADC1, ENABLE);

	TIM_Cmd(TIM1, DISABLE);
	TIM_TimeBaseInitStructure.TIM_Period = 9999;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 99;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	TIM_SetCounter(TIM1, 0);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	LCDStoreBgImgToBuff(0, 50, 80, 13, bgbuf);

	while(1){
		temp_f = 0.0f;
		cnt = ADC_SAMPLE_CNT;
		while(cnt-- > 0){
			ADC_SoftwareStartConv(ADC1);
			while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == Bit_RESET){};

			temp_val = ADC_GetConversionValue(ADC1);
			temp_val *= 3300;
			temp_val /= 0xfff;
			temp_val /= 1000.0f;
			temp_val = ( (temp_val - 0.76f) / 0.0025f) + 25.0f;
			temp_f += temp_val;
		}
		temp_f /= (float)ADC_SAMPLE_CNT;

		LCDPutBuffToBgImg(0, 50, 80, 13, bgbuf);
		SPRINTF(s, "%.1fC", temp_f);
		LCDGotoXY(0, 50);
		LCDPutString(s, WHITE);

		while(!TIM_GetFlagStatus(TIM1, TIM_FLAG_Update)){
			if((TP_PEN_INPUT_BB == Bit_RESET)){
				goto EXIT_SETTINGS_CORE_TEMPERATURE;
			}
			if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE)){
				USART_ClearFlag(USART3, USART_FLAG_RXNE);
				if(USART_ReceiveData(USART3) == CURSOR_ENTER){
					goto EXIT_SETTINGS_CORE_TEMPERATURE;
				}
			}
		}
		TIM_SetCounter(TIM1, 0);
		TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	}
EXIT_SETTINGS_CORE_TEMPERATURE:
	ADC_DeInit();
	return NULL;
}

void *SETTINGS_BAUDRATE(void *arg)
{
	settings_item_typedef *baudrate_item = (settings_item_typedef*)arg;

	settings_group.debug_conf.baudrate = baudrate_item->item_array[baudrate_item->selected_id];

	SETTINGS_Save();

	USARTInit();

	debug.printf("\r\nBaudrate changed:%d", settings_group.debug_conf.baudrate);

	return NULL;
}

void *SETTINGS_FONT_ENABLE(void *arg)
{
	settings_item_typedef *font_item = (settings_item_typedef*)arg;

	settings_group.filer_conf.fontEnabled = font_item->item_array[font_item->selected_id];

	if(!settings_group.filer_conf.fontEnabled){
	    if(C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font) != -1){
	    	debug.printf("\r\ninternal flash font loaded.");
	    	LCD_FUNC.putChar = C_PCFPutChar;
	    	LCD_FUNC.putWideChar = C_PCFPutChar;
	    	LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;
	    } else {
	    	debug.printf("\r\ninternal flash font load failed.");
	    }
	}

	SETTINGS_Save();

	return NULL;
}


void *SETTINGS_FILER_SORT(void *arg)
{
	settings_item_typedef *sort_item = (settings_item_typedef*)arg;

	settings_group.filer_conf.sort = sort_item->item_array[sort_item->selected_id];

	SETTINGS_Save();

	return NULL;
}


void *SETTINGS_PHOTO_FRAME_TD(void *arg)
{
	settings_item_typedef *photo_frame_td_item = (settings_item_typedef*)arg;

	settings_group.filer_conf.photo_frame_td = photo_frame_td_item->item_array[photo_frame_td_item->selected_id];

	SETTINGS_Save();

	return NULL;
}


void *SETTINGS_DISPLAY_BRIGHTNESS(void *arg)
{
	settings_item_typedef *brightness_item = (settings_item_typedef*)arg;

	settings_group.disp_conf.brightness = brightness_item->item_array[brightness_item->selected_id];

	LCDBackLightInit();

	SETTINGS_Save();

	debug.printf("\r\nbrightness:%d", settings_group.disp_conf.brightness);

	return NULL;
}

void *SETTINGS_FFT(void *arg)
{
	settings_item_typedef *fft_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft = fft_item->item_array[fft_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_FFT_BAR_TYPE(void *arg)
{
	settings_item_typedef *fft_bar_type_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft_bar_type = fft_bar_type_item->item_array[fft_bar_type_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_FFT_BAR_COLOR(void *arg)
{
	settings_item_typedef *fft_bar_color_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft_bar_color_idx = fft_bar_color_item->item_array[fft_bar_color_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_MUSICINFO(void *arg)
{
	settings_item_typedef *musicinfo_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.musicinfo = musicinfo_item->item_array[musicinfo_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_PREHALVE(void *arg)
{
	settings_item_typedef *prehalve_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.prehalve = prehalve_item->item_array[prehalve_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_DISPLAY_SLEEP(void *arg)
{
	settings_item_typedef *sleep_item = (settings_item_typedef*)arg;

	settings_group.disp_conf.time2sleep = sleep_item->item_array[sleep_item->selected_id];

	SETTINGS_Save();

	debug.printf("\r\nsleep time:%d", settings_group.disp_conf.time2sleep);

	return NULL;
}
