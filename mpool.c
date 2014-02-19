/*
 * mpool.c
 *
 *  Created on: 2011/05/14
 *      Author: Tonsuke
 */

#include "mpool.h"
#include "fat.h"
#include <stdlib.h>

void create_mpool(){
	mpool_struct.mem_seek = 0;
}

void* mpool_alloc(uint32_t sizeofmemory){
	uint32_t current_seek;

	current_seek = mpool_struct.mem_seek;
	mpool_struct.mem_seek += sizeofmemory;

	return (void*)(mempool + current_seek);
}

void mpool_destroy(){
}

void* jmemread(MY_FILE *fp, size_t *nbytes, int32_t n)
{
	if(n <= 0){
		*nbytes = 0;
		return 0;
	}

	void *ret_buf;

	if(fp->fileSize < (fp->seekBytes + n)){
		n = fp->fileSize - fp->seekBytes;
	}

	ret_buf = (void*)((char*)CCM_BASE + fp->seekBytes);

	fp->seekBytes += n;
	*nbytes = n;

	return ret_buf;
}
