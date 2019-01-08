#ifndef DISPLAY__COLOR_H
#define DISPLAY__COLOR_H

#include <stdint.h>

// Represents an 8-bit RGBA color
#pragma pack(push, 1)
struct Color {
    Color()
        : r(0), g(0), b(0), a(255) {};
    Color(uint32_t rgba)
        : r(rgba >> 24), g((rgba >> 16) & 0xff), b((rgba >> 8) & 0xff), a(rgba & 0xff) {};
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0)
        : r(r), g(g), b(b), a(a) {};
    Color(const Color &copy)
        : r(copy.r), g(copy.g), b(copy.b), a(copy.a) {};

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    inline uint32_t rgba() const
    {
        return (r << 24) | (g << 16) | (b << 8) | a;
    };

    // is this color totally transparent?
    inline bool transparent() const
    {
        return a == 0;
    };

    // is this color totally opaque?
    inline bool opaque() const
    {
        return a == 255;
    };
};// __attribute__((packed));
#pragma pack(pop)

#endif // DISPLAY__COLOR_H