#include <stdint.h>

#include "TouchGFXCache.hpp"
#include "touchgfx_board.h"

extern "C" uint8_t __touchgfx_buffers_start__;
extern "C" uint8_t __touchgfx_buffers_end__;

void TouchGFX_CleanInvalidateBuffers(void)
{
    const uintptr_t start = reinterpret_cast<uintptr_t>(&__touchgfx_buffers_start__);
    const uintptr_t end = reinterpret_cast<uintptr_t>(&__touchgfx_buffers_end__);
    const uintptr_t size = end - start;

    if (end <= start || (start & 31U) != 0U || (size & 31U) != 0U)
    {
        TouchGFX_ReportFatalError("TouchGFX: invalid framebuffer region\r\n");
    }

    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
    {
        SCB_CleanInvalidateDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(size));
    }
}
