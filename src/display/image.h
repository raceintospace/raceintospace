#ifndef DISPLAY_IMAGE_H
#define DISPLAY_IMAGE_H

#include "palette.h"
#include "graphics.h"

#include <stdio.h>
#include <string>
#include <SDL.h>

#include <boost/shared_ptr.hpp>

namespace display
{

class Image
{
public:
    // create a new PNGImage by reading from a buffer
    // note that the buffer can be freed or re-used immediately after construction
    Image(const void *buffer, size_t length);

    // copy an existing PNGImage
    Image(const Image &source);

    // create a screenshot
    Image(const Graphics &source, int x = 0, int y = 0, int width = Graphics::WIDTH, int height = Graphics::HEIGHT);

    virtual ~Image();

    unsigned int width() const {
        return _width;
    };

    unsigned int height() const {
        return _height;
    };

    const Palette &palette() const {
        return _palette;
    };

    void exportPalette(uint8_t start = 0, uint8_t end = 255) const {
        legacy_palette.copy_from(_palette, start, end);
    };

    SDL_Surface *surface() const {
        return _surface;
    }

    void write_png(const std::string &filename);

    static std::string libpng_headers_version();
    static std::string libpng_runtime_version();
    static bool libpng_versions_match();

private:
    void init_png();
    void read_png();
    void destroy_png();

    // these exist just long enough to read everything in the constructor
    void *png_ptr;
    void *info_ptr;

    unsigned int _width;
    unsigned int _height;
    SDL_Surface *_surface;
    Palette _palette;

};

} // namespace display

#endif // DISPLAY_IMAGE_H
