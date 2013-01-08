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

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "newmis.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "draw.h"
#include "mis_c.h"
#include "place.h"
#include "port.h"
#include "radar.h"
#include "mc.h"
#include "rush.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "filesystem.h"

struct order Order[7] ;
unsigned int colss;

char Month[12][11] = {
    "JANUARY ", "FEBRUARY ", "MARCH ", "APRIL ", "MAY ", "JUNE ",
    "JULY ", "AUGUST ", "SEPTEMBER ", "OCTOBER ", "NOVEMBER ", "DECEMBER "
};

void MisOrd(char num);


/** Function to compare two Missions
 *
 * Orders by date and budget
 *
 * \param
 * \param
 *
 * \return -1 if first date earlier than second
 * \return 0 if date and budget are the same
 * \return 1 if first budget is lower than second
 */
static int
cmp_order(const void *p1, const void *p2)
{
    struct order *o1 = (struct order *) p1;
    struct order *o2 = (struct order *) p2;

    if (o1->date < o2->date) {
        return -1;
    } else if (o1->date == o2->date) {
        if (o1->budget > o2->budget) {
            return -1;
        } else if (o1->budget == o2->budget) {
            char whoFirst = brandom(2);

            if (whoFirst == 1) {
                return 1;
            } else {
                return -1;
            }
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

char
OrderMissions(void)
{
    int i, j, k;

    memset(Order, 0x00, sizeof Order);
    // Sort Missions for Proper Order
    k = 0;

    for (i = 0; i < NUM_PLAYERS; i++)
        for (j = 0; j < MAX_MISSIONS; j++)
            if (Data->P[i].Mission[j].MissionCode
                && Data->P[i].Mission[j].part != 1) {
                Order[k].plr = i;
                Order[k].loc = j;
                Order[k].budget = Data->P[i].Budget;
                Order[k].date = Data->P[i].Mission[j].Month;
                k++;
            };

    if (k) {
        qsort(Order, k, sizeof(struct order), cmp_order);
    }

    if (MAIL == -1 && Option == -1 && AI[0] == 0 && AI[1] == 0 && k != 0) {
        MisOrd(k);
    }

    return k;
}

void MisOrd(char num)
{
    int i, j = 0;

    ShBox(63, 19, 257, 173);
    InBox(74, 36, 246, 163);
    display::graphics.setForegroundColor(36);
    draw_string(77, 30, "       LAUNCH ORDER");

    for (i = 0; i < num; i++) {
        InBox(78, 39 + 21 * j, 105, 55 + 21 * j);
        draw_small_flag(Order[i].plr, 79, 40 + 21 * j);
        display::graphics.setForegroundColor(34);
        draw_string(110, 45 + 21 * j, "SCHEDULED LAUNCH");
        draw_string(110, 52 + 21 * j, "DATE: ");
        display::graphics.setForegroundColor(1);

        draw_string(0, 0,
                    Month[Data->P[Order[i].plr].Mission[Order[i].loc].Month]);

        draw_string(0, 0, " 19");
        draw_number(0, 0, Data->Year);
        j++;
    };

    FadeIn(2, display::graphics.palette(), 10, 0, 0);

    WaitForMouseUp();

    WaitForKeyOrMouseDown();

    WaitForMouseUp();

    FadeOut(2, display::graphics.palette(), 10, 0, 0);
}


void MisAnn(char plr, char pad)
{
    int i, j, bud;
    struct mStr Mis2;
    char k, hold, Digit[4], HelpFlag = 0;
    char pad_str[2] = {'A' + pad, '\0'};

    for (i = 0; i < 768; i++) {
        display::graphics.palette()[i] = 0;
    }

    display::graphics.screen()->clear();

    PortPal(plr);
    ShBox(41, 20, 281, 184);
    InBox(46, 25, 276, 179);
    InBox(46, 25, 117, 65);
    draw_flag(47, 26, plr);
    InBox(122, 25, 276, 65);
    display::graphics.setForegroundColor(9);
    draw_string(127, 33, "SCHEDULED LAUNCH"); //was 154,33
    display::graphics.setForegroundColor(34);
    draw_string(127, 40, "LAUNCH FACILITY: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "PAD ");
    draw_string(0, 0, pad_str);
    display::graphics.setForegroundColor(34);
    draw_string(127, 47, "DATE: ");
    display::graphics.setForegroundColor(1);

    draw_string(0, 0, Month[Data->P[plr].Mission[pad].Month]);

    draw_string(0, 0, "19");
    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(1);

    GetMisType(Data->P[plr].Mission[pad].MissionCode);
    memcpy(&Mis2, &Mis, sizeof Mis);


    if ((Mis.mVab[0] & 0x10) == 0x10 && Data->P[plr].DockingModuleInOrbit <= 0) {
        i = 0;

        while (dg[Mis2.Index][i] != 0 && Mis.Doc == 1) {
            GetMisType(dg[Mis2.Index][i]);
            i++;
        }

        if (dg[Mis2.Index][i] == 0) {
            Data->P[plr].Mission[pad].MissionCode = 4;    // new mission
        } else {
            Data->P[plr].Mission[pad].MissionCode = Mis.Index;    // new mission
        }

        GetMisType(Data->P[plr].Mission[pad].MissionCode);
        HelpFlag = 1;
    }

    draw_string(127, 54, Mis.Abbr);
    int MisCod;
    MisCod = Data->P[plr].Mission[pad].MissionCode;

    if ((MisCod > 24 && MisCod < 32) || MisCod == 33 || MisCod == 34 || MisCod == 35 || MisCod == 37 || MisCod == 40 || MisCod == 41)
        // Show duration level only on missions with a Duration step - Leon
    {
        switch (Data->P[plr].Mission[pad].Duration) {
        case 1:
            draw_string(0, 0, "");
            break;

        case 2:
            draw_string(0, 0, " (B)");
            break;

        case 3:
            draw_string(0, 0, " (C)");
            break;

        case 4:
            draw_string(0, 0, " (D)");
            break;

        case 5:
            draw_string(0, 0, " (E)");
            break;

        case 6:
            draw_string(0, 0, " (F)");
            break;

        default:
            draw_string(0, 0, "");
            break;
        }
    }

    IOBox(57, 68, 118, 84);
    IOBox(131, 68, 197, 84);
    IOBox(205, 68, 266, 84);
    display::graphics.setForegroundColor(1);
    draw_string(65, 78, "CONTINUE");
    draw_string(139, 78, "PLAY FULL");
    draw_string(221, 78, "SCRUB");
    display::graphics.setForegroundColor(9);
    draw_string(65, 78, "C");
    draw_string(139, 78, "P");
    draw_string(221, 78, "S");

    //IOBox(85,68,158,84);IOBox(172,68,245,84);
    //display::graphics.setForegroundColor(1);draw_string(102,78,"CONTINUE");draw_string(189,78,"SCRUB");
    //display::graphics.setForegroundColor(9);
    //draw_string(102,78,"C");draw_string(189,78,"S");

    if (Data->P[plr].Mission[pad].Joint == 0) {
        draw_string(128, 91, "SINGLE LAUNCH");
    } else {
        draw_string(131, 91, "JOINT LAUNCH");
        draw_string(49, 101, "PART ONE");
        draw_string(102, 101, "PAD ");

        //draw_number(0,0,pad);
        switch (pad) {
        case 0:
            draw_string(0, 0, "A");
            break;

        case 1:
            draw_string(0, 0, "B");
            break;

        case 2:
            draw_string(0, 0, "C");
            break;
        }

        draw_string(160, 101, "PART TWO");

        draw_string(213, 101, "PAD ");

        //draw_number(0,0,pad+1);
        switch (pad + 1) {
        case 0:
            draw_string(0, 0, "A");
            break;

        case 1:
            draw_string(0, 0, "B");
            break;

        case 2:
            draw_string(0, 0, "C");
            break;
        }

    }

    for (i = 0; i < Data->P[plr].Mission[pad].Joint + 1; i++) {
        k = 0;

        if (i == 0) {
            bud = 49;
        } else {
            bud = 160;
        }

        for (j = Mission_Capsule; j <= Mission_PrimaryBooster; j++) {
            hold = Data->P[plr].Mission[pad + i].Hard[j];

            switch (j) {
            case 0:
                if (hold > -1) {
                    display::graphics.setForegroundColor(7);
                    draw_string(bud, 109 + 14 * k, "CAPSULE: ");
                    display::graphics.setForegroundColor(1);
                    draw_string(0, 0, &Data->P[plr].Manned[hold].Name[0]);
                    display::graphics.setForegroundColor(11);
                    draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                    Data->P[plr].Manned[hold].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                    sprintf(&Digit[0], "%d", Data->P[plr].Manned[hold].Safety + Data->P[plr].Manned[hold].Damage);
                    draw_string(0, 0, &Digit[0]);
                    draw_string(0, 0, "%");
                    // draw_string(144+i*111,116+14*k,"%");
                    ++k;
                };

                break;

            case 1:
                if (hold > -1) {
                    display::graphics.setForegroundColor(7);
                    draw_string(bud, 109 + 14 * k, "KICKER: ");
                    display::graphics.setForegroundColor(1);
                    draw_string(0, 0, &Data->P[plr].Misc[hold].Name[0]);
                    display::graphics.setForegroundColor(11);
                    draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                    Data->P[plr].Misc[hold].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                    sprintf(&Digit[0], "%d", Data->P[plr].Misc[hold].Safety + Data->P[plr].Misc[hold].Damage);
                    draw_string(0, 0, &Digit[0]);
                    draw_string(0, 0, "%");
                    // draw_number(0,0,Data->P[plr].Misc[hold].Safety);
                    //    draw_string(144+i*111,116+14*k,"%");
                    ++k;
                };

                break;

            case 2:
                if (hold > -1) {
                    display::graphics.setForegroundColor(7);
                    draw_string(bud, 109 + 14 * k, "LM: ");
                    display::graphics.setForegroundColor(1);
                    draw_string(0, 0, &Data->P[plr].Manned[hold].Name[0]);
                    display::graphics.setForegroundColor(11);
                    draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                    Data->P[plr].Manned[hold].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                    sprintf(&Digit[0], "%d", Data->P[plr].Manned[hold].Safety + Data->P[plr].Manned[hold].Damage);
                    draw_string(0, 0, &Digit[0]);
                    draw_string(0, 0, "%");
                    //draw_number(0,0,Data->P[plr].Manned[hold].Safety);
                    //draw_string(144+i*111,116+14*k,"%");
                    ++k;
                };

                break;

            case 3:
                if (hold > -1) {
                    if (hold < 3) {
                        display::graphics.setForegroundColor(7);
                        draw_string(bud, 109 + 14 * k, "PROBE: ");
                        display::graphics.setForegroundColor(1);
                        draw_string(0, 0, &Data->P[plr].Probe[hold].Name[0]);
                        display::graphics.setForegroundColor(11);
                        draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                        Data->P[plr].Probe[hold].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                        sprintf(&Digit[0], "%d", Data->P[plr].Probe[hold].Safety + Data->P[plr].Probe[hold].Damage);
                        draw_string(0, 0, &Digit[0]);
                        draw_string(0, 0, "%");
                        //draw_number(0,0,Data->P[plr].Probe[hold].Safety);
                        //draw_string(144+i*111,116+14*k,"%");
                        ++k;
                    } else if (hold == 4) {
                        display::graphics.setForegroundColor(7);
                        draw_string(bud, 109 + 14 * k, "DOCKING: ");
                        display::graphics.setForegroundColor(1);
                        draw_string(0, 0, &Data->P[plr].Misc[hold].Name[0]);
                        display::graphics.setForegroundColor(11);
                        draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                        Data->P[plr].Misc[hold].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                        sprintf(&Digit[0], "%d", Data->P[plr].Misc[hold].Safety + Data->P[plr].Misc[hold].Damage);
                        draw_string(0, 0, &Digit[0]);
                        draw_string(0, 0, "%");
                        //draw_number(0,0,Data->P[plr].Misc[hold].Safety);
                        //draw_string(144+i*111,116+14*k,"%");
                        ++k;
                    }
                };

                break;

            case 4:
                if (hold > -1) {
                    if (hold < 5) {
                        display::graphics.setForegroundColor(7);
                        draw_string(bud, 109 + 14 * k, "ROCKET: ");
                        display::graphics.setForegroundColor(1);
                        draw_string(0, 0, &Data->P[plr].Rocket[hold - 1].Name[0]);
                        display::graphics.setForegroundColor(11);
                        draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                        Data->P[plr].Rocket[hold - 1].Damage != 0 ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment, Nikakd, 10/8/10
                        sprintf(&Digit[0], "%d", Data->P[plr].Rocket[hold - 1].Safety + Data->P[plr].Rocket[hold - 1].Damage);
                        draw_string(0, 0, &Digit[0]);
                        draw_string(0, 0, "%");
                        //draw_number(0,0,Data->P[plr].Rocket[hold-1].Safety);
                        //draw_string(144+i*111,116+14*k,"%");
                        ++k;
                    } else {
                        display::graphics.setForegroundColor(7);
                        draw_string(bud, 109 + 14 * k, "ROCKET: ");
                        display::graphics.setForegroundColor(1);
                        draw_string(0, 0, &Data->P[plr].Rocket[hold - 5].Name[0]);
                        draw_string(0, 0, " W/B");
                        display::graphics.setForegroundColor(11);
                        draw_string(bud, 116 + 14 * k, "SAFETY FACTOR: ");
                        (Data->P[plr].Rocket[hold - 5].Damage != 0 || Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Damage != 0) ? display::graphics.setForegroundColor(9) : display::graphics.setForegroundColor(1); //Damaged Equipment && Booster's Safety Mod, Nikakd, 10/8/10
                        sprintf(&Digit[0], "%d", RocketBoosterSafety(Data->P[plr].Rocket[hold - 5].Safety + Data->P[plr].Rocket[hold - 5].Damage, Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Safety + Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Damage));
                        draw_string(0, 0, &Digit[0]);
                        draw_string(0, 0, "%");
                        // draw_number(0,0,(Data->P[plr].Rocket[hold-5].Safety+Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Safety)/2);
                        // draw_string(144+i*111,116+14*k,"%");
                        ++k;
                    }
                }

                break;

            default:
                break;
            }
        }
    }

    FadeIn(2, display::graphics.palette(), 10, 0, 0);


    WaitForMouseUp();

    if (HelpFlag) {
        Help("i156");    // Notification of being an Help
    }

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 59 && y >= 70 && x <= 116 && y <= 82 && mousebuttons > 0) || key == K_ENTER || key == 'C') {
            InBox(59, 70, 116, 82);
            WaitForMouseUp();
            OutBox(59, 70, 116, 82);
            FadeOut(2, display::graphics.palette(), 10, 0, 0);
            BIG = 0;
            return;
        } else if ((x >= 133 && y >= 70 && x <= 195 && y <= 82 && mousebuttons > 0) || key == K_ENTER || key == 'P') {
            InBox(133, 70, 195, 82);
            WaitForMouseUp();
            OutBox(133, 70, 195, 82);
            FadeOut(2, display::graphics.palette(), 10, 0, 0);
            BIG = 1;
            return;
        }

        else if ((x >= 207 && y >= 70 && x <= 264 && y <= 82 && mousebuttons > 0) || key == 'S') {
            InBox(207, 70, 264, 82);
            WaitForMouseUp();
            OutBox(207, 70, 264, 82);
            {
                ClrMiss(plr, pad);
            }

            if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
                FadeOut(2, display::graphics.palette(), 10, 0, 0);
                return;
            }
        }
    };
}


void AI_Begin(char plr)
{
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
    display::graphics.screen()->draw(countrySeals, 110 * plr, 0, 107, 93, 30, 85);
    display::graphics.setForegroundColor(11);
    draw_string(60, 58, "COMPUTER TURN:  THINKING...");
    music_start(M_SOVTYP);
    FadeIn(2, display::graphics.palette(), 10, 0, 0);
    colss = 0;
}

void AI_Done(void)
{
    music_stop();
    FadeOut(2, display::graphics.palette(), 10, 0, 0);
    display::graphics.screen()->clear();
}

