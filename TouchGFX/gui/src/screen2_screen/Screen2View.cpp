#include <gui/screen2_screen/Screen2View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen2View::Screen2View()
{
	volumeButton.forceState(audio_get_mute());
	volumeButton.invalidate();
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f + 0.5f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void Screen2View::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void Screen2View::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    invalidate();
}

void Screen2View::handleGestureEvent(const touchgfx::GestureEvent& evt)
{
    if (evt.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL)
    {
        int16_t v = evt.getVelocity();

        if (v < -5)
        {
        	application().gotoScreen3ScreenNoTransition();
        }
        else if (v > 5)
        {
        	application().gotoScreen1ScreenNoTransition();
        }
    }
    Screen2ViewBase::handleGestureEvent(evt);
}

void Screen2View::handleTickEvent()
{
    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
