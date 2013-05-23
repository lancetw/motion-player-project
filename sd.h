/*
 * sd.h
 *
 *  Created on: 2011/02/22
 *      Author: masayuki
 */

#ifndef SD_H_
#define SD_H_

#include "stm32f4xx_conf.h"
#include "fat.h"


/* --- STA Register ---*/
/* Alias word address of RXFIFOHF bit */
#define STA_OFFSET              (SDIO_BASE + 0x34)
#define RXFIFOHF_BitNumber      0x0F
#define STA_RXFIFOHF_BB_FLAG    (*(uint32_t *)(PERIPH_BB_BASE + (STA_OFFSET * 32) + (RXFIFOHF_BitNumber * 4)))

/* Alias word address of RXDAVL bit */
#define RXDAVL_BitNumber      0x15
#define STA_RXDAVL_BB_FLAG    (*(uint32_t *)(PERIPH_BB_BASE + (STA_OFFSET * 32) + (RXDAVL_BitNumber * 4)))


#define SDIO_CMD_STATUS_MASK (SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CCRCFAIL)
#define SDIO_BLOCK_READ_STATUS_MASK (SDIO_FLAG_DATAEND | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_RXOVERR | SDIO_FLAG_STBITERR)

#define	CSD_VER_1XX 0
#define CSD_VER_2XX 1

typedef struct{
	uint8_t speedClass, csdVer, specVer, busWidth;
	uint32_t tranSpeed, maxClkFreq;
	uint32_t rca;
	uint32_t c_size, c_size_mult, read_bl_len, totalBlocks;
}card_info_typedef;

extern volatile card_info_typedef cardInfo;


static const char specVer[3][9] = {"1.0-1.01", "1.10", "2.00"};
static const char busWidth[6][10] = {"1bit", "", "", "", "", "1bit/4bit"};
static const int tranUnit[] = {100, 1000, 10000, 100000};
static const float timeVal[] = {0, 1.0, 1.2, 1.3, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 7.0, 8.0};

extern int SDInit(void);
uint32_t SDSendCMD(int cmdIdx, uint32_t arg, uint32_t resType, uint32_t *resbuf);
extern inline uint32_t SDBlockRead(void *buf, uint32_t blockAddress);
extern inline uint32_t SDMultiBlockRead(void *buf, uint32_t blockAddress, uint32_t count);
extern int SD_Switch_BusWidth(int width);


#endif /* SD_H_ */
