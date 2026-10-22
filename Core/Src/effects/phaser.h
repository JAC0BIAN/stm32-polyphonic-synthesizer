/*
 * phaser.h
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_EFFECTS_PHASER_H_
#define SRC_EFFECTS_PHASER_H_

#include <stdint.h>
#include "../filters/allpass.h"

#define PHASER_STAGES 	8u

typedef struct {
    Allpass1 	stages[PHASER_STAGES];
    uint32_t 	lfo_phase;
    uint32_t 	phase_inc;
    const float *sine_lut;
    float 		depth;
    float 		feedback;
    float 		mix;
    float 		fb_sample;
} Phaser;

void Phaser_Init(Phaser *p, const float *sine_lut);
float Phaser_Process(Phaser *p, float x);

#endif /* SRC_EFFECTS_PHASER_H_ */
