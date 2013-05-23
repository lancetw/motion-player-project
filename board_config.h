/*
 * board_config.h
 *
 *  Created on: 2013/02/07
 *      Author: Tonsuke
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#include "stm32f4xx_conf.h"

#define FLASH_SECTOR_1_OFFSET (0x4000)
#define FLASH_SECOTR_1_ADDR (FLASH_BASE + FLASH_SECTOR_1_OFFSET)
#define FLASH_SECTOR_1_SIZE (16384)

#define FLASH_SETTING_OFFSET (0x3000)
#define FLASH_SETTING_BASE (FLASH_BASE + FLASH_SECTOR_1_OFFSET + FLASH_SETTING_OFFSET)
#define FLASH_SETTING_SIZE (4096)
#define	TP_CAL_OFFSET 0
#define GET_VAL_FLASH_SETTING(type, offset) (*(type*)(FLASH_SETTING_BASE + offset))


/* XPT2046_CS_BB */
/* --- GPIOA ODR Register ---*/
/* Alias word address of GPIOA ODR 15 bit */
#define GPIOA_ODR_OFFSET       (GPIOA_BASE + 0x14)
#define GPIOA_ODR_15_BitNumber  0x0F
#define GPIOA_ODR_15_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (GPIOA_ODR_OFFSET * 32) + (GPIOA_ODR_15_BitNumber * 4)))
#define XPT2046_CS_BB GPIOA_ODR_15_BB

#define XPT2046_CS_DEASSERT (XPT2046_CS_BB = 1)
#define XPT2046_CS_ASSERT   (XPT2046_CS_BB = 0)


/* TP_PEN_INPUT_BB */
/* --- GPIOC IDR Register ---*/
/* Alias word address of GPIOC IDR 4 bit */
#define GPIOC_IDR_OFFSET       (GPIOC_BASE + 0x10)
#define GPIOC_IDR_4_BitNumber  0x04
#define GPIOC_IDR_4_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (GPIOC_IDR_OFFSET * 32) + (GPIOC_IDR_4_BitNumber * 4)))
#define TP_PEN_INPUT_BB GPIOC_IDR_4_BB


/* AUDIO_OUT_BB */
/* --- GPIOC ODR Register ---*/
/* Alias word address of GPIOC ODR 5 bit */
#define GPIOC_ODR_OFFSET       (GPIOC_BASE + 0x14)
#define GPIOC_ODR_5_BitNumber  0x05
#define GPIOC_ODR_5_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (GPIOC_ODR_OFFSET * 32) + (GPIOC_ODR_5_BitNumber * 4)))
#define AUDIO_OUT_BB GPIOC_ODR_5_BB

#define AUDIO_OUT_SHUTDOWN 	(AUDIO_OUT_BB = 0)
#define AUDIO_OUT_ENABLE 	(AUDIO_OUT_BB = 1)


/* SOUND_DMA_HALF_TRANS_BB */
/* --- DMA1 LISR Register ---*/
/* Alias word address of DMA1 LISR DMA_LISR_HTIF1 bit */
#define DMA1_LISR_OFFSET       (DMA1_BASE + 0x0)
#define DMA1_LISR_DMA_LISR_HTIF1_BitNumber  10
#define DMA1_LISR_DMA_LISR_HTIF1_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_LISR_OFFSET * 32) + (DMA1_LISR_DMA_LISR_HTIF1_BitNumber * 4)))
#define SOUND_DMA_HALF_TRANS_BB DMA1_LISR_DMA_LISR_HTIF1_BB


/* SOUND_DMA_CLEAR_HALF_TRANS_BB */
/* --- DMA1 LIFCR Register ---*/
/* Alias word address of DMA1 LIFCR DMA_LIFCR_CHTIF1 bit */
#define DMA1_LIFCR_OFFSET       (DMA1_BASE + 0x8)
#define DMA1_LIFCR_DMA_LIFCR_CHTIF1_BitNumber  10
#define DMA1_LIFCR_DMA_LIFCR_CHTIF1_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_LIFCR_OFFSET * 32) + (DMA1_LIFCR_DMA_LIFCR_CHTIF1_BitNumber * 4)))
#define SOUND_DMA_CLEAR_HALF_TRANS_BB DMA1_LIFCR_DMA_LIFCR_CHTIF1_BB

/* SOUND_DMA_FULL_TRANS_BB */
/* --- DMA1 LISR Register ---*/
/* Alias word address of DMA1 LISR DMA_LISR_TCIF1 bit */
#define DMA1_LISR_OFFSET       (DMA1_BASE + 0x0)
#define DMA1_LISR_DMA_LISR_TCIF1_BitNumber  11
#define DMA1_LISR_DMA_LISR_TCIF1_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_LISR_OFFSET * 32) + (DMA1_LISR_DMA_LISR_TCIF1_BitNumber * 4)))
#define SOUND_DMA_FULL_TRANS_BB DMA1_LISR_DMA_LISR_TCIF1_BB

/* SOUND_DMA_CLEAR_FULL_TRANS_BB */
/* --- DMA1 LIFCR Register ---*/
/* Alias word address of DMA1 LIFCR DMA_LIFCR_CTCIF1 bit */
#define DMA1_LIFCR_OFFSET       (DMA1_BASE + 0x8)
#define DMA1_LIFCR_DMA_LIFCR_CTCIF1_BitNumber  11
#define DMA1_LIFCR_DMA_LIFCR_CTCIF1_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_LIFCR_OFFSET * 32) + (DMA1_LIFCR_DMA_LIFCR_CTCIF1_BitNumber * 4)))
#define SOUND_DMA_CLEAR_FULL_TRANS_BB DMA1_LIFCR_DMA_LIFCR_CTCIF1_BB


#define TRANSFER_IT_ENABLE_MASK (uint32_t)(DMA_SxCR_TCIE | DMA_SxCR_HTIE | \
                                           DMA_SxCR_TEIE | DMA_SxCR_DMEIE)


#define DMA_SOUND_IT_ENABLE (DMA1_Stream1->CR |= (uint32_t)((DMA_IT_TC | DMA_IT_HT) & TRANSFER_IT_ENABLE_MASK))
#define DMA_SOUND_IT_DISABLE (DMA1_Stream1->CR &= ~(uint32_t)((DMA_IT_TC | DMA_IT_HT) & TRANSFER_IT_ENABLE_MASK))


#endif /* BOARD_CONFIG_H_ */
