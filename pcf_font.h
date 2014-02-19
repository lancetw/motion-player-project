/*
 * pcf_font.h
 *
 *  Created on: 2012/03/12
 *      Author: Tonsuke
 */

#ifndef PCF_FONT_H_
#define PCF_FONT_H_

#include "stm32f4xx_conf.h"
#include "lcd.h"
#include "fat.h"

static const char type[][21] = {
		"PCF_PROPERTIES", \
		"PCF_ACCELERATORS", \
		"PCF_METRICS", \
		"PCF_BITMAPS", \
		"PCF_INK_METRICS", \
		"PCF_BDF_ENCODINGS", \
		"PCF_SWIDTHS", \
		"PCF_GLYPH_NAMES", \
		"PCF_BDF_ACCELERATORS",
};

#define PCF_PROPERTIES			(1 << 0)
#define PCF_ACCELERATORS		(1 << 1)
#define PCF_METRICS				(1 << 2)
#define PCF_BITMAPS				(1 << 3)
#define PCF_INK_METRICS			(1 << 4)
#define	PCF_BDF_ENCODINGS		(1 << 5)
#define PCF_SWIDTHS				(1 << 6)
#define PCF_GLYPH_NAMES			(1 << 7)
#define PCF_BDF_ACCELERATORS	(1 << 8)

#define C_FONT_UNDEF_CODE       0x0080

typedef struct toc_entry {
	uint32_t type, \
			 format, \
			 size, \
			 offset;
} toc_entry;

typedef struct metric_data_typedef {
	int8_t left_sided_bearing, \
			right_sided_bearing, \
			character_width, \
			character_ascent, \
			character_descent;
}metric_data_typedef;

typedef struct encode_info_typedef {
	uint16_t min_char_or_byte2, \
			 max_char_or_byte2, \
			 min_byte1, \
			 max_byte1, \
			 default_char;
}encode_info_typedef;

typedef struct metrics_table_typedef {
	uint32_t size, offset;
	MY_FILE fp;
}metrics_table_typedef;

typedef struct bitmap_table_typedef {
	uint32_t size, offset;
	MY_FILE fp_offset, fp_bitmap;
}bitmap_table_typedef;

typedef struct encode_table_typedef {
	uint32_t size, offset, glyphindeces;
	MY_FILE fp;
}encode_table_typedef;

typedef struct cache_typedef {
	void *start_address;
	int glyph_count;
}cache_typedef;

typedef struct metrics {
	uint8_t hSpacing;
}metrics_typedef;

#define PCF_METRICS_DEFAULT_HSPACING 2

volatile struct {
	uint32_t table_count;
	metrics_table_typedef metrics_tbl;
	bitmap_table_typedef bitmap_tbl;
	encode_table_typedef enc_tbl;
	encode_info_typedef enc_info;
	cache_typedef cache;
	metrics_typedef metrics;
	int8_t c_loaded;
}pcf_font;


static const int bit_count_table[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

static const float color_tone_table_4bit[] = {
		0,
		0.0625,
		0.125,
		0.1875,
		0.25,
		0.3125,
		0.375,
		0.4375,
		0.5,
		0.5625,
		0.625,
		0.6875,
		0.75,
		0.8125,
		0.875,
		0.9372,
		1.0
};

static const float color_tone_table_3bit[] = {
		0,
		0.111,
		0.222,
		0.333,
		0.444,
		0.555,
		0.666,
		0.777,
		0.888,
		1.0
};

extern const char internal_flash_pcf_font[];
extern char _sizeof_internal_flash_pcf_font[];

typedef struct{
	uint16_t code, size;
	metric_data_typedef metric;
	int width;
}pcf_glyph_cache_head_typedef;


extern int PCFFontInit(int id);
extern void PCFPutChar(uint16_t code, colors color);
extern void PCFPutChar16px(uint16_t code, colors color);
extern void PCFSetGlyphCacheStartAddress(void *addr);
extern void PCFCachePlayTimeGlyphs(uint8_t px);
extern void PCFCacheGlyph(uint16_t code, uint16_t font_width);
extern void PCFPutCharCache(uint16_t code, colors color);
extern void PCFPutString(const uint16_t *uni_str, int n, colors color);
extern uint16_t PCFGetCharPixelLength(uint16_t code, uint16_t font_width);

extern int C_PCFFontInit(uint32_t fileAddr, size_t fileSize);
extern void C_PCFPutChar(uint16_t code, colors color);
extern void C_PCFPutChar16px(uint16_t code, colors color);
extern void C_PCFPutString(const uint16_t *uni_str, int n, colors color);
extern void C_PCFPutString16px(const uint16_t *uni_str, int n, colors color);
extern uint16_t C_PCFGetCharPixelLength(uint16_t code, uint16_t font_width);


#endif /* PCF_FONT_H_ */
