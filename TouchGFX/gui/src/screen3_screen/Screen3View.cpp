#include <gui/screen3_screen/Screen3View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen3View::Screen3View()
{
	volumeButton.forceState(audio_get_mute());
	volumeButton.invalidate();
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f + 0.5f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}

void Screen3View::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void Screen3View::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void Screen3View::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    invalidate();
}

void Screen3View::handleGestureEvent(const touchgfx::GestureEvent& evt)
{
    if (evt.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL)
    {
        int16_t v = evt.getVelocity();

        if (v > 5)
        {
        	application().gotoScreen2ScreenNoTransition();
        }
    }
    Screen3ViewBase::handleGestureEvent(evt);
}

void Screen3View::handleTickEvent()
{
    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
