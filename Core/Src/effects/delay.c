/*
 * delay.c
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#include "delay.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

void Delay_Init(Delay * d, float *buf)
{
	d -> d_buff = buf;
	d -> idx = 0;			// reset buffer index;

	for (uint32_t i = 0; i < D_BUF_SIZE; i++) { // Initialize delay buffer with 0's
		d->d_buff[i] = 0.0f;
	};

	float q		 = 1;							   // q = 1 -> 4th of a note, 4 -> 16th of a note
	float base_d = (SAMPLE_RATE_HZ*60.0f)/(BPM*q); // based? (base delay depending on BPM and q (4ths/8ths etc. of a note)

	for (uint32_t i = 0; i < N; i++){
		D_Voice *v	= &d->voices[i];
		v -> d0 	= base_d * (float)(i + 1);   // delay, multiplied for next voices
		v -> gain 	= powf(0.7f, i);			 // exponentially decaying amplitude for next voices
	};
}

float Delay_Process(Delay * d, float x)
{
	d->d_buff[d->idx] = x;	// save current sample to the buffer
	float sum = 0.0f;

	for (uint32_t i = 0; i < N; i++){
		D_Voice *v = &d->voices[i];
		float delay = v->d0;				// delay
		uint32_t d_int = (uint32_t)delay;	// change delay to integer (will be used as an index)
		float frac = delay - (float)d_int;

		uint32_t index    = (d->idx - d_int) & D_BUF_MASK;
		uint32_t idx_prev = (index - 1) & D_BUF_MASK;

		float a = d->d_buff[index];
		float b = d->d_buff[idx_prev];
		sum += v->gain * (a + frac * (b - a));
	}
	d->idx = (d->idx + 1) & D_BUF_MASK;
	return x + sum;
}
