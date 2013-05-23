/*
 * fft.h
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#ifndef FFT_H_
#define FFT_H_

#include <stdint.h>
#include "arm_math.h"
#include "icon.h"

typedef struct {
	q31_t left_inbuf[128], right_inbuf[128], outbuf[128];
	uint32_t samples;
	uint16_t length;
	uint8_t ifftFlag, bitReverseFlag, rightShift;
	arm_status status;
	arm_cfft_radix4_instance_q31 S;
}FFT_Struct_Typedef;


#define FFT_ANALYZER_LEFT_POS_X 55
#define FFT_ANALYZER_LEFT_POS_Y 190
#define FFT_ANALYZER_RIGHT_POS_X 228
#define FFT_ANALYZER_RIGHT_POS_Y 190

extern void FFT_Init(FFT_Struct_Typedef *FFT);
extern void FFT_Sample(FFT_Struct_Typedef *FFT, uint32_t *pSrc);
extern void FFT_Display_Left(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color);
extern void FFT_Display_Right(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color);

#endif /* FFT_H_ */
