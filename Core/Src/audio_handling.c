/*
 * audio_handling.c
 *
 * Created on: Jul 24, 2026
 * Author: JAC0BIAN
 * Author: ARTUR-MICHNA
 */

#include "audio_handling.h"
#include "main.h"
#include <stdio.h>

// - debug attempt -
volatile uint32_t dbg_last_cyc = 0;
volatile uint32_t dbg_call_count = 0;
volatile uint8_t  dbg_usb_irq_active = 0;
volatile uint16_t dbg_dma_during_usb = 0;
volatile uint32_t dbg_max_gap_us = 0;
// -----------------

static int16_t audio_dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));
static Synth synth;
static Voice voice[MAX_VOICES];
static int16_t mono_cache[AUDIO_BUFFER_SIZE];

// Midi queue
#define MIDI_QUEUE_LENGHT 16u

typedef struct{
	uint8_t note;
	uint8_t note_on;		// 1 - on , 0 - off
} MIDI_event;

static volatile MIDI_event	midi_queue[MIDI_QUEUE_LENGHT];
static volatile uint32_t	midi_queue_head = 0;
static volatile uint32_t	midi_queue_tail = 0;
//static volatile uint8_t		midi_queue_count = 0;

static void midi_queue_push(uint8_t note, uint8_t note_on){
	uint32_t head = midi_queue_head;
	uint32_t next = (head+1u)%(MIDI_QUEUE_LENGHT);

	if ((next == midi_queue_tail)){
		return;			// Return instead of overwriting
		// TODO add logs or debug here to see if queue doesn't over fill
	}

	// Assign values and pass into queue
	midi_queue[head].note = note;
	midi_queue[head].note_on = note_on;
	__DMB();		// Force memory order (snippet z stackOverflow)
	midi_queue_head = next;
	//if(midi_queue_count < MIDI_QUEUE_LENGHT) midi_queue_count++;
}// queueueueueuue

static void midi_queue_pop(void){
	while (midi_queue_tail != midi_queue_head){
		__DMB();
		MIDI_event temp = midi_queue[midi_queue_tail];
		midi_queue_tail = (midi_queue_tail+1u)%(MIDI_QUEUE_LENGHT);

		if (temp.note_on){
			synth_note_on(&synth, voice, temp.note, 1);
		}
		else{
			synth_note_off(&synth, voice, temp.note);
		}
		//midi_queue_count--;
	}
}

//----------------------------------------------

static void fill_stereo_block(int16_t *dst, size_t frames)
{
	// - debug attempt -

	if (dbg_usb_irq_active) dbg_dma_during_usb++;
	//- - - - - - - - - ARTUR
	uint32_t now = DWT->CYCCNT;
	if (dbg_last_cyc != 0){
	    uint32_t gap_cycles = now - dbg_last_cyc;
	    uint32_t gap_us = gap_cycles / (HAL_RCC_GetHCLKFreq() / 1000000u);
	    if (gap_us > dbg_max_gap_us) dbg_max_gap_us = gap_us;
	}
	dbg_last_cyc = now;
	dbg_call_count++;
	// -----------------
	midi_queue_pop();
    synth_generate_block_i16(&synth, voice, mono_cache, frames);
    // Duplicate mono channel to emulate stereo
    // TODO move stereo generation to synth
    for (size_t n = 0; n < frames; n++) {
        dst[2 * n]     = mono_cache[n];
        dst[2 * n + 1] = mono_cache[n];
    }
}


void audio_init(void)
{
    synth_init(&synth, voice);

    // voice.num_harmonics = 0;

    // - debug attempt -
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // - count DWT cycles -

    fill_stereo_block(&audio_dma_buffer[0], AUDIO_BLOCK_SIZE);
    fill_stereo_block(&audio_dma_buffer[AUDIO_BLOCK_SIZE * 2], AUDIO_BLOCK_SIZE);

    SCB_CleanDCache_by_Addr((uint32_t*)audio_dma_buffer, sizeof(audio_dma_buffer));

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, AUDIO_FREQUENCY_48K) == AUDIO_OK)
    {
    	HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 1, 0);

        BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
        BSP_AUDIO_OUT_Play((uint16_t *)audio_dma_buffer, AUDIO_BUFFER_SIZE);
    }
}

void audio_note_on(uint8_t note, uint8_t velocity)
{
    (void)velocity;

    midi_queue_push(note, 1);
}

void audio_note_off(uint8_t note)
{
    midi_queue_push(note, 0);
}

// - debug attempt -
void audio_debug_print(UART_HandleTypeDef *huart)
{
    char buf[96];
    int len = snprintf(buf, sizeof(buf),
        "gap_max=%luus count=%lu dma_during_usb=%u\r\n",
        (unsigned long)dbg_max_gap_us,
        (unsigned long)dbg_call_count,
        (unsigned)dbg_dma_during_usb);

    if (len > 0) {
        HAL_UART_Transmit(huart, (uint8_t *)buf, (uint16_t)len, 100);
    }

    dbg_max_gap_us = 0;
    dbg_call_count = 0;
    dbg_dma_during_usb = 0;
}
// -----------------

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[0];
    fill_stereo_block(ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t *)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    int16_t *ptr = &audio_dma_buffer[AUDIO_BLOCK_SIZE * 2];
    fill_stereo_block(ptr, AUDIO_BLOCK_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t *)ptr, AUDIO_BLOCK_SIZE * 2 * sizeof(int16_t));
}
