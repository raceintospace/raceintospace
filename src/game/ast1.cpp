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
/** \file ast1.c Astronaut Recruitment
 */

// This file handles 'naut recruitment.

/* Variables in this file
   BarA   The line where the bar is showing on the Left (0-7)
   BarB   The line where the bar is showing on the Right (0-7)
   MaxMen The total number of candidates available for this recruitment
   MaxSel The maximum number of candidates you can select in this recruitment
   min    The first person being read from the 'naut roster
   max    The last person being read from the 'naut roster
   count  The number of candidates selected so far
   now    The selected candidate (?) on the Right
   now2   The selected candidate (?) on the Left
   ksel   Which side is currently selected (0=Right, 1=Left) */

#include "display/graphics.h"

#include "ast1.h"
#include "Buzz_inc.h"
#include "options.h"   //Naut Randomize && Naut Compatibility, Nikakd, 10/8/10
#include "draw.h"
#include "game_main.h"
#include "place.h"
#include "ast0.h"
#include "aipur.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"

void DispEight(char now, char loc);
void DispEight2(int nw, int lc, int cnt);
void DrawAstCheck(char plr);
void DrawAstSel(char plr);
void Recruit(char plr, uint8_t pool, uint8_t candidate);


/** display list of 'naut names
 *
 */
void DispEight(char now, char loc)
{
    int i, start;
    start = now - loc;

    for (i = start; i < start + 8; i++) {
        if (MCol[i] == 1) {
            display::graphics.setForegroundColor(8);
        } else {
            display::graphics.setForegroundColor(6 + (Men[i].Sex + 1) * 6);
        }

        draw_string(189, 136 + (i - start) * 8, &Men[i].Name[0]);
    }

    fill_rectangle(206, 48, 306, 52, 3);
    fill_rectangle(221, 57, 306, 61, 3);
    fill_rectangle(293, 66, 301, 70, 3);
    fill_rectangle(274, 98, 281, 102, 3);

    display::graphics.setForegroundColor(1);

    if (Men[now].Sex == 0) {
        draw_string(206, 52, "MR. ");
    } else {
        draw_string(206, 52, "MS. ");
    }

    draw_string(0, 0, &Men[now].Name[0]);
    draw_number(294, 70, Men[now].Cap);
    draw_number(275, 102, Men[now].Endurance);
    return;
} /* End of Disp8 */


void DispEight2(int nw, int lc, int cnt)
{
    int i, start, num;
    start = nw - lc;
    num = (cnt < 8) ? cnt : 8;

    display::graphics.setForegroundColor(11);

    for (i = start; i < start + num; i++) {
        if (sel[i] != -1) {
            display::graphics.setForegroundColor(6 + (Men[sel[i]].Sex + 1) * 6);
            draw_string(28, 136 + (i - start) * 8, &Men[ sel[i] ].Name[0]);
        }
    }

    fill_rectangle(45, 48, 145, 52, 3);
    fill_rectangle(60, 57, 145, 61, 3);
    fill_rectangle(132, 66, 140, 70, 3);
    fill_rectangle(113, 98, 120, 102, 3);
    fill_rectangle(292, 36, 310, 41, 7);
    display::graphics.setForegroundColor(11);
    draw_number(292, 41, MaxSel - cnt);

    if (cnt > 0) {
        display::graphics.setForegroundColor(1);

        if (Men[sel[nw]].Sex == 0) {
            draw_string(45, 52, "MR. ");
        } else {
            draw_string(45, 52, "MS. ");
        }

        draw_string(0, 0, &Men[sel[nw]].Name[0]);
        draw_number(133, 70, Men[sel[nw]].Cap);
        draw_number(114, 102, Men[sel[nw]].Endurance);
    }

    return;
}


void DrawAstCheck(char plr)
{
    int i, pos, ad = 0;

    pos = 0; /* XXX check uninitialized */

    if (Data->P[plr].AstroDelay > 0) {
        ad = 1;
    }

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(80, 44, 237, 155);
    InBox(87, 49, 230, 103);
    fill_rectangle(88, 50, 229, 102, 7 + plr * 3);
    IOBox(98, 133, 150, 149);
    IOBox(166, 133, 218, 149);
    display::graphics.setForegroundColor(5);

    if (plr == 0) {
        draw_string(99, 60, "ASTRONAUT");
    } else {
        draw_string(99, 60, "COSMONAUT");
    }

    draw_string(0, 0, " RECRUITMENT");
    display::graphics.setForegroundColor(11);
    draw_string(100, 73, "GROUP ");

    switch (Data->P[plr].AstroLevel) {
    case 0:
        draw_string(0, 0, "I");
        pos = ASTRO_POOL_LVL1;
        break;

    case 1:
        draw_string(0, 0, "II");
        pos = ASTRO_POOL_LVL2;
        break;

    case 2:
        draw_string(0, 0, "III");
        pos = ASTRO_POOL_LVL3;
        break;

    case 3:
        draw_string(0, 0, "IV");
        pos = ASTRO_POOL_LVL4;
        break;

    case 4:
        draw_string(0, 0, "V");
        pos = ASTRO_POOL_LVL5;
        break;
    }

    if (Data->Season == 0) {
        draw_string(160, 73, "SPRING 19");
    } else {
        draw_string(170, 73, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    if (Data->P[plr].AstroLevel == 0) {
        i = 20;
    } else {
        i = 15;
    }

    if (ad == 0) {
        if (pos < 10) {
            draw_number(110, 86, pos);
        } else {
            draw_number(108, 86, pos);
        }

        draw_string(0, 0, " POSITIONS TO FILL");
        draw_string(133, 97, "COST: ");
        draw_number(0, 0, i);
        draw_string(0, 0, " MB");
    } else {
        if (Data->P[plr].AstroDelay != 1) {
            draw_number(114, 86, Data->P[plr].AstroDelay);
        } else {
            draw_number(118, 86, Data->P[plr].AstroDelay);
        }

        draw_string(0, 0, " SEASON");

        if (Data->P[plr].AstroDelay != 1) {
            draw_string(0, 0, "S");
        }

        draw_string(0, 0, " TO WAIT");
        draw_string(104, 97, "FOR THE NEW RECRUITS");
    }

    display::graphics.setForegroundColor(1);

    if (ad == 1) {
        draw_string(102, 113, "  YOU CANNOT RECRUIT");

        if (plr == 0) {
            draw_string(102, 122, "ASTRONAUTS THIS TURN");
        } else {
            draw_string(102, 122, "COSMONAUTS THIS TURN");
        }

        display::graphics.setForegroundColor(8);
        draw_string(114, 143, "EXIT");
        draw_string(182, 143, "EXIT");
    }

    if (ad == 0) {
        if (Data->P[plr].Cash < i) {
            draw_string(110, 113, "YOU CANNOT AFFORD");

            if (plr == 0) {
                draw_string(103, 122, "ASTRONAUTS THIS TURN");
            } else {
                draw_string(103, 122, "COSMONAUTS THIS TURN");
            }

            display::graphics.setForegroundColor(8);
            draw_string(113, 143, "EXIT");
            draw_string(181, 143, "EXIT");
        } else {
            draw_string(101, 113, "DO YOU WISH TO RECRUIT");

            if (plr == 0) {
                draw_string(100, 122, "ASTRONAUTS THIS TURN?");
            } else {
                draw_string(100, 122, "COSMONAUTS THIS TURN?");
            }

            display::graphics.setForegroundColor(6);
            draw_string(116, 143, "Y");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, "ES");
            display::graphics.setForegroundColor(6);
            draw_string(187, 143, "N");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, "O");
        }
    }

    draw_small_flag(plr, 4, 4);

    FadeIn(2, 10, 0, 0);

    return;
}


/** Draw selection screen for Astronaut recruitment
 *
 */
void DrawAstSel(char plr)
{

    helpText = "i012";
    keyHelpText = "k012";
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 158, 199);
    ShBox(161, 24, 319, 199);
    fill_rectangle(5, 129, 19, 195, 0);
    fill_rectangle(166, 129, 180, 195, 0);
    fill_rectangle(25, 129, 153, 195, 0);
    fill_rectangle(186, 129, 314, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    ShBox(167, 130, 179, 161);
    ShBox(167, 163, 179, 194);
    IOBox(243, 3, 316, 19);
    IOBox(5, 109, 153, 125);
    IOBox(166, 109, 315, 125);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    InBox(185, 128, 315, 196);
    InBox(165, 128, 181, 196);
    InBox(3, 3, 30, 19);
    draw_up_arrow(9, 133);
    draw_down_arrow(9, 166); //Left
    draw_up_arrow(170, 133);
    draw_down_arrow(170, 166); //Right
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    draw_heading(35, 5, "RECRUITMENT", 0, -1);
    draw_small_flag(plr, 4, 4);

    InBox(3, 27, 155, 44);
    fill_rectangle(4, 28, 154, 43, 7);
    InBox(164, 27, 316, 44);
    fill_rectangle(165, 28, 315, 43, 7);

    display::graphics.setForegroundColor(1);

    if (plr == 0) {
        draw_string(25, 34, "ASTRONAUT SELECTION");
    } else {
        draw_string(25, 34, "COSMONAUT SELECTION");
    }

    draw_string(192, 34, "POOL OF APPLICANTS");

    display::graphics.setForegroundColor(11);
    draw_string(12, 41, "GROUP ");

    switch (Data->P[plr].AstroLevel) {
    case 0:
        draw_string(0, 0, "I");
        break;

    case 1:
        draw_string(0, 0, "II");
        break;

    case 2:
        draw_string(0, 0, "III");
        break;

    case 3:
        draw_string(0, 0, "IV");
        break;

    case 4:
        draw_string(0, 0, "V");
        break;
    }

    draw_string(185, 41, "REMAINING POSITIONS:");

    if (Data->Season == 0) {
        draw_string(88, 41, "SPRING 19");
    } else {
        draw_string(98, 41, "FALL 19");
    }

    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(9);
    draw_string(12, 52, "NAME:");
    draw_string(173, 52, "NAME:");
    draw_string(12, 61, "SERVICE:");
    draw_string(173, 61, "SERVICE:");
    draw_string(12, 70, "SKILLS:");
    draw_string(173, 70, "SKILLS:");
    display::graphics.setForegroundColor(11);
    draw_string(54, 70, "CAPSULE PILOT:");
    draw_string(215, 70, "CAPSULE PILOT:");
    display::graphics.setForegroundColor(11);
    draw_string(54, 78, "L.M. PILOT: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 78, "L.M. PILOT: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 86, "E.V.A.: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 86, "E.V.A.: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 94, "DOCKING: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 94, "DOCKING: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 102, "ENDURANCE:");
    draw_string(215, 102, "ENDURANCE:");
    display::graphics.setForegroundColor(6);
    draw_string(33, 119, "D");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ISMISS APPLICANT");
    display::graphics.setForegroundColor(6);
    draw_string(194, 119, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ECRUIT APPLICANT");

    return;
}


/**
 * Copy astronaut data from the roster pool into the player data.
 *
 * When an astronaut is recruited from the pool of candidates, there
 * is a chance of losing a point of Capsule or Endurance, and they will
 * lose 3 points randomly split among LM, EVA, and Docking (which can
 * take the astronaut below 0).
 *
 * \param plr   0 for the USA, 1 for the USSR.
 * \param pool  The index of the astronaut in the recruited 'nauts pool.
 * \param candidate  The index of the candidate among all available 'nauts.
 */
void Recruit(const char plr, const uint8_t pool, const uint8_t candidate)
{
    struct Astros &recruit = Data->P[plr].Pool[pool];

    strcpy(&recruit.Name[0], &Men[candidate].Name[0]);
    recruit.Sex = Men[candidate].Sex;
    recruit.Cap = Men[candidate].Cap;
    recruit.LM = Men[candidate].LM;
    recruit.EVA = Men[candidate].EVA;
    recruit.Docking = Men[candidate].Docking;
    recruit.Endurance = Men[candidate].Endurance;
    recruit.Status = AST_ST_TRAIN_BASIC_1;
    recruit.TrainingLevel = 1;
    recruit.Group = Data->P[plr].AstroLevel;
    recruit.CR = brandom(2) + 1;
    recruit.CL = brandom(2) + 1;
    recruit.Task = 0;
    recruit.Crew = 0;
    recruit.Unassigned = 0;
    recruit.Pool = 0;
    recruit.Compat = brandom(options.feat_compat_nauts) + 1; //Naut Compatibility, Nikakd, 10/8/10
    recruit.Mood = 85 + 5 * brandom(4);
    recruit.Face = recruit.Sex ? brandom(77) : (77 + brandom(8));

    if (brandom(10) > 5) {
        if (brandom(2) == 0) {
            recruit.Endurance--;
        } else {
            recruit.Cap--;
        }
    }

    for (uint8_t j = 0; j < 3; j++) {
        uint8_t skill = brandom(3);

        switch (skill) {
        case 0:
            recruit.LM--;
            break;

        case 1:
            recruit.EVA--;
            break;

        case 2:
            recruit.Docking--;
            break;

        default:
            break;
        }
    }
}


//Naut Randomize, Nikakd, 10/8/10
// Note: These stats are far more generous than the historical stats.
void RandomizeNauts()
{
    int i;

    for (i = 0; i < 106; i++) {
        Men[i].Cap = brandom(5);
        Men[i].LM  = brandom(5);
        Men[i].EVA = brandom(5);
        Men[i].Docking = brandom(5);
        Men[i].Endurance = brandom(5);
    }
}


void AstSel(char plr)
{
    char i, j, k, BarA, BarB, MaxMen, Index, now, now2, max, maxLeft, change, min, count,
         ksel = 0;
    FILE *fin;

    bool femaleAstronautsAllowed =
        (options.feat_female_nauts ||
         Data->P[plr].FemaleAstronautsAllowed == 1);
    bool femaleAstronautsRequired = Data->P[plr].FemaleAstronautsAllowed;

    MaxMen = Index = now = now2 = max = min = count = 0;

    music_start(M_DRUMSM);
    DrawAstCheck(plr);
    WaitForMouseUp();
    i = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 100 && y >= 135 && x <= 148 && y <= 147 && mousebuttons > 0) || key == 'Y') {

            InBox(100, 135, 148, 147);
            WaitForMouseUp();

            if (Data->P[plr].AstroLevel == 0) {
                Index = 20;
            } else {
                Index = 15;
            }

            if (Data->P[plr].Cash >= Index) {
                i = 1;
            } else {
                i = 2;
            }

            if (Data->P[plr].AstroDelay > 0) {
                i = 2;
            }

            if (key > 0) {
                delay(150);
            }

            OutBox(100, 135, 148, 147);
        }

        if ((x >= 168 && y >= 135 && x <= 216 && y <= 147 && mousebuttons > 0)
            || key == 'N' || key == K_ESCAPE || key == K_ENTER) {

            InBox(168, 135, 216, 147);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 2;
            OutBox(168, 135, 216, 147);
        }
    }

    if (i == 2) {
        music_stop();    /* too poor for astronauts or NO */
        return;
    }

    BarA = 0;
    BarB = 0;
    DrawAstSel(plr);

    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
    ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);

    memset(sel, -1, sizeof(sel));
    memset(MCol, 0x00, sizeof(MCol));

    //TODO: Candidate for protobuf replacement?
    Men = (struct ManPool *) buffer;
    fin = sOpen("MEN.DAT", "rb", 1); /* Open Astronaut Data File  */
    fseek(fin, ((sizeof(struct ManPool)) * 106)*plr, SEEK_SET);
    fread(Men, (sizeof(struct ManPool)) * 106, 1, fin);
    fclose(fin);

    if (options.feat_random_nauts == 1) {
        RandomizeNauts();    //Naut Randomize, Nikakd, 10/8/10
    }

    switch (Data->P[plr].AstroLevel) {
    case 0:
        MaxMen = femaleAstronautsAllowed ? 13 : 10;
        MaxSel = ASTRO_POOL_LVL1;
        Index = 0;
        // TODO: This is an ugly hack...
        Data->P[plr].Cash -= 5;
        Data->P[plr].Spend[0][2] += 5;
        break;

    case 1:
        MaxMen = femaleAstronautsAllowed ? 20 : 17;
        MaxSel = ASTRO_POOL_LVL2;
        Index = 14;
        break;

    case 2:
        MaxMen = femaleAstronautsAllowed ? 22 : 19;
        MaxSel = ASTRO_POOL_LVL3;
        Index = 35;
        break;

    case 3:
        MaxMen = 27;
        MaxSel = ASTRO_POOL_LVL4;
        Index = 58;
        femaleAstronautsRequired = false;
        break;

    case 4:
        MaxMen = 19;
        MaxSel = ASTRO_POOL_LVL5;
        Index = 86;
        femaleAstronautsRequired = false;
        break;
    }

    Data->P[plr].Cash -= 15;
    Data->P[plr].Spend[0][2] += 15;

    now = Index;
    max = Index + MaxMen;
    min = Index;
    now2 = 0;
    count = 0; /* counter for # selected */

    DispEight(now, BarB);
    DispEight2(now2, BarA, count);

    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < 8; i++) {
            // Right Select Box
            if (((x >= 188 && y >= (131 + i * 8) && x <= 312 && y <= (137 + i * 8) && mousebuttons > 0)
                 || (key == RT_ARROW && ksel == 1)) && (now - BarB + i) <= max) {

                if (ksel == 1) {
                    ksel = 0;
                }

                fill_rectangle(186, 129, 314, 195, 0);
                now -= BarB;
                now += i;
                BarB = i;
                ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                DispEight(now, BarB);
                WaitForMouseUp();
            }

            if (((x >= 27 && y >= (131 + i * 8) && x <= 151 && y <= (137 + i * 8) && mousebuttons > 0)
                 || (key == LT_ARROW && ksel == 0)) && (now2 - BarA + i) <= (count - 1)) {
                // Left Select Box

                if (ksel == 0) {
                    ksel = 1;
                }

                fill_rectangle(26, 129, 153, 195, 0);
                now2 -= BarA;
                now2 += i;
                BarA = i;
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                DispEight2(now2, BarA, count);
                WaitForMouseUp();
            }
        }

        // else
        if ((key == K_HOME && ksel == 1) && count > 0) {
            /* Left Home */
            InBox(6, 130, 18, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                //GetMouse();
                delay(10);

                BarA = 0;

                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                DispEight2(now2, BarA, count);

                i = 51;
            }

            BarA = 0;
            now2 = 0;
            fill_rectangle(26, 129, 153, 195, 0);
            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
            DispEight2(now2, BarA, count);

            key = 0;

            //GetMouse();

            OutBox(6, 130, 18, 161);
            delay(10);
        } else if (((x >= 6 && y >= 130 && x <= 18 && y <= 161 && mousebuttons > 0) || (key == UP_ARROW && ksel == 1)) && count > 0) {
            /* Left Up */
            InBox(6, 130, 18, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 0) {
                        if (now2 > 0) {
                            now2--;
                            fill_rectangle(26, 129, 153, 195, 0);
                            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                            DispEight2(now2, BarA, count);
                        }
                    }

                    if (BarA > 0) {
                        fill_rectangle(26, 129, 153, 195, 0);
                        BarA--;
                        now2--;
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispEight2(now2, BarA, count);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == UP_ARROW) {
                delay(100);

                if (BarA == 0) {
                    if (now2 > 0) {
                        now2--;
                        fill_rectangle(26, 129, 153, 195, 0);
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispEight2(now2, BarA, count);
                    }
                }

                if (BarA > 0) {
                    fill_rectangle(26, 129, 153, 195, 0);
                    BarA--;
                    now2--;
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    DispEight2(now2, BarA, count);
                }

                key = 0;

                GetMouse();
            }

            //WaitForMouseUp();
            OutBox(6, 130, 18, 161);
            delay(10);
        } else if (((x >= 6 && y >= 163 && x <= 18 && y <= 194 && mousebuttons > 0) || (key == DN_ARROW && ksel == 1)) && count > 0) {
            /* Left Down */
            InBox(6, 163, 18, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 7) {
                        if (now2 < count - 1) {
                            now2++;
                            fill_rectangle(26, 129, 153, 195, 0);
                            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                            DispEight2(now2, BarA, count);
                        }
                    }

                    if (BarA < 7) {
                        if (now2 < count - 1) {
                            fill_rectangle(26, 129, 153, 195, 0);
                            BarA++;
                            now2++;
                            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                            DispEight2(now2, BarA, count);
                        }
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == DN_ARROW) {
                delay(100);

                if (BarA == 7) {
                    if (now2 < count - 1) {
                        now2++;
                        fill_rectangle(26, 129, 153, 195, 0);
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispEight2(now2, BarA, count);
                    }
                }

                if (BarA < 7) {
                    if (now2 < count - 1) {
                        fill_rectangle(26, 129, 153, 195, 0);
                        BarA++;
                        now2++;
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispEight2(now2, BarA, count);
                    }
                }

                key = 0;

                GetMouse();
            }

            WaitForMouseUp();
            OutBox(6, 163, 18, 194);
            delay(10);

        } else if (key == K_PGDN && ksel == 1) {
            // I couldn't get PgDn to work properly on either side,
            // so remming them out for now. -Leon
            /* Left PgDn */
            /*
            InBox(6, 163, 18, 194);
            delay(10);

            if (now2 > count - 8) {
                now2 += 8;
            } else {
                //BarA = 7;
                now2 = count - 8;
            }
            if ((now2 > count - 8) && (BarA < 7)) {
                BarA = 7 - (count - now2);
            }
            if (now2 > count) {
                now2 = count;
            }

            fill_rectangle(26, 129, 153, 195, 0);
            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
            DispEight2(now2, BarA, count);

            key = 0;
            OutBox(6, 163, 18, 194);
            */
        } else if ((key == K_END && ksel == 1) && count > 0) {
            /* Left End */
            InBox(6, 163, 18, 194);

            maxLeft = max;

            if (count < 7) {
                BarA = count - 1;
            } else {
                BarA = 7;
            }

            now2 = count - 1;
            fill_rectangle(26, 129, 153, 195, 0);
            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
            DispEight2(now2, BarA, count);

            key = 0;

            delay(10);
            OutBox(6, 163, 18, 194);

        } else if (key == K_HOME && ksel == 0) {
            /* Right Home */
            InBox(167, 130, 179, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                BarB = 0;
                ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                DispEight(now, BarB);

                i = 51;
            }

            BarB = 0;
            now = min;
            fill_rectangle(186, 129, 314, 195, 0);
            ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
            DispEight(now, BarB);

            if (BarB == 0) {
                if (now > min) {
                    now = min;
                    fill_rectangle(186, 129, 314, 195, 0);
                    ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                    DispEight(now, BarB);
                }
            }

            if (BarB > 0) {
                fill_rectangle(186, 129, 314, 195, 0);
                BarB = 0;
                now = min;
                ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                DispEight(now, BarB);
            }

            key = 0;

            OutBox(167, 130, 179, 161);
            delay(10);

        } else if (key == K_PGDN && ksel == 0) {
            // I couldn't get PgUp to work properly either,
            // so remming it out for now. -Leon
            /* Right PgUp */
            /*
            InBox(167, 163, 179, 194);
            delay(10);

            if (now > 8) {
                now -= 8;
            } else {
                BarB = 0;
                now = max;
            }

            fill_rectangle(186, 129, 314, 195, 0);
            ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
            DispEight(now, BarB);

            key = 0;
            OutBox(167, 163, 179, 194);
            */
        } else if ((x >= 167 && y >= 130 && x <= 179 && y <= 161 && mousebuttons > 0) || (key == UP_ARROW && ksel == 0)) {
            /* Right Up */
            InBox(167, 130, 179, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarB == 0) {
                        if (now > min) {
                            now--;
                            fill_rectangle(186, 129, 314, 195, 0);
                            ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                            DispEight(now, BarB);
                        }
                    }

                    if (BarB > 0) {
                        fill_rectangle(186, 129, 314, 195, 0);
                        BarB--;
                        now--;
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == UP_ARROW) {
                delay(100);

                if (BarB == 0) {
                    if (now > min) {
                        now--;
                        fill_rectangle(186, 129, 314, 195, 0);
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }
                }

                if (BarB > 0) {
                    fill_rectangle(186, 129, 314, 195, 0);
                    BarB--;
                    now--;
                    ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                    DispEight(now, BarB);
                }

                key = 0;
                GetMouse();
            }

            // WaitForMouseUp();
            OutBox(167, 130, 179, 161);
            delay(10);

        } else if ((x >= 167 && y >= 163 && x <= 179 && y <= 194 && mousebuttons > 0) || (key == DN_ARROW && ksel == 0)) {
            /* Right Down */
            InBox(167, 163, 179, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarB == 7) {
                        if (now <= max) {
                            if (now < max) {
                                now++;
                            }

                            fill_rectangle(186, 129, 314, 195, 0);
                            ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                            DispEight(now, BarB);
                        }
                    }

                    if (BarB < 7) {
                        fill_rectangle(186, 129, 314, 195, 0);
                        BarB++;
                        now++;
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == DN_ARROW) {
                delay(100);

                if (BarB == 7) {
                    if (now <= max) {
                        if (now < max) {
                            now++;
                        }

                        fill_rectangle(186, 129, 314, 195, 0);
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }
                }

                if (BarB < 7) {
                    fill_rectangle(186, 129, 314, 195, 0);
                    BarB++;
                    now++;
                    ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                    DispEight(now, BarB);
                }

                key = 0;

                GetMouse();
            }

            OutBox(167, 163, 179, 194);
            delay(10);

        } else if (key == K_PGDN && ksel == 0) {
            // I couldn't get PgDn to work properly on either side,
            // so remming them out for now. -Leon
            /* Right PgDn */
            /*
            InBox(167, 163, 179, 194);
            delay(10);

            if (now < max - 8) {
                now += 8;
            } else {
                BarB = 7;
                now = max;
            }
            if ((now > max - 8) && (BarB < 7)) {
                BarB = 7 - (max - now);
            }

            fill_rectangle(186, 129, 314, 195, 0);
            ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
            DispEight(now, BarB);

            key = 0;
            OutBox(167, 163, 179, 194);
            */
        } else if (key == K_END && ksel == 0) {
            /* Right End */
            InBox(167, 163, 179, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarB == 7) {
                        now = max;
                        fill_rectangle(186, 129, 314, 195, 0);
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }

                    if (BarB < 7) {
                        fill_rectangle(186, 129, 314, 195, 0);
                        BarB = 7;
                        now = max;
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }

                    i = 51;
                }

                now = max;
                fill_rectangle(186, 129, 314, 195, 0);
                ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                DispEight(now, BarB);

                key = 0;
            }

            // WaitForMouseUp();
            OutBox(167, 163, 179, 194);
            delay(10);

        } else if ((x >= 7 && y >= 111 && x <= 151 && y <= 123 && count > 0 && mousebuttons > 0) || (key == 'D' && count > 0)) {
            /* Dismiss */
            InBox(7, 111, 151, 123);
            count--;
            MCol[sel[now2]] = 0;

            for (i = now2; i < count + 1; i++) {
                sel[i] = sel[i + 1];
            }

            sel[i] = -1; /* remove astronaut from left */

            if (now2 == count) {
                if (now2 > 0) {
                    now2--;
                }

                if (BarA > 0) {
                    BarA--;
                }
            }

            fill_rectangle(26, 129, 153, 195, 0);
            ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
            DispEight2(now2, BarA, count);
            DispEight(now, BarB);

            WaitForMouseUp();

            if (key > 0) {
                delay(110);
            }

            OutBox(7, 111, 151, 123);

        } else if ((x >= 164 && y >= 111 && x <= 313 && y <= 123 && MCol[now] == 0 && mousebuttons > 0) || (key == 'R' && MCol[now] == 0)) {
            /* Recruit */
            InBox(168, 111, 313, 123);

            if (count < MaxSel) {
                sel[count] = now; /* move astronaut into left */
                MCol[now] = 1;
                count++;

                if (BarB == 7) {
                    if (now < max) {
                        now++;
                        fill_rectangle(186, 129, 314, 195, 0);
                        ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                        DispEight(now, BarB);
                    }
                }

                if (BarB < 7) {
                    fill_rectangle(186, 129, 314, 195, 0);
                    BarB++;
                    now++;
                    ShBox(187, 130 + BarB * 8, 313, 138 + BarB * 8);
                    DispEight(now, BarB);
                }

                fill_rectangle(26, 129, 153, 195, 0);
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                DispEight2(now2, BarA, count);
            }

            WaitForMouseUp();

            if (key > 0) {
                delay(110);
            }

            OutBox(168, 111, 313, 123);
        }

        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) { /* Exit - not 'til done */
            bool femaleAstronautsSelected = false;

            if (femaleAstronautsRequired) {
                for (i = 0; i < count; i++) {
                    if (Men[sel[i]].Sex == 1) {
                        femaleAstronautsSelected = true;
                        break;
                    }
                }

                if (! femaleAstronautsSelected) {
                    InBox(245, 5, 314, 17);
                    Help("i100");
                    OutBox(245, 5, 314, 17);
                }
            }

            if ((! femaleAstronautsRequired || femaleAstronautsSelected) &&
                count == MaxSel) {
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                for (i = 0; i < count; i++) {
                    Recruit(plr, i + Data->P[plr].AstroCount, sel[i]);
                }

                Data->P[plr].AstroLevel++;
                Data->P[plr].AstroCount += count;

                switch (Data->P[plr].AstroLevel) {
                case 1:
                    Data->P[plr].AstroDelay = 6;
                    break;

                case 2:
                case 3:
                    Data->P[plr].AstroDelay = 4;
                    break;

                case 4:
                    Data->P[plr].AstroDelay = 8;
                    break;

                case 5:
                    Data->P[plr].AstroDelay = 99;
                    break;
                }

                OutBox(245, 5, 314, 17);

                music_stop();

                return;  /* Done */
            }
        }
    }
}
