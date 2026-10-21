/*
 * reverb.c
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#include "reverb.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

void Reverb_Init(Reverb * r)
{
	LPF_Init(&r->lpf, 1200.0f);
	r -> beta = 1.0f / (float)NR;

	for (uint32_t k = 0; k < NR; k++) {
		R_Voice	* v = &r -> voices[k];
		v -> idx = 0;								// reset buffer index;
		for (uint32_t i = 0; i < R_BUF_SIZE; i++) { // Initialize reverb buffer with 0's
			v->r_buff[i] = 0.0f;
		};
		v->gain = 0.95;
	};

	r -> voices[0].d0 	= 1559;
	r -> voices[1].d0 	= 1867;
	r -> voices[2].d0 	= 2053;
	r -> voices[3].d0 	= 2251;
	r -> voices[4].d0 	= 2399;
	r -> voices[5].d0 	= 3513;

	r->ap[0].delay = 400;
	r->ap[0].gain  = 0.65f;
	r->ap[0].idx   = 0;
	r->ap[1].delay = 113;
	r->ap[1].gain  = 0.65f;
	r->ap[1].idx   = 0;

	for (uint32_t k = 0; k < AP_COUNT; k++) {
	    for (uint32_t i = 0; i < AP_BUF_SIZE; i++) {
	        r->ap[k].buf[i] = 0.0f;
	    };
	};
}

static float Allpass_Process(Allpass *ap, float x)
{
    uint32_t read_idx = (ap->idx - ap->delay) & AP_BUF_MASK;
    float delayed = ap->buf[read_idx];
    float y = delayed - ap->gain * x;
    ap->buf[ap->idx] = x + ap->gain * y;
    ap->idx = (ap->idx + 1u) & AP_BUF_MASK;
    return y;
}

float Reverb_Process(Reverb *r, float x)
{
    float sum = 0.0f;

    for (uint32_t k = 0; k < NR; k++) {
        R_Voice *v = &r->voices[k];

        uint32_t delay = (uint32_t)v->d0;
        uint32_t read_idx = (v->idx - delay) & R_BUF_MASK;
        float delayed = v->r_buff[read_idx];

        v->r_buff[v->idx] = x + v->gain * delayed;
        sum += delayed;
        v->idx = (v->idx + 1u) & R_BUF_MASK;
    }

    float wet = r->beta * sum;

    for (uint32_t k = 0; k < AP_COUNT; k++) {
        wet = Allpass_Process(&r->ap[k], wet);
    }

    wet = LPF_Process(&r->lpf, wet);
    return 0.7f * x + 0.3f * wet;
}
