#ifndef HARDWARE_BUTTONS_H
#define HARDWARE_BUTTONS_H

#include <boost/shared_ptr.hpp>
#include "display/surface.h"

class HardwareButtons
{
public:
    HardwareButtons(int y, int player);
    ~HardwareButtons();

    void drawButtons(int highlighted_button = -1) const;

protected:
    void drawButton(int group, bool highlighted, display::Surface &target, int x, int y) const;

    boost::shared_ptr<display::Surface> buttons;
    int _y;
};

#endif // HARDWARE_BUTTONS_H
