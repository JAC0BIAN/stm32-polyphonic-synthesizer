/*
 * phaser.c
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */
#include "phaser.h"
#include "../dds.h"

void Phaser_Init(Phaser *p, const float *sine_lut)
{
    p->sine_lut  = sine_lut;
    p->lfo_phase = 0.0f;
    p->phase_inc = freq_to_phase_inc(0.1f, SAMPLE_RATE_HZ);
    p->feedback  = 0.4f;
    p->depth 	 = 0.9f;
    p->mix 		 = 0.9f;
    p->fb_sample = 0.0f;

    for (uint32_t i = 0; i < PHASER_STAGES; i++) {
        Allpass1_Init(&p->stages[i], 0.0f);
    }
}

float Phaser_Process(Phaser *p, float x)
{
    float lfo = lut_sine_direct(p->sine_lut, p->lfo_phase);
    float a = 0.06f + p->depth * lfo;
    if (a > 0.99f)  a = 0.99f;
    if (a < -0.99f) a = -0.99f;
    float wet = x + p->feedback * p->fb_sample;

    for (uint32_t i = 0; i < PHASER_STAGES; i++) {
    	Allpass1_SetCoeff(&p->stages[i], a);
        wet = Allpass1_Process(&p->stages[i], wet);
    }
    p->fb_sample = wet;
    p->lfo_phase += p->phase_inc;
    return (1.0f - p->mix) * x + 2.0f * p->mix * wet;
}
