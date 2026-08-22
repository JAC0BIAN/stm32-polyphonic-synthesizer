#ifndef VOICEPRESENTER_HPP
#define VOICEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class voiceView;

class voicePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    voicePresenter(voiceView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~voicePresenter() {}

private:
    voicePresenter();

    voiceView& view;
};

#endif // VOICEPRESENTER_HPP
