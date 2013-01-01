#ifndef DISPLAY__PALETTE_H
#define DISPLAY__PALETTE_H

#include "color.h"
#include <stdio.h>
#include <SDL.h>

namespace display
{

// Represents a set of 256 { r, g, b, a } values
class PaletteInterface
{
public:
    virtual ~PaletteInterface();

    virtual void set(uint8_t index, const Color &color) = 0;
    virtual const Color get(uint8_t index) const = 0;

    inline void copy_from(const PaletteInterface &other, uint8_t start = 0, uint8_t count = 255, uint8_t offset = 0) {
        for (uint8_t i = 0; i < count; i++) {
            set(i + offset, other.get(i + start));
        }
    };

    inline const Color operator[](uint8_t index) const {
        return get(index);
    };
};

class Palette : public PaletteInterface
{
public:
    Palette();
    Palette(const Palette &copy);
    Palette(const PaletteInterface &copy);
    virtual ~Palette();

    virtual void set(uint8_t index, const Color &color);
    virtual const Color get(uint8_t index) const;

protected:
    Color colors[256];
};

// Presents a PaletteInterface for an SDL_Color[256] array
// SDL_Color does not support alpha, so all Colors are forced to opaque
class SDLPalette : public PaletteInterface
{
public:
    SDLPalette();
    virtual ~SDLPalette();

    virtual void set(uint8_t index, const Color &color);
    virtual const Color get(uint8_t index) const;

    SDL_Color sdl_colors[256];
};

// Presents a PaletteInterface for the "pal" 768-byte { r, g, b } array, where r/g/b
// have 6 significant bits but use 8 in RAM
class LegacyPalette : public PaletteInterface
{
public:
    LegacyPalette();
    virtual ~LegacyPalette();

    virtual void set(uint8_t index, const Color &color);
    virtual const Color get(uint8_t index) const;
};

extern LegacyPalette legacy_palette;

};

#endif // DISPLAY__PALETTE_H