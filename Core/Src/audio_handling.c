/*
 * audio_handling.c
 *
 *  Created on: Jul 24, 2026
 *      Author: JAC0BIAN
 */

/* audio_handling.c */

#include "audio_handling.h"
#include "main.h"

static int16_t audio_dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));
static Synth synth;
static Voice voice;
static int16_t mono_cache[AUDIO_BUFFER_SIZE];


static void fill_stereo_block(int16_t *dst, size_t frames){
	synth_generate_block_i16(&synth, &voice, mono_cache, frames);
	// Duplicate mono channel to emulate stereo
	// TODO move stereo generation to synth
	for (size_t n = 0; n < frames; n++){
		dst[2*n] = mono_cache[n];
		dst[2*n+1] = mono_cache[n];
	}
}


void audio_init(void)
{
    synth_init(&synth, &voice);

    voice.num_harmonics = 0;

    fill_stereo_block(&audio_dma_buffer[0], AUDIO_BLOCK_SIZE);
    fill_stereo_block(&audio_dma_buffer[AUDIO_BLOCK_SIZE * 2], AUDIO_BLOCK_SIZE);

    SCB_CleanDCache_by_Addr((uint32_t*)audio_dma_buffer, sizeof(audio_dma_buffer));

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, AUDIO_FREQUENCY_48K) == AUDIO_OK)
    {
        BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
        BSP_AUDIO_OUT_Play((uint16_t*)audio_dma_buffer, AUDIO_BUFFER_SIZE);
    }
}

void audio_note_on(uint8_t note, uint8_t velocity)
{
    (void)velocity;

    if (voice.midi_note == (int)note && voice.num_harmonics > 0)
    {
        return;
    }

    synth_note_on(&synth, &voice, note, 1);
}

void audio_note_off(uint8_t note)
{

    if (voice.midi_note == note)
    {
        voice.num_harmonics = 0;
    }
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[0];
    fill_stereo_block(ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t*)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[AUDIO_BLOCK_SIZE * 2];
    fill_stereo_block(ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t*)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}
