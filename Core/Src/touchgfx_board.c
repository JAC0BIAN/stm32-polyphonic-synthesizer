#include "touchgfx_board.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

LTDC_HandleTypeDef hltdc;

void TouchGFX_LTDC_Init(void)
{
  LTDC_LayerCfgTypeDef layer = {0};

  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 40;
  hltdc.Init.VerticalSync = 9;
  hltdc.Init.AccumulatedHBP = 53;
  hltdc.Init.AccumulatedVBP = 11;
  hltdc.Init.AccumulatedActiveW = 533;
  hltdc.Init.AccumulatedActiveH = 283;
  hltdc.Init.TotalWidth = 565;
  hltdc.Init.TotalHeigh = 285;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    TouchGFX_ReportFatalError("TouchGFX: LTDC init failed\r\n");
  }

  layer.WindowX0 = 0;
  layer.WindowX1 = 480;
  layer.WindowY0 = 0;
  layer.WindowY1 = 272;
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  layer.Alpha = 255;
  layer.Alpha0 = 0;
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  layer.FBStartAdress = TOUCHGFX_FRAMEBUFFER_ADDRESS;
  layer.ImageWidth = 480;
  layer.ImageHeight = 272;
  layer.Backcolor.Blue = 0;
  layer.Backcolor.Green = 0;
  layer.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &layer, 0) != HAL_OK)
  {
    TouchGFX_ReportFatalError("TouchGFX: LTDC layer setup failed\r\n");
  }
}

void TouchGFX_LogError(const char *message)
{
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)message, (uint16_t)strlen(message), 100);
}

void TouchGFX_ReportFatalError(const char *message)
{
  TouchGFX_LogError(message);
  for (;;)
  {
    osDelay(1000);
  }
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *ltdc)
{
  if (ltdc->Instance == LTDC)
  {
    __HAL_RCC_LTDC_CLK_ENABLE();
  }
}
