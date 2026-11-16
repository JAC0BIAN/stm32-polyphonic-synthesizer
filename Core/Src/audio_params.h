/*
 * audio_params.h
 *
 *  Created on: Nov 15, 2026
 *      Author: JAC0BIAN
 */

#ifndef AUDIO_PARAMS_H_
#define AUDIO_PARAMS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AUDIO_FX_LPF = 0,
    AUDIO_FX_HPF,
    AUDIO_FX_CHORUS,
    AUDIO_FX_FLANGER,
    AUDIO_FX_PHASER,
    AUDIO_FX_DELAY,
    AUDIO_FX_REVERB,
    AUDIO_FX_OVERDRIVE,
    AUDIO_FX_DISTORTION,
    AUDIO_FX_COMPRESSOR,
    AUDIO_FX_COUNT
} AudioFx;

// Same order as SynthType / WaveType in synth.h (checked by _Static_assert in the .c file)
typedef enum { AUDIO_SYNTH_BASIC = 0, AUDIO_SYNTH_ADD, AUDIO_SYNTH_SUB } AudioSynthType;
typedef enum { AUDIO_WAVE_SINE = 0, AUDIO_WAVE_SAW, AUDIO_WAVE_SQUARE,
               AUDIO_WAVE_TRIANGLE, AUDIO_WAVE_TRAPEZ } AudioWave;

void    audio_set_effect(AudioFx fx, uint8_t on);
uint8_t audio_get_effect(AudioFx fx);          // to initialise toggle buttons from the GUI

void    audio_set_synth_type(AudioSynthType t);
void    audio_set_wave(AudioWave w);
void    audio_set_harmonics(uint8_t n);

void audio_set_osc2_enable(uint8_t on);
void audio_set_osc2_wave(AudioWave w);
void audio_set_osc2_semitones(float semi);
void audio_set_osc2_detune(float cents);
void audio_set_osc2_level(float level);

void  audio_set_volume(float v);
float audio_get_volume(void);

void    audio_set_mute(uint8_t on);
uint8_t audio_get_mute(void);

AudioSynthType audio_get_synth_type(void);
AudioWave      audio_get_wave(void);
uint8_t        audio_get_harmonics(void);
uint8_t        audio_get_osc2_enable(void);
AudioWave      audio_get_osc2_wave(void);
float          audio_get_osc2_semitones(void);
float          audio_get_osc2_detune(void);
float          audio_get_osc2_level(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_PARAMS_H_ */
