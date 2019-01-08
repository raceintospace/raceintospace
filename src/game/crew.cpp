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

#include <string>

#include "display/graphics.h"

#include "crew.h"
#include "Buzz_inc.h"
#include "options.h"  //No Capsule Training, Nikakd, 10/8/10 - Also No requirement to assign Backup crews -Leon
#include "game_main.h"
#include "draw.h"
#include "place.h"
#include "mc.h"
#include "gr.h"
#include "pace.h"

int AsnCrew(char plr, char pad, char part);
void FutFltsTxt(char nw, char col);
void FutSt(char plr, int pr, int p, int b);
void FutSt2(int num, int type);
void FutAstList(char plr, char men, int M1, int M2, int M3, int M4);
void DrawHard(char mode, char pad, char mis, char plr);
int HardRequest(char plr, char mode, char mis, char pad);
int SecondHard(char plr, char mode, char mis, char pad);



int HardCrewAssign(char plr, char Pad, int MisType, char NewType)
{
    int M = 0;

    if (NewType <= 2) {
        Data->P[plr].Future[MisType].Joint = 0;
    } else {
        Data->P[plr].Future[MisType].Joint = 1;
    }

    switch (NewType) {
    case 0:
        return 1;

    case 1:
        M = HardRequest(plr, 0, MisType, Pad);

        if (M == 0) {
            return 0;
        } else {
            return 1;
        }

    case 2:
        M = SecondHard(plr, 0, MisType, Pad);

        if (M != 0) {
            M = AsnCrew(plr, Pad, 0);
        } else {
            return 0;
        }

        return (M != 0);

    case 3:
        M = SecondHard(plr, 1, MisType, Pad);
        Data->P[plr].Future[Pad].part = 0;
        Data->P[plr].Future[Pad].Joint = 1;
        Data->P[plr].Future[Pad].MissionCode = MisType;

        if (M != 0) {
            M = AsnCrew(plr, Pad + 1, 1);
        } else {
            return 0;
        }

        Data->P[plr].Future[Pad + 1].Joint = 1;
        Data->P[plr].Future[Pad + 1].part = 1;
        Data->P[plr].Future[Pad + 1].MissionCode = MisType;

        if (M == 0) {
            return 0;
        } else {
            return 1;
        }

    case 4:
        M = SecondHard(plr, 0, MisType, Pad);
        {
            // scope block to avoid initialization skipped by 'case' label error"
            display::LegacySurface local3(171, 188);
            local3.copyFrom(display::graphics.legacyScreen(), 74, 3, 244, 190);

            if (M != 0) {
                M = AsnCrew(plr, Pad, 0);
            } else {
                return 0;
            }

            Data->P[plr].Future[Pad].part = 0;

            Data->P[plr].Future[Pad].Joint = 1;

            Data->P[plr].Future[Pad].MissionCode = MisType;

            local3.copyTo(display::graphics.legacyScreen(), 74, 3);
        }

        if (M != 0) {
            M = SecondHard(plr, 1, MisType, Pad);
        } else {
            return 0;
        }

        if (M != 0) {
            M = AsnCrew(plr, Pad + 1, 1);
        } else {
            return 0;
        }

        Data->P[plr].Future[Pad + 1].Joint = 1;
        Data->P[plr].Future[Pad + 1].part = 1;
        Data->P[plr].Future[Pad + 1].MissionCode = MisType;

        if (M == 0) {
            return 0;
        } else {
            return 1;
        }

    case 5:
        M = SecondHard(plr, 0, MisType, Pad + 1);
        Data->P[plr].Future[Pad].part = 0;
        Data->P[plr].Future[Pad].MissionCode = MisType;
        Data->P[plr].Future[Pad].Joint = 1;
        Data->P[plr].Future[Pad + 1].Joint = 1;
        Data->P[plr].Future[Pad + 1].part = 1;
        Data->P[plr].Future[Pad + 1].MissionCode = MisType;
        Data->P[plr].Future[Pad + 1].Men = 0;

        if (M == 0) {
            return 0;
        } else {
            return 1;
        }

    default:
        break;
    }

    return M; // all the proper hardware and crews have been assigned
}

void ClrFut(char plr, char pad)
{
    char prime, back, men, i, prg;
    prg = Data->P[plr].Future[pad].Prog;

    if (Data->P[plr].Future[pad].PCrew != 0) {
        prime = Data->P[plr].Future[pad].PCrew - 1;
    } else {
        prime = -1;
    }

    if (Data->P[plr].Future[pad].BCrew != 0) {
        back = Data->P[plr].Future[pad].BCrew - 1;
    } else {
        back = -1;
    }

    men = Data->P[plr].Future[pad].Men;

    if (prime != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 0;
        }

    if (back != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 0;
        }

    if (Data->P[plr].Future[pad].Joint == 1 && Data->P[plr].Future[pad + 1].part == 1) {
        prg = Data->P[plr].Future[pad + 1].Prog;

        if (Data->P[plr].Future[pad + 1].PCrew != 0) {
            prime = Data->P[plr].Future[pad + 1].PCrew - 1;
        } else {
            prime = -1;
        }

        if (Data->P[plr].Future[pad + 1].BCrew != 0) {
            back = Data->P[plr].Future[pad + 1].BCrew - 1;
        } else {
            back = -1;
        }

        men = Data->P[plr].Future[pad + 1].Men;

        if (prime != -1)
            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 0;
            }

        if (back != -1)
            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 0;
            }

        Data->P[plr].Future[pad + 1].part = 0;
        Data->P[plr].Future[pad + 1].Prog = 0;
        Data->P[plr].Future[pad + 1].PCrew = 0;
        Data->P[plr].Future[pad + 1].BCrew = 0;
        Data->P[plr].Future[pad + 1].Duration = 0;
        Data->P[plr].Future[pad + 1].Joint = 0;
        Data->P[plr].Future[pad + 1].Men = 0;
        Data->P[plr].Future[pad + 1].MissionCode = Mission_None;
    }

    Data->P[plr].Future[pad].Prog = 0;
    Data->P[plr].Future[pad].PCrew = 0;
    Data->P[plr].Future[pad].BCrew = 0;
    Data->P[plr].Future[pad].Men = 0;
    Data->P[plr].Future[pad].Duration = 0;
    Data->P[plr].Future[pad].Joint = 0;
    Data->P[plr].Future[pad].MissionCode = Mission_None;
    return;
}

int AsnCrew(char plr, char pad, char part)
{
    int count = 0, i, prg = 0, grp = -1, prime = -1, men = 0, back = -1, t = 0, s = 0, k = 0, yes = 0, stflag = 0, bug;

    std::string oldKeyHelpText = keyHelpText;
    keyHelpText = "k200";
    men = Data->P[plr].Future[pad].Men;
    prg = Data->P[plr].Future[pad].Prog;

    if (Data->P[plr].Future[pad].PCrew == 0) {
        prime = -1;
    } else {
        prime = Data->P[plr].Future[pad].PCrew - 1;
    }

    if (Data->P[plr].Future[pad].BCrew == 0) {
        back = -1;
    } else {
        back = Data->P[plr].Future[pad].BCrew - 1;
    }

    if (prime != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 0;
        }

    if (back != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 0;
        }

    Data->P[plr].Future[pad].PCrew = 0;
    Data->P[plr].Future[pad].BCrew = 0;
    prime = -1;
    back = -1;
    count = 0;

    for (i = 0; i < ASTRONAUT_CREW_MAX; i++) { // Flight Crew Settings
        if (Data->P[plr].Crew[prg][i][0] == 0 || (options.feat_no_cTraining == 0 && Data->P[plr].Pool[Data->P[plr].Crew[prg][i][0] - 1].Moved == 0) //No Capsule Training, Nikakd, 10/8/10
            || Data->P[plr].Pool[Data->P[plr].Crew[prg][i][0] - 1].Prime > 0) {
            stflag = 0;
        } else {
            stflag = 1;
        }

        if (stflag == 0) {
            count++;    // increment the counter
        }
    }

    if ((count >= 7 && options.feat_no_backup == 0) || (count >= 8 && options.feat_no_backup > 0)) {
        keyHelpText = oldKeyHelpText;
        Help("i107");
        return 0;
    } else {
        bug = 0;
    }

    ShBox(74, 3, 244, 190);
    IOBox(80, 102, 157, 116);
    IOBox(161, 102, 238, 116); // primary,secondary
    IOBox(80, 7, 154, 21);
    IOBox(164, 7, 238, 21); // assign,cancel
    InBox(80, 36, 238, 99); // center screen
    fill_rectangle(81, 37, 237, 98, 7 + 3 * plr);
    ShBox(80, 24, 238, 33);

    for (i = 0; i < 4; i++) {
        IOBox(78, 121 + 16 * i, 156, 139 + 16 * i);
        FutFltsTxt(i, 1);
    }

    for (i = 0; i < 4; i++) {
        IOBox(162, 121 + 16 * i, 240, 139 + 16 * i);
        FutFltsTxt(i + 4, 1);
    }

    FutSt(plr, prg, prime, back);

    display::graphics.setForegroundColor(1);

    draw_string(100, 16, "CANCEL");

    display::graphics.setForegroundColor(9);

    draw_string(184, 16, "A");

    display::graphics.setForegroundColor(1);

    draw_string(0, 0, "SSIGN");

    draw_string(86, 111, "MAKE ");

    display::graphics.setForegroundColor(9);

    draw_string(0, 0, "P");

    display::graphics.setForegroundColor(1);

    draw_string(0, 0, "RIMARY");

    draw_string(169, 111, "MAKE ");

    display::graphics.setForegroundColor(9);

    draw_string(0, 0, "B");

    display::graphics.setForegroundColor(1);

    draw_string(0, 0, "ACKUP");

    display::graphics.setForegroundColor(11);

    if (part == 0) {
        draw_string(105, 31, "SELECT PRIMARY CREW");
    } else {
        draw_string(100, 31, "SELECT SECONDARY CREW");
    }

    display::graphics.setForegroundColor(1); // reset the color
    count = 0;

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < ASTRONAUT_CREW_MAX; i++) { // Flight Crew Settings
            if (Data->P[plr].Crew[prg][i][0] == 0 || (options.feat_no_cTraining == 0 && Data->P[plr].Pool[Data->P[plr].Crew[prg][i][0] - 1].Moved == 0) //No Capsule Training, Nikakd, 10/8/10
                || Data->P[plr].Pool[Data->P[plr].Crew[prg][i][0] - 1].Prime > 0) {
                stflag = 0;
            } else {
                stflag = 1;
            }

            if (stflag == 0) {
                ++count;    // increment the counter
            }

            t = (i < 4) ? 0 : 1;
            yes = 0;

            if (t == 1) {
                k = i - 4;
            } else {
                k = i;    // adjust for second side
            }

            if (!t && x >= 80 && y >= 123 + k * 16 && x <= 154 && y <= 137 + k * 16 && grp != i && mousebuttons != 0 && stflag == 1) {
                yes = 1;
            } else if (t && x >= 164 && y >= 123 + k * 16 && x <= 238 && y <= 137 + k * 16 && (grp != i) && mousebuttons != 0 && stflag == 1) {
                yes = 1;
            } else if (grp != i && stflag == 1 && key == '1' + i) {
                yes = 1;
            }

            if (yes == 1 && bug == 0) {

                if (grp != -1) {
                    if (grp < 4) {
                        OutBox(80, 123 + grp * 16, 154, 137 + grp * 16);
                    } else {
                        s = grp - 4;
                        OutBox(164, 123 + s * 16, 238, 137 + s * 16);
                    }
                }

                if (i < 4) {
                    InBox(80, 123 + i * 16, 154, 137 + i * 16);
                } else {
                    InBox(164, 123 + k * 16, 238, 137 + k * 16);
                }

                grp = i;
                FutAstList(plr, men, Data->P[plr].Crew[prg][grp][0],
                           Data->P[plr].Crew[prg][grp][1],
                           Data->P[plr].Crew[prg][grp][2],
                           Data->P[plr].Crew[prg][grp][3]);

                WaitForMouseUp();
            }
        }

        if (((x >= 166 && y >= 9 && x <= 236 && y <= 19 && mousebuttons > 0) || key == K_ENTER || key == 'A') && prime != -1 && (back != -1 || options.feat_no_backup == 1) && bug == 0) {
            // Assign

            InBox(166, 9, 236, 19);

            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 4;
            }

            Data->P[plr].Future[pad].PCrew = prime + 1;
            Data->P[plr].Future[pad].BCrew = back + 1;

            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 2;
            }

            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            keyHelpText = oldKeyHelpText;
            return 1;
        } else if ((x >= 82 && y >= 9 && x <= 152 && y <= 19 && mousebuttons != 0) || key == K_ESCAPE) {
            InBox(82, 9, 152, 19);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            ClrFut(plr, pad);
            keyHelpText = oldKeyHelpText;
            return 0;  // Abort - Redo Mission
        } else if (((x >= 82 && y >= 104 && x <= 155 && y <= 114 && mousebuttons > 0) || key == 'P') && prime != grp && back != grp && grp != -1 && bug == 0) {
            InBox(82, 104, 155, 114);
            prime = grp;
            FutSt(plr, prg, prime, back);
            WaitForMouseUp();

            if (key > 0) {
                delay(110);
            }

            OutBox(82, 104, 155, 114);
        } // End Primary Set
        else if (((x >= 163 && y >= 104 && x <= 236 && y <= 114 && mousebuttons > 0) || key == 'B')  && prime != grp && back != grp && grp != -1 && bug == 0) {
            InBox(163, 104, 236, 114);
            back = grp;
            FutSt(plr, prg, prime, back);
            WaitForMouseUp();
            OutBox(163, 104, 236, 114);

            if (key > 0) {
                delay(110);
            }
        } // End Backup Set
    } // end-while
}

void FutFltsTxt(char nw, char col)
{
    int temp = 0;

    display::graphics.setForegroundColor(col);

    if (nw < 4) {
        draw_string(83, 129 + nw * 16, "FLT. CREW ");
    } else {
        temp = nw - 4;
        draw_string(167, 129 + temp * 16, "FLT. CREW ");
    }

    switch (nw) {
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

    case 5:
        draw_string(0, 0, "VI");
        break;

    case 6:
        draw_string(0, 0, "VII");
        break;

    case 7:
        draw_string(0, 0, "VIII");
        break;

    default:
        break;
    }

    return;
}

void FutSt(char plr, int pr, int p, int b)
{
    int i;

    for (i = 0; i < ASTRONAUT_CREW_MAX; i++) {
        if (i == p) {
            FutSt2(i, 3);
        }

        if (i == b) {
            FutSt2(i, 4);
        }

        if (i != p && i != b) {
            if (Data->P[plr].Crew[pr][i][0] == 0) {
                FutSt2(i, 2);
            } else if (Data->P[plr].Pool[Data->P[plr].Crew[pr][i][0] - 1].Prime > 0) {
                FutSt2(i, 5);
            } else if (Data->P[plr].Pool[Data->P[plr].Crew[pr][i][0] - 1].Moved == 0) {
                FutSt2(i, 1);
            } else {
                FutSt2(i, 0);
            }
        }
    }

    return;
}

void FutSt2(int num, int type)
{
    int temp = 0;

    if (num < 4) {
        fill_rectangle(84, 130 + num * 16, 151, 136 + num * 16, 3);
        grMoveTo(84, 135 + num * 16);
    } else {
        temp = num - 4;
        fill_rectangle(168, 130 + temp * 16, 235, 136 + temp * 16, 3);
        grMoveTo(168, 135 + temp * 16);
    }

    switch (type) {
    case 0:
        display::graphics.setForegroundColor(6);
        draw_string(0, 0, "UNASSIGNED");
        break;

    case 1:
        display::graphics.setForegroundColor(8);

        if (options.feat_no_cTraining > 0) {
            display::graphics.setForegroundColor(12);
        }

        draw_string(0, 0, "TRAINING");
        break;

    case 2:
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "VACANT");
        break;

    case 3:
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, "PRIMARY");
        break;

    case 4:
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, "BACKUP");
        break;

    case 5:
        display::graphics.setForegroundColor(10);
        draw_string(0, 0, "ASSIGNED");
        break;
    }

    return;
}

void FutAstList(char plr, char men, int M1, int M2, int M3, int M4)
{
    int i = 0, m[4];

    m[0] = M1;
    m[1] = M2;
    m[2] = M3;
    m[3] = M4;
    fill_rectangle(82, 38, 236, 97, 7 + plr * 3); // center screen
    display::graphics.setForegroundColor(1);

    for (i = 0; i < men; i++) {
        if (m[i] > 0) {
            if (Data->P[plr].Pool[m[i] - 1].Sex == 1) {
                display::graphics.setForegroundColor(5);    // Print name in blue if 'naut is female
            }

            if (Data->P[plr].Pool[m[i] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(3);    // Print name in gray if 'naut has announced retirement (black doesn't show well here) -Leon
            }

            draw_string(100, 44 + i * 14, &Data->P[plr].Pool[m[i] - 1].Name[0]);
            display::graphics.setForegroundColor(3);

            if (Data->P[plr].Pool[m[i] - 1].Missions > 0) {
                draw_string(0, 0, " (");
                draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].Missions);
                draw_string(0, 0, ")");
            }

            display::graphics.setForegroundColor(1);
            fill_rectangle(87, 39 + i * 14, 94, 39 + i * 14, 2); // Top
            fill_rectangle(87, 39 + i * 14, 87, 44 + i * 14, 2); // Left
            fill_rectangle(87, 45 + i * 14, 94, 45 + i * 14, 3); // Bottom
            fill_rectangle(95, 39 + i * 14, 95, 45 + i * 14, 3); // Right

            if (Data->P[plr].Pool[m[i] - 1].Mood >= 65) {
                fill_rectangle(88, 40 + i * 14, 94, 44 + i * 14, 16);
            }

            if (Data->P[plr].Pool[m[i] - 1].Mood < 65 && Data->P[plr].Pool[m[i] - 1].Mood >= 40) {
                fill_rectangle(88, 40 + i * 14, 94, 44 + i * 14, 11);
            }

            if (Data->P[plr].Pool[m[i] - 1].Mood < 40 && Data->P[plr].Pool[m[i] - 1].Mood >= 20) {
                fill_rectangle(88, 40 + i * 14, 94, 44 + i * 14, 8);
            }

            if (Data->P[plr].Pool[m[i] - 1].Mood < 20) {
                fill_rectangle(88, 40 + i * 14, 94, 44 + i * 14, 0);
            }

            if (Data->P[plr].Pool[m[i] - 1].Mood == 0) {
                fill_rectangle(88, 40 + i * 14, 94, 44 + i * 14, 3);
            }

            //87 - 169
            if (i == 0) {
                display::graphics.setForegroundColor(11);   /* Highlight CA for command pilot */
            }

            draw_string(87, 51 + i * 14, "CP:");
            draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].Cap);
            display::graphics.setForegroundColor(1);

            if (i == 1 && men > 1) {
                display::graphics.setForegroundColor(11);   /* Highlight LM for LM pilot */
            }

            draw_string(0, 0, "  LM:");
            draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].LM);
            display::graphics.setForegroundColor(1);

            if (men == 1 || ((men == 2 || men == 3) && i == 1) || (men == 4 && i > 1)) {
                display::graphics.setForegroundColor(11);   /* Highlight EV for EVA specialist */
            }

            draw_string(0, 0, "  EV:");
            draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].EVA);
            display::graphics.setForegroundColor(1);

            if ((men == 2 && i == 0) || (men == 3 && i == 2)) {
                display::graphics.setForegroundColor(11);   /* Highlight DO for docking specialist */
            }

            draw_string(0, 0, "  DO:");
            draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].Docking);
            display::graphics.setForegroundColor(1);  /* Never highlight EN skill */
            draw_string(0, 0, "  EN:");
            draw_number(0, 0, Data->P[plr].Pool[m[i] - 1].Endurance);
        }
    }

    return;
}


void DrawHard(char mode, char pad, char mis, char plr)
{
    int lenprogname;  // Variable to hold and manipulate length of program name

    ShBox(75, 43, 244, 173);
    InBox(81, 60, 238, 95);
    IOBox(81, 154, 238, 167); // continue
    InBox(81, 47, 238, 56);
    fill_rectangle(82, 61, 237, 94, 6 + 3 * plr);
    display::graphics.setForegroundColor(11);

    if (mode == 0) {
        draw_string(90, 54, "SELECT PRIMARY VEHICLE");
    } else {
        draw_string(90, 54, "SELECT SECONDARY VEHICLE");
    }

    display::graphics.setForegroundColor(1);
    GetMisType(mis);
    draw_string(85, 70, Mis.Abbr);
//Missions(plr,85,70,mis,0);
    int MisCod;
    MisCod = Data->P[plr].Future[pad].MissionCode;

    if ((MisCod > 24 && MisCod < 32) || MisCod == 33 || MisCod == 34 || MisCod == 35 || MisCod == 37 || MisCod == 40 || MisCod == 41)
        // Show duration level only on missions with a Duration step - Leon
    {
        switch (Data->P[plr].Future[pad].Duration) {
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

    draw_string(85, 85, "PAD: "); // Used to be followed by: "draw_number(0,0,pad+1);"--now shows PAD: A/B/C instead of 1/2/3 -Leon

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

    lenprogname = (3 - strlen(Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Name)) * 3;
    draw_string(111 + lenprogname, 109, Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Name); //draw_number(0,0,94+18+(3-strlen(Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Name))*3);
    lenprogname = (3 - strlen(Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Name)) * 3;
    draw_string(190 + lenprogname, 109, Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Name); //draw_number(0,0,174+18+(3-strlen(Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Name))*3);
    lenprogname = (3 - strlen(Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Name)) * 3;
    draw_string(111 + lenprogname, 126, Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Name); //draw_number(0,0,94+18+(3-strlen(Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Name))*3);
    lenprogname = (3 - strlen(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name)) * 3;
    draw_string(190 + lenprogname, 126, Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name); //draw_number(0,0,174+18+(3-strlen(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name))*3);
    lenprogname = (3 - strlen(Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Name)) * 3;
    draw_string(149 + 4 + lenprogname, 143, Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Name);
    draw_string(143, 163, "CANCEL");
    return;
}

int HardRequest(char plr, char mode, char mis, char pad)
{
    int i = 0, pr[5], t = 0;

    std::string oldKeyHelpText = keyHelpText;
    keyHelpText = "k201";

    for (i = 0; i < 5; i++) {
        if (Data->P[plr].Manned[i].Num >= 0) {
            pr[i] = 1;
            t++;
        } else {
            pr[i] = 0;
        }
    }

    // special case: 1-man capsule can't go to the Moon
    GetMisType(mis);

    // exceptions
    if (Mis.Lun == 1 || Mis.Doc == 1 || Mis.mEq > 1 || Mis.Days > 1 ||
        Data->P[plr].Future[pad].Duration > 2) {
        pr[0] = 0;
    }

    if (Data->P[plr].Future[pad].Duration > 5 || Mis.Days > 5) {
        pr[1] = 0;
    }

    if (Data->P[plr].Future[pad].Duration > 3) {
        pr[3] = 0;
    }

    if (mis == 54) {
        pr[0] = pr[1] = pr[2] = pr[3] = 0;
    }

    if (mis == 57 && plr == 1) {
        pr[0] = pr[1] = pr[3] = pr[4] = 0;
    }

    if (Mis.Doc == 1) {
        pr[4] = 0;
    }

//  if (t==0) {Help("i126"); return 0;};


    DrawHard(mode, pad, mis, plr);

    if (pr[0] == 0) {
        InBox(81, 100, 158, 114);
    } else {
        IOBox(81, 100, 158, 114);
    }

    if (pr[1] == 0) {
        InBox(161, 100, 238, 114);
    } else {
        IOBox(161, 100, 238, 114);
    }

    if (pr[2] == 0) {
        InBox(81, 117, 158, 131);
    } else {
        IOBox(81, 117, 158, 131);
    }

    if (pr[3] == 0) {
        InBox(161, 117, 238, 131);
    } else {
        IOBox(161, 117, 238, 131);
    }

    if (pr[4] == 0) {
        InBox(121, 134, 198, 148);
    } else {
        IOBox(121, 134, 198, 148);
    }

    i = 0;
    WaitForMouseUp();

    while (i == 0) {
        key = 0;
        GetMouse();

        if (mousebuttons != 0 || key > 0) {
            if (((x >= 83 && y >= 102 && x <= 156 && y <= 112) || key == '1') && pr[0]) {
                InBox(83, 102, 156, 112);
                i = 1;
                WaitForMouseUp();
            } // ONE
            else if (((x >= 163 && y >= 102 && x <= 236 && y <= 112) || key == '2') && pr[1]) {
                InBox(163, 102, 236, 112);
                i = 2;
                WaitForMouseUp();
            } // TWO
            else if (((x >= 83 && y >= 119 && x <= 156 && y <= 129) || key == '3') && pr[2]) {
                InBox(83, 119, 156, 129);
                i = 3;
                WaitForMouseUp();
            } // THREE
            else if (((x >= 163 && y >= 119 && x <= 236 && y <= 129) || key == '4') && pr[3]) {
                InBox(163, 119, 236, 129);
                i = 4;
                WaitForMouseUp();
            } // FOUR
            else if (((x >= 123 && y >= 136 && x <= 196 && y <= 146) || key == '5') && pr[4]) {
                InBox(123, 136, 196, 146);
                i = 5;
                WaitForMouseUp();
            } // FIVE
            else if ((x >= 83 && y >= 156 && x <= 236 && y <= 165 && mousebuttons != 0) || key == K_ENTER || key == K_ESCAPE) {
                InBox(83, 156, 236, 165);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                keyHelpText = oldKeyHelpText;
                return 0;  // Abort - Redo Mission
            }
        }
    } /* End while */

    Data->P[plr].Future[pad].Prog = i;

    keyHelpText = oldKeyHelpText;

    return i;
}

int SecondHard(char plr, char mode, char mis, char pad)
{

    int i = 0, men = 0, prg = 0, prog[5], t = 0;

    std::string oldKeyHelpText = keyHelpText;
    keyHelpText = "k201";

    for (i = 0; i < 5; i++) if (Data->P[plr].Manned[i].Num >= 0) {
            men++;
        }

    if (men == 0) {
        Help("i126");
        keyHelpText = oldKeyHelpText;
        return 0;
    }

    if (mode == 1) {
        pad++;    // if second part of Joint mission
    }

    for (i = 0; i < 5; i++) {
        if (Data->P[plr].Manned[i].Num >= 0) {
            prog[i] = 1;
            t++;
        } else {
            prog[i] = 0;
        }
    }

    GetMisType(mis);

    // exceptions
    if (Mis.Lun == 1 || Mis.Doc == 1 || Mis.mEq > 1 || Mis.Days > 2 ||
        Data->P[plr].Future[pad].Duration > 2) {
        prog[0] = 0;
    }

    if (Data->P[plr].Future[pad].Duration > 5 || Mis.Days > 5) {
        prog[1] = 0;
    }

    if (Data->P[plr].Future[pad].Duration > 4) {
        prog[3] = 0;
    }

    if (mis == 54) {
        prog[0] = prog[1] = prog[2] = prog[3] = 0;
    }

    if (Mis.Doc == 1) {
        prog[4] = 0;
    }

    if (mis == 57 && plr == 1) {
        prog[0] = prog[1] = prog[3] = prog[4] = 0;
    }

    DrawHard(mode, pad, mis, plr);

    if (prog[0] == 0) {
        InBox(81, 100, 158, 114);
    } else {
        IOBox(81, 100, 158, 114);
    }

    if (prog[1] == 0) {
        InBox(161, 100, 238, 114);
    } else {
        IOBox(161, 100, 238, 114);
    }

    if (prog[2] == 0) {
        InBox(81, 117, 158, 131);
    } else {
        IOBox(81, 117, 158, 131);
    }

    if (prog[3] == 0) {
        InBox(161, 117, 238, 131);
    } else {
        IOBox(161, 117, 238, 131);
    }

    if (prog[4] == 0) {
        InBox(121, 134, 198, 148);
    } else {
        IOBox(121, 134, 198, 148);
    }

    i = 0;
    WaitForMouseUp();

    while (i == 0) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {
            if (((x >= 83 && y >= 102 && x <= 156 && y <= 112 && mousebuttons != 0) || key == '1') && prog[0] != 0 && prg != 1) {
                men = 1;
                InBox(83, 102, 156, 112);
                i = 1;
                WaitForMouseUp();
            } // One Man Program
            else if (((x >= 163 && y >= 102 && x <= 236 && y <= 112 && mousebuttons != 0) || key == '2') && prog[1] != 0 && prg != 3) {
                men = 2;
                InBox(163, 102, 236, 112);
                i = 2;
                WaitForMouseUp();
            } // Two Man Program
            else if (((x >= 83 && y >= 119 && x <= 156 && y <= 129 && mousebuttons != 0) || key == '3') && prog[2] != 0 && prg != 2) {
                men = 3;
                InBox(83, 119, 156, 129);
                i = 3;
                WaitForMouseUp();
            } // Three Man Program
            else if (((x >= 163 && y >= 119 && x <= 236 && y <= 129 && mousebuttons != 0) || key == '4') && prog[3] != 0 && prg != 4) {
                men = 3;
                InBox(163, 119, 236, 129);
                i = 4;
                WaitForMouseUp();
            } // Minishuttle Man Program
            else if (((x >= 123 && y >= 136 && x <= 196 && y <= 146 && mousebuttons != 0) || key == '5') && prog[4] != 0 && prg != 5) {
                men = 4;
                InBox(123, 136, 196, 146);
                i = 5;
                WaitForMouseUp();
            } // Four Man Program
            else if ((x >= 83 && y >= 156 && x <= 236 && y <= 165 && mousebuttons != 0) || key == K_ENTER) {
                InBox(83, 156, 236, 165);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                keyHelpText = oldKeyHelpText;
                return 0;  // Abort - Redo Mission
            }
        }
    }

    Data->P[plr].Future[pad].Prog = i;
    Data->P[plr].Future[pad].Men = men;
    keyHelpText = oldKeyHelpText;
    return i;
}


// EOF
