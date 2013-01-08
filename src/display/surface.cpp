
#include "surface.h"

#include <SDL.h>
#include <algorithm>
#include <assert.h>

namespace display
{

Surface::Surface(SDL_Surface *surface) :
    _screen(surface),
    _dirty(false)
{
}

Surface::~Surface()
{
    SDL_FreeSurface(_screen);
    _screen = NULL;
}

SDL_Surface *Surface::surface() const
{
    return _screen;
}

void Surface::draw(Surface *surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y)
{
    _dirty = true;

    SDL_Rect src;
    SDL_Rect dst;

    src.x = srcX;
    src.y = srcY;
    src.w = srcW;
    src.h = srcH;

    dst.x = x;
    dst.y = y;
    dst.w = src.w;
    dst.h = src.h;
    SDL_BlitSurface(surface->_screen, &src, _screen, &dst);
}


} // namespace display
