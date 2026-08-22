#ifndef VOICEVIEW_HPP
#define VOICEVIEW_HPP

#include <gui_generated/voice_screen/voiceViewBase.hpp>
#include <gui/voice_screen/voicePresenter.hpp>

class voiceView : public voiceViewBase
{
public:
    voiceView();
    virtual ~voiceView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // VOICEVIEW_HPP
