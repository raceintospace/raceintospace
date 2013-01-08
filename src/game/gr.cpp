#include "display/graphics.h"
#include "display/surface.h"

#include "gr.h"
#include "Buzz_inc.h"
#include <assert.h>
#include "sdlhelper.h"

#include <boost/swap.hpp>

using boost::swap;

static int gr_cur_x;
static int gr_cur_y;

int
grGetMouseButtons(void)
{
    int val;

    val = av_mouse_pressed_latched || av_mouse_pressed_cur;
    av_mouse_pressed_latched = 0;
    return (val);
}

int
grGetMousePressedPos(int *xp, int *yp)
{
    *xp = av_mouse_pressed_x / 2;
    *yp = av_mouse_pressed_y / 2;
    return (0);
}

int
grGetMouseCurPos(int *xp, int *yp)
{
    *xp = av_mouse_cur_x / 2;
    *yp = av_mouse_cur_y / 2;
    return (0);
}

void
gr_sync(void)
{
    av_sync();
}

void
grMoveTo(int x, int y)
{
    gr_cur_x = x;
    gr_cur_y = y;
}

void
grLineTo(int x_arg, int y_arg)
{
    int deltax, deltay;
    int error;
    int ystep;
    int x, y;
    int x0, y0, x1, y1;
    int steep;

    x0 = gr_cur_x;
    y0 = gr_cur_y;

    x1 = x_arg;
    y1 = y_arg;

    steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    deltax = x1 - x0;
    deltay = abs(y1 - y0);
    error = 0;

    y = y0;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = x0; x <= x1; x++) {
        if (steep) {
            display::graphics.legacyScreen()->setPixel(y, x, display::graphics.foregroundColor());
        } else {
            display::graphics.legacyScreen()->setPixel(x, y, display::graphics.foregroundColor());
        }

        error = error + deltay;

        if (2 * error >= deltax) {
            y = y + ystep;
            error = error - deltax;
        }
    }

    gr_cur_x = x_arg;
    gr_cur_y = y_arg;
}

void
grLineRel(int dx, int dy)
{
    grLineTo(gr_cur_x + dx, gr_cur_y + dy);
}

void
grMoveRel(int dx, int dy)
{
    grMoveTo(gr_cur_x + dx, gr_cur_y + dy);
}

