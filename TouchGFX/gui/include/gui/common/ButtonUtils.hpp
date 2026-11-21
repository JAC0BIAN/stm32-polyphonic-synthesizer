/*
 * ButtonUtils.hpp
 *
 *  Created on: Nov 16, 2026
 *      Author: JAC0BIAN
 */

#ifndef GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_
#define GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_

#include <touchgfx/Color.hpp>

template <typename ButtonT>
inline void showButton(ButtonT& b, bool selected, bool enabled)
{
    touchgfx::colortype off   = touchgfx::Color::getColorFromRGB(60, 60, 60);
    touchgfx::colortype onC   = touchgfx::Color::getColorFromRGB(0, 160, 80);
    touchgfx::colortype dis   = touchgfx::Color::getColorFromRGB(35, 35, 35);
    touchgfx::colortype disOn = touchgfx::Color::getColorFromRGB(0, 80, 40);
    touchgfx::colortype edge  = touchgfx::Color::getColorFromRGB(200, 200, 200);
    touchgfx::colortype edgeD = touchgfx::Color::getColorFromRGB(90, 90, 90);

    touchgfx::colortype fill = enabled ? (selected ? onC : off) : (selected ? disOn : dis);
    touchgfx::colortype bord = enabled ? edge : edgeD;

    b.setBoxWithBorderColors(fill, fill, bord, bord);
    b.setTouchable(enabled);
    b.invalidate();
}

#endif /* GUI_INCLUDE_GUI_COMMON_BUTTONUTILS_HPP_ */
