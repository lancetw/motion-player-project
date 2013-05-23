/*
 * aac.h
 *
 *  Created on: 2012/03/27
 *      Author: Tonsuke
 */

#ifndef AAC_H_
#define AAC_H_

#include "stm32f4xx_conf.h"
#include "mjpeg.h"

typedef struct media_info_typedef {
	media_sound sound;
	sound_format format;
	esds_format bitrate;
} media_info_typedef;

extern int PlayAAC(int id);

#endif /* AAC_H_ */
