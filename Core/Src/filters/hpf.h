/*
 * hpf.h
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_FILTERS_HPF_H_
#define SRC_FILTERS_HPF_H_

#include <stdint.h>
#include <stddef.h>

#define HPF_STAGES 8   // 8 x 6 dB/oct = 48 dB/oct

typedef struct {
	float		alpha;
	float		prev_y[HPF_STAGES]; //y[n-1] (previous output state)
	float		prev_x[HPF_STAGES];				//x[n-1] (previous input  state)
}HPF;

void HPF_Init(HPF * f, float cutoff);
float HPF_Process(HPF *f, float x);

#endif /* SRC_FILTERS_HPF_H_ */
