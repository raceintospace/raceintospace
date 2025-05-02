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
/** \file endgame.cpp End Game Routines
 */

// This file shows the End Game screen, intuitively enough.  It also shows the Prestige First window.

#include "endgame.h"

#include <string>

#include <boost/shared_ptr.hpp>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "aipur.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "fireworks.h"
#include "game_main.h"
#include "gr.h"
#include "mis_c.h"
#include "mission_util.h"
#include "newmis.h"
#include "pace.h"
#include "pbm.h"
#include "place.h"
#include "replay.h"
#include "sdlhelper.h"
#include "start.h"


char PF[29][40] = {
    "ORBITAL SATELLITE", "LUNAR FLYBY", "MERCURY FLYBY", "VENUS FLYBY",
    "MARS FLYBY", "JUPITER FLYBY", "SATURN FLYBY", "LUNAR PROBE LANDING",
    "DURATION LEVEL F", "DURATION LEVEL E", "DURATION LEVEL D",
    "DURATION LEVEL C", "DURATION LEVEL B", "ONE-PERSON CRAFT",
    "TWO-PERSON CRAFT", "THREE-PERSON CRAFT", "MINISHUTTLE", "FOUR-PERSON CRAFT",
    "MANNED ORBITAL", "MANNED LUNAR PASS", "MANNED LUNAR ORBIT",
    "MANNED RESCUE ATTEMPT", "MANNED LUNAR LANDING", "ORBITING LAB",
    "MANNED DOCKING", "WOMAN IN SPACE", "SPACEWALK", "MANNED SPACE MISSION"
};


char Burst(char win);
void EndGame(char win, char pad);
void Load_LenFlag(char win);
void Draw_NewEnd(char win);
void FakeHistory(char plr, char Fyear);
std::string HistFile(unsigned char bud);
void PrintHist(const char *buf);
void PrintOne(const char *buf, char tken);
void AltHistory(char plr);
void EndPict(int x, int y, char poff, unsigned char coff);
void LoserPict(char poff, unsigned char coff);

/**
 * Control loop for post-game celebration.
 *
 * Illustrates the endgame fireworks during the victory celebration.
 *
 * \param win  Country index of the winning side (0 for USA, 1 for USSR).
 * \return     Menu code for the player's choice in the control loop.
 */
char Burst(char win)
{
    char R_value = 0;

    helpText = "i144";
    keyHelpText = "k044";

    Fireworks animation((int)win);

    while (1) {
        animation.step();

        /* We can't wait 30 ms on default timer */
        key = 0;
        GetMouse();

        if (key > 0 || mousebuttons > 0) {
            if ((x >= 14 && y >= 182 && x <= 65 && y <= 190
                 && mousebuttons > 0) || key == 'H') {
                R_value = 1;
            } else if ((x >= 74 && y >= 182 && x <= 125 && y <= 190
                        && mousebuttons > 0) || key == 'S') {
                R_value = 2;
            } else if ((x >= 134 && y >= 182 && x <= 185 && y <= 190
                        && mousebuttons > 0) || key == 'P') {
                R_value = 3;
            } else if ((x >= 194 && y >= 182 && x <= 245 && y <= 190
                        && mousebuttons > 0) || key == 'M') {
                R_value = 4;
            } else if ((x >= 254 && y >= 182 && x <= 305 && y <= 190
                        && mousebuttons > 0) || key == K_ENTER || key == 'E') {
                R_value = 5;
            }

            if (R_value > 0) {
                animation.clear();
                return (R_value);
            }
        }
    }  // end while
}

void EndGame(char win, char pad)
{
    int i = 0, r;
    char miss, prog, man1, man2, man3, man4, bud;
    char month, firstOnMoon, capName[30];

    FadeOut(2, 10, 0, 0);
    helpText = "i000";
    keyHelpText = "k000";
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(242, 3, 315, 19);
    ShBox(0, 24, 319, 199);
    fill_rectangle(5, 28, 314, 195, 0);
    fill_rectangle(5, 105, 239, 110, 3);
    ShBox(101, 102, 218, 113);
    display::graphics.setForegroundColor(6);
    draw_string(112, 109, "ALTERNATE HISTORY");

    if (win == 0) {
        draw_heading(45, 5, "US WINS", 1, -1);
    } else {
        draw_heading(45, 5, "USSR WINS", 1, -1);
    }

    draw_small_flag(win, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(256, 13, "CONTINUE");

    if (Option == -1 && MAIL == -1) {
        miss = Data->P[win].Mission[pad].MissionCode;
    } else {
        miss = Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].MissionCode;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 50, "MISSION TYPE: ");
    display::graphics.setForegroundColor(8);

    if (miss == Mission_Jt_LunarLanding_EOR ||
        miss == Mission_Jt_LunarLanding_LOR ||
        miss == Mission_Soyuz_LL) {
        i = 1;
    } else {
        i = 0;
    }

    DrawMissionName(miss, 80, 50, 24);
	
    if (Option == -1 && MAIL == -1) {
        strcpy(capName, Data->P[win].Mission[pad].Name);
        month = Data->P[win].Mission[pad].Month;
    } else {
        month = Data->Prestige[Prestige_MannedLunarLanding].Month;

        if (MAIL != -1 || Option == win) {
            strcpy(capName, Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].MissionName[0]);
        } else {
            prog = Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].Hard[i][0] + 1;
            strcpy(capName, &Data->P[win].Manned[prog - 1].Name[0]);
            strcat(capName, " ");
            strncat(capName, RomanNumeral(Data->P[win].Manned[prog - 1].Used + 1).c_str(), 8);
        }
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 40, "MISSION: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, capName);
    display::graphics.setForegroundColor(6);
    draw_string(0, 0, "  -  ");
    display::graphics.setForegroundColor(8);
    draw_number(0, 0, dayOnMoon);
    draw_string(0, 0, " ");
    draw_string(0, 0, Month[month]);
    draw_string(0, 0, "19");
    draw_number(0, 0, Data->Year);

    // correct mission pictures
    int historyIndex;

    if (Option == -1 && MAIL == -1) {
        historyIndex = Data->P[win].PastMissionCount - 1;
    } else {
        historyIndex = Data->Prestige[Prestige_MannedLunarLanding].Index;
    }

    struct PastInfo &historyEntry = Data->P[win].History[historyIndex];

    if (win == 1 && historyEntry.Hard[i][0] >= 3) {
        bud = 5;
    } else if (win == 0 && historyEntry.Hard[i][0] == 4) {
        bud = 2;
    } else {
        bud = ((historyEntry.Hard[i][2] - 5) + (win * 3));
    }

    if (bud < 0 || bud > 5) {
        bud = 0 + win;
    }

    InBox(241, 67, 313, 112);
    EndPict(242, 68, bud, 128);
    PatchMe(win, 270, 34, historyEntry.Hard[i][0],
            historyEntry.Patch[win]);
    man1 = historyEntry.Man[i][0];
    man2 = historyEntry.Man[i][1];
    man3 = historyEntry.Man[i][2];
    man4 = historyEntry.Man[i][3];
// no astronaut kludge
    r = Data->P[win].AstroCount;

    if (man1 <= -1) {
        man1 = brandom(r);
    }

    if (man2 <= -1) {
        man2 = brandom(r);
    }

    if (man3 <= -1) {
        man3 = brandom(r);
    }

    if (man4 <= -1) {
        man4 = brandom(r);
    }

    if (!(Option == -1 || Option == win)) {
        historyEntry.Man[i][0] = man1;
        historyEntry.Man[i][1] = man2;
        historyEntry.Man[i][2] = man3;
        historyEntry.Man[i][3] = man4;
    }

    prog = historyEntry.Hard[i][0] + 1;

    for (i = 1; i < 5; i++) {
        display::graphics.setForegroundColor(6);

        switch (i) {
        case 1:
            if (prog == 1) {
                draw_string(10, 70, "CAPSULE PILOT - EVA: ");
            } else if (prog == 2) {
                draw_string(10, 70, "CAPSULE PILOT - DOCKING: ");
            } else if (prog >= 3) {
                draw_string(10, 70, "COMMAND PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man1].Sex == 1) {
                display::graphics.setForegroundColor(14);  // Show females in orange
            }

            if (man1 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man1].Name[0]);
                display::graphics.setForegroundColor(8);
            }

            break;

        case 2:
            if (prog > 1 && prog < 5) {
                draw_string(10, 79, "LM PILOT - EVA: ");
            } else if (prog == 5) {
                draw_string(10, 79, "LUNAR PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man2].Sex == 1) {
                display::graphics.setForegroundColor(14);  // Show females in orange
            }

            if (man2 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man2].Name[0]);
                display::graphics.setForegroundColor(8);
            }

            break;

        case 3:
            if (prog > 2 && prog < 5) {
                draw_string(10, 88, "DOCKING SPECIALIST: ");
            } else if (prog == 5) {
                draw_string(10, 88, "EVA SPECIALIST: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man3].Sex == 1) {
                display::graphics.setForegroundColor(14);  // Show females in orange
            }

            if (man3 != -1 && prog > 2) {
                draw_string(0, 0, &Data->P[win].Pool[man3].Name[0]);
                display::graphics.setForegroundColor(8);
            }

            break;

        case 4:
            if (prog == 5) {
                draw_string(10, 97, "EVA SPECIALIST: ");
                display::graphics.setForegroundColor(8);

                if (Data->P[win].Pool[man4].Sex == 1) {
                    display::graphics.setForegroundColor(14);  // Show females in orange
                }

                if (man4 != -1) {
                    draw_string(0, 0, &Data->P[win].Pool[man4].Name[0]);
                    display::graphics.setForegroundColor(8);
                }
            }

            break;

        default:
            break;
        }
    }

    // Show the first to walk on the Moon
    firstOnMoon = (manOnMoon == 1 ? man1 : manOnMoon == 2 ? man2 : manOnMoon == 3 ? man3 : manOnMoon == 4 ? man4 : man2);
    display::graphics.setForegroundColor(11);
    draw_string(10, 60, "FIRST ON THE MOON: ");
    display::graphics.setForegroundColor(15);

    if (Data->P[win].Pool[firstOnMoon].Sex == 1) {
        display::graphics.setForegroundColor(17);  // Show females in light green
    }

    draw_string(0, 0, &Data->P[win].Pool[firstOnMoon].Name[0]);

    display::graphics.setForegroundColor(6);
    AltHistory(win);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    while (true) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            key = 0;
            OutBox(244, 5, 313, 17);
            break;
        }
    }

    return;
}

/**
 * Display a patriotic image indicating the winner's side.
 *
 * The image displayed for the winning side is:
 *   0: American flag
 *   1: Statue of Lenin
 *
 * This image uses a 128-color palette located at [128, 255], which
 * is exported to the main display.
 *
 * \param win  the winning player (0 for USA, 1 for USSR)
 * \throws runtime_error  if Filesystem cannot load the image
 */
void Load_LenFlag(char win)
{
    int x, y;
    std::string filename;

    if (win == 1) {
        filename = "images/lenin.png";
        x = 224;
        y = 26;
    } else {
        filename = "images/flagger.png";
        x = 195;
        y = 0;
    }

    boost::shared_ptr<display::PalettizedSurface> image(
        Filesystem::readImage(filename));
    image->exportPalette(128, 255);
    display::graphics.screen()->draw(image, x, y);
}

void Draw_NewEnd(char win)
{
    music_start(M_VICTORY);

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();

    boost::shared_ptr<display::PalettizedSurface> winner(Filesystem::readImage("images/winner.but.0.png"));
    winner->exportPalette(0, 128);
    display::graphics.screen()->draw(winner, 0, 0);

    ShBox(0, 173, 319, 199);
    InBox(5, 178, 314, 194);
    IOBox(12, 180, 67, 192);
    IOBox(72, 180, 127, 192);
    IOBox(132, 180, 187, 192);
    IOBox(192, 180, 247, 192);
    IOBox(252, 180, 307, 192);
    display::graphics.setForegroundColor(9);
    draw_string(21, 188, "H");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ISTORY");
    display::graphics.setForegroundColor(9);
    draw_string(85, 188, "S");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "TATS");
    display::graphics.setForegroundColor(9);
    draw_string(142, 188, "P");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ARADE");
    display::graphics.setForegroundColor(9);
    draw_string(198, 188, "M");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "OON EVA");
    display::graphics.setForegroundColor(9);
    draw_string(270, 188, "E");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "XIT");
    FadeIn(0, 10, 128, 0);
    Load_LenFlag(win);
    FadeIn(1, 40, 128, 1);
}

void NewEnd(char win, char loc)
{
    int i, Re_Draw = 0;
    char R_V = 0;

    music_start(M_VICTORY);
    EndGame(win, loc);
    Draw_NewEnd(win);
    R_V = Burst(win);
    WaitForMouseUp();
    i = 0;
    key = 0;
    display::LegacySurface local(162, 92);
    local.clear(0);

    while (i == 0) {
        key = 0;
        GetMouse();
        helpText = "i144";
        keyHelpText = "k044";

        music_start(M_VICTORY);

        if (((key == 'P' || key == 'M' || key == 'H' || key == 'S') || mousebuttons > 0) || R_V == 0)
            if (Re_Draw == 1) {
                if ((x >= 14 && y >= 182 && x <= 65 && y <= 190 && mousebuttons > 0) || key == 'H') {
                    R_V = 1;
                }

                if ((x >= 74 && y >= 182 && x <= 125 && y <= 190 && mousebuttons > 0) || key == 'S') {
                    R_V = 2;
                }

                if ((x >= 134 && y >= 182 && x <= 185 && y <= 190 && mousebuttons > 0) || key == 'P') {
                    R_V = 3;
                }

                if ((x >= 194 && y >= 182 && x <= 245 && y <= 190 && mousebuttons > 0) || key == 'M') {
                    R_V = 4;
                }

                if ((x >= 254 && y >= 182 && x <= 305 && y <= 190 && mousebuttons > 0) || key == K_ENTER) {
                    R_V = 5;
                }

                local.copyTo(display::graphics.legacyScreen(), 149, 9);
                {
                    display::AutoPal p(display::graphics.legacyScreen());
                    memset(&p.pal[384], 0, 384);
                }
                local.clear(0);
                Load_LenFlag(win);
                FadeIn(1, 40, 128, 1);

                if (R_V == 0 || R_V == -1) {
                    R_V = Burst(win);
                }

                Re_Draw = 0;
                helpText = "i144";
                keyHelpText = "k044";
            }

        if (((x >= 14 && y >= 182 && x <= 65 && y <= 190 && mousebuttons > 0) || key == 'H') || R_V == 1) {
            // History box
            InBox(14, 182, 65, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            i = 0;
            key = 0;
            OutBox(14, 182, 65, 190);
            EndGame(win, loc);
            Draw_NewEnd(win);
            helpText = "i144";
            keyHelpText = "k044";
            R_V = Burst(win);
        }

        if (((x >= 74 && y >= 182 && x <= 125 && y <= 190 && mousebuttons > 0) || key == 'S') || R_V == 2) {
            // Stats box
            music_stop();
            InBox(74, 182, 125, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            i = 0;
            key = 0;
            OutBox(74, 182, 125, 190);
            music_start(M_THEME);
            Stat(win);
            Draw_NewEnd(win);
            helpText = "i144";
            keyHelpText = "k044";
            R_V = Burst(win);
        }

        if (((x >= 134 && y >= 182 && x <= 185 && y <= 190 && mousebuttons > 0) || key == 'P') || R_V == 3) {
            // Parade
            music_stop();
            InBox(134, 182, 185, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            if (R_V == 3) {
                R_V = -1;
            }

            i = 0;
            key = 0;
            Re_Draw = 1;
            OutBox(134, 182, 185, 190);
            FadeOut(1, 40, 128, 1);
            fill_rectangle(195, 0, 319, 172, 0);
            local.copyFrom(display::graphics.legacyScreen(), 149, 9, 309, 100);
            ShBox(149, 9, 309, 100);
            InBox(153, 13, 305, 96);
            music_start(M_PRGMTRG);
            // TODO: Sequence variation for the parade
            Replay(win, 0, 154, 14, 149, 82, (win == 0) ? "310UPAR" : "520SPAR");
            music_stop();
            helpText = "i144";
            keyHelpText = "k044";
        }

        if (((x >= 194 && y >= 182 && x <= 245 && y <= 190 && mousebuttons > 0) || key == 'M') || R_V == 4) {
            // Moon EVA
            music_stop();
            InBox(194, 182, 245, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            if (R_V == 4) {
                R_V = -1;
            }

            i = 0;
            key = 0;
            OutBox(194, 182, 245, 190);
            Re_Draw = 1;
            FadeOut(1, 40, 128, 1);
            fill_rectangle(195, 0, 319, 172, 0);
            local.copyFrom(display::graphics.legacyScreen(), 149, 9, 309, 100);
            ShBox(149, 9, 309, 100);
            InBox(153, 13, 305, 96);
            music_start(M_MISSPLAN);
            // TODO: show the actual lunar EVA
            Replay(win, 0, 154, 14, 149, 82, (win == 0) ? "520PUM3C6" : "121PSM3C6");
            music_stop();
            helpText = "i144";
            keyHelpText = "k044";
        }

        if (((x >= 254 && y >= 182 && x <= 305 && y <= 190 && mousebuttons > 0) || key == K_ENTER) || R_V == 5) {
            music_stop();
            InBox(254, 182, 305, 190);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(254, 182, 305, 190);
        }
    }
}

void FakeWin(char win)
{
    int i, r;
    char miss, prog, man1, man2, man3, man4, bud, yr, monthWin;
    monthWin = brandom(12);

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(242, 3, 315, 19);
    ShBox(0, 24, 319, 199);
    fill_rectangle(5, 28, 314, 195, 0);
    fill_rectangle(5, 105, 239, 110, 3);
    ShBox(101, 102, 218, 113);
    display::graphics.setForegroundColor(6);
    draw_string(112, 110, "ALTERNATE HISTORY");

    if (win == 0) {
        draw_heading(36, 5, "US WINS", 1, -1);
    } else {
        draw_heading(36, 5, "USSR WINS", 1, -1);
    }

    draw_small_flag(win, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    r = brandom(100);

    if (r < 45) {
        miss = Mission_HistoricalLanding;
    } else if (r < 50) {
        miss = Mission_DirectAscent_LL;
    } else if (r < 85) {
        miss = Mission_Jt_LunarLanding_EOR;
    } else {
        miss = Mission_Jt_LunarLanding_LOR;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 50, "MISSION TYPE: ");
    display::graphics.setForegroundColor(8);

    DrawMissionName(miss, 80, 50, 24);
    display::graphics.setForegroundColor(6);

    if (Data->Year <= 65) {
        r = 65 + brandom(5);
    } else if (Data->Year <= 70) {
        r = 70 + brandom(3);
    } else if (Data->Year <= 77) {
        r = Data->Year;
    }

    yr = r;
    r = brandom(100);

    if (miss == Mission_DirectAscent_LL) {
        prog = 5;
    } else if (r < 20) {
        prog = 2;
    } else if (r < 60) {
        prog = 3;
    } else {
        prog = 4;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 40, "MISSION: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, &Data->P[win].Manned[prog - 1].Name[0]);
    draw_string(0, 0, " ");
    draw_string(0, 0, RomanNumeral(brandom(15) + 2).c_str());
    display::graphics.setForegroundColor(6);
    draw_string(0, 0, "  -  ");
    display::graphics.setForegroundColor(8);;
    draw_number(0, 0, brandom(daysAMonth[monthWin]) + 1);
    draw_string(0, 0, " ");
    draw_string(0, 0, Month[monthWin]);
    draw_string(0, 0, "19");
    draw_number(0, 0, yr);
    r = brandom(100);

    if (win == 1 && prog == 5) {
        bud = 5;
    } else if (win == 0 && prog == 5) {
        bud = 2;
    } else {
        bud = (r < 50) ? 0 + (win * 3) : 1 + (win * 3);
    }

    if (bud < 0 || bud > 5) {
        bud = 0 + win;
    }

    InBox(241, 67, 313, 112);
    EndPict(242, 68, bud, 128);
    PatchMe(win, 270, 34, prog - 1, brandom(9));
    r = Data->P[win].AstroCount;
    man1 = brandom(r);
    man2 = brandom(r);
    man3 = brandom(r);
    man4 = brandom(r);

    while (1) {
        if ((man1 != man2) && (man1 != man3) && (man2 != man4) &&
            (man2 != man3) && (man3 != man4) && (man1 != man4)) {
            break;
        }

        while (man1 == man2) {
            man2 = brandom(r);
        }

        while (man1 == man3) {
            man3 = brandom(r);
        }

        while (man2 == man4) {
            man2 = brandom(r);
        }

        while (man2 == man3) {
            man3 = brandom(r);
        }

        while (man3 == man4) {
            man4 = brandom(r);
        }

        while (man1 == man4) {
            man4 = brandom(r);
        }
    }

    for (i = 1; i < 5; i++) {
        display::graphics.setForegroundColor(6);

        switch (i) {
        case 1:
            if (prog >= 1 && prog <= 3) {
                draw_string(10, 70, "CAPSULE PILOT - EVA: ");
            }

            if (prog > 3) {
                draw_string(10, 70, "COMMAND PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man1].Sex == 1) {
                display::graphics.setForegroundColor(12);  // Show females in orange
            }

            if (man1 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man1].Name[0]);
            }

            break;

        case 2:
            if (prog > 1 && prog < 5) {
                draw_string(10, 79, "LM PILOT - EVA: ");
            } else if (prog == 5) {
                draw_string(10, 79, "LUNAR PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man2].Sex == 1) {
                display::graphics.setForegroundColor(12);  // Show females in orange
            }

            if (man2 != -1 && (prog > 1 && prog < 5)) {
                draw_string(0, 0, &Data->P[win].Pool[man2].Name[0]);
            }

            break;

        case 3:
            if (prog > 2 && prog < 5) {
                draw_string(10, 88, "DOCKING SPECIALIST: ");
            } else if (prog == 5) {
                draw_string(10, 88, "EVA SPECIALIST: ");
            }

            display::graphics.setForegroundColor(8);

            if (Data->P[win].Pool[man3].Sex == 1) {
                display::graphics.setForegroundColor(12);  // Show females in orange
            }

            if (man3 != -1 && prog > 2) {
                draw_string(0, 0, &Data->P[win].Pool[man3].Name[0]);
            }

            break;

        case 4:
            if (prog == 5) {
                draw_string(10, 97, "EVA SPECIALIST: ");
                display::graphics.setForegroundColor(8);

                if (Data->P[win].Pool[man4].Sex == 1) {
                    display::graphics.setForegroundColor(12);  // Show females in orange
                }

                if (man4 != -1 && prog == 5) {
                    draw_string(0, 0, &Data->P[win].Pool[man4].Name[0]);
                }
            }

            break;

        default:
            break;
        }
    }

    manOnMoon = man2;

    if (prog == 3 || prog == 4) {
        manOnMoon = man1;
    }

    display::graphics.setForegroundColor(11);
    draw_string(10, 60, "FIRST ON THE MOON: ");

    if (&Data->P[win].Pool[manOnMoon].Sex == 0) {
        display::graphics.setForegroundColor(15);
    } else {
        display::graphics.setForegroundColor(17);
    }

    draw_string(0, 0, &Data->P[win].Pool[manOnMoon].Name[0]);
    display::graphics.setForegroundColor(6);
    FakeHistory(win, yr);
    music_start(M_INTERLUD);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    i = 0;
    key = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(244, 5, 313, 17);
        }
    }

    music_stop();
    return;
}

void FakeHistory(char plr, char Fyear)  // holds the winning player
{
    char bud;

    if (Fyear <= 65) {
        bud = 0 + plr;
    } else if (Fyear <= 67) {
        bud = 2 + plr;
    } else if (Fyear <= 69) {
        bud = 4 + plr;
    } else if (Fyear <= 71) {
        bud = 6 + plr;
    } else if (Fyear >= 72) {
        bud = 8 + plr;
    } else {
        bud = 10 + plr;
    }

    std::string history = HistFile(bud);
    PrintHist(history.c_str());
    return;
}

std::string HistFile(unsigned char bud)
{
    std::vector<std::string> text;
    
    std::string filename = "endgame.json";
    std::ifstream file(locate_file(filename.c_str(), FT_DATA));
    if (!file) {
		throw std::runtime_error(filename + " could not be opened.");
	}
    cereal::JSONInputArchive ar(file);
    ar(CEREAL_NVP(text));
    
    return text[bud];
}

void PrintHist(const char *buf)
{
    int i, k;
    display::graphics.setForegroundColor(8);
    k = 121;
    grMoveTo(10, k);

    for (i = 0; i < (int)strlen(buf); i++) {
        if (buf[i] == '*') {
            k += 7;
            grMoveTo(10, k);
        } else {
            draw_character(buf[i]);
        }
    }
}

void PrintOne(const char *buf, char tken)
{
    int i, k;
    display::graphics.setForegroundColor(7);

    if (tken == 0)  {
        k = 127;
    } else {
        k = 170;
    }

    grMoveTo(10, k);

    for (i = 0; i < (int)strlen(buf); i++) {
        if (buf[i] == '*') {
            k += 7;
            grMoveTo(10, k);
        } else {
            draw_character(buf[i]);
        }
    }
}

void AltHistory(char plr)  // holds the winning player
{
    char bud;

    if (Data->Year <= 65) {
        bud = 0 + plr;
    } else if (Data->Year <= 67) {
        bud = 2 + plr;
    } else if (Data->Year <= 69) {
        bud = 4 + plr;
    } else if (Data->Year <= 71) {
        bud = 6 + plr;
    } else if (Data->Year >= 72) {
        bud = 8 + plr;
    } else {
        bud = 10 + plr;
    }

    std::string history = HistFile(bud);
    PrintHist(history.c_str());
    return;
}

void SpecialEnd(void)
{
    char i;
    music_start(M_BADNEWS);

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 24);
    draw_heading(7, 5, "FAILED OBJECTIVE", 1, -1);
    ShBox(0, 26, 319, 199);
    fill_rectangle(1, 27, 318, 198, 7);
    InBox(5, 31, 314, 194);
    fill_rectangle(6, 32, 313, 193, 3);
    IOBox(242, 3, 315, 19);
    display::graphics.setForegroundColor(1);
    draw_string(257, 13, "CONTINUE");
    ShBox(6, 109, 313, 119);
    ShBox(6, 151, 313, 161);
    display::graphics.setForegroundColor(9);
    draw_string(130, 116, "UNITED STATES");
    draw_string(134, 158, "SOVIET UNION");
    fill_rectangle(6, 32, 313, 108, 0);
    InBox(178, 3, 205, 19);
    draw_small_flag(0, 179, 4);
    InBox(210, 3, 237, 19);
    draw_small_flag(1, 211, 4);
    LoserPict(0, 128);  // load loser picture
    std::string history = HistFile(10);
    PrintOne(history.c_str(), 0);
    history = HistFile(11);
    PrintOne(history.c_str(), 1);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    i = 0;
    key = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
        }
    }

    music_stop();
    return;
}


/**
 * Display endgame images.
 *
 * The endgame.but.%d.png images use a 128-color palette beginning at
 * index 128 through 255. Consequently, the coff parameter is overridden
 * by 128. (Changed to 31 to correct corruption in the colors. LPB)
 *
 * \param x     the upper-left x coordinate of the image destination.
 * \param y     the upper-left y coordinate of the image destination.
 * \param poff  the endgame image index, 0-5.
 * \param coff  the color palette index to export the endgame palette.
 * \throws runtime_error  if Filesystem is unable to load the image.
 */
void
EndPict(int x, int y, char poff, unsigned char coff)
{
    assert(poff >= 0 && poff <= 5);

    coff = 31;  // PNG images have their palette at [128, 255]
    char filename[128];
    snprintf(filename, sizeof(filename),
             "images/endgame.but.%d.png", (int) poff);
    boost::shared_ptr<display::PalettizedSurface> endgame(
        Filesystem::readImage(filename));

    endgame->exportPalette(coff, coff + 127);  // 128-color palette
    display::graphics.screen()->draw(endgame, x, y);
}

/**
 * Draw a 308x77 pixel Moonrise image.
 *
 * This uses a 128-color palette, which is stored to the global
 * display at [128, 255]. Currently, the coff parameter is fixed
 * at 128.
 *
 * NOTE: Originally, this drew the image masking for color = 0
 * transparency. However, the png image isn't transparent, and
 * it isn't meant to be transparent, so that's being skipped.
 *   -- rnyoakum
 *
 * \param poff  which of the "loser" images to draw (only 0 is valid).
 * \param coff  the color index to start storing the image's palette.
 *
 */
void
LoserPict(char poff, unsigned char coff)
{
    assert(poff == 0);
    coff = 128;

    const int x = 6;
    const int y = 32;

    char filename[128];
    snprintf(filename, sizeof(filename),
             "images/loser.but.%d.png", (int) poff);
    boost::shared_ptr<display::PalettizedSurface> image(
        Filesystem::readImage(filename));

    image->exportPalette(coff, coff + 127);  // 128-color palette
    display::graphics.screen()->draw(image, x, y);
}


void PlayFirst(char plr, char first)
{
    char i, w = 0, index;
    int Check = 0;

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    music_start(M_LIFTOFF);
    ShBox(80, 10, 240, 31);
    draw_heading(93, 14, "PRESTIGE FIRST", 0, -1);
    ShBox(80, 33, 240, 124);
    InBox(84, 37, 236, 118);
    // fill_rectangle(85, 46, 235, 125, 0);
    ShBox(80, 126, 240, 190);  // 77 first parameter
    display::graphics.setForegroundColor(1);
    draw_string(84, 132, "GOAL STEP COMPLETE: ");
    display::graphics.setForegroundColor(6);

// Modem Opponent => assure prestige first that mission
    Check = Data->Prestige[first].Index;
    index = plr;

    if (index == 0) {
        draw_string(0, 0, "U.S.A.");
    } else {
        draw_string(0, 0, "SOVIET");
    }

    for (i = first; i < 28; i++) {
        display::graphics.setForegroundColor(9);

        if (Data->Prestige[i].Place == index && !(Data->PD[plr][i] & 2)) {
            if (Option == -1 && MAIL == -1) {
                draw_string(84, 140 + w * 8, &PF[i][0]);
                ++w;
                Data->PD[index][i] |= 2;
            } else {
                // Found prestige first same mission
                if (Data->Prestige[i].Index == Check) {
                    draw_string(84, 140 + w * 8, &PF[i][0]);
                    ++w;
                    Data->PD[index][i] |= 2;
                }
            }
        }
    }

    display::graphics.setForegroundColor(7);
    FadeIn(2, 10, 0, 0);

    // Play the mission replay video
    if (Option == -1 && MAIL == -1) {
        Replay(plr, Data->P[plr].PastMissionCount - 1, 85, 38, 150, 80, "OOOO");
    } else {
        Replay(index, Data->Prestige[first].Index, 85, 38, 150, 80, "OOOO");
    }

    PauseMouse();
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    music_stop();
    return;
}

/* Play all pending Prestige First videos */
void PlayAllFirsts(char plr)
{
    int i;

    for (i = 0; i < 28; i++) {
        if (i != 4 && i != 5 && i != 6) {
            if (Data->Prestige[i].Place == plr && !(Data->PD[plr][i] & 2)) {
                PlayFirst(plr, i);
            }
        }
    }
}

/* vim: set noet ts=4 sw=4 tw=77: */
