#include "bzanim.h"

#include <cassert>
#include <stdexcept>

#include "display/graphics.h"

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
};


/**
 * Load the specified animation resource.
 *
 * This method automatically exports the animation's palette to the
 * global display.
 *
 * TODO: This basically uses openAnim as an initialization function.
 * That is done to clearly show how code is being moved and rewritten
 * as the class is created, as a step-by-step process rather than a
 * sudden dump of entirely new code. It will be changed.
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

    BZAnimation::Ptr animation( new BZAnimation(x, y) );
    animation->fin = fin;
    animation->openAnim(id);

    return animation;
}


/**
 *
 * \param x  the top-left x coordinate of the animation window.
 * \param y  the top-left y coordinate of the animation window.
 */
BZAnimation::BZAnimation(int x, int y)
    : dply(NULL), fin(NULL)
{
    if (x < 0 || x >= display::Graphics::WIDTH) {
        WARNING2("Animation param x=%d out of range.", x);
    }

    if (y < 0 || y >= display::Graphics::HEIGHT) {
        WARNING2("Animation param y=%d out of range.", y);
    }

    mX = x;
    mY = y;
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

    if (fin) {
        fclose(fin);
    }
}


/**
 * Advance the animation to the next frame and display it.
 */
void BZAnimation::advance()
{
    stepAnim(mX, mY);
}


/**
 * Open the animations file and seek the proper animation.
 */
void BZAnimation::openAnim(const char *fname)
{
    struct TM {
        char ID[4];
        int32_t offset;
        int32_t size;
    } AIndex;

    assert(fin);

    // TODO: Add a check to make sure fname is found in file, else
    // this becomes an infinite loop.
    do {
        // fread(&AIndex, sizeof AIndex, 1, fin);
        fread(&AIndex.ID[0], sizeof(AIndex.ID), 1, fin);
        fread(&AIndex.offset, sizeof(AIndex.offset), 1, fin);
        fread(&AIndex.size, sizeof(AIndex.size), 1, fin);
    } while (strncmp(AIndex.ID, fname, 4) != 0);

    Swap32bit(AIndex.offset);
    Swap32bit(AIndex.size);
    fseek(fin, AIndex.offset, SEEK_SET);
    ImportAnimType(fin, AHead);

    dply = new display::LegacySurface(AHead.w, AHead.h);
    dply->palette().copy_from(display::graphics.legacyScreen()->palette());
    {
        display::AutoPal p(dply);
        fread(&p.pal[AHead.cOff * 3], AHead.cNum * 3, 1, fin);
    }
    aLoc = ftell(fin);
    tFrames = AHead.fNum;
    cFrame = 0;

    display::graphics.legacyScreen()->palette().copy_from(dply->palette());
}


/**
 * Advance to the next frame of the animation and display it.
 *
 * \param x  the top-left x coordinate of the animation window.
 * \param y  the top-left y coordinate of the animation window.
 * \return  the count of remaining frames.
 */
int BZAnimation::stepAnim(int x, int y)
{
    if (cFrame == tFrames) {
        fseek(fin, aLoc, SEEK_SET);
        cFrame = 0;
    }

    if (cFrame < tFrames) {
        ImportBlockHead(fin, BHead);

        assert(BHead.fSize < 128 * 1024);
        char *buf = (char *)alloca(BHead.fSize);
        fread(buf, BHead.fSize, 1, fin);

        switch (BHead.cType) {
        case 0:
            memcpy(dply->pixels(), buf, BHead.fSize);
            break;

        case 1:
            RLED_img(buf, dply->pixels(), BHead.fSize, dply->width(), dply->height());
            break;

        case 2:
            RLED_img(buf, dply->pixels(), BHead.fSize, dply->width(), dply->height());
            break;

        default:
            break;
        }

        dply->pixels()[AHead.w * AHead.h - 1] = dply->pixels()[AHead.w * AHead.h - 2];
        dply->palette().copy_from(display::graphics.legacyScreen()->palette());
        display::graphics.screen()->draw(*dply, x, y);
        cFrame++;
    }

    return (tFrames - cFrame); //remaining frames
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

};
