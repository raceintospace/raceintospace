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

#include "display/image.h"
#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "utils.h"
#include "intro.h"
#include "draw.h"
#include "game_main.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "filesystem.h"

struct CREDIT {
    char page;
    int col;
    int x, y;
    char Txt[35];
} CREDIT[] = {
    { 0, 22, 96, 12, "OPEN-SOURCED THE GAME"},
    { 0, 10, 119, 20, "FRITZ BRONNER"},
    { 0, 10, 108, 27, "MICHAEL K MCCARTY"},
    { 0, 22, 70, 39, "WORK ON THE OPEN-SOURCE VERSION"},
    { 0, 22, 109, 51, "PROJECT MANAGERS"},
    { 0, 48, 108, 58, "MICHAEL K MCCARTY"},
    { 0, 48, 121, 65, "LEON BARADAT"},
    { 0, 22, 51, 77, "PROGRAMMING"},
    { 0, 48, 16, 84, "KRZYSTOF KOSCIUSZKIEWICZ"},
    { 0, 48, 56, 91, "WILL GLYNN"},
    { 0, 48, 45, 98, "PACE WILLISSON"},
    { 0, 48, 41, 105, "NICOLAS LAPLUME"},
    { 0, 48, 46, 112, "BRYANT MOSCON"},
    { 0, 48, 52, 119, "RYAN YOAKUM"},
    { 0, 48, 53, 126, "PETR MENSIK"},
    { 0, 22, 195, 77, "ENHANCEMENTS"},
    { 0, 48, 188, 84, "NICOLAS LAPLUME"},
    { 0, 48, 199, 91, "RYAN YOAKUM"},
    { 0, 22, 184, 103, "ADDED VISUAL CUES"},
    { 0, 48, 196, 110, "LEON BARADAT"},
    { 0, 22, 178, 122, "INSPIRATION & ADVICE"},
    { 0, 48, 194, 129, "ERIK ANDERSON"},
    { 0, 22, 110, 141, "WINDOWS RELEASES"},
    { 0, 48, 87, 148, "KRZYSTOF KOSCIUSZKIEWICZ"},
    { 0, 22, 116, 160, "LINUX RELEASES"},
    { 0, 48, 117, 167, "PACE WILLISSON"},
    { 0, 22, 121, 179, "MAC RELEASES"},
    { 0, 48, 128, 186, "WILL GLYNN"},

    { 1, 22, 108, 12, "CREATION AND DESIGN"},
    { 1, 39, 124, 20, "FRITZ BRONNER"},
    { 1, 22, 128, 34, "PROGRAMMING"},
    { 1, 48, 116, 42, "MICHAEL K MCCARTY"},
    { 1, 22, 83, 54, "A.I. AND ADDITIONAL PROGRAMMING"},
    { 1, 48, 122, 62, "MORGAN ROARTY"},
    { 1, 22, 185, 74, "GRAPHIC MANAGEMENT"},
    { 1, 48, 194, 82, "TOM CHAMBERLAIN"},
    { 1, 22, 190, 93, "COMPUTER ARTISTS"},
    { 1, 48, 202, 101, "DAVID MOSHER"},
    { 1, 48, 198, 108, "MARGIE BEESON"},
    { 1, 48, 195, 115, "TODD J CAMASTA"},
    { 1, 48, 193, 122, "TOM CHAMBERLAIN"},
    { 1, 48, 199, 129, "CHERYL AUSTIN"},
    { 1, 48, 190, 136, "CHARLES H WEIDMAN"},
    { 1, 48, 206, 143, "RONAN JOYCE"},
    { 1, 48, 207, 150, "STEVE STIPP"},
    { 1, 48, 200, 157, "FRITZ BRONNER"},
    { 1, 48, 189, 164, "MICHAEL K MCCARTY"},
    { 1, 22, 203, 174, "SOUND EDITING"},
    { 1, 48, 201, 182, "FRITZ BRONNER"},
    { 1, 48, 190, 189, "MICHAEL K MCCARTY"},
    { 1, 22, 33, 74, "VIDEO DIGITIZING"},
    { 1, 48, 33, 82, "TOM CHAMBERLAIN"},
    { 1, 48, 40, 89, "FRITZ BRONNER"},
    { 1, 48, 31, 96, "MICHAEL K MCCARTY"},
    { 1, 48, 42, 103, "RONAN JOYCE"},
    { 1, 48, 37, 110, "MORGAN ROARTY"},
    { 1, 22,  9, 121, "ORIGINAL MUSIC COMPOSITION"},
    { 1, 48, 35, 129, "BRIAN LANGSBARD"},
    { 1, 48, 35, 136, "MICHAEL MCNERNY"},
    { 1, 22, 27, 146, "MUSIC ADAPTATIONS"},
    { 1, 48, 32, 154, "RICHARD JACKSON"},
    { 1, 22, 34, 165, "AUDIO DIGITIZING"},
    { 1, 48, 39, 173, "FRITZ BRONNER"},
    { 1, 48, 30, 180, "MICHAEL K MCCARTY"},
    { 1, 48, 35, 187, "CHARLES DEENEN"},

    { 2, 22, 140, 10, "DIRECTOR"},
    { 2, 48, 126, 18, "FRITZ BRONNER"},
    { 2, 22, 131, 28, "PRODUCED BY"},
    { 2, 48, 127, 37, "FRITZ BRONNER"},
    { 2, 48, 117, 44, "MICHAEL K MCCARTY"},
    { 2, 22, 47, 62, "DATA ENTRY"},
    { 2, 48, 33, 70, "TOM CHAMBERLAIN"},
    { 2, 48, 42, 77, "ENDA MCCARTY"},
    { 2, 22, 29, 88, "COSMONAUT VOICES"},
    { 2, 48, 33, 96, "GHERMAN TRIGUB"},
    { 2, 48, 37, 103, "MARINA TRIGUB"},
    { 2, 22, 40, 114, "OTHER VOICES"},
    { 2, 48, 35, 122, "GEORGE SPELVIN"},
    { 2, 48, 36, 129, "GENNIE SPELVIN"},
    { 2, 48, 49, 136, "JIM BROWN"},
    { 2, 22, 34, 149, "VECTOR GRAPHICS"},
    { 2, 48, 31, 157, "J SCOTT MATTHEWS"},
    { 2, 48, 34, 164, "MICHAEL PACKARD"},
    { 2, 22, 31, 179, "SPACE MINIATURES"},
    { 2, 48, 33, 187, "BRONNER STUDIOS"},
    { 2, 22, 178, 62, "INTERPLAY'S PRODUCER"},
    { 2, 48, 183, 70, "JACOB R BUCHERT III"},
    { 2, 48, 182, 78, "BRUCE SCHLICKBERND"},
    { 2, 22, 170, 88, "DIRECTOR OF PLAYTESTING"},
    { 2, 48, 192, 96, "KERRY GARRISON"},
    { 2, 22, 189, 110, "LEAD PLAYTESTERS"},
    { 2, 48, 202, 118, "JEREMY AIREY"},
    { 2, 48, 193, 125, "MICHAEL PACKARD"},
    { 2, 48, 199, 132, "MARK HARRISON"},
    { 2, 48, 199, 139, "ERIK ANDERSON"},
    { 2, 48, 176, 146, "JENNIFER SCHLICKBERND"},
    { 2, 22, 199, 158, "COVER PAINTING"},
    { 2, 48, 209, 166, "KEN HODGES"},
    { 2, 22, 212, 179, "SDL PORT"},
    { 2, 48, 195, 187, "PACE WILLISSON"},
};
// Note: the structure of the items above is:
// { Screen to display in, Color, X position, Y position, "WORDING"},

int nCREDIT = sizeof CREDIT / sizeof CREDIT[0];

void Credits(void)
{
    int k, i;
    keyHelpText = "i999";

    FadeOut(2, 30, 0, 0);

    boost::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage("images/first.img.3.png"));

    image->exportPalette();

    for (k = 0; k < 3; k++) {

        if (k != 0) {
            FadeOut(2, 30, 0, 0);    // Screen #2
        }

        image->exportPalette();
        display::graphics.screen()->draw(image, 0, 0);

        for (i = 0; i < nCREDIT; i++) {
            if (CREDIT[i].page == k) {
                display::graphics.setForegroundColor(CREDIT[i].col);
                draw_string(CREDIT[i].x, CREDIT[i].y, CREDIT[i].Txt);
            }
        }

        FadeIn(2, 30, 0, 0);

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

    FadeOut(2, 30, 0, 0);
    display::graphics.screen()->clear();
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

        boost::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage(filename));

        image->exportPalette();
        display::graphics.screen()->draw(image, 0, 0);

        FadeIn(2, 30, 0, 0);

        start = get_time();

        while (get_time() - start < 3) {
            av_step();

            key = 0;
            PauseMouse();

            if (key || grGetMouseButtons()) {
                goto done;
            }
        }

        FadeOut(2, 30, 0, 0);
    }

done:
    display::graphics.screen()->clear();
}

void NextTurn(char plr)
{
    helpText = "i000";
    keyHelpText = "k000";

    boost::shared_ptr<display::PalettizedSurface> countrySeals(Filesystem::readImage("images/turn.but.0.png"));
    countrySeals->exportPalette();

    display::graphics.screen()->clear();

    ShBox(0, 60, 319, 80);
    display::graphics.setForegroundColor(6 + plr * 3);

    if (plr == 0) {
        draw_heading(15, 64, "DIRECTOR OF THE UNITED STATES", 0, -1);
    } else {
        draw_heading(30, 64, "CHIEF DESIGNER OF THE USSR", 0, -1);
    }

    display::graphics.setForegroundColor(11);
    grMoveTo(175, 122);

    if (Data->Season == 0) {
        draw_string(0, 0, "SPRING 19");
    } else {
        draw_string(0, 0, "FALL 19");
    }

    draw_number(0, 0, Data->Year);
    display::graphics.screen()->draw(countrySeals, 110 * plr, 0, 107, 94, 30, 85);

    FadeIn(2, 10, 0, 0);
    music_start((plr == 0) ? M_GOOD : M_FUTURE);

    WaitForMouseUp();
    WaitForKeyOrMouseDown();
    key = 0;

    music_stop();
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
