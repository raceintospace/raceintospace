#include "surface.h"

#include <SDL.h>
#include <algorithm>
#include <assert.h>

namespace display {

Surface::Surface( unsigned int width, unsigned int height ):
	_width( width ),
	_height( height ),
	_screen( SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0 ) ),
	_dirty( false )
{
}

Surface::~Surface()
{
}

SDL_Surface * Surface::surface() const 
{
	return _screen;
}

char * Surface::pixels() const 
{
	return (char *)_screen->pixels;
}

void Surface::clear(int colour)
{
	_dirty = true;
    SDL_FillRect(_screen, NULL, colour);
}

void Surface::fillRect(int x1, int y1, int x2, int y2, char color)
{
	_dirty = true;
    SDL_Rect r;
    int left = std::min(x1, x2);
    int right = std::max(x1, x2);
    int top = std::min(y1, y2);
    int bottom = std::max(y1, y2);

    r.x = left;
    r.y = top;

    // Original used inclusive coordinates.  The +1 was in the original.
    r.w = (right - left) + 1;
    r.h = (bottom - top) + 1;
    fillRect(r, color);
}

void Surface::fillRect(const SDL_Rect &area, char color)
{
	_dirty = true;
    SDL_FillRect(_screen, const_cast<SDL_Rect *>(&area), color);
}

void Surface::setPixel(int x, int y, char color)
{
	_dirty = true;
    *((char *)(_screen->pixels) + (y * _screen->pitch) + x) = color;
}

char Surface::getPixel(int x, int y)
{
	_dirty = true;
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);

    return ((char *)_screen->pixels)[(y * _screen->pitch) + x];
}

void Surface::outlineRect(int x1, int y1, int x2, int y2, char color)
{
	_dirty = true;
	line( x1, y1, x2, y1, color );
	line( x2, y1, x2, y2, color );
	line( x2, y2, x1, y2, color );
	line( x1, y2, x1, y1, color );
}

#define swap(a,b) (t = a, a = b, b = t)
void Surface::line(int x1, int y1, int x2, int y2, char color)
{
	_dirty = true;
    int deltax, deltay;
    int error;
    int ystep;
    int x, y;
    int steep;
    int t;

    steep = abs(y2 - y1) > abs(x2 - x1);

    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    deltax = x2 - x1;
    deltay = abs(y2 - y1);
    error = 0;

    y = y1;

    if (y1 < y2) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = x1; x <= x2; x++) {
        if (steep) {
            setPixel(y, x, color);
        } else {
            setPixel(x, y, color);
        }

        error = error + deltay;

        if (2 * error >= deltax) {
            y = y + ystep;
            error = error - deltax;
        }
    }
}

void Surface::copyFrom(Surface * surface, int x1, int y1, int x2, int y2)
{
	_dirty = true;

    int w, h, from_idx, to_idx, row;

    assert(surface);
    assert(0 <= x1 && x1 < (int)surface->width());
    assert(0 <= x2 && x2 < (int)surface->width());
    assert(0 <= y1 && y1 < (int)surface->height());
    assert(0 <= y2 && y2 < (int)surface->height());
    assert(x1 <= x2);
    assert(y1 <= y2);

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;

    assert(w <= _width);
    assert(h <= _height);

    for (row = 0; row < h; row++) {
        from_idx = (y1 + row) * surface->width() + x1;
        to_idx = row * _width;
        memcpy((void *)((char *)_screen->pixels)[to_idx], (void *)(surface->pixels()[from_idx]), w);
    }
}

void Surface::copyTo(Surface * surface, int mode, int x, int y, Surface::Operation operation)
{
	surface->_dirty = true;

    int row, col, from_idx, to_idx;
    int clip_x, clip_y;
    SDL_Rect r;

    assert(surface);
    assert(operation == Surface::Set || operation == Surface::Xor);
    assert(0 <= x && x < surface->width());
    assert(0 <= y && y < surface->height());

    clip_y = std::min(_height + y, surface->height()) - y;
    clip_x = std::min(_width + x, surface->width()) - x;

    switch (mode) {
	case Surface::Set:
        for (row = 0; row < clip_y; row++) {
            from_idx = row * _width;
            to_idx = (y + row) * surface->width() + x;
            memcpy((void *)(surface->pixels()[to_idx]), (void *)(((char *)_screen->pixels)[from_idx]), clip_x);
        }

        break;

	case Surface::Xor:
        for (row = 0; row < clip_y; row++) {
            from_idx = row * _width;
            to_idx = (y + row) * surface->width() + x;

            for (col = 0; col < clip_x; col++) {				
                surface->pixels()[to_idx + col] ^= ((char *)_screen->pixels)[from_idx + col];
            }
        }

        break;
    }
}

void Surface::scaleTo(Surface * surface)
{
    int dest_row, dest_col, dest_idx;
    int src_row, src_col, src_idx;

    for (dest_row = 0; dest_row < surface->height(); dest_row++) {
        src_row = (dest_row * _height) / surface->height();

        for (dest_col = 0; dest_col < surface->width(); dest_col++) {
            src_col  = (dest_col * _width) / surface->width();

            src_idx  = src_row  * _width  + src_col;
            dest_idx = dest_row * surface->width() + dest_col;

            surface->pixels()[dest_idx] = pixels()[src_idx];
        }
    }
}


} // namespace display