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

#define	NR			6u
#define R_BUF_SIZE 	4096
#define R_BUF_MASK 	(R_BUF_SIZE - 1)
#define AP_BUF_SIZE 1024u
#define AP_BUF_MASK (AP_BUF_SIZE - 1u)
#define AP_COUNT 	2u

typedef struct {
    float 		buf[AP_BUF_SIZE];
    uint32_t 	idx;
    uint32_t 	delay;
    float 		gain;
} Allpass;

typedef struct{
    float       gain;
	uint32_t 	idx;
    float		r_buff[R_BUF_SIZE];
    float		d0;
}R_Voice;

typedef struct{
	R_Voice		voices[NR];
	float		beta;
    Allpass 	ap[AP_COUNT];
    LPF 		lpf;
}Reverb;

void 	Reverb_Init(Reverb * r);
float 	Reverb_Process(Reverb * r, float x);

#endif /* SRC_EFFECTS_REVERB_H_ */
