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

#include "display/png_image.h"
#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "utils.h"
#include "intro.h"
#include "game_main.h"
#include "sdlhelper.h"
#include "gr.h"
#include "gx.h"
#include "pace.h"
#include "filesystem.h"

struct CREDIT {
    char page;
    int col;
    int x, y;
    char Txt[35];
} CREDIT[] = {
    { 0, 11, 108, 12, "CREATION AND DESIGN"},
    { 0, 1, 124, 20, "FRITZ BRONNER"},
    { 0, 11, 128, 34, "PROGRAMMING"},
    { 0, 1, 116, 42, "MICHAEL K MCCARTY"},
    { 0, 11, 83, 54, "A.I. AND ADDITIONAL PROGRAMMING"},
    { 0, 1, 122, 62, "MORGAN ROARTY"},
    { 0, 11, 185, 74, "GRAPHIC MANAGEMENT"},
    { 0, 1, 194, 82, "TOM CHAMBERLAIN"},
    { 0, 11, 190, 93, "COMPUTER ARTISTS"},
    { 0, 1, 202, 101, "DAVID MOSHER"},
    { 0, 1, 198, 108, "MARGIE BEESON"},
    { 0, 1, 195, 115, "TODD J CAMASTA"},
    { 0, 1, 193, 122, "TOM CHAMBERLAIN"},
    { 0, 1, 199, 129, "CHERYL AUSTIN"},
    { 0, 1, 190, 136, "CHARLES H WEIDMAN"},
    { 0, 1, 206, 143, "RONAN JOYCE"},
    { 0, 1, 207, 150, "STEVE STIPP"},
    { 0, 1, 200, 157, "FRITZ BRONNER"},
    { 0, 1, 189, 164, "MICHAEL K MCCARTY"},
    { 0, 11, 203, 174, "SOUND EDITING"},
    { 0, 1, 201, 182, "FRITZ BRONNER"},
    { 0, 1, 190, 189, "MICHAEL K MCCARTY"},
    { 0, 11, 33, 74, "VIDEO DIGITIZING"},
    { 0, 1, 33, 82, "TOM CHAMBERLAIN"},
    { 0, 1, 40, 89, "FRITZ BRONNER"},
    { 0, 1, 31, 96, "MICHAEL K MCCARTY"},
    { 0, 1, 42, 103, "RONAN JOYCE"},
    { 0, 1, 37, 110, "MORGAN ROARTY"},
    { 0, 11,  9, 121, "ORIGINAL MUSIC COMPOSITION"},
    { 0, 1, 35, 129, "BRIAN LANGSBARD"},
    { 0, 1, 35, 136, "MICHAEL MCNERNY"},
    { 0, 11, 27, 146, "MUSIC ADAPTATIONS"},
    { 0, 1, 32, 154, "RICHARD JACKSON"},
    { 0, 11, 34, 165, "AUDIO DIGITIZING"},
    { 0, 1, 39, 173, "FRITZ BRONNER"},
    { 0, 1, 30, 180, "MICHAEL K MCCARTY"},
    { 0, 1, 35, 187, "CHARLES DEENEN"},
    { 1, 11, 140, 10, "DIRECTOR"},
    { 1, 1, 126, 18, "FRITZ BRONNER"},
    { 1, 11, 131, 28, "PRODUCED BY"},
    { 1, 1, 127, 37, "FRITZ BRONNER"},
    { 1, 1, 117, 44, "MICHAEL K MCCARTY"},
    { 1, 11, 47, 62, "DATA ENTRY"},
    { 1, 1, 33, 70, "TOM CHAMBERLAIN"},
    { 1, 1, 42, 77, "ENDA MCCARTY"},
    { 1, 11, 29, 88, "COSMONAUT VOICES"},
    { 1, 1, 33, 96, "GHERMAN TRIGUB"},
    { 1, 1, 37, 103, "MARINA TRIGUB"},
    { 1, 11, 40, 114, "OTHER VOICES"},
    { 1, 1, 35, 122, "GEORGE SPELVIN"},
    { 1, 1, 36, 129, "GENNIE SPELVIN"},
    { 1, 1, 49, 136, "JIM BROWN"},
    { 1, 11, 34, 149, "VECTOR GRAPHICS"},
    { 1, 1, 31, 157, "J SCOTT MATTHEWS"},
    { 1, 1, 34, 164, "MICHAEL PACKARD"},
    { 1, 11, 34, 179, "SPACE MINITURES"},
    { 1, 1, 33, 187, "BRONNER STUDIOS"},
    { 1, 11, 178, 62, "INTERPLAY'S PRODUCER"},
    { 1, 1, 183, 70, "JACOB R BUCHERT III"},
    { 1, 1, 182, 78, "BRUCE SCHLICKBERND"},
    { 1, 11, 170, 88, "DIRECTOR OF PLAYTESTING"},
    { 1, 1, 192, 96, "KERRY GARRISON"},
    { 1, 11, 186, 110, "LEAD PLAY TESTERS"},
    { 1, 2, 202, 118, "JEREMY AIREY"},
    { 1, 2, 193, 125, "MICHAEL PACKARD"},
    { 1, 2, 199, 132, "MARK HARRISON"},
    { 1, 1, 199, 139, "ERIK ANDERSON"},
    { 1, 2, 176, 146, "JENNIFER SCHLICKBERND"},
    { 1, 11, 199, 158, "COVER PAINTING"},
    { 1, 1, 209, 166, "KEN HODGES"},

    { 1, 11, 212, 179, "SDL PORT"},
    { 1, 1, 195, 187, "PACE WILLISSON"},
};
int nCREDIT = sizeof CREDIT / sizeof CREDIT[0];

void Credits(void)
{
    int k, i;
    keyHelpText = "i999";

    FadeOut(2, display::graphics.palette(), 30, 0, 0);

    boost::shared_ptr<display::PNGImage> image(Filesystem::readImage("images/first.img.3.png"));

    image->export_to_legacy_palette();

    for (k = 0; k < 2; k++) {

        if (k != 0) {
            FadeOut(2, display::graphics.palette(), 30, 0, 0);    // Screen #2
        }

        image->export_to_legacy_palette();
        image->draw();

        for (i = 0; i < nCREDIT; i++) {
            if (CREDIT[i].page == k) {
                display::graphics.setForegroundColor(CREDIT[i].col);
                PrintAt(CREDIT[i].x, CREDIT[i].y, CREDIT[i].Txt);
            }
        }

        FadeIn(2, display::graphics.palette(), 30, 0, 0);

        while (1)  {
            GetMouse();

            if (mousebuttons == 0) {
                break;
            }
        }

        key = 0;

        while (mousebuttons == 0 && key == 0) {
            GetMouse();
        }

        key = 0;
    }

    FadeOut(2, display::graphics.palette(), 30, 0, 0);
    display::graphics.screen()->clear(0);
    keyHelpText = "k000";
}


#define INTRO_IMAGE_COUNT 15

void Introd(void)
{
    int k;
    double start;

    music_start(M_LIFTOFF);

    /* Frame 0 is Interplay, and frame 1 is Strategic Visions */
    /* These are both defunct, so start at frame 2 instead */
    for (k = 2; k < INTRO_IMAGE_COUNT; k++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "images/first.img.%i.png", k);

        boost::shared_ptr<display::PNGImage> image(Filesystem::readImage(filename));

        image->export_to_legacy_palette();
        image->draw();

        FadeIn(2, display::graphics.palette(), 30, 0, 0);

        start = get_time();

        while (get_time() - start < 3) {
            av_step();

            key = 0;
            PauseMouse();

            if (key) {
                goto done;
            } else if (key || grGetMouseButtons()) {
                break;
            }
        }

        FadeOut(2, display::graphics.palette(), 30, 0, 0);
    }

done:
    gxClearDisplay(0, 0);
}

void NextTurn(char plr)
{
    FILE *fin = NULL;
    int32_t len = 0;

    memset(display::graphics.palette(), 0x00, 3 * 256);
    helpText = "i000";
    keyHelpText = "k000";

    fin = sOpen("TURN.BUT", "rb", 0);
    fread(display::graphics.palette(), 768, 1, fin);
    len = fread(display::graphics.screen()->pixels(), 1, MAX_X * MAX_Y, fin);
    fclose(fin);

    RLED_img(display::graphics.screen()->pixels(), (char *)vhptr.vptr, (unsigned int)len, vhptr.w, vhptr.h);

    gxClearDisplay(0, 0);

    ShBox(0, 60, 319, 80);
    display::graphics.setForegroundColor(6 + plr * 3);

    if (plr == 0) {
        DispBig(15, 64, "DIRECTOR OF THE UNITED STATES", 0, -1);
    } else {
        DispBig(30, 64, "CHIEF DESIGNER OF THE USSR", 0, -1);
    }

    display::graphics.setForegroundColor(11);
    grMoveTo(175, 122);

    if (Data->Season == 0) {
        PrintAt(0, 0, "SPRING 19");
    } else {
        PrintAt(0, 0, "FALL 19");
    }

    DispNum(0, 0, Data->Year);
    gxVirtualDisplay(&vhptr, 110 * plr, 0, 30, 85, 31 + 107, 85 + 94, 0);

    FadeIn(2, display::graphics.palette(), 10, 0, 0);
    music_start((plr == 0) ? M_GOOD : M_FUTURE);

    WaitForMouseUp();
    WaitForKeyOrMouseDown();
    key = 0;

    music_stop();
    FadeOut(2, display::graphics.palette(), 10, 0, 0);
    gxClearDisplay(0, 0);
    return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
