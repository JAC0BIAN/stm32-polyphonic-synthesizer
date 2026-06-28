# STM32-based polyphonic digital sound synthesizer

This project is a real-time polyphonic digital synthesizer developed as part of an engineering thesis. It is designed for an embedded STM32 platform and features polyphonic audio synthesis, multiple oscillator types, digital filters, ADSR envelopes, time-based audio effects, USB-MIDI support, and a touchscreen user interface. The project focuses on efficient DSP implementation to achieve low-latency, high-quality audio performance.

## Project status
- [x] Basic project setup
- [x] MIDI-USB communication
- [ ] Audio synthesis
- [ ] Effects
- [ ] Modulation
- [ ] Touchscreen GUI

## Requirements

The project is developed and tested on the **STM32F746G-DISCO Discovery Kit** by **STMicroelectronics**.

### Required software:

- **STM32CubeIDE** 1.18.1 or newer (newer versions may require project metadata updates)
- **STM32CubeMX** (required separately in newer STM32CubeIDE versions for editing `.ioc` configuration files)

### Required hardware:

- **STM32F746G-DISCO** Discovery Kit
- USB-MIDI controller
- **USB Mini-B** cable for the ST-LINK interface – used for programming, debugging and primary board power
- **Micro-USB** cable for the USB OTG FS connector – used to connect the MIDI controller
- External 5 V USB power supply for the USB OTG FS port (USB host operation)

### Power Configuration

The board should be configured to use the **ST-LINK USB connector as the primary power source**. Set the power selection jumper accordingly, as described in the **STM32F746G-DISCO** user manual.

When using a **USB-MIDI controller**, the **USB OTG FS connector** requires an **external 5 V supply** to provide power to the connected USB device (connect the power supply to **USB HS** port).

## Setup

To run the project on your **STM32F746G-DISCO** board, connect it to your PC (as described in the user manual), open the project in **STM32CubeIDE** and compile the firmware.

## Authors

- Jakub Łukaszewski (@JAC0BIAN)
- Artur Michna (@Artur-Michna)

## Licenses

This project uses external software components licensed as follows:

| Component               | License                                                                                                               |
|-------------------------|-----------------------------------------------------------------------------------------------------------------------|
| github/gitignore        | [Creative Commons Zero v1.0 Universal (CC0)](https://github.com/github/gitignore/blob/main/LICENSE)                   |
| TinyUSB                 | [MIT License](https://github.com/hathach/tinyusb/blob/master/LICENSE)                                                 |
| STM32F7 HAL             | [BSD 3-Clause](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md)                              |
| BSP STM32746G-Discovery | [BSD 3-Clause](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md)                              |
| CMSIS                   | [Apache License 2.0](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md)                        |
| CMSIS Device            | [Apache License 2.0](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md)                        |
| STM32Utilities          | [BSD 3-Clause](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md)                              |
| STM32 Projects          | [SLA0044 (BSD-3-Clause for basic Examples)](https://github.com/STMicroelectronics/STM32CubeF7/blob/master/LICENSE.md) |