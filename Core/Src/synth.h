
#ifndef SINH
#define SINH

#include <stdint.h>
#include <stddef.h>

#include "dds.h"
// ---------------------- test --------------------------
typedef enum {	// code will switch between them to simulate a sound envelope
	ADSR_ATTACK,		// how fast the volume rises
	ADSR_DECAY,		// how fast it falls to normall level
	ADSR_SUSTAIN,	// how long it stays at normall level
	ADSR_RELEASE,		// how fast it falls
	ADSR_OFF			// signal to clean the note
} ADSR_state;

typedef struct {
	float 		attack_t;
	float 		decay_t;
	float 		sustain;
	float 		release_t;
}ADSR_params;

typedef struct{
	ADSR_state	state;
	float		level;
	float		attack_inc;
	float		decay_inc;
	float 		release_inc;
}ADSR_envelope;

// -------------------------------------------------------

typedef enum {  // different ways of audio generation:
    BASIC,		// basic waveforms
    ADD,		// additive synthesis
    SUB			// subtractive synthesis
} SynthType;

typedef enum {  // basic waveforms
	SINE,
	SAW,
	SQUARE,
	TRIANGLE
} WaveType;

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
    ADSR_params env;				// todo zmienić nazewe
}Synth;


typedef struct{
    Harmonic    harmonics[MAX_HARMONICS];
    uint32_t    num_harmonics;

    SynthType 	synth_type;
    WaveType 	wave;

    int         midi_note;
    float       f0_hz;

    ADSR_envelope env;
}Voice;

void 		 synth_init(Synth *s, Voice *v);
void         synth_note_on(Synth *s, Voice *v, int midi_note, uint8_t velocity, uint32_t requested_harmonics, WaveType wave, SynthType synth_type);
void         synth_note_off(Synth *s, Voice *v, int midi_note);
float        synth_process_one(Synth *s, Voice *v);
void         synth_generate_block_i16(Synth *s, Voice *v, int16_t *out, size_t frames);

#endif /* SINH */
