/*Simple DDS based synth*/

#include "synth.h"

#include <stdint.h>
#include <stddef.h>
#include <math.h>
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
	v -> env.state = ADSR_OFF;
	v -> env.level = 0.0f;
	v -> env.attack_inc = 0.0f;
	v -> env.decay_inc = 0.0f;
	v -> env.release_inc = 0.0f;
}
// ----------------------------------- ADSR ---------------------------------------------
// ADSR_state -> enum list with all states of the ADSR
//
static void ADSR_start(ADSR_envelope *e, const ADSR_params *p, float sr ){
	float a = p -> attack_t;
	float d = p -> decay_t;

	if (a < 0.001f){a = 0.001f;}
	if (d < 0.001f){d = 0.001f;}


	e->attack_inc = 1.0/(a*sr);
	e->decay_inc  = (1.0f - p->sustain)/(d*sr);
	e->state = ADSR_ATTACK;

}

static void ADSR_release(ADSR_envelope *e, const ADSR_params *p, float sr){
	float r = p->release_t;
	if (r < 0.001f){r = 0.001f;}


	e->release_inc = e->level / (r *sr);
	e->state = ADSR_RELEASE;
}

static float ADSR_tick(ADSR_envelope *e, float sustain) {
	switch (e->state){					// kocham stanowe maszyny
	case ADSR_ATTACK:
		e->level += e->attack_inc;		// if volume reaches max value then state switches to decay
		if (e->level >= 1.0f) {
			e->level = 1.0f;
			e->state = ADSR_DECAY;
		}
		break;
	case ADSR_DECAY:
		e->level -= e->decay_inc;
		if (e->level <= sustain){		// when the volume decreases to the sustain level it will
		e->level = sustain;				// stay there as long as the key is held
		e->state = ADSR_SUSTAIN;
		}
		break;
	case ADSR_SUSTAIN:
		e->level = sustain;
		break;
	case ADSR_RELEASE:
		e->level -= e->release_inc;
		if (e->level <= 0.0f){
		e->level = 0.0f;
		e->state = ADSR_OFF;
		}
		break;
	default:				// state -> note off
		e->level = 0.0f;
		break;
	}
	return e->level;
}
// ================================== Filters ===========================================
void LPF_Init(LPF * f, float cutoff)
{
	f->alpha = cutoff/(cutoff + SAMPLE_RATE_HZ/(2.0f*PI));
	for (int i = 0; i < LPF_STAGES; i++) f->prev_y[i] = 0.0f;
}

float LPF_Process(LPF *f, float x)
{
	for (int i = 0; i < LPF_STAGES; i++) {
	        f->prev_y[i] = (1.0f - f->alpha) * f->prev_y[i] + f->alpha * x;
	        x = f->prev_y[i];   // output of this stage feeds the next
	    }
	return x;
}

// =================================== Synth ============================================
void update_synth_gain(Synth *s) {
    float mod = (float)s->active_voices;
    if (mod < 1.0f) mod = 1.0f;
    s->target_gain = s->master_gain / sqrtf(mod);
}

void synth_init(Synth *s, Voice *v){
    //
    s -> sample_rate = SAMPLE_RATE_HZ;
    s -> master_gain = 0.1f;
    s -> gain_inc = 0.01f;
	s -> active_voices = 0;
    v -> num_harmonics = 0;
    v -> midi_note = 69;
    v -> f0_hz = 420.0f;

    s->env.attack_t  = 2.01f;
    s->env.decay_t   = 0.10f;
    s->env.sustain   = 0.70f;
    s->env.release_t = 0.30f; //INITIAL ADSR PRESETS

    //s->env.attack_t  = 0.005f;
    //s->env.decay_t   = 1.00f;
    //s->env.sustain   = 0.00f;
    //s->env.release_t = 0.01f; //ADSR preset to showcase Subtractive synthesis (plucked string imitation)

    generate_sine_lut(s -> sine_lut, LUT_SIZE);

    // MIDI note to frequency
    for (uint32_t i = 21; i < 109; i++){
    	s->midi_freakyuency[i] = (uint16_t)(440.0f * powf(2.0f, ((float)i - 69.0f) / 12.0f) + 0.5f);
    }

    for (uint32_t i = 0; i < MAX_VOICES; i++){
    	voice_reset(&v[i]);
    }

    update_synth_gain(s);
    s -> current_gain = s -> target_gain;
    LPF_Init(&s->lpf, 1200.0f);
    Chorus_Init(&s -> chorus, s->sine_lut);
    Flanger_Init(&s -> flanger, s -> sine_lut);
}


void synth_note_on(Synth *s, Voice *vs, int midi_note,uint8_t velocity, uint32_t requested_harmonics, WaveType wave, SynthType synth_type){
	if (midi_note < 20) midi_note = 20;
	if (midi_note > 109) midi_note = 109;
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
    v -> synth_type = synth_type;

    ADSR_start(&v->env, &s->env, s->sample_rate);

    float nyquist = s -> sample_rate * 0.5f;                // Antialiasing filter
    uint32_t max_nyquist = (uint32_t)(nyquist/ (v -> f0_hz));
    if (max_nyquist < 1u) max_nyquist = 1u;

    uint32_t n = requested_harmonics;
    if (n > MAX_HARMONICS) n = MAX_HARMONICS;               // Reduce to permited max harmonics
    if (n > max_nyquist) n = max_nyquist;                   // Reduce to max without aliasing
    v -> num_harmonics = n;

    float norm_velocity = (float)velocity/127.0f;

    if (synth_type == ADD) {
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
    } else {
    	Harmonic *h = &v->harmonics[0];

    	h->phase = 0;
    	h->phase_inc = freq_to_phase_inc(
    	    v->f0_hz, s->sample_rate
    	);
    	h->amp = norm_velocity;
    }

    for (uint32_t i = n; i < MAX_HARMONICS; i++){
    	 	v->harmonics[i].phase = 0;
            v -> harmonics[i].phase_inc = 0;
            v -> harmonics[i].amp = 0.0f;
    }

    update_synth_gain(s);
}

/*
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
*/
static void free_unused_voices(Synth *s, Voice *vs){
	uint32_t i = 0;
	while(i < s->active_voices){
		if(vs[i].env.state == ADSR_OFF){
			vs[i] = vs[s->active_voices - 1];
			voice_reset(&vs[s->active_voices-1]);
			s->active_voices--;
		} else{
			i++;									//pls work 👉👈
		}
	}
	update_synth_gain(s);
}

void synth_note_off(Synth *s, Voice *vs, int midi_note){
	for (uint32_t i = 0; i < s->active_voices; i++){
		if(vs[i].midi_note == midi_note &&
		   vs[i].env.state != ADSR_RELEASE &&
		   vs[i].env.state != ADSR_OFF){
			ADSR_release(&vs[i].env, &s->env, s->sample_rate);
			return;
		}
	}
}

void synth_generate_block_i16(Synth *s, Voice *voices, int16_t *out, size_t frames) {
    static float float_buf[MAX_BLOCK_FRAMES]; //static - no allocation on stack
    static float voice_buf[MAX_BLOCK_FRAMES];
    int		 	 lowpass 	= 0; //flag for LPF
    int		 	 chorus_on 	= 0; //flag for chorus effect
    int		 	 flanger_on = 1; //flag for flanger effect

    if (frames > MAX_BLOCK_FRAMES) frames = MAX_BLOCK_FRAMES;

    for (size_t n = 0; n < frames; n++) {
        float_buf[n] = 0.0f;
    }

    uint32_t shift = 32u - LUT_BITS;
    const float *lut = s->sine_lut;

    // only active voices, avoid unnecessary checking of inactive voices
    for (uint32_t vi = 0; vi < s->active_voices; vi++) {
        Voice *v = &voices[vi];

        for(size_t n =0; n < frames; n++) voice_buf[n] = 0.0f;

        switch(v->synth_type){
        case ADD: {
        	uint32_t n_harm = v->num_harmonics;
			for (uint32_t i = 0; i < n_harm; i++) {
				Harmonic *h = &v->harmonics[i];

				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {
					uint32_t idx = phase >> shift;
					voice_buf[n] += amp * lut[idx];
					phase += phase_inc;
				}

				h->phase = phase; //save phase to RAM only once per block
			}
			break;
        }
        case BASIC: {
        	switch(v->wave){
			case SINE: {
				if (v->synth_type == SUB) break;
				Harmonic *h = &v->harmonics[0];

				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {
						uint32_t idx = phase >> shift;
						voice_buf[n] += amp * lut[idx];
						phase += phase_inc;
				}

				h->phase = phase;
				break;
			}
			case SQUARE: { //square waveform created simply: + amplitude 1st half of the period, - amplitude 2nd half of the period
				Harmonic *h = &v->harmonics[0];
				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {

					if (phase < 0x80000000u)
						voice_buf[n] += amp;
					else
						voice_buf[n] -= amp;

					phase += phase_inc;
				}

				h->phase = phase;
				break;
			}
			case TRIANGLE: { //triangle is like saw, but starts going back down after half of the period
				Harmonic *h = &v->harmonics[0];
				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {
					float saw = ((float)phase / 2147483648.0f) - 1.0f;
					if (phase < 0x80000000u)
						voice_buf[n] += amp * saw;
					else
						voice_buf[n] -= amp * saw;
					phase += phase_inc;
				}

				h->phase = phase;
				break;
			}
			case SAW: { //saw is just linearly rising phase, shifted down by 1 (starts from -amplitude ends at +amplitude)
				Harmonic *h = &v->harmonics[0];
				register uint32_t phase = h->phase;
				register uint32_t phase_inc = h->phase_inc;
				register float amp = h->amp;

				for (size_t n = 0; n < frames; n++) {
					float saw = ((float)phase / 2147483648.0f) - 1.0f;
					voice_buf[n] += amp * saw;
					phase += phase_inc;
				}

				h->phase = phase;
				break;
			}
			default:
				break;
			}
        	break;
        	}
        case SUB: {
        	lowpass = 1;
        	Harmonic *h = &v->harmonics[0];
        	register uint32_t phase = h->phase;
        	register uint32_t phase_inc = h->phase_inc;
        	register float amp = h->amp;

        	for (size_t n = 0; n < frames; n++) {
        		float saw = ((float)phase / 2147483648.0f) - 1.0f;
        		voice_buf[n] += amp * saw;
        		phase += phase_inc;
        	}

        	h->phase = phase;
        	break;
        }
        default:
        	break;
        }

        for (size_t n = 0; n < frames; n++){
        	float_buf[n] += voice_buf[n] * ADSR_tick(&v->env, s->env.sustain);
        }
    }

    free_unused_voices(s, voices);

    float gain = s->current_gain;
    const float target = s->target_gain;
    const float k = s->gain_inc;

    for (size_t n = 0; n < frames; n++) {
    	gain += (target-gain)*k;
    	float sample = float_buf[n] * gain;
    	if (lowpass == 1){ //TEMP: LPF only applied to subtractive synthesis TODO: apply to presets that use LPF
    		sample = LPF_Process(&s->lpf, sample)* 4.0f; //TODO: normalize volume after filtering
    	}
    	if (chorus_on == 1){
    		sample = Chorus_Process(&s->chorus, sample);
    	}
    	if (flanger_on == 1){
    	    sample = Flanger_Process(&s->flanger, sample);
    	}
        out[n] = float_to_int16(sample);
        s->current_gain = gain;
    }

}
