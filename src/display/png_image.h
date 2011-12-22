#ifndef DISPLAY__PNG_IMAGE_H
#define DISPLAY__PNG_IMAGE_H

#include <stdio.h>

#include "display/palette.h"

// we avoid including <png.h> here to minimize the number of global symbols -- however,
// PNGImage wants to hold onto some libpng pointers, so we need to declare them as structs
struct png_struct_def;
struct png_info_struct;

namespace display {

class PNGImage
{
public:
    // create a new PNGImage by reading from a file
    PNGImage(const char * filename);
    
    // create a new PNGImage by reading from a <stdio.h> file pointer
    PNGImage(FILE * fp);
    
    // create a new PNGImage by reading from a buffer
    // note that the buffer can be freed or re-used immediately after construction
    PNGImage(const void * buffer, size_t length);
    
    // copy an existing PNGImage
    PNGImage(const PNGImage &copy);
    
    virtual ~PNGImage();
    
    inline int get_width() const { return width; };
    inline int get_height() const { return height; };
    inline const Palette& get_palette() const { return palette; };
    
    inline void export_to_legacy_palette(uint8_t start=0, uint8_t count=255) const {
        legacy_palette.copy_from(get_palette(), start, count);
    };
    
    // draw this PNG onto the global screen
    void draw(int x = 0, int y = 0);
    
protected:
    // these exist just long enough to read everything in the constructor
    png_struct_def * png_ptr;
    png_info_struct * info_ptr;
    
    int width, height;
    uint8_t * pixel_data;
    Palette palette;
    
    void init_png();
    void read_png();
    void destroy_png();
};

};

#endif // DISPLAY_PNG_IMAGE_H
