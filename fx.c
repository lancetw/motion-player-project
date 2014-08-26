/*
 * fx.c
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#include <math.h>

#include "fx.h"
#include "sound.h"

#include "arm_math.h"

#include "usart.h"

#define M_PI_F ((float)(M_PI))

void REVERB_Set_Prams(REVERB_Struct_Typedef *RFX)
{
	RFX->amp[0] = 0.0f;
	RFX->delay[0] = 0;
	RFX->repeat[0] = 0;
	REVERB_Init(RFX, 0);

	RFX->amp[1] = 0.5f;
	RFX->delay[1] = RFX->fs * 0.05f;
	RFX->repeat[1] = 3;
	REVERB_Init(RFX, 1);

	RFX->amp[2] = 0.5f;
	RFX->delay[2] = RFX->fs * 0.10f;
	RFX->repeat[2] = 3;
	REVERB_Init(RFX, 2);

	RFX->amp[3] = 0.5f;
	RFX->delay[3] = RFX->fs * 0.30f;
	RFX->repeat[3] = 1;
	REVERB_Init(RFX, 3);
}

void REVERB_Init(REVERB_Struct_Typedef *RFX, int n)
{
	int i;

	RFX->clipper[n] = 1.0f;

	for(i = 1;i <= RFX->repeat[n];i++){
		RFX->clipper[n] += powf(RFX->amp[n], (float)i);
		RFX->coeff_table[n][i] = powf(RFX->amp[n], (float)i);
	}

	RFX->clipper[n] = 1.0f / RFX->clipper[n];

	RFX->delay_buffer->idx = 0;
}


void REVERB(REVERB_Struct_Typedef *RFX, uint32_t *out_ptr)
{
	uint32_t left_in, right_in, left_out, right_out;
	int i, j, m;

	if((RFX->delay_buffer->idx -= RFX->num_blocks) < 0){
		RFX->delay_buffer->idx += RFX->delay_buffer->size;
	}

	for(i = 0;i < RFX->num_blocks;i++){

		left_out  = LOWER_OF_WORD(out_ptr[i]);
		right_out = UPPER_OF_WORD(out_ptr[i]);

		for(j = 1;j <= RFX->repeat[RFX->number];j++){
			m = RFX->delay_buffer->idx - RFX->delay[RFX->number] * j;
			if(m < 0){
				m += RFX->delay_buffer->size;
			}
			left_in  = LOWER_OF_WORD(RFX->delay_buffer->ptr[m]);
			right_in = UPPER_OF_WORD(RFX->delay_buffer->ptr[m]);

			left_out += RFX->coeff_table[RFX->number][j] * left_in;
			right_out += RFX->coeff_table[RFX->number][j] * right_in;
		}

		left_out *= RFX->clipper[RFX->number];
		right_out *= RFX->clipper[RFX->number];

		left_out = __USAT(left_out, 16);
		right_out = __USAT(right_out, 16);

		out_ptr[i] = __PKHBT(left_out, right_out, 16);

		if(++RFX->delay_buffer->idx >= RFX->delay_buffer->size){
			RFX->delay_buffer->idx = 0;
		}
	}
}


void IIR_Set_Params(IIR_Filter_Struct_Typedef *IIR)
{
	int m;

	IIR->fc = 250.0f / IIR->fs; // cut off freq
	IIR->Q = 1.0f / sqrtf(2.0f);

	IIR->g = 0.0f;
	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[0][m] = IIR->a[m];
		IIR->B[0][m] = IIR->b[m];
	}

	IIR->g = 0.6f;
	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[1][m] = IIR->a[m];
		IIR->B[1][m] = IIR->b[m];
	}

	IIR->g = 1.1f;
	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[2][m] = IIR->a[m];
		IIR->B[2][m] = IIR->b[m];
	}

	IIR->g = 2.0f;
	IIR_low_shelving(IIR->fc, IIR->Q, IIR->g, IIR->a, IIR->b);

	for (m = 0; m <= 2; m++)
	{
		IIR->A[3][m] = IIR->a[m];
		IIR->B[3][m] = IIR->b[m];
	}
}

void IIR_Filter(IIR_Filter_Struct_Typedef *IIR, uint32_t *out_ptr, float *sout_ptr_A, float *sout_ptr_B)
{

	float Sleft_in, Sright_in, Sleft_out, Sright_out;
	int i, sub, m;
	uint32_t left_out, right_out;

	if((IIR->delay_buffer->idx -= IIR->num_blocks) < 0){
		IIR->delay_buffer->idx += IIR->delay_buffer->size;
	}

	for(i = 0;i < IIR->num_blocks;i++){

		Sleft_out = 0;
		Sright_out = 0;

		for (m = 0; m <= 2; m++)
		{
			sub = IIR->delay_buffer->idx - m;
			if(sub < 0){
				sub += IIR->delay_buffer->size;
			}
			Sleft_in  = (float)((short)(LOWER_OF_WORD(IIR->delay_buffer->ptr[sub]) ^ 0x8000)) / 32768.0f;
			Sright_in = (float)((short)(UPPER_OF_WORD(IIR->delay_buffer->ptr[sub]) ^ 0x8000)) / 32768.0f;

			Sleft_out  += IIR->B[IIR->number][m] * Sleft_in;
			Sright_out += IIR->B[IIR->number][m] * Sright_in;
		}
		sout_ptr_A[i * 2] = Sleft_out;
		sout_ptr_A[i * 2 + 1] = Sright_out;

		for (m = 1; m <= 2; m++)
		{
			sub = i - m;
			if(sub < 0){
				Sleft_in  = sout_ptr_B[(IIR->sbuf_size >> 1) + sub * 2];
				Sright_in = sout_ptr_B[(IIR->sbuf_size >> 1) + sub * 2 + 1];
			} else {
				Sleft_in  = sout_ptr_A[sub * 2];
				Sright_in = sout_ptr_A[sub * 2 + 1];
			}
			Sleft_out  += -IIR->A[IIR->number][m] * Sleft_in;
			Sright_out += -IIR->A[IIR->number][m] * Sright_in;
		}
		sout_ptr_A[i * 2] = Sleft_out;
		sout_ptr_A[i * 2 + 1] = Sright_out;

		left_out  = __SSAT((int32_t)(Sleft_out * 32768.0f), 16); // clip to -32768 <= x <= 32767
		right_out = __SSAT((int32_t)(Sright_out * 32768.0f), 16);

		out_ptr[i] = __PKHBT(left_out, right_out, 16); // combine channels
		out_ptr[i] ^= 0x80008000; // signed to unsigned

		if(++IIR->delay_buffer->idx >= IIR->delay_buffer->size){
			IIR->delay_buffer->idx = 0;
		}
	}
}

void IIR_resonator(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 2.0f * M_PI_F * fc / Q / a[0];
  b[1] = 0.0f;
  b[2] = -2.0f * M_PI_F * fc / Q / a[0];

  a[0] = 1.0f;
}

void IIR_LPF(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 4.0f * M_PI_F * M_PI_F * fc * fc / a[0];
  b[1] = 8.0f * M_PI_F * M_PI_F * fc * fc / a[0];
  b[2] = 4.0f * M_PI_F * M_PI_F * fc * fc / a[0];

  a[0] = 1.0f;
}

void IIR_HPF(float fc, float Q, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = 1.0f / a[0];
  b[1] = -2.0f / a[0];
  b[2] = 1.0f / a[0];

  a[0] = 1.0f;
}

void IIR_low_shelving(float fc, float Q, float g, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = (1.0f + sqrtf(1.0f + g) * 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g)) / a[0];
  b[1] = (8.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g) - 2.0f) / a[0];
  b[2] = (1.0f - sqrtf(1.0f + g) * 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc * (1.0f + g)) / a[0];

  a[0] = 1.0f;
}


void IIR_peaking(float fc, float Q, float g, float a[], float b[])
{
  fc = tanf(M_PI_F * fc) / (2.0f * M_PI_F);

  a[0] = 1.0f + 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc;
  a[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  a[2] = (1.0f - 2.0f * M_PI_F * fc / Q + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[0] = (1.0f + 2.0f * M_PI_F * fc / Q * (1.0f + g) + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];
  b[1] = (8.0f * M_PI_F * M_PI_F * fc * fc - 2.0f) / a[0];
  b[2] = (1.0f - 2.0f * M_PI_F * fc / Q * (1.0f + g) + 4.0f * M_PI_F * M_PI_F * fc * fc) / a[0];

  a[0] = 1.0f;
}
