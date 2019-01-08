#ifndef DISPLAY__PALETTE_H
#define DISPLAY__PALETTE_H

#include "color.h"
#include <stdio.h>
#include <SDL/SDL.h>
#include <assert.h>

namespace display
{

// Represents a set of 256 { r, g, b, a } values
class PaletteInterface
{
public:
    virtual ~PaletteInterface();

    virtual void set(uint8_t index, const Color &color) = 0;
    virtual const Color get(uint8_t index) const = 0;

    inline void copy_from(const PaletteInterface &other, uint8_t start = 0, uint8_t end = 255)
    {
        assert(start <= end);

        for (int i = start; i <= end; i++) {
            const Color &color = other.get(i);
            set(i, color);
        }
    };

    inline const Color operator[](uint8_t index) const
    {
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

// Presents a PaletteInterface for a SDL_Surface pointer. This allows
// us to get(), set(), and copy_from() palettes belonging to SDL surfaces.
//
// SDL_Color does not support alpha, so all Colors are forced to opaque
class SDLPaletteWrapper : public PaletteInterface
{
public:
    SDLPaletteWrapper(SDL_Surface *sdl_surface);
    virtual ~SDLPaletteWrapper();

    virtual void set(uint8_t index, const Color &color);
    virtual const Color get(uint8_t index) const;

protected:
    SDL_Surface *_sdl_surface;
};

class LegacySurface;

// AutoPal is helper class to bridge a PaletteInterface to a pal[768] array.
// The pal[768] array is created from the target palette in the constructor,
// and is copied back in the destructor.
//
// This facilitates the following idiom:
//
// {
//    AutoPal p(surface.palette());
//    /* do something with p.pal */
// } /* palette changes are automatically pushed back when the scope closes */
//
// This is similar in flavor to AutoLock schemes.
class AutoPal
{
public:
    AutoPal(PaletteInterface &pal);
    AutoPal(LegacySurface *legacySurface);
    ~AutoPal();

    struct triple {
        char r;
        char g;
        char b;
    };

    inline char *operator()()
    {
        return pal;
    };
    char pal[768];

    inline triple *operator[](int index)
    {
        int i = (index * 3);
        triple *t = (triple *)(pal + i);
        return t;
    }

private:
    PaletteInterface &_pal;
};

};

#endif // DISPLAY__PALETTE_H