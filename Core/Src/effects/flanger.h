/*
 * flanger.h
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_FLANGER_H_
#define SRC_FLANGER_H_

#include <stdint.h>
#include <stddef.h>

#define F_BUF_SIZE 2048
#define F_BUF_MASK (F_BUF_SIZE - 1)

typedef struct{
    float       gain;
    float		d0;
	float		lfo_gain;
	uint32_t 	lfo_phase;
	uint32_t 	phase_inc;
}F_Voice;

typedef struct{
	F_Voice		voice;
	float		f_buff[F_BUF_SIZE];
	uint32_t 	idx;
	float		beta;
	float		prev_y;
	const float *sine_lut;
}Flanger;

void 	Flanger_Init(Flanger * f, const float *sine_lut);
float 	Flanger_Process(Flanger * f, float x);

#endif /* SRC_FLANGER_H_ */
