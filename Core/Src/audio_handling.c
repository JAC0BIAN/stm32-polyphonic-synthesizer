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

void audio_init(void)
{
    synth_init(&synth);

    synth.num_harmonics = 0;

    synth_generate_block_stereo_i16(&synth, &audio_dma_buffer[0], AUDIO_BLOCK_SIZE);
    synth_generate_block_stereo_i16(&synth, &audio_dma_buffer[AUDIO_BLOCK_SIZE * 2], AUDIO_BLOCK_SIZE);

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

    if (synth.midi_note == (int)note && synth.num_harmonics > 0)
    {
        return;
    }

    synth_note_on(&synth, note, 1);
}

void audio_note_off(uint8_t note)
{

    if (synth.midi_note == note)
    {
        synth.num_harmonics = 0;
    }
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[0];
    synth_generate_block_stereo_i16(&synth, ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t*)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[AUDIO_BLOCK_SIZE * 2];
    synth_generate_block_stereo_i16(&synth, ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t*)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}
