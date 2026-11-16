#ifndef SCREEN3VIEW_HPP
#define SCREEN3VIEW_HPP

#include <gui_generated/Screen3_screen/Screen3ViewBase.hpp>
#include <gui/Screen3_screen/Screen3Presenter.hpp>
#include "../../../../../Core/Src/audio_params.h"
#include "../../../../../Core/Src/midi_status.h"

class Screen3View : public Screen3ViewBase
{
public:
    Screen3View();
    virtual ~Screen3View() {}
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

#endif // SCREEN3VIEW_HPP
