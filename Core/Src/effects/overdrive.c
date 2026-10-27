#include "overdrive.h"

#include <stdint.h>
#include "../dds.h"

#define Q15_MAX 			32767
#define FLOAT_TO_Q15		32768.0f
#define Q15_TO_FLOAT		(1.0f / 32768.0f)


void overdrive_init(Overdrive *ov){
	ov -> lpf_z = 0;
	ov -> gain = (int32_t)12000;
	ov -> level = (int32_t)2000;		// Not more than 32767
	int32_t cutoff_freq = 8000;					// for lpf

	if(cutoff_freq > 20000u) cutoff_freq = 20000u;
	uint32_t a = (205887u * cutoff_freq)/(uint32_t)SAMPLE_RATE_HZ;
	if(a > Q15_MAX) a = Q15_MAX;
	ov -> tone = (int32_t)a;
}

float overdrive_process(Overdrive *ov, float x){
	int32_t y = (int32_t)(x * FLOAT_TO_Q15);		// conversion to int (it may improve performance)(i hope it does)

	y = (y * ov -> gain) >> 8;

	if(y > Q15_MAX) 	y = Q15_MAX;
	if(y < -Q15_MAX)	y = -Q15_MAX;

	// soft clip: f(y) = 1.5*(y - y^3/3) = (3y - y^3)/2
	int32_t y2 = (y*y) >> 15;
	int32_t y3 = (y2*y) >>15;
	y = (3*y-y3) >> 1;

	if(y > Q15_MAX) 	y = Q15_MAX;
	if(y < -Q15_MAX)	y = -Q15_MAX;

	ov -> lpf_z += (ov -> tone * (y - ov -> lpf_z)) >> 15;

	int32_t out = (ov -> lpf_z * ov -> level) >> 15;
	return (float)out * Q15_TO_FLOAT;

}
