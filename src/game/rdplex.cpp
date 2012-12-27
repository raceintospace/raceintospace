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
/** \file rdplex.c Research and Development Complex
 */

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "rdplex.h"
#include "options.h"
#include "ast4.h"
#include "game_main.h"
#include "place.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include <assert.h>

int call;
int wh;
display::Surface * but;
display::Surface * mans;
int avoidf;


void SRPrintAt(int x, int y, char *text, char fgd, char bck);
void DrawRD(char plr);
void BButs(char old, char nw);
void RDButTxt(int v1, int val, char plr, char SpDModule); //DM Screen, Nikakd, 10/8/10
void ManSel(int activeButtonIndex);
void ShowUnit(char hw, char un, char plr);
void OnHand(char qty);
void DrawHPurc(char plr);
void BuyUnit(char hw2, char un2, char plr);


/* We need to mask number of rolls into R&D history for current turn
 * Max number rolled on 5 dice with mods is 5*7 = 35
 * To keep binary compatibility we use 8 bits for value
 * 35 + 5*40 == 235 < 255, so fits within single byte and is unambiguous.
 */
enum {
    NUM_ROLLS_MULT = 40,
};

/**
 * Extract number of rolls from encoded value.
 * @param encoded
 * @return number of rolls
 */
static inline
uint8_t
decodeNumRolls(uint8_t encoded)
{
    return encoded / NUM_ROLLS_MULT;
}

/**
 * Extract sum of dice rolls from encoded value.
 * @param encoded
 * @return sum of dice rolls
 */
static inline
uint8_t
decodeRollValue(uint8_t encoded)
{
    return encoded % NUM_ROLLS_MULT;
}

/**
 * Encode number of dice and total rolled value.
 * @param nRolls nubmer of dice
 * @param value sum of rolled dice
 * @return encoded value
 */
static inline
uint8_t
encodeRolls(uint8_t nRolls, uint8_t value)
{
    return nRolls * NUM_ROLLS_MULT + value;
}

void SRPrintAt(int x, int y, char *text, char fgd, char bck)
{
    display::graphics.setForegroundColor(bck);
    PrintAt(x + 1, y + 1, text);
    display::graphics.setForegroundColor(fgd);
    PrintAt(x, y, text);
    return;
}


void Load_RD_BUT(char player_index)
{
    FILE *fin;
    struct {
        char na[4];
        uint32_t size;
    } Boo;

    if (BUTLOAD == 1) {
        return;
    }

	but = new display::Surface(282, 61);
	mans = new display::Surface(119, 17);

    fin = sOpen("RDBOX.BUT", "rb", 0);
    fread(&Boo, sizeof Boo, 1, fin);
    Swap32bit(Boo.size);

    if (player_index == 1)
        while (strncmp("SBUT", &Boo.na[0], 4) != 0) {
            fseek(fin, Boo.size, SEEK_CUR);
            fread(&Boo, sizeof Boo, 1, fin);
            Swap32bit(Boo.size);
        }

    fread(buffer, Boo.size, 1, fin);
    RLED_img(buffer, but->pixels(), Boo.size, but->width(), but->height());
    fread(&Boo, sizeof Boo, 1, fin);
    Swap32bit(Boo.size);
    fread(buffer, Boo.size, 1, fin);
    fclose(fin);
    RLED_img(buffer, mans->pixels(), Boo.size, mans->width(), mans->height());
    BUTLOAD = 1;
    return;
}

void Del_RD_BUT(void)
{
    BUTLOAD = 0;
	delete but;
	but = NULL;
	delete mans;
	mans = NULL;
}

void DrawRD(char player_index)
{
    int i;
    FILE *fin;
    helpText = "i009";
    keyHelpText = "k009";

    FadeOut(2, display::graphics.palette(), 10, 0, 0);
    fin = sOpen("VAB.IMG", "rb", 0);
    fread(display::graphics.palette(), 768, 1, fin);
    fclose(fin);

    display::graphics.screen()->clear(0);
    Load_RD_BUT(player_index);
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 65);

    ShBox(0, 68, 158, 199);
    IOBox(3, 71, 154, 85);
    IOBox(3, 182, 76, 196);
    IOBox(81, 182, 154, 196);

    ShBox(161, 68, 319, 199);
    IOBox(163, 155, 187, 177);
    IOBox(189, 155, 213, 177);
    IOBox(215, 155, 239, 177);
    IOBox(241, 155, 265, 177);
    IOBox(267, 155, 291, 177);
    IOBox(293, 155, 317, 177);

    IOBox(5, 27, 77, 62);
    IOBox(81, 27, 158, 62);
    IOBox(162, 27, 239, 62);
    IOBox(243, 27, 315, 62);

    IOBox(243, 3, 316, 19);
    InBox(3, 3, 30, 19);
    InBox(165, 157, 185, 175);
    IOBox(163, 182, 317, 196);

    OutBox(21, 90, 136, 176);
    InBox(26, 94, 131, 172);

    LTArrow(24, 186);
    RTArrow(101, 186);

    //GradRect(27,95,130,171,player_index*16+128);
    RectFill(27, 95, 130, 171, 0);

    display::graphics.setForegroundColor(9);
    PrintAt(13, 80, "V");
    display::graphics.setForegroundColor(1);
    PrintAt(0, 0, "ISIT PURCHASING FACILITY");

    for (i = 0; i < 6; i++) {
		mans->copyTo(display::graphics.screen(), i * 20, 0, 166 + i * 26, 158, 184 + i * 26, 174);
    }

    display::graphics.setForegroundColor(3);
    grMoveTo(296, 174);
    grLineTo(314, 174);

	
    but->copyTo(display::graphics.screen(), 0, 0, 8, 30, 74, 59); // Unmanned
    but->copyTo(display::graphics.screen(), 68, 0, 84, 30, 155, 59); // Rocket
    but->copyTo(display::graphics.screen(), 141, 0, 165, 30, 236, 59); // Manned
    but->copyTo(display::graphics.screen(), 214, 0, 246, 30, 312, 59); // Misc

    display::graphics.setForegroundColor(1);
    DispBig(50, 5, "RESEARCH", 0, -1);
    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        PrintAt(157, 8, "SPRING");
    } else {
        PrintAt(162, 8, "FALL");
    }

    PrintAt(163, 15, "19");
    DispNum(0, 0, Data->Year);

    PrintAt(200, 8, "CASH:");
    DispMB(201, 15, Data->P[player_index].Cash);

    display::graphics.setForegroundColor(1);
    PrintAt(258, 13, "CONTINUE");
    FlagSm(player_index, 4, 4);
    QueryUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);
    ShowUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);

    return;
} // End of DrawRD


void BButs(char old, char nw)
{
    switch (old) {
    case PROBE_HARDWARE:
        OutBox(7, 29, 75, 60);
        but->copyTo(display::graphics.screen(), 0, 0, 8, 30, 74, 59); // Unmanned
        break;

    case ROCKET_HARDWARE:
        OutBox(83, 29, 156, 60);
        but->copyTo(display::graphics.screen(), 68, 0, 84, 30, 155, 59); // Rocket
        break;

    case MANNED_HARDWARE:
        OutBox(164, 29, 237, 60);
        but->copyTo(display::graphics.screen(), 141, 0, 165, 30, 236, 59); // Manned
        break;

    case MISC_HARDWARE:
        OutBox(245, 29, 313, 60);
        but->copyTo(display::graphics.screen(), 214, 0, 246, 30, 312, 59); // Misc
        break;
    }

    switch (nw) {
    case PROBE_HARDWARE:
        InBox(7, 29, 75, 60);
        but->copyTo(display::graphics.screen(), 0, 31, 8, 30, 74, 59); // Unmanned
        break;

    case ROCKET_HARDWARE:
        InBox(83, 29, 156, 60);
        but->copyTo(display::graphics.screen(), 68, 31, 84, 30, 155, 59); // Rocket
        break;

    case MANNED_HARDWARE:
        InBox(164, 29, 237, 60);
        but->copyTo(display::graphics.screen(), 141, 31, 165, 30, 236, 59); // Manned
        break;

    case MISC_HARDWARE:
        InBox(245, 29, 313, 60);
        but->copyTo(display::graphics.screen(), 214, 31, 246, 30, 312, 59); // Misc
        break;
    }


    return;
}

void
RDButTxt(int cost, int encodedRolls, char playerIndex, char SpDModule) //DM Screen, Nikakd, 10/8/10
{
    RectFill(166, 185, 314, 193, 3);
    display::graphics.setForegroundColor(1);

    int diceRoll = decodeRollValue(encodedRolls);

    //DM Screen, Nikakd, 10/8/10
    if (SpDModule == 1) {
        PrintAt(184, 191, "CANNOT BE RESEARCHED");
        return;
    }

    if (diceRoll == 0) {
        PrintAt(169, 191, "RE");
        display::graphics.setForegroundColor(9);
        PrintAt(0, 0, "S");
        display::graphics.setForegroundColor(1);
        PrintAt(0, 0, "EARCH PROGRAM FOR ");
        display::graphics.setForegroundColor(9);
        DispNum(0, 0, cost);
        display::graphics.setForegroundColor(1);
        PrintAt(0, 0, " MB");
    } else {
        display::graphics.setForegroundColor(11);
        PrintAt(192, 191, "R&D ");
        DispNum(0, 0, diceRoll);
        PrintAt(0, 0, "% IMPROVEMENT");

        if (Data->P[playerIndex].RD_Mods_For_Turn > 0) {
            PrintAt(0, 0, "+");
        }
    }
}


char RD(char player_index)
{
    short hardware = HARD1, unit = UNIT1;
    short roll = 0,  buy[4][7], i, j, b;

    b = 0; /* XXX check uninitialized */

    for (i = 0; i < 4; i++) for (j = 0; j < 7; j++) {
            buy[i][j] = Data->P[player_index].Buy[i][j];
        }

    helpText = "i009";
    keyHelpText = "k009";

    DrawRD(player_index);
    BButs(PROBE_HARDWARE, hardware);
    ShowUnit(hardware, unit, player_index);
    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

    if (buy[hardware][unit] == 0) {
        QueryUnit(hardware, unit, player_index);
    } else {
        InBox(165, 184, 315, 194);
    };

    ManSel(decodeNumRolls(buy[hardware][unit]));

    helpText = "i009";

    keyHelpText = "k009";

    FadeIn(2, display::graphics.palette(), 10, 0, 0);

    music_start(M_HARDWARE);

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {
            if ((x >= 283 && y >= 90 && x <= 302 && y <= 100) || key == 'F') {
                char EqDmg = 0;

                switch (hardware) {
                case PROBE_HARDWARE:
                    EqDmg = Data->P[player_index].Probe[unit].Damage != 0 ? 1 : 0;
                    break;

                case ROCKET_HARDWARE:
                    EqDmg = Data->P[player_index].Rocket[unit].Damage != 0 ? 1 : 0;
                    break;

                case MANNED_HARDWARE:
                    EqDmg = Data->P[player_index].Manned[unit].Damage != 0 ? 1 : 0;
                    break;

                case MISC_HARDWARE:
                    EqDmg = Data->P[player_index].Misc[unit].Damage != 0 ? 1 : 0;
                    break;

                default:
                    break;
                }

                if (EqDmg) {
                    InBox(283, 90, 302, 100);
                    DamProb(player_index, hardware, unit);
                    DrawRD(player_index);
                    BButs(PROBE_HARDWARE, hardware);
                    ShowUnit(hardware, unit, player_index);
                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    helpText = "i009";

                    keyHelpText = "k009";

                    FadeIn(2, display::graphics.palette(), 10, 0, 0);
                }
            } else if ((y >= 29 && y <= 60 && mousebuttons > 0) || (key == 'U' || key == 'R' || key == 'M' || key == 'C')) {
                if (((x >= 7 && x <= 75 && mousebuttons > 0) || key == 'U') && hardware != PROBE_HARDWARE) { /* Unmanned */
                    roll = 0;
                    BButs(hardware, PROBE_HARDWARE);
                    hardware = PROBE_HARDWARE;
                    unit = PROBE_HW_ORBITAL;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Probe[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 83 && x <= 156 && mousebuttons > 0) || key == 'R') && hardware != ROCKET_HARDWARE) { /* Rockets */
                    roll = 0;
                    BButs(hardware, ROCKET_HARDWARE);
                    hardware = ROCKET_HARDWARE;
                    unit = ROCKET_HW_ONE_STAGE;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Rocket[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 164 && x <= 237 && mousebuttons > 0) || key == 'C') && hardware != MANNED_HARDWARE) { /* Manned */
                    roll = 0;
                    BButs(hardware, MANNED_HARDWARE);
                    hardware = MANNED_HARDWARE;
                    unit = MANNED_HW_ONE_MAN_CAPSULE;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Manned[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 245 && x <= 313 && mousebuttons > 0) || key == 'M') && hardware != MISC_HARDWARE) { /* Misc */
                    roll = 0;
                    BButs(hardware, MISC_HARDWARE);
                    hardware = MISC_HARDWARE;
                    unit = MISC_HW_KICKER_A;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Misc[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                }
            } else if (((y >= 157 && y <= 175 && mousebuttons > 0) || (key >= '0' && key <= '5')) && buy[hardware][unit] == 0) {
                /*  R&D Amount */
                if (((x >= 165 && x <= 185 && mousebuttons > 0) || key == '0') && roll != 0) {
                    roll = 0;
                }

                if (((x >= 191 && x <= 211 && mousebuttons > 0) || key == '1') && roll != 1) {
                    roll = 1;
                }

                if (((x >= 217 && x <= 238 && mousebuttons > 0) || key == '2') && roll != 2) {
                    roll = 2;
                }

                if (((x >= 243 && x <= 263 && mousebuttons > 0) || key == '3') && roll != 3) {
                    roll = 3;
                }

                if (((x >= 269 && x <= 289 && mousebuttons > 0) || key == '4') && roll != 4) {
                    roll = 4;
                }

                if (((x >= 295 && x <= 315 && mousebuttons > 0) || key == '5') && roll != 5) {
                    roll = 5;
                }

                switch (hardware) {
                case PROBE_HARDWARE:
                    b = Data->P[player_index].Probe[unit].RDCost;
                    break;

                case ROCKET_HARDWARE:
                    b = Data->P[player_index].Rocket[unit].RDCost;
                    break;

                case MANNED_HARDWARE:
                    b = Data->P[player_index].Manned[unit].RDCost;
                    break;

                case MISC_HARDWARE:
                    b = Data->P[player_index].Misc[unit].RDCost;
                    break;
                }

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                ManSel(roll);
                WaitForMouseUp();
            } else if ((x >= 5 && y >= 184 && x <= 74 && y <= 194 && mousebuttons > 0) || key == LT_ARROW) { /* LEFT ARROW */
                roll = 0;
                InBox(5, 184, 74, 194);
                WaitForMouseUp();

                unit--;

                switch (hardware) {
                case PROBE_HARDWARE:
                    if (unit < PROBE_HW_ORBITAL) {
                        unit = PROBE_HW_LUNAR;
                    }

                    break;

                case ROCKET_HARDWARE:
                    if (unit < ROCKET_HW_ONE_STAGE) {
                        unit = ROCKET_HW_BOOSTERS;
                    }

                    break;

                case MANNED_HARDWARE:
                    if (unit < MANNED_HW_ONE_MAN_CAPSULE) {
                        unit = MANNED_HW_ONE_MAN_MODULE;
                    }

                    break;

                case MISC_HARDWARE:
                    if (unit < MISC_HW_KICKER_A) {
                        unit = MISC_HW_DOCKING_MODULE;
                    }

                    if (player_index == 0 && unit == MISC_HW_KICKER_C) {
                        unit--;
                    }

                    break;
                }

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

                ManSel(decodeNumRolls(buy[hardware][unit]));

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                };

                ShowUnit(hardware, unit, player_index);

                OutBox(5, 184, 74, 194);
            } else if ((x >= 83 && y >= 184 && x <= 152 && y <= 194 && mousebuttons > 0) || key == RT_ARROW) { /* RIGHT ARROW */
                roll = 0;
                InBox(83, 184, 152, 194);
                WaitForMouseUp();
                unit++;

                switch (hardware) {
                case PROBE_HARDWARE:
                    if (unit > PROBE_HW_LUNAR) {
                        unit = PROBE_HW_ORBITAL;
                    }

                    break;

                case ROCKET_HARDWARE:
                    if (unit > ROCKET_HW_BOOSTERS) {
                        unit = ROCKET_HW_ONE_STAGE;
                    }

                    break;

                case MANNED_HARDWARE:
                    if (unit > MANNED_HW_ONE_MAN_MODULE) {
                        unit = MANNED_HW_ONE_MAN_CAPSULE;
                    }

                    break;

                case MISC_HARDWARE:
                    if (unit > MISC_HW_DOCKING_MODULE) {
                        unit = MISC_HW_KICKER_A;
                    }

                    if (player_index == 0 && unit == MISC_HW_KICKER_C) {
                        unit++;
                    }

                    break;   //DM Screen, Nikakd, 10/8/10
                }

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

                ManSel(decodeNumRolls(buy[hardware][unit]));

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                };

                ShowUnit(hardware, unit, player_index);

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

                OutBox(83, 184, 152, 194);
            } else if (((x >= 165 && y >= 184 && x <= 315 && y <= 194 && mousebuttons > 0) || key == 'S') && buy[hardware][unit] == 0
                       && roll != 0) {
                // b is the cost per roll

                switch (hardware) {
                case PROBE_HARDWARE:
                    b = Data->P[player_index].Probe[unit].RDCost;
                    break;

                case ROCKET_HARDWARE:
                    b = Data->P[player_index].Rocket[unit].RDCost;
                    break;

                case MANNED_HARDWARE:
                    b = Data->P[player_index].Manned[unit].RDCost;
                    break;

                case MISC_HARDWARE:
                    b = Data->P[player_index].Misc[unit].RDCost;
                    break;
                }

                // Add to the expenditure data

                if ((b * roll <= Data->P[player_index].Cash) && QueryUnit(hardware, unit, player_index)
                    && MaxChk(hardware, unit, player_index)) {
                    buy[hardware][unit] = RDUnit(hardware, unit, roll, player_index);

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    };

                    Data->P[player_index].Cash -= b * roll;

                    // add the amount to the expenditure budget
                    Data->P[player_index].Spend[0][hardware] += b * roll;


                    ShowUnit(hardware, unit, player_index);

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10
                } else {
                    QueryUnit(hardware, unit, player_index);
                }
            } else if (((y >= 3 && y <= 19) && (x >= 243 && x <= 316 && mousebuttons > 0)) || key == K_ENTER) {
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                for (i = 0; i < 4; i++) for (j = 0; j < 7; j++) {
                        Data->P[player_index].Buy[i][j] = buy[i][j];
                    }

                music_stop();
                Del_RD_BUT();
                call = 0;
                HARD1 = PROBE_HARDWARE;
                UNIT1 = PROBE_HW_ORBITAL;
                return 0;
            } else if ((x >= 5 && y >= 73 && x <= 152 && y <= 83 && mousebuttons > 0) || key == 'V') {
                InBox(5, 73, 152, 83);
                HARD1 = hardware;
                UNIT1 = unit;

                for (i = 0; i < 4; i++) for (j = 0; j < 7; j++) {
                        Data->P[player_index].Buy[i][j] = buy[i][j];
                    }

                music_stop();

                //DM Screen, Nikakd, 10/8/10 (Removed line)
                if (call == 1) {
                    return 1;    // go back through gateway
                }

                call = 1;
                wh = HPurc(player_index);

                if (call == 0) {
                    return 0;
                }

                hardware = HARD1;
                unit = UNIT1;
                call = 0;

                for (i = 0; i < 4; i++) for (j = 0; j < 7; j++) {
                        buy[i][j] = Data->P[player_index].Buy[i][j];
                    }

                DrawRD(player_index);
                //DM Screen, Nikakd, 10/8/10 (Removed line)
                BButs(PROBE_HARDWARE, hardware);
                ShowUnit(hardware, unit, player_index);
                RDButTxt(0, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0)); //DM Screen, Nikakd, 10/8/10

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                };

                FadeIn(2, display::graphics.palette(), 10, 0, 0);

                music_start(M_HARDWARE);

                WaitForMouseUp();
            };
        };

        gr_maybe_sync();
    };
}

/**
 * Draw proper outlines on active/inactive research team buttons.
 *
 * @param activeButtonIndex
 */
void ManSel(int activeButtonIndex)
{
    int dx = 26;
    int i;

    for (i = 0; i < 6; ++i) {
        if (i == activeButtonIndex) {
            InBox(165 + i * dx, 157, 185 + i * dx, 175);
        } else {
            OutBox(165 + i * dx, 157, 185 + i * dx, 175);
        }
    }

    av_need_update_xy(165, 157, 165 + i * dx, 175);
}

/**
 Determine if the hardware/unit exists and draws the appropriate the button

 @param hardware_index Zero based index
 @param unit_index Zero based index
 @param play_index Which player
 */
char QueryUnit(char hardware_index, char unit_index, char player_index)
{
    char enableButton = 0;

    enableButton =
        ((hardware_index == PROBE_HARDWARE && (Data->P[player_index].Probe[unit_index].Num != PROGRAM_NOT_STARTED)) ||
         (hardware_index == ROCKET_HARDWARE && (Data->P[player_index].Rocket[unit_index].Num != PROGRAM_NOT_STARTED)) ||
         (hardware_index == MANNED_HARDWARE && (Data->P[player_index].Manned[unit_index].Num != PROGRAM_NOT_STARTED)) ||
         (hardware_index == MISC_HARDWARE && (unit_index != MISC_HW_DOCKING_MODULE && Data->P[player_index].Misc[unit_index].Num != PROGRAM_NOT_STARTED))); //DM Screen, Nikakd, 10/8/10

    if (IsHumanPlayer(player_index)) {
        if (enableButton) {
            OutBox(165, 184, 315, 194);
        } else {
            InBox(165, 184, 315, 194);
        }
    }

    return(enableButton);
}


/**
 Check the maximum safety level

 @param hardware_index
 @param unit_index
 @param player_index
 */
char MaxChk(char hardware_index, char unit_index, char player_index)
{
    if (hardware_index == PROBE_HARDWARE) {
        return(Data->P[player_index].Probe[unit_index].Safety < Data->P[player_index].Probe[unit_index].MaxRD);
    }

    if (hardware_index == ROCKET_HARDWARE) {
        return(Data->P[player_index].Rocket[unit_index].Safety < Data->P[player_index].Rocket[unit_index].MaxRD);
    }

    if (hardware_index == MANNED_HARDWARE) {
        return(Data->P[player_index].Manned[unit_index].Safety < Data->P[player_index].Manned[unit_index].MaxRD);
    }

    if (hardware_index == MISC_HARDWARE) {
        return(Data->P[player_index].Misc[unit_index].Safety < Data->P[player_index].Misc[unit_index].MaxRD);
    }

    return(0);
}

/**
 * Roll dice and improve R&D value of a given hardware.
 *
 * HACK: need to store number of dices rolled in return value
 *
 * @param[in] hardwareTypeIndex
 * @param[in] hardwareIndex
 * @param[in] nRolls
 * @param[in] playerIndex
 * @return encoded number of dice and sum of rolls
 */
uint8_t
RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls, char playerIndex)
{
    int diceRoll = 0;
    int i;
    struct BuzzData *p = &Data->P[playerIndex];
    Equipment *eqArr[4] = {p->Probe, p->Rocket, p->Manned, p->Misc};
    Equipment *eq;

    assert(hardwareTypeIndex >= PROBE_HARDWARE);
    assert(hardwareTypeIndex <= MISC_HARDWARE);
    assert(hardwareIndex >= MANNED_HW_ONE_MAN_CAPSULE);
    assert(hardwareIndex <= MANNED_HW_ONE_MAN_MODULE);

    eq = &eqArr[hardwareTypeIndex][hardwareIndex];

    diceRoll = 0;

    for (i = 0; i < nRolls; i++) {
        diceRoll += rand() % (6 + p->RD_Mods_For_Turn) + 1;
    }

    eq->Safety += diceRoll;

    if (eq->Safety > eq->MaxRD) {
        eq->Safety = eq->MaxRD;
    }

    return encodeRolls(nRolls, diceRoll);
}

/**
 @param hw Zero based hardware index
 @param un Zero based unit index
 @param player_index Which player
 */
void ShowUnit(char hw, char un, char player_index)
{
    Equipment *PL;
    char qty = 1, SCol = 0;
    int Unit_Cost, Init_Cost;

    PL = NULL; /* XXX check uninitialized */

    switch (hw) {
    case PROBE_HARDWARE:
        PL = (Equipment *)&Data->P[player_index].Probe[un].Name[0];
        break;

    case ROCKET_HARDWARE:
        PL = (Equipment *)&Data->P[player_index].Rocket[un].Name[0];
        break;

    case MANNED_HARDWARE:
        PL = (Equipment *)&Data->P[player_index].Manned[un].Name[0];
        break;

    case MISC_HARDWARE:
        PL = (Equipment *)&Data->P[player_index].Misc[un].Name[0];
        break;

    default:
        // invalid hw value
        assert(false);
    }

    display::graphics.setForegroundColor(1);

    RectFill(162, 69, 318, 146, 3);
    RectFill(200, 9, 238, 21, 3);
    display::graphics.setForegroundColor(1);
    PrintAt(170, 97, "INITIAL COST:");
    PrintAt(170, 104, "UNIT COST:");
    PrintAt(170, 118, "R&D COST PER TEAM:");
    PrintAt(170, 125, "UNIT WEIGHT:");
    PrintAt(170, 132, "MAXIMUM PAYLOAD:");
    PrintAt(170, 146, "MAXIMUM SAFETY:");

    avoidf = 0;

    switch (hw) {
    case PROBE_HARDWARE:
        if (Data->P[player_index].Probe[un].SaveCard > 0) {
            avoidf = 1;
        }

        break;

    case ROCKET_HARDWARE:
        if (Data->P[player_index].Rocket[un].SaveCard > 0) {
            avoidf = 1;
        }

        break;

    case MANNED_HARDWARE:
        if (Data->P[player_index].Manned[un].SaveCard > 0) {
            avoidf = 1;
        }

        break;

    case MISC_HARDWARE:
        if (Data->P[player_index].Misc[un].SaveCard > 0) {
            avoidf = 1;
        }

        break;
    }

    if (avoidf > 0) {
        RectFill(286, 71, 316, 71, 5);
        RectFill(286, 86, 316, 86, 5);
        RectFill(286, 72, 286, 85, 5);
        RectFill(316, 72, 316, 85, 5);
        display::graphics.setForegroundColor(11);
        PrintAt(288, 77, "AVOID");
        PrintAt(291, 84, "FAIL");
        display::graphics.setForegroundColor(1);
    }

    if (Data->P[player_index].RD_Mods_For_Turn != 0) {
        if (Data->P[player_index].RD_Mods_For_Turn > 0) {
            display::graphics.setForegroundColor(5);
            PrintAt(170, 153, "RESEARCH STRONG:   ");
            RectFill(264, 149, 264, 153, 5);
            RectFill(262, 151, 266, 151, 5); // This fakes a plus sign, which the game apparently can't draw -Leon
        } else {
            display::graphics.setForegroundColor(8);
            PrintAt(170, 153, "RESEARCH WEAK: ");
        }

        DispNum(0, 0, Data->P[player_index].RD_Mods_For_Turn);
        PrintAt(0, 0, "%/TEAM");
    }

    display::graphics.setForegroundColor(20);
    PrintAt(170, 139, "MAXIMUM R&D:");

    char EqDmg = 0;

    switch (hw) {
    case PROBE_HARDWARE:
        EqDmg = Data->P[player_index].Probe[un].Damage != 0 ? 1 : 0;
        break;

    case ROCKET_HARDWARE:
        EqDmg = Data->P[player_index].Rocket[un].Damage != 0 ? 1 : 0;
        break;

    case MANNED_HARDWARE:
        EqDmg = Data->P[player_index].Manned[un].Damage != 0 ? 1 : 0;
        break;

    case MISC_HARDWARE:
        EqDmg = Data->P[player_index].Misc[un].Damage != 0 ? 1 : 0;
        break;

    default:
        break;
    }

    if (EqDmg != 0) {
        IOBox(281, 88, 304, 102);
        display::graphics.setForegroundColor(8);
        PrintAt(285, 97, "F");
        display::graphics.setForegroundColor(11);
        PrintAt(0, 0, "IX");
    }

    SCol = (PL->Num < 0) ? 20 : ((PL->Safety < 75) ? 8 : 16);
    display::graphics.setForegroundColor(SCol);
    PrintAt(170, 111, "SAFETY FACTOR:");

    display::graphics.setForegroundColor(11);
    DispMB(201, 15, Data->P[player_index].Cash);

    display::graphics.setForegroundColor(11);

    if (!(player_index == 1 && hw == ROCKET_HARDWARE && un == MANNED_HW_FOUR_MAN_CAPSULE)) {
        PrintAt(170, 80, &PL->Name[0]);
    }

    switch (hw) {
    case PROBE_HARDWARE:
        switch (un) {
        case PROBE_HW_ORBITAL:
        case PROBE_HW_INTERPLANETARY:
            PrintAt(0, 0, " SATELLITE");
            break;

        case PROBE_HW_LUNAR:
            PrintAt(0, 0, " PROBE");
            break;
        }

        break;

    case ROCKET_HARDWARE:
        switch (un - 1) {
        case ROCKET_HW_ONE_STAGE:
        case ROCKET_HW_TWO_STAGE:
        case ROCKET_HW_THREE_STAGE:
        case ROCKET_HW_MEGA_STAGE:
            PrintAt(0, 0, " ROCKET");
            break;

        case ROCKET_HW_BOOSTERS:
            if (player_index == 0) {
                PrintAt(0, 0, " STRAP-ON");
            } else {
                PrintAt(170, 80, "BOOSTER STAGE");
            }

            break;
        }

        break;

    case MANNED_HARDWARE:
        switch (un) {
        case MANNED_HW_ONE_MAN_CAPSULE:
        case MANNED_HW_TWO_MAN_CAPSULE:
        case MANNED_HW_THREE_MAN_CAPSULE:
            PrintAt(0, 0, " CAPSULE");
            break;

        case MANNED_HW_MINISHUTTLE:
            PrintAt(0, 0, " MINISHUTTLE");
            break;

        case MANNED_HW_FOUR_MAN_CAPSULE:
            PrintAt(0, 0, " SPACECRAFT");
            break;

        case MANNED_HW_TWO_MAN_MODULE:
        case MANNED_HW_ONE_MAN_MODULE:
            PrintAt(0, 0, " MODULE");
            break;
        }

        break;

    case MISC_HARDWARE:
        if (un <= MISC_HW_KICKER_C) {
            PrintAt(0, 0, " BOOSTER");
        }

        if (un == MISC_HW_DOCKING_MODULE) {
            PrintAt(0, 0, " MODULE");
        }

        break;
    }

    Init_Cost = PL->InitCost;
    Unit_Cost = PL->UnitCost;

    if (Data->P[player_index].TurnOnly == 3) {
        Init_Cost /= 2;
        Init_Cost = MAX(1, Init_Cost);

        if (Unit_Cost > 1) {
            Unit_Cost /= 2;
        }
    };

    DispNum(241, 97, Init_Cost);

    DispNum(230, 104, Unit_Cost);

    DispNum(275, 118, PL->RDCost);

    if (hw != ROCKET_HARDWARE) {
        DispNum(240, 125, PL->UnitWeight);
    } else {
        PrintAt(240, 125, "N/A");
    }

    display::graphics.setForegroundColor(20);

    if (PL->MaxRD != 0)   {
        DispNum(242, 139, PL->MaxRD);
        DispChr('%');

        if (options.want_debug) {
            PrintAt(0, 0, " / ");
            DispNum(0, 0, PL->MSF);
            DispChr('%'); //Used to test if MSF was holding the right value
        }
    } else {
        PrintAt(242, 139, "--");
    }

    display::graphics.setForegroundColor(11);
    DispNum(254, 146, PL->MaxSafety);
    DispChr('%');

    if (hw != ROCKET_HARDWARE) {
        PrintAt(268, 132, "N/A");    /* Payload */
    } else {
        DispNum(268, 132, PL->MaxPay);
    }

    if (PL->Num == -1) {
        display::graphics.setForegroundColor(8);
        PrintAt(170, 90, "NO PROGRAM INITIATED");
        display::graphics.setForegroundColor(20);
        DispNum(256, 111, 0);
        DispChr('%');
    } else {
        OnHand(PL->Num);
        display::graphics.setForegroundColor(SCol);
        DispNum(256, 111, PL->Safety);
        DispChr('%');

        //Display Damaged Equipment
        if (PL->Damage != 0) {
            display::graphics.setForegroundColor(8);
            PrintAt(280, 111, "(");
            DispNum(283, 111, PL->Safety + PL->Damage < 1 ? 1 : PL->Safety + PL->Damage);
            DispChr('%');
            DispChr(')');
        }

        qty = 0;
    };

    RectFill(27, 95, 130, 171, 0);

    BigHardMe(player_index, 27, 95, hw, un, qty, 32);
}

void OnHand(char qty)
{
    if (qty > 0) {
        display::graphics.setForegroundColor(16);
    } else {
        display::graphics.setForegroundColor(9);
    }

    PrintAt(170, 90, "UNITS ON HAND:");
    DispNum(251, 90, qty);
    return;
}

void DrawHPurc(char player_index)
{
    FILE *fin;

    FadeOut(2, display::graphics.palette(), 10, 0, 0);
    fin = sOpen("VAB.IMG", "rb", 0);
    fread(display::graphics.palette(), 768, 1, fin);
    fclose(fin);

    Load_RD_BUT(player_index);
    display::graphics.screen()->clear(0);
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 65);
    ShBox(17, 68, 143, 199);
    ShBox(21, 71, 139, 87);
    ShBox(161, 68, 319, 199);

    ShBox(0, 68, 158, 199);
    IOBox(3, 71, 154, 85);
    IOBox(3, 182, 76, 196);
    IOBox(81, 182, 154, 196);

    IOBox(5, 27, 77, 62);
    IOBox(81, 27, 158, 62);
    IOBox(162, 27, 239, 62);
    IOBox(243, 27, 315, 62);
    IOBox(163, 180, 317, 197);
    IOBox(243, 3, 316, 19);
    InBox(3, 3, 30, 19);
    FlagSm(player_index, 4, 4);

    IOBox(264, 162, 316, 176);
    display::graphics.setForegroundColor(11);
    PrintAt(279, 171, "UNDO");

    OutBox(21, 90, 136, 176);
    InBox(26, 94, 131, 172);

    display::graphics.setForegroundColor(1);
    DispBig(35, 5, "PURCHASING", 0, -1);
    //GradRect(27,95,130,171,player_index*16+128);
    RectFill(27, 95, 130, 171, 0);

    LTArrow(24, 186);
    RTArrow(101, 186);
    display::graphics.setForegroundColor(9);
    PrintAt(34, 80, "V");
    display::graphics.setForegroundColor(1);
    PrintAt(0, 0, "ISIT R&D FACILITY");

    but->copyTo(display::graphics.screen(), 0, 0, 8, 30, 74, 59); // Unmanned
    but->copyTo(display::graphics.screen(), 68, 0, 84, 30, 155, 59); // Rocket
    but->copyTo(display::graphics.screen(), 141, 0, 165, 30, 236, 59); // Manned
    but->copyTo(display::graphics.screen(), 214, 0, 246, 30, 312, 59); // Misc

    display::graphics.setForegroundColor(9);
    PrintAt(191, 190, "P");
    display::graphics.setForegroundColor(11);
    PrintAt(0, 0, "URCHASE EQUIPMENT");

    if (Data->Season == 0) {
        PrintAt(158, 8, "SPRING");
    } else {
        PrintAt(162, 8, "FALL");
    }

    PrintAt(163, 15, "19");
    DispNum(0, 0, Data->Year);

    PrintAt(200, 8, "CASH:");
    DispMB(201, 15, Data->P[player_index].Cash);

    display::graphics.setForegroundColor(1);
    PrintAt(258, 13, "CONTINUE");
    ShowUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);
    helpText = "i008";
    keyHelpText = "k008";

    return;
}

char HPurc(char player_index)
{
    short hardware, unit;
    FILE *undo;

    remove_savedat("UNDO.TMP");
    undo = sOpen("UNDO.TMP", "wb", 1);
    fwrite(Data, sizeof(struct Players), 1, undo);
    fclose(undo);

    hardware = HARD1;
    unit = UNIT1;
    helpText = "i008";
    keyHelpText = "k008";
    DrawHPurc(player_index);
    BButs(PROBE_HARDWARE, hardware);
    ShowUnit(hardware, unit, player_index);

    //Specs: undo fix
    //memset(vhptr->pixels(),0x00,64000);
    //memcpy(vhptr->pixels(),Data,sizeof(struct Players));

    FadeIn(2, display::graphics.palette(), 10, 0, 0);
    music_start(M_FILLER);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 283 && y >= 90 && x <= 302 && y <= 100 && mousebuttons > 0) || key == 'F') {
            char EqDmg = 0;

            switch (hardware) {
            case PROBE_HARDWARE:
                EqDmg = Data->P[player_index].Probe[unit].Damage != 0 ? 1 : 0;
                break;

            case ROCKET_HARDWARE:
                EqDmg = Data->P[player_index].Rocket[unit].Damage != 0 ? 1 : 0;
                break;

            case MANNED_HARDWARE:
                EqDmg = Data->P[player_index].Manned[unit].Damage != 0 ? 1 : 0;
                break;

            case MISC_HARDWARE:
                EqDmg = Data->P[player_index].Misc[unit].Damage != 0 ? 1 : 0;
                break;

            default:
                break;
            }

            if (EqDmg) {
                InBox(283, 90, 302, 100);
                DamProb(player_index, hardware, unit);
                helpText = "i008";
                keyHelpText = "k008";
                DrawHPurc(player_index);
                BButs(PROBE_HARDWARE, hardware);
                ShowUnit(hardware, unit, player_index);

                FadeIn(2, display::graphics.palette(), 10, 0, 0);
                music_start(M_FILLER);
                WaitForMouseUp();
            }
        } else if ((x > 266 && y > 164 && x < 314 && y < 174 && mousebuttons > 0) || key == 'Z') {
            InBox(266, 164, 314, 174);
            WaitForMouseUp();
            //    memcpy(Data,vhptr->pixels(),sizeof(struct Players));
            undo = sOpen("UNDO.TMP", "rb", 1);
            fread(Data, sizeof(struct Players), 1, undo);
            fclose(undo);
            ShowUnit(hardware, unit, player_index);
            OutBox(266, 164, 314, 174);
            key = 0;
        }

        if ((y >= 29 && y <= 60 && mousebuttons > 0) || (key == 'U' || key == 'R' || key == 'M' || key == 'C')) {
            if (((x >= 7 && x <= 75 && mousebuttons > 0) || key == 'U') && hardware != PROBE_HARDWARE) { /* PROBES */
                BButs(hardware, PROBE_HARDWARE);
                hardware = PROBE_HARDWARE;
                unit = PROBE_HW_ORBITAL;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 83 && x <= 156 && mousebuttons > 0) || key == 'R') && hardware != ROCKET_HARDWARE) { /* ROCKETS  */
                BButs(hardware, ROCKET_HARDWARE);
                hardware = ROCKET_HARDWARE;
                unit = ROCKET_HW_ONE_STAGE;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 164 && x <= 237 && mousebuttons > 0) || key == 'C') && hardware != MANNED_HARDWARE) { /* CAPSULES */
                BButs(hardware, MANNED_HARDWARE);
                hardware = MANNED_HARDWARE;
                unit = MANNED_HW_ONE_MAN_CAPSULE;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 245 && x <= 313 && mousebuttons > 0) || key == 'M') && hardware != MISC_HARDWARE) { /* MISC */
                BButs(hardware, MISC_HARDWARE);
                hardware = MISC_HARDWARE;
                unit = MISC_HW_KICKER_A;
                ShowUnit(hardware, unit, player_index);
            }
        } else if ((x >= 5 && y >= 184 && x <= 74 && y <= 194 && mousebuttons > 0) || key == LT_ARROW) { /* LEFT ARROW */
            InBox(5, 184, 74, 194);
            delay(5);
            WaitForMouseUp();
            OutBox(5, 184, 74, 194);
            unit--;

            switch (hardware) {
            case PROBE_HARDWARE:
                if (unit < PROBE_HW_ORBITAL) {
                    unit = PROBE_HW_LUNAR;
                }

                break;

            case ROCKET_HARDWARE:
                if (unit < ROCKET_HW_ONE_STAGE) {
                    unit = ROCKET_HW_BOOSTERS;
                }

                break;

            case MANNED_HARDWARE:
                if (unit < MANNED_HW_ONE_MAN_CAPSULE) {
                    unit = MANNED_HW_ONE_MAN_MODULE;
                }

                break;

            case MISC_HARDWARE:
                if (unit < MISC_HW_KICKER_A) {
                    unit = MISC_HW_DOCKING_MODULE;
                }

                if (player_index == 0 && unit == MISC_HW_KICKER_C) {
                    unit--;
                }

                break;
            }

            ShowUnit(hardware, unit, player_index);
        } else if ((x >= 83 && y >= 184 && x <= 152 && y <= 194 && mousebuttons > 0) || key == RT_ARROW) { /* RIGHT ARROW */
            InBox(83, 184, 152, 194);
            WaitForMouseUp();
            OutBox(83, 184, 152, 194);

            unit++;

            switch (hardware) {
            case PROBE_HARDWARE:
                if (unit > PROBE_HW_LUNAR) {
                    unit = PROBE_HW_ORBITAL;
                }

                break;

            case ROCKET_HARDWARE:
                if (unit > ROCKET_HW_BOOSTERS) {
                    unit = ROCKET_HW_ONE_STAGE;
                }

                break;

            case MANNED_HARDWARE:
                if (unit > MANNED_HW_ONE_MAN_MODULE) {
                    unit = MANNED_HW_ONE_MAN_CAPSULE;
                }

                break;

            case MISC_HARDWARE:
                if (unit > MISC_HW_DOCKING_MODULE) {
                    unit = MISC_HW_KICKER_A;
                }

                if (player_index == 0 && unit == MISC_HW_KICKER_C) {
                    unit++;
                }

                break;   //DM Screen, Nikakd, 10/8/10
            }

            ShowUnit(hardware, unit, player_index);
        } else if ((y >= 182 && y <= 195 && x >= 166 && x <= 314 && mousebuttons > 0) || key == 'P') { /* PURCHASE */
            InBox(165, 182, 315, 195);
            WaitForMouseUp();
            OutBox(165, 182, 315, 195);

            // NEED DELAY CHECK
            switch (hardware) {
            case PROBE_HARDWARE:
                if (Data->P[player_index].Probe[unit].Delay == 0) {
                    BuyUnit(hardware, unit, player_index);
                } else {
                    Help("i135");
                }

                break;

            case ROCKET_HARDWARE:
                if (Data->P[player_index].Rocket[unit].Delay == 0) {
                    BuyUnit(hardware, unit, player_index);
                } else {
                    Help("i135");
                }

                break;

            case MANNED_HARDWARE:
                if (Data->P[player_index].Manned[unit].Delay == 0) {
                    BuyUnit(hardware, unit, player_index);
                } else {
                    Help("i135");
                }

                break;

            case MISC_HARDWARE:
                if (Data->P[player_index].Misc[unit].Delay == 0) {
                    BuyUnit(hardware, unit, player_index);
                } else {
                    Help("i135");
                }

                break;

            default:
                break;
            }
        } else if (((y >= 3 && y <= 19) && (x >= 243 && x <= 316 && mousebuttons > 0)) || key == K_ENTER) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();
            music_stop();
            Del_RD_BUT();
            call = 0;
            HARD1 = PROBE_HARDWARE;
            UNIT1 = PROBE_HW_ORBITAL;
            remove_savedat("UNDO.TMP");
            return 0;   // Continue
        } else if ((x >= 5 && y >= 73 && x <= 152 && y <= 83 && mousebuttons > 0) || key == 'V') { // Gateway to RD
            InBox(5, 73, 152, 83);
            WaitForMouseUp();
            HARD1 = hardware;
            UNIT1 = unit;
            music_stop();
            remove_savedat("UNDO.TMP");

            //DM Screen, Nikakd, 10/8/10 (Removed line)
            if (call == 1) {
                return 1;
            }

            call = 1;
            wh = RD(player_index);

            if (call == 0) {
                return 0;    // Exit
            }

            call = 0;
            hardware = HARD1;
            unit = UNIT1;
            //DM Screen, Nikakd, 10/8/10 (Removed line)
            DrawHPurc(player_index);
            //    memcpy(vhptr->pixels(),Data,sizeof(struct Players));
            ShowUnit(hardware, unit, player_index);
            BButs(PROBE_HARDWARE, hardware);

            // Just Added stuff by mike
            undo = sOpen("UNDO.TMP", "wb", 1);
            fwrite(Data, sizeof(struct Players), 1, undo);
            fclose(undo);

            FadeIn(2, display::graphics.palette(), 10, 0, 0);
            music_start(M_FILLER);
            WaitForMouseUp();
        };
    };
}

/**
 * Record purchase of a hardware unit.
 * @param category index of hardware category
 * @param unit index of specific unit in the category
 * @param player_index player doing the purchase
 */

void
BuyUnit(char category, char unit, char player_index)
{
    short n1, n2, n3, n4, n5, n6, n7;
    char new_program = 0;
    int Init_Cost, Unit_Cost;
    Equipment *unit_ptr = NULL;

    assert(PROBE_HARDWARE <= category && category <= MISC_HARDWARE);

    switch (category) {
    case PROBE_HARDWARE:
        assert(PROBE_HW_ORBITAL <= unit && unit <= PROBE_HW_LUNAR);
        unit_ptr = &(Data->P[player_index].Probe[unit]);
        break;

    case ROCKET_HARDWARE:
        assert(ROCKET_HW_ONE_STAGE <= unit && unit <= ROCKET_HW_BOOSTERS);
        unit_ptr = &(Data->P[player_index].Rocket[unit]);
        break;

    case MANNED_HARDWARE:
        assert(MANNED_HW_ONE_MAN_CAPSULE <= unit && unit <= MANNED_HW_ONE_MAN_MODULE);
        unit_ptr = &(Data->P[player_index].Manned[unit]);
        break;

    case MISC_HARDWARE:
        assert(MISC_HW_KICKER_A <= unit && unit <= MISC_HW_DOCKING_MODULE);
        unit_ptr = &(Data->P[player_index].Misc[unit]);
        break;
    }

    Init_Cost = unit_ptr->InitCost;
    Unit_Cost = unit_ptr->UnitCost;

    /* this is the "half-off" sale */
    if (Data->P[player_index].TurnOnly == 3) {
        /* make sure changes do not affect items that are already free */
        if (Init_Cost) {
            Init_Cost = MAX(1, Init_Cost / 2);
        }

        if (Unit_Cost) {
            Unit_Cost = MAX(1, Unit_Cost / 2);
        }
    }

    if (unit_ptr->Num == -1 && Data->P[player_index].Cash >= Init_Cost) {
        Data->P[player_index].Cash -= Init_Cost;
        unit_ptr->Num = 1;
        new_program = 1;
        Data->P[player_index].Spend[0][category] += Init_Cost;
    } else if (unit_ptr->Num >= 0 && unit_ptr->Num < 6
               && Data->P[player_index].Cash >= Unit_Cost) {
        Data->P[player_index].Cash -= Unit_Cost;
        unit_ptr->Num += 1;
        Data->P[player_index].Spend[0][category] += Unit_Cost;
    }

    float tt = 0.0f;  // initialize variable for tech transfer

    /* compute technology transfer for Probe category */
    if (new_program && category == PROBE_HARDWARE) {
        n1 = Data->P[player_index].Probe[PROBE_HW_ORBITAL].Safety;
        n2 = Data->P[player_index].Probe[PROBE_HW_INTERPLANETARY].Safety;
        n3 = Data->P[player_index].Probe[PROBE_HW_LUNAR].Safety;

        switch (unit) {
        case PROBE_HW_ORBITAL:    // Explorer/Sputnik
            if (n2 > 40) {
                tt = (float)(n2 - 40) / 3.5f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_ORBITAL].Safety = 50;
            if (n3 > 40 && n3 > n1) {
                tt = (float)(n3 - 40) / 3.5f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_ORBITAL].Safety = 60;
            if (tt > 10.0f) {
                tt = 10.0f;
            }

            Data->P[player_index].Probe[PROBE_HW_ORBITAL].Safety = 40 + tt;

            break;

        case PROBE_HW_INTERPLANETARY:    // Ranger/Cosmos
            if (n1 > 40) {
                tt = (float)(n1 - 40) / 7.0f;
            }

            if (tt > 5.0f) {
                tt = 5.0f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_INTERPLANETARY].Safety = 45;
            if (n3 > 40 && n3 > n1) {
                tt = (float)(n3 - 40) / 3.5f;
            }

            if (tt > 10) {
                tt = 10.0f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_INTERPLANETARY].Safety = 50;
            Data->P[player_index].Probe[PROBE_HW_INTERPLANETARY].Safety = 40 + tt;

            break;

        case PROBE_HW_LUNAR:    // Surveyor/Luna
            if (n1 > 40) {
                tt = (float)(n1 - 40) / 7.0f;
            }

            if (tt > 5.0f) {
                tt = 5.0f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_LUNAR].Safety = 45;
            if (n2 > 40 && n2 > n1) {
                tt = (float)(n2 - 40) / 3.5f;
            }

            if (tt > 10.0f) {
                tt = 10.0f;
            }

            //old code: Data->P[player_index].Probe[PROBE_HW_LUNAR].Safety = 50;
            Data->P[player_index].Probe[PROBE_HW_LUNAR].Safety = 40 + tt;

            break;
        }
    }

    /* compute technology transfer for Rocket category */
    if (new_program && category == ROCKET_HARDWARE) {
        n1 = Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety; /* One-stage - A    */
        n2 = Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety; /* Two-stage - B    */
        n3 = Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety; /* Three-stage - C   */
        n4 = Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety; /* Mega - G    */
        n5 = Data->P[player_index].Rocket[ROCKET_HW_BOOSTERS].Safety; /* Booster - D */

        switch (unit) {
        case ROCKET_HW_ONE_STAGE:    // Atlas/R-7
            if (n2 > 10) {  // Tech from Titan/Proton
                tt = (float)(n2 - 10) / 2.6f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety = 10 + tt;

            if (n3 > 5) {    // Tech from Saturn/N1
                tt = (float)(n3 - 5) / 2.8f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety = 10 + tt;

            if (n4 > 5) {    // Tech from Nova/UR-700
                tt = (float)(n4 - 5) / 2.8f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            if (n5 > 10) { // Tech from Boosters
                tt = (float)(n5 - 10) / 2.6f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety = 10 + tt;
            //old code: if (n2 >= 75 || n3 >= 75 || n4 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety = 35;

            break;

        case ROCKET_HW_TWO_STAGE:    // Titan/Proton
            if (n1 > 10) {
                tt = (float)(n1 - 10) / 4.3f;
            }

            if (tt > 15.0f) {
                tt = 15.0f;
            }

            if (n5 > 10 && n5 > n1) {
                tt = (float)(n5 - 10) / 4.3f;
            }

            if (tt > 15.0f) {
                tt = 15.0f;
            }

            //old code: if (n1 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety = 25;
            if (n3 > 5) {
                if ((n3 - 5) / 4.6 > tt) {
                    tt = (float)(n3 - 5) / 2.3f;
                }
            }

            if (tt > 30) {
                tt = 30.0f;
            }

            if (n4 > 5) {
                if ((float)(n4 - 5) / 4.6f > tt) {
                    tt = (float)(n4 - 5) / 2.3f;
                }
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            //old code: if (n3 >= 75 || n4 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety = 40;
            Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety = 10 + tt;

            if ((n1 >= 75 || n5 >= 75) && (n3 >= 75 || n4 >= 75))  // Tech from multiple programs
                //old code: Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety = 65;

            {
                break;
            }

        case ROCKET_HW_THREE_STAGE:    // Saturn/N1
            if (n1 > 10) {
                tt = (float)(n1 - 10) / 6.5f;
            }

            if (tt > 10.0f) {
                tt = 10.0f;
            }

            if (n5 > 10 && n5 > n1) {
                tt = (float)(n5 - 10) / 6.5f;
            }

            if (tt > 10.0f) {
                tt = 10.0f;
            }

            //old code: if (n1 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety = 15;
            if (n2 > 10) {
                tt = (float)(n2 - 10) / 2.16f;
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            if (n4 > 5) {
                if ((float)(n4 - 5) / 3.5f > tt) {
                    tt = (float)(n4 - 5) / 2.3f;
                }
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            //old code: if (n2 >= 75 || n4 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety = 35;
            Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety = 5 + tt;

            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75)) { // Tech from multiple programs
                Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety = 60;
            }

            break;

        case ROCKET_HW_MEGA_STAGE:    // Nova/UR-700
            if (n1 > 10) {
                tt = (float)(n1 - 10) / 13.0f;
            }

            if (tt > 5.0f) {
                tt = 5.0f;
            }

            if (n5 > 10 && n5 > n1) {
                tt = (float)(n5 - 10) / 13.0f;
            }

            if (tt > 5.0f) {
                tt = 5.0f;
            }

            //old code: if (n1 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety = 10;
            if (n2 > 10) {
                tt = (float)(n2 - 5) / 3.25f;
            }

            if (tt > 5.0f) {
                tt = 20.0f;
            }

            if (n3 > 10) {
                if ((float)(n3 - 5) / 3.5f > tt) {
                    tt = (float)(n3 - 5) / 3.5f;
                }
            }

            if (tt > 5.0f) {
                tt = 20.0f;
            }

            Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety = 5 + tt;

            //old code: if (n2 >= 75 || n3 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety = 25;
            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n3 >= 75)) {  // Tech from multiple programs
                Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety = 35;
            }

            break;

        case ROCKET_HW_BOOSTERS:    // Boosters
            if (n1 > 10) {  // Tech from Atlas/R-7
                tt = (float)(n1 - 10) / 3.25f;
            }

            if (n2 > 10) {  // Tech from Titan/Proton
                if ((float)(n2 - 10) / 3.25f > tt) {
                    tt = (float)(n2 - 10) / 3.25f;
                }
            }

            if (n3 > 5) {    // Tech from Saturn/N1
                if ((float)(n3 - 5) / 3.5f > tt) {
                    tt = (float)(n3 - 5) / 3.5f;
                }
            }

            if (n4 > 5) {    // Tech from Nova/UR-700
                if ((float)(n4 - 5) / 3.5f > tt) {
                    tt = (float)(n4 - 5) / 3.5f;
                }
            }

            if (tt > 20.0f) {
                tt = 20.0f;
            }

            Data->P[player_index].Rocket[ROCKET_HW_BOOSTERS].Safety = 10 + tt;
            // old code: if (n1 >= 75 || n2 >= 75 || n3 >= 75 || n4 >= 75)
            //old code: Data->P[player_index].Rocket[ROCKET_HW_BOOSTERS].Safety = 30;
            break;
        }
    }

    /* compute technology transfer for Manned category */
    if (new_program && category == MANNED_HARDWARE) {
        n1 = Data->P[player_index].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety; /* One-person - a        */
        n2 = Data->P[player_index].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety; /* Two-person - b        */
        n3 = Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety; /* Three-person - c      */
        n4 = Data->P[player_index].Manned[MANNED_HW_MINISHUTTLE].Safety; /* Minishuttle - f */
        n5 = Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety; /* Dir Ascent - h    */
        n6 = Data->P[player_index].Manned[MANNED_HW_TWO_MAN_MODULE].Safety; /* 2-seat LM - d     */
        n7 = Data->P[player_index].Manned[MANNED_HW_ONE_MAN_MODULE].Safety; /*1-seat LM - e      */

        switch (unit) {
        case MANNED_HW_ONE_MAN_CAPSULE:  // Mercury/Vostok
            if (n2 > 5) { // tech from Gemini/Voskhod
                tt = (float)(n2 - 5) / 2.0f;
            }

            if (n3 > 5) {  // tech from Apollo/Soyuz
                if ((float)(n3 - 5) / 2.0f > tt) {
                    tt = (float)(n3 - 5) / 2.0f;
                }
            }

            if (n5 > 5) {  // tech from Jupiter/LK-700
                if ((float)(n5 - 5) / 2.0f > tt) {
                    tt = (float)(n5 - 5) / 2.0f;
                }
            }

            if (tt > 35.0f) {
                tt = 35.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety = 5 + tt;
            //old code: if (n2 >= 75 || n3 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety = 40;
            break;

        case MANNED_HW_TWO_MAN_CAPSULE:  // Gemini/Voskhod
            if (n1 > 1) {  // tech from Mercury/Vostok
                tt = (float)(n1 - 5) / 4.66f;
            }

            if (tt > 15.0f) {
                tt = 15.0f;
            }

            //old code: if (n1 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety = 20;
            if (n3 > 5) { // tech from Apollo/Soyuz
                if ((float)(n3 - 5) / 2.0f > tt) {
                    tt = (float)(n3 - 5) / 2.0f;
                }
            }

            if (n5 > 5) {  // tech from Jupiter/LK-700
                if ((float)(n5 - 5) / 2.3f > tt) {
                    tt = (float)(n5 - 5) / 2.3f;
                }
            }

            if (tt > 35.0f) {
                tt = 35.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety = 5 + tt;
            //old code: if (n3 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety = 40;
            break;

        case MANNED_HW_THREE_MAN_CAPSULE:  // Apollo/Soyuz
            if (n1 > 5) {  // tech from Mercury/Vostok
                tt = (float)(n1 - 5) / 4.66f;
            }

            if (tt > 15.0f) {
                tt = 15.0f;
            }

            if (n5 > 5) {  // tech from Jupiter/LK-700
                if ((float)(n5 - 5) / 2.3f > tt) {
                    tt = (float)(n5 - 5) / 2.3f;
                }
            }

            if (tt > 35.0f) {
                tt = 35.0f;
            }

            //old code: if (n1 >= 75 || n5 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety = 20;
            if (n2 > 5) {  // tech from Gemini/Voskhod
                if ((float)(n2 - 5) / 2.3f > tt) {
                    tt = (float)(n2 - 5) / 2.3f;
                }
            }

            if (tt > 35.0f) {
                tt = 35.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety = 5 + tt;

            //old code: if (n2 >= 75 || n4 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety = 30;
            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75)) {  // Tech from multiple programs
                Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety = 40;
            }

            break;

        case MANNED_HW_MINISHUTTLE: // Minishuttles
            break;

        case MANNED_HW_FOUR_MAN_CAPSULE:  // Jupiter/LK-700
            if (n1 > 5) {  // tech from Mercury/Vostok
                tt = (float)(n1 - 5) / 14.0f;
            }

            if (tt > 5.0f) {
                tt = 5.0f;
            }

            //old code: if (n1 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 10;
            if (n2 > 5) { // tech from Gemini/Voskhod
                if ((float)(n2 - 5) / 7.0f > tt) {
                    tt = (float)(n2 - 5) / 7.0f;
                }
            }

            if (tt > 10.0f) {
                tt = 10.0f;
            }

            //old code: if (n2 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 15;
            if (n3 > 5) { // tech from Apollo/Soyuz
                if ((float)(n3 - 5) / 3.5f > tt) {
                    tt = (float)(n3 - 5) / 3.5f;
                }
            }

            if (tt > 20.0f) {
                tt = 20.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 5 + tt;

            //old code: if (n3 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 25;
            if ((n1 >= 75 || n2 >= 75 || n3 >= 75) && (n6 >= 75  || n7 >= 75)) { // Tech from multiple programs
                Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 35;
            }

            break;

        case MANNED_HW_TWO_MAN_MODULE:  // Eagle/LKM
            if (n7 > 10) { // tech from Cricket/LK
                tt = (float)(n7 - 10) / 3.25f;
            }

            if (tt > 20.0f) {
                tt = 20.0f;
            }

            //old code: if (n7 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_TWO_MAN_MODULE].Safety = 30;
            if (n5 > 5) {  // tech from Jupiter/LK-700
                if ((float)(n5 - 5) / 2.3f > tt) {
                    tt = (float)(n5 - 5) / 2.3f;
                }
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_TWO_MAN_MODULE].Safety = 10 + tt;
            //old code: if (n5 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_TWO_MAN_MODULE].Safety = 40;

            break;

        case MANNED_HW_ONE_MAN_MODULE:  // Cricket/LK
            if (n6 > 10) {  // tech from Eagle/LKM
                tt = (float)(n6 - 10) / 3.25f;
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            //old code: if (n6 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_ONE_MAN_MODULE].Safety = 30;
            if (n5 > 5) { // tech from Jupiter/LK-700
                if ((float)(n5 - 5) / 2.3f > tt) {
                    tt = (float)(n5 - 5) / 2.3f;
                }
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            Data->P[player_index].Manned[MANNED_HW_ONE_MAN_MODULE].Safety = 10 + tt;
            //old code: if (n5 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_ONE_MAN_MODULE].Safety = 40;

            break;
        }
    }

    /* compute technology transfer for Misc category */
    if (new_program && category == MISC_HARDWARE) {
        n1 = Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety; /* One - A    */
        n2 = Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety; /* Two - B    */

        switch (unit) {
        case MISC_HW_KICKER_A:  // Kicker-A
            if (n2 > 10) { // tech from Titan/Proton
                tt = (float)(n2 - 10) / 2.16f;
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            Data->P[player_index].Misc[MISC_HW_KICKER_A].Safety = 10 + tt;
            //old code: if (n2 >= 75)
            //old code: Data->P[player_index].Manned[MISC_HW_KICKER_A].Safety = 40;

            break;

        case MISC_HW_KICKER_B:  // Kicker-B
            if (n1 > 10)  { // tech from Atlas/R-7
                tt = (float)(n1 - 10) / 2.6f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            Data->P[player_index].Misc[MISC_HW_KICKER_B].Safety = 10 + tt;
            //old code: if (n1 >= 75)
            //old code: Data->P[player_index].Manned[MISC_HW_KICKER_B].Safety = 35;

            break;

        case MISC_HW_KICKER_C:  // Kicker-C
            if (n1 > 10) { // tech from R-7
                tt = (float)(n1 - 10) / 3.25f;
            }

            if (n2 > 10) {  // tech from Proton
                if ((float)(n2 - 10) / 3.25f > tt) {
                    tt = (float)(n2 - 10) / 3.25f;
                }
            }

            if (tt > 20.0f) {
                tt = 20.0f;
            }

            Data->P[player_index].Misc[MISC_HW_KICKER_C].Safety = 5 + tt;
            //old code: if (n1 >= 75 || n2 >= 75)
            //old code: Data->P[player_index].Manned[MISC_HW_KICKER_C].Safety = 25;

            break;

        default:
            break;
        }
    }

    /* update safety that could have changed */
    if (new_program) {
        unit_ptr->Base = unit_ptr->Safety;
    }

    ShowUnit(category, unit, player_index);
    return;
}

/* vim: set noet ts=4 sw=4 tw=77 */




