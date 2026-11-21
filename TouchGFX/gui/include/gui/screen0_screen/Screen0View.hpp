#ifndef SCREEN0VIEW_HPP
#define SCREEN0VIEW_HPP

#include <gui_generated/Screen0_screen/Screen0ViewBase.hpp>
#include <gui/Screen0_screen/Screen0Presenter.hpp>
#include "../../../../../Core/Src/audio_params.h"
#include "../../../../../Core/Src/midi_status.h"

class Screen0View : public Screen0ViewBase
{
public:
    Screen0View();
    virtual ~Screen0View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void volumeChanged(int value) override;
    virtual void muteToggled() override;
    virtual void handleGestureEvent(const touchgfx::GestureEvent& evt) override;

    virtual void toggleLanguage();
    virtual void handleTickEvent() override;

    virtual void selectBasic() override;
    virtual void selectAdditive() override;

    virtual void selectSine() override;
    virtual void selectSaw() override;
    virtual void selectSquare() override;
    virtual void selectTriangle() override;
    virtual void selectTrapez() override;

    virtual void osc2Toggled() override;
    virtual void osc2SelectSine() override;
    virtual void osc2SelectSaw() override;
    virtual void osc2SelectSquare() override;
    virtual void osc2SelectTriangle() override;
    virtual void osc2SelectTrapez() override;

    virtual void osc2SemitonesChanged(int value) override;
    virtual void osc2DetuneChanged(int value) override;
    virtual void osc2LevelChanged(int value) override;
    virtual void harmonicsChanged(int value) override;

    virtual void lpfToggled() override;
    virtual void hpfToggled() override;

    virtual void attackChanged(int value) override;
    virtual void decayChanged(int value) override;
    virtual void sustainChanged(int value) override;
    virtual void releaseChanged(int value) override;

    virtual void envSlidersToggled() override;
    virtual void envLeft() override;
protected:
    bool midiShown = false;

    bool    envDirty = false;
    uint8_t envTick  = 0;

    void refreshOscUi();
    void updateSliderLabels();
    void updateEnvGraph();
    void updateAdsrLabels();
};

#endif // SCREEN0VIEW_HPP
