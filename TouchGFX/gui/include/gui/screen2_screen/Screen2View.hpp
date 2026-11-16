#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/Screen2_screen/Screen2ViewBase.hpp>
#include <gui/Screen2_screen/Screen2Presenter.hpp>
#include "../../../../../Core/Src/audio_params.h"
#include "../../../../../Core/Src/midi_status.h"

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void volumeChanged(int value) override;
    virtual void muteToggled() override;
    virtual void handleGestureEvent(const touchgfx::GestureEvent& evt) override;

    virtual void toggleLanguage();
    virtual void handleTickEvent() override;
protected:
    bool midiShown = false;
};

#endif // SCREEN2VIEW_HPP
