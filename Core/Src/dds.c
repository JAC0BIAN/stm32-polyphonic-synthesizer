/*
 * dds.c
 *
 *  Created on: Aug 22, 2026
 *      Author: JAC0BIAN
 */


/* Common DDS and synthesis utilities */

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "dds.h"


int16_t float_to_int16(float x)
{
    float scaled = x * 32767.0f;

    if (scaled > 32767.0f) return 32767;
    if (scaled < -32768.0f) return -32768;

    return (int16_t)scaled;
}


uint32_t freq_to_phase_inc(float f, float fs)
{
    // Increments phase based on the frequency

    double v = ((double)f * 4294967296.0) / (double)fs;

    // Faza inc = 2^32 / częstotliwość
    // bo jeden okres to pełen zakres fazy czyli 2^32

    if (v < 0.0) v = 0.0;
    if (v > 4294967295.0) v = 4294967295.0;

    return (uint32_t)(v + 0.5);
}


float lut_sine_direct(const float *lut, uint32_t phase)
{
    return lut[phase >> (32u - LUT_BITS)];
}


void generate_sine_lut(float *lut, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        lut[i] = sinf(
            2.0f * PI * (float)i / (float)size
        );
    }
}
