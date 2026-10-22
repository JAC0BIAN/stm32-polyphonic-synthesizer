/*
 * allpass.c
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */


#include "allpass.h"

void Allpass_Init(Allpass *ap, uint32_t delay, float gain)
{
    ap->idx = 0;
    ap->delay = delay;
    ap->gain = gain;

    for (uint32_t i = 0; i < AP_BUF_SIZE; i++) {
        ap->buf[i] = 0.0f;
    };
}

float Allpass_Process(Allpass *ap, float x)
{
    uint32_t read_idx =
        (ap->idx - ap->delay) & AP_BUF_MASK;

    float delayed = ap->buf[read_idx];
    float y = delayed - ap->gain * x;

    ap->buf[ap->idx] = x + ap->gain * y;

    ap->idx = (ap->idx + 1u) & AP_BUF_MASK;

    return y;
}

void Allpass1_Init(Allpass1 *ap, float a)
{
    ap-> prev_x1 = 0.0f;
    ap-> prev_y1 = 0.0f;
    Allpass1_SetCoeff(ap, a);
}

void Allpass1_SetCoeff(Allpass1 *ap, float a)
{
    if (a > 0.99f)  a = 0.99f;
    if (a < -0.99f) a = -0.99f;

    ap->a = a;
}

float Allpass1_Process(Allpass1 *ap, float x)
{
    float y = ap->a * x + ap->prev_x1 - ap->a * ap->prev_y1;

    ap->prev_x1 = x;
    ap->prev_y1 = y;

    return y;
}
