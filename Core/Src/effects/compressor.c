/*
 * compressor.c
 *
 *  Created on: Oct 27, 2026
 *      Author: Artur
 */

#include "compressor.h"

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

static inline float fast_log2(float x){
	union {float f; uint32_t i; } u = {x};							// write x as float and read as uint for binary access
	float e = (float)((int32_t)((u.i >> 23) & 0xFFu) - 128);		// unpack float
	u.i = (u.i & 0x007FFFFFu) | 0x3F800000u;						// leave only significant bits and divide by 127
	float m = u.f;													//
	return e + (-0.34484843f * m + 2.02466578f) * m - 0.65871759f;	//
}

static inline float fast_exp2(float x){
	if (x > 126.0f) 	x = 126.0f;
	if (x < -126.0f) 	x = -126.0f;

	int32_t ip = (int32_t)x;
	if ((float)ip > (float)x) ip --;
	float f = x - (float)ip;

	float y = 1.0f + f * (0.6960656f + f * (0.2244011f + f * 0.0794330f));	// 2^f approximation

	union {float f; uint32_t i;} u;
	u.i = (uint32_t)(ip + 127) << 23;
	return y * u.f;
}

// Compressor init
void compressor_init(Compressor *c){
	float threshold_db	= -26.0f;	// [dBFS]
	float ratio			= 4.0f;		// 4:1
	float knee_db		= 6.0f;		// soft knee width [dB]
	float attack_t		= 0.010f;	// [s]
	float release_t		= 0.100f;	// [s]
	float rms_t			= 0.010f;	// RMS averaging time [s]
	float makeup_db		= 4.0f;		// [dB]
	float sc_cutoff		= 100.0f;	// sidechain HPF cutoff [Hz]

	if (ratio < 1.0f) ratio = 1.0f;
	if (knee_db < 0.0f) knee_db = 0.0f;

	// calculate starting values of state variables
	float fs_ctrl = (float)SAMPLE_RATE_HZ / (float)COMP_CTRL_RATE;	// control rate

	c -> thresh			= threshold_db * COMP_DB_TO_LOG;
	c -> knee 			= knee_db * COMP_DB_TO_LOG;
	c -> slope 			= 1.0f - 1.0f / ratio;
	c -> makeup_gain 	= makeup_db * COMP_DB_TO_LOG;
	c -> a_rms 			= expf(-1.0f / (rms_t * fs_ctrl));
	c -> a_att 			= expf(-1.0f / (attack_t * fs_ctrl));
	c -> a_rel 			= expf(-1.0f / (release_t * fs_ctrl));

	HPF_Init(&c -> cm_hpf, sc_cutoff);

	// ---- state ----
	c -> ms_acc		= 0.0f;
	c -> ms			= 0.0f;
	c -> comp_gr	= 0.0f;
	c -> gain		= fast_exp2(c -> makeup_gain);
	c -> gain_step	= 0.0f;
	c -> ctrl_cnt	= 0;
}

static inline float compressor_curve(const Compressor *c, float level){
	float overshot = level - c -> thresh;
	float w = c -> knee;

	// below smoothing range - don't compress
	if (2.0f * overshot <= -w){
		return 0.0f;
	}

	// above smoothing range - apply max compression
	if (2.0f * overshot >= w){
		return -(c -> slope * overshot);
	}

	// inside smoothing range
	float t = overshot + 0.5f * w;
	return -c -> slope *t *t / (2.0f *w);
}

float compressor_process(Compressor *c, float x){
	// sidechain with HPF
	float sc = HPF_Process(&c -> cm_hpf, x);
	c -> ms_acc += sc * sc;

	float y = x * c -> gain;
	c -> gain += c -> gain_step;


	if (++c -> ctrl_cnt >= COMP_CTRL_RATE) {
		c -> ctrl_cnt = 0;

		// RMS smoothed by LPF
		float block_ms = c -> ms_acc * (1.0f / (float)COMP_CTRL_RATE);
		c -> ms_acc = 0.0f;
		c -> ms = c -> a_rms * c -> ms + (1.0f - c -> a_rms) * block_ms;

		// level in log2: log2(sqrt(ms)) = 0.5 * log2(ms)
		float level = 0.5f * fast_log2(c -> ms + 1e-12f);

		// calculate gain
		float gr = compressor_curve(c, level);
		float a = (gr < c -> comp_gr) ? c -> a_att : c -> a_rel;
		c -> comp_gr = a * c -> comp_gr + (1.0f - a) * gr;

		// new linear target
		float target = fast_exp2(c -> comp_gr + c -> makeup_gain);
		c -> gain_step = (target - c -> gain) * (1.0f / (float)COMP_CTRL_RATE);
	}

		return y;

}

















