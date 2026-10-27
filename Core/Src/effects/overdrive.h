#ifndef OVEDRIVE_H_
#define OVEDRIVE_H_

#include <stdint.h>

typedef struct {
	int32_t		gain;
	int32_t		level;
	int32_t		tone;
	int32_t		lpf_z;
} Overdrive;


void overdrive_init(Overdrive *ov);
float overdrive_process(Overdrive *ov, float x);


#endif
