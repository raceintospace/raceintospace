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

// This file handles the Capsule/Shuttle Screen.

#include "ast4.h"

#include <cassert>
#include <string>

#include "display/graphics.h"
#include "display/palettized_surface.h"

#include "ast0.h"
#include "astros.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "hardware.h"
#include "pace.h"
#include "place.h"
#include "sdlhelper.h"

#define Guy(a,b,c,d) (Data->P[a].Crew[b][c][d]-1)


void AstLevel(char plr, char prog, char crew, char ast);
void DrawProgs(char plr, char prog);
int CheckProgram(char plr, char prog);
void DrawPosition(char prog, int pos);
void ClearIt(void);
void NewAstList(char plr, char prog, int M1, int M2, int M3, int M4);
void AstStats(int plr, int prog, int man, int num);
void AstNames(int man, const Astros &crew);
void Flts(char old, char nw);
void FltsTxt(char nw, char col);



void AstLevel(char plr, char prog, char crew, char ast)
{
    int i, k, man, over = 0, temp, val;
    man = Guy(plr, prog, crew, ast);

    display::LegacySurface local(143, 74);
    local.copyFrom(display::graphics.legacyScreen(), 94, 38, 236, 111);
    ShBox(94, 38, 236, 95);
    InBox(98, 41, 232, 61);
    fill_rectangle(99, 42, 231, 60, 7 + plr * 3);
    display::graphics.setForegroundColor(12);
    draw_string(115, 48, "COMPATIBILITY");
    over = 0;
    val = 0;

    for (i = 0; i < Data->P[plr].CrewCount[prog][crew]; i++) {
        if (man != Guy(plr, prog, crew, i)) {
            temp = 0;

            if (Compatible(Data->P[plr].Pool[man],
                           Data->P[plr].Pool[Guy(plr, prog, crew, i)])) {
                temp++;
            }

            if ((plr == 1 && Data->Def.Ast2 == 0) || (plr == 0 && Data->Def.Ast1 == 0)) {
                InBox(111, 66 + 9 * val, 119, 72 + 9 * val);

                if (temp == 0) {
                    fill_rectangle(112, 67 + 9 * val, 118, 71 + 9 * val, 9);
                    over++;
                } else {
                    fill_rectangle(112, 67 + 9 * val, 118, 71 + 9 * val, 16);
                }

                display::graphics.setForegroundColor(1);
                draw_string(122, 71 + 9 * val, Data->P[plr].Pool[Guy(plr, prog, crew, i)].Name);
            } else if (temp == 0) {
                over++;
            }

            val++;
        }
    }

    switch (prog) {
    case 1:
        i = 16;
        break;

    case 2:
        i = (over == 1) ? 9 : 16;
        break;

    case 3:
        i = (over == 1) ? 11 : ((over == 2) ? 9 : 16) ;
        break;

    case 4:
        i = (over == 1) ? 11 : ((over == 2) ? 9 : 16) ;
        break;

    case 5:
        i = (over == 1) ? 11 : ((over >= 2) ? 9 : 16) ;
        break;

    default:
        i = 16;
        break;
    }

    // Level 2 Only
    if ((plr == 1 && Data->Def.Ast2 == 1) || (plr == 0 && Data->Def.Ast1 == 1)) {
        InBox(111, 66, 119, 72);
        fill_rectangle(112, 67, 118, 71, i);
        display::graphics.setForegroundColor(1);
        draw_string(122, 71, "CREW RELATIONSHIP");
    }

    if ((plr == 1 && Data->Def.Ast2 == 2) || (plr == 0 && Data->Def.Ast1 == 2)) {
        display::graphics.setForegroundColor(1);
        draw_string(111, 71, "NO INFO AT THIS LEVEL");
    }



    //fill_rectangle(113,53,119,57,i);  // shouldn't be mood
    display::graphics.setForegroundColor(11);
    draw_string(115, 57, Data->P[plr].Pool[man].Name);
    // don't do this for level three
    draw_string(0, 0, "  M: ");
    draw_number(0, 0, Data->P[plr].Pool[man].Mood);

    key = 0;

    if (mousebuttons) {
        WaitForMouseUp();
    } else {
        while (key == 0) {
            GetMouse();
        }
    }

    local.copyTo(display::graphics.legacyScreen(), 94, 38);
}

void DrawProgs(char plr, char prog)
{
    std::string title(Data->P[plr].Manned[prog - 1].Name);
    title.append(" PROGRAM");
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    display::graphics.setForegroundColor(1);
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 81);
    ShBox(0, 83, 319, 123);
    ShBox(0, 125, 158, 199);
    ShBox(161, 125, 319, 199);
    fill_rectangle(25, 129, 153, 195, 0);
    fill_rectangle(5, 129, 19, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    IOBox(243, 86, 316, 102);
    IOBox(243, 104, 316, 120);
    IOBox(243, 3, 316, 19);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    InBox(60, 27, 141, 78);
    InBox(3, 3, 30, 19);
    draw_up_arrow(9, 133);
    draw_down_arrow(9, 166);
    fill_rectangle(61, 28, 140, 77, 0);  // Kludge to change background from gray to black  -Leon

    {
        char filename[128];
        snprintf(filename, sizeof(filename), "images/aprog/aprog.%d.%d.png", plr, prog);
        boost::shared_ptr<display::PalettizedSurface> image(Filesystem::readImage(filename));

        image->exportPalette();
        display::graphics.screen()->draw(image, 61, 28);

    }

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 4; i++) {
            ShBox(164 + 77 * j, 139 + i * 15, 238 + 77 * j, 151 + i * 15);
        }
    }

    ShBox(4, 86, 12, 92);

    if (prog >= 2) {
        ShBox(4, 95, 12, 101);
    }

    if (prog >= 3) {
        ShBox(4, 104, 12, 110);
    }

    if (prog == 5) {
        ShBox(4, 113, 12, 119);
    }

    draw_small_flag(plr, 4, 4);
    display::graphics.setForegroundColor(9);
    draw_string(250, 96, "A");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "SSIGN CREW");
    display::graphics.setForegroundColor(9);
    draw_string(252, 114, "B");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "REAK CREW");
    draw_string(258, 13, "CONTINUE");
    display::graphics.setForegroundColor(5);
    draw_string(183, 133, "FLIGHT ");
    display::graphics.setForegroundColor(9);
    draw_string(0, 0, "C");
    display::graphics.setForegroundColor(5);
    draw_string(0, 0, "REW SELECTION");
    display::graphics.setForegroundColor(7);
    draw_string(152, 34, &Data->P[plr].Manned[prog - 1].Name[0]);

    for (int8_t i = 0; i < 8; i++) {
        if (prog == 1 && Data->P[plr].Pool[i].Assign == 1 && Data->P[plr].Pool[i].Moved >= 5) {
            // If Mercury/Vostok, mark as obsolete if crew(s) are about to start losing morale for staying here too long
            display::graphics.setForegroundColor(5);
            draw_string(0, 0, "  (OBSOLETE)");
            break;
        }
    }

    display::graphics.setForegroundColor(9);

    if (prog == 1) {
        draw_string(152, 42, "ONE");
    }

    if (prog == 2) {
        draw_string(152, 42, "TWO");
    }

    if (prog == 3 || prog == 4) {
        draw_string(152, 42, "THREE");
    }

    if (prog == 5) {
        draw_string(152, 42, "FOUR");
    }

    draw_string(0, 0, "-PERSON CAPACITY");
    display::graphics.setForegroundColor(7);
    draw_string(152, 50, "SAFETY FACTOR: ");
    display::graphics.setForegroundColor(11);
    draw_number(0, 0, Data->P[plr].Manned[prog - 1].Safety);
    draw_string(0, 0, "%");
    display::graphics.setForegroundColor(7);
    draw_string(152, 58, "UNIT WEIGHT: ");
    display::graphics.setForegroundColor(11);
    draw_number(0, 0, Data->P[plr].Manned[prog - 1].UnitWeight);
    display::graphics.setForegroundColor(7);
    draw_string(152, 66, "MAX DURATION: ");
    display::graphics.setForegroundColor(11);
    draw_number(0, 0, Data->P[plr].Manned[prog - 1].Duration);
    draw_string(0, 0, " DAYS (");

    if (prog == 1) {
        draw_string(0, 0, "B)");
    }

    if (prog == 2) {
        draw_string(0, 0, "E)");
    }

    if (prog == 3 || prog == 5) {
        draw_string(0, 0, "F)");
    }

    if (prog == 4) {
        draw_string(0, 0, "D)");
    }

    display::graphics.setForegroundColor(7);
    draw_string(152, 74, "AVOID FAILURE: ");
    display::graphics.setForegroundColor(11);

    if (Data->P[plr].Manned[prog - 1].SaveCard > 0) {
        draw_string(0, 0, "YES");
    } else {
        draw_string(0, 0, "NO");
    }

    display::graphics.setForegroundColor(1);
    draw_heading(40, 5, title.c_str(), 0, -1);
    return;
}

/** need to check programs to see if there are adequate astronauts there
 * check any assigned Cgroups yes then return
 * check how many astronauts assigned to current prog
 *
 *
 */
int CheckProgram(char plr, char prog)
{
    int check = 0;

    for (int i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Crew != 0) {
            ++check;
        }
    }

    if (check > 0) {
        return 1;
    }

    check = 0;

    for (int i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Assign == prog) {
            ++check;
        }
    }

    if (prog >= 1 && prog <= 3 && check >= prog) {
        return 2;
    } else if ((prog == 4 || prog == 5) && check >= prog - 1) {
        return 2;
    } else {  // return to limbo
        for (int i = 0; i < Data->P[plr].AstroCount; i++) {
            if (Data->P[plr].Pool[i].Assign == prog) {
                Data->P[plr].Pool[i].Assign = 0;
            }
        }
    }

    return 0;
}

void FixPrograms(char plr)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (Data->P[plr].Manned[i].DCost > 0 &&
            Data->P[plr].Manned[i].DCost <= Data->P[plr].Cash) {
            DamProb(plr, 2, i);
        }
    }

    for (i = 0; i < 5; i++) {
        if (Data->P[plr].Rocket[i].DCost > 0 &&
            Data->P[plr].Rocket[i].DCost <= Data->P[plr].Cash) {
            DamProb(plr, 1, i);
        }
    }

    for (i = 0; i < 4; i++) {
        if (Data->P[plr].Misc[i].DCost > 0 &&
            Data->P[plr].Misc[i].DCost <= Data->P[plr].Cash) {
            DamProb(plr, 3, i);
        }
    }

    for (i = 0; i < 3; i++) {
        if (Data->P[plr].Probe[i].DCost > 0 &&
            Data->P[plr].Probe[i].DCost <= Data->P[plr].Cash) {
            DamProb(plr, 0, i);
        }
    }

    return;
}


/**
 * Launch a dialogue box allowing the player to repair a damaged program.
 *
 * TODO: Enforce parameter prog value range.
 *
 * \param plr   the player index (0 or 1).
 * \param prog  the type of hardware (Rocket, Probe, etc.).
 * \param chk   the index of the damaged program (within its category).
 */
void DamProb(char plr, char prog, int chk)
{
    int D_Cost, Saf_Loss, ESafety;
    char Name[30];

    Saf_Loss = D_Cost = ESafety = 0;

    FadeOut(2, 10, 0, 0);

    display::graphics.screen()->clear();

    Equipment &hardware = HardwareProgram(plr, prog, chk);

    assert(hardware.DCost <= Data->P[plr].Cash);

    D_Cost = hardware.DCost;
    Saf_Loss = hardware.Damage;
    ESafety = hardware.Safety;
    strcpy(Name, hardware.Name);

    ShBox(35, 81, 288, 159);
    InBox(40, 86, 111, 126);
    InBox(116, 86, 283, 126);
    IOBox(116, 130, 189, 155);
    IOBox(201, 130, 274, 155);
    draw_flag(41, 87, plr);
    draw_heading(135, 136, "YES", 1, 0);
    draw_heading(225, 136, "NO", 1, 0);
    draw_heading(44, 135, "REPAIR", 1, -1);
    display::graphics.setForegroundColor(6);
    draw_string(121, 95, "DIRECTOR: ");
    display::graphics.setForegroundColor(8);

    if (plr == 0) {
        draw_string(0, 0, &Data->P[Data->plr[0]].Name[0]);
    } else {
        draw_string(0, 0, &Data->P[Data->plr[1]].Name[0]);
    }

    display::graphics.setForegroundColor(6);
    draw_string(121, 104, "DAMAGE: ");
    display::graphics.setForegroundColor(11);
    strcat(Name, " PROGRAM");
    draw_string(0, 0, &Name[0]);
    display::graphics.setForegroundColor(6);
    draw_string(121, 113, "DAMAGE COST: ");
    display::graphics.setForegroundColor(1);
    draw_number(0, 0, D_Cost);
    draw_string(0, 0, " M.B.  (OF ");
    draw_megabucks(0, 0, Data->P[plr].Cash);
    draw_string(0, 0, ")");
    display::graphics.setForegroundColor(6);
    draw_string(121, 122, "SAFETY LOSS: ");
    display::graphics.setForegroundColor(1);
    draw_number(0, 0, Saf_Loss);
    draw_string(0, 0, "%  (FROM ");
    draw_number(0, 0, ESafety);
    draw_string(0, 0, "%)");
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {
            if ((x >= 118 && y >= 132 && x <= 187 && y <= 153 && mousebuttons > 0) || key == 'Y') {
                InBox(118, 132, 187, 153);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                Data->P[plr].Cash -= D_Cost;
                hardware.DCost = 0;
                hardware.Damage = 0;

                return;
            } else if ((x >= 203 && y >= 132 && x <= 272 && y <= 153 && mousebuttons > 0) || key == 'N') {
                InBox(203, 132, 272, 153);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                return;
            }
        }
    }
}

void DrawPosition(char prog, int pos)
{

    display::graphics.setForegroundColor(5);

    switch (pos) {
    case 1:
        if (prog == 1) {
            draw_string(17, 91, "CAPSULE PILOT - EVA SPECIALIST");
        } else if (prog == 2) {
            draw_string(17, 91, "CAPSULE PILOT - DOCKING SPECIALIST");
        } else  if (prog >= 3) {
            draw_string(17, 91, "COMMAND PILOT");
        }

        break;

    case 2:
        if (prog > 1 && prog < 5) {
            draw_string(17, 100, "LM PILOT - EVA SPECIALIST");
        } else if (prog == 5) {
            draw_string(17, 100, "LUNAR PILOT");
        }

        break;

    case 3:
        if (prog > 2 && prog < 5) {
            draw_string(17, 109, "DOCKING SPECIALIST");
        } else if (prog == 5) {
            draw_string(17, 109, "EVA SPECIALIST");
        }

        break;

    case 4:
        if (prog == 5) {
            draw_string(17, 118, "EVA SPECIALIST");
        }

        break;

    default:
        break;
    }

    display::graphics.setForegroundColor(1);

    return;
}


/**
 * The main capsule building interface.
 *
 * Programs refers to the Manned capsule programs, specifically the
 * ones where astronauts may be assigned. This has the main control
 * loop for the capsule building user interface.
 *
 * \param plr  the country running the program
 * \param prog  the capsule style
 */
void Programs(char plr, char prog)
{
    int i, max, chk, tst;
    int now2 = 0, count = 0, grp = 0, BarA = 0;
    int M[100], CrewCount[8];
    char ksel = 0;

    helpText = "i036";
    keyHelpText = "k036";

    for (i = 0; i < 100; i++) {
        M[i] = -1;
    }

    if (prog > 4) {
        max = 4;
    } else if (prog == 4) {
        max = 3;
    } else {
        max = prog;
    }

    music_start(M_PRGMTRG);
    DrawProgs(plr, prog);
    Flts(0, 0);

    for (i = 0; i < ASTRONAUT_CREW_MAX; i++) {
        CrewCount[i] = Data->P[plr].CrewCount[prog][i];

        if (CrewCount[i] == 0) {
            FltsTxt(i, 8);
        }

        if (CrewCount[i] < max && CrewCount[i] != 0) {
            FltsTxt(i, 9);
        }

        if (CrewCount[i] == max) {
            int stt = 1;

            tst = Data->P[plr].Crew[prog][i][0] - 1;
            fill_rectangle(4, 40, 54, 66, 3);

            if (Data->P[plr].Pool[tst].Prime == 3) {
                // Primary crew this turn
                stt = 6;
            }

            if (Data->P[plr].Pool[tst].Prime == 4) {
                // Primary crew next turn
                stt = 17;
            }

            if (Data->P[plr].Pool[tst].Prime == 1) {
                // Backup crew this turn
                stt = 5;
            }

            if (Data->P[plr].Pool[tst].Prime == 2) {
                // Backup crew next turn
                stt = 16;
            }

            FltsTxt(i, stt);
        }
    }

    for (i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Assign == prog
            && Data->P[plr].Pool[i].Crew == 0) {
            M[count++] = i;
        }
    }

    DispLeft(plr, BarA, count, now2, &M[0]);
    NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
               Data->P[plr].Crew[prog][grp][1],
               Data->P[plr].Crew[prog][grp][2], Data->P[plr].Crew[prog][grp][3]);
    FadeIn(2, 10, 0, 0);

    chk = CheckProgram(plr, prog);

    if (chk == 0) {
        if (plr == 0) {
            Help("i113");
        } else {
            Help("i114");
        }

        music_stop();
        return;
    }

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < 8; i++) {
            // Right Select Box
            if (x >= 27 && y >= (131 + i * 8) && x <= 151
                && y <= (137 + i * 8) && mousebuttons > 0
                && (now2 - BarA + i) <= (count - 1)) {
                // Left
                now2 -= BarA;
                now2 += i;
                BarA = i;
                DispLeft(plr, BarA, count, now2, &M[0]);
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                BarSkill(plr, BarA, now2, &M[0]);
                WaitForMouseUp();
                DispLeft(plr, BarA, count, now2, &M[0]);
            }
        }

        if (mousebuttons > 0 || key > 0) {  /* Gameplay */
            if (((x >= 6 && y >= 130 && x <= 18 && y <= 161
                  && mousebuttons > 0) || key == UP_ARROW)
                && count > 0) {
                /* Lft Up */
                InBox(6, 130, 18, 161);

                for (i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {

                        if (BarA == 0)
                            if (now2 > 0) {
                                now2--;
                                DispLeft(plr, BarA, count, now2, &M[0]);
                            }

                        if (BarA > 0) {
                            BarA--;
                            now2--;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                        }

                        i = 51;
                    }
                }

                while (mousebuttons == 1 || key == UP_ARROW) {
                    delay(100);

                    if (BarA == 0)
                        if (now2 > 0) {
                            now2--;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                        }

                    if (BarA > 0) {
                        BarA--;
                        now2--;
                        DispLeft(plr, BarA, count, now2, &M[0]);
                    }

                    key = 0;

                    GetMouse();
                }

                OutBox(6, 130, 18, 161);
                delay(10);
            } else if (((x >= 6 && y >= 163 && x <= 18 && y <= 194
                         && mousebuttons > 0) || key == DN_ARROW)
                       && count > 0) {
                /* Lft Dwn */
                InBox(6, 163, 18, 194);

                for (i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {

                        if (BarA == 7)
                            if (now2 < count - 1) {
                                now2++;
                                DispLeft(plr, BarA, count, now2, &M[0]);
                            }

                        if (BarA < 7)
                            if (now2 < count - 1) {
                                BarA++;
                                now2++;
                                DispLeft(plr, BarA, count, now2, &M[0]);
                            }

                        i = 51;
                    }
                }

                while (mousebuttons == 1 || key == DN_ARROW) {
                    delay(100);

                    if (BarA == 7)
                        if (now2 < count - 1) {
                            now2++;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                        }

                    if (BarA < 7)
                        if (now2 < count - 1) {
                            BarA++;
                            now2++;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                        }

                    key = 0;

                    GetMouse();
                }

                //WaitForMouseUp();
                OutBox(6, 163, 18, 194);
            } else if (key == K_HOME) {
                BarA = 0;
                now2 = 0;
                DispLeft(plr, BarA, count, now2, &M[0]);
            } else if (key == K_END) {
                BarA = MIN(count - 1, 7);
                now2 = count - 1;
                DispLeft(plr, BarA, count, now2, &M[0]);
            } else if (key == 'S') {
                // Show Skill
                if (now2 >= 0 && now2 < count) {
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    BarSkill(plr, BarA, now2, &M[0]);
                }
            } else if (((x >= 4 && y >= 86 && x <= 117 && y <= 92  // used to be x <= 12
                         && mousebuttons > 0) || key == '1')
                       && CrewCount[grp] >= 1) {
                /* Display Man 1 */
                InBox(4, 86, 12, 92);
                AstLevel(plr, prog, grp, 0);
                OutBox(4, 86, 12, 92);
            } else if (((x >= 4 && y >= 95 && x <= 117 && y <= 101  // used to be x <= 12
                         && mousebuttons > 0) || key == '2') && prog >= 2
                       && CrewCount[grp] >= 2) {
                /* Display Man 2 */
                InBox(4, 95, 12, 101);
                AstLevel(plr, prog, grp, 1);
                OutBox(4, 95, 12, 101);
            } else if (((x >= 4 && y >= 104 && x <= 117 && y <= 110  // used to be x <= 12
                         && mousebuttons > 0) || key == '3') && prog >= 3
                       && CrewCount[grp] >= 3) {
                /* Display Man 3 */
                InBox(4, 104, 12, 110);
                AstLevel(plr, prog, grp, 2);
                OutBox(4, 104, 12, 110);
            } else if (((x >= 4 && y >= 113 && x <= 117 && y <= 119  // used to be x <= 12
                         && mousebuttons > 0) || key == '4') && prog >= 5
                       && CrewCount[grp] >= 4) {
                /* Display Man 4 */
                InBox(4, 113, 12, 119);
                AstLevel(plr, prog, grp, 3);
                OutBox(4, 113, 12, 119);
            } else if (key == 'C') {
                ClearIt();
                ksel = grp;        //save old flt crew

                if (grp == 7) {
                    grp = 0;
                } else {
                    ++grp;
                }

                Flts(ksel, grp);
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);

                if (key > 0) {
                    delay(150);
                }
            } else if (x >= 164 && y >= 139 && x <= 238 && y <= 151
                       && mousebuttons > 0) {
                /* Flt Crew I */
                ClearIt();
                Flts(grp, 0);
                grp = 0;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 164 && y >= 154 && x <= 238 && y <= 166
                       && mousebuttons > 0) {
                /* Flt Crew II */
                ClearIt();
                Flts(grp, 1);
                grp = 1;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 164 && y >= 169 && x <= 238 && y <= 181
                       && mousebuttons > 0) {
                /* Flt Crew III */
                ClearIt();
                Flts(grp, 2);
                grp = 2;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 164 && y >= 184 && x <= 238 && y <= 196
                       && mousebuttons > 0) {
                /* Flt Crew IV */
                ClearIt();
                Flts(grp, 3);
                grp = 3;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 241 && y >= 139 && x <= 315 && y <= 151
                       && mousebuttons > 0) {
                /* Flt Crew V */
                ClearIt();
                Flts(grp, 4);
                grp = 4;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 241 && y >= 154 && x <= 315 && y <= 166
                       && mousebuttons > 0) {
                /* Flt Crew VI */
                ClearIt();
                Flts(grp, 5);
                grp = 5;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 241 && y >= 169 && x <= 315 && y <= 181
                       && mousebuttons > 0) {
                /* Flt Crew VII */
                ClearIt();
                Flts(grp, 6);
                grp = 6;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (x >= 241 && y >= 184 && x <= 315 && y <= 196
                       && mousebuttons > 0) {
                /* Flt Crew VIII */
                ClearIt();
                Flts(grp, 7);
                grp = 7;
                NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                           Data->P[plr].Crew[prog][grp][1],
                           Data->P[plr].Crew[prog][grp][2],
                           Data->P[plr].Crew[prog][grp][3]);
                WaitForMouseUp();
            } else if (((x >= 245 && y >= 88 && x <= 314 && y <= 100
                         && mousebuttons > 0) || key == 'A')
                       && CrewCount[grp] < max) {
                /* Assign 'Naut */
                if (Data->P[plr].Crew[prog][grp][CrewCount[grp]] == 0
                    && count > 0) {
                    InBox(245, 88, 314, 100);
                    Data->P[plr].Crew[prog][grp][CrewCount[grp]] = M[now2] + 1;

                    AstNames(CrewCount[grp], Data->P[plr].Pool[M[now2]]);
                    Data->P[plr].Pool[M[now2]].Crew = grp + 1;
                    Data->P[plr].Pool[M[now2]].Task = CrewCount[grp];
                    Data->P[plr].Pool[M[now2]].Unassigned = 1;

                    for (i = now2; i < count; i++) {
                        M[i] = M[i + 1];
                    }

                    M[i] = -1;
                    count--;

                    if (now2 == count) {
                        if (now2 > 0) {
                            now2--;
                        }

                        if (BarA > 0) {
                            BarA--;
                        }
                    }

                    DispLeft(plr, BarA, count, now2, &M[0]);

                    CrewCount[grp]++;

                    Data->P[plr].CrewCount[prog][grp] = CrewCount[grp];

                    if (CrewCount[grp] == max) {
                        FltsTxt(grp, 1);
                    } else {
                        FltsTxt(grp, 9);
                    }

                    NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
                               Data->P[plr].Crew[prog][grp][1],
                               Data->P[plr].Crew[prog][grp][2],
                               Data->P[plr].Crew[prog][grp][3]);
                    WaitForMouseUp();

                    if (key > 0) {
                        delay(150);
                    }

                    OutBox(245, 88, 314, 100);
                }                 /* End outer if */
            } else if ((x >= 245 && y >= 106 && x <= 314 && y <= 118
                        && mousebuttons > 0) || key == 'B') {
                /* Break Group */
                tst = Data->P[plr].Crew[prog][grp][0] - 1;

                /* If the crew is assigned to a mission, create an alert
                 * that it cannot be broken while the mission is planned. */
                if (Data->P[plr].Pool[tst].Prime > 0) {
                    OutBox(245, 106, 314, 118);
                    /* Copy the screen area into a buffer before drawing
                     * the dialog so it can be repainted after the dialog
                     * is dismissed */
                    display::LegacySurface buffer(170, 131);
                    buffer.copyFrom(display::graphics.legacyScreen(), 75, 43, 244, 173);
                    /* Draw the alert message */
                    ShBox(75, 43, 244, 173);
                    IOBox(81, 152, 238, 167);
                    InBox(81, 70, 238, 113);
                    fill_rectangle(82, 71, 237, 112, 7 + 3 * plr);
                    display::graphics.setForegroundColor(1);
                    draw_heading(118, 50, "PROBLEM", 0, -1);
                    draw_string(136, 161, "CONTINUE");
                    display::graphics.setForegroundColor(11);
                    draw_string(88, 80, "FLIGHT CREW ");
                    draw_number(0, 0, grp + 1);

                    if (CrewCount[grp] == 0) {
                        draw_string(0, 0, " IS EMPTY.");
                    } else {
                        draw_string(0, 0, " IS ALREADY");
                        draw_string(88, 88, "ASSIGNED TO THE ");

                        if (Data->P[plr].Pool[tst].Prime == 4
                            || Data->P[plr].Pool[tst].Prime == 3) {
                            draw_string(0, 0, "PRIMARY");
                        } else {
                            draw_string(0, 0, "BACKUP");
                        }

                        draw_string(88, 96, "CREW OF A CURRENT MISSION:");
                    }

                    draw_string(88, 104, "CANNOT BREAK THIS CREW.");

                    WaitForMouseUp();
                    i = 1;

                    while (i == 1) {
                        key = 0;
                        GetMouse();

                        if (mousebuttons > 0 || key > 0) {
                            if ((x >= 83 && y >= 154 && x <= 236 && y <= 165
                                 && mousebuttons != 0) || key == K_ENTER || key == K_ESCAPE) {
                                InBox(83, 154, 236, 165);
                                WaitForMouseUp();
                                OutBox(83, 154, 236, 165);
                                /* Closing the alert message.
                                 * Redraw the screen behind it from buffer */
                                buffer.copyTo(display::graphics.legacyScreen(),
                                              75, 43);
                                i = 2;
                            }
                        }
                    }
                } else if (Data->P[plr].Crew[prog][grp][0] != 0) {
                    InBox(245, 106, 314, 118);

                    while (CrewCount[grp] > 0) {
                        M[count] =
                            Data->P[plr].Crew[prog][grp][CrewCount[grp] - 1] - 1;
                        Data->P[plr].Crew[prog][grp][CrewCount[grp] - 1] = 0;
                        Data->P[plr].Pool[M[count]].Crew = 0;
                        Data->P[plr].Pool[M[count]].Moved = 0;
                        Data->P[plr].Pool[M[now2]].Unassigned = 0;
                        CrewCount[grp]--;
                        count++;
                    }

                    ClearIt();

                    FltsTxt(grp, 8);

                    DispLeft(plr, BarA, count, now2, &M[0]);

                    for (i = 1; i < 5; i++) {
                        DrawPosition(prog, i);
                    }

                    WaitForMouseUp();
                    OutBox(245, 106, 314, 118);
                }
            } else if ((x >= 245 && y >= 5 && x <= 314 && y <= 17
                        && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
                /* Exit */
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(245, 5, 314, 17);
                delay(10);

                for (i = 0; i < 8; i++) {
                    if (CrewCount[i] < max)
                        while (CrewCount[i] > 0) {
                            M[count] =
                                Data->P[plr].Crew[prog][i][CrewCount[i] - 1] - 1;
                            Data->P[plr].Crew[prog][i][CrewCount[i] - 1] = 0;
                            Data->P[plr].Pool[M[count]].Crew = 0;
                            Data->P[plr].CrewCount[prog][i] = 0;
                            CrewCount[i]--;
                            count++;
                        }

                    Data->P[plr].CrewCount[prog][i] = CrewCount[i];
                }

                for (i = 0; i < count; i++) {
                    Data->P[plr].Pool[M[i]].Assign = 0;
                }

                music_stop();

                return;            /* Done */
            }
        }
    }
}

/** clears the screen
 */
void ClearIt(void)
{

    fill_rectangle(16, 87, 75, 91, 3);
    fill_rectangle(5, 87, 11, 91, 3);
    fill_rectangle(16, 96, 75, 100, 3);
    fill_rectangle(5, 96, 11, 100, 3);
    fill_rectangle(16, 105, 75, 109, 3);
    fill_rectangle(5, 105, 11, 109, 3);
    fill_rectangle(16, 114, 75, 118, 3);
    fill_rectangle(5, 114, 11, 118, 3);
    fill_rectangle(16, 87, 238, 121, 3);

    return;
}


void NewAstList(char plr, char prog, int M1, int M2, int M3, int M4)
{
    /* Clear area that says Primary Crew Next Turn etc. */
    fill_rectangle(4, 40, 54, 66, 3);
    fill_rectangle(13, 86, 231, 122, 3);  /* Clear Astro Area */
    display::graphics.setForegroundColor(1);

    if (M1 > 0) {
        AstNames(0, Data->P[plr].Pool[M1 - 1]);
        AstStats(plr, prog, 0, M1 - 1);
    } else {
        DrawPosition(prog, 1);
    }

    if (M2 > 0) {
        AstNames(1, Data->P[plr].Pool[M2 - 1]);
        AstStats(plr, prog, 1, M2 - 1);
    } else {
        DrawPosition(prog, 2);
    }

    if (M3 > 0) {
        AstNames(2, Data->P[plr].Pool[M3 - 1]);
        AstStats(plr, prog, 2, M3 - 1);
    } else {
        DrawPosition(prog, 3);
    }

    if (M4 > 0) {
        AstNames(3, Data->P[plr].Pool[M4 - 1]);
        AstStats(plr, prog, 3, M4 - 1);
    } else {
        DrawPosition(prog, 4);
    }

    return;
}

/** Draws Astronaut attributes
 *
 * The program parameter is used for highlighting crew role attribues.
 * It **does not** use the standard EquipMannedIndex values.
 * Instead, it uses the indexing in the Crew[][][] array:
 *    1=Mercury/Vostok   2=Gemini/Voskhod    3=Apollo/Soyuz
 *    4=XMS-2/Lapot      5=Jupiter/Kvartet
 *
 * \param plr
 * \param program  Capsule/shuttle index in Crew[].
 * \param man
 * \param num
 */
void AstStats(int plr, int program, int man, int num)
{
    int y, yy;
    display::graphics.setForegroundColor(1);
    y = 91 + man * 9;

    if (man == 0) {
        display::graphics.setForegroundColor(11);   /* Highlight CA for command pilot */
    }

    draw_string(119, y, "CA:");
    yy = 0;

    if (Data->P[plr].Pool[num].Cap == 1) {
        yy = 134;
    }

    draw_number(yy, y, Data->P[plr].Pool[num].Cap);
    display::graphics.setForegroundColor(1);

    if (man == 1 && program > 1) {
        display::graphics.setForegroundColor(11);   /* Highlight LM for LM pilot */
    }

    draw_string(143, y, "LM:");
    yy = 0;

    if (Data->P[plr].Pool[num].LM == 1) {
        yy = 158;
    }

    draw_number(yy, y, Data->P[plr].Pool[num].LM);
    display::graphics.setForegroundColor(1);

    if (program == 1 || ((program == 2 || program == 3 || program == 4) && man == 1) || (program == 5 && man > 1)) {
        display::graphics.setForegroundColor(11);   /* Highlight EV for EVA specialist */
    }

    draw_string(167, y, "EV:");
    yy = 0;

    if (Data->P[plr].Pool[num].EVA == 1) {
        yy = 182;
    }

    draw_number(yy, y, Data->P[plr].Pool[num].EVA);
    display::graphics.setForegroundColor(1);

    if ((program == 2 && man == 0) || ((program == 3 || program == 4) && man == 2)) {
        display::graphics.setForegroundColor(11);   /* Highlight DO for docking specialist */
    }

    draw_string(192, y, "DO:");
    yy = 0;

    if (Data->P[plr].Pool[num].Docking == 1) {
        yy = 207;
    }

    draw_number(yy, y, Data->P[plr].Pool[num].Docking);

    display::graphics.setForegroundColor(1);  /* Never highlight EN skill */
    draw_string(217, y, "EN:");
    yy = 0;

    if (Data->P[plr].Pool[num].Endurance == 1) {
        yy = 232;
    }

    draw_number(yy, y, Data->P[plr].Pool[num].Endurance);

    fill_rectangle(4, 40, 54, 66, 3);
    // Now tell if this 'naut is assigned to a crew

    if (Data->P[plr].Pool[num].Prime == 3) {
        display::graphics.setForegroundColor(6);
        draw_string(11, 45, "PRIMARY");
        draw_string(19, 53, "CREW");
        draw_string(6, 61, "THIS TURN");
    }

    if (Data->P[plr].Pool[num].Prime == 4) {
        display::graphics.setForegroundColor(17);
        draw_string(11, 45, "PRIMARY");
        draw_string(19, 53, "CREW");
        draw_string(5, 61, "NEXT TURN");
    }

    if (Data->P[plr].Pool[num].Prime == 1) {
        display::graphics.setForegroundColor(5);
        draw_string(13, 45, "BACKUP");
        draw_string(19, 53, "CREW");
        draw_string(6, 61, "THIS TURN");
    }

    if (Data->P[plr].Pool[num].Prime == 2) {
        display::graphics.setForegroundColor(16);
        draw_string(13, 45, "BACKUP");
        draw_string(19, 53, "CREW");
        draw_string(5, 61, "NEXT TURN");
    }

    return;
}


void AstNames(int man, const Astros &crew)
{
    switch (man) {
    case 0:
        fill_rectangle(16, 87, 75, 91, 3);
        fill_rectangle(5, 87, 11, 91, 3);
        break;

    case 1:
        fill_rectangle(16, 96, 75, 100, 3);
        fill_rectangle(5, 96, 11, 100, 3);
        break;

    case 2:
        fill_rectangle(16, 105, 75, 109, 3);
        fill_rectangle(5, 105, 11, 109, 3);
        break;

    case 3:
        fill_rectangle(16, 114, 75, 118, 3);
        fill_rectangle(5, 114, 11, 118, 3);
        break;
    }

    display::graphics.setForegroundColor(1);

    if (crew.Sex == 1) {
        // Show name in blue if 'naut is female.
        display::graphics.setForegroundColor(18);
    }

    if (crew.RetirementDelay > 0) {
        // Show name in black if 'naut is male and has announced
        // retirement.
        display::graphics.setForegroundColor(0);
    }

    if (crew.Sex == 1 && crew.RetirementDelay > 0) {
        // Show name in purple if 'naut is female and has announced
        // retirement.
        display::graphics.setForegroundColor(7);
    }

    std::string name(&crew.Name[0], ARRAY_LENGTH(crew.Name));


    switch (man) {
    case 0:
        draw_string(17, 91, name.c_str());

        if (crew.Missions > 0) {
            draw_string(0, 0, " (");
            draw_number(0, 0, crew.Missions);
            draw_string(0, 0, ")");
        }

        break;

    case 1:
        draw_string(17, 100, name.c_str());

        if (crew.Missions > 0) {
            draw_string(0, 0, " (");
            draw_number(0, 0, crew.Missions);
            draw_string(0, 0, ")");
        }

        break;

    case 2:
        draw_string(17, 109, name.c_str());

        if (crew.Missions > 0) {
            draw_string(0, 0, " (");
            draw_number(0, 0, crew.Missions);
            draw_string(0, 0, ")");
        }

        break;

    case 3:
        draw_string(17, 118, name.c_str());

        if (crew.Missions > 0) {
            draw_string(0, 0, " (");
            draw_number(0, 0, crew.Missions);
            draw_string(0, 0, ")");
        }

        break;
    }

    uint8_t color = MoodColor(crew.Mood);

    switch (man) {
    case 0:
        fill_rectangle(5, 87, 11, 91, color);
        break;

    case 1:
        fill_rectangle(5, 96, 11, 100, color);
        break;

    case 2:
        fill_rectangle(5, 105, 11, 109, color);
        break;

    case 3:
        fill_rectangle(5, 114, 11, 118, color);
        break;
    }
}

void Flts(char old, char nw)
{

    switch (old) {
    case 0:
        OutBox(164, 139, 238, 151);
        break;

    case 1:
        OutBox(164, 154, 238, 166);
        break;

    case 2:
        OutBox(164, 169, 238, 181);
        break;

    case 3:
        OutBox(164, 184, 238, 196);
        break;

    case 4:
        OutBox(241, 139, 315, 151);
        break;

    case 5:
        OutBox(241, 154, 315, 166);
        break;

    case 6:
        OutBox(241, 169, 315, 181);
        break;

    case 7:
        OutBox(241, 184, 315, 196);
        break;
    }

    switch (nw) {
    case 0:
        InBox(164, 139, 238, 151);
        break;

    case 1:
        InBox(164, 154, 238, 166);
        break;

    case 2:
        InBox(164, 169, 238, 181);
        break;

    case 3:
        InBox(164, 184, 238, 196);
        break;

    case 4:
        InBox(241, 139, 315, 151);
        break;

    case 5:
        InBox(241, 154, 315, 166);
        break;

    case 6:
        InBox(241, 169, 315, 181);
        break;

    case 7:
        InBox(241, 184, 315, 196);
        break;
    }
} /* End of Flts */

void FltsTxt(char nw, char col)
{
    display::graphics.setForegroundColor(col);

    switch (nw) {
    case 0:
        draw_string(169, 147, "FLT. CREW I");
        break;

    case 1:
        draw_string(169, 162, "FLT. CREW II");
        break;

    case 2:
        draw_string(169, 177, "FLT. CREW III");
        break;

    case 3:
        draw_string(169, 192, "FLT. CREW IV");
        break;

    case 4:
        draw_string(246, 147, "FLT. CREW V");
        break;

    case 5:
        draw_string(246, 162, "FLT. CREW VI");
        break;

    case 6:
        draw_string(246, 177, "FLT. CREW VII");
        break;

    case 7:
        draw_string(246, 192, "FLT. CREW VIII");
        break;
    }
} /* End of FltsTxt */

/* vi: set noet ts=4 sw=4 tw=78: */
