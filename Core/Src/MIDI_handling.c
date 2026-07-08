#include "MIDI_handling.h"
#include "main.h"       
#include "tusb.h"       

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
}

void tuh_midi_rx_cb(uint8_t idx, uint32_t xferred_bytes)
{
  (void) xferred_bytes;
  uint8_t packet[4];

  while (tuh_midi_packet_read(idx, packet))
  {
    uint8_t status   = packet[1] & 0xF0; // apply mask to ignore channel number (only one device is connected)
    //uint8_t note     = packet[2]; //unused for now
    uint8_t velocity = packet[3];

    if (status == 0x90 && velocity > 0) // Note-On event (0x90 = 1001nnn status byte)
    {
    	//HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_SET);
    }
    else if (status == 0x80 || (status == 0x90 && velocity == 0)) // Note-Off event (0x80 = 1000nnn status byte)
    {
    	//HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);
    }
  }
}
