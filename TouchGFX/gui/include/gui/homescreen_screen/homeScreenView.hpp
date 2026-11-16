#ifndef HOMESCREENVIEW_HPP
#define HOMESCREENVIEW_HPP

#include <gui_generated/homescreen_screen/homeScreenViewBase.hpp>
#include <gui/homescreen_screen/homeScreenPresenter.hpp>
#include "../../../../../Core/Src/audio_params.h"
#include "../../../../../Core/Src/midi_status.h"

class homeScreenView : public homeScreenViewBase
{
public:
    homeScreenView();
    virtual ~homeScreenView() {}
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

#endif // HOMESCREENVIEW_HPP
