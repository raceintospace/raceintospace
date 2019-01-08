#ifndef DISPLAY_PALETTIZED_SURFACE_H
#define DISPLAY_PALETTIZED_SURFACE_H

#include "surface.h"
#include "palette.h"
#include "graphics.h"

namespace display
{

// Represents a 2D array of pixels, along with its associated [opaque] color palette.
class PalettizedSurface : public Surface
{
public:
    PalettizedSurface(unsigned int width, unsigned int height, const PaletteInterface &palette);
    virtual ~PalettizedSurface();

    // Exports this palette -- or a subset of it -- to the legacy palette
    void exportPalette(uint8_t start = 0, uint8_t end = 255) const
    {
        display::graphics.legacyScreen()->palette().copy_from(_palette, start, end);
    };

    // Directly copies a row of image data into this surface
    void copyRow(uint32_t row, const void *pixelData);

    // Get a const reference to this palette
    inline const PaletteInterface &palette()
    {
        return _palette;
    };

private:
    display::SDLPaletteWrapper _palette;
};

} // namespace display

#include <boost/static_assert.hpp>
// ensure we weren't included with modified structure packing
// if this line explodes, move the display includes to the top
BOOST_STATIC_ASSERT(sizeof(display::PalettizedSurface) % 4 == 0);

#endif // DISPLAY_PALETTIZED_SURFACE_H