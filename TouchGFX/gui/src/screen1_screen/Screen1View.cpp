#include <gui/screen1_screen/Screen1View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen1View::Screen1View()
{
	reverbButton.forceState(audio_get_effect(AUDIO_FX_REVERB));
    reverbButton.invalidate();

    delayButton.forceState(audio_get_effect(AUDIO_FX_DELAY));
    delayButton.invalidate();

    chorusButton.forceState(audio_get_effect(AUDIO_FX_CHORUS));
    chorusButton.invalidate();

    flangerButton.forceState(audio_get_effect(AUDIO_FX_FLANGER));
    flangerButton.invalidate();

    phaserButton.forceState(audio_get_effect(AUDIO_FX_PHASER));
    phaserButton.invalidate();

    overdriveButton.forceState(audio_get_effect(AUDIO_FX_OVERDRIVE));
    overdriveButton.invalidate();

    distortionButton.forceState(audio_get_effect(AUDIO_FX_DISTORTION));
    distortionButton.invalidate();

    compressorButton.forceState(audio_get_effect(AUDIO_FX_COMPRESSOR));
    compressorButton.invalidate();

    volumeButton.forceState(audio_get_mute());
    volumeButton.invalidate();
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f + 0.5f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::reverbToggled()
{
    audio_set_effect(AUDIO_FX_REVERB, reverbButton.getState());
}

void Screen1View::delayToggled()
{
    audio_set_effect(AUDIO_FX_DELAY, delayButton.getState());
}

void Screen1View::chorusToggled()
{
    audio_set_effect(AUDIO_FX_CHORUS, chorusButton.getState());
}

void Screen1View::flangerToggled()
{
    audio_set_effect(AUDIO_FX_FLANGER, flangerButton.getState());
}

void Screen1View::phaserToggled()
{
    audio_set_effect(AUDIO_FX_PHASER, phaserButton.getState());
}

void Screen1View::overdriveToggled()
{
    audio_set_effect(AUDIO_FX_OVERDRIVE, overdriveButton.getState());
}

void Screen1View::distortionToggled()
{
    audio_set_effect(AUDIO_FX_DISTORTION, distortionButton.getState());
}

void Screen1View::compressorToggled()
{
    audio_set_effect(AUDIO_FX_COMPRESSOR, compressorButton.getState());
}

void Screen1View::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void Screen1View::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void Screen1View::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    reverbText.resizeToCurrentText();
    delayText.resizeToCurrentText();
    chorusText.resizeToCurrentText();
    flangerText.resizeToCurrentText();
    phaserText.resizeToCurrentText();
    overdriveText.resizeToCurrentText();
    distortionText.resizeToCurrentText();
    compressorText.resizeToCurrentText();
    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    invalidate();
}

void Screen1View::handleGestureEvent(const touchgfx::GestureEvent& evt)
{
    if (evt.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL)
    {
        int16_t v = evt.getVelocity();

        if (v < -5)
        {
        	application().gotoScreen2ScreenNoTransition();
        }
        else if (v > 5)
        {
        	application().gotoScreen0ScreenNoTransition();
        }
    }
    Screen1ViewBase::handleGestureEvent(evt);
}

void Screen1View::handleTickEvent()
{
    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
