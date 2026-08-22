#ifndef PRESETSVIEW_HPP
#define PRESETSVIEW_HPP

#include <gui_generated/presets_screen/presetsViewBase.hpp>
#include <gui/presets_screen/presetsPresenter.hpp>

class presetsView : public presetsViewBase
{
public:
    presetsView();
    virtual ~presetsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // PRESETSVIEW_HPP
