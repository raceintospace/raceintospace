#include <png.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

/* known strategies:
    utils/but2png ../gamedata/prefs.but "320x240 RLE"
    utils/but2png ../gamedata/turn.but "320x240 RLE"
    utils/but2png ../gamedata/vab.img vab
    utils/but2png ../gamedata/moon.but moon
    utils/but2png ../gamedata/aprog.but aprog
    utils/but2png ../gamedata/lmer.but lmer
    utils/but2png ../gamedata/intel.but intel
    utils/but2png ../gamedata/rdfull.but rdfull
    utils/but2png ../gamedata/prfx.but prfx
    utils/but2png ../gamedata/lfacil.but lfacil
    utils/but2png ../gamedata/usa_port.dat port
    utils/but2png ../gamedata/sov_port.dat port
    utils/but2png ../gamedata/winner.but winner
    utils/but2png ../gamedata/satbld.but satbld
    utils/but2png ../gamedata/budd.but budd         # XXX: unknown palette
    utils/but2png ../gamedata/endgame.but endgame
    utils/but2png ../gamedata/loser.but loser
    utils/but2png ../gamedata/beggam.but beggam
    utils/but2png ../gamedata/patches.but patches   # XXX: missing palettes, PatchMe() data hack

    mv ../gamedata/*.png ../images/
*/

/* unknown strategies:
    utils/but2png ../gamedata/arrows.but
    utils/but2png ../gamedata/cia.but
    utils/but2png ../gamedata/faces.but
    utils/but2png ../gamedata/flagger.but
    utils/but2png ../gamedata/inte_1.but
    utils/but2png ../gamedata/lenin.but
    utils/but2png ../gamedata/loser.but
    utils/but2png ../gamedata/lpads.but
    utils/but2png ../gamedata/mhist.but
    utils/but2png ../gamedata/nfutbut.but
    utils/but2png ../gamedata/portbut.but
    utils/but2png ../gamedata/presr.but
    utils/but2png ../gamedata/rdbox.but
    utils/but2png ../gamedata/tracker.but
    utils/but2png ../gamedata/control.img
    utils/but2png ../gamedata/first.img
    utils/but2png ../gamedata/fmin.img
    utils/but2png ../gamedata/news.img
*/


#pragma pack(push, 1)
struct {
    uint8_t r, g, b;
} pal[256];

typedef struct  {
    int16_t size;
    int32_t offset;
} SimpleHdr;

typedef struct {
    int16_t w;
    int16_t h;
    uint16_t size;
    uint32_t offset;
} PatchHdr;

typedef struct {
    int8_t w;
    int8_t h;
    uint16_t size;
    uint32_t offset;
} PatchHdrSmall;
#pragma pack(pop)

uint8_t raw_data[320 * 240];
uint8_t screen[320 * 240];
uint8_t color_is_transparent[256];

int width, height;

int offset_x, offset_y;

int output_number;
char output_pattern[256];

int
RLED_img(void *src_raw, void *dest_raw, unsigned int src_size, int w, int h)
{
    signed char *src = src_raw;
    signed char *dest;
    unsigned int used;
    int count, val;
    int total;
    signed char buf[128 * 1024];
    int row;

    dest = buf;

    used = 0;

    while (used < src_size) {
        count = src[used++];

        if (count < 0) {
            count = -count + 1;
            val = src[used++];
            memset(dest, val, count);
            dest += count;
        } else {
            count++;
            memcpy(dest, &src[used], count);
            used += count;
            dest += count;
        }
    }

    total = dest - buf;

    if (total < w * h + h) {
        memcpy(dest_raw, buf, w * h);
        return (w * h);
    }

    dest = dest_raw;

    for (row = 0; row < h; row++) {
        memcpy(dest, &buf[row * (w + 1)], w);
        dest += w;
    }

    return (w * h);
}

int
PCX_D(void *src_raw, void *dest_raw, unsigned src_size)
{
    char *src = src_raw;
    char *dest = dest_raw;
    char num;
    char *orig_dest = dest;

    do {
        if ((*src & 0xc0) == 0xc0) {
            num = *(src++) & 0x3f;
            src_size--;

            while ((num--) > 0) {
                *(dest++) = *src;
            }

            src++;
            src_size--;
        } else {
            (*dest++) = *(src++);
            src_size--;
        };
    }  while (src_size);

    return (dest - orig_dest);
}

int write_image()
{
    FILE * fp;
    png_structp png_ptr;
    png_infop info_ptr;
    char filename[256];
    uint8_t * rows[240];
    uint8_t transparent_index[256], transparent_colors;
    int i;
    
    snprintf(filename, sizeof(filename), output_pattern, output_number++);
    
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "unable to open output file: %s\n", strerror(errno));
        return 4;
    }
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        return 5;
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return 6;
    
    if (setjmp(png_jmpbuf(png_ptr))) {
       png_destroy_write_struct(&png_ptr, &info_ptr);
       fclose(fp);
       fprintf(stderr, "PNG write error!\n");
       return 7;
    }
    
    png_init_io(png_ptr, fp);
    
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
    
    png_set_IHDR(png_ptr, info_ptr,
        width, height,
        8, PNG_COLOR_TYPE_PALETTE,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
        );
    
    png_color png_pal[256];
    for (i = 0; i < 256; i ++) {
        // pal[i].{r,g,b} are 0-63, and we want to map to 0-255
        // fill in the right two bits with the first two
        // this gives the desirable properties that 0 = 0 and 63 = 255
        png_pal[i].red = (pal[i].r << 2 | pal[i].r >> 6);
        png_pal[i].green = (pal[i].g << 2 | pal[i].g >> 6);
        png_pal[i].blue = (pal[i].b << 2 | pal[i].b >> 6);
    }
    png_set_PLTE(png_ptr, info_ptr, png_pal, 256);
    
    // copy any transparent palette entries
    transparent_colors = 0;
    for (i = 0; i < 256; i ++) {
        if (color_is_transparent[i]) {
            transparent_index[transparent_colors++] = i;
        }
    }
    if (transparent_colors > 0) {
        png_set_tRNS(png_ptr, info_ptr, transparent_index, transparent_colors, NULL);
    }
    
    // copy the offset, if any
    if (offset_x >= 0 && offset_y >= 0) {
        png_set_oFFs(png_ptr, info_ptr, offset_x, offset_y, PNG_OFFSET_PIXEL);
    }
    
    for (i = 0; i < height; i++) {
        rows[i] = screen + (i * width);
    }
    png_set_rows(png_ptr, info_ptr, rows);
    
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    
    return 0;
}

// decodes a simple_hdr packed image
//  - palette_offset: the number of colors to skip, since some packed images
//    use only a subset of colors
//  - shift_data: if set, shifts the color values read up by palette_offset
//  - single_palette: sometimes there's one palette at a fixed location
//    if so, give the number of headers in that it starts at
int simplehdrs(FILE * fp, int colors, int palette_offset, int shift_data, int single_palette)
{
    SimpleHdr headers[100];
    int i, last_valid_header, rv;
    
    // read as many headers as possible
    fread(headers, sizeof(headers), 1, fp);
    
    // read the file-wide pallete if applicable
    if (single_palette) {
        fseek(fp, sizeof(headers[0]) * single_palette, SEEK_SET);
        memset(pal, 0, sizeof(pal));
        fread(pal + palette_offset, colors, sizeof(pal[0]), fp);
    }
    
    // see how many of them seem reasonable
    last_valid_header = 0;
    for (i = 0; i < sizeof(headers) / sizeof(headers[0]); i++) {
        printf("header %i: offset %u, size %i\n", i, headers[i].offset, headers[i].size);

        if (i > 0) {
            // compare offset to the last, and break if we're going backwards
            if (headers[i].offset < headers[last_valid_header].offset)
                continue;
        }
        
        if (headers[i].offset < 0) // this is bogus
            continue;
        if (headers[i].offset > 460000) // this is the largest file
            continue;
        
        if (headers[i].size < 50)       // this seems unreasonably small
            continue;
        if (headers[i].size > 13000)    // this seems unreasonably large
            continue;
        
        // seems valid
        printf("  valid\n");

        // seek to it
        fseek(fp, headers[i].offset, SEEK_SET);
        
        // read the individual palette, if applicable
        if (!single_palette) {
            memset(pal, 0, sizeof(pal));
            fread(pal + palette_offset, colors, sizeof(pal[0]), fp);
        }
        
        // read and decode the image data
        fread(raw_data, headers[i].size, 1, fp);
        RLED_img(raw_data, screen, headers[i].size, width, height);
        
        // rewrite the pixel data by an offset if required
        if (shift_data) {
            int pixels = width * height;
            int j;
            for (j = 0; j < pixels; j++) {
                screen[j] += palette_offset;
            }
        }
        
        // export using the correct image number
        output_number = i;
        if (rv = write_image())
            return rv;
        
        last_valid_header = i;
    }
    
    return 0;
}

int patchhdrs(FILE * fp, int use_small_headers, int palette_style, int encoding)
{
    PatchHdr headers[100];
    PatchHdrSmall small_headers[100];
    int rv, i, last_valid_offset, palette_offset;

    memset(pal, 0, sizeof(pal));
    
    if (palette_style == 1) {
        // 256 colors
        fread(pal, 1, sizeof(pal), fp);
        palette_offset = 0;

    } else if (palette_style == 2) {
        // 128 colors, starting at 128
        fread(&pal[128], 128, sizeof(pal[0]), fp);
        palette_offset = 128;
        
    } else if (palette_style == 2) {
        // 32 colors, starting at 32, but data does not need shifting
        fread(&pal[32], 32, sizeof(pal[0]), fp);
        palette_offset = 32;
    }
    
    // read as many headers as possible
    if (use_small_headers) {
        fread(small_headers, sizeof(small_headers), 1, fp);
    } else {
        fread(headers, sizeof(headers), 1, fp);
    }
    
    // see how many of them seem reasonable
    last_valid_offset = 0;
    for (i = 0; i < 100; i++) {
        int offset, size;
        if (use_small_headers) {
            offset = small_headers[i].offset;
            size = small_headers[i].size;
            width = small_headers[i].w;
            height = small_headers[i].h;
        } else {
            offset = headers[i].offset;
            size = headers[i].size;
            width = headers[i].w;
            height = headers[i].h;
        }

        printf("header %i: offset %u, size %i\n", i, offset, size);
        
        // compare offset to the last, and break if we're going backwards
        if (offset < last_valid_offset)
            continue;
        
        if (offset < 0) // this is bogus
            continue;
        if (offset > 460000) // this is the largest file
            continue;
        
        if (size < 50)       // this seems unreasonably small
            continue;
        if (size > (encoding == 1 ? 30000 : 64000))    // this seems unreasonably large
            continue;
        
        // seems valid
        printf("  valid\n");

        // seek to it
        fseek(fp, offset, SEEK_SET);
        
        // read and decode the image data
        fread(raw_data, size, 1, fp);
        
        switch (encoding) {
        case 0:
            // raw
            memcpy(screen, raw_data, size);
            break;
            
        case 1:
            // RLE
            RLED_img(raw_data, screen, size, width, height);
            break;
        
        case 2:
            // raw, but the data file contains an extra column
            {
                int y = 0;
                for (y = 0; y < height; y++) {
                    memcpy(screen + width * y, raw_data + (width + 1) * y, width);
                }
            }
            break;
        }
        
        // shift around the pixel data if appropriate
        if (palette_offset) {
            int pixels = width * height;
            int j;
            for (j = 0; j < pixels; j++) {
                screen[j] += palette_offset;
            }
        }
        
        // export using the correct image number
        output_number = i;
        if (rv = write_image())
            return rv;
        
        last_valid_offset = offset;
    }
    
    return 0;
}

// decoding strategies

int translate_320x240_rle(FILE * fp)
{
    int bytes;
    
    width = 320;
    height = 240;
    
    bytes = fread(pal, sizeof(pal), 1, fp);
    if (bytes < 1) {
        fprintf(stderr, "short read while reading palette\n");
        return 3;
    }
    
    bytes = fread(raw_data, 1, sizeof(raw_data), fp);
    fclose(fp);
    
    RLED_img(raw_data, screen, bytes, width, height);
    
    return write_image();
}

// two 320x240 PCX-compressed images, independent palettes, glued together
int translate_vab(FILE * fp)
{
    int i, rv;
    
    width = 320;
    height = 240;
    
    uint16_t bytes;
    
    for (i = 0; i < 2; i++) {
        fread(pal, sizeof(pal), 1, fp);
        fread(&bytes, sizeof(bytes), 1, fp);
        fread(raw_data, 1, bytes, fp);
        PCX_D(raw_data, screen, bytes);

        rv = write_image();
        if (rv) return rv;
    }
    
    return 0;
}

// half palette PCX-encoded 320x240
int translate_winner(FILE * fp)
{
    int size;
    
    memset(pal, 0, sizeof(pal));
    fread(pal, 384, 1, fp);
    size = fread(raw_data, 1, sizeof(raw_data), fp);
    PCX_D(raw_data, screen, size);
    
    width = 320; height = 240;
    return write_image();
}

#define SIMPLEHDRS_FILE(name, w, h, colors, pal, offset, single, transparent) \
    int translate_ ## name (FILE * fp) { \
        width = w; height = h; \
        if (transparent) color_is_transparent[0] = 1; \
        return simplehdrs(fp, colors, pal, offset, single); \
    }

SIMPLEHDRS_FILE(moon, 104, 82, 128, 128, 1, 0, 0);
SIMPLEHDRS_FILE(aprog, 80, 50, 256, 0, 0, 14, 1);
SIMPLEHDRS_FILE(lmer, 156, 89, 224, 32, 0, 8, 0);
SIMPLEHDRS_FILE(intel, 157, 100, 256, 0, 0, 71, 1);
SIMPLEHDRS_FILE(rdfull, 104, 77, 96, 32, 1, 0, 0);
SIMPLEHDRS_FILE(prfx, 127, 80, 96, 112, 1, 0, 0);
SIMPLEHDRS_FILE(lfacil, 148, 148, 256, 0, 0, 6, 0);
SIMPLEHDRS_FILE(presr, 126, 84, 224, 32, 0, 0, 0);

#define PATCHHDRS_FILE(name, use_small_headers, palette_style, encoding) \
    int translate_ ## name (FILE * fp) { \
        return patchhdrs(fp, use_small_headers, palette_style, encoding); \
    }

PATCHHDRS_FILE(satbld, 1, 1, 1);
PATCHHDRS_FILE(budd, 1, 0, 1);
PATCHHDRS_FILE(endgame, 1, 2, 2);
PATCHHDRS_FILE(loser, 0, 2, 0);
PATCHHDRS_FILE(beggam, 0, 2, 0);
PATCHHDRS_FILE(patches, 1, 3, 2);

int translate_port(FILE * fin)
{
#pragma pack(push, 1)
    // Port data structures
    struct {
        char Text[28];  /**< File Copyright Notice */
        int32_t oMObj;     /**< Offset to MObj data table */
        int32_t oTab;      /**< Offset to Table of data */
        int32_t oPal;      /**< Offset to Palette */
        int32_t oPort;     /**< Offset to start of Port Images */
        int32_t oMse;      /**< Offset to Mouse Objects */
        int32_t oOut;      /**< Offset to port Outlines */
        int32_t oAnim;     /**< Offset to start of Port Anims */
    } PHead;

    struct {
        char Name[30];       /**< Name of region */
        char qty;            /**< Number of regions */
        char Help[3];        /**< Help Box Stuff */

        struct {
            char qty;          /**< number of BOUNDS */
            struct {
                int16_t x1, y1, x2, y2;
            } CD[4];       /**< boundry coords for mouse location */
            char iNum;
            char sNum;         /**< value for surround box */
            char PreDraw;      /**< Code of Special to Draw first */
        } Reg[4];       /**< At Max 4 regions */
    } MObj[35];

    struct {
        int32_t Size;         /**<  Size of Image (bytes) */
        char Comp;         /**<  Type of Compression Used */
        int16_t Width;         /**<  Width of Image */
        int16_t Height;        /**<  Height of Image */
        int16_t PlaceX;        /**<  Where to Place Img:X */
        int16_t PlaceY;        /**<  Where to Place Img:Y */
    } Img;
#pragma pack(pop)
    
    uint32_t table[43];
    int rv, i;
    
    fread(&PHead, sizeof PHead, 1, fin);
    fread(MObj, sizeof MObj, 1, fin);
    fread(table, sizeof table, 1, fin);

    fseek(fin, PHead.oPal, SEEK_SET);
    fread(pal, sizeof(pal), 1, fin);
    
    for (i = 0; i < 43; i++) {
        fseek(fin, table[i], SEEK_SET);
        fread(&Img, sizeof Img, 1, fin);
        
        // ignore stupid values, since not all images are valid
        if (Img.Width > 321 || Img.Height > 241 || Img.Size > 64000)
            continue;
        
        if (Img.Size == (Img.Width - 1) * (Img.Height - 1)) {
            // we're raw
            width = Img.Width - 1;
            height = Img.Height - 1;
            fread(screen, Img.Size, 1, fin);

            color_is_transparent[0] = 0;
            offset_x = offset_y = -1;
        } else {
            // we're RLE'd and have transparent entries
            width = Img.Width;
            height = Img.Height;
            fread(raw_data, Img.Size, 1, fin);
            RLED_img(raw_data, screen, Img.Size, width, height);
            
            color_is_transparent[0] = 1;
            
            offset_x = Img.PlaceX;
            offset_y = Img.PlaceY;
        }

        // write it back out
        output_number = i;
        if (rv = write_image())
            return rv;
    }
    
    return 0;
}

#define FILE_STRATEGY(name) { #name, translate_ ## name }
struct {
    const char * name;
    int(*function)(FILE * fp);
} strategies[] = {
    { "320x240 RLE", translate_320x240_rle },
    FILE_STRATEGY(vab),
    FILE_STRATEGY(moon),
    FILE_STRATEGY(aprog),
    FILE_STRATEGY(lmer),
    FILE_STRATEGY(intel),
    FILE_STRATEGY(rdfull),
    FILE_STRATEGY(prfx),
    FILE_STRATEGY(lfacil),
    FILE_STRATEGY(presr),
    FILE_STRATEGY(port),
    FILE_STRATEGY(winner),
    FILE_STRATEGY(satbld),
    FILE_STRATEGY(budd),
    FILE_STRATEGY(endgame),
    FILE_STRATEGY(loser),
    FILE_STRATEGY(beggam),
    FILE_STRATEGY(patches),
};

#define STRATEGY_COUNT (sizeof(strategies) / sizeof(strategies[0]))

void print_usage()
{
    int i;
    
    fprintf(stderr, "usage: but2png <file.but> <strategy>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "strategies:\n");
    for (i = 0; i < STRATEGY_COUNT; i++) {
        fprintf(stderr, "  - %s\n", strategies[i].name);
    }
}

int main(int argc, char ** argv)
{
    int i;
    FILE * fp;
    
    memset(color_is_transparent, 0, sizeof(color_is_transparent));
    offset_x = offset_y = -1;
    
    if (argc != 3) {
        print_usage();
        return 1;
    }
    
    fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "unable to open %s: %s\n", argv[1], strerror(errno));
        return 2;
    }
    
    output_number = 0;
    snprintf(output_pattern, sizeof(output_pattern), "%s.%%i.png", argv[1]);
    
    for (i = 0; i < STRATEGY_COUNT; i++) {
        if (strcasecmp(strategies[i].name, argv[2]) == 0) {
            // match!
            return strategies[i].function(fp);
        }
    }
    
    fprintf(stderr, "unknown strategy: %s\n", argv[2]);
    print_usage();
    return 1;
}
