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

// This page diplays the official game credits

#include "intro.h"

#include "display/image.h"
#include "display/graphics.h"
#include "display/surface.h"
#include "raceintospace_config.h"

#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "pace.h"
#include "sdlhelper.h"
#include "utils.h"

const struct CREDIT {
    char page;
    int col;
    int x, y;
    const char *Txt;
} CREDIT[] = {
    { 0, 22, 96, 12, "OPEN-SOURCED THE GAME"},
    { 0, 10, 119, 20, "FRITZ BRONNER"},
    { 0, 10, 108, 27, "MICHAEL K MCCARTY"},
    { 0, 22, 57, 39, "WORK ON THE OPEN-SOURCE VERSION " PACKAGE_VERSION},
    { 0, 22, 109, 51, "PROJECT MANAGERS"},
    { 0, 48, 108, 58, "MICHAEL K MCCARTY"},
    { 0, 48, 121, 65, "LEON BARADAT"},
    { 0, 22, 51, 77, "PROGRAMMING"},
    { 0, 48, 16, 84, "KRZYSZTOF KOSCIUSZKIEWICZ"},
    { 0, 48, 56, 91, "WILL GLYNN"},
    { 0, 48, 45, 98, "PACE WILLISSON"},
    { 0, 48, 41, 105, "NICOLAS LAPLUME"},
    { 0, 48, 52, 112, "RYAN YOAKUM"},
    { 0, 48, 53, 119, "PETR MENSIK"},
    { 0, 48, 45, 126, "HENDRIK WEIMER"},
    { 0, 48, 70, 133, "ALSED"},
    { 0, 22, 194, 77, "ENHANCEMENTS"},
    { 0, 48, 187, 84, "NICOLAS LAPLUME"},
    { 0, 48, 198, 91, "RYAN YOAKUM"},
    { 0, 22, 183, 103, "ADDED VISUAL CUES"},
    { 0, 48, 196, 110, "LEON BARADAT"},
    { 0, 22, 178, 122, "INSPIRATION & ADVICE"},
    { 0, 48, 194, 129, "ERIK ANDERSON"},
    { 0, 22, 69, 145, "MISSION MOVIES, WINDOWS RELEASES"},
    { 0, 48, 86, 152, "KRZYSZTOF KOSCIUSZKIEWICZ"},
    { 0, 22, 116, 164, "LINUX RELEASES"},
    { 0, 48, 76, 171, "PACE WILLISSON, HENDRIK WEIMER"},
    { 0, 22, 121, 183, "MAC RELEASES"},
    { 0, 48, 98, 190, "WILL GLYNN, SEAN FLYNN"},

    { 1, 22, 108, 12, "CREATION AND DESIGN"},
    { 1, 10, 123, 20, "FRITZ BRONNER"},
    { 1, 22, 128, 34, "PROGRAMMING"},
    { 1, 48, 116, 42, "MICHAEL K MCCARTY"},
    { 1, 22, 83, 54, "A.I. AND ADDITIONAL PROGRAMMING"},
    { 1, 48, 122, 62, "MORGAN ROARTY"},
    { 1, 22, 184, 74, "GRAPHIC MANAGEMENT"},
    { 1, 48, 193, 82, "TOM CHAMBERLAIN"},
    { 1, 22, 190, 93, "COMPUTER ARTISTS"},
    { 1, 48, 202, 101, "DAVID MOSHER"},
    { 1, 48, 198, 108, "MARGIE BEESON"},
    { 1, 48, 195, 115, "TODD J CAMASTA"},
    { 1, 48, 193, 122, "TOM CHAMBERLAIN"},
    { 1, 48, 199, 129, "CHERYL AUSTIN"},
    { 1, 48, 189, 136, "CHARLES H WEIDMAN"},
    { 1, 48, 205, 143, "RONAN JOYCE"},
    { 1, 48, 206, 150, "STEVE STIPP"},
    { 1, 48, 200, 157, "FRITZ BRONNER"},
    { 1, 48, 189, 164, "MICHAEL K MCCARTY"},
    { 1, 22, 201, 174, "SOUND EDITING"},
    { 1, 48, 200, 182, "FRITZ BRONNER"},
    { 1, 48, 189, 189, "MICHAEL K MCCARTY"},
    { 1, 22, 34, 74, "VIDEO DIGITIZING"},
    { 1, 48, 33, 82, "TOM CHAMBERLAIN"},
    { 1, 48, 40, 89, "FRITZ BRONNER"},
    { 1, 48, 29, 96, "MICHAEL K MCCARTY"},
    { 1, 48, 44, 103, "RONAN JOYCE"},
    { 1, 48, 38, 110, "MORGAN ROARTY"},
    { 1, 22, 9, 121, "ORIGINAL MUSIC COMPOSITION"},
    { 1, 48, 34, 129, "BRIAN LANGSBARD"},
    { 1, 48, 34, 136, "MICHAEL MCNERNY"},
    { 1, 22, 28, 146, "MUSIC ADAPTATIONS"},
    { 1, 48, 33, 154, "RICHARD JACKSON"},
    { 1, 22, 34, 165, "AUDIO DIGITIZING"},
    { 1, 48, 39, 173, "FRITZ BRONNER"},
    { 1, 48, 29, 180, "MICHAEL K MCCARTY"},
    { 1, 48, 35, 187, "CHARLES DEENEN"},

    { 2, 22, 52, 15, "DIRECTOR"},
    { 2, 48, 39, 23, "FRITZ BRONNER"},
    { 2, 22, 199, 10, "PRODUCED BY"},
    { 2, 48, 194, 18, "FRITZ BRONNER"},
    { 2, 48, 183, 25, "MICHAEL K MCCARTY"},
    { 2, 22, 47, 49, "DATA ENTRY"},
    { 2, 48, 33, 57, "TOM CHAMBERLAIN"},
    { 2, 48, 41, 64, "ENDA MCCARTY"},
    { 2, 22, 30, 75, "COSMONAUT VOICES"},
    { 2, 48, 35, 83, "GHERMAN TRIGUB"},
    { 2, 48, 39, 90, "MARINA TRIGUB"},
    { 2, 22, 41, 101, "OTHER VOICES"},
    { 2, 48, 35, 109, "GEORGE SPELVIN"},
    { 2, 48, 36, 116, "GENNIE SPELVIN"},
    { 2, 48, 49, 123, "JIM BROWN"},
    { 2, 22, 32, 134, "VECTOR GRAPHICS"},
    { 2, 48, 29, 142, "J SCOTT MATTHEWS"},
    { 2, 48, 32, 149, "MICHAEL PACKARD"},
    { 2, 22, 36, 160, "COVER PAINTING"},
    { 2, 48, 46, 168, "KEN HODGES"},
    { 2, 22, 31, 179, "SPACE MINIATURES"},
    { 2, 48, 33, 187, "BRONNER STUDIOS"},
    { 2, 22, 173, 47, "INTERPLAY'S PRODUCERS"},
    { 2, 48, 182, 55, "JACOB R BUCHERT III"},
    { 2, 48, 181, 62, "BRUCE SCHLICKBERND"},
    { 2, 48, 199, 69, "ALAN PAVLISH"},
    { 2, 48, 194, 76, "WILLIAM CHURCH"},
    { 2, 22, 169, 87, "DIRECTOR OF PLAYTESTING"},
    { 2, 48, 193, 95, "KERRY GARRISON"},
    { 2, 22, 187, 106, "LEAD PLAYTESTERS"},
    { 2, 48, 200, 114, "JEREMY AIREY"},
    { 2, 48, 191, 121, "MICHAEL PACKARD"},
    { 2, 48, 197, 128, "MARK HARRISON"},
    { 2, 48, 197, 135, "ERIK ANDERSON"},
    { 2, 48, 173, 142, "JENNIFER SCHLICKBERND"},
    { 2, 48, 201, 149, "JOHN SRAMEK"},
    { 2, 22, 167, 160, "DIR OF QUALITY ASSURANCE"},
    { 2, 48, 208, 168, "KIRK TOME"},
    { 2, 22, 210, 179, "SDL PORT"},
    { 2, 48, 194, 187, "PACE WILLISSON"},
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
    display::graphics.screen()->draw(countrySeals, 110 * plr, 0, 107, 95, 30, 85);

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
