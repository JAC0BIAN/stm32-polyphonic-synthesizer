/*
 * chorus.c
 *
 *  Created on: Oct 16, 2026
 *      Author: JAC0BIAN
 */

#include "chorus.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "dds.h"

void Chorus_Init(Chorus * ch, const float *sine_lut)
{
	ch -> sine_lut 	= sine_lut;		// pass existing sine LUT to chorus effect (sine is used for LFOs)
	ch -> idx 		= 0;			// reset buffer index;

	for (uint32_t i = 0; i < CH_BUF_SIZE; i++) { // Initialize chorus buffer with 0's
		ch->ch_buff[i] = 0.0f;
	};

	for (uint32_t i = 0; i < N; i++){
		CH_Voice *v = &ch->voices[i];
		float freq = 1.0f + (float)rand() / (float)RAND_MAX; //random frequency: [1,2]Hz (LFO)
		v -> d0 	= 600.0f + 15.0f * (float)i;   // base delay
		v -> gain 	= 0.5f;		// initially all voices have this gain
		v -> lfo_gain = 60.0f; 	// LFO modulation gain
		v -> lfo_phase = (uint32_t)(((uint64_t)i << 32) / N);
		v -> phase_inc = freq_to_phase_inc(freq, SAMPLE_RATE_HZ);		// random frequency makes each voice slightly different
	};
}

float Chorus_Process(Chorus * ch, float x)
{
	ch->ch_buff[ch->idx] = x;	// save current sample to the buffer
	float sum = 0.0f;

	for (uint32_t i = 0; i < N; i++){
		CH_Voice *v = &ch->voices[i];
		float lfo = lut_sine_direct(ch->sine_lut, v->lfo_phase);// LFO from sine LUT
		float delay = v->d0 + v->lfo_gain * lfo;				// delay: D = A*sin(lfo_phase)
		uint32_t d_int = (uint32_t)delay;						// change delay to integer (will be used as an index)
		float frac = delay - (float)d_int;

		uint32_t index    = (ch->idx - d_int) & CH_BUF_MASK;
		uint32_t idx_prev = (index - 1) & CH_BUF_MASK;

		float a = ch->ch_buff[index];
		float b = ch->ch_buff[idx_prev];
		sum += v->gain * (a + frac * (b - a));
		v->lfo_phase += v->phase_inc; // phase update
	}
	ch->idx = (ch->idx + 1) & CH_BUF_MASK;
	return x + sum;
}


