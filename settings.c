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
#include "sd.h"

char settings_mode;
unsigned char settings_root_list_fileCnt;

settings_group_typedef settings_group;


settings_list_typedef *settings_p;
settings_stack_typedef settings_stack;

settings_item_typedef settings_item_card;
settings_item_typedef settings_item_cpu;
settings_item_typedef settings_item_usart;
settings_item_typedef settings_item_sort;
settings_item_typedef settings_item_brightness;
settings_item_typedef settings_item_sleeptime;
settings_item_typedef settings_item_fft;
settings_item_typedef settings_item_fft_bar_type;
settings_item_typedef settings_item_fft_color_type;
settings_item_typedef settings_item_musicinfo;
settings_item_typedef settings_item_prehalve;

const icon_ptr_typedef music_icon[] = {onpu_22x22, onpu_22x22_alpha};
const icon_ptr_typedef folder_icon[] = {folder_22x22, folder_22x22_alpha};
const icon_ptr_typedef card_icon[] = {_binary_card_22x22_bin_start, _binary_card_22x22_alpha_bin_start};
const icon_ptr_typedef cpu_icon[] = {_binary_cpu_22x22_bin_start, _binary_cpu_22x22_alpha_bin_start};
const icon_ptr_typedef display_icon[] = {_binary_display_22x22_bin_start, _binary_display_22x22_alpha_bin_start};
const icon_ptr_typedef debug_icon[] = {_binary_debug_22x22_bin_start, _binary_debug_22x22_alpha_bin_start};
const icon_ptr_typedef info_icon[] = {_binary_info_22x22_bin_start, _binary_info_22x22_alpha_bin_start};


const settings_list_typedef settings_root_list[] = {
		{"..", NULL, 0, NULL, },
		{"About Motion Player", info_icon, 1, NEXT_LIST(settings_about_motionplayer_list), SETTING_ABOUT_MOTIONPLAYER},
		{"Card", card_icon, 4, NEXT_LIST(settings_card_list)},
		{"CPU", cpu_icon, 2, NEXT_LIST(settings_cpu_list)},
		{"Display", display_icon, 3, NEXT_LIST(settings_display_list)},
		{"Debug", debug_icon, 2, NEXT_LIST(settings_debug_list)},
		{"Filer", folder_icon, 2, NEXT_LIST(settings_filer_list)},
		{"Music", music_icon, 4, NEXT_LIST(settings_music_list)},
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
		{"Sort Items", NULL, 3, NEXT_LIST(settings_sort_list), NULL, SETTING_TYPE_ITEM, &settings_item_sort},
};

const settings_list_typedef settings_sort_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_filer_list)},
		{"Enable", NULL, 0, NULL, NULL, 0, &settings_item_sort},
		{"Disable", NULL, 0, NULL, NULL, 0, &settings_item_sort},
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
};

const settings_list_typedef settings_cpufreq_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_cpu_list)},
		{"72MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"100MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"120MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"168MHz", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"200MHz OC", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
		{"225MHz OC", NULL, 0, NULL, NULL, 0, &settings_item_cpu},
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


	/* Filer Sort Item */
	static const unsigned int sort_tbl[] = {1, 0};
#define SORT_TABLE_ITEMS (sizeof(sort_tbl) / sizeof(sort_tbl[0]))
	settings_group.filer_conf.sort = validate_saved_val(settings_group.filer_conf.sort, 1, sort_tbl, SORT_TABLE_ITEMS);
	settings_item_sort.selected_id = selected_id(settings_group.filer_conf.sort, sort_tbl, SORT_TABLE_ITEMS);
	settings_item_sort.item_count = SORT_TABLE_ITEMS;
	settings_item_sort.item_array = sort_tbl;
	settings_item_sort.func = SETTINGS_FILER_SORT;

	/* Init USART Baudrate Item */
	static const unsigned int baudrate_tbl[] = {9600, 19200, 38400, 76800, 115200, 230400, 460800, 921600};
#define BAUDRATE_TABLE_ITEMS (sizeof(baudrate_tbl) / sizeof(baudrate_tbl[0]))
	settings_group.debug_conf.baudrate = validate_saved_val(settings_group.debug_conf.baudrate, 115200, baudrate_tbl, BAUDRATE_TABLE_ITEMS);
	settings_item_usart.selected_id = selected_id(settings_group.debug_conf.baudrate, baudrate_tbl, BAUDRATE_TABLE_ITEMS);
	settings_item_usart.item_count = BAUDRATE_TABLE_ITEMS;
	settings_item_usart.item_array = baudrate_tbl;
	settings_item_usart.func = SETTINGS_BAUDRATE;

	/* Init CPU Frequency Item */
	static const unsigned int cpu_freq_tbl[] = {72, 100, 120, 168, 200, 225, 250};
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

void *SETTING_DISPLAY_CARDINFO(void *arg)
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
	sprintf(s, "Cluster Size:%dKB\n", (fat.sectorsPerCluster * 512) / 1024);
	LCDPutString(s, WHITE);

	sprintf(s, "\nSpec Version:%s\n", &specVer[cardInfo.specVer][0]);
	LCDPutString(s, WHITE);

	sprintf(s, "High Capacity:%s\n", cardInfo.csdVer ? "Yes" : "No");
	LCDPutString(s, WHITE);

	if(cardInfo.speedClass){
		sprintf(s, "Speed Class:CLASS%d\n", cardInfo.speedClass);
	} else {
		strcpy(s, "Speed Class:N/A\n");
	}
	LCDPutString(s, WHITE);

	sprintf(s, "Supported Bus Widths:%s\n", &busWidth[cardInfo.busWidth][0]);
	LCDPutString(s, WHITE);

	sprintf(s, "Max Transfer Speed Per Bus:%dMbit/s\n", cardInfo.tranSpeed / 1000);
	LCDPutString(s, WHITE);

	sprintf(s, "Max Clock Frequency:%dMHz\n", cardInfo.maxClkFreq);
	LCDPutString(s, WHITE);

	sprintf(s, "Total Blocks:%d\n", cardInfo.totalBlocks);
	LCDPutString(s, WHITE);

	sprintf(s, "Card Capacity:%0.2fGB\n", (float)(((float)cardInfo.totalBlocks / 1000000000UL) * 512));
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
		sprintf(s, "%d:%dKB/s" , i, (count * (blocksize * 512)) / 1000);
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

void *SETTING_ABOUT_MOTIONPLAYER(void *arg)
{
	char s[30];

	LCDGotoXY(0, 50);

	sprintf(s, "Version: %d.%d (%s)\n\n", VERSION_MAJOR, VERSION_MINOR, __DATE__);
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

	SETTINGS_Save();

	SystemInit();
	SystemCoreClockUpdate();

	USARTInit();

	debug.printf("\r\nSystemCoreClock:%d", SystemCoreClock);

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

void *SETTINGS_FILER_SORT(void *arg)
{
	settings_item_typedef *sort_item = (settings_item_typedef*)arg;

	settings_group.filer_conf.sort = sort_item->item_array[sort_item->selected_id];

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
