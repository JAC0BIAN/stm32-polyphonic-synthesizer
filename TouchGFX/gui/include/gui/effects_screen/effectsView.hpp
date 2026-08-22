#ifndef EFFECTSVIEW_HPP
#define EFFECTSVIEW_HPP

#include <gui_generated/effects_screen/effectsViewBase.hpp>
#include <gui/effects_screen/effectsPresenter.hpp>

class effectsView : public effectsViewBase
{
public:
    effectsView();
    virtual ~effectsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // EFFECTSVIEW_HPP
