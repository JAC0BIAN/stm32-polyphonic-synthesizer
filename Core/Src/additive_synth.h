
#ifndef SINH
#define SINH

#include <stdint.h>
#include <stddef.h>

#include "dds.h"

typedef struct {
    uint32_t phase;
    uint32_t phase_inc;
    float    amp;
} Harmonic;

typedef struct{
    //int16_t        sine_lut[LUT_SIZE];
	float        sine_lut[LUT_SIZE];
    uint16_t    midi_freakyuency[128];
    float        sample_rate;
    float         master_gain;

    uint32_t    active_voices;
    float 		current_gain;
}Synth;


typedef struct{
    Harmonic    harmonics[MAX_HARMONICS];
    uint32_t    num_harmonics;
    char 		wave;

    int         midi_note;
    float       f0_hz;
}Voice;

void 		synth_init(Synth *s, Voice *v);
void         synth_note_on(Synth *s, Voice *v, int midi_note, uint8_t velocity, uint32_t requested_harmonics, char wave);
void         synth_note_off(Synth *s, Voice *v, int midi_note);
float        synth_process_one(Synth *s, Voice *v);
void         synth_generate_block_i16(Synth *s, Voice *v, int16_t *out, size_t frames);

#endif /* SINH */
