#include "MIDI_handling.h"
#include "audio_handling.h"
#include "main.h"       
#include "tusb.h"

// - debug attempt -
//volatile uint8_t dbg_usb_irq_active = 0;
//volatile uint32_t dbg_dma_during_usb = 0;
// -----------------

void MIDI_Init(void)
{
    tusb_init(BOARD_TUH_RHPORT);
}

void MIDI_Process(void)
{
    tuh_task();
}

uint32_t tusb_time_millis_api(void)
{
  return HAL_GetTick();
}

void tuh_midi_mount_cb(uint8_t idx, const tuh_midi_mount_cb_t *mount_cb_data)
{
  (void) idx;
  (void) mount_cb_data;

  //turn on LED when device is connected
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);
}

void tuh_midi_umount_cb(uint8_t idx)
{
  (void) idx;
  //turn off LED when device is disconnected
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);
  audio_all_notes_off();
}

void tuh_midi_rx_cb(uint8_t idx, uint32_t xferred_bytes)
{
  (void) xferred_bytes;
  uint8_t packet[4];

  while (tuh_midi_packet_read(idx, packet))
  {
	uint8_t system  = packet[0] & 0x0F;
	uint8_t status 	= packet[1] & 0xF0;
    uint8_t data1 	= packet[2];
    uint8_t data2 	= packet[3];

    if (data1 >= 128 || data2 >= 128) // MIDI data bytes must be 7-bit
               continue;

    if (status == 0x90 && data2 > 0 && system == 0x9) // Note-On event (0x90 = 1001nnn status byte)
    {
    	audio_note_on(data1, data2);
    }
    else if ((status == 0x80 && system == 0x8) ||
    		 (status == 0x90 && system == 0x9 && data2 == 0)) // Note-Off event (0x80 = 1000nnn status byte)
    {
    	audio_note_off(data1);
    }
    else if (status == 0xE0 && system == 0xE) {
        uint16_t pitch_bend = data1 | (data2 << 7);
        audio_pitch_bend(pitch_bend);
    }
    else {
    	continue;
    }
  }
}
