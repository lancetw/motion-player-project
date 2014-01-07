/*
 * fx.h
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#ifndef FX_H_
#define FX_H_

#include <stdint.h>
#include <stddef.h>


typedef struct {
	uint32_t *ptr;
	int idx;
	size_t size;
}delay_buffer_typedef;

typedef struct {
	int repeat[4], delay[4], fs, number;
	float amp[4], clipper[4], coeff_table[4][10];
	delay_buffer_typedef *delay_buffer;
	size_t num_blocks;
}REVERB_Struct_Typedef;

typedef struct {
	int number, fs;
	float fc, Q, g, a[3], b[3], A[4][3], B[4][3];
	delay_buffer_typedef *delay_buffer;
	size_t num_blocks, sbuf_size;
}IIR_Filter_Struct_Typedef;


extern void REVERB_Set_Prams(REVERB_Struct_Typedef *RFX);
extern void REVERB_Init(REVERB_Struct_Typedef *RFX, int n);
extern void REVERB(REVERB_Struct_Typedef *RFX, uint32_t *out_ptr);
extern void IIR_Set_Params(IIR_Filter_Struct_Typedef *IIR);
extern void IIR_Filter(IIR_Filter_Struct_Typedef *IIR, uint32_t *out_ptr, float *sout_ptr_A, float *sout_ptr_B);
extern void IIR_resonator(float fc, float Q, float a[], float b[]);
extern void IIR_LPF(float fc, float Q, float a[], float b[]);
extern void IIR_HPF(float fc, float Q, float a[], float b[]);
extern void IIR_low_shelving(float fc, float Q, float g, float a[], float b[]);
extern void IIR_peaking(float fc, float Q, float g, float a[], float b[]);

//float fc, fcv, Q, g, a[3], b[3], A[10][3], B[10][3];



#endif /* FX_H_ */
