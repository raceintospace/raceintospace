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

// This file handles the main Spaceport screen, including animations

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "port.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "utils.h"
#include "admin.h"
#include "ast0.h"
#include "ast2.h"
#include "ast3.h"
#include "ast4.h"
#include "budget.h"
#include "intel.h"
#include "game_main.h"
#include "museum.h"
#include "place.h"
#include "radar.h"
#include "rdplex.h"
#include "review.h"
#include "rush.h"
#include "sdlhelper.h"
#include "vab.h"
#include "mc.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "filesystem.h"

#include <stdio.h>
#include <boost/shared_ptr.hpp>

#define LET_A   0x09
#define LET_M   0x0A
#define LET_O   0x0B
#define LET_V   0x0C
#define LET_R   0x08

#define SPOT_ON 1 /**< turn off until everything else works - pace */
#define BABYSND 1
#define pNOREDRAW 0
#define pREDRAW 1
#define pEXIT 2
#define pQUIT 3
#define pNOFADE 4
#define pNEWMUSIC 5

int put_serial(unsigned char n);

char RUSH, SUSPEND;

typedef struct portoutlinerestore {
    uint16_t loc;
    char val;
} PORTOUTLINE;

PORTOUTLINE *pPortOutlineRestore;

struct PortHeader {
    char Text[28];  /**< File Copyright Notice */
    int32_t oMObj;     /**< Offset to MObj data table */
    int32_t oTab;      /**< Offset to Table of data */
    int32_t oPal;      /**< Offset to Palette */
    int32_t oPort;     /**< Offset to start of Port Images */
    int32_t oMse;      /**< Offset to Mouse Objects */
    int32_t oOut;      /**< Offset to port Outlines */
    int32_t oAnim;     /**< Offset to start of Port Anims */
} PHead;

typedef struct cBoxx {
    int16_t x1, y1, x2, y2;
} BOUND;

typedef struct Img {
    int32_t Size;         /**<  Size of Image (bytes) */
    char Comp;         /**<  Type of Compression Used */
    int16_t Width;         /**<  Width of Image */
    int16_t Height;        /**<  Height of Image */
    int16_t PlaceX;        /**<  Where to Place Img:X */
    int16_t PlaceY;        /**<  Where to Place Img:Y */
} IMG;

typedef struct region {
    char qty;          /**< number of BOUNDS */
    BOUND CD[4];       /**< boundry coords for mouse location */
    char iNum;
    char sNum;         /**< value for surround box */
    char PreDraw;      /**< Code of Special to Draw first */
} REGION;


typedef struct mobj {
    char Name[30];       /**< Name of region */
    char qty;            /**< Number of regions */
    char Help[3];        /**< Help Box Stuff */
    REGION Reg[4];       /**< At Max 4 regions */
} MOBJ;

#define S_QTY 43

MOBJ MObj[35];

/** These are the valid hotkeys */
char HotKeyList[] = "AIMRPVCQETB\0";

int FCtr;
boost::shared_ptr<display::PalettizedSurface> flaggy;

/** SPOT structures and data structure variables */
struct {        // Main SPOT Header
    uint8_t ID[40];     /**< Copyright notice */
    uint8_t Qty;        /**< Number of Paths */
    uint32_t sOff;      /**< Spot Offsets */
    uint32_t pOff;      /**< Path Offsets */
} MSPOT;

struct sPATH {       // Spot Anim Path Struct
    uint16_t Image;        // Which image to Use
    int16_t  xPut, yPut;   // Where to place this image
    int16_t iHold;         // Repeat this # times
    float Scale;       // Scale object
};

struct sIMG {
    uint8_t w, h;   // Width and Height
};



int16_t sCount;     // sCount is the number of steps
int16_t Vab_Spot;
FILE *sFin;
SimpleHdr hSPOT;  // Filled by Seek_sOff();
struct sPATH sPath, sPathOld;
struct sIMG sImg, sImgOld;
uint32_t pTable, pLoc;

// Unnamed namespace for local globals & function prototypes.
// TODO: Move other file variables here.
namespace
{
boost::shared_ptr<display::LegacySurface> portViewBuffer;
};


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


char PName[20];

#define SPOT_LOAD 0
#define SPOT_STEP 1
#define SPOT_DONE 2
#define SPOT_KILL 3


void Seek_sOff(int where);
void Seek_pOff(int where);
void SpotCrap(char loc, char mode);
void WaveFlagSetup(void);
void WaveFlagDel(void);
void PortPlace(FILE *fin, int32_t table);
void PortText(int x, int y, char *txt, char col);
void UpdatePortOverlays(void);
void DoCycle(void);
void PortOutLine(unsigned int Count, uint16_t *buf, char mode);
void PortRestore(unsigned int Count);
int MapKey(char plr, int key, int old) ;
void Port(char plr);
char PortSel(char plr, char loc);
char Request(char plr, char *s, char md);
size_t ImportPortHeader(FILE *fin, struct PortHeader &target);
size_t ImportMOBJ(FILE *fin, MOBJ &target);
size_t ImportSPath(FILE *fin, struct sPATH &target);


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
void SpotCrap(char loc, char mode)
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
        sFin = sOpen("SPOTS.CDR", "rb", 0);

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
        SpotCrap(0, SPOT_STEP);
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

        {
            int expected_w = hSPOT.size / sImg.h;

            /* DEBUG - FIXING sImg.w */
            if (sImg.w != expected_w) {
                sImg.w = expected_w;
            }
        }

        // TODO: This makes the previous block obsolete
        sImg.w = hSPOT.size / sImg.h;
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
        SpotCrap(0, SPOT_DONE);
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

void WaveFlagSetup(char plr)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "images/flag.seq.%d.png", plr);
    flaggy = boost::shared_ptr<display::PalettizedSurface>(Filesystem::readImage(filename));
}

void WaveFlagDel(void)
{
    flaggy.reset();
}


/**
 * Display a port building/location image.
 *
 * Image composition is:
 *     int32_t Size      -- Size of Image (bytes)
 *     char Comp         -- Type of Compression Used
 *     int16_t Width     -- Width of Image
 *     int16_t Height    -- Height of Image
 *     int16_t PlaceX    -- Where to Place Img:X
 *     int16_t PlaceY    -- Where to Place Img:Y
 *
 * \param fin    an open (usa/sov)_port.dat file.
 * \param table  offset to the image data in the Port file.
 */
void PortPlace(FILE *fin, int32_t table)
{
    IMG Img;
    // const size_t sizeof_IMG = 4 + 1 + 2 + 2 + 2 + 2;

    fseek(fin, table, SEEK_SET);
    //fread(&Img, sizeof Img, 1, fin);
    fread(&Img.Size, sizeof(Img.Size), 1, fin);
    fread(&Img.Comp, sizeof(Img.Comp), 1, fin);
    fread(&Img.Width, sizeof(Img.Width), 1, fin);
    fread(&Img.Height, sizeof(Img.Height), 1, fin);
    fread(&Img.PlaceX, sizeof(Img.PlaceX), 1, fin);
    fread(&Img.PlaceY, sizeof(Img.PlaceY), 1, fin);
    Swap32bit(Img.Size);
    Swap16bit(Img.Width);
    Swap16bit(Img.Height);
    Swap16bit(Img.PlaceX);
    Swap16bit(Img.PlaceY);

    display::LegacySurface local(Img.Width, Img.Height);

    // read the compressed image data into a buffer, decompress, and
    // write the pixel data into an appropriately sized image.
    char *buf = (char *)alloca(Img.Size);
    fread(buf, Img.Size, 1, fin);
    RLED_img(buf, local.pixels(), Img.Size, local.width(), local.height());
    local.palette().copy_from(display::graphics.legacyScreen()->palette());
    local.setTransparentColor(0);

    // draw it
    display::graphics.screen()->draw(local, Img.PlaceX, Img.PlaceY);
}


/**
 * Loads the port palette into the global display palette, overwriting
 * the current palette.
 *
 * The Port palette is a 256-color palette, differing slightly per side.
 * The first 27 colors, however, are the same.
 *
 * \param plr  The player palette to use (0 for USA, 1 for USSR)
 */
void PortPal(char plr)
{
    FILE *fin;
    fin = sOpen((plr == 0) ? "USA_PORT.DAT" : "SOV_PORT.DAT", "rb", 0);
    // fread(&PHead, sizeof PHead, 1, fin);
    // TODO: Add in some error checking...
    ImportPortHeader(fin, PHead);

    fseek(fin, PHead.oPal, SEEK_SET);
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(p.pal, 768, 1, fin);
    }
    fclose(fin);
    return;
}


void DrawSpaceport(char plr)
{
    int32_t table[S_QTY];
    FILE *fin;

    fin = sOpen((plr == 0) ? "USA_PORT.DAT" : "SOV_PORT.DAT", "rb", 0);

    // TODO: Add in some error checking...
    ImportPortHeader(fin, PHead);

    for (int i = 0; i < (int)(sizeof(MObj) / sizeof(MOBJ)); i++) {
        ImportMOBJ(fin, MObj[i]);
    }

    fread(&table[0], sizeof table, 1, fin);

    // Endianness swap
    for (int i = 0; i < S_QTY; i++) {
        Swap32bit(table[i]);
    }

    // Draw the main port image
    {
        const char *filename =
            (plr == 0 ? "images/usa_port.dat.0.png" :
             "images/sov_port.dat.0.png");
        boost::shared_ptr<display::PalettizedSurface> image(
            Filesystem::readImage(filename));
        image->exportPalette();
        display::graphics.screen()->draw(image, 0, 0);
    }

    UpdatePortOverlays();

    if (xMODE & xMODE_CLOUDS) {
        PortPlace(fin, table[1]);    // Clouds
    }

    // Pads
    for (int i = 0; i < 3; i++) {
        Data->P[plr].Port[PORT_LaunchPad_A + i] = 1;  // Draw launch pad

        if (Data->P[plr].Mission[i].MissionCode) {
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 2;  // Draw damaged launch pad
        } else if (Data->P[plr].LaunchFacility[i] > 1) {
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 3;
        } else if (Data->P[plr].LaunchFacility[i] < 0) {  // No launch facility
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 0;
        }
    }

    if (Vab_Spot == 1 && Data->P[plr].Port[PORT_VAB] == 2) {
        Data->P[plr].Port[PORT_LaunchPad_A] = plr;
    }


    if (Data->P[plr].AstroCount > 0) {
        PortPlace(fin, table[16 - plr * 4]);  // Draw CPX
        HotKeyList[9] = 'T';
        HotKeyList[10] = 'B';
    } else {    // No manned program hotkeys
        HotKeyList[9] = '\0';
        HotKeyList[10] = '\0';
    }

    if (Data->P[plr].Pool[0].Active >= 1) {
        PortPlace(fin, table[17 - plr * 4]);    // Draw TRN
    }

    if (Data->P[plr].Port[PORT_Research] > 1) {
        PortPlace(fin, table[13 + 15 * plr]);    // RD Stuff
    }

    if (Data->P[plr].Port[PORT_Research] > 2) {
        PortPlace(fin, table[14 + 15 * plr]);
    }

    if (Data->P[plr].Port[PORT_Research] == 3) {
        PortPlace(fin, table[15 + 15 * plr]);
    }

    for (int fm = 0; fm < 35; fm++) {
        int idx = Data->P[plr].Port[fm];  // Current Port Level for MObj

        if (MObj[fm].Reg[idx].PreDraw > 0) {  // PreDrawn Shape
            PortPlace(fin, table[MObj[fm].Reg[idx].PreDraw]);
        }

        if (MObj[fm].Reg[idx].iNum > 0) {  // Actual Shape
            PortPlace(fin, table[MObj[fm].Reg[idx].iNum]);
        }
    }

    fclose(fin);

    ShBox(0, 190, 319, 199);            // Base Box :: larger

    display::graphics.setForegroundColor(0);
    draw_string(257, 197, "CASH:");
    draw_megabucks(285, 197, Data->P[plr].Cash);
    display::graphics.setForegroundColor(11);
    draw_string(256, 196, "CASH:");
    draw_megabucks(284, 196, Data->P[plr].Cash);

    display::graphics.setForegroundColor(0);

    if (Data->Season == 0) {
        draw_string(166, 197, "SPRING 19");
    } else {
        draw_string(166, 197, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        draw_string(165, 196, "SPRING 19");
    } else {
        draw_string(165, 196, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    if (plr == 0) {
        display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 49, 121);
    } else {
        display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 220, 141);
    }
}

void PortText(int x, int y, char *txt, char col)
{
    fill_rectangle(1, 192, 160, 198, 3);
    display::graphics.setForegroundColor(0);
    draw_string(x + 1, y + 1, txt);
    display::graphics.setForegroundColor(col);
    draw_string(x, y, txt);
}


void UpdatePortOverlays(void)
{
    char i, j;

    for (i = 0; i < NUM_PLAYERS; i++) {  // Programs
        for (j = 0; j < 5; j++) {
            Data->P[i].Port[PORT_Mercury - j] = (Data->P[i].Manned[j].Num >= 0) ? 1 : 0;
        }

#ifdef DEADCODE
        // Zond thingy -- this was never implemented and was available after 6 manned seasons
        //if (i==1 && Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].Seas>6) Data->P[i].Port[PORT_Zond]=1;
#endif

        if (Data->P[i].Probe[PROBE_HW_ORBITAL].Num >= 0 || Data->P[i].Probe[PROBE_HW_INTERPLANETARY].Num >= 0 ||
            Data->P[i].Probe[PROBE_HW_LUNAR].Num >= 0) {
            Data->P[i].Port[PORT_Satellite] = 1;
        }

        if (Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].Num >= 0 || Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].Num >= 0) {
            Data->P[i].Port[PORT_LM] = 1;
        }

        // Museum
        if (Data->Prestige[Prestige_MannedOrbital].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 1);    // Mus:1
        }

        if (Data->Prestige[Prestige_LunarFlyby].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 2);    // Mus:2
        }

        if (Data->Prestige[Prestige_MannedLunarOrbit].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 3);    // Mus:3
        }

        // R&D
        if (Data->P[i].Budget >= 85) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 1);    // RD:1
        }

        if (Data->P[i].Budget >= 112) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 2);    // RD:2
        }

        if (Data->P[i].Budget >= 150) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 3);    // RD:3
        }

        // VAB

        if (Data->Prestige[Prestige_Duration_B].Goal[i] > 0) {
            Data->P[i].Port[PORT_VAB] = MAX(Data->P[i].Port[PORT_VAB], 1);    // VAB:1
        }

        if (Data->P[i].Budget > 115) {
            Data->P[i].Port[PORT_VAB] = MAX(Data->P[i].Port[PORT_VAB], 2);    // VAB:2
        }

        // Admin
        if (Data->P[i].AstroLevel >= 2) {
            Data->P[i].Port[PORT_Admin] = MAX(Data->P[i].Port[PORT_Admin], 1);    // Adm:1
        }

        if (Data->P[i].AstroLevel >= 4) {
            Data->P[i].Port[PORT_Admin] = MAX(Data->P[i].Port[PORT_Admin], 2);    // Adm:2
        }

        if (Data->Prestige[Prestige_OnePerson].Goal[i] > 0) {
            Data->P[i].Port[PORT_Tracking] = MAX(Data->P[i].Port[PORT_Tracking], 1);    // Trk:1
        }

        if (Data->Prestige[Prestige_MannedLunarPass].Goal[i] > 0) {
            Data->P[i].Port[PORT_MissionControl] = MAX(Data->P[i].Port[PORT_MissionControl], 1);    // MC:1
        }

        if (Data->P[i].AstroCount > 0) {
            Data->P[i].Port[PORT_AstroComplex] = Data->P[i].Port[PORT_BasicTraining] = 1;
        }

        if (Data->P[i].Pool[0].Active > 0) {     // Astros
            Data->P[i].Port[PORT_Helipad] = Data->P[i].Port[PORT_Pool] = Data->P[i].Port[PORT_Planetarium] = 1;
            Data->P[i].Port[PORT_Centrifuge] = Data->P[i].Port[PORT_MedicalCtr] = Data->P[i].Port[PORT_Airfield] = 1;
        }
    }
}

void Master(char plr)
{
    int i, r_value, t_value = 0, g_value = 0;
    sFin = NULL;
    helpText = "i000";
    keyHelpText = "i000";
    WaveFlagSetup(plr);
    sCount = -1;
    SUSPEND = 0;
    Vab_Spot = 0;

    for (i = 0; i < 3; i++) {
        GetMisType(Data->P[plr].Mission[i].MissionCode);
        Data->P[plr].Mission[i].Joint = Mis.Jt;
    }

    // Entering screen for the first time so fade out and in.
    FadeOut(2, 10, 0, 0);
    DrawSpaceport(plr);
    FadeIn(2, 10, 0, 0);

    int height = display::graphics.legacyScreen()->height();
    int width = display::graphics.legacyScreen()->width();

    portViewBuffer = boost::shared_ptr<display::LegacySurface>(
                         new display::LegacySurface(width, height));
    portViewBuffer->palette().copy_from(
        display::graphics.legacyScreen()->palette());
    portViewBuffer->draw(*display::graphics.screen(), 0, 0);

#if SPOT_ON

    if ((Data->P[plr].Pool[0].Active | Data->P[plr].Pool[1].Active | Data->P[plr].Pool[2].Active) >= 1) {
        g_value = 1;
    }

    for (i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_BASIC_2) {
            t_value = 1;
        }
    }

    r_value = brandom(1000);

    if (xMODE & xMODE_CLOUDS) {
        if (plr == 0 && Data->P[plr].Port[PORT_VAB] == 0) {
            SpotCrap(14, SPOT_LOAD);    //USA Storm
        } else if (plr == 1) {
            SpotCrap(12, SPOT_LOAD);    //Sov Storm
        }
    } else if ((xMODE & xMODE_SPOT_ANIM) && g_value) {
        SpotCrap(3 + (5 * plr), SPOT_LOAD);
        xMODE &= ~xMODE_SPOT_ANIM;
    } else if (t_value && g_value) {
        SpotCrap(0 + (5 * plr), SPOT_LOAD);    //LEM
    } else if (r_value < 150) {
        if (plr == 1 && Data->P[plr].Port[PORT_MedicalCtr] == 1) {
            SpotCrap(18, SPOT_LOAD);
        } else {
            SpotCrap(1 + (5 * plr), SPOT_LOAD);
        }
    } else if (r_value > 850) {
        SpotCrap(2 + (5 * plr), SPOT_LOAD);    //Heli
    }

#endif

    Port(plr);
    helpText = "i000";
    keyHelpText = "i000";
    WaveFlagDel();

    if (sFin) {
        fclose(sFin);
        sFin = NULL;
    }

    portViewBuffer.reset();
}


/**
 * Get the mouse or keyboard input.
 *
 * A non-blocking check of mouse/keyboard input based on GetMouse_fast
 * which updates flagpole, USA water, and spot animations for the Port
 * screen.
 */
void GetMse(char plr, char fon)
{
    static double last_wave_step;
    double now;

    now = get_time();

    if (now - last_wave_step > .125) {
        last_wave_step = now;

        if (plr == 0) {
            DoCycle();
        }

        if (fon != 1) {
            goto done;
        }

#if SPOT_ON
        SpotCrap(0, SPOT_STEP);
#endif
        FCtr = FCtr % 5;

        if (plr == 0) {
            display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 49, 121);
        } else {
            display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 220, 141);
        }

done:
        FCtr++;
    }

    GetMouse_fast();
}

void
DoCycle(void)                   // Three ranges of color cycling
{
    int i, tmp1, tmp2, tmp3, j;
    display::AutoPal p(display::graphics.legacyScreen());

    j = 384;

    i = 0;
    tmp1 = p.pal[j + 3 * i + 0];
    tmp2 = p.pal[j + 3 * i + 1];
    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 3; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;

    p.pal[j + 3 * i + 1] = tmp2;

    p.pal[j + 3 * i + 2] = tmp3;

    i = 4;

    tmp1 = p.pal[j + 3 * i + 0];

    tmp2 = p.pal[j + 3 * i + 1];

    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 11; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;

    p.pal[j + 3 * i + 1] = tmp2;

    p.pal[j + 3 * i + 2] = tmp3;

    i = 12;

    tmp1 = p.pal[j + 3 * i + 0];

    tmp2 = p.pal[j + 3 * i + 1];

    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 15; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;

    p.pal[j + 3 * i + 1] = tmp2;

    p.pal[j + 3 * i + 2] = tmp3;
}

/** ???
 *
 * \param mode ...  0 = ?   1 = copy stored outline ?
 */
void
PortOutLine(unsigned int Count, uint16_t *outline, char mode)
{
    int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
    unsigned int i;

    pPortOutlineRestore = (PORTOUTLINE *)xrealloc(pPortOutlineRestore,
                          sizeof(PORTOUTLINE) * Count);

    for (i = 0; i < Count; i++) {
        if (mode == 1) {
            // Save value from the screen
            pPortOutlineRestore[i].loc = outline[i];    // Offset of the outline into the buffer
            pPortOutlineRestore[i].val = display::graphics.legacyScreen()->pixels()[outline[i]];    // Save original pixel value
        } else {                   // dunno
            outline[i] = pPortOutlineRestore[i].loc;
        }

        display::graphics.legacyScreen()->pixels()[outline[i]] = 11;   // Color the outline index 11, which should be Yellow
        min_x = MIN(min_x, outline[i] % MAX_X);
        min_y = MIN(min_y, outline[i] / MAX_X);
        max_x = MAX(max_x, outline[i] % MAX_X);
        max_y = MAX(max_y, outline[i] / MAX_X);
    }
}

void
PortRestore(unsigned int Count)
{
    int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
    unsigned int i;
    int loc;

    for (i = 0; i < Count; i++) {
        loc = pPortOutlineRestore[i].loc;
        display::graphics.legacyScreen()->pixels()[loc] = pPortOutlineRestore[i].val;
        min_x = MIN(min_x, loc % MAX_X);
        min_y = MIN(min_y, loc / MAX_X);
        max_x = MAX(max_x, loc % MAX_X);
        max_y = MAX(max_y, loc / MAX_X);
    }

    free(pPortOutlineRestore);
    pPortOutlineRestore = NULL;
}


/** Map a keypress to a spaceport building selection
 */
int MapKey(char plr, int key, int old)
{
    int val, j, found = 0;
    char high = -1, low = -1;

    for (j = 0; j < 35; j++) {
        if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
            if (low == -1) {
                low = j;
            }

            high = j;
        }
    }

    val = old;

    switch (key) {
    case 'A':
        if (MObj[6].Reg[Data->P[plr].Port[PORT_Admin]].sNum > 0) {
            val = 6;
        }

        mousebuttons = 1;
        break;

    case 'I':
        if (MObj[1].Reg[Data->P[plr].Port[PORT_Pentagon]].sNum > 0) {
            val = 1;
        }

        mousebuttons = 1;
        break;

    case 'M':
        if (MObj[5].Reg[Data->P[plr].Port[PORT_Museum]].sNum > 0) {
            val = 5;
        }

        mousebuttons = 1;
        break;

    case 'R':
        if (MObj[22].Reg[Data->P[plr].Port[PORT_Research]].sNum > 0) {
            val = 22;
        }

        mousebuttons = 1;
        break;

    case 'P':
        if (MObj[2].Reg[Data->P[plr].Port[PORT_Capitol]].sNum > 0) {
            val = 2;
        }

        mousebuttons = 1;
        break;

    case 'V':
        if (MObj[4].Reg[Data->P[plr].Port[PORT_VAB]].sNum > 0) {
            val = 4;
        }

        mousebuttons = 1;
        break;

    case 'C':
        if (MObj[26].Reg[Data->P[plr].Port[PORT_MissionControl]].sNum > 0) {
            val = 26;
        }

        mousebuttons = 1;
        break;

    case 'Q':
        if (MObj[29].Reg[Data->P[plr].Port[PORT_Gate]].sNum > 0) {
            val = 29;
        }

        mousebuttons = 1;
        break;

    case 'E':
        if (MObj[28].Reg[Data->P[plr].Port[PORT_FlagPole]].sNum > 0) {
            val = 28;
        }

        mousebuttons = 1;
        break;

    case 'T':
        if (MObj[7].Reg[Data->P[plr].Port[PORT_AstroComplex]].sNum > 0) {
            val = 7;
        }

        mousebuttons = 1;
        break;

    case 'B':
        if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum > 0) {
            val = 9;
        }

        mousebuttons = 1;
        break;

    case UP_ARROW:
        if (old == high) {
            old = 0;
        } else {
            old = old + 1;
        }

        found = 0;

        for (j = old; j < high + 1; j++) {
            if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
                if (found == 0) {
                    val = j;
                    found = 1;
                }
            }
        }

        break;

    case DN_ARROW:
        if (old == low) {
            old = 34;
        } else {
            old = old - 1;
        }

        found = 0;

        for (j = old; j > low - 1; j--) {
            if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
                if (found == 0) {
                    val = j;
                    found = 1;
                }
            }
        }

        break;

    default:
        val = old;
        break;
    }

    return val;
}

void Port(char plr)
{
    int i, j, kMode, kEnt, k;
    char good, res;
    int kPad, pKey, gork;
    FILE *fin;
    int32_t stable[55];
    uint16_t Count, *bone;

    helpText = "i043";
    keyHelpText = "k043";
    bone = (uint16_t *) buffer;

    fin = sOpen((plr == 0) ? "USA_PORT.DAT" : "SOV_PORT.DAT", "rb", 0);
    // TODO: Add some error checking...
    ImportPortHeader(fin, PHead);

    fseek(fin, PHead.oOut, SEEK_SET);
    fread(&stable[0], sizeof stable, 1, fin);

    for (i = 0; i < 55; i++) {
        Swap32bit(stable[i]);
    }

    if (plr == 0 && Data->Year > 65) {
        PortText(5, 196, "CAPE KENNEDY", 12);
    } else if (plr == 0) {
        PortText(5, 196, "THE CAPE", 12);
    } else {
        PortText(5, 196, "BAIKONUR", 12);
    }

    pKey = 0;

    music_start((plr == 0) ? M_USPORT : M_SVPORT);
    kMode = kPad = kEnt = 0;
    i = 0;  // this is used to loop through all the selection regions on the port

    while (1) {
        av_block();

        if (kMode == 0) {
            i = 0;
        } else if (kMode == 1) {
            kEnt = 0;
        }

        do {
#if BABYSND
            UpdateAudio();
#endif

            if (pKey == 0) {
                key = 0;
                GetMse(plr, 1);
            } else {
                pKey = 0;
            }

            if (kPad > 0 && kMode == 1) {
                key = kPad;
            }

            if (kMode == 1 && !(x == 319 && y == 199)) {
                kMode = 0;
            }

            if (key > 0) {  // this was only looking for the low byte
                i = MapKey(plr, key, i);  // Get Port offset for keyboard input

                if (key == K_ESCAPE) {
                    kMode = 0;
                    i = 0;
                } else {
                    kMode = 1;
                }

                if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                    x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                    y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
                    kEnt = i;
                }
            }

            if (kMode == 1 && kEnt == i) {
                x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
            } else if (kMode == 1 && kEnt != i) {
                x = -1;
                y = -1;
            }

            for (j = 0; j < MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].qty; j++) {
                if (x >= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].x1 &&
                    y >= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].y1 &&
                    x <= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].x2 &&
                    y <= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].y2) {
                    PortText(5, 196, MObj[i].Name, 11);

                    if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                        fseek(fin, stable[MObj[i].Reg[Data->P[plr].Port[i]].sNum], SEEK_SET);
                        fread_uint16_t(&Count, 1, fin);
                        fread_uint16_t(bone, Count, fin);
                        PortOutLine(Count, bone, 1);
                        strncpy(&helpText[1], MObj[i].Help, 3);
                    }

                    good = 0;

                    // Search hotkey string for valid selection
                    for (k = 0; k < (int)strlen(HotKeyList); k++) {
                        if (HotKeyList[k] == ((char)(0x00ff & key))) {
                            good = 1;
                        }
                    }

                    while (x >= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x1 &&
                           y >= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y1 &&
                           x <= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x2 &&
                           y <= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y2) {
                        av_block();
#if BABYSND
                        UpdateAudio();
#endif
                        kPad = 0;
                        key = 0;
                        GetMse(plr, 1);

                        if (kMode == 0 && key > 0) {
                            x = y = 0;
                            pKey = key;
                        }

                        if (kMode == 1 && !(x == 319 && y == 199)) {
                            kMode = 0;
                        }

                        if (kMode == 1) {
                            x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                            y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
                        }

                        if (key > 0 && kMode == 1)  // got a keypress
                            if (key != K_ENTER) {  // not return
                                x = -1;
                                y = -1;
                                kPad = key;
                            }

                        if (good == 1 || (kMode == 0 && mousebuttons == 1) || (kMode == 1 && key == K_ENTER)
                            || (kMode == 0 && key == K_ENTER)) {
                            PortRestore(Count);
                            Count = 0;

                            // || i==33

                            if (!(i == 28 || i == 29 || i == 0 || i == 31
                                  || (Data->Year == 57 || (Data->Year == 58 && Data->Season == 0)))) {
#if SPOT_ON
                                SpotCrap(0, SPOT_KILL);  // remove spots
#endif
                                music_stop();
                            } else {
                                SUSPEND = 1;
                            }

                            res = PortSel(plr, i);

                            switch (res) {
                            case pNOREDRAW:
                                PortText(5, 196, MObj[i].Name, 11);
                                break;

                            case pREDRAW:
                            case pNEWMUSIC:
                            case pNOFADE:
                                if (res != pNOFADE) {
                                    music_stop();
                                    music_start((plr == 0) ? M_USPORT : M_SVPORT);
                                }

                                SpotCrap(0, SPOT_KILL);  // remove spots

                                // Returning to spaceport so fade between redraws
                                if (res == pREDRAW) {
                                    FadeOut(2, 10, 0, 0);
                                }

                                DrawSpaceport(plr);

                                if (res == pREDRAW) {
                                    FadeIn(2, 10, 0, 0);
                                }

#if SPOT_ON
                                portViewBuffer->resetPalette();
                                portViewBuffer->copyFrom(
                                    display::graphics.legacyScreen(),
                                    0, 0,
                                    display::graphics.screen()->width() - 1,
                                    display::graphics.screen()->height() - 1);
                                gork = brandom(100);

                                if (Vab_Spot == 1 && Data->P[plr].Port[PORT_VAB] == 2) {
                                    Data->P[plr].Port[PORT_LaunchPad_A] = 1;

                                    if (plr == 0) {
                                        if (gork <= 60) {
                                            SpotCrap(4, SPOT_LOAD);    //Rocket to Pad
                                        } else {
                                            SpotCrap(15, SPOT_LOAD);    //Rocket&Truck/Door
                                        }
                                    } else if (plr == 1) {
                                        SpotCrap(16, SPOT_LOAD);
                                    }
                                } else if (Vab_Spot == 4 && plr == 0 && Data->P[plr].Port[PORT_VAB] == 0) {
                                    SpotCrap(19, SPOT_LOAD);
                                } else if (Vab_Spot == 2 && plr == 1) {
                                    SpotCrap(10, SPOT_LOAD);
                                } else if (Vab_Spot == 3) {
                                    if (plr == 1) {
                                        SpotCrap(17, SPOT_LOAD);
                                    } else if (plr == 0) {
                                        SpotCrap(11, SPOT_LOAD);
                                    }
                                } else if (gork < 30) {
                                    if (plr == 1 && Data->P[plr].Port[PORT_MedicalCtr] == 1) {
                                        SpotCrap(18, SPOT_LOAD);
                                    } else {
                                        SpotCrap(1 + (5 * plr), SPOT_LOAD);
                                    }
                                } else if (plr == 1 && gork < 40) {
                                    SpotCrap(10, SPOT_LOAD);
                                } else if (gork < 60) {
                                    SpotCrap(2 + (5 * plr), SPOT_LOAD);
                                }

#endif
                                Vab_Spot = 0;
#ifdef DEADCODE
                                // I'm not sure why we're redrawing the outlines here;
                                // commenting it out for now.  If no problems are seen
                                // with the port outlines then restore this
                                //   if (pPortOutlineRestore)
                                //      PortOutLine(Count,bone,0);
#endif
                                PortText(5, 196, MObj[i].Name, 11);
                                break;

                            case pEXIT:
                                FadeOut(2, 10, 0, 0);
                                fclose(fin);
#if BABYSND

                                if (i == 28 || i == 29) {
                                    SUSPEND = 0;
                                }

#endif
                                SpotCrap(0, SPOT_KILL);  // remove spots
                                music_stop();
                                save_game("AUTOSAVE.SAV");
                                return;

                            case pQUIT:
                                FadeOut(2, 10, 0, 0);
#if BABYSND

                                if (i == 28 || i == 29) {
                                    SUSPEND = 0;
                                }

#endif
                                SpotCrap(0, SPOT_KILL);  // remove spots
                                music_stop();
                                return;
                            } // switch

                            kMode = good = SUSPEND = 0;

                            if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                                fseek(fin, stable[MObj[i].Reg[Data->P[plr].Port[i]].sNum], SEEK_SET);
                                fread_uint16_t(&Count, 1, fin);
                                fread_uint16_t(bone, Count, fin);
                                //pPortOutlineRestore = (PORTOUTLINE *) malloc((sizeof (PORTOUTLINE))*Count);
                                PortOutLine(Count, bone, 1);
                            }

                            while (mousebuttons == 1) {
                                GetMse(plr, 1);
                            }
                        } // if
                    } //while

                    if (plr == 0 && Data->Year > 65) {
                        PortText(5, 196, "CAPE KENNEDY", 12);
                    } else if (plr == 0) {
                        PortText(5, 196, "THE CAPE", 12);
                    } else {
                        PortText(5, 196, "BAIKONUR", 12);
                    }

                    PortRestore(Count);
                    Count = 0;
                    helpText = "i043";
                    keyHelpText = "k043";
                } // if
            }

            if (kMode == 0) {
                i++;
            }

            if (kMode == 1) {
                kEnt++;
            }
        } while ((kMode == 0 && i < 35 && i >= 0) || (kMode == 1 && kEnt < 35 && kEnt >= 0));
    } // while
}


/**
 * This is the code that controls the jumpoff point from the Spaceports
 * to the various areas.  It basically assigns a help message, then
 * makes a call into the module - which would have its own event loop.
 */
char PortSel(char plr, char loc)
{
    int i, MisOK, LPad = 0;
    Vab_Spot = 0;  // clear the damn thing.

    switch (loc) {
    case PORT_Monument:
        Help((plr == 0) ? "i023" : "i022");
        keyHelpText = "k022";
        return pNOREDRAW;  // Monuments

    case PORT_Pentagon:
        if (Data->Year == 57 || (Data->Year == 58 && Data->Season == 0)) {
            Help("i108");
            return pNOREDRAW;
        }

        helpText = "i027\0";
        Intel(plr);
        return pNEWMUSIC;

    case PORT_Capitol:
        helpText = (plr == 0) ? "i021" : "i532";
        keyHelpText = (plr == 0) ? "k021" : "k532";
        Review(plr);
        return pREDRAW;

    case PORT_Cemetery:
        helpText = "i020";
        Cemetery(plr);
        return pREDRAW;

    case PORT_VAB:
        if (Option != -1) {
            put_serial(LET_V);
            put_serial(LET_V);
            put_serial(LET_V);
        }

        helpText = "i015";
        VAB(plr);
        return pREDRAW;

    case PORT_Museum:
        if (Option != -1) {
            put_serial(LET_M);
            put_serial(LET_M);
            put_serial(LET_M);
        }

        helpText = "i027";
        Museum(plr);
        return pNEWMUSIC;

    case PORT_Admin:
        if (Option != -1) {
            put_serial(LET_A);
            put_serial(LET_A);
            put_serial(LET_A);
        }

        helpText = "i027";
        Admin(plr);

        if (LOAD == 1) {
            return pEXIT;
        } else if (QUIT == 1) {
            return pQUIT;
        } else {
            if (plr == 0) {
                Vab_Spot = 4;
            }

            return pNEWMUSIC;
        }

    case PORT_AstroComplex:
        helpText = "i039";
        Limbo(plr);
        return pREDRAW;  // Astro Complex

    case PORT_MedicalCtr:
        helpText = "i041";
        Hospital(plr);
        return pREDRAW;

    case PORT_BasicTraining:
        helpText = "i038";
        Train(plr, 0);
        return pREDRAW;

    case PORT_Helipad:
        helpText = "i037";
        Train(plr, 2);
        return pREDRAW;

    case PORT_Pool:
        helpText = "i037";
        Train(plr, 3);
        return pREDRAW;

    case PORT_Planetarium:
        helpText = "i037";
        Train(plr, 4);
        return pREDRAW;

    case PORT_Centrifuge:
        helpText = "i037";
        Train(plr, 5);
        return pREDRAW;

    case PORT_Airfield:
        helpText = "i037";
        Train(plr, 1);
        return pREDRAW;

    case PORT_Satellite:
        helpText = "i019";
        SatBld(plr);
        return pREDRAW;

    case PORT_LM:
        helpText = "i044";
        keyHelpText = "k209";
        LMBld(plr);
        return pREDRAW;  // LM Program

    case PORT_Jupiter:
        helpText = "i036";
        Programs(plr, 5);
        return pREDRAW;

    case PORT_XMS:
        helpText = "i036";
        Programs(plr, 4);
        return pREDRAW;

    case PORT_Apollo:
        helpText = "i036";
        Programs(plr, 3);
        return pREDRAW;

    case PORT_Gemini:
        helpText = "i036";
        Programs(plr, 2);
        return pREDRAW;

    case PORT_Mercury:
        helpText = "i036";
        Programs(plr, 1);
        return pREDRAW;

    case PORT_Research:
        if (Option != -1) {
            put_serial(LET_R);
            put_serial(LET_R);
            put_serial(LET_R);
        }

        helpText = "i009";
        RD(plr);

        if (plr == 1) {
            Vab_Spot = 2;
        }

        return pREDRAW;

    case PORT_LaunchPad_A:
    case PORT_LaunchPad_B:
    case PORT_LaunchPad_C:
        helpText = "i028";
        ShowPad(plr, loc - 23);
        return pREDRAW;

    case PORT_MissionControl:
        helpText = "i018";
        keyHelpText = "k018";
        MisOK = 0;

        for (i = 0; i < 3; i++) {
            if (Data->P[plr].Mission[i].MissionCode &&
                Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0) {
                MisOK = 10;
            }

            if (Data->P[plr].Mission[i].MissionCode) {
                MisOK++;
            }
        }

        if (MisOK >= 10) {
            Help("i005");
        } else if (MisOK > 0) {
            Rush(plr);
            RUSH = 1;
            return pREDRAW;
        } else  {
            Help("i104");
        }

        return pNOFADE;

    case PORT_ViewingStand:
        helpText = "i017";
        keyHelpText = "k017";
        Viewing(plr);
        return pREDRAW;

    case PORT_FlagPole:  // Flagpole : End turn
        MisOK = 0;

        // Check to see if missions are good to go
        for (i = 0; i < 3; i++) {
            if (Data->P[plr].Mission[i].MissionCode &&
                Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0) {
                MisOK = 10;
            }

            if (Data->P[plr].Mission[i].MissionCode) {
                MisOK++;
            }

            if (Data->P[plr].Future[i].MissionCode) {
                LPad++;
            }
        }

        if (Data->Year == 57 && Data->Season == 0) {
            LPad = 1;
        }

        if (MisOK >= 10) {
            Help("i005");
            return pNOREDRAW;
        } else if (LPad == 0) {
            // Warn player that they have no missions scheduled for
            // next turn, except on the first turn of the game. -Leon
            if (Help("i161") >= 0) {
                LPad = 10;  // Set LPad above zero so you won't be warned again this turn  -Leon
                i = Request(plr, "END TURN", 1);

                if (i) {
                    return pEXIT;
                }
            }
        } else if (MisOK == 0) {
            i = Request(plr, "END TURN", 1);

            if (i) {
                return pEXIT;
            }
        } else if (RUSH == 1 && MisOK > 0 && MisOK < 10) {
            MisOK = Help("i004");           // Mission Control

            if (MisOK >= 0) {
                if (Option != -1) {
                    put_serial(LET_O);
                    put_serial(LET_O);
                    put_serial(LET_O);
                }

                RUSH = 0;
                return pEXIT;
            }
        } else if (RUSH == 0) {
            Help("i103");
        }

        return pNOREDRAW;

    case PORT_Gate: // Security Gate: Quit
        QUIT = Request(plr, "QUIT", 1);

        if (QUIT) {
            if (Option != -1) {
                DoModem(2);
            }

            return pQUIT;
        }

        return pNOREDRAW;

    case PORT_Moon:
        helpText = "i029";
        Moon(plr);
        return pREDRAW;  // Moon

    case PORT_SovMonumentAlt:
        Help("i025");
        return pNOREDRAW;  // Sov Mon #2

    case PORT_Zond:
        helpText = "i036";
        Programs(plr, 3);
        return pREDRAW;  // Zond

    case PORT_Tracking:
        if (Option != -1) {
            MesCenter();    // Tracking
            Vab_Spot = 3;
            return pREDRAW;
        } else {
            Help("i042");
            Vab_Spot = 3;
            return pNOFADE;
        }

    case PORT_SVHQ:
        return pNOREDRAW;  // SV

    default:
        return pNOREDRAW;
    }
}


char Request(char plr, char *s, char md)
{
    char i;
    display::LegacySurface local(196, 84);

    if (md > 0) {  // Save Buffer
        local.copyFrom(display::graphics.legacyScreen(), 85, 52, 280, 135);
    }

    i = strlen(s) >> 1;
    display::graphics.setForegroundColor(0);
    ShBox(85, 52, 249, 135);
    IOBox(170, 103, 243, 130);
    IOBox(91, 103, 164, 130);
    InBox(92, 58, 243, 97);
    display::graphics.setForegroundColor(1);
    draw_heading(111, 110, "YES", 0, 0);
    draw_heading(194, 110, "NO", 0, 0);

    display::graphics.setForegroundColor(11);

    if (md == 6)  {
        draw_string(166 - i * 7, 65, s);
    } else {
        draw_heading(166 - i * 10, 65, &s[0], 0, -1);
    }

    draw_string(135, 94, "ARE YOU SURE?");

    while (1) {
        if (md != 6) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if (mousebuttons == 0) {
            break;
        }
    }

    i = 2;

    while (i == 2) {
        if (md != 6) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if ((x >= 172 && y >= 105 && x <= 241 && y <= 128 && mousebuttons != 0) || (key == 'N')) {
            InBox(172, 105, 241, 128);
            i = 0;
            delay(50);
            key = 0;
        }

        if ((x > 93 && y >= 105 && x <= 162 && y <= 128 && mousebuttons != 0) || (key == 'Y')) {
            InBox(93, 105, 162, 128);
            i = 1;
            delay(50);
            key = 0;
        }

        delay(50);
    } /* End while */

    if (md > 0) {
        display::graphics.legacyScreen()->palette().copy_from(local.palette());
        display::graphics.screen()->draw(local, 85, 52);
    }

    return i;
}


/**
 * Confirm all scheduled missions are a go.
 *
 * Launches a pop-up to review the missions scheduled for launch this
 * turn. A more advaned version of the existing setup, providing a
 * summary of the missions and access to Mission Control.
 *
 * TODO: This is an unfinished function that is never called.
 *
 * \param plr
 * \return  0 or 1 (Unsure which is proceed and which not, b/c unfinished).
 */
char MisReq(char plr)
{
    int i, num = 0;
    display::LegacySurface local(184, 132);

    local.copyFrom(display::graphics.legacyScreen(), 53, 29, 236, 160);

    for (i = 0; i < 3; i++) {
        if ((Data->P[plr].Mission[i].MissionCode) &&
            (Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0)) {
            num++;
        }
    }

    ShBox(53, 29, 236, 160);
    ShBox(60, 34, 229, 44);
    InBox(60, 47, 229, 120);
    // fill_rectangle(61,48,228,119,0);
    ShBox(63, 50, 226, 117);

    if (num == 0) {
        IOBox(60, 141, 141, 155);
        IOBox(148, 141, 229, 155);
        display::graphics.setForegroundColor(1);
        draw_string(70, 129, "CONFIRM LAUNCH SCHEDULE OR");
        draw_string(80, 136, "CHOOSE TO REVIEW IT.");
        display::graphics.setForegroundColor(8);
        draw_string(85, 150, "C");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "ONFIRM");
        display::graphics.setForegroundColor(8);
        draw_string(179, 150, "R");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "EVIEW");
    } else {
        IOBox(60, 141, 229, 155);
        display::graphics.setForegroundColor(1);
        draw_string(62, 129, "MISSIONS DO NOT ALL HAVE");
        draw_string(62, 136, "ASSIGNMENTS. NO COMMIT POSSIBLE");
        draw_string(120, 150, "REVIEW MISSIONS");
    }

    display::graphics.setForegroundColor(10);

    draw_string(94, 41, "LAUNCH CONFIRMATION");

    display::graphics.setForegroundColor(1);

    for (i = 0; i < 3; i++) {
        draw_string(68, 59 + 20 * i, "PAD ");
        draw_character(0x41 + i);
        draw_string(0, 0, ": ");

        if (Data->P[plr].Mission[i].MissionCode) {
            draw_string(0, 0, &Data->P[plr].Mission[i].Name[0]);

            if (Data->P[plr].Mission[i].Men > 0) {
                draw_string(86, 65 + 20 * i, "MANNED MISSION");
            } else {
                draw_string(86, 65 + 20 * i, "UNMANNED MISSION");
            }

            if (Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0) {
                display::graphics.setForegroundColor(9);
                draw_string(86, 71 + 20 * i, "HARDWARE UNASSIGNED");
            } else {
                draw_string(86, 71 + 20 * i, "HARDWARE ASSIGNED");
            }

            display::graphics.setForegroundColor(1);
        }
    }

    while (1) {
        if (plr == 0) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if (mousebuttons == 0) {
            break;
        }
    }

    i = 2;

    while (i == 2) {
        if (plr == 0) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if ((x >= 62 && y >= 143 && x <= 139 && y <= 153 && mousebuttons != 0 && num == 0)
            || (key == 'C' && num == 0)) {
            InBox(62, 143, 139, 153);
            i = 1;
            delay(50);
            key = 0;
            Rush(plr);
        }

        if ((x > 150 && y >= 143 && x <= 227 && y <= 153 && mousebuttons != 0 && num == 0)
            || (key == 'R' && num == 0)) {
            InBox(150, 143, 227, 153);
            i = 0;
            delay(50);
            key = 0;
        }

        if ((x > 62 && y >= 143 && x <= 227 && y <= 153 && mousebuttons != 0 && num > 0)
            || (key == 'R' && num > 0)) {
            InBox(62, 143, 227, 153);
            i = 0;
            delay(50);
            key = 0;
        }
    } /* End while */

    local.copyTo(display::graphics.legacyScreen(), 53, 29);

    return i;
}


/**
 * Read a PortHeader struct stored as raw data in a file, correcting
 * for endianness.
 *
 * If import is not successful, the contents of the target PortHeader
 * are not guaranteed.
 *
 * \param fin  An open port data file at the start of the header data.
 * \param target  The destination for the read data.
 * \return  1 if successfully read, 0 otherwise.
 */
size_t ImportPortHeader(FILE *fin, struct PortHeader &target)
{
    // Chain freads so they stop if one fails...
    bool read =
        fread(&target.Text[0], sizeof(target.Text), 1, fin) &&
        fread(&target.oMObj, sizeof(target.oMObj), 1, fin) &&
        fread(&target.oTab, sizeof(target.oTab), 1, fin) &&
        fread(&target.oPal, sizeof(target.oPal), 1, fin) &&
        fread(&target.oPort, sizeof(target.oPort), 1, fin) &&
        fread(&target.oMse, sizeof(target.oMse), 1, fin) &&
        fread(&target.oOut, sizeof(target.oOut), 1, fin) &&
        fread(&target.oAnim, sizeof(target.oAnim), 1, fin);

    if (read) {
        Swap32bit(target.oMObj);
        Swap32bit(target.oTab);
        Swap32bit(target.oPal);
        Swap32bit(target.oPort);
        Swap32bit(target.oMse);
        Swap32bit(target.oOut);
        Swap32bit(target.oAnim);
    }

    return (read ? 1 : 0);
}


/**
 * Read a MOBJ struct stored as raw data in a file, correcting
 * for endianness.
 *
 * If import is not successful, the contents of the target MOBJ
 * are not guaranteed.
 *
 * \param fin  An open port data file at the start of the MOBJ data.
 * \param target  The destination for the read data.
 * \return  1 if successfully read, 0 otherwise.
 */
size_t ImportMOBJ(FILE *fin, MOBJ &target)
{
    // Chain freads so they stop if one fails...
    bool read =
        fread(&target.Name[0], sizeof(target.Name), 1, fin) &&
        fread(&target.qty, sizeof(target.qty), 1, fin) &&
        fread(&target.Help[0], sizeof(target.Help), 1, fin);

    for (int i = 0; i < 4 && read; i++) {
        read = read &&
               fread(&target.Reg[i].qty,
                     sizeof(target.Reg[i].qty), 1, fin);

        for (int j = 0; j < 4 && read; j++) {
            read = read &&
                   fread(&target.Reg[i].CD[j].x1,
                         sizeof(target.Reg[i].CD[j].x1), 1, fin) &&
                   fread(&target.Reg[i].CD[j].y1,
                         sizeof(target.Reg[i].CD[j].y1), 1, fin) &&
                   fread(&target.Reg[i].CD[j].x2,
                         sizeof(target.Reg[i].CD[j].x2), 1, fin) &&
                   fread(&target.Reg[i].CD[j].y2,
                         sizeof(target.Reg[i].CD[j].y2), 1, fin);

            if (read) {
                Swap16bit(target.Reg[i].CD[j].x1);
                Swap16bit(target.Reg[i].CD[j].x2);
                Swap16bit(target.Reg[i].CD[j].y1);
                Swap16bit(target.Reg[i].CD[j].y2);
            }
        }

        read = read &&
               fread(&target.Reg[i].iNum,
                     sizeof(target.Reg[i].iNum), 1, fin) &&
               fread(&target.Reg[i].sNum,
                     sizeof(target.Reg[i].sNum), 1, fin) &&
               fread(&target.Reg[i].PreDraw,
                     sizeof(target.Reg[i].PreDraw), 1, fin);
    }

    return (read ? 1 : 0);
}

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

// Edit r settings {{{
// ex: ts=4 noet sw=2
// ex: foldmethod=marker
// }}}
