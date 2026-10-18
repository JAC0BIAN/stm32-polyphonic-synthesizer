/*
 * flanger.c
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#include "flanger.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

void Flanger_Init(Flanger * f, const float *sine_lut)
{
	f -> sine_lut 	= sine_lut;		// pass existing sine LUT to flanger effect (sine is used for LFOs)
	f -> idx 		= 0;			// reset buffer index;
	f -> beta 		= 0.9f;
	f -> prev_y		= 0.0f;

	for (uint32_t i = 0; i < F_BUF_SIZE; i++) { // Initialize flanger buffer with 0's
		f->f_buff[i] = 0.0f;
	};

	F_Voice * v = &f -> voice;
	float freq 		= 0.3; 	// LFO frequency
	v -> d0 		= 40.0f;   // base delay
	v -> gain 		= 0.9f;
	v -> lfo_gain 	= 40.0f; 	// LFO modulation gain
	v -> lfo_phase 	= 0;
	v -> phase_inc 	= freq_to_phase_inc(freq, SAMPLE_RATE_HZ);		// random frequency makes each voice slightly different
}

float Flanger_Process(Flanger * f, float x)
{
	f->f_buff[f->idx] = x;	// save current sample to the buffer
	float sum = 0.0f;

	F_Voice * v = &f -> voice;
	float lfo = lut_sine_direct(f->sine_lut, v->lfo_phase);// LFO from sine LUT
	float delay = v->d0 + v->lfo_gain * lfo;				// delay: D = A*sin(lfo_phase)
	uint32_t d_int = (uint32_t)delay;						// change delay to integer (will be used as an index)
	float frac = delay - (float)d_int;

	uint32_t index    = (f->idx - d_int) & F_BUF_MASK;
	uint32_t idx_prev = (index - 1) & F_BUF_MASK;

	float a = f->f_buff[index];
	float b = f->f_buff[idx_prev];
	sum += v->gain * (a + frac * (b - a));
	v->lfo_phase += v->phase_inc; // phase update

	f->idx = (f->idx + 1) & F_BUF_MASK;
	float feedback = f->prev_y;
	f->f_buff[f->idx] = x + f->beta * feedback;
	float y = x + sum;
	f->prev_y = sum;
	return y;
}





