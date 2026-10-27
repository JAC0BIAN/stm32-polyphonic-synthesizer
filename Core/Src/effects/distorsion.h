// Here and in overdrive im using int32 operations inside so im using my own simple lpf filter. I hope it will make the performence better since we dont have a fpu.
#ifndef DISTORTION_H_
#define DISTORTION_H_

#include <stdint.h>

typedef struct {
	int32_t gain;		// pre-gain
	int32_t level;		// output level
	int32_t thresh;		// clipping threshold
	int32_t tone;		// LPF coefficient
	int32_t lpf_z;		// LPF state
} Distortion;

void  distortion_init(Distortion *d);
float distortion_process(Distortion *d, float x);

#endif
