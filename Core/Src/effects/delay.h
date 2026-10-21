/*
 * delay.h
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_EFFECTS_DELAY_H_
#define SRC_EFFECTS_DELAY_H_

#include <stdint.h>
#include <stddef.h>

#define	N			6u
#define D_BUF_SIZE  (1u << 18)          // 131072 samples, power of 2
#define D_BUF_MASK  (D_BUF_SIZE - 1)
#define BPM			120

typedef struct{
    float       gain;
    float		d0;
}D_Voice;

typedef struct{
	D_Voice		voices[N];
	float		*d_buff;
	uint32_t 	idx;
}Delay;

void 	Delay_Init(Delay * d, float *buf);
float 	Delay_Process(Delay * d, float x);

#endif /* SRC_EFFECTS_DELAY_H_ */
