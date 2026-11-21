#include <gui/screen0_screen/Screen0View.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Texts.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <gui/common/ButtonUtils.hpp>
#include <touchgfx/Unicode.hpp>
#include <cmath>

static const float ATTACK_MAX_S  = 2.0f;
static const float DECAY_MAX_S   = 2.0f;
static const float RELEASE_MAX_S = 3.0f;

static bool s_envSlidersShown = false;

static void setVis(touchgfx::Drawable& d, bool v)
{
    d.invalidate();
    d.setVisible(v);
    d.invalidate();
}

Screen0View::Screen0View()
{
	volumeButton.forceState(audio_get_mute());
	volumeButton.invalidate();

	lpfButton.forceState(audio_get_effect(AUDIO_FX_LPF));
	lpfButton.invalidate();

	hpfButton.forceState(audio_get_effect(AUDIO_FX_HPF));
	hpfButton.invalidate();
}

void Screen0View::setupScreen()
{
    Screen0ViewBase::setupScreen();

    volume.setValue((int)(audio_get_volume() * 100.0f));
    volume.invalidate();

    language.forceState(touchgfx::Texts::getLanguage() == PL);
    language.invalidate();

    midiShown = midi_is_connected();
    MIDIstatus.setVisible(midiShown);
    MIDIstatus.invalidate();

    osc2Button.forceState(audio_get_osc2_enable());
    osc2Button.invalidate();

    osc2SemitonesSlider.setValue((int)lroundf(audio_get_osc2_semitones()));
    osc2DetuneSlider.setValue((int)lroundf(audio_get_osc2_detune()));
    osc2LevelSlider.setValue((int)lroundf(audio_get_osc2_level() * 100.0f));
    harmonicsSlider.setValue((int)lroundf(audio_get_harmonics()));

    attackSlider.setValue((int)lroundf(audio_get_attack()   / ATTACK_MAX_S  * 100.0f));
    decaySlider.setValue((int)lroundf(audio_get_decay()     / DECAY_MAX_S   * 100.0f));
    sustainSlider.setValue((int)lroundf(audio_get_sustain() * 100.0f));
    releaseSlider.setValue((int)lroundf(audio_get_release() / RELEASE_MAX_S * 100.0f));

    envToggleButton.forceState(s_envSlidersShown);
    envToggleButton.invalidate();

    updateAdsrLabels();
    updateEnvGraph();
    updateSliderLabels();
    refreshOscUi();
}

void Screen0View::selectBasic()    { audio_set_synth_type(AUDIO_SYNTH_BASIC); refreshOscUi(); }
void Screen0View::selectAdditive() { audio_set_synth_type(AUDIO_SYNTH_ADD);   refreshOscUi(); }

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
    refreshOscUi();
}

void Screen0View::osc2SelectSine()     { audio_set_osc2_wave(AUDIO_WAVE_SINE);     refreshOscUi(); }
void Screen0View::osc2SelectSaw()      { audio_set_osc2_wave(AUDIO_WAVE_SAW);      refreshOscUi(); }
void Screen0View::osc2SelectSquare()   { audio_set_osc2_wave(AUDIO_WAVE_SQUARE);   refreshOscUi(); }
void Screen0View::osc2SelectTriangle() { audio_set_osc2_wave(AUDIO_WAVE_TRIANGLE); refreshOscUi(); }
void Screen0View::osc2SelectTrapez()   { audio_set_osc2_wave(AUDIO_WAVE_TRAPEZ);   refreshOscUi(); }

void Screen0View::osc2SemitonesChanged(int value) { audio_set_osc2_semitones((float)value);	updateSliderLabels(); }
void Screen0View::osc2DetuneChanged(int value)    { audio_set_osc2_detune((float)value);  	updateSliderLabels(); }
void Screen0View::osc2LevelChanged(int value)     { audio_set_osc2_level(value / 100.0f);   updateSliderLabels(); }
void Screen0View::harmonicsChanged(int value)     { audio_set_harmonics((uint8_t)value);   	updateSliderLabels(); }

void Screen0View::tearDownScreen()
{
	s_envSlidersShown = false;
    Screen0ViewBase::tearDownScreen();
}

void Screen0View::refreshOscUi()
{
    AudioSynthType t = audio_get_synth_type();
    AudioWave w  = audio_get_wave();
    AudioWave w2 = audio_get_osc2_wave();

    bool osc1WaveEn = (t == AUDIO_SYNTH_BASIC);
    bool osc2En     = (t != AUDIO_SYNTH_ADD);
    bool harmEn     = (t == AUDIO_SYNTH_ADD);
    bool osc2Active = osc2En && audio_get_osc2_enable();

    showButton(basicButton,    t == AUDIO_SYNTH_BASIC, true);
    showButton(additiveButton, t == AUDIO_SYNTH_ADD,   true);

    showButton(sineButton,     w == AUDIO_WAVE_SINE,     osc1WaveEn);
    showButton(sawButton,      w == AUDIO_WAVE_SAW,      osc1WaveEn);
    showButton(squareButton,   w == AUDIO_WAVE_SQUARE,   osc1WaveEn);
    showButton(triangleButton, w == AUDIO_WAVE_TRIANGLE, osc1WaveEn);
    showButton(trapezButton,   w == AUDIO_WAVE_TRAPEZ,   osc1WaveEn);

    showButton(osc2SineButton,     w2 == AUDIO_WAVE_SINE,     osc2Active);
    showButton(osc2SawButton,      w2 == AUDIO_WAVE_SAW,      osc2Active);
    showButton(osc2SquareButton,   w2 == AUDIO_WAVE_SQUARE,   osc2Active);
    showButton(osc2TriangleButton, w2 == AUDIO_WAVE_TRIANGLE, osc2Active);
    showButton(osc2TrapezButton,   w2 == AUDIO_WAVE_TRAPEZ,   osc2Active);

    bool showSub = osc2En && !s_envSlidersShown;
    bool showAdd = harmEn && !s_envSlidersShown;

    osc2Dim.invalidate();
    osc2Dim.setVisible(showSub && !audio_get_osc2_enable());
    osc2Dim.invalidate();

    osc2Button.setTouchable(osc2En);
    osc2SemitonesSlider.setTouchable(osc2Active);
    osc2DetuneSlider.setTouchable(osc2Active);
    osc2LevelSlider.setTouchable(osc2Active);

    setVis(subSliders, showSub);
    setVis(addSliders, showAdd);
    setVis(envSliders, s_envSlidersShown);

    osc2Button.invalidate();
}

void Screen0View::updateSliderLabels()
{
    Unicode::snprintf(osc2SemitonesValueBuffer, OSC2SEMITONESVALUE_SIZE, "%d", osc2SemitonesSlider.getValue());
    osc2SemitonesValue.invalidate();

    Unicode::snprintf(osc2DetuneValueBuffer, OSC2DETUNEVALUE_SIZE, "%d", osc2DetuneSlider.getValue());
    osc2DetuneValue.invalidate();

    Unicode::snprintf(osc2LevelValueBuffer, OSC2LEVELVALUE_SIZE, "%d", osc2LevelSlider.getValue());
    osc2LevelValue.invalidate();

    Unicode::snprintf(harmonicsValueBuffer, HARMONICSVALUE_SIZE, "%d", harmonicsSlider.getValue());
    harmonicsValue.invalidate();
}

void Screen0View::volumeChanged(int value) {
    audio_set_volume(value / 100.0f);
}

void Screen0View::muteToggled()
{
    audio_set_mute(volumeButton.getState());
}

void Screen0View::lpfToggled()
{
    audio_set_effect(AUDIO_FX_LPF, lpfButton.getPressed());
}

void Screen0View::hpfToggled()
{
    audio_set_effect(AUDIO_FX_HPF, hpfButton.getPressed());
}

void Screen0View::updateEnvGraph()
{
    float a = audio_get_attack();
    float d = audio_get_decay();
    float s = audio_get_sustain();
    float r = audio_get_release();

    float hold  = fmaxf(0.2f, 0.25f * (a + d + r));
    float total = a + d + hold + r;
    float k = 1000.0f / total;

    envGraph.clear();
    envGraph.addDataPoint(0.0f, 0.0f);
    envGraph.addDataPoint(a * k, 100.0f);
    envGraph.addDataPoint((a + d) * k, s * 100.0f);
    envGraph.addDataPoint((a + d + hold) * k, s * 100.0f);
    envGraph.addDataPoint(1000.0f, 0.0f);
    envGraph.invalidate();
}

void Screen0View::updateAdsrLabels()
{
    Unicode::snprintf(attackValueBuffer,  ATTACKVALUE_SIZE,  "%d", (int)lroundf(audio_get_attack()  * 1000.0f));
    attackValue.invalidate();
    Unicode::snprintf(decayValueBuffer,   DECAYVALUE_SIZE,   "%d", (int)lroundf(audio_get_decay()   * 1000.0f));
    decayValue.invalidate();
    Unicode::snprintf(sustainValueBuffer, SUSTAINVALUE_SIZE, "%d", (int)lroundf(audio_get_sustain() * 100.0f));
    sustainValue.invalidate();
    Unicode::snprintf(releaseValueBuffer, RELEASEVALUE_SIZE, "%d", (int)lroundf(audio_get_release() * 1000.0f));
    releaseValue.invalidate();
}

void Screen0View::envSlidersToggled()
{
    s_envSlidersShown = !s_envSlidersShown;
    envToggleButton.forceState(s_envSlidersShown);
    envToggleButton.invalidate();
    if (s_envSlidersShown) envDirty = true;
    refreshOscUi();
}

void Screen0View::toggleLanguage()
{
    touchgfx::Texts::setLanguage(touchgfx::Texts::getLanguage() == GB ? PL : GB);

    langText.resizeToCurrentText();
    infoText.resizeToCurrentText();
    envelopeTxt.resizeToCurrentText();
    level.resizeToCurrentText();
    detune.resizeToCurrentText();
    semitones.resizeToCurrentText();
    harmonics.resizeToCurrentText();
    osc2txt.resizeToCurrentText();
    osc1txt.resizeToCurrentText();
    invalidate();
}

void Screen0View::attackChanged(int v)  { audio_set_attack(v / 100.0f * ATTACK_MAX_S);   updateAdsrLabels(); envDirty = true; }
void Screen0View::decayChanged(int v)   { audio_set_decay(v / 100.0f * DECAY_MAX_S);     updateAdsrLabels(); envDirty = true; }
void Screen0View::sustainChanged(int v) { audio_set_sustain(v / 100.0f);                 updateAdsrLabels(); envDirty = true; }
void Screen0View::releaseChanged(int v) { audio_set_release(v / 100.0f * RELEASE_MAX_S); updateAdsrLabels(); envDirty = true; }

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

void Screen0View::envLeft()
{
    if (s_envSlidersShown)
    {
        s_envSlidersShown = false;
        envToggleButton.forceState(false);
        envToggleButton.invalidate();
        refreshOscUi();
    }
}

void Screen0View::handleTickEvent()
{
	if (envDirty && s_envSlidersShown && ++envTick >= 4)
    {
        envTick = 0;
        envDirty = false;
        updateEnvGraph();
    }

    bool c = midi_is_connected();
    if (c != midiShown)
    {
        midiShown = c;
        MIDIstatus.setVisible(c);
        MIDIstatus.invalidate();
    }
}
