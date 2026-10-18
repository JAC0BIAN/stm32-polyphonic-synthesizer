/*
 * lpf.c
 *
 *  Created on: Oct 18, 2026
 *      Author: JAC0BIAN
 */

#include "lpf.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "../dds.h"

void LPF_Init(LPF * f, float cutoff)
{
	f->alpha = cutoff/(cutoff + SAMPLE_RATE_HZ/(2.0f*PI));
	for (int i = 0; i < LPF_STAGES; i++) f->prev_y[i] = 0.0f;
}

float LPF_Process(LPF *f, float x)
{
	for (int i = 0; i < LPF_STAGES; i++) {
	        f->prev_y[i] = (1.0f - f->alpha) * f->prev_y[i] + f->alpha * x;
	        x = f->prev_y[i];   // output of this stage feeds the next
	    }
	return x;
}





