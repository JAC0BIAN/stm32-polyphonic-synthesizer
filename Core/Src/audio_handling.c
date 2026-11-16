/*
 * audio_handling.c
 *
 * Created on: Jul 24, 2026
 * Author: JAC0BIAN
 * Author: ARTUR-MICHNA
 */

#include "audio_handling.h"
#include "main.h"
#include "touchgfx_board.h"
#include <stdio.h>

volatile uint8_t  dbg_usb_irq_active = 0;	// old debug flags
volatile uint16_t dbg_dma_during_usb = 0;

static volatile uint32_t dbg_last_cyc      = 0;	// CYCCNT at previous callback
static volatile uint32_t dbg_gap_max_cyc   = 0;	// max distance between callbacks		should be ~1333 us
static volatile uint32_t dbg_blocks        = 0;	// number of blocks generated			should be ~750
static volatile uint64_t dbg_total_sum_cyc = 0;	// sum of whole-callback time
static volatile uint32_t dbg_total_max_cyc = 0;	// max whole-callback time
static volatile uint32_t dbg_synth_max_cyc = 0;	// max synth_generate_block_i16 time
static volatile uint32_t dbg_midi_max_cyc  = 0;	// max midi_queue_pop time
static volatile uint32_t dbg_overruns      = 0;	// blocks that exceeded the budget
static volatile uint32_t dbg_voices_max    = 0;	// max active voices seen

static uint32_t dbg_budget_cyc = 1;			// set in audio_init()


static int16_t audio_dma_buffer[AUDIO_BUFFER_SIZE] __attribute__((aligned(32)));
static Synth synth;
static Voice voice[MAX_VOICES];
static int16_t mono_cache[AUDIO_BUFFER_SIZE];

_Static_assert((int)AUDIO_SYNTH_SUB == (int)SUB && (int)AUDIO_SYNTH_ADD == (int)ADD, "AudioSynthType must match SynthType");
_Static_assert((int)AUDIO_WAVE_TRAPEZ == (int)TRAPEZ && (int)AUDIO_WAVE_SAW == (int)SAW, "AudioWave must match WaveType");

static volatile SynthType patch_type      = BASIC;
static volatile WaveType  patch_wave      = SAW;
static volatile uint8_t   patch_harmonics = 4;

static volatile uint16_t * const fx_flag[AUDIO_FX_COUNT] = {
    &synth.lowpass,
    &synth.highpass,
    &synth.chorus_on,
    &synth.flanger_on,
    &synth.phaser_on,
    &synth.delay_on,
    &synth.reverb_on,
    &synth.overdrive_on,
    &synth.distortion_on,
    &synth.compressor_on,
};

void audio_set_effect(AudioFx fx, uint8_t on) {
    if (fx >= AUDIO_FX_COUNT) return;
    *fx_flag[fx] = on ? 1u : 0u;
}

uint8_t audio_get_effect(AudioFx fx) {
    if (fx >= AUDIO_FX_COUNT) return 0;
    return (uint8_t)*fx_flag[fx];
}

void audio_set_synth_type(AudioSynthType t) { patch_type = (SynthType)t; }
void audio_set_wave(AudioWave w)            { patch_wave = (WaveType)w; }
void audio_set_harmonics(uint8_t n) {
    if (n < 1u) n = 1u;
    if (n > MAX_HARMONICS) n = MAX_HARMONICS;
    patch_harmonics = n;
}

static struct {
    uint16_t en;
    WaveType wave;
    float    semi, detune, level;
} osc2_cfg = { 0u, SAW, -12.0f, 7.0f, 0.8f };

static volatile uint8_t osc2_dirty = 0;

void audio_set_osc2_enable(uint8_t on)       { osc2_cfg.en     = on ? 1u : 0u;   __DMB(); osc2_dirty = 1; }
void audio_set_osc2_wave(AudioWave w)        { osc2_cfg.wave   = (WaveType)w;    __DMB(); osc2_dirty = 1; }
void audio_set_osc2_semitones(float semi)    { osc2_cfg.semi   = semi;           __DMB(); osc2_dirty = 1; }
void audio_set_osc2_detune(float cents)      { osc2_cfg.detune = cents;          __DMB(); osc2_dirty = 1; }
void audio_set_osc2_level(float level)       { osc2_cfg.level  = level;          __DMB(); osc2_dirty = 1; }

AudioSynthType audio_get_synth_type(void)     { return (AudioSynthType)patch_type; }
AudioWave      audio_get_wave(void)           { return (AudioWave)patch_wave; }
uint8_t        audio_get_harmonics(void)      { return patch_harmonics; }

uint8_t        audio_get_osc2_enable(void)    { return (uint8_t)osc2_cfg.en; }
AudioWave      audio_get_osc2_wave(void)      { return (AudioWave)osc2_cfg.wave; }
float          audio_get_osc2_semitones(void) { return osc2_cfg.semi; }
float          audio_get_osc2_detune(void)    { return osc2_cfg.detune; }
float          audio_get_osc2_level(void)     { return osc2_cfg.level; }

void audio_set_volume(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    synth.volume = v;
}

float audio_get_volume(void) {
    return synth.volume;
}

void    audio_set_mute(uint8_t on) { synth.mute = on ? 1u : 0u; }
uint8_t audio_get_mute(void)       { return (uint8_t)synth.mute; }

// Midi queue
#define MIDI_QUEUE_LENGHT 16u

typedef enum {
    MIDI_EVENT_NOTE_ON,
    MIDI_EVENT_NOTE_OFF,
    MIDI_EVENT_PITCH_BEND,
	MIDI_EVENT_ALL_NOTES_OFF

} MIDI_event_type;

typedef struct {
	MIDI_event_type type;
	uint8_t note;
	uint8_t velocity;		// pressed key dynamics
	uint16_t pitch_bend;
} MIDI_event;

static volatile MIDI_event	midi_queue[MIDI_QUEUE_LENGHT];
static volatile uint32_t	midi_queue_head = 0;
static volatile uint32_t	midi_queue_tail = 0;
//static volatile uint8_t		midi_queue_count = 0;

static void midi_queue_push(uint8_t note, uint8_t velocity, MIDI_event_type type){
	uint32_t head = midi_queue_head;
	uint32_t next = (head+1u)%(MIDI_QUEUE_LENGHT);

	if ((next == midi_queue_tail)){
		return;			// Return instead of overwriting
		// TODO add logs or debug here to see if queue doesn't over fill
	}

	// Assign values and pass into queue
	midi_queue[head].type = type;
	midi_queue[head].note = note;
	midi_queue[head].velocity = velocity;
	midi_queue[head].pitch_bend = 0;
	__DMB();		// Force memory order (snippet z stackOverflow)
	midi_queue_head = next;
	//if(midi_queue_count < MIDI_QUEUE_LENGHT) midi_queue_count++;
}// queueueueueuue

static void midi_queue_pop(void){
	while (midi_queue_tail != midi_queue_head){
		__DMB();
		MIDI_event temp = midi_queue[midi_queue_tail];
		midi_queue_tail = (midi_queue_tail+1u)%(MIDI_QUEUE_LENGHT);

		switch (temp.type) {
		case MIDI_EVENT_NOTE_ON: {
			//last 3 parameters below are: number of voices in additive, basic waveform type (for basic synth type), synth type (basic, add, sub)
			synth_note_on(&synth, voice, temp.note, temp.velocity, patch_harmonics, patch_wave, patch_type);
			//TODO: change preset parameters passing from hardcoded above to config files loaded once on preset selection. ADD GUI handled selection.
			}
			break;
		case MIDI_EVENT_NOTE_OFF: {
			synth_note_off(&synth, voice, temp.note);
			}
			break;
		case MIDI_EVENT_PITCH_BEND: {
		    synth_pitch_bend(&synth, voice, temp.pitch_bend);
			}
		    break;
		case MIDI_EVENT_ALL_NOTES_OFF: {
		    synth_all_notes_off(&synth, voice);
		    break;
		}
		default: break;
		}
		//midi_queue_count--;
	}
}// queuo

//----------------------------------------------
static void midi_queue_push_pitch_bend(uint16_t value) {
    uint32_t head = midi_queue_head;
    uint32_t next = (head + 1u) % MIDI_QUEUE_LENGHT;

    if (next == midi_queue_tail)
        return;

    midi_queue[head].type = MIDI_EVENT_PITCH_BEND;
    midi_queue[head].pitch_bend = value;

    __DMB();

    midi_queue_head = next;
}
//----------------------------------------------

static void fill_stereo_block(int16_t *dst, size_t frames)
{
	// ---- profiling: start ----
	uint32_t t_start = DWT->CYCCNT;

	if (dbg_usb_irq_active) dbg_dma_during_usb++;

	if (dbg_last_cyc != 0){
	    uint32_t gap = t_start - dbg_last_cyc;
	    if (gap > dbg_gap_max_cyc) dbg_gap_max_cyc = gap;
	}
	dbg_last_cyc = t_start;
	// --------------------------
	if (osc2_dirty) {
	    osc2_dirty = 0;
	    synth_set_osc2(&synth, osc2_cfg.en, osc2_cfg.wave,
	                   osc2_cfg.semi, osc2_cfg.detune, osc2_cfg.level);
	}
	midi_queue_pop();
	midi_queue_pop();
	uint32_t t_midi = DWT->CYCCNT;

    synth_generate_block_i16(&synth, voice, mono_cache, frames);
    uint32_t t_synth = DWT->CYCCNT;

    // Duplicate mono channel to emulate stereo
    // TODO move stereo generation to synth
    for (size_t n = 0; n < frames; n++) {
        dst[2 * n]     = mono_cache[n];
        dst[2 * n + 1] = mono_cache[n];
    }

    // ---- profiling: end ----
    uint32_t t_end = DWT->CYCCNT;

    uint32_t midi_cyc  = t_midi  - t_start;
    uint32_t synth_cyc = t_synth - t_midi;
    uint32_t total_cyc = t_end   - t_start;

    if (midi_cyc  > dbg_midi_max_cyc)  dbg_midi_max_cyc  = midi_cyc;
    if (synth_cyc > dbg_synth_max_cyc) dbg_synth_max_cyc = synth_cyc;
    if (total_cyc > dbg_total_max_cyc) dbg_total_max_cyc = total_cyc;
    if (total_cyc > dbg_budget_cyc)    dbg_overruns++;
    if (synth.active_voices > dbg_voices_max) dbg_voices_max = synth.active_voices;

    dbg_total_sum_cyc += total_cyc;
    dbg_blocks++;
    // ------------------------

}


void audio_init(void)
{
	if (BSP_SDRAM_Init() != SDRAM_OK)
	{
		TouchGFX_LogError("BOOT ERROR: SDRAM init failed\r\n");
		Error_Handler();
	}
	TouchGFX_LogError("BOOT: SDRAM init OK\r\n");
    synth_init(&synth, voice);

    // voice.num_harmonics = 0;

    // ---- additional oscillators (config BEFORE audio begins) ----
    synth_set_osc2(&synth, osc2_cfg.en, osc2_cfg.wave, osc2_cfg.semi, osc2_cfg.detune, osc2_cfg.level);


    // ---- DWT cycle counter ----
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    *(volatile uint32_t *)0xE0001FB0 = 0xC5ACCE55;	// DWT->LAR unlock
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // cycles available for one block
    dbg_budget_cyc = (uint32_t)(((uint64_t)SystemCoreClock * AUDIO_BLOCK_SIZE) / 48000u);
    // ---------------------------


    fill_stereo_block(&audio_dma_buffer[0], AUDIO_BLOCK_SIZE);
    fill_stereo_block(&audio_dma_buffer[AUDIO_BLOCK_SIZE * 2], AUDIO_BLOCK_SIZE);

    SCB_CleanDCache_by_Addr((uint32_t*)audio_dma_buffer, sizeof(audio_dma_buffer));

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, AUDIO_FREQUENCY_48K) != AUDIO_OK)
    {
        TouchGFX_LogError("BOOT ERROR: audio codec init failed\r\n");
        return;
    }

    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 1, 0);

    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    if (BSP_AUDIO_OUT_Play((uint16_t *)audio_dma_buffer, sizeof(audio_dma_buffer)) != AUDIO_OK)
    {
        TouchGFX_LogError("BOOT ERROR: audio playback start failed\r\n");
        return;
    }
    TouchGFX_LogError("BOOT: audio playback started\r\n");
//        BSP_AUDIO_OUT_Play((uint16_t *)audio_dma_buffer, AUDIO_BUFFER_SIZE);
}

void audio_note_on(uint8_t note, uint8_t velocity) {
	if (velocity == 0) { // velocity 0 is note-off in MIDI standard
		if (note < 21u || note > 108u) return;
	    audio_note_off(note);
	    return;
	}

    midi_queue_push(note, velocity, MIDI_EVENT_NOTE_ON);
}

void audio_note_off(uint8_t note) {
	if (note < 21u || note > 108u) return;
    midi_queue_push(note, 0, MIDI_EVENT_NOTE_OFF);
}

void audio_all_notes_off(void) {
    midi_queue_push(0, 0, MIDI_EVENT_ALL_NOTES_OFF);
}

void audio_pitch_bend(uint16_t value) {
    midi_queue_push_pitch_bend(value);
}

// - debug attempt - -> now used for performance checks
void audio_debug_print(UART_HandleTypeDef *huart)
{
    __disable_irq();
    uint32_t blocks    = dbg_blocks;
    uint64_t sum_cyc   = dbg_total_sum_cyc;
    uint32_t total_max = dbg_total_max_cyc;
    uint32_t synth_max = dbg_synth_max_cyc;
    uint32_t midi_max  = dbg_midi_max_cyc;
    uint32_t overruns  = dbg_overruns;
    uint32_t voices    = dbg_voices_max;
    uint32_t gap_max   = dbg_gap_max_cyc;

    dbg_blocks = 0;
    dbg_total_sum_cyc = 0;
    dbg_total_max_cyc = 0;
    dbg_synth_max_cyc = 0;
    dbg_midi_max_cyc = 0;
    dbg_overruns = 0;
    dbg_voices_max = 0;
    dbg_gap_max_cyc = 0;
    dbg_dma_during_usb = 0;
    __enable_irq();

    uint32_t avg_cyc  = blocks ? (uint32_t)(sum_cyc / blocks) : 0;

    // load in 0.1 % units (integer math - newlib-nano printf has no %f)
    uint32_t avg_load = (uint32_t)(((uint64_t)avg_cyc   * 1000u) / dbg_budget_cyc);
    uint32_t max_load = (uint32_t)(((uint64_t)total_max * 1000u) / dbg_budget_cyc);
    uint32_t gap_us   = gap_max / (SystemCoreClock / 1000000u);

    char buf[160];
    int len = snprintf(buf, sizeof(buf),
        "CPU avg %lu.%lu%% max %lu.%lu%% | synth %lu midi %lu cyc | budget %lu | over %lu | voices %lu | gap %luus | blk %lu\r\n",
        (unsigned long)(avg_load / 10u), (unsigned long)(avg_load % 10u),
        (unsigned long)(max_load / 10u), (unsigned long)(max_load % 10u),
        (unsigned long)synth_max,
        (unsigned long)midi_max,
        (unsigned long)dbg_budget_cyc,
        (unsigned long)overruns,
        (unsigned long)voices,
        (unsigned long)gap_us,
        (unsigned long)blocks);

    if (len > 0) {
        if (len > (int)sizeof(buf)) len = sizeof(buf) - 1;
        HAL_UART_Transmit(huart, (uint8_t *)buf, (uint16_t)len, 100);
    }

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
