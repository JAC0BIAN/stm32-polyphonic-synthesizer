/*
 * chorus.h
 *
 *  Created on: Oct 16, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_CHORUS_H_
#define SRC_CHORUS_H_

#include <stdint.h>
#include <stddef.h>

#define	N			3u
#define CH_BUF_SIZE 2048
#define CH_BUF_MASK (CH_BUF_SIZE - 1)

typedef struct{
    float       gain;
    float		d0;
	float		lfo_gain;
	uint32_t 	lfo_phase;
	uint32_t 	phase_inc;
}CH_Voice;

typedef struct{
	CH_Voice	voices[N];
	float		ch_buff[CH_BUF_SIZE];
	uint32_t 	idx;
	const float *sine_lut;
}Chorus;

void 	Chorus_Init(Chorus *ch, const float *sine_lut);
float 	Chorus_Process(Chorus *ch, float x);

#endif /* SRC_CHORUS_H_ */
