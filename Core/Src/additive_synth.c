/*Simple DDS based synth*/

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "additive_synth.h"
#include "dds.h"

static void voice_reset(Voice *v){
	// Clear unused voice
	v -> midi_note = -1;
	v -> num_harmonics = 0;
	v -> wave = 'a';
	for (uint32_t i = 0; i < MAX_HARMONICS; i++){
		v -> harmonics[i].phase = 0;
		v -> harmonics[i].phase_inc = 0;
		v -> harmonics[i].amp = 0.0f;
	}
}

// =================================== Synth ============================================
void update_synth_gain(Synth *s) {
    float mod = (float)s->active_voices;
    if (mod < 1.0f) mod = 1.0f;
    s->current_gain = s->master_gain / mod;
}

void synth_init(Synth *s, Voice *v){
    //
    s -> sample_rate = SAMPLE_RATE_HZ;
    s -> master_gain = 0.4f;
	s -> active_voices = 0;
    v -> num_harmonics = 0;
    v -> midi_note = 69;
    v -> f0_hz = 420.0f;

    generate_sine_lut(s -> sine_lut, LUT_SIZE);

    // MIDI note to frequency
    for (uint32_t i = 0; i < 128; i++){
    	s->midi_freakyuency[i] = (uint16_t)(440.0f * powf(2.0f, ((float)i - 69.0f) / 12.0f) + 0.5f);
    }

    for (uint32_t i = 0; i < MAX_VOICES; i++){
    	voice_reset(&v[i]);
    }

    update_synth_gain(s);
}


void synth_note_on(Synth *s, Voice *vs, int midi_note,uint8_t velocity, uint32_t requested_harmonics, char wave){
	if (midi_note < 0) midi_note = 0;
	if (midi_note > 127) midi_note = 127;
	if (velocity > 127) velocity = 127;

	uint32_t idx = 0xFFFFFFFF;

	// Checks if requested voice already exists.
	for (uint32_t i = 0; i < s->active_voices; i++) {
		if (vs[i].midi_note == midi_note) {
			idx = i;
	        break;
		}
	}

	// If it doesn't exits, adds to the list and increments active_voices.
	if (idx == 0xFFFFFFFF) {
	    if (s->active_voices < MAX_VOICES) {
	        idx = s->active_voices;
	        s->active_voices++;
	    } else {
	     // If max voices was already reached, steal the oldest one.
	        idx = 0;
	    }
	}

	Voice *v = &vs[idx];

	//-----------------------------

    v -> midi_note = midi_note;
    v -> f0_hz = s -> midi_freakyuency[midi_note];
    v -> wave = wave;

    float nyquist = s -> sample_rate * 0.5f;                // Antialiasing filter
    uint32_t max_nyquist = (uint32_t)(nyquist/ (v -> f0_hz));
    if (max_nyquist < 1u) max_nyquist = 1u;

    uint32_t n = requested_harmonics;
    if (n > MAX_HARMONICS) n = MAX_HARMONICS;               // Reduce to permited max harmonics
    if (n > max_nyquist) n = max_nyquist;                   // Reduce to max without aliasing
    v -> num_harmonics = n;

    float norm_velocity = (float)velocity/127.0f;

    float amp_sum = 0.0f;
    for (uint32_t k = 1; k <= n; k++){
        Harmonic * h = &v -> harmonics[k - 1];
        h -> phase = 0;
        h -> phase_inc = freq_to_phase_inc(v -> f0_hz * (float)k, s -> sample_rate);
        h -> amp = 1.0f / (float)k;
        amp_sum = amp_sum + h -> amp;
    }

    if (amp_sum > 0.0f){
    	float norm = (1.0f / amp_sum) * norm_velocity;
        for (uint32_t i = 0; i < n; i++) {
            v->harmonics[i].amp *= norm;
    	}
	}
    for (uint32_t i = n; i < MAX_HARMONICS; i++){
            v -> harmonics[i].phase_inc = 0;
            v -> harmonics[i].amp = 0.0f;
    }

    update_synth_gain(s);
}

void synth_note_off(Synth *s, Voice *vs, int midi_note) {
    for (uint32_t i = 0; i < s->active_voices; i++) {
        if (vs[i].midi_note == midi_note) {
            if (i < s->active_voices - 1) {
                vs[i] = vs[s->active_voices - 1];
            }
            voice_reset(&vs[s->active_voices - 1]);
            s->active_voices--;

            update_synth_gain(s);
            return;
        }
    }
}

void synth_generate_block_i16(Synth *s, Voice *voices, int16_t *out, size_t frames) {
    static float float_buf[MAX_BLOCK_FRAMES]; //static - no allocation on stack

    if (frames > MAX_BLOCK_FRAMES) frames = MAX_BLOCK_FRAMES;

    for (size_t n = 0; n < frames; n++) {
        float_buf[n] = 0.0f;
    }

    uint32_t shift = 32u - LUT_BITS;
    const float *lut = s->sine_lut;

    // only active voices, avoid unnecessary checking of inactive voices
    for (uint32_t vi = 0; vi < s->active_voices; vi++) {
        Voice *v = &voices[vi];
        uint32_t n_harm = v->num_harmonics;

        switch(v->wave){
        case 'a': {
			for (uint32_t i = 0; i < n_harm; i++) {
				Harmonic *h = &v->harmonics[i];

				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {
					uint32_t idx = phase >> shift;
					float_buf[n] += amp * lut[idx];
					phase += phase_inc;
				}

				h->phase = phase; //save phase to RAM only once per block
			}
			break;
        }
        case 's': {
        	Harmonic *h = &v->harmonics[0];

        	register uint32_t phase = h->phase;
        	register uint32_t phase_inc = h->phase_inc;
			register float amp = h->amp;

			for (size_t n = 0; n < frames; n++) {
					uint32_t idx = phase >> shift;
					float_buf[n] += amp * lut[idx];
					phase += phase_inc;
			}

			h->phase = phase;
			break;
        }
        case 'q': { //square waveform created simply: + amplitude 1st half of the period, - amplitude 2nd half of the period
        	Harmonic *h = &v->harmonics[0];
        	register uint32_t phase = h->phase;
        	register uint32_t phase_inc = h->phase_inc;
        	register float amp = h->amp;

        	for (size_t n = 0; n < frames; n++) {

        	    if (phase < 0x80000000u)
        	        float_buf[n] += amp;
        	    else
        	        float_buf[n] -= amp;

        	    phase += phase_inc;
        	}

        	h->phase = phase;
        	break;
        }
        case 't': { //triangle is like saw, but starts going back down after half of the period
        	Harmonic *h = &v->harmonics[0];
        	register uint32_t phase = h->phase;
        	register uint32_t phase_inc = h->phase_inc;
        	register float amp = h->amp;

        	for (size_t n = 0; n < frames; n++) {
        		float saw = ((float)phase / 2147483648.0f) - 1.0f;
        		if (phase < 0x80000000u)
        			float_buf[n] += amp * saw;
        		else
        			float_buf[n] -= amp * saw;
        		phase += phase_inc;
        	}

        	h->phase = phase;
        	break;
        }
        case 'w': { //saw is just linearly rising phase, shifted down by 1 (starts from -amplitude ends at +amplitude)
        	Harmonic *h = &v->harmonics[0];
        	register uint32_t phase = h->phase;
        	register uint32_t phase_inc = h->phase_inc;
        	register float amp = h->amp;

        	for (size_t n = 0; n < frames; n++) {
        		float saw = ((float)phase / 2147483648.0f) - 1.0f;
        		float_buf[n] += amp * saw;
        		phase += phase_inc;
        	}

        	h->phase = phase;
            break;
        }
        default:
            break;
        }
    }

    float gain = s->current_gain;
    for (size_t n = 0; n < frames; n++) {
        out[n] = float_to_int16(float_buf[n] * gain);
    }
}
