#include "image.h"

#include "graphics.h"
#include "surface.h"

#include <png.h>
#include <zlib.h>
#include <assert.h>
#include <errno.h>

#include <boost/format.hpp>
#include <string>
#include <stdexcept>

namespace display
{

namespace
{
// anonymous namespace eliminates the possibility of symbol collision

// libpng helpers for use within this module
struct read_png_from_memory_t {
    const char *buffer;
    size_t length;
    size_t cursor;
};

void read_png_from_memory(png_structp png_ptr, png_bytep output, png_size_t length)
{
    read_png_from_memory_t *data = (read_png_from_memory_t *)png_get_io_ptr(png_ptr);

    if (data->length - data->cursor < length) {
        png_error((png_structp)png_ptr, "libpng requested more data than is available");
    }

    memcpy(output, data->buffer + data->cursor, length);
    data->cursor += length;
}

void throw_exception_on_png_error(png_structp png_ptr, png_const_charp error_msg)
{
    fprintf(stderr, "libpng error: %s\n", error_msg);
    throw std::runtime_error(error_msg);
}

} // namespace anonymous


//--------------------------------------------------------------------------------------------------------
// Image implementation

void Image::init_png()
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, throw_exception_on_png_error, NULL);
    info_ptr = png_create_info_struct((png_structp)png_ptr);
}

void Image::read_png()
{
    png_read_png((png_structp)png_ptr, (png_infop)info_ptr, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

    _width = png_get_image_width((png_structp)png_ptr, (png_infop)info_ptr);
    _height = png_get_image_height((png_structp)png_ptr, (png_infop)info_ptr);

    switch (png_get_color_type((png_structp)png_ptr, (png_infop)info_ptr)) {
    case PNG_COLOR_TYPE_PALETTE: {
        _surface = SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 8, 0, 0, 0, 0);
        // read the palette into a local
        png_color *png_palette;
        int num_palette;
        png_get_PLTE((png_structp)png_ptr, (png_infop)info_ptr, &png_palette, &num_palette);

        // read the alpha values into a local
        uint8_t *trans;
        int num_trans;

        if (png_get_tRNS((png_structp)png_ptr, (png_infop)info_ptr, &trans, &num_trans, NULL) == 0) {
            // no transparency
            num_trans = 0;
        }

        // copy both into the instance-wide palette
        for (int i = 0; i < 256; i++) {
            Color c;

            if (i < num_palette) {
                c.r = png_palette[i].red;
                c.g = png_palette[i].green;
                c.b = png_palette[i].blue;
                c.a = (i < num_trans) ? trans[i] : 255;
            }

            _palette.set(i, c);
        }
    }
    break;

    default:
        throw std::runtime_error("unhandled PNG color_type; is this an 8-bit palettized image?");
    }

    // copy the image data
    {
        png_bytep *row_pointers;
        row_pointers = png_get_rows((png_structp)png_ptr, (png_infop)info_ptr);

        // this assumes 8-bit images
        for (unsigned int i = 0; i < _height; i ++) {
            memcpy(&((char *)_surface->pixels)[_surface->pitch * i], row_pointers[i], _width);
        }
    }

    destroy_png();
}

void Image::destroy_png()
{
    png_destroy_read_struct((png_structpp)(&png_ptr), (png_infopp)(&info_ptr), NULL);
}

Image::Image(const std::string &filename):
    _width(0),
    _height(0),
    _surface(NULL)
{
    FILE *fp;
    fp = fopen(filename.c_str(), "rb");

    if (!fp) {
        throw std::runtime_error("unable to open file");
    }

    try {
        init_png();
        png_init_io((png_structp)png_ptr, fp);
        read_png();
        fclose(fp);
    } catch (...) {
        destroy_png();
        fclose(fp);
        throw;
    }

    fclose(fp);
}

Image::Image(FILE *fp):
    _width(0),
    _height(0),
    _surface(NULL)
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, throw_exception_on_png_error, NULL);

    try {
        init_png();
        png_init_io((png_structp)png_ptr, fp);
        read_png();
    } catch (...) {
        destroy_png();
        throw;
    }
}

Image::Image(const void *buffer, size_t length):
    _width(0),
    _height(0),
    _surface(NULL)
{
    read_png_from_memory_t descriptor;
    descriptor.buffer = (const char *) buffer;
    descriptor.length = length;
    descriptor.cursor = 0;

    try {
        init_png();
        png_set_read_fn((png_structp)png_ptr, &descriptor, read_png_from_memory);
        read_png();
    } catch (...) {
        destroy_png();
        throw;
    }
}

Image::Image(const Image &source):
    _width(source._width),
    _height(source._height),
    _palette(source._palette)
{
    _surface = SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 8, 0, 0, 0, 0);
    // XXX: is this supposed to copy the bits?
}

Image::~Image()
{
    if (_surface) {
        SDL_FreeSurface(_surface);
        _surface = NULL;
    }
}

// Create a screenshot
Image::Image(const Graphics &source, int x, int y, int width, int height):
    _width(width),
    _height(height),
    _palette(legacy_palette)
{
    _surface = SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 8, 0, 0, 0, 0);

    SDL_Rect src;
    src.x = x;
    src.y = y;
    src.w = _width;
    src.h = _height;

    SDL_Rect dst;
    dst.x = 0;
    dst.y = 0;
    dst.w = _width;
    dst.h = _height;

    SDL_BlitSurface(source.screen()->surface(), &src, _surface, &dst);
}

void Image::write_png(const std::string &filename)
{
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    uint8_t **rows = (uint8_t **)alloca(sizeof(void *) * _height);

    fp = fopen(filename.c_str(), "wb");

    if (!fp) {
        fprintf(stderr, "unable to open output file: %s\n", strerror(errno));
        throw std::runtime_error("unable to open output file");
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        throw std::runtime_error("unable to create write struct");
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr) {
        throw std::runtime_error("unable to create info struct");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        throw std::runtime_error("PNG write error");
    }

    png_init_io(png_ptr, fp);

    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    png_set_IHDR(png_ptr, info_ptr,
                 _width, _height,
                 8, PNG_COLOR_TYPE_PALETTE,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
                );

    png_color png_pal[256];

    for (int i = 0; i < 256; i ++) {
        const Color &color = _palette.get(i);
        png_pal[i].red = color.r;
        png_pal[i].green = color.g;
        png_pal[i].blue = color.b;
    }

    png_set_PLTE(png_ptr, info_ptr, png_pal, 256);

    for (int i = 0; i < _height; i++) {
        rows[i] = ((uint8_t *)_surface->pixels) + (_surface->pitch * i);
    }

    png_set_rows(png_ptr, info_ptr, rows);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    fclose(fp);
}

#define formatted_libpng_version(version) ((boost::format("%1%.%2%.%3%") % (version / 10000) % ((version / 100) % 100) % (version % 100)).str())

std::string Image::libpng_runtime_version()
{
    png_uint_32 version = png_access_version_number();
    return formatted_libpng_version(version);
}

std::string Image::libpng_headers_version()
{
    return formatted_libpng_version(PNG_LIBPNG_VER);
}

bool Image::libpng_versions_match()
{
#if PNG_LIBPNG_VER > 10000
    // xx.yy.zz = xxyyzz
    // compare only on xx.yy
    return (png_access_version_number() / 100) == (PNG_LIBPNG_VER / 100);
#else
    // who knows?
    return png_access_version_number() == PNG_LIBPNG_VER;
#endif
}

} // namespace display
