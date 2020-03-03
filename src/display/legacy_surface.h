#ifndef DISPLAY_LEGACY_SURFACE_H
#define DISPLAY_LEGACY_SURFACE_H

#include "surface.h"
#include "palette.h"

namespace display
{

// Represents a 2D array of pixels, along with its associated [opaque] color palette.
// Every aspect of this surface is mutable:
//
//   * You can twiddle the pixel buffer directly, no locking needed.
//     It's also guaranteed not to have internal padding.
//   * You can twiddle the palette buffer directly. It's 768 bytes long,
//     and only the bottom 6 bits of each palette entry are significant.
//     updatePalette() must be called to map the legacy palette into an SDL
//     palette after setting the palette and before any draw()ing.
//
// These assumptions are consistent with the DOS version's video environemnt.
class LegacySurface : public Surface
{
public:
    LegacySurface(unsigned int width, unsigned int height);
    virtual ~LegacySurface();

    enum Operation {
        Set,
        Xor
    };

    enum MaskSource {
        SourceEqual,
        DestinationEqual,
        SourceNotEqual,
        DestinationNotEqual
    };

    enum FilterTest {
        Equal,
        NotEqual,
        Any
    };

    inline char *pixels()
    {
        return _pixels;
    };

    // Return a PaletteInterface for this surface
    //
    // You'd better use this pointer, since retrieving it sets hasValidPalette().
    inline PaletteInterface &palette()
    {
        _hasValidPalette = true;
        return *_palette;
    };

    inline bool hasValidPalette() const
    {
        return _hasValidPalette;
    };

    // Clear the hasValidPalette() flag
    // The next copyFrom() or copyTo() operation will thus inherit the target's palette
    inline void resetPalette()
    {
        _hasValidPalette = false;
    };

    char *pixels() const;
    void clear(char colour);
    void fillRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void fillRect(const SDL_Rect &area, char color);
    void setPixel(unsigned int x, unsigned int y, char color);
    char getPixel(unsigned int x, unsigned int y);
    void outlineRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void copyFrom(const LegacySurface *surface, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void copyFrom(const LegacySurface *surface, unsigned int srcX1, unsigned int srcY1, unsigned int srcX2, unsigned int srcY2, unsigned int dstX, unsigned int dstY);
    void copyTo(LegacySurface *surface, unsigned int x, unsigned int y, Operation operation = Set);
    void copyTo(LegacySurface *surface, unsigned int srcX, unsigned int srcY, unsigned int destX1, unsigned int destY1, unsigned int destX2, unsigned int destY2);
    void scaleTo(const LegacySurface *surface);
    void maskCopy(const LegacySurface *source, char maskValue, MaskSource maskSource, char offset = 0);
    void filter(char testValue, char offset, FilterTest filterTest);

    // Set a color as transparent, or -1 to disable transaprency
    void setTransparentColor(int color = -1);

private:
    char *_pixels;
    display::SDLPaletteWrapper *_palette;
    bool _hasValidPalette;

    // Whenever we're copying between two surfaces, we call this function to check their palettes.
    // If both surfaces have valid palettes, it compares them for equality and raises an assertion
    // failure if they're different. If only one of the surfaces has a valid palette, it copies the
    // palette prior to performing a draw operation.
    void checkPaletteCompatibility(const LegacySurface *other);
    void checkPaletteCompatibility(LegacySurface *other);
};

} // namespace display

#include <boost/static_assert.hpp>
// ensure we weren't included with modified structure packing
// if this line explodes, move the display includes to the top
BOOST_STATIC_ASSERT(sizeof(display::LegacySurface) % 4 == 0);

#endif // DISPLAY_PALETTIZED_SURFACE_H
