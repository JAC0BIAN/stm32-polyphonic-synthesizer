/*
 * ButtonUtils.hpp
 *
 *  Created on: Nov 16, 2026
 *      Author: FUJITSU
 */

#ifndef GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_
#define GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_

#include <touchgfx/Color.hpp>

template <typename ButtonT>
inline void showState(ButtonT& b, bool on)
{
    touchgfx::colortype off  = touchgfx::Color::getColorFromRGB(60, 60, 60);
    touchgfx::colortype onC  = touchgfx::Color::getColorFromRGB(0, 160, 80);
    touchgfx::colortype edge = touchgfx::Color::getColorFromRGB(200, 200, 200);

    b.setBoxWithBorderColors(on ? onC : off, on ? onC : off, edge, edge);
    b.invalidate();
}

#endif /* GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_ */
