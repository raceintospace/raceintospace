#include "image.h"

#include "graphics.h"
#include "surface.h"
#include "palettized_surface.h"

#include <png.h>
#include <zlib.h>
#include <assert.h>
#include <errno.h>

#include <boost/format.hpp>
#include <string>
#include <stdexcept>

namespace display
{

namespace image
{

//----------------------------------------------------------------------------
// PNG implementation details
class PNGReader
{
public:
    PNGReader(const void *b, size_t l);
    ~PNGReader();
    PalettizedSurface *readIntoPalettizedSurface();

    png_structp png_ptr;
    png_infop info_ptr;

    inline uint32_t width()
    {
        return png_get_image_width(png_ptr, info_ptr);
    };
    inline uint32_t height()
    {
        return png_get_image_height(png_ptr, info_ptr);
    };

    const char *buffer;
    size_t length;
    size_t cursor;
};

void read_png_from_memory(png_structp png_ptr, png_bytep output, png_size_t length)
{
    PNGReader *data = (PNGReader *)png_get_io_ptr(png_ptr);

    if (data->length - data->cursor < length) {
        png_error(png_ptr, "libpng requested more data than is available");
    }

    memcpy(output, data->buffer + data->cursor, length);
    data->cursor += length;
}

void throw_exception_on_png_error(png_structp png_ptr, png_const_charp error_msg)
{
    fprintf(stderr, "libpng error: %s\n", error_msg);
    throw std::runtime_error(error_msg);
}

PNGReader::PNGReader(const void *b, size_t l) : buffer((const char *)b), length(l), cursor(0)
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, throw_exception_on_png_error, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_set_read_fn(png_ptr, this, read_png_from_memory);
}

PNGReader::~PNGReader()
{
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

PalettizedSurface *PNGReader::readIntoPalettizedSurface()
{
    // read the palette
    Palette palette;
    png_color *png_palette;
    int num_palette;
    png_get_PLTE(png_ptr, info_ptr, &png_palette, &num_palette);

    // read the alpha values into a local
    uint8_t *trans;
    int num_trans;
    png_color_16p trans_values = NULL;

    if (png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values) == 0) {
        // no transparency
        num_trans = 0;
    }

    // copy colors into our local palette
    for (int i = 0; i < 256; i++) {

        if (i < num_palette) {
            Color color(
                png_palette[i].red,
                png_palette[i].green,
                png_palette[i].blue,
                (i < num_trans) ? trans[i] : 255
            );
            palette.set(i, color);
        }

    }

    // create the target surface
    PalettizedSurface *surface;
    surface = new PalettizedSurface(width(), height(), palette);

    // copy the image data itself
    // this will only work for 8-bit images, but that's what we are (right?)
    png_bytep *row_pointers;
    row_pointers = png_get_rows(png_ptr, info_ptr);

    for (unsigned int i = 0; i < surface->height(); i ++) {
        surface->copyRow(i, row_pointers[i]);
    }

    return surface;
}

//----------------------------------------------------------------------------
// display::image implementation

Surface *readPNG(const void *buffer, size_t length)
{
    // we can't handle true-color PNGs, so pass through to readPalettizedPNG()
    return readPalettizedPNG(buffer, length);
}

PalettizedSurface *readPalettizedPNG(const void *buffer, size_t length)
{
    PNGReader r(buffer, length);

    png_read_png(r.png_ptr, r.info_ptr, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

    switch (png_get_color_type(r.png_ptr, r.info_ptr)) {
    case PNG_COLOR_TYPE_PALETTE: {
        return r.readIntoPalettizedSurface();
    }
    break;

    default:
        throw std::runtime_error("unhandled PNG color_type; is this an 8-bit palettized image?");
    }
}


#define formatted_libpng_version(version) ((boost::format("%1%.%2%.%3%") % (version / 10000) % ((version / 100) % 100) % (version % 100)).str())

std::string libpng_runtime_version()
{
    png_uint_32 version = png_access_version_number();
    return formatted_libpng_version(version);
}

std::string libpng_headers_version()
{
    return formatted_libpng_version(PNG_LIBPNG_VER);
}

bool libpng_versions_match()
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

} // namespace image

} // namespace display
