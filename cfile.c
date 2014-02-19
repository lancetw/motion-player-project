/*
 * fat.c
 *
 *  Created on: 2011/02/27
 *      Author: Tonsuke
 */

#include "cfile.h"
#include <stdlib.h>
#include <string.h>

C_FILE* c_fopen(uint32_t fileAddr, size_t fileSize)
{
	C_FILE *fp = malloc(sizeof(C_FILE));

	fp->c_file_addr = fileAddr;
	fp->seekBytes = fileAddr;
	fp->fileSize = fileSize;

	return fp;
}

void c_fclose(C_FILE *fp)
{
	free((void*)fp);
	fp = '\0';
}

int c_fseek(C_FILE *fp, int64_t offset, int whence)
{
	switch (whence) {
		case SEEK_SET:
			fp->seekBytes = fp->c_file_addr + offset;
			if(offset == 0){
				fp->seekBytes = fp->c_file_addr;
				return 0;
			}
			break;
		case SEEK_CUR:
			fp->seekBytes += offset;
			break;
		case SEEK_END:
			fp->seekBytes = fp->c_file_addr + fp->fileSize + offset - 1;
			break;
		default:
			return 1;
	}

	if((fp->seekBytes - fp->c_file_addr) >= fp->fileSize){
		return 0;
	}

	return 1;
}


size_t c_fread(void *buf, size_t size, size_t count, C_FILE *fp)
{
	size_t n = size * count;

	if(n <= 0){
		return 0;
	}

	if(fp->fileSize < ((fp->seekBytes - fp->c_file_addr) + n)){ // 要求サイズがファイルサイズを超えていた場合
		n = fp->fileSize - (fp->seekBytes - fp->c_file_addr);
	}

	memcpy((void*)buf, (void*)fp->seekBytes, n);

	fp->seekBytes += n;

	return ( size == 1 ? n : (n / size) );
}


