#ifndef DISPLAY_IMAGE_H
#define DISPLAY_IMAGE_H

#include <cstdio>
#include <string>
#include <SDL.h>

#include <boost/shared_ptr.hpp>

#include "graphics.h"
#include "palette.h"
#include "palettized_surface.h"
#include "surface.h"


namespace display
{

namespace image
{

Surface *readPNG(const void *buffer, size_t length);
PalettizedSurface *readPalettizedPNG(const void *buffer, size_t length);

std::string libpng_headers_version();
std::string libpng_runtime_version();
bool libpng_versions_match();

}

} // namespace display

#endif // DISPLAY_IMAGE_H
