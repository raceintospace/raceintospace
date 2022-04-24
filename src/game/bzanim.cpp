#include "bzanim.h"

#include <cassert>
#include <stdexcept>

#include "display/graphics.h"
#include "display/palette.h"

#include "data.h"
#include "endianness.h"
#include "fs.h"
#include "ioexception.h"
#include "logging.h"
#include "pace.h"


LOG_DEFAULT_CATEGORY(video)


namespace
{
size_t ImportAnimType(FILE *fin, struct AnimType &target);
size_t ImportBlockHead(FILE *fin, struct BlockHead &target);
uint8_t *ReadFrame(FILE *fin, int width, int height);
display::Palette ReadPalette(FILE *fin, int offset, int colors);
void SeekAnimation(FILE *fin, const char *name);
};


/**
 * Load the specified animation resource.
 *
 * This method automatically exports the animation's palette to the
 * global display.
 *
 * \param file  the animation file.
 * \param id    the animation ID.
 * \param x  the top-left x coordinate of the animation window.
 * \param y  the top-left y coordinate of the animation window.
 * \throw IOException  if an error occurs while reading the file.
 */
BZAnimation::Ptr BZAnimation::load(
    const char *file, const char *id, int x, int y)
{
    if (file == NULL) {
        throw std::invalid_argument("Parameter file may not be null.");
    }

    if (id == NULL) {
        throw std::invalid_argument("Parameter id may not be null.");
    }

    FILE *fin = open_gamedat(file);

    if (!fin) {
        std::string msg("Cannot open file ");
        msg += file;
        throw IOException(msg);
    }

    struct AnimType AHead;
    std::vector<uint8_t *> frames;

    SeekAnimation(fin, id);
    ImportAnimType(fin, AHead);
    display::Palette palette = ReadPalette(fin, AHead.cOff, AHead.cNum);

    for (int i = 0; i < AHead.fNum; i++) {
        uint8_t *pixels = ReadFrame(fin, AHead.w, AHead.h);
        frames.push_back(pixels);
    }

    BZAnimation::Ptr animation(
        new BZAnimation(AHead, palette, frames, x, y));

    return animation;
}


/**
 *
 * \param x  the top-left x coordinate of the animation window.
 * \param y  the top-left y coordinate of the animation window.
 */
BZAnimation::BZAnimation(struct AnimType header,
                         display::Palette palette,
                         std::vector<uint8_t *> frames,
                         int x,
                         int y)
    : dply(NULL), AHead(header), mFrameData(frames)
{
    if (x < 0 || x >= display::Graphics::WIDTH) {
        WARNING2("Animation param x=%d out of range.", x);
    }

    if (y < 0 || y >= display::Graphics::HEIGHT) {
        WARNING2("Animation param y=%d out of range.", y);
    }

    mX = x;
    mY = y;

    dply = new display::LegacySurface(AHead.w, AHead.h);
    dply->palette().copy_from(display::graphics.legacyScreen()->palette());

    dply->palette().copy_from(
        palette, AHead.cOff, AHead.cOff + AHead.cNum - 1);
    display::graphics.legacyScreen()->palette().copy_from(dply->palette());

    tFrames = AHead.fNum;
    cFrame = 0;
}


/**
 *
 *
 */
BZAnimation::~BZAnimation()
{
    if (dply) {
        delete dply;
    }

    for (int i = 0; i < mFrameData.size(); i++) {
        delete[] mFrameData[i];
    }

    // if (fin) {
    //     fclose(fin);
    // }
}


/**
 * Advance the animation to the next frame and display it.
 */
void BZAnimation::advance()
{
    if (cFrame == tFrames) {
        cFrame = 0;
    }

    if (cFrame < tFrames) {
        uint8_t *pixels = mFrameData[cFrame];
        memcpy(dply->pixels(), pixels, dply->width() * dply->height());

        // dply->palette().copy_from(display::graphics.legacyScreen()->palette());
        display::graphics.screen()->draw(*dply, mX, mY);
        cFrame++;
    }

    // TODO: Add duration method(s) to give access to time length,
    // time remaining information.
    // return (tFrames - cFrame); //remaining frames
}


//----------------------------------------------------------------------

namespace {

/**
 * Read an AnimType struct stored in a file as raw data.
 *
 * \param fin  Pointer to a FILE object that specifies an input stream.
 * \param target  The destination for the read data.
 * \return  1 if successful, 0 otherwise.
 */
size_t ImportAnimType(FILE *fin, struct AnimType &target)
{
    bool success =
        fread(&target.ID[0], sizeof(target.ID), 1, fin) &&
        fread(&target.OVL[0], sizeof(target.OVL), 1, fin) &&
        fread(&target.SD[0][0], sizeof(target.SD), 1, fin) &&
        fread(&target.w, sizeof(target.w), 1, fin) &&
        fread(&target.h, sizeof(target.h), 1, fin) &&
        fread(&target.sPlay[0], sizeof(target.sPlay), 1, fin) &&
        fread(&target.fNum, sizeof(target.fNum), 1, fin) &&
        fread(&target.fLoop, sizeof(target.fLoop), 1, fin) &&
        fread(&target.cOff, sizeof(target.cOff), 1, fin) &&
        fread(&target.cNum, sizeof(target.cNum), 1, fin);

    Swap16bit(target.w);
    Swap16bit(target.h);
    return (success ? 1 : 0);
}


/**
 * Read a BlockHead struct stored in a file as raw data.
 *
 * A BlockHead is a header in an animation file, at the beginning of
 * an animation frame. It contains a value for identifying the
 * compression, and the size (in bytes) of the animation pixel data
 * that follows.
 *
 * \param fin  Pointer to a FILE object that specifies an input stream.
 * \param target  The destination for the read data
 * \return  1 if successful, 0 otherwise
 */
size_t ImportBlockHead(FILE *fin, struct BlockHead &target)
{
    bool success =
        fread(&target.cType, sizeof(target.cType), 1, fin) &&
        fread(&target.fSize, sizeof(target.fSize), 1, fin);

    Swap32bit(target.fSize);
    return (success ? 1 : 0);
}


/**
 * Reads and decodes the raw pixel data for a frame.
 *
 * The returned pixel data is an array of length (width * height),
 * containing codes corresponding ti a 256-color palette. The relevant
 * color space of that palette is found following the Animation Header.
 *
 * \param fin     File positioned at the start of the frame.
 * \param width   The frame width (in pixels).
 * \param height  The frame height (in pixels).
 * \return  uncompressed pixel data.
 */
uint8_t *ReadFrame(FILE *fin, int width, int height)
{
    assert(fin);

    struct BlockHead header;
    ImportBlockHead(fin, header);

    assert(header.fSize < 128 * 1024);
    uint8_t *buf = (uint8_t *)alloca(header.fSize);
    fread(buf, header.fSize, 1, fin);

    uint8_t *frame = new uint8_t[width * height];

    // TODO: Create an enum for the different compression codes.
    // TODO: What makes codes 1 & 2 different?  - rnyoakum
    switch (header.cType) {
    case 0:
        memcpy(frame, buf, header.fSize);
        break;

    case 1:
        RLED_img((char *)buf, (char *)frame, header.fSize, width, height);
        break;

    case 2:
        RLED_img((char *)buf, (char *)frame, header.fSize, width, height);
        break;

    default:
        break;
    }

    frame[width * height - 1] = frame[width * height - 2];

    return frame;
}


/**
 * Reads the raw palette data from a file.
 *
 * \param fin  File positioned at the start of the animation palette data.
 * \param offset  The start of the color space.
 * \param colors  The number of colors in the color space.
 * \return  a palette with colors defined for [offset, offset + colors).
 * \throws  IOException  if palette data could not be read.
 */
display::Palette ReadPalette(FILE *fin, int offset, int colors)
{
    assert(fin);
    display::Palette palette;

    {
        display::AutoPal p(palette);

        if (! fread(&p.pal[offset * 3], colors * 3, 1, fin)) {
            throw IOException("Could not read palette data.");
        }
    }

    return palette;
}


/**
 * Sets the file position to the start of the animation header.
 *
 * After calling the function, the file header will be positioned
 * to read the AnimType data for the specified animation. If the
 * named animation isn't found in the animation index, behavior
 * is undefined.
 *
 * \param fin   An open .abz animation file.
 * \param name  The 4-character animation identifier.
 */
void SeekAnimation(FILE *fin, const char *name)
{
    struct BZFileHeader {
        char ID[4];
        int32_t offset;
        int32_t size;
    } indexEntry;

    assert(fin);
    fseek(fin, 0, SEEK_SET);

    // TODO: Add a check to make sure fname is found in file, else
    // this becomes an infinite loop.
    do {
        fread(&indexEntry.ID[0], sizeof(indexEntry.ID), 1, fin);
        fread(&indexEntry.offset, sizeof(indexEntry.offset), 1, fin);
        fread(&indexEntry.size, sizeof(indexEntry.size), 1, fin);
    } while (strncmp(indexEntry.ID, name, 4) != 0);

    Swap32bit(indexEntry.offset);
    Swap32bit(indexEntry.size);
    fseek(fin, indexEntry.offset, SEEK_SET);
}

};  // End of namespace
