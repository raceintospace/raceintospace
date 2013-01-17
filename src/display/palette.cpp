#include <memory.h>
#include <assert.h>

#include "graphics.h"
#include "palette.h"

namespace display
{

// virtual destructor for the pure-virtual class
PaletteInterface::~PaletteInterface()
{
}


Palette::Palette()
{
    memset(colors, 0, sizeof(colors));
}

Palette::Palette(const Palette &copy)
{
    memcpy(colors, copy.colors, sizeof(colors));
}

Palette::Palette(const PaletteInterface &copy)
{
    for (int i = 0; i < 256; i++) {
        set(i, copy.get(i));
    }
}

Palette::~Palette()
{
}

void Palette::set(uint8_t index, const Color &color)
{
    colors[index] = color;
}

const Color Palette::get(uint8_t index) const
{
    return colors[index];
}


SDLPaletteWrapper::SDLPaletteWrapper(SDL_Surface *sdl_surface)
    : _sdl_surface(sdl_surface)
{
    assert(sdl_surface != NULL);
    assert(sdl_surface->format != NULL);
    assert(sdl_surface->format->palette != NULL);
}

SDLPaletteWrapper::~SDLPaletteWrapper()
{
}

void SDLPaletteWrapper::set(uint8_t index, const Color &color)
{
    assert(index < _sdl_surface->format->palette->ncolors);

    SDL_Color sdl_color;
    sdl_color.r = color.r;
    sdl_color.g = color.g;
    sdl_color.b = color.b;

    SDL_SetPalette(_sdl_surface, SDL_LOGPAL, &sdl_color, index, 1);
}

const Color SDLPaletteWrapper::get(uint8_t index) const
{
    assert(index < _sdl_surface->format->palette->ncolors);

    return Color(
               _sdl_surface->format->palette->colors[index].r,
               _sdl_surface->format->palette->colors[index].g,
               _sdl_surface->format->palette->colors[index].b
           );
}


AutoPal::AutoPal(PaletteInterface &p)
    : _pal(p)
{
    for (int i = 0; i < 256; i++) {
        const Color &color = _pal.get(i);
        pal[i * 3 + 0] = color.r >> 2;
        pal[i * 3 + 1] = color.g >> 2;
        pal[i * 3 + 2] = color.b >> 2;
    }
}

AutoPal::AutoPal(LegacySurface *legacySurface)
    : _pal(legacySurface->palette())
{
    for (int i = 0; i < 256; i++) {
        const Color &color = _pal.get(i);
        pal[i * 3 + 0] = color.r >> 2;
        pal[i * 3 + 1] = color.g >> 2;
        pal[i * 3 + 2] = color.b >> 2;
    }
}

AutoPal::~AutoPal()
{
    for (int i = 0; i < 256; i++) {
        uint8_t
        r = pal[i * 3 + 0],
        g = pal[i * 3 + 1],
        b = pal[i * 3 + 2];
        _pal.set(i, Color(
                     (r << 2) | (r >> 6),
                     (g << 2) | (g >> 6),
                     (b << 2) | (b >> 6)
                 ));
    }
}



}