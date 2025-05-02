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

// This file handles Luna, the Satellite Building, the LM Program screen, and has a function for displaying 'nauts

#include "ast0.h"

#include "display/graphics.h"
#include "display/palettized_surface.h"

#include "Buzz_inc.h"
#include "data.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "pace.h"
#include "place.h"
#include "sdlhelper.h"

#include <boost/format.hpp>
#include <iostream>
#include <sstream>

uint8_t MCol[MAX_ROSTER];
uint8_t sel[MAX_SELECTION];
uint8_t MaxSel;


void SatDraw(char plr);
void LMDraw(char plr);
void SatText(char plr);
void PlanText(char plr, char plan);
void LMPict(char poff);


/**
 * Get the color corresponding to the Astronaut/Cosmonaut's mood.
 *
 * /return  a color index for the standard part of the Port palette (0-31).
 */
int MoodColor(const uint8_t mood)
{
    if (mood >= 65) {
        return 16;
    } else if (mood >= 40) {
        return 11;
    } else if (mood >= 20) {
        return 8;
    } else {
        return 0;
    }
}


void Moon(char plr)
{
    int val;
    long size;
    helpText = "i029";
    keyHelpText = "k029";
    val = Data->P[plr].Misc[MISC_HW_PHOTO_RECON].Safety;
    FadeOut(2, 0, 0, 0);


    display::graphics.screen()->clear();
    ShBox(109, 24, 222, 167);
    InBox(113, 27, 218, 39);
    fill_rectangle(114, 28, 217, 38, 7 + 3 * plr);
    InBox(113, 130, 218, 146);
    fill_rectangle(114, 131, 217, 145, 7 + 3 * other(plr));
    IOBox(113, 150, 218, 164);

    size = ((val - 55) / 3);

    if (size > 13) {
        size = 13;
    } else if (size < 0) {
        size = 0;
    }

    char filename[128];
    snprintf(filename, sizeof(filename), "images/moon.but.%d.png", (int)size);
    boost::shared_ptr<display::PalettizedSurface> moonRecon(Filesystem::readImage(filename));
    moonRecon->exportPalette(128, 255);

    display::graphics.screen()->draw(moonRecon, 114, 43);
    InBox(113, 42, 218, 125);
    ShBox(113, 42, 143, 60);
    fill_rectangle(113, 42, 142, 59, 3);
    InBox(113, 42, 140, 58);
    display::graphics.setForegroundColor(11);
    draw_string(134, 35, "PHOTO RECON.");
    draw_string(129, 137, "CURRENT RECON ");
    draw_string(127, 143, "LEVEL IS AT ");
    draw_number(0, 0, val);
    draw_string(0, 0, "%");
    display::graphics.setForegroundColor(1);
    draw_string(143, 159, "CONTINUE");
    draw_small_flag(plr, 114, 43);

    music_start(M_HISTORY);
    FadeIn(2, 0, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 115 && y >= 152 && x <= 216 && y <= 162 && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
            InBox(115, 152, 216, 162);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            music_stop();
            key = 0;
            return; // Continue
        }
    }
}



void DispLeft(char plr, int lc, int cnt, int nw, const int *ary)
{
    int i, start, num;
    start = nw - lc;
    num = (cnt < 8) ? cnt : 8;

    fill_rectangle(26, 129, 153, 195, 0);
    ShBox(26, 130 + lc * 8, 152, 138 + lc * 8);

    display::graphics.setForegroundColor(11);

    for (i = start; i < start + num; i++) {
        if (ary[i] >= 0) {
            display::graphics.setForegroundColor(11 + 7 * Data->P[plr].Pool[ary[i]].Sex);

            if (Data->P[plr].Pool[ary[i]].RetirementDelay > 0) {
                display::graphics.setForegroundColor(9);  // Print name in gray if 'naut has announced retirement (black shows poorly here) -Leon
            }
            draw_string(28, 136 + (i - start) * 8, &Data->P[plr].Pool[ary[i]].Name[0]);

            if (Data->P[plr].Pool[ary[i]].InjuryDelay > 0) {
                if (Data->P[plr].Pool[ary[i]].InjuryDelay == 1) {
                    draw_number(120, 136 + (i - start) * 8, Data->P[plr].Pool[ary[i]].InjuryDelay);
                    draw_string(0, 0, " TURN");
                } else {
                    draw_number(118, 136 + (i - start) * 8, Data->P[plr].Pool[ary[i]].InjuryDelay);
                    draw_string(0, 0, " TRNS");
                }
            } else if (Data->P[plr].Pool[ary[i]].Missions > 0) {
                draw_string(0, 0, " (");
                draw_number(0, 0, Data->P[plr].Pool[ary[i]].Missions);
                draw_string(0, 0, ")");
            }
        }
    }

    return;
}

void BarSkill(char plr, int lc, int nw, int *ary)
{
    display::graphics.setForegroundColor(11);

    if (Data->P[plr].Pool[ary[nw]].Sex == 1) {
        display::graphics.setForegroundColor(18);    // Print in blue if 'naut is female
    }

    grMoveTo(28, 136 + lc * 8);
    // CA LM EV DO EN
    draw_string(0, 0, "CA:");
    draw_number(0, 0, Data->P[plr].Pool[ary[nw]].Cap);
    draw_string(0, 0, " LM:");
    draw_number(0, 0, Data->P[plr].Pool[ary[nw]].LM);
    draw_string(0, 0, " EV:");
    draw_number(0, 0, Data->P[plr].Pool[ary[nw]].EVA);
    draw_string(0, 0, " DO:");
    draw_number(0, 0, Data->P[plr].Pool[ary[nw]].Docking);
    draw_string(0, 0, " EN:");
    draw_number(0, 0, Data->P[plr].Pool[ary[nw]].Endurance);
    return;
}


void SatDraw(char plr)
{
    int i;
    int loc[4];

    FadeOut(2, 0, 0, 0);

    display::graphics.screen()->clear();
    ShBox(1, 0, 319, 22);
    IOBox(243, 3, 316, 19);
    InBox(4, 3, 31, 19);

    for (i = 0; i < 4; i++) {
        ShBox(1 + i * 80, 24, 79 + i * 80, 71);
        ShBox(1 + i * 80, 73, 79 + i * 80, 199);

        if (i == 2) {
            InBox(4 + i * 80, 27, 76 + i * 80, 56);
            fill_rectangle(5 + i * 80, 28, 75 + i * 80, 55, 0);
            InBox(164, 58, 236, 68);
            fill_rectangle(165, 59, 235, 67, 0);
            ShBox(166, 60, 199, 66);
            ShBox(201, 60, 234, 66);
        } else {
            InBox(4 + i * 80, 27, 76 + i * 80, 68);
            fill_rectangle(5 + i * 80, 28, 75 + i * 80, 67, 7);
        }
    }

    loc[0] = (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num < 0) ? 0 : 1 + plr * 3;
    loc[1] = (Data->P[plr].Probe[PROBE_HW_ORBITAL].Num < 0) ? 0 : 2 + plr * 3;
    loc[2] = (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num < 0) ? 0 : 0;
    loc[3] = (Data->P[plr].Probe[PROBE_HW_LUNAR].Num < 0) ? 0 : 3 + plr * 3;

    for (i = 0; i < 4; i++) {

        char filename[128];
        snprintf(filename, sizeof(filename), "images/satbld.but.%d.png", loc[i]);
        boost::shared_ptr<display::PalettizedSurface> satellite(Filesystem::readImage(filename));
        satellite->exportPalette();

        if (i != 2) {
            display::graphics.screen()->draw(satellite, 5 + i * 80, 28);
        } else {
            display::graphics.screen()->draw(satellite, 0, 0, 71, 28, 5 + i * 80, 28);
        }

    }

    draw_small_flag(plr, 5, 4);
    display::graphics.setForegroundColor(11);
    grMoveTo(180, 63);
    grLineTo(184, 63);
    grMoveTo(215, 63);
    grLineTo(219, 63);
    grMoveTo(217, 61);
    grLineTo(217, 65);
    draw_heading(40, 5, "SATELLITE BUILDING", 0, -1);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    return;
}


void LMDraw(char plr)
{
    char ind = 0;
    FadeOut(2, 0, 0, 0);

    display::graphics.screen()->clear();
    ShBox(1, 0, 319, 22);
    IOBox(243, 3, 316, 19);
    InBox(4, 3, 31, 19);
    ShBox(1, 24, 319, 118);
    InBox(4, 26, 316, 116);
    fill_rectangle(5, 27, 315, 115, 0);  // middle screen

    if (Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Num >= 0) {
        ind = 4 + plr;
    } else {
        ind = 0 + plr;
    }

    LMPict(ind);

    if (Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE].Num >= 0) {
        ind = 6 + plr;
    } else {
        ind = 2 + plr;
    }

    LMPict(ind);
    ShBox(110, 24, 203, 36);
    InBox(112, 26, 201, 34);
    display::graphics.setForegroundColor(1);
    draw_string(117, 32, "CENTRAL HANGAR");
    draw_small_flag(plr, 5, 4);
    draw_heading(50, 5, "LUNAR MODULE", 0, -1);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    return;
}

void SatText(char plr)
{
    int i;

    display::graphics.setForegroundColor(11);

    for (i = 0; i < 4; i++) {
        if (i != 2) {
            display::graphics.setForegroundColor(11);

            switch (i) {
            case 0:
                draw_string(5 + i * 80, 80, "DUR LVL: ");  // Show highest Duration level achieved -Leon

                switch (Data->P[plr].DurationLevel) {
                case 1:
                    draw_string(0, 0, "A");
                    break;

                case 2:
                    draw_string(0, 0, "B");
                    break;

                case 3:
                    draw_string(0, 0, "C");
                    break;

                case 4:
                    draw_string(0, 0, "D");
                    break;

                case 5:
                    draw_string(0, 0, "E");
                    break;

                case 6:
                    draw_string(0, 0, "F");
                    break;

                default:
                    draw_string(0, 0, "NONE");
                    break;
                }

                draw_string(5 + i * 80, 94, "DOCKING");

                if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num >= 0) {
                    draw_number(5 + i * 80, 110, Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety);
                } else {
                    draw_number(5 + i * 80, 110, 0);
                }

                draw_string(0, 0, "%");

                draw_number(5 + i * 80, 127, Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Steps);

                draw_number(5 + i * 80, 144, Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Steps - Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Failures);

                if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Steps > 0) {
                    draw_number(5 + i * 80, 161, 100 * (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Steps - Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Failures) / Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Steps);
                } else {
                    draw_number(5 + i * 80, 161, 0);
                }

                if (Data->Prestige[Prestige_MannedDocking].Place == 0) {
                    draw_string(5 + i * 80, 178, "U.S.");
                } else if (Data->Prestige[Prestige_MannedDocking].Place == 1) {
                    draw_string(5 + i * 80, 178, "SOVIET");
                } else {
                    draw_string(5 + i * 80, 178, "NONE");
                }

                draw_number(5 + i * 80, 195, Data->Prestige[Prestige_MannedDocking].Points[plr]);
                break;

            case 1:
                draw_string(5 + i * 80, 80, "ORBITAL SAT.");

                if (Data->P[plr].Probe[PROBE_HW_ORBITAL].Num >= 0) {
                    draw_number(5 + i * 80, 110, Data->P[plr].Probe[PROBE_HW_ORBITAL].Safety);
                } else {
                    draw_number(5 + i * 80, 110, 0);
                }

                draw_string(0, 0, "%");

                draw_number(5 + i * 80, 127, Data->P[plr].Probe[PROBE_HW_ORBITAL].Used);

                draw_number(5 + i * 80, 144, Data->P[plr].Probe[PROBE_HW_ORBITAL].Used - Data->P[plr].Probe[PROBE_HW_ORBITAL].Failures);

                if (Data->P[plr].Probe[PROBE_HW_ORBITAL].Used > 0) {
                    draw_number(5 + i * 80, 161, 100 * (Data->P[plr].Probe[PROBE_HW_ORBITAL].Used - Data->P[plr].Probe[PROBE_HW_ORBITAL].Failures) / Data->P[plr].Probe[PROBE_HW_ORBITAL].Used);
                } else {
                    draw_number(5 + i * 80, 161, 0);
                }

                if (Data->Prestige[Prestige_OrbitalSatellite].Place == 0) {
                    draw_string(5 + i * 80, 178, "U.S.");
                } else if (Data->Prestige[Prestige_OrbitalSatellite].Place == 1) {
                    draw_string(5 + i * 80, 178, "SOVIET");
                } else {
                    draw_string(5 + i * 80, 178, "NONE");
                }

                draw_number(5 + i * 80, 195, Data->Prestige[Prestige_OrbitalSatellite].Points[plr]);
                break;


            case 3:
                draw_string(5 + i * 80, 80, "LUNAR PROBE");

                if (Data->P[plr].Probe[PROBE_HW_LUNAR].Num >= 0) {
                    draw_number(5 + i * 80, 110, Data->P[plr].Probe[PROBE_HW_LUNAR].Safety);
                } else {
                    draw_number(5 + i * 80, 110, 0);
                }

                draw_string(0, 0, "%");

                draw_number(5 + i * 80, 127, Data->P[plr].Probe[PROBE_HW_LUNAR].Used);

                draw_number(5 + i * 80, 144, Data->P[plr].Probe[PROBE_HW_LUNAR].Used - Data->P[plr].Probe[PROBE_HW_LUNAR].Failures);

                if (Data->P[plr].Probe[PROBE_HW_LUNAR].Used > 0) {
                    draw_number(5 + i * 80, 161, 100 * (Data->P[plr].Probe[PROBE_HW_LUNAR].Used - Data->P[plr].Probe[PROBE_HW_LUNAR].Failures) / Data->P[plr].Probe[PROBE_HW_LUNAR].Used);
                } else {
                    draw_number(5 + i * 80, 161, 0);
                }

                if (Data->Prestige[Prestige_LunarProbeLanding].Place == 0) {
                    draw_string(5 + i * 80, 178, "U.S.");
                } else if (Data->Prestige[Prestige_LunarProbeLanding].Place == 1) {
                    draw_string(5 + i * 80, 178, "SOVIET");
                } else {
                    draw_string(5 + i * 80, 178, "NONE");
                }

                draw_number(5 + i * 80, 195, Data->Prestige[Prestige_LunarProbeLanding].Points[plr]);
                break;

            }
        } else {
            PlanText(plr, 0);
        }

        display::graphics.setForegroundColor(6 + 3 * plr);
        draw_string(5 + i * 80, 103, "SAFETY:");
        draw_string(5 + i * 80, 120, "ATTEMPTS:");
        draw_string(5 + i * 80, 137, "SUCCESSFUL:");
        draw_string(5 + i * 80, 154, "% SUCCESS:");
        draw_string(5 + i * 80, 171, "FIRST:");
        draw_string(5 + i * 80, 188, "PRESTIGE:");

    }


    return;
}

void PlanText(char plr, char plan)
{
    char tx;
    int pUsed, pFails, Find, i;

    Find = 0; /* XXX check uninitialized */

    tx = (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num >= 0) ? 1 : 0;

    fill_rectangle(164, 75, 78 + 160, 89, 3);
    fill_rectangle(162, 105, 218, 113, 3);
    fill_rectangle(162, 123, 218, 131, 3);
    fill_rectangle(162, 157, 218, 165, 3);
    fill_rectangle(162, 173, 218, 181, 3);
    fill_rectangle(162, 190, 218, 198, 3);

    fill_rectangle(162, 139, 218, 145, 3);

    if (tx == 1) {
        fill_rectangle(165, 28, 235, 55, 0);
    }

    display::graphics.setForegroundColor(11);

    switch (plan) {
    case 0:
        draw_string(4 + 160, 80, "LUNAR FLYBY");
        display::graphics.setForegroundColor(11);
        draw_string(4 + 160, 87, "RECON: ");
        draw_number(0, 0, Data->P[plr].Misc[MISC_HW_PHOTO_RECON].Safety);
        draw_string(0, 0, "%");
        Find = Mission_LunarFlyby;

        if (tx == 1) {
            SmHardMe(plr, 190, 34, 6, 6, 32);
        }

        break;

    case 1:
        draw_string(4 + 160, 80, "MERCURY FLYBY");
        SmHardMe(plr, 190, 34, 6, 1, 32);
        Find = Mission_MercuryFlyby;
        break;

    case 2:
        draw_string(4 + 160, 80, "VENUS FLYBY");
        SmHardMe(plr, 190, 34, 6, 2, 32);
        Find = Mission_VenusFlyby;
        break;

    case 3:
        draw_string(4 + 160, 80, "MARS FLYBY");
        SmHardMe(plr, 190, 34, 6, 3, 32);
        Find = Mission_MarsFlyby;
        break;

    case 4:
        draw_string(4 + 160, 80, "JUPITER FLYBY");
        SmHardMe(plr, 190, 34, 6, 4, 32);
        Find = Mission_JupiterFlyby;
        break;

    case 5:
        draw_string(4 + 160, 80, "SATURN FLYBY");
        SmHardMe(plr, 190, 34, 6, 5, 32);
        Find = Mission_SaturnFlyby;
        break;
    }

    pUsed = pFails = 0;

    for (i = 0; i < Data->P[plr].PastMissionCount; i++) {
        if (Data->P[plr].History[i].MissionCode == Find) {
            pUsed++;

            if (Data->P[plr].History[i].spResult != 1) {
                pFails++;
            }
        }
    }

    draw_number(5 + 160, 110, Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety * tx);
    draw_string(0, 0, "%");


    draw_number(5 + 160, 127, pUsed);
    draw_number(5 + 160, 144, pUsed - pFails);

    if (pUsed) {
        draw_number(5 + 160, 161, 100 * (pUsed - pFails) / pUsed);
    } else {
        draw_number(5 + 160, 161, 0);
    }

    if (Data->Prestige[1 + plan].Place == 0) {
        draw_string(5 + 160, 178, "U.S.");
    } else if (Data->Prestige[1 + plan].Place == 1) {
        draw_string(5 + 160, 178, "SOVIET");
    } else {
        draw_string(5 + 160, 178, "NONE");
    }

    draw_number(5 + 160, 195, Data->Prestige[1 + plan].Points[plr]);


    return;
}



void LMPict(char poff)
{
    char filename[128];
    snprintf(filename, sizeof(filename), "images/lmer.but.%d.png", poff);
    boost::shared_ptr<display::PalettizedSurface> lunarModule(Filesystem::readImage(filename));
    lunarModule->exportPalette(32, 255);

    if (poff == 0 || poff == 1 || poff == 4 || poff == 5) {
        display::graphics.screen()->draw(lunarModule, 5, 27);
    } else {
        display::graphics.screen()->draw(lunarModule, 160, 27);
    }
}

void LMBld(char plr)
{
    LMDraw(plr);

    ShBox(1, 120, 157, 168);
    InBox(4, 122, 154, 166);
    ShBox(160, 120, 319, 168);
    InBox(163, 122, 316, 166);
    ShBox(1, 170, 319, 194);
    IOBox(41, 172, 279, 193);
    draw_heading(71, 176, "TOTAL LM POINTS", 0, -1);

    if (Data->P[plr].LMpts > 999) {
        draw_heading(240, 176, "999+", 0, -1);
    } else {
        char points[5];
        snprintf(&points[0], sizeof(points), "%d", Data->P[plr].LMpts);
        draw_heading(240, 176, &points[0], 0, -1);
    }

    for (int i = 0; i < 2; i++) {
        int m = i ? 12 : 172;

        display::graphics.setForegroundColor(8);
        draw_string(m, 130, &Data->P[plr].Manned[5 + i].Name[0]);
        display::graphics.setForegroundColor(9);
        display::graphics.setForegroundColor(6);
        draw_string(m + 66, 130, "AVOID FAIL: ");
        display::graphics.setForegroundColor(11);

        if (Data->P[plr].Manned[5 + i].SaveCard > 0) {
            draw_string(m + 124, 130, "YES");
        } else {
            draw_string(m + 126, 130, "NO");
        }

        display::graphics.setForegroundColor(6);
        draw_string(m, 138, "SAFETY FACTOR: ");
        display::graphics.setForegroundColor(1);
        draw_number(0, 0, (Data->P[plr].Manned[5 + i].Num >= 0) ? Data->P[plr].Manned[5 + i].Safety : 0);
        draw_string(0, 0, "%");
        display::graphics.setForegroundColor(6);
        draw_string(m, 146, "CURRENT QUANTITY: ");
        display::graphics.setForegroundColor(1);
        draw_number(0, 0, (Data->P[plr].Manned[5 + i].Num >= 0) ? Data->P[plr].Manned[5 + i].Num : 0);

        display::graphics.setForegroundColor(6);
        draw_string(m, 154, "NUMBER OF ATTEMPTS: ");
        display::graphics.setForegroundColor(1);
        draw_number(0, 0, Data->P[plr].Manned[5 + i].Steps);
        display::graphics.setForegroundColor(6);
        draw_string(m, 162, "SUCCESSFUL ATTEMPTS: ");
        display::graphics.setForegroundColor(1);
        draw_number(0, 0, Data->P[plr].Manned[5 + i].Steps - Data->P[plr].Manned[5 + i].Failures);
    }

    music_start(M_HISTORY);
    FadeIn(2, 0, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            music_stop();
            key = 0;
            return;  // Continue
        }

        if (x >= 41 && y >= 172 && x <= 314 && y <= 193 && mousebuttons > 0) {
            OutBox(41, 172, 279, 193);
            delay(100);
            WaitForMouseUp();
            InBox(41, 172, 279, 193);
            Help("i172");  // Show help item explaining the LM Points system
        }
    }
}

void SatBld(char plr)
{
    int plan;
    plan = 0;
    SatDraw(plr);
    SatText(plr);
    helpText = "i019";
    keyHelpText = "k019";

    if (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num >= 0) {
        PlanText(plr, plan);
    }

    music_start(M_HISTORY);
    FadeIn(2, 0, 0, 0);

    key = 0;
    WaitForMouseUp();

    while (1) {
        GetMouse();

        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            // OutBox(245,5,314,17);
            music_stop();
            key = 0;
            return; // Continue
        } else if (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num >= 0 && ((x >= 166 && y >= 60 && x <= 199 && y <= 66 && mousebuttons > 0) || key == LT_ARROW)) {
            InBox(166, 60, 199, 66);
            key = 0;
            WaitForMouseUp();
            delay(10);
            key = 0;

            if (plan == 0) {
                plan = 5;
            } else {
                plan--;
            }

            PlanText(plr, plan);
            OutBox(166, 60, 199, 66);
        } else if (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num >= 0 && ((x >= 201 && y >= 60 && x <= 234 && y <= 66 && mousebuttons > 0) || key == RT_ARROW)) {
            InBox(201, 60, 234, 66);
            key = 0;
            WaitForMouseUp();
            delay(10);
            key = 0;

            if (plan > 4) {
                plan = 0;
            } else {
                plan++;
            }

            PlanText(plr, plan);
            OutBox(201, 60, 234, 66);
        }
    }
}


// EOF
