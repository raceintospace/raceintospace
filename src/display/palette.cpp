#include <memory.h>
#include <assert.h>

#include "graphics.h"
#include "palette.h"

namespace display
{

// the global palette, defined in the display library
LegacyPalette legacy_palette;

// virtual destructor for the pure-virtual class
PaletteInterface::~PaletteInterface()
{
}


Palette::Palette()
{
}

Palette::Palette(const Palette &copy)
{
    memcpy(colors, copy.colors, sizeof(colors));
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


SDLPalette::SDLPalette()
{
    memset(sdl_colors, 0, sizeof(sdl_colors));
}

SDLPalette::~SDLPalette()
{
}

void SDLPalette::set(uint8_t index, const Color &color)
{
    sdl_colors[index].r = color.r;
    sdl_colors[index].g = color.g;
    sdl_colors[index].b = color.b;
}

const Color SDLPalette::get(uint8_t index) const
{
    // copy the top two bits to the lowest two, so 0 = 0 and 63 = 255
    return Color(
               sdl_colors[index].r,
               sdl_colors[index].g,
               sdl_colors[index].b
           );
}



LegacyPalette::LegacyPalette()
{
}

LegacyPalette::~LegacyPalette()
{
}

void LegacyPalette::set(uint8_t index, const Color &color)
{
    uint8_t *pal = graphics.palette();

    // this unfortunately discards the alpha channel
    // chop the lowest two bits while copying
    pal[index * 3 + 0] = color.r >> 2;
    pal[index * 3 + 1] = color.g >> 2;
    pal[index * 3 + 2] = color.b >> 2;
}

const Color LegacyPalette::get(uint8_t index) const
{
    uint8_t *pal = graphics.palette();
    uint8_t
    r = pal[index * 3 + 0],
    g = pal[index * 3 + 1],
    b = pal[index * 3 + 2];

    // copy the top two bits to the lowest two, so 0 = 0 and 63 = 255
    return Color(
               (r << 2) | (r >> 6),
               (g << 2) | (g >> 6),
               (b << 2) | (b >> 6)
           );
}

}