/*
 * dds.h
 *
 *  Created on: Aug 22, 2026
 *      Author: FUJITSU
 */

#ifndef SRC_DDS_H_
#define SRC_DDS_H_

#include <stdint.h>
#include <stddef.h>

#define PI                  3.14159265359
#define SAMPLE_RATE_HZ     48000.0f

#define LUT_SIZE            2048u
#define LUT_BITS            11u
#define LUT_MASK            (LUT_SIZE-1)

#define MAX_HARMONICS       16u
#define MAX_VOICES          8u
#define MAX_BLOCK_FRAMES    256

uint32_t freq_to_phase_inc(float f, float fs);

float lut_sine_direct(const float *lut, uint32_t phase);

void generate_sine_lut(float *lut, uint32_t size);

int16_t float_to_int16(float x);


#endif /* SRC_DDS_H_ */
