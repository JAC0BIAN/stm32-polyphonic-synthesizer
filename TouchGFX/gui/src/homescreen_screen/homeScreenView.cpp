#include <gui/homescreen_screen/homeScreenView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>

homeScreenView::homeScreenView()
{
	volumeButton.forceState(audio_get_mute());
	volumeButton.invalidate();
}

void homeScreenView::setupScreen()
{
    homeScreenViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f + 0.5f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();
}

void homeScreenView::tearDownScreen()
{
    homeScreenViewBase::tearDownScreen();
}

void homeScreenView::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void homeScreenView::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void homeScreenView::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    welcomeText.resizeToCurrentText();
    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    invalidate();
}

void homeScreenView::handleGestureEvent(const touchgfx::GestureEvent& evt)
{
    if (evt.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL)
    {
        int16_t v = evt.getVelocity();

        if (v < -5)
        {
        	application().gotoScreen0ScreenNoTransition();
        }
    }
    homeScreenViewBase::handleGestureEvent(evt);
}

void homeScreenView::handleTickEvent()
{
    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
