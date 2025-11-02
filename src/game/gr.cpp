// utilities to draw with digital pen
// it can "recenter its view" and draw straight lines
// also can detect mouse position and button presses, which I assume was used by game's graphics designer to make game's font

#include "gr.h"

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "sdlhelper.h"

using std::swap;

static int gr_cur_x;
static int gr_cur_y;

int grGetMouseButtons()
{
    int val = av_mouse_pressed_latched || av_mouse_pressed_cur;
    av_mouse_pressed_latched = 0;
    return val;
}

int grGetMousePressedPos(int *xp, int *yp)
{
    *xp = av_mouse_pressed_x / display::graphics.SCALE;
    *yp = av_mouse_pressed_y / display::graphics.SCALE;
    return 0;
}

int grGetMouseCurPos(int *xp, int *yp)
{
    *xp = av_mouse_cur_x / display::graphics.SCALE;
    *yp = av_mouse_cur_y / display::graphics.SCALE;
    return 0;
}

void gr_sync(void)
{
    av_sync();
}

// Moves pen onto new (absolute) location without drawing
void grMoveTo(int x, int y)
{
    gr_cur_x = x;
    gr_cur_y = y;
}

// Draws pixelated line from pen's position to input (absolute) location
// via https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm ?
void grLineTo(int x_arg, int y_arg)
{
    int x0 = gr_cur_x;
    int y0 = gr_cur_y;

    int x1 = x_arg;
    int y1 = y_arg;

    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = 0;

    int ystep;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (int x = x0, y = y0; x <= x1; x++) {
        if (steep) {
            display::graphics.legacyScreen()->setPixel(y, x, display::graphics.foregroundColor());
        } else {
            display::graphics.legacyScreen()->setPixel(x, y, display::graphics.foregroundColor());
        }

        error = error + deltay;

        if (display::graphics.SCALE * error >= deltax) {
            y = y + ystep;
            error = error - deltax;
        }
    }

    gr_cur_x = x_arg;
    gr_cur_y = y_arg;
}

// Draws line to (relative) location
void grLineRel(int dx, int dy)
{
    grLineTo(gr_cur_x + dx, gr_cur_y + dy);
}

// Moves pen to new (relative) location without drawing
void grMoveRel(int dx, int dy)
{
    grMoveTo(gr_cur_x + dx, gr_cur_y + dy);
}

