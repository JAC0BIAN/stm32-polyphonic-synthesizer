/*
 * lpf.h
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_LPF_H_
#define SRC_LPF_H_

#include <stdint.h>
#include <stddef.h>

#define LPF_STAGES 4   // 8 x 6 dB/oct = 48 dB/oct

typedef struct {
	float		alpha; //alpha = Ts/(T+Ts), where Ts is the sampling period and T = 1/(2pi * cutoff frequency)
	float		prev_y[LPF_STAGES]; //y[n-1] (previous output state)
}LPF;

void LPF_Init(LPF * f, float cutoff);
float LPF_Process(LPF *f, float x);

#endif /* SRC_LPF_H_ */
