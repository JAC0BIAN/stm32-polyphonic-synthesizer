#include "additive_synth.h"
#include <math.h>

#define PI 3.14159265358979323846f

static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static int16_t float_to_int16(float x) {
    x = clampf(x, -1.0f, 1.0f);
    int32_t v = (int32_t)(x * 32767.0f);
    if (v > 32767)  v = 32767;
    if (v < -32767) v = -32767;
    return (int16_t)v;
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

static float lut_sine_linear(const int16_t *lut, uint32_t phase) {
    uint32_t i0 = phase >> (32u - LUT_BITS);
    uint32_t i1 = (i0 + 1u) & LUT_MASK;
    uint32_t frac_u16 = (phase >> (32u - LUT_BITS - 16u)) & 0xFFFFu;
    float frac = (float)frac_u16 / 65536.0f;
    float y0 = (float)lut[i0] / 32768.0f;
    float y1 = (float)lut[i1] / 32768.0f;
    return y0 + frac * (y1 - y0);
}

void synth_init(Synth *s) {
    s->sample_rate = SAMPLE_RATE_HZ;
    s->master_gain = 0.2f;
    s->num_harmonics = 0;
    s->midi_note = -1;
    s->f0_hz = 440.0f;

    for (uint32_t i = 0; i < LUT_SIZE; i++) {
        float ph = (2.0f * PI * (float)i) / (float)LUT_SIZE;
        float v = sinf(ph);
        s->sine_lut[i] = (int16_t)(v * 32767.0f);
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

float synth_process_one(Synth *s) {
    if (s->num_harmonics == 0) {
        return 0.0f;
    }

    float y = 0.0f;
    for (uint32_t i = 0; i < s->num_harmonics; i++) {
        Harmonic *h = &s->harmonics[i];
        y += h->amp * lut_sine_linear(s->sine_lut, h->phase);
        h->phase += h->phase_inc;
    }
    return y * s->master_gain;
}

void synth_generate_block_stereo_i16(Synth *s, int16_t *out_stereo, size_t frames) {
    for (size_t n = 0; n < frames; n++) {
        int16_t sample = float_to_int16(synth_process_one(s));
        out_stereo[2 * n]     = sample; // Kanał lewy
        out_stereo[2 * n + 1] = sample; // Kanał prawy
    }
}
