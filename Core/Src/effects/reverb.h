/*
 * reverb.h
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_EFFECTS_REVERB_H_
#define SRC_EFFECTS_REVERB_H_

#include <stdint.h>
#include <stddef.h>
#include "../filters/lpf.h"
#include "../filters/allpass.h"

#define	NR			6u
#define R_BUF_SIZE 	4096
#define R_BUF_MASK 	(R_BUF_SIZE - 1)

typedef struct{
    float       gain;
	uint32_t 	idx;
    float		r_buff[R_BUF_SIZE];
    float		d0;
}R_Voice;

typedef struct{
	R_Voice		voices[NR];
	float		beta;
	Allpass 	ap[2];
    LPF 		lpf;
}Reverb;

void 	Reverb_Init(Reverb * r);
float 	Reverb_Process(Reverb * r, float x);

#endif /* SRC_EFFECTS_REVERB_H_ */
