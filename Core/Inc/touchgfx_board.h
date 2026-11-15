#ifndef TOUCHGFX_BOARD_H
#define TOUCHGFX_BOARD_H

#include "stm32f7xx_hal.h"

#define TOUCHGFX_FRAMEBUFFER_ADDRESS 0xC0100000U

#ifdef __cplusplus
extern "C" {
#endif

extern LTDC_HandleTypeDef hltdc;

void TouchGFX_LTDC_Init(void);
void TouchGFX_LogError(const char *message);
void TouchGFX_ReportFatalError(const char *message);

#ifdef __cplusplus
}
#endif

#endif
