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

// This file handles the Astronaut Complex / Cosmonaut Center.

#include "display/graphics.h"

#include "ast2.h"
#include "Buzz_inc.h"
#include "ast0.h"
#include "ast3.h"
#include "ast4.h"
#include "game_main.h"
#include "draw.h"
#include "place.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"


void DrawLimbo(char plr);
void Clear(void);
void LimboText(char plr, int astro);


void DrawLimbo(char plr)
{
    int i;
    FadeOut(2, 10, 0, 0);

    helpText = "i039";
    keyHelpText = "k039";
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    if (plr == 0) {
        draw_heading(36, 5, "ASTRONAUT COMPLEX", 0, -1);
    } else {
        draw_heading(40, 5, "COSMONAUT CENTER", 0, -1);
    }

    ShBox(0, 24, 158, 199);
    InBox(9, 51, 90, 102);
    fill_rectangle(25, 129, 153, 195, 0);
    fill_rectangle(5, 129, 19, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    draw_up_arrow(8, 133);
    draw_down_arrow(8, 166);
    ShBox(161, 24, 319, 72);
    InBox(165, 28, 315, 42);
    fill_rectangle(166, 29, 314, 41, 7);
    ShBox(161, 74, 319, 199);
    InBox(165, 77, 315, 89);
    fill_rectangle(166, 78, 314, 88, 10);
    fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
    InBox(165, 46, 315, 67);
    fill_rectangle(166, 47, 314, 66, 0);
    ShBox(167, 48, 239, 65);
    ShBox(241, 48, 313, 65);
    InBox(167, 48, 239, 65);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    // new boxes
    OutBox(0, 24, 158, 47);
    OutBox(0, 105, 158, 128);
    InBox(9, 28, 148, 42);
    InBox(9, 108, 148, 123);

    display::graphics.setForegroundColor(20);
    draw_string(180, 55, "TRANSFER");

    if (plr == 0) {
        draw_string(178, 62, "ASTRONAUT");
    } else {
        draw_string(178, 62, "COSMONAUT");
    }

    draw_string(256, 55, "FACILITY");
    draw_string(254, 62, "TRANSFER");

    fill_rectangle(166, 78, 314, 88, 10);
    display::graphics.setForegroundColor(11);
    draw_string(185, 85, "TRANSFER TO LOCATION");

    for (i = 0; i < 5; i++) {
        IOBox(165, 93 + 21 * i, 238, 111 + 21 * i);
        IOBox(242, 93 + 21 * i, 315, 111 + 21 * i);

        if (Data->P[plr].Manned[i].Num < 0) {
            InBox(167, 95 + 21 * i, 236, 109 + 21 * i);
            display::graphics.setForegroundColor(17);
        } else {
            display::graphics.setForegroundColor(11);
        }

        char str[10];
        sprintf(&str[0], Data->P[plr].Manned[i].Name);
        draw_string(201 - TextDisplayLength(&str[0]) / 2, 101 + 21 * i, &str[0]);  // Program name is centered
        draw_string(181, 107 + 21 * i, "PROGRAM");

        display::graphics.setForegroundColor(11);
        draw_string(257, 107 + 21 * i, "TRAINING");
        //grMoveTo(254,101+21*i);  This remmed out to manually place the Adv. Training names so they could be centered -Leon

        switch (i + AST_FOCUS_CAPSULE) {
        case AST_FOCUS_CAPSULE:
            draw_string(258, 101 + 21 * i, "CAPSULE");
            break;

        case AST_FOCUS_LEM:
            draw_string(271, 101 + 21 * i, "L.M.");
            break;

        case AST_FOCUS_EVA:
            draw_string(268, 101 + 21 * i, "E.V.A.");
            break;

        case AST_FOCUS_DOCKING:
            draw_string(259, 101 + 21 * i, "DOCKING");
            break;

        case AST_FOCUS_ENDURANCE:
            draw_string(252, 101 + 21 * i, "ENDURANCE");
            break;

        default:
            break;
        }


    }

    display::graphics.setForegroundColor(11);
    draw_string(187, 37, "OPERATION SELECTION");

    display::graphics.setForegroundColor(20);
    //draw_string(256,13,"CONTINUE");
    draw_string(15, 37, "NAME:");
    draw_string(17, 118, "GROUP:");
    draw_string(88, 118, "TENURE:");
    display::graphics.setForegroundColor(11);
    draw_string(102, 60, "MOOD:");
    draw_string(102, 68, "CAP:");
    draw_string(102, 76, "L.M.:");
    draw_string(102, 84, "EVA:");
    draw_string(102, 92, "DOCK:");
    draw_string(102, 100, "END:");
    draw_small_flag(plr, 4, 4);

    return;
}

void Limbo(char plr)
{
    int i, AstroList[100], BarA, count, now2, tag = 0;
    memset(AstroList, -1, sizeof AstroList);
    DrawLimbo(plr);
    music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

    now2 = BarA = count = 0;
    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);

    for (i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status == AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign == 0) {
            AstroList[count++] = i;
        }
    }

    DispLeft(plr, BarA, count, now2, &AstroList[0]);

    if (count > 0) {
        LimboText(plr, AstroList[now2]);
    }

    FadeIn(2, 10, 0, 0);

    while (1) {
        key = 0;
        WaitForMouseUp();
        key = 0;
        GetMouse();

        //Mouse ManSelect from being Clicked on
        for (i = 0; i < 8; i++) {
            if (x >= 27 && y >= (131 + i * 8) && x <= 151 && y <= (137 + i * 8) && mousebuttons > 0 && (now2 - BarA + i) <= (count - 1)) {
                now2 -= BarA;
                now2 += i;
                BarA = i;
                fill_rectangle(26, 129, 153, 195, 0);
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                DispLeft(plr, BarA, count, now2, &AstroList[0]);
                LimboText(plr, AstroList[now2]);
                WaitForMouseUp();

            }
        }

        // Left Arrow Up
        if ((mousebuttons > 0 && x >= 6 && y >= 130 && x <= 18 && y <= 161 && count > 0) || key == UP_ARROW) {
            InBox(6, 130, 18, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 0 && now2 > 0) {
                        now2--;
                        fill_rectangle(26, 129, 153, 195, 0);
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispLeft(plr, BarA, count, now2, &AstroList[0]);
                        LimboText(plr, AstroList[now2]);
                    }

                    if (BarA > 0) {
                        fill_rectangle(26, 129, 153, 195, 0);
                        BarA--;
                        now2--;
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispLeft(plr, BarA, count, now2, &AstroList[0]);
                        LimboText(plr, AstroList[now2]);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == UP_ARROW) {
                delay(100);

                if (BarA == 0 && now2 > 0) {
                    now2--;
                    fill_rectangle(26, 129, 153, 195, 0);
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    DispLeft(plr, BarA, count, now2, &AstroList[0]);
                    LimboText(plr, AstroList[now2]);
                }

                if (BarA > 0) {
                    fill_rectangle(26, 129, 153, 195, 0);
                    BarA--;
                    now2--;
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    DispLeft(plr, BarA, count, now2, &AstroList[0]);
                    LimboText(plr, AstroList[now2]);
                }

                key = 0;

                GetMouse();
            }

            //WaitForMouseUp();key=0;
            OutBox(6, 130, 18, 161);
            delay(10);
        }

        // Left Arrow Down
        if ((mousebuttons > 0 && x >= 6 && y >= 163 && x <= 18 && y <= 194 && count > 0) || key == DN_ARROW) {
            InBox(6, 163, 18, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 7 && (now2 < count - 1)) {
                        now2++;
                        fill_rectangle(26, 129, 153, 195, 0);
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispLeft(plr, BarA, count, now2, &AstroList[0]);
                        LimboText(plr, AstroList[now2]);
                    }

                    if (BarA < 7 && now2 < count - 1) {
                        fill_rectangle(26, 129, 153, 195, 0);
                        BarA++;
                        now2++;
                        ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                        DispLeft(plr, BarA, count, now2, &AstroList[0]);
                        LimboText(plr, AstroList[now2]);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == DN_ARROW) {
                delay(100);

                if (BarA == 7 && (now2 < count - 1)) {
                    now2++;
                    fill_rectangle(26, 129, 153, 195, 0);
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    DispLeft(plr, BarA, count, now2, &AstroList[0]);
                    LimboText(plr, AstroList[now2]);
                }

                if (BarA < 7 && now2 < count - 1) {
                    fill_rectangle(26, 129, 153, 195, 0);
                    BarA++;
                    now2++;
                    ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
                    DispLeft(plr, BarA, count, now2, &AstroList[0]);
                    LimboText(plr, AstroList[now2]);
                }

                key = 0;

                GetMouse();
            }

            //key=0;WaitForMouseUp();key=0;
            OutBox(6, 163, 18, 194);
            delay(10);
        }

        // Continue
        if ((mousebuttons > 0 && x >= 245 && y >= 5 && x <= 314 && y <= 17) || key == K_ENTER) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);
            music_stop();
            return;
        }

        // Select Transfer Button
        if ((mousebuttons > 0 && x >= 167 && y >= 48 && x <= 239 && y <= 65) || key == LT_ARROW) {
            tag = 0;
            InBox(167, 48, 239, 65);
            OutBox(241, 48, 313, 65);
            fill_rectangle(166, 78, 314, 88, 10);
            display::graphics.setForegroundColor(11);
            draw_string(185, 85, "TRANSFER TO LOCATION");

        }

        // Select Visit To Button
        if ((mousebuttons > 0 && x >= 241 && y >= 48 && x <= 313 && y <= 65) || key == RT_ARROW) {
            tag = 1;
            InBox(241, 48, 313, 65);
            OutBox(167, 48, 239, 65);
            fill_rectangle(166, 78, 314, 88, 10);
            display::graphics.setForegroundColor(11);
            draw_string(203, 85, "VISIT LOCATION");

        }

        // Selection Loops
        for (i = 0; i < 5; i++) {
            // Program Transfer
            if ((tag == 0 && Data->P[plr].Manned[i].Num >= 0) &&
                ((mousebuttons > 0 && x >= 167 && y >= (95 + 21 * i) && x <= 236 && y <= (109 + 21 * i))
                 || key == 0x0030 + i)) {
                InBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(167, 95 + 21 * i, 236, 109 + 21 * i);
                Data->P[plr].Pool[AstroList[now2]].Assign = i + 1;
                Data->P[plr].Pool[AstroList[now2]].Unassigned = 0;
                Data->P[plr].Pool[AstroList[now2]].Moved = 0;

                for (i = now2; i < count; i++) {
                    AstroList[i] = AstroList[i + 1];
                }

                AstroList[i] = -1;
                count--;

                if (count == 0) {
                    fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
                    Clear();
                }

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

                DispLeft(plr, BarA, count, now2, &AstroList[0]);

                if (count > 0) {
                    LimboText(plr, AstroList[now2]);
                }

            }

            // Program Visit
            if ((tag == 1 && Data->P[plr].Manned[i].Num >= 0) &&
                ((mousebuttons > 0 && x >= 167 && y >= (95 + 21 * i) && x <= 236 && y <= (109 + 21 * i))
                 || key == 0x0030 + i)) {
                InBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(167, 95 + 21 * i, 236, 109 + 21 * i);
                music_stop();
                Programs(plr, i + 1);
                DrawLimbo(plr);
                music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

                InBox(241, 48, 313, 65);
                OutBox(167, 48, 239, 65);
                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(203, 85, "VISIT LOCATION");
                fill_rectangle(26, 129, 153, 195, 0);
                now2 = BarA = count = 0;
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);

                for (i = 0; i < Data->P[plr].AstroCount; i++) {
                    if (Data->P[plr].Pool[i].Status == AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign == 0) {
                        AstroList[count++] = i;
                    }
                }

                DispLeft(plr, BarA, count, now2, &AstroList[0]);

                if (count > 0) {
                    LimboText(plr, AstroList[now2]);
                }

                //   ShBox(26,130+BarA*8,152,138+BarA*8);
                //   DispLeft(plr,BarA,count,now2,&AstroList[0]);
                //   LimboText(plr,AstroList[now2]);
                FadeIn(2, 10, 0, 0);

            }


            // Training Transfer
            if ((tag == 0 && count > 0) && ((mousebuttons > 0 && x >= 244 && y >= (95 + 21 * i) && x <= 313 && y <= (109 + 21 * i)) || key == 0x0035 + i)) {
                InBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                int skilLev = 0;

                // Figure out relevant skill level before sending to
                // Adv Training -Leon
                if (i == 0) {
                    skilLev = Data->P[plr].Pool[AstroList[now2]].Cap;
                }

                if (i == 1) {
                    skilLev = Data->P[plr].Pool[AstroList[now2]].LM;
                }

                if (i == 2) {
                    skilLev = Data->P[plr].Pool[AstroList[now2]].EVA;
                }

                if (i == 3) {
                    skilLev = Data->P[plr].Pool[AstroList[now2]].Docking;
                }

                if (i == 4) {
                    skilLev = Data->P[plr].Pool[AstroList[now2]].Endurance;
                }

                display::graphics.setForegroundColor(2);

                if (Data->P[plr].Pool[AstroList[now2]].TrainingLevel > 6) {
                    Help("i120");
                } else if (skilLev > 3) {
                    // If they have a 4 in that skill, don't send to
                    // Adv Training  -Leon
                    OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                } else if (Data->P[plr].Cash < 3) {
                    Help("i121");
                } else {
                    if (skilLev > 2) {
                        // If they have a 3 in that skill, send them
                        // directly to Adv III and charge just 2MB  -Leon
                        Data->P[plr].Pool[AstroList[now2]].Status = AST_ST_TRAIN_ADV_3;
                        Data->P[plr].Cash -= 2;
                    } else {
                        Data->P[plr].Pool[AstroList[now2]].Status = AST_ST_TRAIN_ADV_1;
                        Data->P[plr].Cash -= 3;
                    }

                    Data->P[plr].Pool[AstroList[now2]].Focus = i + 1;
                    Data->P[plr].Pool[AstroList[now2]].Assign = 0;
                    Data->P[plr].Pool[AstroList[now2]].Unassigned = 0;
                    Data->P[plr].Pool[AstroList[now2]].Moved = 0;

                    for (i = now2; i < count; i++) {
                        AstroList[i] = AstroList[i + 1];
                    }

                    AstroList[i] = -1;
                    count--;

                    if (count == 0) {
                        fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
                        Clear();
                    }

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

                    DispLeft(plr, BarA, count, now2, &AstroList[0]);

                    if (count > 0) {
                        LimboText(plr, AstroList[now2]);
                    }
                }  // else
            }  // if adv training

            // Training Visit
            if (tag == 1 &&
                ((mousebuttons > 0 && x >= 244 && y >= (95 + 21 * i) && x <= 313 && y <= (109 + 21 * i)) || key == 0x0035 + i)) {
                InBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                music_stop();
                Train(plr, i + 1);
                DrawLimbo(plr);
                music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(203, 85, "VISIT LOCATION");
                InBox(241, 48, 313, 65);
                OutBox(167, 48, 239, 65);
                fill_rectangle(26, 129, 153, 195, 0);
                now2 = BarA = count = 0;
                ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);

                for (i = 0; i < Data->P[plr].AstroCount; i++) {
                    if (Data->P[plr].Pool[i].Status == AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign == 0) {
                        AstroList[count++] = i;
                    }
                }

                DispLeft(plr, BarA, count, now2, &AstroList[0]);

                if (count > 0) {
                    LimboText(plr, AstroList[now2]);
                }

                FadeIn(2, 10, 0, 0);

            }
        }

    }  /* end while */
}  /* end Limbo */

void Clear(void)
{
    fill_rectangle(44, 31, 145, 40, 3);
    fill_rectangle(49, 112, 60, 119, 3);
    fill_rectangle(123, 62, 145, 77, 3);
    fill_rectangle(125, 79, 145, 85, 3);
    fill_rectangle(128, 86, 145, 93, 3);
    fill_rectangle(123, 95, 145, 101, 3);
    fill_rectangle(127, 113, 141, 120, 3);
    fill_rectangle(130, 54, 155, 61, 3);
    return;
}

void LimboText(char plr, int astro)
{
    fill_rectangle(44, 31, 145, 40, 3);
    display::graphics.setForegroundColor(11);

    int color = MoodColor(Data->P[plr].Pool[astro].Mood);

    // Print 'naut name in green/yellow/red/black depending on mood -Leon
    display::graphics.setForegroundColor(color);

    // Print name in black if 'naut has announced retirement (override mood) -Leon
    if (Data->P[plr].Pool[astro].RetirementDelay > 0) {
        display::graphics.setForegroundColor(0);
    }

    draw_string(46, 37, Data->P[plr].Pool[astro].Name);
    display::graphics.setForegroundColor(11);
    fill_rectangle(49, 112, 80, 119, 3);
    display::graphics.setForegroundColor(11);

    switch (Data->P[plr].Pool[astro].Group) {
    case 0:
        draw_string(53, 118, "I");
        break;

    case 1:
        draw_string(53, 118, "II");
        break;

    case 2:
        draw_string(53, 118, "III");
        break;

    case 3:
        draw_string(53, 118, "IV");
        break;

    case 4:
        draw_string(53, 118, "V");
        break;
    }

    fill_rectangle(123, 62, 145, 77, 3);
    fill_rectangle(125, 79, 145, 85, 3);
    fill_rectangle(131, 86, 145, 92, 3);
    fill_rectangle(123, 95, 145, 101, 3);
    fill_rectangle(130, 54, 155, 61, 3);

    // Print 'naut mood in green/yellow/red/black depending on mood -Leon
    display::graphics.setForegroundColor(color);
    draw_number(132, 60, Data->P[plr].Pool[astro].Mood);
    display::graphics.setForegroundColor(11);
    draw_number(125, 68, Data->P[plr].Pool[astro].Cap);
    draw_number(123, 76, Data->P[plr].Pool[astro].LM);
    draw_number(125, 84, Data->P[plr].Pool[astro].EVA);
    draw_number(131, 92, Data->P[plr].Pool[astro].Docking);
    draw_number(125, 100, Data->P[plr].Pool[astro].Endurance);
    fill_rectangle(127, 113, 141, 120, 3);
    display::graphics.setForegroundColor(11);
    draw_number(130, 118, Data->P[plr].Pool[astro].Active);
    AstFaces(plr, 10, 52, Data->P[plr].Pool[astro].Face);
    return;
}


// EOF
