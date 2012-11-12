#include <png.h>
#include <zlib.h>
#include <assert.h>

#include <boost/format.hpp>
#include <string>
#include <stdexcept>

#include "png_image.h"

// the global screen buffer, defined somewhere in RIS
extern unsigned char *screen;

namespace display
{

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


// PNGImage implementation

void PNGImage::init_png()
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, throw_exception_on_png_error, NULL);
    info_ptr = png_create_info_struct((png_structp)png_ptr);
}

void PNGImage::read_png()
{
    png_read_png((png_structp)png_ptr, (png_infop)info_ptr, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

    width = png_get_image_width((png_structp)png_ptr, (png_infop)info_ptr);
    height = png_get_image_height((png_structp)png_ptr, (png_infop)info_ptr);

    switch (png_get_color_type((png_structp)png_ptr, (png_infop)info_ptr)) {
    case PNG_COLOR_TYPE_PALETTE: {
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

            palette.set(i, c);
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
        pixel_data = new uint8_t[width * height];

        // this assumes 8-bit images
        for (int i = 0; i < height; i ++) {
            memcpy(&pixel_data[width * i], row_pointers[i], width);
        }
    }

    destroy_png();
}

void PNGImage::destroy_png()
{
    png_destroy_read_struct((png_structpp)(&png_ptr), (png_infopp)(&info_ptr), NULL);
}

PNGImage::PNGImage(const char *filename)
    : pixel_data(NULL)
{
    FILE *fp;
    fp = fopen(filename, "rb");

    if (!fp) {
        throw std::runtime_error("unable to open file");
    }

    try {
        init_png();
        png_init_io((png_structp)png_ptr, fp);
        read_png();
        fclose(fp);
    } catch (std::runtime_error &e) {
        destroy_png();
        fclose(fp);
        throw;
    }

    fclose(fp);
}

PNGImage::PNGImage(FILE *fp)
    : pixel_data(NULL)
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, throw_exception_on_png_error, NULL);

    try {
        init_png();
        png_init_io((png_structp)png_ptr, fp);
        read_png();
    } catch (std::runtime_error &e) {
        destroy_png();
        throw;
    }
}

PNGImage::PNGImage(const void *buffer, size_t length)
    : pixel_data(NULL)
{
    read_png_from_memory_t descriptor;
    descriptor.buffer = (const char *) buffer;
    descriptor.length = length;
    descriptor.cursor = 0;

    try {
        init_png();
        png_set_read_fn((png_structp)png_ptr, &descriptor, read_png_from_memory);
        read_png();
    } catch (std::runtime_error &e) {
        destroy_png();
        throw;
    }
}

PNGImage::PNGImage(const PNGImage &copy)
    : width(copy.width), height(copy.height), palette(copy.palette)
{
    pixel_data = new uint8_t[width * height];
    memcpy(pixel_data, copy.pixel_data, width * height);
}

PNGImage::~PNGImage()
{
    if (pixel_data) {
        delete[] pixel_data;
    }
}

void PNGImage::draw(int x, int y)
{
    int width_to_copy;
    int height_to_copy;

    assert(x >= 0);
    assert(y >= 0);
    assert(x < 320);
    assert(y < 200);

    width_to_copy = 320 - x;

    if (width_to_copy > width) {
        width_to_copy = width;
    }

    height_to_copy = 200 - y;

    if (height_to_copy > height) {
        height_to_copy = height;
    }

    for (int i = 0; i < height_to_copy; i++) {
        // this assumes an 8-bit screen buffer and 8-bit pixel_data, both with no padding
        memcpy(&screen[320 * (y + i) + x], &pixel_data[width * i], width_to_copy);
    }
}

#define formatted_libpng_version(version) ((boost::format("%1%.%2%.%3%") % (version / 10000) % ((version / 100) % 100) % (version % 100)).str())

std::string PNGImage::libpng_runtime_version()
{
    png_uint_32 version = png_access_version_number();
    return formatted_libpng_version(version);
}

std::string PNGImage::libpng_headers_version()
{
    return formatted_libpng_version(PNG_LIBPNG_VER);
}

bool PNGImage::libpng_versions_match()
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

};
