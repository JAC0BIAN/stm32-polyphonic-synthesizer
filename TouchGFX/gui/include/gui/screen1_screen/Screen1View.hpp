#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include "../../../../../Core/Src/audio_params.h"
#include "../../../../../Core/Src/midi_status.h"

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void reverbToggled() override;
    virtual void delayToggled() override;
    virtual void chorusToggled() override;
    virtual void flangerToggled() override;
    virtual void phaserToggled() override;
    virtual void overdriveToggled() override;
    virtual void distortionToggled() override;
    virtual void compressorToggled() override;

    virtual void volumeChanged(int value) override;
    virtual void muteToggled() override;
    virtual void handleGestureEvent(const touchgfx::GestureEvent& evt) override;

    virtual void toggleLanguage();
    virtual void handleTickEvent() override;
protected:
    bool midiShown = false;
};

#endif // SCREEN1VIEW_HPP
