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
/** \file ast3.c Training and Hospital code
 */

// This file handles Basic and Advanced Training, Hospital, and Cemetery.

#include "ast3.h"

#include <string>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "ast0.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "options.h"
#include "pace.h"
#include "place.h"
#include "replay.h"
#include "sdlhelper.h"

namespace
{

enum {
    HOSPITAL_BLD = 0,
    CEMETERY_BLD = 1,
};


class VideoBlock
{
public:
    VideoBlock(int plr, std::string video)
    {
        Replay(plr, 0, 4, 28, 149, 82, video);
        AbzFrame(plr, 4, 28, 149, 82, video);
    }

    ~VideoBlock()
    {
        display::graphics.videoRect().w = 0;
        display::graphics.videoRect().h = 0;
    }
};

};  // end of namespace


void DrawTrain(char plr, char lvl);
void TrainText(char plr, int astro, int cnt);
void InjuredNautCenter(char plr, int sel);


void DrawTrain(char plr, char lvl)
{
    char TrnName[20];
    const char *Trner = "TRAINING\0";

    if (lvl == 0) {
        helpText = "i038";
        keyHelpText = "k038";
    } else {
        helpText = "i037";
        keyHelpText = "k037";
    }

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 158, 114);
    ShBox(161, 24, 319, 199);
    ShBox(0, 116, 158, 199);
    fill_rectangle(5, 129, 19, 195, 0);
    fill_rectangle(25, 129, 153, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    IOBox(243, 3, 316, 19);
    IOBox(166, 179, 316, 195);
    InBox(3, 3, 30, 19);
    InBox(202, 28, 283, 79);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    InBox(3, 27, 154, 110);
    draw_up_arrow(9, 133);
    draw_down_arrow(9, 166);
    display::graphics.setForegroundColor(5);

    if (plr == 0) {
        draw_string(20, 124, "ASTRONAUTS");
    } else {
        draw_string(20, 124, "COSMONAUTS");
    }

    draw_string(0, 0, " IN TRAINING");
    memset(TrnName, 0x00, sizeof(TrnName));

    // TODO: Should use AstronautAdvancedFocus enum, but since it
    // doesn't include an option for Basic Training, lvl cannot be
    // defined by the enum, and so cannot be held to the same
    // restrictions. -- rnyoakum
    switch (lvl) {
    case 0:
        strcpy(TrnName, "BASIC ");
        strcat(TrnName, Trner);
        draw_heading(40, 5, TrnName, 0, -1);
        break;

    case 1:
        strcpy(TrnName, "CAPSULE ");
        strcat(TrnName, Trner);
        draw_heading(37, 5, TrnName, 0, -1);
        break;

    case 2:
        strcpy(TrnName, "LM ");
        strcat(TrnName, Trner);
        draw_heading(40, 5, TrnName, 0, -1);
        break;

    case 3:
        strcpy(TrnName, "EVA ");
        strcat(TrnName, Trner);
        draw_heading(40, 5, TrnName, 0, -1);
        break;

    case 4:
        strcpy(TrnName, "DOCKING ");
        strcat(TrnName, Trner);
        draw_heading(37, 5, TrnName, 0, -1);
        break;

    case 5:
        strcpy(TrnName, "ENDURANCE ");
        strcat(TrnName, Trner);
        draw_heading(37, 5, TrnName, 0, -1);
        break;
    }

    display::graphics.setForegroundColor(1);

    draw_string(258, 13, "CONTINUE");

    display::graphics.setForegroundColor(7);

    draw_string(169, 88, "NAME:");      /* 196,32 */

    display::graphics.setForegroundColor(1);

    draw_string(169, 97, "GROUP ");

    draw_string(222, 97, "TENURE: ");

    grMoveTo(201, 97);

    /*  Data->P[plr].AstroLevel     201,41 */

    display::graphics.setForegroundColor(7);

    draw_string(169, 111, "STATUS:");

    draw_string(169, 120, "MOOD:");

    display::graphics.setForegroundColor(9);

    draw_string(169, 133, "SKILL:");

    display::graphics.setForegroundColor(7);

    draw_string(192, 142, "CAP:");

    draw_string(192, 150, "L.M.:");

    draw_string(192, 158, "E.V.A.:");

    draw_string(192, 166, "DOCK:");

    draw_string(192, 174, "END:");

    display::graphics.setForegroundColor(9);

    draw_string(181, 189, "W");

    display::graphics.setForegroundColor(1);

    draw_string(0, 0, "ITHDRAW FROM TRAINING");

    fill_rectangle(203, 29, 282, 78, 7 + (plr * 3));

    draw_small_flag(plr, 4, 4);

    return;
}

void
TrainText(char plr, int astro, int cnt)
{
    char Fritz[20];

    fill_rectangle(200, 83, 291, 88, 3);
    fill_rectangle(200, 116, 220, 120, 3);
    fill_rectangle(202, 93, 220, 97, 3);
    fill_rectangle(216, 102, 294, 106, 3);
    fill_rectangle(212, 107, 317, 124, 3);
    fill_rectangle(215, 138, 235, 142, 3);
    fill_rectangle(213, 145, 243, 150, 3);
    fill_rectangle(221, 153, 241, 158, 3);
    fill_rectangle(220, 161, 240, 166, 3);
    fill_rectangle(215, 170, 235, 174, 3);
    fill_rectangle(282, 91, 318, 97, 3);

    if (cnt == 0) {
        return;
    }

    display::graphics.setForegroundColor(1);
    if (Data->P[plr].Pool[astro].Sex == 1) {
        display::graphics.setForegroundColor(6);  // Show women in navy blue
    }
    draw_string(200, 88, &Data->P[plr].Pool[astro].Name[0]);

    // Print 'naut name in green/yellow/red/black depending on mood -Leon
    int color = MoodColor(Data->P[plr].Pool[astro].Mood);
    display::graphics.setForegroundColor(color);
    draw_string(200, 120, "   ");
    draw_number(200, 120, Data->P[plr].Pool[astro].Mood);
    grMoveTo(212, 111);
    display::graphics.setForegroundColor(11);
    memset(Fritz, 0x00, sizeof(Fritz));

    if (Data->P[plr].Pool[astro].Status >= AST_ST_TRAIN_BASIC_1
        && Data->P[plr].Pool[astro].Status <= AST_ST_TRAIN_BASIC_3) {
        strncpy(Fritz, "BASIC TRAINING ", 14);
    }

    switch (Data->P[plr].Pool[astro].Status) {
    case AST_ST_ACTIVE:
        draw_string(0, 0, "ACTIVE");
        break;

    case AST_ST_TRAIN_BASIC_1:
        strcat(Fritz, " I");
        draw_string(0, 0, &Fritz[0]);
        break;

    case AST_ST_TRAIN_BASIC_2:
        strcat(Fritz, " II");
        draw_string(0, 0, &Fritz[0]);
        break;

    case AST_ST_TRAIN_BASIC_3:
        strcat(Fritz, " III");
        draw_string(0, 0, &Fritz[0]);
        break;

    case AST_ST_TRAIN_ADV_1:
        draw_string(0, 0, "ADV TRAINING I");
        break;

    case AST_ST_TRAIN_ADV_2:
        draw_string(0, 0, "ADV TRAINING II");
        break;

    case AST_ST_TRAIN_ADV_3:
        draw_string(0, 0, "ADV TRAINING III");
        break;

    case AST_ST_TRAIN_ADV_4:
        if (options.feat_shorter_advanced_training) {
            draw_string(0, 0, "ADV TRAINING III");
        } else    {
            draw_string(0, 0, "ADV TRAINING IV");
        }

        break;
    }

    display::graphics.setForegroundColor(11);

    grMoveTo(203, 97);

    switch (Data->P[plr].Pool[astro].Group) {
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

    default:
        draw_string(0, 0, "A.F.");
        break;
    }

    fill_rectangle(264, 87, 315, 100, 3);
    draw_number(264, 97, Data->P[plr].Pool[astro].Active);
    draw_string(0, 0, " SEASON");

    if (Data->P[plr].Pool[astro].Active != 1) {
        draw_string(0, 0, "S");
    }

    display::graphics.setForegroundColor(1);

    draw_number(215, 142, Data->P[plr].Pool[astro].Cap);

    draw_number(213, 150, Data->P[plr].Pool[astro].LM);

    draw_number(221, 158, Data->P[plr].Pool[astro].EVA);

    draw_number(220, 166, Data->P[plr].Pool[astro].Docking);

    draw_number(215, 174, Data->P[plr].Pool[astro].Endurance);

    AstFaces(plr, 203, 29, Data->P[plr].Pool[astro].Face);

    return;
}


/* level is as follows
     0  =  Basic Training
     1  =  Advanced Capsule
     2  =  Advanced LM
     3  =  Advanced EVA
     4  =  Advanced Docking
     5  =  Advanced Endurance
*/

void Train(char plr, int level)
{
    int now2, BarA, count, i, M[100];
    char temp, Train[10];
    static int m = 1;

    for (i = 0; i < 100; i++) {
        M[i] = -1;
    }

    now2 = BarA = count = 0;
    DrawTrain(plr, level);

    for (i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_BASIC_1
            && Data->P[plr].Pool[i].Status <= AST_ST_TRAIN_BASIC_3
            && level == 0) {
            M[count++] = i;
        }

        if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_ADV_1
            && Data->P[plr].Pool[i].Status <= AST_ST_TRAIN_ADV_4
            && Data->P[plr].Pool[i].Focus == level) {
            M[count++] = i;
        }
    }

    DispLeft(plr, BarA, count, now2, &M[0]);

    if (count > 0) {
        TrainText(plr, M[0], count);
    }

    if (count > 8) {
        draw_down_arrow_highlight(9, 166);
    }

    FadeIn(2, 10, 0, 0);

    if (level == 0) {
        if (m > 4) {
            m = 1;
        } else {
            m++;
        }

        level = m;
    }

    memset(Train, 0x00, sizeof(Train));

    if (level == 0) {
        music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);
    } else {
        music_start(M_DRUMSM);
    }

    // TODO: This should use the AstronautAdvancedFocus enum, but
    // level is designed to incorporate Basic training, which is
    // not part of the enum, so consistency between level and the
    // enum cannot be guaranteed.  -- rnyoakum
    switch (level) {
    case 1:
        strcpy(Train, (plr == 0) ? "UTCP" : "STCP");
        break;

    case 2:
        strcpy(Train, (plr == 0) ? "UTLM" : "STLM");
        break;

    case 3:
        strcpy(Train, (plr == 0) ? "UTEV" : "STEV");
        break;

    case 4:
        strcpy(Train, (plr == 0) ? "UTDO" : "STDO");
        break;

    case 5:
        strcpy(Train, (plr == 0) ? "UTDU" : "STDU");
        break;

    default:
        break;
    }

    VideoBlock video(plr, Train);

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < 8; i++) {  // Right Select Box
            if (x >= 27 && y >= (131 + i * 8) && x <= 151 && y <= (137 + i * 8) && mousebuttons > 0 && (now2 - BarA + i) <= (count - 1)) {  // Left

                now2 -= BarA;
                now2 += i;
                BarA = i;
                DispLeft(plr, BarA, count, now2, &M[0]);
                TrainText(plr, M[now2], count);
                WaitForMouseUp();

            }
        }

        if (mousebuttons > 0 || key > 0) {
            if (((x >= 6 && y >= 130 && x <= 18 && y <= 161 && mousebuttons > 0) || key == UP_ARROW) && count > 0) {
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
                                TrainText(plr, M[now2], count);
                            }

                        if (BarA > 0) {
                            BarA--;
                            now2--;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                            TrainText(plr, M[now2], count);
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
                            TrainText(plr, M[now2], count);
                        }

                    if (BarA > 0) {
                        BarA--;
                        now2--;
                        DispLeft(plr, BarA, count, now2, &M[0]);
                        TrainText(plr, M[now2], count);
                    }

                    key = 0;

                    GetMouse();
                }

                // WaitForMouseUp();
                OutBox(6, 130, 18, 161);
                delay(10);
            } else if (key == K_HOME) {
                BarA = 0;
                now2 = 0;
                DispLeft(plr, BarA, count, now2, &M[0]);
                TrainText(plr, M[now2], count);
                key = 0;
                GetMouse();
                OutBox(6, 130, 18, 161);
                delay(10);
            } else if (((x >= 6 && y >= 163 && x <= 18 && y <= 194 && mousebuttons > 0) || key == DN_ARROW) && count > 0) {
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
                                TrainText(plr, M[now2], count);
                            }

                        if (BarA < 7)
                            if (now2 < count - 1) {
                                BarA++;
                                now2++;
                                DispLeft(plr, BarA, count, now2, &M[0]);
                                TrainText(plr, M[now2], count);
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
                            TrainText(plr, M[now2], count);
                        }

                    if (BarA < 7)
                        if (now2 < count - 1) {
                            BarA++;
                            now2++;
                            DispLeft(plr, BarA, count, now2, &M[0]);
                            TrainText(plr, M[now2], count);
                        }

                    key = 0;

                    GetMouse();
                }

                // WaitForMouseUp();
                OutBox(6, 163, 18, 194);
                delay(10);
            } else if (key == K_PGUP) {
                BarA = 0;
                now2 -= 7;

                if (now2 < 0) {
                    now2 = 0;
                }

                DispLeft(plr, BarA, count, now2, &M[0]);
                TrainText(plr, M[now2], count);
                key = 0;
                delay(10);
            } else if (key == K_PGDN) {
                BarA = 7;
                now2 += 7;

                if (now2 > count - 1) {
                    now2 = count - 1;
                }

                DispLeft(plr, BarA, count, now2, &M[0]);
                TrainText(plr, M[now2], count);
                key = 0;
                delay(10);
            } else if (key == K_END) {
                BarA = 7;
                now2 = count - 1;
                DispLeft(plr, BarA, count, now2, &M[0]);
                TrainText(plr, M[now2], count);
                key = 0;
                delay(10);

            } else if (((x >= 168 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0) || key == 'W') && count > 0) {
                InBox(168, 181, 314, 193);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(168, 181, 314, 193);
                // Help box: are you sure you want to withdraw from training early

                if (plr == 0) {
                    temp = Help("i102");
                } else {
                    temp = Help("i109");
                }

                if (temp == 1) {
                    if (Data->P[plr].Pool[M[now2]].Status == AST_ST_TRAIN_ADV_1) {
                        Data->P[plr].Cash += 3;  // refund for early withdrawal
                    }

                    if (Data->P[plr].Pool[M[now2]].Status == AST_ST_TRAIN_ADV_2) {
                        Data->P[plr].Cash += 2;  // partial refund for early withdrawal
                    }

                    if (Data->P[plr].Pool[M[now2]].Status == AST_ST_TRAIN_ADV_3) {
                        Data->P[plr].Cash += 1;  // partial refund for early withdrawal
                    }

                    if (Data->P[plr].Pool[M[now2]].Status == AST_ST_TRAIN_ADV_1 || Data->P[plr].Pool[M[now2]].Status == AST_ST_TRAIN_ADV_2) {
                        Data->P[plr].Pool[M[now2]].TrainingLevel = 0;
                    } else {
                        Data->P[plr].Pool[M[now2]].TrainingLevel = Data->P[plr].Pool[M[now2]].Status;
                    }

                    Data->P[plr].Pool[M[now2]].Status = AST_ST_ACTIVE;
                    Data->P[plr].Pool[M[now2]].Assign = 0;

                    if (Data->P[plr].Pool[M[now2]].Cap < 0) {
                        Data->P[plr].Pool[M[now2]].Cap = 0;
                    }

                    if (Data->P[plr].Pool[M[now2]].LM < 0) {
                        Data->P[plr].Pool[M[now2]].LM = 0;
                    }

                    if (Data->P[plr].Pool[M[now2]].EVA < 0) {
                        Data->P[plr].Pool[M[now2]].EVA = 0;
                    }

                    if (Data->P[plr].Pool[M[now2]].Docking < 0) {
                        Data->P[plr].Pool[M[now2]].Docking = 0;
                    }

                    if (Data->P[plr].Pool[M[now2]].Endurance < 0) {
                        Data->P[plr].Pool[M[now2]].Endurance = 0;
                    }

                    for (i = now2; i < count; i++) {
                        M[i] = M[i + 1];
                    }

                    M[i] = -1;
                    count--;

                    if (count == 0) {
                        fill_rectangle(203, 29, 282, 78, 7 + (plr * 3));
                    }

                    if (now2 == count) {
                        if (now2 > 0) {
                            now2--;
                        }

                        if (BarA > 0) {
                            BarA--;
                        }
                    }

                    DispLeft(plr, BarA, count, now2, &M[0]);

                    TrainText(plr, M[now2], count);

                }

            }  /* end x-y if */
            else if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(245, 5, 314, 17);
                music_stop();
                return;  /* Done */
            }  /* end x-y if */

            if (now2 < count - 1 && count > 8 && (8 - BarA) + now2 < count) {
                draw_down_arrow_highlight(9, 166);
            } else {
                draw_down_arrow(9, 166);
            }

            if (now2 - BarA > 0) {
                draw_up_arrow_highlight(9, 133);
            } else {
                draw_up_arrow(9, 133);
            }

        }  /* end mouse if */
    }  /* end while */
}  /* end Limbo */


/* Interface for the Hospital/Cemetery.
 * sel is 0 for hospital, 1 for cemetery.
 */
void InjuredNautCenter(char plr, int sel)
{
    int now2;
    int BarA;
    int count;
    int i;
    int j;
    int M[100];

    if (sel == HOSPITAL_BLD) {
        helpText = "i041";
        keyHelpText = "k041";
    } else {
        helpText = "i020";
        keyHelpText = "k020";
    }

    for (i = 0; i < 100; i++) {
        M[i] = 0;
    }

    now2 = 0;
    BarA = count = 0;

    FadeOut(2, 10, 0, 0);

    char filename[128];

    if (sel == HOSPITAL_BLD) {
        snprintf(filename, sizeof(filename), "images/hospital.%d.png", plr);
    } else if (sel == CEMETERY_BLD) {
        snprintf(filename, sizeof(filename), "images/cemetery.%d.png", plr);
    }

    boost::shared_ptr<display::PalettizedSurface> location(Filesystem::readImage(filename));
    location->exportPalette(32, 255);

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(161, 103, 319, 199);
    ShBox(0, 103, 158, 199);
    IOBox(243, 3, 316, 19);
    fill_rectangle(5, 129, 19, 195, 0);
    fill_rectangle(25, 129, 153, 195, 0);
    ShBox(6, 163, 18, 194);
    ShBox(6, 130, 18, 161);
    InBox(3, 3, 30, 19);
    InBox(26, 38, 107, 89);
    InBox(166, 107, 313, 195);
    InBox(9, 109, 148, 123);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    draw_up_arrow(9, 133);
    draw_down_arrow(9, 166);
    draw_small_flag(plr, 4, 4);
    ShBox(0, 24, 319, 101);

    display::graphics.screen()->draw(location, 0, 0, 318, 75, 1, 25);

    display::graphics.screen()->draw(location, 0, 81, 146, 86, 167, 108);

    display::graphics.setForegroundColor(1);

    if (plr == 0) {
        if (sel == HOSPITAL_BLD) {
            draw_heading(40, 5, "US HOSPITAL", 0, -1);
            music_start(M_BADNEWS);
        } else {
            draw_heading(37, 5, "ARLINGTON CEMETERY", 0, -1);
            music_start(M_USFUN);
        }
    }

    if (plr == 1) {
        if (sel == HOSPITAL_BLD) {
            draw_heading(40, 5, "SOVIET INFIRMARY", 0, -1);
            music_start(M_INTERLUD);
        } else {
            draw_heading(40, 5, "KREMLIN WALL", 0, -1);
            music_start(M_SVFUN);
        }
    }

    display::graphics.setForegroundColor(1);
    draw_string(257, 13, "CONTINUE");
    display::graphics.setForegroundColor(11);

    if (plr == 0) {
        draw_string(25, 118, "ASTRONAUT");
    } else {
        draw_string(25, 118, "COSMONAUT");
    }

    draw_string(0, 0, " SELECTION");

    j = (sel == HOSPITAL_BLD) ? AST_ST_INJURED : AST_ST_DEAD;

    for (i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status == j) {
            M[count++] = i;
        }
    }

    DispLeft(plr, BarA, count, now2, &M[0]);

    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < 8; i++) {
            if (x >= 27 && y >= (131 + i * 8) && x <= 151 && y <= (137 + i * 8) && mousebuttons > 0 && (now2 - BarA + i) <= (count - 1)) {

                now2 -= BarA;
                now2 += i;
                BarA = i;
                DispLeft(plr, BarA, count, now2, &M[0]);
                WaitForMouseUp();

            }
        }

        if (((x >= 6 && y >= 130 && x <= 18 && y <= 161 && mousebuttons > 0) || key == UP_ARROW) && count > 0) {
            // Up
            InBox(6, 130, 18, 161);

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

            // WaitForMouseUp();
            OutBox(6, 130, 18, 161);

            delay(10);
        } else if (((mousebuttons > 0 && x >= 6 && y >= 163 && x <= 18 && y <= 194) || key == DN_ARROW) && count > 0) {
            // Down
            InBox(6, 163, 18, 194);

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

            // WaitForMouseUp();
            OutBox(6, 163, 18, 194);

            delay(10);
        }

        if ((mousebuttons > 0 && x >= 245 && y >= 5 && x <= 314 && y <= 17) || key == K_ENTER || key == K_ESCAPE) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();
            music_stop();
            return;  /* Done */
        }  /* end x-y if */
    }  /* end while */
}



/* Interface for the Cemetery.
 */
void Cemetery(char plr)
{
    InjuredNautCenter(plr, CEMETERY_BLD);
}


/* Interface for the Hospital.
 */
void Hospital(char plr)
{
    InjuredNautCenter(plr, HOSPITAL_BLD);
}

