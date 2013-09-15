#ifndef DISPLAY_IMAGE_H
#define DISPLAY_IMAGE_H

#include "palette.h"
#include "graphics.h"
#include "surface.h"
#include "palettized_surface.h"

#include <stdio.h>
#include <string>
#include <SDL/SDL.h>

#include <boost/shared_ptr.hpp>

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
