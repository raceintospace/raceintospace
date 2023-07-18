/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty
//

// This file handles the Spaceport Animations.


#include "spot.h"

#include <boost/shared_ptr.hpp>

#include <string>

#include "display/graphics.h"

#include "data.h"
#include "endianness.h"
#include "fs.h"
#include "gamedata.h"
#include "logging.h"
#include "macros.h"
#include "pace.h"
#include "sdlhelper.h"


/** SPOT structures and data structure variables */
struct SpotHeader {     // Main SPOT Header
    uint8_t ID[40];     /**< Copyright notice */
    uint8_t Qty;        /**< Number of Paths */
    uint32_t sOff;      /**< Spot Offsets */
    uint32_t pOff;      /**< Path Offsets */
};


// SimpleHdr is defined in gamedata.h and reprinted here for quick
// reference.
//
// typedef struct {
//     uint16_t size;
//     uint32_t offset;
// } SimpleHdr;


struct AnimationStep {       // Spot Anim Path Struct
    uint16_t Image;        // Which image to Use (by index)
    int16_t xPut, yPut;    // Where to place this image
    int16_t iHold;         // Repeat this # times
    float Scale;           // Scale object
};

struct CelHeader {
    uint8_t w, h;   // Width and Height
};

namespace
{
const int MAX_X = display::Graphics::WIDTH - 1;
const int MAX_Y = display::Graphics::HEIGHT - 1;

boost::shared_ptr<display::LegacySurface> portViewBuffer;
bool SUSPEND = true;
bool isTrackPlaying = false;
int16_t stepCount;     // stepCount is the number of steps
FILE *sFin = NULL;
struct SpotHeader mainHeader;
struct AnimationStep sPath;
struct CelHeader sImg;
uint32_t pLoc;

void AdvanceFrame();
#if BABYSND
std::string AudioTrack(int trackIndex);
#endif
size_t ImportSpotHeader(FILE *fin, struct SpotHeader &target);
size_t ImportSPath(FILE *fin, struct AnimationStep &target);
void SeekCelData(int celIndex);
void SeekAnimation(int index);
};


/**
 *  Plays the next frame in the currently active animation sequence.
 */
void SpotAdvance()
{
    if (SUSPEND) {
        return;
    }

    if (sPath.iHold == 1 && stepCount > 0) {
        AdvanceFrame();
        stepCount--;
    } else if (sPath.iHold > 1 && stepCount > 0) {
        sPath.iHold--;
    } else if (sPath.iHold == 1 && stepCount == 0) {
        SpotKill();  // Close the file and stop the audio.
    }
}


/**
 * Free up resources after the Spaceport Animation is complete.
 *
 * Once SpotClose is called, no calls to SpotActivity should be made
 * until SpotInit() is called.
 */
void SpotClose()
{
    SpotKill();
    portViewBuffer.reset();
}


/**
 * Initialize the Spaceport Animation system.
 *
 * This caches the current screen so it can update without having to
 * redraw the entire background. It must be called *after* the
 * background is drawn in DrawSpaceport().
 */
void SpotInit()
{
    sFin = NULL;
    stepCount = -1;
    SUSPEND = false;
    isTrackPlaying = false;

    int height = display::graphics.legacyScreen()->height();
    int width = display::graphics.legacyScreen()->width();

    portViewBuffer = boost::shared_ptr<display::LegacySurface>(
                         new display::LegacySurface(width, height));
    portViewBuffer->palette().copy_from(
        display::graphics.legacyScreen()->palette());
    portViewBuffer->draw(*display::graphics.screen(), 0, 0);
}


/**
 * Terminate any spaceport animation currently playing.
 *
 * Stops any active animation, any active sound effects, and closes
 * the animation file. An animation stopped with this function is
 * not paused. It may not be resumed and will have to be reloaded.
 *
 * Calling this should be safe when there is no animation loaded.
 */
void SpotKill()
{
    if (sFin) {
        fclose(sFin);
        sFin = NULL;
    }

    sPath.iHold = 0;
    sPath.xPut = -1;
    stepCount = -1;

#if BABYSND

    if (isTrackPlaying) {
        StopAudio(0);
        isTrackPlaying = false;
    }

#endif
}


/**
 * Animates a Port background activity.
 *
 * The main spaceport has a variety of animated activities, some
 * triggered by player activities, that run in the background.
 * Examples include planes flying past & rockets being transported
 * to the main launch pad.
 *
 * The spots.cdr file is composed of:
 *
 *     SpotHeader
 *     Image Headers
 *     Images
 *     Sequence Directory
 *     Animations
 *
 * The SpotHeader (mainHeader) contains file offsets to an image
 * headers list and an animaton sequence directory, and the quantity of
 * animation sequences.
 *
 * The Image Headers list is a SimpleHdr array, accessed via
 * mainHeader.sOff. These contain image size and an offset to the
 * cel data.
 *  - Implementation: spots.cdr has space for 300 SimpleHdr structs
 *    reserved (1800 bytes); 0-282 have a SimpleHdr defined.
 *
 * Images consist of raw palettized pixel data (coded to the Port
 * palette) preceded by a two-byte CelHeader header {width, height}.
 * The SimpleHdr size gives the length of the pixel data, not including
 * the CelHeader.
 *
 * The animation sequence directory is a uint32_t array, accessed via
 * mainHeader.pOff. It contains offsets to the animation sequences.
 *   - Implementation: spots.cdr has space reserved for 30 listings.
 *     0-20 have offsets defined, the remaining slots are noisy.
 *
 * Each animation sequence consists of a header:
 *   - char[20] containing the sequence name
 *   - uint16_t containing the count of sequence parts
 * followed by a series of AnimationStep(s) defining each sequence part.
 *
 * Each AnimationStep contains
 *    Image    - an index for which cel image to use
 *    xPut     - x coordinate for displaying the image
 *    yPut     - y coordinate for displaying the image
 *    iHold    - For timing, how many times to redraw this frame
 *    Scale    - A multiplier to scale the cel up or down
 *
 * \param animationIndex  Animation's index in spots.cdr
 */
void SpotLoad(int animationIndex)
{
    if (animationIndex == SPOT_NONE) {
        SpotKill();
        return;
    }

    if (sFin) {
        SpotKill();
    }

    // Open File
    sFin = sOpen("SPOTS.CDR", "rb", FT_DATA);

    // Read in Spot Header
    ImportSpotHeader(sFin, mainHeader);

    if (animationIndex < 0 || animationIndex >= mainHeader.Qty) {
        SpotKill();
        CERROR3(multimedia,
                "Cannot load spaceport animation %d: Invalid choice",
                animationIndex);
        return;
    }

    char pathName[20];
    SeekAnimation(animationIndex);  // go to correct path
    fread(&pathName, sizeof pathName, 1, sFin);    // Animation name
    fread(&stepCount, sizeof stepCount, 1, sFin);  // # of path parts
    Swap16bit(stepCount);
    pLoc = ftell(sFin);

    // Initialize some values because no animation step has been read.
    sPath.iHold = 1;
    sPath.xPut = -1;
    portViewBuffer->copyFrom(display::graphics.legacyScreen(), 0, 0,
                             display::graphics.screen()->width() - 1,
                             display::graphics.screen()->height() - 1);

#if BABYSND

    if (!IsChannelMute(AV_SOUND_CHANNEL)) {
        std::string track = AudioTrack(animationIndex);

        if (!track.empty()) {
            PlayAudio(track.c_str(), 0);
            isTrackPlaying = true;
        }
    }

#endif

    // All opened up
    SpotAdvance();
}


void SpotPause()
{
    SUSPEND = true;

#if BABYSND

    if (isTrackPlaying) {
        StopAudio(0);
        isTrackPlaying = false;
    }

#endif
}


void SpotRefresh()
{
    portViewBuffer->resetPalette();
    portViewBuffer->copyFrom(
        display::graphics.legacyScreen(),
        0, 0,
        display::graphics.screen()->width() - 1,
        display::graphics.screen()->height() - 1);
}


void SpotResume()
{
    SUSPEND = false;
}


//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

namespace
{

/**
 *
 */
void AdvanceFrame()
{
    bool firstStep = (sPath.xPut == -1);

    portViewBuffer->palette().copy_from(
        display::graphics.legacyScreen()->palette());

    // Draw over previous frame with cached Port display.
    if (! firstStep) {
        portViewBuffer->copyTo(
            display::graphics.legacyScreen(),
            sPath.xPut, sPath.yPut,
            sPath.xPut, sPath.yPut,
            MIN(sPath.xPut + sImg.w - 1, MAX_X),
            MIN(sPath.yPut + sImg.h - 1, MAX_Y));
    }

    fseek(sFin, pLoc, SEEK_SET);  // position at new path step
    ImportSPath(sFin, sPath);     // Read the AnimationStep
    pLoc = ftell(sFin);           // Save the next Path step's location

    // No point in creating expensive images if they won't be drawn.
    if (sPath.xPut > MAX_X || sPath.yPut > MAX_Y) {
        sPath.xPut = -1;  // Don't draw over this step next frame advance.
        return;
    }

    SeekCelData(sPath.Image);     // point to next image
    fread(&sImg.w, sizeof(sImg.w), 1, sFin);  // Get Cel dimensions
    fread(&sImg.h, sizeof(sImg.h), 1, sFin);

    // Read in the raw image data
    display::LegacySurface *celImage =
        new display::LegacySurface(sImg.w, sImg.h);
    fread(celImage->pixels(), sImg.w * sImg.h, 1, sFin);

    if (sPath.Scale != 1.0) {
        sImg.w = (int)((float) sImg.w * sPath.Scale);
        sImg.h = (int)((float) sImg.h * sPath.Scale);
        display::LegacySurface *scaledImage =
            new display::LegacySurface(sImg.w, sImg.h);
        celImage->scaleTo(scaledImage);

        delete celImage;
        celImage = scaledImage;
    }

    display::LegacySurface *frameBackground =
        new display::LegacySurface(sImg.w, sImg.h);

    frameBackground->copyFrom(portViewBuffer.get(),
                              sPath.xPut, sPath.yPut,
                              MIN(sPath.xPut + sImg.w - 1, MAX_X),
                              MIN(sPath.yPut + sImg.h - 1, MAX_Y),
                              0, 0);

    celImage->maskCopy(
        frameBackground, 0, display::LegacySurface::DestinationEqual, 0);
    celImage->copyTo(
        display::graphics.legacyScreen(), sPath.xPut, sPath.yPut);

    delete frameBackground;
    frameBackground = NULL;
    delete celImage;
    celImage = NULL;
}


#if BABYSND
std::string AudioTrack(int trackIndex)
{
    switch (trackIndex) {
    case USA_LM_TEST:
    case SOV_LM_TEST:
        return "vthrust.ogg";

    case USA_PLANE_FLY_BY:
    case SOV_PLANE_FLY_BY:
        return "jet.ogg";

    case USA_HELICOPTER:
    case SOV_HELICOPTER:
        return "heli_00.ogg";

    case USA_LM_CRASH:
    case SOV_LM_CRASH:
        return "vcrash.ogg";

    case USA_ROCKET_TO_PAD:
        return "crawler.ogg";

    case SOV_GATE:
        return "gate.ogg";

    case USA_TRACKING:
        return "radarus.ogg";

    case SOV_STORM_CLOUDS:
    case USA_STORM_CLOUDS:
        return "lightng.ogg";

    case USA_ROCKET_TO_VAB:
        return "truck.ogg";

    case SOV_ROCKET_TO_PAD:
        return "train.ogg";

    case SOV_TRACKING:
        return "radarsv.ogg";

    case SOV_NEW_PLANE:
        return "svprops.ogg";

    case USA_ROTATING_CRANE:
        return "crane.ogg";

    default:
        CNOTICE3(multimedia,
                 "No entry for Spaceport Animation soundtrack index %d",
                 trackIndex);
        break;
    }

    return "";
}
#endif


/**
 * Read in the main header of a Spot animation file, correcting for
 * endianness.
 *
 * If import is not successful, the contents of the target
 * are not guaranteed.
 *
 * \param fin  An open data file at the start of the SpotHeader data.
 * \param target  The destination for the read data.
 * \return  1 if successfully read, 0 otherwise.
 */
size_t ImportSpotHeader(FILE *fin, struct SpotHeader &target)
{
    bool read =
        fread(&target.ID[0], sizeof(target.ID),   1, sFin) &&
        fread(&target.Qty,   sizeof(target.Qty),  1, sFin) &&
        fread(&target.sOff,  sizeof(target.sOff), 1, sFin) &&
        fread(&target.pOff,  sizeof(target.pOff), 1, sFin);

    if (read) {
        Swap32bit(target.sOff);
        Swap32bit(target.pOff);
    }

    return read ? 1 : 0;
}


/**
 * Read a AnimationStep struct stored as raw data in a file, correcting
 * for endianess.
 *
 * If import is not successful, the contents of the target AnimationStep
 * are not guaranteed.
 *
 * The format of the AnimationStep is:
 *   uint16_t Image;        // Which image to Use
 *   int16_t  xPut, yPut;   // Where to place this image
 *   int16_t iHold;         // Repeat this # times
 *   float Scale;       // Scale object
 *
 * \param fin  An open port data file at the start of the AnimationStep data.
 * \param target  The destination for the read data.
 * \return  1 if successfully read, 0 otherwise.
 */
size_t ImportSPath(FILE *fin, struct AnimationStep &target)
{
    // Chain freads so they stop if one fails...
    bool read =
        fread(&target.Image, sizeof(target.Image), 1, fin) &&
        fread(&target.xPut, sizeof(target.xPut), 1, fin) &&
        fread(&target.yPut, sizeof(target.yPut), 1, fin) &&
        fread(&target.iHold, sizeof(target.iHold), 1, fin) &&
        fread(&target.Scale, sizeof(target.Scale), 1, fin);

    if (read) {
        Swap16bit(target.Image);
        Swap16bit(target.xPut);
        Swap16bit(target.yPut);
        Swap16bit(target.iHold);
        Swap32bit(target.Scale);
    }

    return (read ? 1 : 0);
}


/**
 * Seeks the beginning of the animation sequence.
 *
 * There is an animation directory listing the location in the file of
 * each animation. The file's main header lists how many animations
 * exist (SpotHeader.Qty) and where the directory is (SpotHeader.pOff).
 * Each listing is a 32-bit offset value of where to find an animation
 * header.
 *
 * Modifies the global sFin file.
 *
 * \param index  the animation's index in the animation table.
 */
void SeekAnimation(int index)
{
    uint32_t animOffset;
    fseek(sFin, index * (sizeof animOffset) + (mainHeader.pOff), SEEK_SET);
    fread(&animOffset, sizeof animOffset, 1, sFin);
    Swap32bit(animOffset);
    fseek(sFin, animOffset, SEEK_SET);
}


/**
 * Seeks to the beginning of a cel image's dimension header in the
 * currently opened animation file.
 *
 * For a given index, finds the entry for the matching animation cel
 * at the start of the dimension data (uint8_t width, uint8_t height)
 * right before the raw pixel data.
 *
 * Requires the global FILE sFin be open and mainHeader already read.
 *
 * \param celIndex  the entry index in the SimpleHdr table.
 */
void SeekCelData(int celIndex)
{
    SimpleHdr imageListing;
    fseek(sFin, celIndex * sizeof_SimpleHdr + mainHeader.sOff, SEEK_SET);
    fread_SimpleHdr(&imageListing, 1, sFin);
    fseek(sFin, imageListing.offset, SEEK_SET);
}

};  // End of anonymous namespace
