/* sin.h */
#ifndef SIN_H_
#define SIN_H_

#include <stdint.h>
#include <stddef.h>

#define SAMPLE_RATE_HZ   48000.0f
#define LUT_SIZE         2048u
#define LUT_BITS         11u
#define LUT_MASK         (LUT_SIZE-1)
#define MAX_HARMONICS    16u

typedef struct {
    uint32_t phase;
    uint32_t phase_inc;
    float    amp;
} Harmonic;

typedef struct {
    int16_t  sine_lut[LUT_SIZE];
    Harmonic harmonics[MAX_HARMONICS];
    uint32_t num_harmonics;
    float    sample_rate;
    float    master_gain;
    int      midi_note;
    float    f0_hz;
} Synth;

void synth_init(Synth *s);
void synth_note_on(Synth *s, int midi_note, uint32_t requested_harmonics);
float synth_process_one(Synth *s);

void synth_generate_block_stereo_i16(Synth *s, int16_t *out_stereo, size_t frames);

#endif /* SIN_H_ */
