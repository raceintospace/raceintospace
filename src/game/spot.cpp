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

#include "display/graphics.h"

#include "data.h"
#include "endianness.h"
#include "fs.h"
#include "gamedata.h"
#include "macros.h"
#include "pace.h"
#include "sdlhelper.h"


/** SPOT structures and data structure variables */
struct {        // Main SPOT Header
    uint8_t ID[40];     /**< Copyright notice */
    uint8_t Qty;        /**< Number of Paths */
    uint32_t sOff;      /**< Spot Offsets */
    uint32_t pOff;      /**< Path Offsets */
} MSPOT;

struct sPATH {       // Spot Anim Path Struct
    uint16_t Image;        // Which image to Use
    int16_t xPut, yPut;    // Where to place this image
    int16_t iHold;         // Repeat this # times
    float Scale;           // Scale object
};

struct sIMG {
    uint8_t w, h;   // Width and Height
};

// TODO: Replace this...
char SUSPEND;

namespace
{
boost::shared_ptr<display::LegacySurface> portViewBuffer;
char PName[20];
int16_t sCount;     // sCount is the number of steps
FILE *sFin;
SimpleHdr hSPOT;  // Filled by Seek_sOff();
struct sPATH sPath, sPathOld;
struct sIMG sImg, sImgOld;
uint32_t pTable, pLoc;
};


size_t ImportSPath(FILE *fin, struct sPATH &target);
void Seek_sOff(int where);
void Seek_pOff(int where);


/**
 * Animates a Port background activity.
 *
 * The main spaceport has a variety of animated activities, some
 * triggered by player activities, that run in the background.
 * Examples include planes flying past & rockets being transported
 * to the main launch pad.
 *
 * This function handles animation activity based on the mode:
 *   - SPOT_LOAD begins an animation sequence, selecting it as the
 *     active sequence and starting any sound effects.
 *   - SPOT_STEP plays the next frame in the current animation
 *     sequence.
 *   - SPOT_DONE is used when the animation sequence has completed,
 *     stopping any active sound effects and cleaning up globals.
 *   - SPOT_KILL terminates any active sound effects and stops
 *     animation, closing access to the animation file, performing
 *     less cleanup than SPOT_DONE.
 *
 * The spots.cdr file is composed of:
 * SpotHeader
 * Image Headers
 * Images
 * Sequence Directory
 * <Unknown>
 * Animation[]
 *
 * The SpotHeader (MSPOT) contains offsets to an image headers list and
 * an animaton sequence directory, and the quantity of animation
 * sequences.
 *
 * The Image Headers list is a SimpleHdr array, accessed via MSPOT.sOff
 * and read into hSpot. These contain image size and an offset to the
 * image data.
 *  - Implementation: spots.cdr has space for 300 SimpleHdr structs
 *    reserved (1800 bytes); 0-282 have a SimpleHdr defined.
 *
 * Images consist of a two-byte sIMG header {width, height} followed by
 * raw palettized pixel data (coded to the Port palette). The SimpleHdr
 * size is the length of the pixel data, and does not include the sIMG
 * header.
 *
 * The animation sequence directory is a uint32_t array, accessed via
 * MSPOT.pOff and read into pTable. It contains offsets to the
 * animation sequences.
 *
 * Each animation sequence consists of a header:
 *   - char[20] containing the sequence name
 *   - uint16_t containing the count of sequence parts
 * followed by a series of sPath objects defining each sequence part.
 *
 * \param loc   which spot animation to load when mode=SPOT_LOAD
 * \param mode  SPOT_LOAD, SPOT_STEP, SPOT_DONE, or SPOT_KILL
 */
void SpotActivity(char loc, char mode)
{
    display::LegacySurface *SP1;
    display::LegacySurface *SP2;
    display::LegacySurface *SP3;
    static char turnoff = 0;

    if (SUSPEND == 1) {
        if (turnoff == 1) {
            StopVoice();    //Specs: suspend sound babypics
        }

        return;
    }

    if (sCount == -1  && mode != SPOT_LOAD) {
        return;
    }

    if (mode == SPOT_LOAD) {
        // Open File
        sFin = sOpen("SPOTS.CDR", "rb", FT_DATA);

        // Read in Spot Header
        // fread(&MSPOT, sizeof MSPOT, 1, sFin);
        fread(&MSPOT.ID[0], sizeof(MSPOT.ID), 1, sFin);
        fread(&MSPOT.Qty, sizeof(MSPOT.Qty), 1, sFin);
        fread(&MSPOT.sOff, sizeof(MSPOT.sOff), 1, sFin);
        fread(&MSPOT.pOff, sizeof(MSPOT.pOff), 1, sFin);
        Swap32bit(MSPOT.sOff);
        Swap32bit(MSPOT.pOff);

        Seek_pOff(loc);  // go to correct path
        fread(&PName, sizeof PName, 1, sFin);
        fread(&sCount, sizeof sCount, 1, sFin);  // get number of paths parts
        Swap16bit(sCount);
        pLoc = ftell(sFin);
        sPath.iHold = 1;
        portViewBuffer->copyFrom(display::graphics.legacyScreen(), 0, 0,
                                 display::graphics.screen()->width() - 1,
                                 display::graphics.screen()->height() - 1);
        sPathOld.xPut = -1;
        SpotActivity(0, SPOT_STEP);
        // All opened up
    } else if (mode == SPOT_STEP && sPath.iHold == 1 && sCount > 0) {
        // Play Next Seq
        int xx = 0;
        fseek(sFin, pLoc, SEEK_SET);     // position at next path
        // get the next sPATH struct
        ImportSPath(sFin, sPath);

        pLoc = ftell(sFin);               // Path Update Locations

        Seek_sOff(sPath.Image);          // point to next image
        // get image header
        // fread(&sImg, sizeof sImg, 1, sFin);
        fread(&sImg.w, sizeof(sImg.w), 1, sFin);
        fread(&sImg.h, sizeof(sImg.h), 1, sFin);

        SP1 = new display::LegacySurface(sImg.w, sImg.h);
        fread(SP1->pixels(), hSPOT.size, 1, sFin);  // read image data

        if (sPath.Scale != 1.0) {
            sImg.w = (int)((float) sImg.w * sPath.Scale);
            sImg.h = (int)((float) sImg.h * sPath.Scale);
            SP2 = new display::LegacySurface(sImg.w, sImg.h);
            SP1->scaleTo(SP2);
        }

        SP3 = new display::LegacySurface(sImg.w, sImg.h);

        portViewBuffer->palette().copy_from(
            display::graphics.legacyScreen()->palette());
        SP3->copyFrom(portViewBuffer.get(),
                      MIN(sPath.xPut, 319), MIN(sPath.yPut, 199),
                      MIN(sPath.xPut + sImg.w - 1, 319),
                      MIN(sPath.yPut + sImg.h - 1, 199),
                      0, 0);

        if (sPath.Scale != 1.0) {
            xx = hSPOT.size;

            for (int i = 0; i < xx; i++) {
                if (SP2->pixels()[i] == 0) {
                    *(SP2->pixels() + i) = SP3->pixels()[i];
                }
            }

            if (sPathOld.xPut != -1) {
                portViewBuffer->copyTo(display::graphics.legacyScreen(),
                                       sPathOld.xPut, sPathOld.yPut,
                                       sPathOld.xPut, sPathOld.yPut,
                                       sPathOld.xPut + sImgOld.w - 1,
                                       sPathOld.yPut + sImgOld.h - 1);
            }

            SP2->copyTo(display::graphics.legacyScreen(),
                        sPath.xPut, sPath.yPut);
        } else {
            xx = hSPOT.size;

            for (int i = 0; i < xx; i++) {
                if (SP1->pixels()[i] == 0) {
                    *(SP1->pixels() + i) = SP3->pixels()[i];
                }
            }

            if (sPathOld.xPut != -1) {
                portViewBuffer->copyTo(
                    display::graphics.legacyScreen(),
                    sPathOld.xPut, sPathOld.yPut,
                    sPathOld.xPut, sPathOld.yPut,
                    MIN(sPathOld.xPut + sImgOld.w - 1, 319),
                    MIN(sPathOld.yPut + sImgOld.h - 1, 199));
            }

            SP1->copyTo(display::graphics.legacyScreen(),
                        MIN(sPath.xPut, 319), MIN(sPath.yPut, 199));
        }

        sPathOld = sPath;
        sImgOld = sImg;

        delete SP3;
        SP3 = NULL;
        delete SP1;
        SP1 = NULL;

        if (sPath.Scale != 1.0) {
            delete SP2;
            SP2 = NULL;
        }

        sCount--;

    } else if (mode == SPOT_STEP && sPath.iHold > 1 && sCount > 0) {
        sPath.iHold--;
    } else if (mode == SPOT_STEP && sPath.iHold == 1 && sCount == 0) {
        SpotActivity(0, SPOT_DONE);
    } else if ((mode == SPOT_DONE || sCount >= 0) && sFin != NULL) {
        // Close the file and stop the audio.
        fclose(sFin);
        sFin = NULL;
        sPathOld.xPut = -1;
        sPath.iHold = 0;
        sCount = -1;
#if BABYSND

        if (turnoff == 1) {
            StopAudio(0);
            turnoff = 0;
        }

#endif
    } else if (mode == SPOT_KILL && sFin != NULL) {
        fclose(sFin);
        sFin = NULL;
#if BABYSND

        if (turnoff == 1) {
            StopAudio(0);
            turnoff = 0;
        }

#endif
    }

#if BABYSND

    if ((loc >= 0 && loc <= 8) || (loc >= 15 && loc <= 19) || loc == 12 || loc == 14 || loc == 11 || loc == 10) {
        if (mode == SPOT_LOAD && !IsChannelMute(AV_SOUND_CHANNEL)) {
            switch (loc) {
            case 1:
            case 6:
                PlayAudio("jet.ogg", 0);
                break;

            case 3:
            case 8:
                PlayAudio("vcrash.ogg", 0);
                break;

            case 16:
                PlayAudio("train.ogg", 0);
                break;

            case 4:
                PlayAudio("crawler.ogg", 0);
                break;

            case 0:
            case 5:
                PlayAudio("vthrust.ogg", 0);
                break;

            case 10:
                PlayAudio("gate.ogg", 0);
                break;

            case 18:
                PlayAudio("svprops.ogg", 0);
                break;

            case 2:
            case 7:
                PlayAudio("heli_00.ogg", 0);
                break;

            case 17:
                PlayAudio("radarsv.ogg", 0);
                break;

            case 11:
                PlayAudio("radarus.ogg", 0);
                break;

            case 12:
            case 14:
                PlayAudio("lightng.ogg", 0);
                break;

            case 19:
                PlayAudio("crane.ogg", 0);
                break;

            case 15:
                PlayAudio("truck.ogg", 0);
                break;
            }

            turnoff = 1;
        }
    }

#endif
    return;
}


/**
 * Free up resources after the Spaceport Animation is complete.
 */
void SpotClose()
{
    if (sFin) {
        fclose(sFin);
        sFin = NULL;
    }

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
    sCount = -1;
    SUSPEND = 0;

    int height = display::graphics.legacyScreen()->height();
    int width = display::graphics.legacyScreen()->width();

    portViewBuffer = boost::shared_ptr<display::LegacySurface>(
                         new display::LegacySurface(width, height));
    portViewBuffer->palette().copy_from(
        display::graphics.legacyScreen()->palette());
    portViewBuffer->draw(*display::graphics.screen(), 0, 0);
}


void SpotPause()
{
    SUSPEND = 1;
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
    SUSPEND = 0;
}


//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

/**
 * Read a sPATH struct stored as raw data in a file, correcting
 * for endianess.
 *
 * If import is not successful, the contents of the target sPATH
 * are not guaranteed.
 *
 * The format of the sPATH is:
 *   uint16_t Image;        // Which image to Use
 *   int16_t  xPut, yPut;   // Where to place this image
 *   int16_t iHold;         // Repeat this # times
 *   float Scale;       // Scale object
 *
 * \param fin  An open port data file at the start of the sPATH data.
 * \param target  The destination for the read data.
 * \return  1 if successfully read, 0 otherwise.
 */
size_t ImportSPath(FILE *fin, struct sPATH &target)
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
 * Seeks the point in the spot file
 *
 * Seeks in the global FILE sFin and modifies the global var pTable.
 *
 * \param where  the entry index in the animation table.
 */
void Seek_pOff(int where)
{
    fseek(sFin, where * (sizeof pTable) + (MSPOT.pOff), SEEK_SET);
    fread(&pTable, sizeof pTable, 1, sFin);
    Swap32bit(pTable);
    fseek(sFin, pTable, SEEK_SET);
}


/**
 * Seeks the point in the spot file where
 *
 * Seeks in the global FILE sFin and modifies the global var hSPOT.
 *
 * \param where  the entry index in the SimpleHdr table.
 */
void Seek_sOff(int where)
{
    fseek(sFin, where * sizeof_SimpleHdr + MSPOT.sOff, SEEK_SET);
    fread_SimpleHdr(&hSPOT, 1, sFin);
    fseek(sFin, hSPOT.offset, SEEK_SET);
}

