#include <gui/screen0_screen/Screen0View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <gui/common/ButtonUtils.hpp>

Screen0View::Screen0View()
{
	volumeButton.forceState(audio_get_mute());
	volumeButton.invalidate();
}

void Screen0View::setupScreen()
{
    Screen0ViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f + 0.5f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();

    osc2Button.forceState(audio_get_osc2_enable());
    osc2Button.invalidate();

    osc2SemitonesSlider.setValue((int)audio_get_osc2_semitones());
    osc2DetuneSlider.setValue((int)audio_get_osc2_detune());
    osc2LevelSlider.setValue((int)(audio_get_osc2_level() * 100.0f + 0.5f));
    harmonicsSlider.setValue(audio_get_harmonics());

    refreshOscUi();
}

void Screen0View::selectBasic()    { audio_set_synth_type(AUDIO_SYNTH_BASIC); refreshOscUi(); }
void Screen0View::selectAdditive() { audio_set_synth_type(AUDIO_SYNTH_ADD);   refreshOscUi(); }
void Screen0View::selectSub()      { audio_set_synth_type(AUDIO_SYNTH_SUB);   refreshOscUi(); }

void Screen0View::selectSine()     { audio_set_wave(AUDIO_WAVE_SINE);     refreshOscUi(); }
void Screen0View::selectSaw()      { audio_set_wave(AUDIO_WAVE_SAW);      refreshOscUi(); }
void Screen0View::selectSquare()   { audio_set_wave(AUDIO_WAVE_SQUARE);   refreshOscUi(); }
void Screen0View::selectTriangle() { audio_set_wave(AUDIO_WAVE_TRIANGLE); refreshOscUi(); }
void Screen0View::selectTrapez()   { audio_set_wave(AUDIO_WAVE_TRAPEZ);   refreshOscUi(); }

void Screen0View::osc2Toggled()
{
    audio_set_osc2_enable(!audio_get_osc2_enable());
    osc2Button.forceState(audio_get_osc2_enable());
    osc2Button.invalidate();
}

void Screen0View::osc2SelectSine()     { audio_set_osc2_wave(AUDIO_WAVE_SINE);     refreshOscUi(); }
void Screen0View::osc2SelectSaw()      { audio_set_osc2_wave(AUDIO_WAVE_SAW);      refreshOscUi(); }
void Screen0View::osc2SelectSquare()   { audio_set_osc2_wave(AUDIO_WAVE_SQUARE);   refreshOscUi(); }
void Screen0View::osc2SelectTriangle() { audio_set_osc2_wave(AUDIO_WAVE_TRIANGLE); refreshOscUi(); }
void Screen0View::osc2SelectTrapez()   { audio_set_osc2_wave(AUDIO_WAVE_TRAPEZ);   refreshOscUi(); }

void Screen0View::osc2SemitonesChanged(int value) { audio_set_osc2_semitones((float)value); }
void Screen0View::osc2DetuneChanged(int value)    { audio_set_osc2_detune((float)value); }
void Screen0View::osc2LevelChanged(int value)     { audio_set_osc2_level(value / 100.0f); }
void Screen0View::harmonicsChanged(int value)     { audio_set_harmonics((uint8_t)value); }

void Screen0View::tearDownScreen()
{
    Screen0ViewBase::tearDownScreen();
}

void Screen0View::refreshOscUi()
{
    AudioWave w  = audio_get_wave();
    AudioWave w2 = audio_get_osc2_wave();
    AudioSynthType t = audio_get_synth_type();

    showState(basicButton,    t == AUDIO_SYNTH_BASIC);
    showState(additiveButton, t == AUDIO_SYNTH_ADD);
    showState(subButton,      t == AUDIO_SYNTH_SUB);

    showState(sineButton,     w == AUDIO_WAVE_SINE);
    showState(sawButton,      w == AUDIO_WAVE_SAW);
    showState(squareButton,   w == AUDIO_WAVE_SQUARE);
    showState(triangleButton, w == AUDIO_WAVE_TRIANGLE);
    showState(trapezButton,   w == AUDIO_WAVE_TRAPEZ);

    showState(osc2SineButton,     w2 == AUDIO_WAVE_SINE);
    showState(osc2SawButton,      w2 == AUDIO_WAVE_SAW);
    showState(osc2SquareButton,   w2 == AUDIO_WAVE_SQUARE);
    showState(osc2TriangleButton, w2 == AUDIO_WAVE_TRIANGLE);
    showState(osc2TrapezButton,   w2 == AUDIO_WAVE_TRAPEZ);
}

void Screen0View::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void Screen0View::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void Screen0View::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    invalidate();
}

void Screen0View::handleGestureEvent(const touchgfx::GestureEvent& evt)
{
    if (evt.getType() == touchgfx::GestureEvent::SWIPE_HORIZONTAL)
    {
        int16_t v = evt.getVelocity();

        if (v < -5)
        {
        	application().gotoScreen1ScreenNoTransition();
        }
        else if (v > 5)
        {
        	application().gotohomeScreenScreenNoTransition();
        }
    }
    Screen0ViewBase::handleGestureEvent(evt);
}

void Screen0View::handleTickEvent()
{
    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
