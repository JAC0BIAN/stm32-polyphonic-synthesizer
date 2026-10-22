/*
 * allpass.h
 *
 *  Created on: Oct 22, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_FILTERS_ALLPASS_H_
#define SRC_FILTERS_ALLPASS_H_

#include <stdint.h>
#include <stddef.h>

#define AP_BUF_SIZE 1024u
#define AP_BUF_MASK (AP_BUF_SIZE - 1u)

typedef struct {
    float 		buf[AP_BUF_SIZE];
    uint32_t 	idx;
    uint32_t 	delay;
    float 		gain;
} Allpass;

typedef struct {
    float  		prev_x1; //previous input
    float 		prev_y1; //previous output
    float 		a;
} Allpass1;

void 	Allpass1_Init(Allpass1 *ap, float a);
void 	Allpass1_SetCoeff(Allpass1 *ap, float a);
float 	Allpass1_Process(Allpass1 *ap, float x);

void 	Allpass_Init(Allpass *ap, uint32_t delay, float gain);
float 	Allpass_Process(Allpass *ap, float x);

#endif /* SRC_FILTERS_ALLPASS_H_ */
