/*
 * hpf.c
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#include "hpf.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

void HPF_Init(HPF * f, float cutoff)
{
	float rc = 1.0f / (2.0f * PI * cutoff);
	float dt = 1.0f / (float)SAMPLE_RATE_HZ;
	f->alpha = rc / (rc + dt);
	for (int i = 0; i < HPF_STAGES; i++) {
		f->prev_y[i] = 0.0f;
		f->prev_x[i] = 0.0f;
	};
}

float HPF_Process(HPF *f, float x)
{
	for (int i = 0; i < HPF_STAGES; i++) {
	        f->prev_y[i] = f->alpha * f->prev_y[i] + f->alpha * (x - f->prev_x[i]);
			f->prev_x[i] = x;
	        x = f->prev_y[i];   // output of this stage feeds the next
	    };
	return x;
}
