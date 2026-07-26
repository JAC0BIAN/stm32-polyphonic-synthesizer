/* sin.h */
#ifndef SIN_H_
#define SIN_H_

#include <stdint.h>
#include <stddef.h>

#define PI 				 3.14159265359
#define SAMPLE_RATE_HZ   48000.0f
#define LUT_SIZE         1638u
#define LUT_BITS         14u
#define LUT_MASK         (LUT_SIZE-1)
#define MAX_HARMONICS    16u

typedef struct {
    uint32_t phase;
    uint32_t phase_inc;
    float    amp;
} Harmonic;

typedef struct{
	int16_t		sine_lut[LUT_SIZE];
	uint16_t	midi_freakyuency[128];
	float		sample_rate;
	float 		master_gain;
}Synth;


typedef struct{
	Harmonic	harmonics[MAX_HARMONICS];
	uint32_t	num_harmonics;

	int			midi_note;
	float		f0_hz;
}Voice;

void 		synth_init(Synth *s, Voice *v);
void 		synth_note_on(Synth *s, Voice *v, int midi_note, uint32_t requested_harmonics);
float 		synth_process_one(Synth *s, Voice *v);
void 		synth_generate_block_i16(Synth *s, Voice *v, int16_t *out, size_t frames);

#endif /* SIN_H_ */
