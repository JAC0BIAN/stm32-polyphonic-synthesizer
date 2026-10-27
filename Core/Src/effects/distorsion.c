#include "distorsion.h"

#include <stdint.h>
#include "../dds.h"

#define Q15_MAX 			32767
#define FLOAT_TO_Q15		32768.0f
#define Q15_TO_FLOAT		(1.0f / 32768.0f)


void distortion_init(Distortion *di){
	di -> lpf_z = 0;
	di -> gain = (int32_t)12000;
	di -> level = (int32_t)16384;		// Not more than 32767
	di -> thresh = (int32_t)16385;		// threshold for clipping
	int32_t cutoff_freq = 3000;					// for lpf

	if(cutoff_freq > 20000u) cutoff_freq = 20000u;
	uint32_t a = (205887u * cutoff_freq)/(uint32_t)SAMPLE_RATE_HZ;
	if(a > Q15_MAX) a = Q15_MAX;
	di -> tone = (int32_t)a;
}

float distortion_process(Distortion *di, float x){
	int32_t y = (int32_t)(x * FLOAT_TO_Q15);		// conversion to int (it may improve performance)(i hope it does)

	y = (y * di -> gain) >> 8;

	// hard clip
	if(y > di -> thresh) 	y = di -> thresh;
	if(y < -di -> thresh)	y = -di -> thresh;

	di -> lpf_z += (di -> tone * (y - di -> lpf_z)) >> 15;

	int32_t out = (di -> lpf_z * di -> level) >> 15;
	return (float)out * Q15_TO_FLOAT;

}
