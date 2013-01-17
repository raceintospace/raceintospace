#include "palettized_surface.h"

#include <assert.h>

namespace display
{

PalettizedSurface::PalettizedSurface(unsigned int width, unsigned int height, const PaletteInterface &palette)
    : Surface(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0)),
      _palette(_screen)
{
    // copy the provided palette to the SDL surface's palette
    _palette.copy_from(palette);

    // scan the palette for transparency
    int transparent_color = -1;

    for (int i = 0; i < 256; i++) {
        if (palette.get(i).a != 255) {
            transparent_color = i;
            break;
        }
    }

    if (transparent_color == -1) {
        // we're not actually transparent
        // disable color keying
        SDL_SetColorKey(_screen, 0, 0);
    } else {
        // enable color keying on this color
        SDL_SetColorKey(_screen, SDL_SRCCOLORKEY, transparent_color);
    }
}

PalettizedSurface::~PalettizedSurface()
{
}

void PalettizedSurface::copyRow(uint32_t row, const void *pixelData)
{
    assert(!SDL_MUSTLOCK(_screen)); // make sure not locking is okay

    char *pixels = (char *)_screen->pixels;
    memcpy(pixels + (_screen->pitch * row), pixelData, _screen->w);
}

} // namespace display
