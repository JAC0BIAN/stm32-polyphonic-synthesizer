#include "additive_synth.h"
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846f

static float float_buffer[256];

static inline float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline int16_t float_to_int16(float x) {
    x = clampf(x, -1.0f, 1.0f);
    return (int16_t)(x * 32767.0f);
}

static float midi_to_hz(int midi_note) {
    return 440.0f * powf(2.0f, ((float)midi_note - 69.0f) / 12.0f);
}

static uint32_t freq_to_phase_inc(float f, float fs) {
    double v = ((double)f * 4294967296.0) / (double)fs;
    if (v < 0.0) v = 0.0;
    if (v > 4294967295.0) v = 4294967295.0;
    return (uint32_t)(v + 0.5);
}

void synth_init(Synth *s) {
    s->sample_rate = SAMPLE_RATE_HZ;
    s->master_gain = 0.2f;
    s->num_harmonics = 0;
    s->midi_note = -1;
    s->f0_hz = 440.0f;

    for (uint32_t i = 0; i < LUT_SIZE; i++) {
        float ph = (2.0f * PI * (float)i) / (float)LUT_SIZE;
        s->sine_lut[i] = sinf(ph);
    }

    for (uint32_t i = 0; i < MAX_HARMONICS; ++i) {
        s->harmonics[i].phase = 0;
        s->harmonics[i].phase_inc = 0;
        s->harmonics[i].amp = 0.0f;
    }
}

void synth_note_on(Synth *s, int midi_note, uint32_t requested_harmonics) {
    if (midi_note < 0) midi_note = 0;
    if (midi_note > 127) midi_note = 127;

    float f0 = midi_to_hz(midi_note);

    float nyquist = s->sample_rate * 0.5f;
    uint32_t max_nyquist = (uint32_t)(nyquist / f0);
    if (max_nyquist < 1u) max_nyquist = 1u;

    uint32_t n = requested_harmonics;
    if (n > MAX_HARMONICS) n = MAX_HARMONICS;
    if (n > max_nyquist) n = max_nyquist;

    float amp_sum = 0.0f;
    for (uint32_t k = 1; k <= n; k++) {
        Harmonic *h = &s->harmonics[k - 1];
        h->phase_inc = freq_to_phase_inc(f0 * (float)k, s->sample_rate);
        h->amp = 1.0f / (float)k;
        amp_sum += h->amp;
    }

    if (amp_sum > 0.0f) {
        float norm = 1.0f / amp_sum;
        for (uint32_t i = 0; i < n; i++) {
            s->harmonics[i].amp *= norm;
        }
    }
    for (uint32_t i = n; i < MAX_HARMONICS; i++) {
        s->harmonics[i].phase_inc = 0;
        s->harmonics[i].amp = 0.0f;
    }

    s->midi_note = midi_note;
    s->f0_hz = f0;
    s->num_harmonics = n;
}

void synth_generate_block_stereo_i16(Synth *s, int16_t *out_stereo, size_t frames) {
    uint32_t num_harmonics = s->num_harmonics;

    if (num_harmonics == 0) {
        memset(out_stereo, 0, frames * 2 * sizeof(int16_t));
        return;
    }


    memset(float_buffer, 0, frames * sizeof(float));

    const float *lut = s->sine_lut;

    for (uint32_t i = 0; i < num_harmonics; i++) {
        Harmonic *h = &s->harmonics[i];

        uint32_t phase = h->phase;
        uint32_t phase_inc = h->phase_inc;
        float amp = h->amp;

        for (size_t n = 0; n < frames; n++) {
            uint32_t idx = phase >> (32u - LUT_BITS);
            float_buffer[n] += amp * lut[idx];
            phase += phase_inc;
        }

        h->phase = phase;
    }

    float master_gain = s->master_gain;
    for (size_t n = 0; n < frames; n++) {
        int16_t sample = float_to_int16(float_buffer[n] * master_gain);
        out_stereo[2 * n]     = sample;
        out_stereo[2 * n + 1] = sample;
    }
}
