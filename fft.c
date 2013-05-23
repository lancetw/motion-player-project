/*
 * fft.c
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#include "fft.h"

#include "settings.h"

/*
const static uint16_t color_bar[] = {red, yellow, blue, blue, green, green, red, red, yellow, yellow, blue, blue, green, green, red, red,\
									 red, yellow, blue, blue, green, green, red, red, yellow, yellow, blue, blue, green, green, red, red};
*/
//const static uint16_t skyblue_bar[] = {0xef7d, 0xe75d, 0xe75d, 0xdf5d, 0xdf5d, 0xd73d, 0xd73d, 0xcf3d, 0xcf3d, 0xc71d, 0xc71d, 0xc71d, 0xbf1d, 0xbf1d, 0xb6fd, 0xb6fd, 0xaefe, 0xaefe, 0xa6de, 0xa6de, 0x9ede, 0x9ede, 0x9ede, 0x96be, 0x96be, 0x8ebe, 0x8ebe, 0x869e, 0x869e, 0x7e9e, 0x7e9e, 0x7e9f};
//const static uint16_t skyblue_bar[] = {0xef7d, 0xef3c, 0xeefb, 0xeeba, 0xee79, 0xee38, 0xedf7, 0xedb6, 0xed75, 0xed34, 0xecf3, 0xecd2, 0xec91, 0xec50, 0xec0f, 0xebce, 0xf38e, 0xf34d, 0xf30c, 0xf2cb, 0xf28a, 0xf269, 0xf228, 0xf1e7, 0xf1a6, 0xf165, 0xf124, 0xf0e3, 0xf0a2, 0xf061, 0xf020, 0xf800};

const static uint16_t color_bar[7][32] = {
		{0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, // white
				0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d, 0xef7d}, \

		{0xef7d, 0xe75d, 0xe75d, 0xdf5d, 0xdf5d, 0xd73d, 0xd73d, 0xcf3d, 0xcf3d, 0xc71d, 0xc71d, 0xc71d, 0xbf1d, 0xbf1d, 0xb6fd, 0xb6fd, // skyblue
				0xaefe, 0xaefe, 0xa6de, 0xa6de, 0x9ede, 0x9ede, 0x9ede, 0x96be, 0x96be, 0x8ebe, 0x8ebe, 0x869e, 0x869e, 0x7e9e, 0x7e9e, 0x7e9f}, \

		{0xef7d, 0xef7c, 0xef7b, 0xef7a, 0xef79, 0xef78, 0xef77, 0xef76, 0xef95, 0xef94, 0xef93, 0xef92, 0xef91, 0xef90, 0xef8f, 0xef8e, // yellow
				0xf7ae, 0xf7ad, 0xf7ac, 0xf7ab, 0xf7aa, 0xf7a9, 0xf7a8, 0xf7a7, 0xf7c6, 0xf7c5, 0xf7c4, 0xf7c3, 0xf7c2, 0xf7c1, 0xf7c0, 0xffe0},

		{0xef7d, 0xe77c, 0xdf7b, 0xd77a, 0xcf79, 0xc778, 0xbf77, 0xb776, 0xaf95, 0xa794, 0x9f93, 0x9792, 0x8f91, 0x8790, 0x7f8f, 0x778e, // green
				0x77ae, 0x6fad, 0x67ac, 0x5fab, 0x57aa, 0x4fa9, 0x47a8, 0x3fa7, 0x37c6, 0x2fc5, 0x27c4, 0x1fc3, 0x17c2, 0x0fc1, 0x07c0, 0x07e0}, \

		{0xef7d, 0xef3c, 0xeefb, 0xeeba, 0xee79, 0xee38, 0xedf7, 0xedb6, 0xed75, 0xed34, 0xecf3, 0xecd2, 0xec91, 0xec50, 0xec0f, 0xebce, // red
				0xf38e, 0xf34d, 0xf30c, 0xf2cb, 0xf28a, 0xf269, 0xf228, 0xf1e7, 0xf1a6, 0xf165, 0xf124, 0xf0e3, 0xf0a2, 0xf061, 0xf020, 0xf800}, \

		{0xef7d, 0xe73c, 0xdefb, 0xd6ba, 0xce79, 0xc638, 0xbdf7, 0xb5b6, 0xad75, 0xa534, 0x9cf3, 0x94d2, 0x8c91, 0x8450, 0x7c0f, 0x73ce, // black
				0x738e, 0x6b4d, 0x630c, 0x5acb, 0x528a, 0x4a69, 0x4228, 0x39e7, 0x31a6, 0x2965, 0x2124, 0x18e3, 0x10a2, 0x0861, 0x0020, 0x0000}, \

		{red, yellow, blue, blue, green, green, red, red, yellow, yellow, blue, blue, green, green, red, red,  // colorful
											 red, yellow, blue, blue, green, green, red, red, yellow, yellow, blue, blue, green, green, red, red}, \
};

void FFT_Init(FFT_Struct_Typedef *FFT)
{
	FFT->status = arm_cfft_radix4_init_q31(&FFT->S, FFT->length, FFT->ifftFlag, FFT->bitReverseFlag);

	if(settings_group.music_conf.b.prehalve){
		FFT->rightShift = 21;
	} else {
		FFT->rightShift = 22;
	}
}

void FFT_Sample(FFT_Struct_Typedef *FFT, uint32_t *pSrc)
{
	int idx = 0, i;
	uint32_t sample[1];

	for(i = 0;i < FFT->samples;i += (FFT->samples / FFT->length)){
		sample[0] = pSrc[i] ^ 0x80008000; // unsigned to signed

		FFT->left_inbuf[idx] = (*(q31_t*)&sample[0] << 16) & 0xffff0000; // Real Left
		FFT->left_inbuf[idx + 1] = 0; // Imag

		FFT->right_inbuf[idx] = *(q31_t*)&sample[0] & 0xffff0000; // Real Right
		FFT->right_inbuf[idx + 1] = 0; // Imag
 		idx += 2;
	 }
}

void FFT_Display_Left(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color)
{
	int power, i, j;
	fft_analyzer_typedef fftDrawBuff;
	extern settings_group_typedef settings_group;

	arm_cfft_radix4_q31(&FFT->S, FFT->left_inbuf);
	arm_cmplx_mag_q31(FFT->left_inbuf, FFT->outbuf, FFT->length >> 1);

	memcpy(&fftDrawBuff, &drawBuff->fft_analyzer_left, sizeof(fft_analyzer_typedef));

	switch(settings_group.music_conf.b.fft_bar_type){
	case 0: // Solid
		for(i = 0;i < FFT->length >> 1;i++){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 1: // V Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 2: // H Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j += 2){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 3: // Wide
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	default:
		break;
	}

	LCDPutBuffToBgImg(fftDrawBuff.x, fftDrawBuff.y, \
			fftDrawBuff.width, fftDrawBuff.height, fftDrawBuff.p);
}


void FFT_Display_Right(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color)
{
	int power, i, j;
	fft_analyzer_typedef fftDrawBuff;
	extern settings_group_typedef settings_group;

	arm_cfft_radix4_q31(&FFT->S, FFT->right_inbuf);
	arm_cmplx_mag_q31(FFT->right_inbuf, FFT->outbuf, FFT->length >> 1);

	memcpy(&fftDrawBuff, &drawBuff->fft_analyzer_right, sizeof(fft_analyzer_typedef));

	switch(settings_group.music_conf.b.fft_bar_type){
	case 0: // Solid
		for(i = 0;i < FFT->length >> 1;i++){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 1: // V Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 2: // H Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j += 2){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 3: // Wide
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	default:
		break;
	}

	LCDPutBuffToBgImg(fftDrawBuff.x, fftDrawBuff.y, \
			fftDrawBuff.width, fftDrawBuff.height, fftDrawBuff.p);
}

