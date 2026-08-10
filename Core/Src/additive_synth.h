#ifndef ADDITIVE_SYNTH_H_
#define ADDITIVE_SYNTH_H_

#include <stdint.h>
#include <stddef.h>

#define SAMPLE_RATE_HZ 48000.0f
#define LUT_BITS        11u
#define LUT_SIZE        (1u << LUT_BITS)
#define LUT_MASK        (LUT_SIZE - 1u)
#define MAX_HARMONICS   32

typedef struct {
    uint32_t phase;
    uint32_t phase_inc;
    float amp;
} Harmonic;

typedef struct {
    float sample_rate;
    float master_gain;
    uint32_t num_harmonics;
    int midi_note;
    float f0_hz;
    float sine_lut[LUT_SIZE];
    Harmonic harmonics[MAX_HARMONICS];
} Synth;

void synth_init(Synth *s);
void synth_note_on(Synth *s, int midi_note, uint32_t requested_harmonics);
void synth_generate_block_stereo_i16(Synth *s, int16_t *out_stereo, size_t frames);

#endif
