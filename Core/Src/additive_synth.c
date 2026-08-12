/*Simple DDS based synth*/

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "additive_synth.h"

static float clampf(float x, float lo, float hi){
    // Normalize x to set parameter range
    if(x < lo) return lo;
    if(x > hi) return hi;
    return x;
}

static int32_t clampint32(int32_t x, int32_t lo, int32_t hi){
	if(x < lo) return lo;
	if(x > hi) return hi;
	return x;
}


static int16_t float_to_int16(float x){
    // Float to int conversion (shocking)
    x = clampf(x, -1.0f, 1.0f);
    int32_t v = (int32_t)(x * 32767.0f);
    v = clampint32(v,-32767, 32767);
    return (int16_t)v;
}


static uint32_t freq_to_phase_inc(float f, float fs){
    // Increments phase based on the frequency
    double v = ((double)f * 4294967296.0) / (double)fs;
    // Faza inc = 2^32/ częstotliwość --> bo jeden okres to pełen zakres fazy czylli 2^32
    if (v < 0.0) v = 0.0;
    if (v > 4294967295.0) v = 4294967295.0;
    return (uint32_t)(v + 0.5);                         // Float to int cuts .xxx so +0.5 forces correct approximation
}

static float lut_sine_direct(const int16_t *lut, uint32_t phase){
	// Return proper value from the look up table
	uint32_t index = phase >> (32u - LUT_BITS);
	return (float)lut[index]/32768.0f;
}

static void generate_sine_lut(int16_t *lut, uint32_t size){
	//
	for (uint32_t i = 0; i < size; i++){
		float angle = 2.0f * PI * (float)i / (float)size;
		float s = sinf(angle);
		int32_t v = (int32_t)(s * 32767.0f + (s >= 0.0f ? 0.5f : -0.5f));
		lut[i] = (int16_t)v;
	}
}

static void voice_reset(Voice *v){
	// Clear unused voice
	v -> midi_note = -1;
	v -> num_harmonics = 0;
	for (uint32_t i = 0; i < MAX_HARMONICS; i++){
		v -> harmonics[i].phase = 0;
		v -> harmonics[i].phase_inc = 0;
		v -> harmonics[i].amp = 0.0f;
	}
}

// =================================== Synth ============================================

void synth_init(Synth *s, Voice *v){
    //
    s -> sample_rate = SAMPLE_RATE_HZ;
    s -> master_gain = 0.4f;
    v -> num_harmonics = 0;
    v -> midi_note = 69;
    v -> f0_hz = 420.0f;

    generate_sine_lut(s -> sine_lut, LUT_SIZE);

    // MIDI note to frequency
    for (uint32_t i = 21; i < 128; i++){
    	float f = 440.0f * powf(2.0f, ((float)i - 69.0f) / 12.0f);
    	s->midi_freakyuency[i] = (uint16_t)(f+0.5);
    }

    for (uint32_t i = 0; i < MAX_VOICES; i++){
    	voice_reset(&v[i]);
    }

}


void synth_note_on(Synth *s, Voice *vs, int midi_note, uint32_t requested_harmonics){
    //
	uint32_t idx;
	int found = 0;

	// Checks if requested voice already exists.
	for (uint32_t i = 0; i < MAX_VOICES; i++){
		if(vs[i].midi_note == midi_note && vs[i].num_harmonics > 0){
			idx = i;
			found = 1;
			break;
		}
	}

	// If it doesn't exits, adds to the list and increments active_voices.
	if (!found){
		for (uint32_t i = 0; i < MAX_VOICES; i ++){
			if (vs[i].midi_note < 0){
				idx = i;
				found = 1;
				s -> active_voices ++;
				break;
			}
		}
	}

	// If max voices was already reached, steal the oldest one.
	if (!found){
		static uint32_t he_stealing = 0;
		idx = he_stealing % MAX_VOICES;
		he_stealing++;
	}

	Voice *v = &vs[idx];

	//-----------------------------

    v -> midi_note = midi_note;
    v -> f0_hz = s -> midi_freakyuency[midi_note];

    float nyquist = s -> sample_rate * 0.5f;                // Antialiasing filter
    uint32_t max_nyquist = (uint32_t)(nyquist/ (v -> f0_hz));
    if (max_nyquist < 1u) max_nyquist = 1u;

    uint32_t n = requested_harmonics;
    if (n > MAX_HARMONICS) n = MAX_HARMONICS;               // Reduce to permited max harmonics
    if (n > max_nyquist) n = max_nyquist;                   // Reduce to max without aliasing
    v -> num_harmonics = n;

    float amp_sum = 0.0f;
    for (uint32_t k = 1; k <= n; k++){
        Harmonic * h = &v -> harmonics[k - 1];
        h -> phase = 0;
        h -> phase_inc = freq_to_phase_inc(v -> f0_hz * (float)k, s -> sample_rate);
        h -> amp = 1.0f / (float)k;
        amp_sum = amp_sum + h -> amp;
    }

    if (amp_sum > 0.0f){
        float norm = 1.0f / amp_sum;
        for (uint32_t i = 0; i < n; i++) {
            v->harmonics[i].amp *= norm;
    	}
	}
    for (uint32_t i = n; i < MAX_HARMONICS; i++){
            v -> harmonics[i].phase_inc = 0;
            v -> harmonics[i].amp = 0.0f;
    }
}

void synth_note_off(Synth *s, Voice *v, int midi_note){
	for(uint32_t i = 0; i < MAX_VOICES; i ++){
		if (v[i].midi_note == midi_note && v[i].num_harmonics > 0){
			voice_reset(&v[i]);
			if(s -> active_voices > 0) s -> active_voices --;
			return;
		}
	}
}


float synth_process_one(Synth *s, Voice *voices){
    float y = 0.0f;

    for (uint32_t vi = 0; vi < MAX_VOICES; vi++){
        Voice *v = &voices[vi];
        if (v -> midi_note < 0 || v -> num_harmonics == 0) continue;

        for (uint32_t i = 0; i < v -> num_harmonics; i++){
            Harmonic *h = &v -> harmonics[i];
            y += h -> amp * lut_sine_direct(s -> sine_lut, h -> phase);
            h -> phase += h -> phase_inc;
        }
    }

    float mod = (float)s->active_voices;
    if (mod < 1.0f){ mod = 1.0f; }
    y *= s -> master_gain / mod;
    return y;
}


void synth_generate_block_i16(Synth *s,Voice *v, int16_t *out, size_t frames){
    for (size_t n = 0; n <frames; n++){
        out[n] = float_to_int16(synth_process_one(s,v));
    }
}
