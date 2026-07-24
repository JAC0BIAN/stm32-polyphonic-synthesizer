/*
 * audio_handling.h
 *
 *  Created on: Jul 24, 2026
 *      Author: JAC0BIAN
 */

#ifndef SRC_AUDIO_HANDLING_H_
#define SRC_AUDIO_HANDLING_H_

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_audio.h"
#include "additive_synth.h"

#define AUDIO_BLOCK_SIZE    256
#define AUDIO_BUFFER_SIZE   (AUDIO_BLOCK_SIZE * 2 * 2)

void audio_init(void);
void audio_note_on(uint8_t note, uint8_t velocity);
void audio_note_off(uint8_t note);

#endif /* SRC_AUDIO_HANDLING_H_ */
