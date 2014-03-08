/*
 * mjpeg.h
 *
 *  Created on: 2011/07/10
 *      Author: Tonsuke
 */

#ifndef MJPEG_H_
#define MJPEG_H_

#include "stm32f4xx_conf.h"
#include <stddef.h>
#include "fat.h"
#include "mpool.h"


MY_FILE fp_global;

struct {
	int buf_type;
	uint32_t frame_size;
}jpeg_read;


#define CMARK 0xA9

static uint8_t atomHasChild[] = {0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0 ,0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ATOM_ITEMS (sizeof(atomHasChild) / sizeof(atomHasChild[0]))

static const uint8_t atomTypeString[ATOM_ITEMS][5] =
{
	"ftyp", // -
	"wide", // -
	"mdat", // -
	"moov", // +
	"mvhd", // -
	"trak", // +
	"tkhd", // -
	"tapt", // +
	"clef", // -
	"prof", // -
	"enof", // -
	"edts", // +
	"elst", // -
	"mdia", // +
	"mdhd", // -
	"hdlr", // -
	"minf", // +
	"vmhd", // -
	"smhd", // -
	"dinf", // +
	"dref", // -
	"stbl", // +
	"stsd", // -
	"stts", // -
	"stsc", // -
	"stsz", // -
	"stco", // -
	"udta", // +
	"free", // -
	"skip", // -
	"meta", // +
	"load", // -
	"iods", // -
	"ilst", // +
	"keys", // -
	"data", // -
	"trkn", // +
	"disk", // +
	"cpil", // +
	"pgap", // +
	"tmpo", // +
	"gnre", // +
	"covr", // -
	{CMARK, 'n', 'a', 'm', '\0'}, // -
	{CMARK, 'A', 'R', 'T', '\0'}, // -
	{CMARK, 'a', 'l', 'b', '\0'}, // -
	{CMARK, 'g', 'e', 'n', '\0'}, // -
	{CMARK, 'd', 'a', 'y', '\0'}, // -
	{CMARK, 't', 'o', 'o', '\0'}, // -
	{CMARK, 'w', 'r', 't', '\0'}, // -
	"----", // -
};

enum AtomEnum {
	FTYP, // -
	WIDE, // -
	MDAT, // -
	MOOV, // +
	MVHD, // -
	TRAK, // +
	TKHD, // -
	TAPT, // +
	CLEF, // -
	PROF, // -
	ENOF, // -
	EDTS, // +
	ELST, // -
	MDIA, // +
	MDHD, // -
	HDLR, // -
	MINF, // +
	VMHD, // -
	SMHD, // -
	DINF, // +
	DREF, // -
	STBL, // +
	STSD, // -
	STTS, // -
	STSC, // -
	STSZ, // -
	STCO, // -
	UDTA, // +
	FREE, // -
	SKIP, // -
	META, // +
	LOAD, // -
	IODS, // -
	ILST, // +
	KEYS, // -
	DATA, // -
	TRKN, // +
	DISK, // +
	CPIL, // +
	PGAP, // +
	TMPO, // +
	GNRE, // +
	COVR, // -
	CNAM, // -
	CART, // -
	CALB, // -
	CGEN, // -
	CDAY, // -
	CTOO, // -
	CWRT, // -
	NONE, // -
};

volatile struct {
	int8_t id, \
		   done, \
		   resynch, \
		   draw_icon;
	uint32_t paused_chunk, \
			 resynch_entry;
} mjpeg_touch;


struct {
	uint32_t *firstChunk, *prevChunk, *prevSamples, *samples, *totalSamples, *videoStcoCount;
	MY_FILE *fp_video_stsc, *fp_video_stsz, *fp_video_stco, *fp_frame;
} pv_src;

struct {
	uint32_t *firstChunk, *prevChunk, *prevSamples, *samples, *soundStcoCount;
	MY_FILE *fp_sound_stsc, *fp_sound_stsz, *fp_sound_stco;
} ps_src;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stts;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stsc;

struct{
	uint32_t sampleSize,
	         numEntry;
	MY_FILE fp;
} sound_stsz;


struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stco;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stts;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stsc;

struct{
	uint32_t sampleSize,
	         numEntry;
	MY_FILE fp;
} video_stsz;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stco;


typedef struct samples_struct{
	uint32_t numEntry;
	MY_FILE *fp;
} samples_struct;

typedef struct sound_flag{
	uint32_t process, complete;
} sound_flag;

typedef struct video_flag{
	uint32_t process, complete;
} video_flag;

typedef struct sound_format{
	char audioFmtString[4];
	uint8_t reserved[10];
	uint16_t version,
			 revision,
	 	 	 vendor,
	 	 	 numChannel,
	 	 	 sampleSize,
	 	 	 complesionID,
	 	 	 packetSize,
	 	 	 sampleRate,
			 reserved1;
} sound_format;

typedef struct esds_format{
	char esdsString[4];
//	uint8_t reserved[22];
	uint32_t maxBitrate, avgBitrate;
} esds_format;


typedef struct media_sound{
	sound_flag flag;
	sound_format format;
	uint32_t timeScale,
	         duration;
} media_sound;

typedef struct media_video{
	video_flag flag;
	uint32_t timeScale,
	         duration;
	int16_t width, height, frameRate, startPosX, startPosY;
	char videoFmtString[5], videoCmpString[15], playJpeg;
} media_video;

volatile struct{
	media_sound sound;
	media_video video;
} media;

typedef struct {
	int output_scanline, frame_size, rasters, buf_size;
	uint16_t *p_raster;
} raw_video_typedef;


/* --- TIM1 SR Register ---*/
/* Alias word address of TIM1 SR UIF bit */
#define TIM1_SR_OFFSET         (TIM1_BASE + 0x10)
#define TIM1_SR_UIF_BitNumber  0x00
#define TIM1_SR_UIF_BB         (*(uint32_t *)(PERIPH_BB_BASE + (TIM1_SR_OFFSET * 32) + (TIM1_SR_UIF_BitNumber * 4)))

/* --- TIM3 CR1 Register ---*/
/* Alias word address of TIM3 CR1 CEN bit */
#define TIM3_CR1_OFFSET         (TIM3_BASE + 0x00)
#define TIM3_CR1_CEN_BitNumber  0x00
#define TIM3_CR1_CEN_BB         (*(uint32_t *)(PERIPH_BB_BASE + (TIM3_CR1_OFFSET * 32) + (TIM3_CR1_CEN_BitNumber * 4)))

/* --- TIM3 DIER Register ---*/
/* Alias word address of TIM3 DIER UIE bit */
#define TIM3_DIER_OFFSET         (TIM3_BASE + 0x0C)
#define TIM3_DIER_UIE_BitNumber  0x00
#define TIM3_DIER_UIE_BB         (*(uint32_t *)(PERIPH_BB_BASE + (TIM3_DIER_OFFSET * 32) + (TIM3_DIER_UIE_BitNumber * 4)))

/* --- TIM3 SR Register ---*/
/* Alias word address of TIM3 SR UIF bit */
#define TIM3_SR_OFFSET         (TIM3_BASE + 0x10)
#define TIM3_SR_UIF_BitNumber  0x00
#define TIM3_SR_UIF_BB	       (*(uint32_t *)(PERIPH_BB_BASE + (TIM3_SR_OFFSET * 32) + (TIM3_SR_UIF_BitNumber * 4)))

extern uint32_t b2l(void* val, size_t t);
//inline uint32_t getAtomSize(void* atom);
extern int PlayMotionJpeg(int id);

/*
typedef struct
{
  uint8_t mem[50000];
}ExtJpgMem_TypeDef;

#define JPG ((ExtJpgMem_TypeDef*)(0x600030C0))
*/


#endif /* MJPEG_H_ */
