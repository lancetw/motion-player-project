/*
 * fat.h
 *
 *  Created on: 2011/02/27
 *      Author: Tonsuke
 */

#ifndef C_FILE_H_
#define C_FILE_H_

#include "stm32f4xx_conf.h"
#include <stddef.h>


#define	SEEK_SET 0
#define	SEEK_CUR 1
#define	SEEK_END 2


typedef volatile struct C_FILE {
	size_t 	 fileSize, \
           	 seekBytes, \
           	 c_file_addr;
} C_FILE;

extern C_FILE* c_fopen(uint32_t fileAddr, size_t fileSize);
extern void c_fclose(C_FILE *fp);
extern int c_fseek(C_FILE *fp, int64_t offset, int whence);
extern size_t c_fread(void *buf, size_t size, size_t count, C_FILE *fp);




#endif /* FAT_H_ */
