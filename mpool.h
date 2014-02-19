/*
 * mpool.h
 *
 *  Created on: 2011/05/14
 *      Author: Tonsuke
 */

#ifndef MPOOL_H_
#define MPOOL_H_

#include "stm32f4xx_conf.h"
#include "fat.h"

#define CCM_BASE ((uint32_t)(0x10000000)) /* Core Coupled Memory Base Address 64KB */

volatile struct {
	uint32_t mem_seek;
}mpool_struct;

uint8_t mempool[38000];

extern void create_mpool();
extern void* mpool_alloc(uint32_t sizeofmemory);
extern void mpool_destroy();
extern void* jmemread(MY_FILE *fp, size_t *nbytes, int32_t n);




#endif /* MPOOL_H_ */
