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

// This file handles the R&D and Purchasing Facilities, and Technology Transfer

#include "display/graphics.h"
#include "display/palettized_surface.h"

#include "Buzz_inc.h"
#include "rdplex.h"
#include "options.h"
#include "draw.h"
#include "ast4.h"
#include "game_main.h"
#include "place.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include <assert.h>
#include "filesystem.h"
#include "hardware_buttons.h"
#include "hardware.h"

int call;
int wh;
boost::shared_ptr<display::PalettizedSurface> rd_men;


void LoadVABPalette(char plr);
void SRdraw_string(int x, int y, char *text, char fgd, char bck);
void DrawRD(char plr);
void DrawCashOnHand(char plr);
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

/**
 * Load the color palette used in the VAB into the display.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \throws runtime_error  if vab image cannot be loaded by Filesystem.
 */
void LoadVABPalette(const char plr)
{
    char filename[128];
    snprintf(filename, sizeof(filename), "images/vab.img.%d.png", plr);

    boost::shared_ptr<display::PalettizedSurface> sprite(
        Filesystem::readImage(filename));

    sprite->exportPalette();
}


void SRdraw_string(int x, int y, char *text, char fgd, char bck)
{
    display::graphics.setForegroundColor(bck);
    draw_string(x + 1, y + 1, text);
    display::graphics.setForegroundColor(fgd);
    draw_string(x, y, text);
    return;
}


void Load_RD_BUT(char player_index)
{
    char filename[128];

    snprintf(filename, sizeof(filename), "images/rd_men.%d.png", player_index);
    rd_men = boost::shared_ptr<display::PalettizedSurface>(Filesystem::readImage(filename));
}

void Del_RD_BUT()
{
    rd_men.reset();
}

void DrawRD(char player_index)
{
    int i;
    helpText = "i009";
    keyHelpText = "k009";

    FadeOut(2, 10, 0, 0);

    LoadVABPalette(player_index);
    display::graphics.screen()->clear();

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

    draw_left_arrow(24, 186);
    draw_right_arrow(101, 186);

    //GradRect(27,95,130,171,player_index*16+128);
    fill_rectangle(27, 95, 130, 171, 0);

    display::graphics.setForegroundColor(9);
    draw_string(12, 80, "V");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ISIT PURCHASING FACILITY");

    for (i = 0; i < 6; i++) {
        display::graphics.screen()->draw(rd_men, i * 20, 0, 19, 16, 166 + i * 26, 158);
    }

    display::graphics.setForegroundColor(3);
    grMoveTo(296, 174);
    grLineTo(314, 174);

    display::graphics.setForegroundColor(1);
    draw_heading(50, 5, "RESEARCH", 0, -1);
    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        draw_string(157, 9, "SPRING");
    } else {
        draw_string(162, 9, "FALL");
    }

    draw_string(163, 16, "19");
    draw_number(0, 0, Data->Year);

    draw_string(200, 9, "CASH:");
    DrawCashOnHand(player_index);

    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    draw_small_flag(player_index, 4, 4);
    QueryUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);
    ShowUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);

    return;
}  // End of DrawRD


/**
 * Update the player cash on hand in the R&D display.
 *
 * \param plr  the player index.
 */
void DrawCashOnHand(char plr)
{
    char str[10];
    snprintf(&str[0], 9, "%d MB", Data->P[plr].Cash);
    fill_rectangle(195, 10, 240, 21, 3);
    display::graphics.setForegroundColor(11);
    draw_string(213 - TextDisplayLength(&str[0]) / 2, 16, &str[0]);
}


void
RDButTxt(int cost, int encodedRolls, char playerIndex, char SpDModule)  //DM Screen, Nikakd, 10/8/10
{
    fill_rectangle(166, 185, 314, 193, 3);
    display::graphics.setForegroundColor(1);

    int diceRoll = decodeRollValue(encodedRolls);

    //DM Screen, Nikakd, 10/8/10
    if (SpDModule == 1) {
        draw_string(184, 191, "CANNOT BE RESEARCHED");
        return;
    }

    if (diceRoll == 0) {
        draw_string(169, 191, "RE");
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "S");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "EARCH PROGRAM FOR ");
        display::graphics.setForegroundColor(9);
        draw_number(0, 0, cost);
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, " MB");
    } else {
        display::graphics.setForegroundColor(11);
        draw_string(192, 191, "R&D ");
        draw_number(0, 0, diceRoll);
        draw_string(0, 0, "% IMPROVEMENT");

        if (Data->P[playerIndex].RD_Mods_For_Turn > 0) {
            draw_string(0, 0, "+");
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

    HardwareButtons hardware_buttons(30, player_index);

    DrawRD(player_index);
    hardware_buttons.drawButtons(HARD1);

    ShowUnit(hardware, unit, player_index);
    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

    if (buy[hardware][unit] == 0) {
        QueryUnit(hardware, unit, player_index);
    } else {
        InBox(165, 184, 315, 194);
    }

    ManSel(decodeNumRolls(buy[hardware][unit]));

    helpText = "i009";

    keyHelpText = "k009";

    FadeIn(2, 10, 0, 0);

    music_start(M_HARDWARE);

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {
            if ((x >= 283 && y >= 90 && x <= 302 && y <= 100) || key == 'F') {
                Equipment &Program =
                    HardwareProgram(player_index, hardware, unit);

                if (Program.Damage &&
                    Program.DCost <= Data->P[player_index].Cash) {
                    InBox(283, 90, 302, 100);
                    DamProb(player_index, hardware, unit);
                    DrawRD(player_index);
                    hardware_buttons.drawButtons(hardware);
                    ShowUnit(hardware, unit, player_index);
                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    helpText = "i009";
                    keyHelpText = "k009";

                    FadeIn(2, 10, 0, 0);
                }
            } else if ((y >= 29 && y <= 60 && mousebuttons > 0) || (key == 'U' || key == 'R' || key == 'M' || key == 'C')) {
                if (((x >= 7 && x <= 75 && mousebuttons > 0) || key == 'U') && hardware != PROBE_HARDWARE) {  /* Unmanned */
                    roll = 0;
                    hardware = PROBE_HARDWARE;
                    hardware_buttons.drawButtons(hardware);
                    unit = PROBE_HW_ORBITAL;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Probe[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 83 && x <= 156 && mousebuttons > 0) || key == 'R') && hardware != ROCKET_HARDWARE) {  /* Rockets */
                    roll = 0;
                    hardware = ROCKET_HARDWARE;
                    hardware_buttons.drawButtons(hardware);
                    unit = ROCKET_HW_ONE_STAGE;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Rocket[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 164 && x <= 237 && mousebuttons > 0) || key == 'C') && hardware != MANNED_HARDWARE) {  /* Manned */
                    roll = 0;
                    hardware = MANNED_HARDWARE;
                    hardware_buttons.drawButtons(hardware);
                    unit = MANNED_HW_ONE_MAN_CAPSULE;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Manned[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
                } else if (((x >= 245 && x <= 313 && mousebuttons > 0) || key == 'M') && hardware != MISC_HARDWARE) {  /* Misc */
                    roll = 0;
                    hardware = MISC_HARDWARE;
                    hardware_buttons.drawButtons(hardware);
                    unit = MISC_HW_KICKER_A;

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    ManSel(decodeNumRolls(buy[hardware][unit]));

                    ShowUnit(hardware, unit, player_index);

                    b = Data->P[player_index].Misc[unit].RDCost;

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
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

                b = HardwareProgram(player_index, hardware, unit).RDCost;

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
                ManSel(roll);
                WaitForMouseUp();
            } else if ((x >= 5 && y >= 184 && x <= 74 && y <= 194 && mousebuttons > 0) || key == LT_ARROW) {  /* LEFT ARROW */
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

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

                ManSel(decodeNumRolls(buy[hardware][unit]));

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                }

                ShowUnit(hardware, unit, player_index);

                OutBox(5, 184, 74, 194);
            } else if ((x >= 83 && y >= 184 && x <= 152 && y <= 194 && mousebuttons > 0) || key == RT_ARROW) {  /* RIGHT ARROW */
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

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

                ManSel(decodeNumRolls(buy[hardware][unit]));

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                }

                ShowUnit(hardware, unit, player_index);

                RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

                OutBox(83, 184, 152, 194);
            } else if (((x >= 165 && y >= 184 && x <= 315 && y <= 194 && mousebuttons > 0) || key == 'S') && buy[hardware][unit] == 0
                       && roll != 0) {
                // b is the cost per roll

                b = HardwareProgram(player_index, hardware, unit).RDCost;

                // Add to the expenditure data
                if ((b * roll <= Data->P[player_index].Cash) && QueryUnit(hardware, unit, player_index)
                    && MaxChk(hardware, unit, player_index)) {
                    buy[hardware][unit] = RDUnit(hardware, unit, roll, player_index);

                    if (buy[hardware][unit] == 0) {
                        QueryUnit(hardware, unit, player_index);
                    } else {
                        InBox(165, 184, 315, 194);
                    }

                    Data->P[player_index].Cash -= b * roll;

                    // add the amount to the expenditure budget
                    Data->P[player_index].Spend[0][hardware] += b * roll;

                    ShowUnit(hardware, unit, player_index);

                    RDButTxt(b * roll, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10
                } else {
                    QueryUnit(hardware, unit, player_index);
                }
            } else if (((y >= 3 && y <= 19) && (x >= 243 && x <= 316 && mousebuttons > 0)) || key == K_ENTER) {
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                for (i = 0; i < 4; i++) {
                    for (j = 0; j < 7; j++) {
                        Data->P[player_index].Buy[i][j] = buy[i][j];
                    }
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

                // DM Screen, Nikakd, 10/8/10 (Removed line)
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
                hardware_buttons.drawButtons(hardware);
                ShowUnit(hardware, unit, player_index);
                RDButTxt(0, buy[hardware][unit], player_index, ((hardware == MISC_HARDWARE && unit == MISC_HW_DOCKING_MODULE) ? 1 : 0));  //DM Screen, Nikakd, 10/8/10

                if (buy[hardware][unit] == 0) {
                    QueryUnit(hardware, unit, player_index);
                } else {
                    InBox(165, 184, 315, 194);
                }

                FadeIn(2, 10, 0, 0);

                music_start(M_HARDWARE);

                WaitForMouseUp();
            }
        }

        gr_sync();
    }
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
}

/**
 Determine if the hardware/unit exists and draw the appropriate button

 @param hardware_index Zero based index
 @param unit_index Zero based index
 @param play_index Which player
 */
char QueryUnit(char hardware_index, char unit_index, char player_index)
{
    char enableButton = 0;

    // DM Screen, Nikakd, 10/8/10
    if (hardware_index != MISC_HARDWARE ||
        unit_index != MISC_HW_DOCKING_MODULE) {
        enableButton =
            HardwareProgram(player_index, hardware_index, unit_index).Num
            != PROGRAM_NOT_STARTED;
    }

    if (IsHumanPlayer(player_index)) {
        if (enableButton) {
            OutBox(165, 184, 315, 194);
        } else {
            InBox(165, 184, 315, 194);
        }
    }

    return enableButton;
}


/**
 Determine if the program's current safety is below the maximum R&D.

 @param hardware_index
 @param unit_index
 @param player_index
 */
char MaxChk(char hardware_index, char unit_index, char player_index)
{
    const Equipment &program =
        HardwareProgram(player_index, hardware_index, unit_index);
    return program.Safety < program.MaxRD;
}

/**
 * Roll dice and improve R&D value of a given hardware.
 *
 * HACK: need to store number of dice rolled in return value
 *
 * @param[in] hardwareTypeIndex
 * @param[in] hardwareIndex
 * @param[in] nRolls
 * @param[in] playerIndex
 * @return encoded number of dice and sum of rolls
 */
uint8_t
RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls,
       char playerIndex)
{
    assert(hardwareTypeIndex >= PROBE_HARDWARE);
    assert(hardwareTypeIndex <= MISC_HARDWARE);
    assert(hardwareIndex >= MANNED_HW_ONE_MAN_CAPSULE);
    assert(hardwareIndex <= MANNED_HW_ONE_MAN_MODULE);

    Equipment &eq = HardwareProgram(playerIndex,
                                    hardwareTypeIndex,
                                    hardwareIndex);
    int diceRoll = 0;
    int diceType = 6 + Data->P[playerIndex].RD_Mods_For_Turn;

    for (int i = 0; i < nRolls; i++) {
        diceRoll += rand() % diceType + 1;
    }

    eq.Safety += diceRoll;

    if (eq.Safety > eq.MaxRD) {
        eq.Safety = eq.MaxRD;
    }

    return encodeRolls(nRolls, diceRoll);
}

/**
 * Draws the hardware program's statistics display.
 *
 * @param hw Zero based hardware index
 * @param un Zero based unit index
 * @param player_index Which player
 */
void ShowUnit(char hw, char un, char player_index)
{
    char qty = 1, SCol = 0;
    int Unit_Cost, Init_Cost;

    Equipment &program = HardwareProgram(player_index, hw, un);

    display::graphics.setForegroundColor(1);

    fill_rectangle(162, 69, 318, 146, 3);
    display::graphics.setForegroundColor(1);
    draw_string(170, 97, "INITIAL COST:");
    draw_string(170, 104, "UNIT COST:");
    draw_string(170, 118, "R&D COST PER TEAM:");
    draw_string(170, 125, "UNIT WEIGHT:");
    draw_string(170, 132, "MAXIMUM PAYLOAD:");
    draw_string(170, 146, "MAXIMUM SAFETY:");

    // Set Avoid Failure notification
    if (program.SaveCard > 0) {
        fill_rectangle(286, 71, 316, 71, 5);
        fill_rectangle(286, 86, 316, 86, 5);
        fill_rectangle(286, 72, 286, 85, 5);
        fill_rectangle(316, 72, 316, 85, 5);
        display::graphics.setForegroundColor(11);
        draw_string(288, 77, "AVOID");
        draw_string(291, 84, "FAIL");
        display::graphics.setForegroundColor(1);
    }

    if (Data->P[player_index].RD_Mods_For_Turn != 0) {
        if (Data->P[player_index].RD_Mods_For_Turn > 0) {
            display::graphics.setForegroundColor(5);
            draw_string(170, 153, "RESEARCH STRONG: +");
        } else {
            display::graphics.setForegroundColor(8);
            draw_string(170, 153, "RESEARCH WEAK: ");
        }

        draw_number(0, 0, Data->P[player_index].RD_Mods_For_Turn);
        draw_string(0, 0, "/TEAM");
    }

    display::graphics.setForegroundColor(20);
    draw_string(170, 139, "MAXIMUM R&D:");

    if (program.Damage) {
        IOBox(281, 88, 304, 102);

        if (program.DCost > Data->P[player_index].Cash) {
            InBox(283, 90, 302, 100);
        }

        display::graphics.setForegroundColor(8);
        draw_string(285, 97, "F");
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, "IX");
    }

    SCol = (program.Num < 0) ? 20 : ((program.Safety < 75) ? 8 : 16);
    display::graphics.setForegroundColor(SCol);
    draw_string(170, 111, "SAFETY FACTOR:");

    DrawCashOnHand(player_index);
    display::graphics.setForegroundColor(11);

    if (!(player_index == 1 && hw == ROCKET_HARDWARE &&
          un == MANNED_HW_FOUR_MAN_CAPSULE)) {
        draw_string(170, 80, &program.Name[0]);
    }

    switch (hw) {
    case PROBE_HARDWARE:
        switch (un) {
        case PROBE_HW_ORBITAL:
        case PROBE_HW_INTERPLANETARY:
            draw_string(0, 0, " SATELLITE");
            break;

        case PROBE_HW_LUNAR:
            draw_string(0, 0, " PROBE");
            break;
        }

        break;

    case ROCKET_HARDWARE:
        switch (un) {  // This used to say (un - 1) but that made the 1-stage rocket say R-7 instead of R7 ROCKET, and boosters had no name at the top
        case ROCKET_HW_ONE_STAGE:
        case ROCKET_HW_TWO_STAGE:
        case ROCKET_HW_THREE_STAGE:
        case ROCKET_HW_MEGA_STAGE:
            draw_string(0, 0, " ROCKET");
            break;

        case ROCKET_HW_BOOSTERS:
            if (player_index == 0) {
                draw_string(0, 0, " STRAP-ON");
            } else {
                draw_string(170, 80, "BOOSTER STAGE");
            }

            break;
        }

        break;

    case MANNED_HARDWARE:
        switch (un) {
        case MANNED_HW_ONE_MAN_CAPSULE:
        case MANNED_HW_TWO_MAN_CAPSULE:
        case MANNED_HW_THREE_MAN_CAPSULE:
            draw_string(0, 0, " CAPSULE");
            break;

        case MANNED_HW_MINISHUTTLE:
            draw_string(0, 0, " MINISHUTTLE");
            break;

        case MANNED_HW_FOUR_MAN_CAPSULE:
            draw_string(0, 0, " SPACECRAFT");
            break;

        case MANNED_HW_TWO_MAN_MODULE:
        case MANNED_HW_ONE_MAN_MODULE:
            draw_string(0, 0, " MODULE");
            break;
        }

        break;

    case MISC_HARDWARE:
        if (un <= MISC_HW_KICKER_C) {
            draw_string(0, 0, " BOOSTER");
        }

        if (un == MISC_HW_DOCKING_MODULE) {
            draw_string(0, 0, " MODULE");
        }

        break;
    }

    Init_Cost = program.InitCost;
    Unit_Cost = program.UnitCost;

    if (Data->P[player_index].TurnOnly == 3) {
        Init_Cost /= 2;
        Init_Cost = MAX(1, Init_Cost);

        if (Unit_Cost > 1) {
            Unit_Cost /= 2;
        }
    }

    draw_number(241, 97, Init_Cost);
    draw_number(230, 104, Unit_Cost);
    draw_number(275, 118, program.RDCost);

    if (hw != ROCKET_HARDWARE) {
        draw_number(240, 125, program.UnitWeight);
    } else {
        draw_string(240, 125, "N/A");
    }

    display::graphics.setForegroundColor(20);

    if (program.MaxRD != 0)   {
        draw_number(242, 139, program.MaxRD);
        draw_character('%');

        if (options.want_debug) {
            draw_string(0, 0, " / ");
            draw_number(0, 0, program.MSF);
            draw_character('%');  // Used to test if MSF was holding the right value
        }
    } else {
        draw_string(242, 139, "--");
    }

    display::graphics.setForegroundColor(11);
    draw_number(254, 146, program.MaxSafety);
    draw_character('%');

    if (hw != ROCKET_HARDWARE) {
        draw_string(268, 132, "N/A");    /* Payload */
    } else {
        draw_number(268, 132, program.MaxPay);
    }

    if (program.Num == -1) {
        display::graphics.setForegroundColor(8);
        draw_string(170, 90, "NO PROGRAM INITIATED");
        display::graphics.setForegroundColor(20);
        draw_number(256, 111, 0);
        draw_character('%');
    } else {
        OnHand(program.Num);
        display::graphics.setForegroundColor(SCol);
        draw_number(256, 111, program.Safety);
        draw_character('%');

        // Display Damaged Equipment
        if (program.Damage != 0) {
            display::graphics.setForegroundColor(8);
            draw_string(280, 111, "(");
            draw_number(283, 111, program.Safety + program.Damage < 1 ? 1 : program.Safety + program.Damage);
            draw_character('%');
            draw_character(')');
        }

        qty = 0;
    }

    fill_rectangle(27, 95, 130, 171, 0);

    BigHardMe(player_index, 27, 95, hw, un, qty);
}

void OnHand(char qty)
{
    if (qty > 0) {
        display::graphics.setForegroundColor(16);
    } else {
        display::graphics.setForegroundColor(9);
    }

    draw_string(170, 90, "UNITS ON HAND:");
    draw_number(251, 90, qty);
    return;
}

void DrawHPurc(char player_index)
{
    FadeOut(2, 10, 0, 0);

    LoadVABPalette(player_index);
    Load_RD_BUT(player_index);
    display::graphics.screen()->clear();
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
    draw_small_flag(player_index, 4, 4);

    IOBox(264, 162, 316, 176);
    display::graphics.setForegroundColor(11);
    draw_string(279, 171, "UNDO");

    OutBox(21, 90, 136, 176);
    InBox(26, 94, 131, 172);

    display::graphics.setForegroundColor(1);
    draw_heading(35, 5, "PURCHASING", 0, -1);
    // GradRect(27,95,130,171,player_index*16+128);
    fill_rectangle(27, 95, 130, 171, 0);

    draw_left_arrow(24, 186);
    draw_right_arrow(101, 186);
    display::graphics.setForegroundColor(9);
    draw_string(32, 80, "V");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ISIT R&D FACILITY");

    display::graphics.setForegroundColor(9);
    draw_string(191, 190, "P");
    display::graphics.setForegroundColor(11);
    draw_string(0, 0, "URCHASE EQUIPMENT");

    if (Data->Season == 0) {
        draw_string(158, 9, "SPRING");
    } else {
        draw_string(162, 9, "FALL");
    }

    draw_string(163, 16, "19");
    draw_number(0, 0, Data->Year);

    draw_string(200, 9, "CASH:");
    DrawCashOnHand(player_index);

    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    ShowUnit(PROBE_HARDWARE, PROBE_HW_ORBITAL, player_index);
    helpText = "i008";
    keyHelpText = "k008";

    return;
}

char HPurc(char player_index)
{
    short hardware, unit;
    FILE *undo;

    HardwareButtons hardware_buttons(30, player_index);

    remove_savedat("UNDO.TMP");
    undo = sOpen("UNDO.TMP", "wb", 1);
    fwrite(Data, sizeof(struct Players), 1, undo);
    fclose(undo);

    hardware = HARD1;
    unit = UNIT1;
    helpText = "i008";
    keyHelpText = "k008";
    DrawHPurc(player_index);
    hardware_buttons.drawButtons(hardware);
    ShowUnit(hardware, unit, player_index);

    FadeIn(2, 10, 0, 0);
    music_start(M_FILLER);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 283 && y >= 90 && x <= 302 && y <= 100 && mousebuttons > 0) || key == 'F') {
            Equipment &program =
                HardwareProgram(player_index, hardware, unit);

            if (program.Damage &&
                program.DCost <= Data->P[player_index].Cash) {
                InBox(283, 90, 302, 100);
                DamProb(player_index, hardware, unit);
                helpText = "i008";
                keyHelpText = "k008";
                DrawHPurc(player_index);
                hardware_buttons.drawButtons(hardware);
                ShowUnit(hardware, unit, player_index);

                FadeIn(2, 10, 0, 0);
                WaitForMouseUp();
            }
        } else if ((x > 266 && y > 164 && x < 314 && y < 174 && mousebuttons > 0) || key == 'Z') {
            InBox(266, 164, 314, 174);
            WaitForMouseUp();
            undo = sOpen("UNDO.TMP", "rb", 1);
            fread(Data, sizeof(struct Players), 1, undo);
            fclose(undo);
            ShowUnit(hardware, unit, player_index);
            OutBox(266, 164, 314, 174);
            key = 0;
        }

        if ((y >= 29 && y <= 60 && mousebuttons > 0) || (key == 'U' || key == 'R' || key == 'M' || key == 'C')) {
            if (((x >= 7 && x <= 75 && mousebuttons > 0) || key == 'U') && hardware != PROBE_HARDWARE) {  /* PROBES */
                hardware = PROBE_HARDWARE;
                hardware_buttons.drawButtons(hardware);
                unit = PROBE_HW_ORBITAL;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 83 && x <= 156 && mousebuttons > 0) || key == 'R') && hardware != ROCKET_HARDWARE) {  /* ROCKETS */
                hardware = ROCKET_HARDWARE;
                hardware_buttons.drawButtons(hardware);
                unit = ROCKET_HW_ONE_STAGE;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 164 && x <= 237 && mousebuttons > 0) || key == 'C') && hardware != MANNED_HARDWARE) {  /* CAPSULES */
                hardware = MANNED_HARDWARE;
                hardware_buttons.drawButtons(hardware);
                unit = MANNED_HW_ONE_MAN_CAPSULE;
                ShowUnit(hardware, unit, player_index);
            } else if (((x >= 245 && x <= 313 && mousebuttons > 0) || key == 'M') && hardware != MISC_HARDWARE) {  /* MISC */
                hardware = MISC_HARDWARE;
                hardware_buttons.drawButtons(hardware);
                unit = MISC_HW_KICKER_A;
                ShowUnit(hardware, unit, player_index);
            }
        } else if ((x >= 5 && y >= 184 && x <= 74 && y <= 194 && mousebuttons > 0) || key == LT_ARROW) {  /* LEFT ARROW */
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
        } else if ((x >= 83 && y >= 184 && x <= 152 && y <= 194 && mousebuttons > 0) || key == RT_ARROW) {  /* RIGHT ARROW */
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

                break;   // DM Screen, Nikakd, 10/8/10
            }

            ShowUnit(hardware, unit, player_index);
        } else if ((y >= 182 && y <= 195 && x >= 166 && x <= 314 && mousebuttons > 0) || key == 'P') {  /* PURCHASE */
            InBox(165, 182, 315, 195);
            WaitForMouseUp();
            OutBox(165, 182, 315, 195);

            // NEED DELAY CHECK
            if (! HardwareProgram(player_index, hardware, unit).Delay) {
                BuyUnit(hardware, unit, player_index);
            } else {
                Help("i135");
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
        } else if ((x >= 5 && y >= 73 && x <= 152 && y <= 83 && mousebuttons > 0) || key == 'V') {  // Gateway to RD
            InBox(5, 73, 152, 83);
            WaitForMouseUp();
            HARD1 = hardware;
            UNIT1 = unit;
            music_stop();
            remove_savedat("UNDO.TMP");

            // DM Screen, Nikakd, 10/8/10 (Removed line)
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
            DrawHPurc(player_index);
            ShowUnit(hardware, unit, player_index);
            hardware_buttons.drawButtons(hardware);

            // Just Added stuff by mike
            undo = sOpen("UNDO.TMP", "wb", 1);
            fwrite(Data, sizeof(struct Players), 1, undo);
            fclose(undo);

            FadeIn(2, 10, 0, 0);
            music_start(M_FILLER);
            WaitForMouseUp();
        }
    }
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
        n1 = Data->P[player_index].Rocket[ROCKET_HW_ONE_STAGE].Safety;  /* One-stage - A    */
        n2 = Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety;  /* Two-stage - B    */
        n3 = Data->P[player_index].Rocket[ROCKET_HW_THREE_STAGE].Safety;  /* Three-stage - C   */
        n4 = Data->P[player_index].Rocket[ROCKET_HW_MEGA_STAGE].Safety;  /* Mega - G    */
        n5 = Data->P[player_index].Rocket[ROCKET_HW_BOOSTERS].Safety;  /* Booster - D */

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

            if (n5 > 10) {   // Tech from Boosters
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

            // Tech from multiple programs
            if ((n1 >= 75 || n5 >= 75) && (n3 >= 75 || n4 >= 75)) {
                Data->P[player_index].Rocket[ROCKET_HW_TWO_STAGE].Safety = 65;
            }

            break;

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

            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75)) {  // Tech from multiple programs
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
            if (n1 > 10) {   // Tech from Atlas/R-7
                tt = (float)(n1 - 10) / 3.25f;
            }

            if (n2 > 10) {   // Tech from Titan/Proton
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
            // old code: Data->P[player_index].Rocket[ROCKET_HW_BOOSTERS].Safety = 30;
            break;
        }
    }

    /* compute technology transfer for Manned category */
    if (new_program && category == MANNED_HARDWARE) {
        n1 = Data->P[player_index].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety;  /* One-person - a        */
        n2 = Data->P[player_index].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety;  /* Two-person - b        */
        n3 = Data->P[player_index].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety;  /* Three-person - c      */
        n4 = Data->P[player_index].Manned[MANNED_HW_MINISHUTTLE].Safety;  /* Minishuttle - f */
        n5 = Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety;  /* Direct Ascent - h    */
        n6 = Data->P[player_index].Manned[MANNED_HW_TWO_MAN_MODULE].Safety;  /* 2-seat LM - d     */
        n7 = Data->P[player_index].Manned[MANNED_HW_ONE_MAN_MODULE].Safety;  /*1-seat LM - e      */

        switch (unit) {
        case MANNED_HW_ONE_MAN_CAPSULE:  // Mercury/Vostok
            if (n2 > 5) {  // tech from Gemini/Voskhod
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
            if (n3 > 5) {  // tech from Apollo/Soyuz
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

        case MANNED_HW_MINISHUTTLE: // Minishuttles (don't get tech transfer)
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
            if (n2 > 5) {  // tech from Gemini/Voskhod
                if ((float)(n2 - 5) / 7.0f > tt) {
                    tt = (float)(n2 - 5) / 7.0f;
                }
            }

            if (tt > 10.0f) {
                tt = 10.0f;
            }

            //old code: if (n2 >= 75)
            //old code: Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 15;
            if (n3 > 5) {  // tech from Apollo/Soyuz
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
            if ((n1 >= 75 || n2 >= 75 || n3 >= 75) && (n6 >= 75  || n7 >= 75)) {  // Tech from multiple programs
                Data->P[player_index].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety = 35;
            }

            break;

        case MANNED_HW_TWO_MAN_MODULE:  // Eagle/LKM
            if (n7 > 10) {  // tech from Cricket/LK
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
            if (n5 > 5) {  // tech from Jupiter/LK-700
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
            if (n2 > 10) {  // tech from Titan/Proton
                tt = (float)(n2 - 10) / 2.16f;
            }

            if (tt > 30.0f) {
                tt = 30.0f;
            }

            Data->P[player_index].Misc[MISC_HW_KICKER_A].Safety = 10 + tt;
            //old code: if (n2 >= 75)
            //old code: Data->P[player_index].Misc[MISC_HW_KICKER_A].Safety = 40;

            break;

        case MISC_HW_KICKER_B:  // Kicker-B
            if (n1 > 10)  {  // tech from Atlas/R-7
                tt = (float)(n1 - 10) / 2.6f;
            }

            if (tt > 25.0f) {
                tt = 25.0f;
            }

            Data->P[player_index].Misc[MISC_HW_KICKER_B].Safety = 10 + tt;
            //old code: if (n1 >= 75)
            //old code: Data->P[player_index].Misc[MISC_HW_KICKER_B].Safety = 35;

            break;

        case MISC_HW_KICKER_C:  // Kicker-C
            if (n1 > 10) {  // tech from R-7
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
            //old code: Data->P[player_index].Misc[MISC_HW_KICKER_C].Safety = 25;

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
