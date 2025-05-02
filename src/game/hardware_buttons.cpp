#include "hardware_buttons.h"

#include <boost/format.hpp>

#include "draw.h"
#include "filesystem.h"


namespace
{
const int height = 30;
const int x_offsets[] = { 0, 68, 141, 214 };
const int widths[] = { 67, 72, 72, 67 };
};

HardwareButtons::HardwareButtons(int y, int player) :
    buttons(Filesystem::readImage(
                (boost::format("images/hardware_buttons.%1%.png") % player).str()
            )),
    _y(y)
{
}

HardwareButtons::~HardwareButtons()
{
}

void HardwareButtons::drawButtons(int highlighted_group) const
{
    drawButton(0, 0 == highlighted_group, *display::graphics.screen(), 8, _y);
    drawButton(1, 1 == highlighted_group, *display::graphics.screen(), 84, _y);
    drawButton(2, 2 == highlighted_group, *display::graphics.screen(), 165, _y);
    drawButton(3, 3 == highlighted_group, *display::graphics.screen(), 246, _y);
}

void HardwareButtons::drawButton(int group, bool highlighted, display::Surface &target, int image_x, int image_y) const
{
    int width = widths[group];
    int src_x = x_offsets[group];
    int src_y = highlighted ? 31 : 0;

    if (highlighted) {
        InBox(image_x - 1, image_y - 1, image_x + width + 1, image_y + height + 1);
    } else {
        OutBox(image_x - 1, image_y - 1, image_x + width + 1, image_y + height + 1);
    }

    target.draw(buttons, src_x, src_y, width, height, image_x, image_y);
}
