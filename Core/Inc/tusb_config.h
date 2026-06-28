/*
 * tusb_config.h
 *
 *  Created on: Jun 17, 2026
 *      Author: Jakub Lukaszewski
 */

#ifndef INC_TUSB_CONFIG_H_
#define INC_TUSB_CONFIG_H_

#define CFG_TUSB_MCU             OPT_MCU_STM32F7
#define CFG_TUSB_RHPORT0_MODE    (OPT_MODE_HOST | OPT_MODE_FULL_SPEED)
#define CFG_TUH_MIDI             1
#define CFG_TUH_MIDI_RX_BUFSIZE  64
#define CFG_TUH_MIDI_TX_BUFSIZE  64
#define CFG_TUH_DEVICE_MAX       1

#define BOARD_TUH_RHPORT     	 0
#endif /* INC_TINYUSB_CONFIG_H_ */
