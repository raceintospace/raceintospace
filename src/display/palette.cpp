#include <memory.h>
#include <assert.h>

#include "palette.h"

// the global palette, defined somewhere in RIS
// as an array, it is unaffected by structure packing
extern unsigned char pal[3 * 256];

namespace display {

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

void Palette::set(uint8_t index, const Color& color)
{
    colors[index] = color;
}

const Color Palette::get(uint8_t index) const
{
    return colors[index];
}



LegacyPalette::LegacyPalette()
{
}

LegacyPalette::~LegacyPalette()
{
}

void LegacyPalette::set(uint8_t index, const Color& color)
{
    // this unfortunately discards the alpha channel
    // chop the lowest two bits while copying
    ::pal[index * 3 + 0] = color.r >> 2;
    ::pal[index * 3 + 1] = color.g >> 2;
    ::pal[index * 3 + 2] = color.b >> 2;
}

const Color LegacyPalette::get(uint8_t index) const
{
    uint8_t
        r = ::pal[index * 3 + 0],
        g = ::pal[index * 3 + 0],
        b = ::pal[index * 3 + 0];
    
    // copy the top two bits to the lowest two, so 0 = 0 and 63 = 255
    return Color(
        (r << 2) | (r >> 6),
        (g << 2) | (g >> 6),
        (b << 2) | (b >> 6)
        );
}

}