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
/** \file vab.c Vehicle Assembly Building
 *
 */

#include "display/graphics.h"
#include "display/surface.h"

#include "vab.h"
#include "gamedata.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "options.h"
#include "admin.h"
#include "game_main.h"
#include "mis_c.h"
#include "news_suq.h"
#include "place.h"
#include "radar.h"
#include "mc.h"
#include "sdlhelper.h"
#include "port.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"

/* VAS holds all possible payload configurations for the given mission.
 * Each payload consists of four components:
 *   0: Primary (a capsule)
 *   1: Kicker
 *   2: LM
 *   3: Payload (Probe / DM)
 * Any of which may be empty. There are only ever a maximum of seven
 * potential payload combinations available at assembly time, each of
 * which is stored in VAS.
 */
struct VInfo VAS[7][4];
int VASqty; // How many payload configurations there are

// CAP,LM,SDM,DMO,EVA,PRO,INT,KIC
char isDamaged[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* MI contains the location of a vehicle equipment image and it's
 * positioning when drawn inside a vehicle casing.
 *
 * The images of each component used in vehicle assembly are stored as
 * part of a single VAB superimage containing all the component images.
 * The two coordinates (x1, y1) and (x2, y2) represent the top-left
 * corner and bottom-right corner, respectively, of the component's
 * space in that image. The yOffset values describes how much space a
 * capsule image should be given from the top of the casing image in
 * order to properly align it within the launch vehicle casing.
 * The Mercury capsule is a special case, in that the Mercury capsule
 * has a tower meant to extend above the vehicle casing, and thus the
 * offset should be used to move the casing (rather than capsule)
 * down that much.
 */
struct MDA {
    int16_t x1, y1, x2, y2, yOffset;
} MI[2 * 28];

/*
 // The VAB images are found in two different image files, these are the cutout locations

//
 struct MDA {
    int16_t x1, y1, x2, y2, o;
 } MI[2][28] = {
     {104, 1, 119, 55, 0}, // Atlas  0
     {143, 1, 153, 67, 0}, // Titan  1
     {177, 1, 217, 133, 0}, // Saturn 2
     {219, 1, 262, 139, 0}, // Nova   3
     {121, 1, 141, 53, 0}, // Atlas+B 4
     {155, 1, 175, 67, 0}, // Titan+B 5
     {177, 1, 217, 133, 0}, // Saturn+B 6
     {83, 1, 102, 57, 0}, // Casing Small 7
     {264, 1, 318, 145, 0}, // Casing Large 8
     {59, 48, 75, 67, 0}, // Orbital 9
     {59, 69, 73, 84, 0}, // InterPlan 10
     {59, 86, 72, 100, 0}, // Lunar Probe 11
     {130, 55, 141, 59, 0}, // Docking 12
     {2, 1, 18, 39, 13}, // Merc 13
     {79, 69, 111, 117, 2}, // Gemini 14
     {20, 1, 57, 59, 12}, // Apollo 15
     {59, 1, 81, 46, 7}, // MiniSh 16
     {137, 69, 175, 139, 7}, // Four Cap 17
     {223, 141, 262, 178, 0}, // Two Lem 18
     {188, 141, 221, 177, 0}, // One Lem 19
     {138, 141, 160, 166, 0}, // KickA 20
     {162, 141, 186, 172, 0}, // KickB 21
     {0, 0, 0, 0, 0},    // KickC 22
     {0, 0, 0, 0, 0},    // None : Zond 23
     {0, 0, 0, 0, 0},    // Filler Sm
     {20, 61, 57, 141, 0}, // Filler Lg
     {113, 69, 132, 98, 0}, // smShroud
     {81, 119, 135, 153, 0}, // LgShroud

     {86, 1, 113, 74, 0}, // A-Series 0
     {115, 1, 137, 82, 0}, // Proton 1
     {192, 1, 227, 130, 0}, // N-1      2
     {229, 0, 263, 131, 0}, // Energia  3
     {139, 1, 166, 85, 0}, // A-Series 4
     {168, 1, 190, 95, 0}, // Proton+B   5
     {192, 1, 227, 130, 0}, // N-1+B      6
     {3, 60, 22, 116, 0}, // Casing Small 7
     {277, 1, 318, 145, 0}, // Casing Large 8
     {5, 32, 22, 51, 0},  // Orbital  9
     {79, 132, 89, 157, 0}, // InterPlan 10
     {3, 1, 20, 30, 0},   // Lunar Probe 11
     {5, 53, 19, 58, 0},  // Docking 12
     {91, 132, 118, 166, 15}, // Vostok 13
     {120, 132, 148, 167, 15}, // Voshod 14
     {176, 132, 211, 194, 15}, // Soyuz 15
     {249, 132, 275, 193, 11}, // MiniSh 16
     {213, 132, 247, 196, 11}, // Four Cap 17
     {25, 1, 55, 36, 0},  // Two Lem 18
     {57, 1, 84, 37, 0},  // One Lem 19
     {24, 39, 36, 72, 0}, // KicA 20
     {38, 39, 58, 75, 0}, // KicB 21
     {60, 39, 84, 94, 0}, // KicC 22
     {150, 132, 174, 177, 22}, // Zond 23
     {0, 0, 0, 0, 0},     // Filler Sm
     {24, 77, 48, 183, 0}, // Filler Lg
     {3, 118, 22, 147, 0}, // smShroud
     {149, 97, 190, 124, 0} // LgShroud
};
*/


void LoadMIVals();
int ChkDelVab(char plr, char f);
int ChkVabRkt(char plr, int rk, int *q);
void GradRect2(int x1, int y1, int x2, int y2, char plr);
void DispVAB(char plr, char pad);
void FreeMissionHW(char plr, char mis);
int FillVab(char plr, char f, char mode);
int  BuyVabRkt(char plr, int rk, int *q, char mode);
void ShowAutopurchase(char plr, int payload, int rk, int *qty);
void ShowVA(char f);
void ShowRkt(char *Name, int sf, int qty, char mode, char isDmg);
void DispVA(char plr, char f);
void DispRck(char plr, char wh);
void DispWts(int two, int one);
void LMAdd(char plr, char prog, char kic, char part);
void VVals(char plr, char tx, Equipment *EQ, char v4, char v5);


/* Load the coordinates of vehicle hardware components' images into the
 * MI global variable.
 *
 * Hardware components (capsules, rockets, etc.) have images used to
 * create a mock-up of the hardware assigned to the mission. This
 * includes a display of the rocket and a cut-away illustration of the
 * payload contained within the rocket casing. These component images
 * are stored together in a pair of larger VAB sprites. The global
 * variable MI stores the coordinates specifying where to find each
 * component in the VAB sprite.
 */
void LoadMIVals()
{
    size_t MI_size = sizeof(struct MDA) * 28 * 2;

    FILE *file = sOpen("VTABLE.DAT", "rb", 0);
    fread(MI, MI_size, 1, file);
    fclose(file);

    // Endianness swap
    for (int i = 0; i < 2 * 28; i++) {
        Swap16bit(MI[i].x1);
        Swap16bit(MI[i].y1);
        Swap16bit(MI[i].x2);
        Swap16bit(MI[i].y2);
        Swap16bit(MI[i].yOffset);
    }
}


void GradRect2(int x1, int y1, int x2, int y2, char plr)
{
    register int i, j, val;

    val = 3 * plr + 6;

    fill_rectangle(x1, y1, x2, y2, 7 + 3 * plr);

    for (j = x1; j <= x2; j += 3) {
        for (i = y1; i <= y2; i += 3) {
            display::graphics.legacyScreen()->pixels()[j + 320 * i] = val;
        }
    }

    return;
}


/* Draw the Vehicle Assembly / Integration interface layout and print
 * mission-specific information.
 *
 * This loads either the USA or USSR data from VAB.IMG into the global
 * buffer vhptr.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \param pad  The launch pad to which the mission is assigned.
 */
void DispVAB(char plr, char pad)
{
    uint16_t image_len = 0;

    helpText = "i016";
    keyHelpText = "k016";

    FadeOut(2, 10, 0, 0);

    {
        FILE *fp = sOpen("VAB.IMG", "rb", 0);
        display::AutoPal p(display::graphics.legacyScreen());
        fread(p.pal, 768, 1, fp);
        fread_uint16_t(&image_len, 1, fp);

        if (plr == 1) {
            fseek(fp, image_len, SEEK_CUR);
            fread(p.pal, 768, 1, fp);
            fread_uint16_t(&image_len, 1, fp);
        }

        fread(display::graphics.legacyScreen()->pixels(), image_len, 1, fp);
        fclose(fp);
    }

    PCX_D(display::graphics.legacyScreen()->pixels(), vhptr->pixels(), image_len);
    vhptr->palette().copy_from(display::graphics.legacyScreen()->palette());

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 170, 99);
    ShBox(0, 101, 170, 199);
    ShBox(172, 24, 319, 199);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    IOBox(175, 183, 244, 197);

    // Disable the Scrub button if there is no mission.
    if (Data->P[plr].Mission[pad].MissionCode) {
        IOBox(247, 183, 316, 197);
    } else {
        InBox(247, 183, 316, 197);
    }

    InBox(4, 104, 166, 123);
    IOBox(62, 127, 163, 177);
    IOBox(62, 179, 163, 193);

    InBox(177, 28, 314, 180);
    fill_rectangle(178, 29, 313, 179, 3);

    IOBox(4, 84, 165, 96);

    display::graphics.setForegroundColor(9);
    draw_string(200, 192, "E");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "XIT");
    display::graphics.setForegroundColor(9);
    draw_string(268, 192, "S");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "CRUB");
    draw_string(263, 13, "ASSIGN");
    display::graphics.setForegroundColor(9);
    draw_string(18, 136, "P");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "RIMARY:");
    draw_string(24, 148, "KICKER:");
    draw_string(42, 160, "L.M.:");
    draw_string(16, 172, "PAYLOAD:");
    display::graphics.setForegroundColor(9);
    draw_string(22, 188, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "OCKET:     ");

    Name[0] = 'A' + pad;
    Name[1] = 0x00;
    InBox(4, 27, 166, 37);
    fill_rectangle(5, 28, 165, 36, 10);
    display::graphics.setForegroundColor(11);
    draw_string(38, 34, "LAUNCH FACILITY: ");
    draw_string(0, 0, Name);
    display::graphics.setForegroundColor(1);

    if (plr == 0) {
        draw_heading(42, 4, "VEHICLE ASSEMBLY", 0, -1);
    } else {
        draw_heading(37, 4, "VEHICLE INTEGRATION", 0, -1);
    }

    display::graphics.setForegroundColor(5);
    draw_string(5, 45, "MISSION: ");
    draw_string(0, 0, Data->P[plr].Mission[pad].Name);

    display::graphics.setForegroundColor(1);
    draw_string(5, 61, "CREW: ");

    switch (Data->P[plr].Mission[pad].Men) {
    case 0:
        draw_string(0, 0, "UNMANNED");
        break;

    case 1:
        draw_string(0, 0, "ONE PERSON");
        break;

    case 2:
        draw_string(0, 0, "TWO PEOPLE");
        break;

    case 3:
        draw_string(0, 0, "THREE PEOPLE");
        break;

    case 4:
        draw_string(0, 0, "FOUR PEOPLE");
        break;
    }

    fill_rectangle(5, 105, 165, 122, 7 + plr * 3);
    display::graphics.setForegroundColor(11);
    draw_string(40, 111, "MISSION HARDWARE:");
    draw_string(10, 119, "SELECT PAYLOADS AND BOOSTER");

    display::graphics.setForegroundColor(1);

    GetMisType(Data->P[plr].Mission[pad].MissionCode);

    draw_string(5, 52, Mis.Abbr);

    int MisCod;
    MisCod = Data->P[plr].Mission[pad].MissionCode;

    // Show duration level only on missions with a Duration step - Leon
    if ((MisCod > 24 && MisCod < 32) || MisCod == 33 || MisCod == 34 || MisCod == 35 || MisCod == 37 || MisCod == 40 || MisCod == 41) {
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

    draw_small_flag(plr, 4, 4);

    return;
}


/* Frees hardware for other use that was assigned to a planned mission.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \param mis  The index of the mission (pad 0, 1, or 2).
 */
void FreeMissionHW(const char plr, const char mis)
{
    if (Data->P[plr].Mission[mis].Hard[Mission_PrimaryBooster] <= 0) {
        return;
    }

    for (int i = Mission_Capsule; i <= Mission_Probe_DM; i++) {
        switch (i) {
        case Mission_Capsule:
        case Mission_LM:  // Manned+LM
            Data->P[plr].Manned[Data->P[plr].Mission[mis].Hard[i]].Spok--;
            break;

        case Mission_Kicker:  // Kicker
            Data->P[plr].Misc[Data->P[plr].Mission[mis].Hard[i]].Spok--;
            break;

        case Mission_Probe_DM:  // DM+Probes
            if (Data->P[plr].Mission[mis].Hard[i] == MISC_HW_DOCKING_MODULE) {
                Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Spok--;
            } else {
                Data->P[plr].Probe[ Data->P[plr].Mission[mis].Hard[i]].Spok--;
            }

            break;
        }
    }

    Data->P[plr].Rocket[(Data->P[plr].Mission[mis].Hard[Mission_PrimaryBooster] - 1) % 4].Spok--;

    if (Data->P[plr].Mission[mis].Hard[Mission_PrimaryBooster] > 3) {
        Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok--;
    }
}


/* Calculate the cost of autopurchasing all of the missing hardware
 * components to create the given payload.
 *
 * If the mode argument is set to true, it will proceed to acquire
 * the missing hardware, spending the player's cash accordingly
 * (even if they do not have enough). This will not initiate any
 * hardware programs which haven't been started.
 *
 * This function does not ensure the player has sufficient cash on
 * hand to make the purchases.
 *
 * \param  plr  The player assembling the hardware.
 * \param  f    The VAS index of the given payload hardware set.
 * \param  mode 1 to auto-purchase missing components, 0 otherwise.
 * \return      The total cost of all components that will have to be
 *              purchased (0 if auto-purchasing).
 */
int FillVab(char plr, char f, char mode)
{
    int i, cost;
    cost = 0;

    for (i = 0; i < 4; i++) {
        if ((VAS[f][i].qty - VAS[f][i].ac) <= 0 && strncmp(VAS[f][i].name, "NONE", 4) != 0) {
            switch (i) {
            case 0:
            case 2:
                if ((Data->P[plr].Manned[VAS[f][i].dex].Num - Data->P[plr].Manned[VAS[f][i].dex].Spok) == 0) {
                    int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Manned[VAS[f][i].dex].UnitCost / 2) : Data->P[plr].Manned[VAS[f][i].dex].UnitCost;

                    if (mode == 1) {
                        Data->P[plr].Cash -= temp_cost;
                        Data->P[plr].Manned[VAS[f][i].dex].Num++;
                    } else {
                        cost += temp_cost;
                    }
                }

                break;

            case 1:
                if ((Data->P[plr].Misc[VAS[f][i].dex].Num - Data->P[plr].Misc[VAS[f][i].dex].Spok) == 0) {
                    int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Misc[VAS[f][i].dex].UnitCost / 2) : Data->P[plr].Misc[VAS[f][i].dex].UnitCost;

                    if (mode == 1) {
                        Data->P[plr].Cash -= temp_cost;
                        Data->P[plr].Misc[VAS[f][i].dex].Num++;
                    } else {
                        cost += temp_cost;
                    }
                }

                break;

            case 3:
                if (VAS[f][i].dex != 4) {
                    if ((Data->P[plr].Probe[VAS[f][i].dex].Num - Data->P[plr].Probe[VAS[f][i].dex].Spok) == 0) {
                        int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Probe[VAS[f][i].dex].UnitCost / 2) : Data->P[plr].Probe[VAS[f][i].dex].UnitCost;

                        if (mode == 1) {
                            Data->P[plr].Cash -= temp_cost;
                            Data->P[plr].Probe[VAS[f][i].dex].Num++;
                        } else {
                            cost += temp_cost;
                        }
                    }
                } else {
                    if ((Data->P[plr].Misc[VAS[f][i].dex].Num - Data->P[plr].Misc[VAS[f][i].dex].Spok) == 0) {
                        int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Misc[VAS[f][i].dex].UnitCost / 2) : Data->P[plr].Misc[VAS[f][i].dex].UnitCost;

                        if (mode == 1) {
                            Data->P[plr].Cash -= temp_cost;
                            Data->P[plr].Misc[VAS[f][i].dex].Num++;
                        } else {
                            cost += temp_cost;
                        }
                    }
                }

                break;
            }
        }
    }

    return cost;
}


/* Checks to see if any of the payload hardware is already fully
 * assigned to other missions (or not on hand) and subject to a delay
 * preventing it frum being autopurchased.
 *
 * \param  plr  The player assembling the hardware.
 * \param  f    The VAS index of the given payload hardware set.
 * \return      0 if stopped by delay, 1 otherwise.
 */
int ChkDelVab(char plr, char f)
{
    int i;

    for (i = 0; i < 4; i++) {
        if ((VAS[f][i].qty - VAS[f][i].ac) <= 0 && strcmp(VAS[f][i].name, "NONE") != 0) {
            switch (i) {
            case 0:
            case 2:
                if (Data->P[plr].Manned[VAS[f][i].dex].Delay != 0) {
                    return 0;
                }

                break;

            case 1:
                if (Data->P[plr].Misc[VAS[f][i].dex].Delay != 0) {
                    return 0;
                }

                break;

            case 3:
                if (VAS[f][i].dex != 4) {
                    if (Data->P[plr].Probe[VAS[f][i].dex].Delay != 0) {
                        return 0;
                    }
                } else {
                    if (Data->P[plr].Misc[VAS[f][i].dex].Delay != 0) {
                        return 0;
                    }
                }

                break;
            }
        }
    }

    return 1;
}


/* Calculate the cost of autopurchasing the specified missing rocket.
 *
 * If the mode argument is set, it will proceed to acquire the
 * missing rocket, subtracting the cost from the player's available
 * cash. This will not initiate any new rocket program which hasn't
 * been started.
 *
 * This function does not ensure the player has sufficient cash on
 * hand to make the purchase, or that the program has been initiated.
 *
 * \param plr    The player assembling the hardware.
 * \param rk     The index of the rocket hardware chosen.
 * \param q      Tracks how many of each rocket is already purchased.
 * \return       The cost of the rocket, 0 if it was purchased.
 */
int BuyVabRkt(char plr, int rk, int *q, char mode)
{
    int cost = 0;

    if ((Data->P[plr].Rocket[rk % 4].Num - Data->P[plr].Rocket[rk % 4].Spok) == 0) {
        int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Rocket[rk % 4].UnitCost / 2) : Data->P[plr].Rocket[rk % 4].UnitCost;

        if (mode == 1) {
            Data->P[plr].Cash -= temp_cost;
            Data->P[plr].Rocket[rk % 4].Num++;
            q[rk % 4]++;
        } else {
            cost = temp_cost;
        }
    }

    if (rk > 3 && (Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Num - Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok) == 0) {
        int temp_cost = Data->P[plr].TurnOnly == 3 ? MAX(1, Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].UnitCost / 2) : Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].UnitCost;

        if (mode == 1) {
            Data->P[plr].Cash -= temp_cost;
            Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Num++;
            q[rk]++;
        } else {
            cost += temp_cost;
        }

    }

    return cost;
}


/* Checks to see if a rocket is unavailable due to a delay in receiving
 * purchases.
 *
 * \param plr  The player assembling the mission hardware.
 * \param rk   The rocket index per EquipRocketIndex, +4 if boosters added.
 * \param q    An array of the quantity of rockets already purchased.
 * \return     0 if the rocket cannot be purchased, 1 otherwise.
 */
int ChkVabRkt(char plr, int rk, int *q)
{
    if (Data->P[plr].Rocket[rk % 4].Delay != 0 && q[rk] == 0) {
        return 0;
    }

    if (rk > 3 && Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Delay != 0 && q[rk] == 0) {
        return 0;
    }

    return 1;
}


/* Prints the cost of autopurchasing the missing components of the
 * given payload. It prints the cost compared to the player's available
 * funds (XX of YY) on the autopurchase button.
 *
 * \param plr      The player assembling the hardware.
 * \param payload  The VAS index of the given payload hardware set.
 * \param rk       The index of the rocket hardware chosen.
 * \param qty      An array of the quantity of rockets available.
 */
void ShowAutopurchase(const char plr, const int payload, const int rk,
                      int *qty)
{
    int hasDelay, cost;

    hasDelay = ChkDelVab(plr, payload) || ChkVabRkt(plr, rk, &qty[0]);
    cost = FillVab(plr, payload, 0) + BuyVabRkt(plr, rk, &qty[0], 0);

    fill_rectangle(7, 87, 162, 93, 3);
    display::graphics.setForegroundColor(9);
    draw_string(13, 92, "A");
    display::graphics.setForegroundColor(1);

    //if can't buy (delay, cost>cash) ->red letters
    if (hasDelay == 0 || cost > Data->P[plr].Cash) {
        display::graphics.setForegroundColor(9);
    }

    draw_string(0, 0, "UTOPURCHASE (");
    draw_number(0, 0, cost);
    draw_string(0, 0, " OF ");
    draw_megabucks(0, 0, Data->P[plr].Cash);
    draw_string(0, 0, ")");
}


/* Prints the payload components for the selected payload configuration.
 *
 * The text is printed on the Mission Hardware button found in the
 * lower left corner of the main Vehicle Assembly screen.
 *
 * \param  f   The VAS index of the given payload hardware set.
 */
void ShowVA(char f)
{
    int i;

    fill_rectangle(65, 130, 160, 174, 3);
    display::graphics.setForegroundColor(1);

    for (i = 0; i < 4; i++) {
        if (VAS[f][i].qty < 0) {
            display::graphics.setForegroundColor(9);
        } else {
            display::graphics.setForegroundColor(1);
        }

        draw_string(67, 136 + 12 * i, &VAS[f][i].name[0]);

        if ((VAS[f][i].qty - VAS[f][i].ac) < 0) {
            draw_number(152, 136 + 12 * i, 0);
            draw_number(128, 136 + 12 * i, 0);
        } else {
            draw_number(152, 136 + 12 * i, VAS[f][i].qty - VAS[f][i].ac);

            if (VAS[f][i].dmg) {
                display::graphics.setForegroundColor(9);
            }

            draw_number(128, 136 + 12 * i, VAS[f][i].sf);
        }

        draw_string(0, 0, "%");
    }

    return;
}


/* Prints the selected rocket's name, safety factor, and quantity.
 *
 * This adds the text to the small rocket selection button in the
 * Mission Hardware area in the lower-left quadrant of the VAB screen.
 *
 * \param Name   The name of the rocket program.
 * \param sf     The safety factor of the rocket program.
 * \param qty    The number of unassigned rockets of the program.
 * \param mode   1 if the rocket cannot lift the current payload
 * \param isDmg  1 if the safety factor of the rocket is reduced
 */
void ShowRkt(char *Name, int sf, int qty, char mode, char isDmg)
{
    fill_rectangle(65, 182, 160, 190, 3);

    if (qty < 0 || mode == 1) {
        display::graphics.setForegroundColor(9);
    } else {
        display::graphics.setForegroundColor(1);
    }

    draw_string(67, 188, &Name[0]);

    if (qty < 0) {
        draw_number(152, 188, 0);
        draw_number(128, 188, 0);
    } else {
        draw_number(152, 188, qty);

        if (isDmg) {
            display::graphics.setForegroundColor(9);
        }

        draw_number(128, 188, sf);
    }

    draw_string(0, 0, "%");

    return;
}


/* Draw the launch vehicle illustration in the Vehicle Assemble mock-up
 * screen. Depending on the payload cargo, an appropriate casing is
 * chosen and the payload components are rendered along the length of
 * the casing, as per a cut-out illustration, in order of
 * Primary (capsule), LM, Kicker, and Payload.
 * For larger casings, the remaining space is occupied by a filler image
 * depicting the rocket's fuel supply.
 *
 * This function makes extensive use of two global variables, VAS and
 * MI.
 *
 * \param plr      The assembling player (0 for USA, 1 for USSR).
 * \param payload  The VAS index of the given payload hardware set.
 */
void DispVA(char plr, char payload)
{
    int i, TotY, IncY;
    int casingWidth, casingHeight, x1, y1, x2, y2, w2, h2, cx, off = 0;
    uint8_t casing, images, img;

    images = 0; /**< number of pictures */

    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 0) {
            images++;
        }
    }

    /* casing: 7 - casing small, 8 - casing large */
    casing = ((images == 1 && VAS[payload][3].img > 0) || images == 0) ?
             7 : 8;

    // The Mercury capsule has a tower that sticks through the top of
    // the casing. On the large casing image, there isn't space to
    // display this, so the smaller casing image is forced.
    if (VAS[payload][0].img == 13 && plr == 0) { // Mercury capsule
        casing = 7;
    }

    /* TotY: sum of height of all payload images */
    TotY = 0;

    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 1) {
            TotY += MI[plr * 28 + VAS[payload][i].img].y2 -
                    MI[plr * 28 + VAS[payload][i].img].y1 + 1;
        }
    }

    /* Load proper (casing) background into buffer */
    x1 = MI[plr * 28 + casing].x1;
    y1 = MI[plr * 28 + casing].y1;
    x2 = MI[plr * 28 + casing].x2;
    y2 = MI[plr * 28 + casing].y2;
    casingWidth = x2 - x1 + 1;
    casingHeight = y2 - y1 + 1;

    if (plr == 0 && VAS[payload][0].img == 13) {
        casingHeight += 13;
        off = 13;
    }

    display::LegacySurface local(casingWidth, casingHeight);

    local.clear(0);

    local.copyFrom(vhptr, x1, y1, x2, y2, 0, 0 + off);

    /* Copy area background into buffer underneath casing */
    display::LegacySurface local2(casingWidth, casingHeight);

    fill_rectangle(178, 29, 243, 179, 3); /* TODO: magic numbers */

    local2.copyFrom(display::graphics.legacyScreen(),
                    210 - casingWidth / 2,
                    103 - casingHeight / 2,
                    210 - casingWidth / 2 + casingWidth - 1,
                    103 - casingHeight / 2 + casingHeight - 1);

    local.maskCopy(&local2, 0, display::LegacySurface::DestinationEqual);

    local2.clear(0);

    /* IncY is the y-axis increment for drawing each of the payload
     * component images. If there is no Primary (capsule) image, the
     * payload components are centered. If there is a capsule, they
     * start at the top, plus the component's offset.
     *
     * Mercury capsules, as usual, start at the very top with extra
     * space alloted for that big tower.
     */
    IncY = (casingHeight - TotY) / 2;

    if (VAS[payload][0].img > 0) {
        if (VAS[payload][0].img == 13 && plr == 0) {
            IncY = 0;
        } else {
            IncY = MI[plr * 28 + VAS[payload][0].img].yOffset;
        }
    }

    /* Draw each of the component images into the local2 buffer */
    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 0) {
            img = VAS[payload][i].img;
            x1 = MI[plr * 28 + img].x1;
            y1 = MI[plr * 28 + img].y1;
            x2 = MI[plr * 28 + img].x2;
            y2 = MI[plr * 28 + img].y2;
            w2 = x2 - x1 + 1;
            h2 = y2 - y1 + 1;
            cx = casingWidth / 2 - w2 / 2 - 1; // Center on x-axis

            if (cx + w2 > casingWidth || IncY + h2 > casingHeight) {
                CWARNING3(graphic, "can't fit %s image into spaceship casing!",
                          VAS[payload][i].name);
                continue;
            } else {
                local2.copyFrom(vhptr, x1, y1, x2, y2, cx, IncY);
            }

            IncY += h2 + 1;
        }
    }

    local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);

    // Overlay the shroud to give the appearance of a cutaway.
    // The shroud image is overlaid on the bottom of the casing image.
    // If using a large casing, fill the unused space at the bottom
    // with fiery rocket power!
    if (casing == 8) {
        x1 = MI[plr * 28 + 25].x1;
        y1 = MI[plr * 28 + 25].y1;
        x2 = MI[plr * 28 + 25].x2;
        y2 = MIN(y1 + (casingHeight - IncY - 1), MI[plr * 28 + 25].y2);
        w2 = x2 - x1 + 1;
        cx = casingWidth / 2 - w2 / 2 - 1;
        local2.copyFrom(vhptr, x1, y1, x2, y2, cx, IncY);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);

        x1 = MI[plr * 28 + 27].x1;
        y1 = MI[plr * 28 + 27].y1;
        x2 = MI[plr * 28 + 27].x2;
        y2 = MI[plr * 28 + 27].y2;
        h2 = y2 - y1 + 1;

        local2.copyFrom(vhptr, x1, y1, x2, y2, 0, casingHeight - h2);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);
    } else {
        // There is no small filler defined, so skip to the shroud.
        x1 = MI[plr * 28 + 26].x1;
        y1 = MI[plr * 28 + 26].y1;
        x2 = MI[plr * 28 + 26].x2;
        y2 = MI[plr * 28 + 26].y2;
        h2 = y2 - y1 + 1;
        local2.copyFrom(vhptr, x1, y1, x2, y2, 0, casingHeight - h2);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);
    }

    local.copyTo(display::graphics.legacyScreen(),
                 210 - casingWidth / 2, 103 - casingHeight / 2);
}


/* Draw the rocket illustration in the Vehicle Assembly mock-up screen.
 *
 * The Rocket graphics, as with other VAB images, are stored in a
 * texture atlas, with the global variable MI storing the texture
 * coordinates.
 *
 * This function makes extensive use of two global variables, MI and
 * vhptr. It relies upon the VAB sprite having been loaded into the
 * global vhptr buffer.
 *
 * The rockets are indexed in the MI[] array as:
 *   0 / 28:   Atlas  / A-Series
 *   1 / 29:   TItan  / Proton
 *   2 / 30:   Saturn / N-1
 *   3 / 31:   Nova   / Energia
 *   4 / 32:   Atlas + Boosters  / A-Series + Boosters
 *   5 / 33:   Titan + Boosters  / Proton + Boosters
 *   6 / 34:   Saturn + Boosters / N-1 + Boosters
 *
 * \param plr  The assembling player (0 for USA, 1 for USSR).
 * \param wh   The rocket's index in the MI[] array.
 */
void DispRck(char plr, char wh)
{
    int w;
    int h;
    int x1;
    int y1;
    int x2;
    int y2;

    x1 = MI[plr * 28 + wh].x1;
    y1 = MI[plr * 28 + wh].y1;
    x2 = MI[plr * 28 + wh].x2;
    y2 = MI[plr * 28 + wh].y2;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    display::LegacySurface local(w, h);
    display::LegacySurface local2(w, h);

    local.copyFrom(vhptr, x1, y1, x2, y2, 0, 0);

    fill_rectangle(247, 29, 313, 179, 3);
    local2.copyFrom(display::graphics.legacyScreen(), 282 - w / 2, 103 - h / 2, 282 - w / 2 + w - 1, 103 - h / 2 + h - 1);

    local.maskCopy(&local2, 0, display::LegacySurface::DestinationEqual);

    local.copyTo(display::graphics.legacyScreen(), 282 - w / 2, 103 - h / 2);
}


/* Print the readout of the vehicle payload weight.
 *
 * This displays the current weight of the payload as well as the
 * maximum payload supported by the rocket currently assigned.
 *
 * \param two  The total weight of the current payload hardware.
 * \param one  The maximum payload weight for the current rocket.
 */
void DispWts(int two, int one)
{
    fill_rectangle(5, 65, 140, 83, 3);

    display::graphics.setForegroundColor(1);
    draw_string(5, 77, "MAXIMUM PAYLOAD: ");
    draw_number(0, 0, one);

    display::graphics.setForegroundColor(1);
    draw_string(5, 70, "CURRENT PAYLOAD: ");

    if (one < two) {
        display::graphics.setForegroundColor(9);
    }

    draw_number(0, 0, two);

    return;
}


void VAB(char plr)
{
    int ccc, rk;               // Payload index & rocket index
    int mis, wgt, cwt, ab, ac;
    int sf[8], qty[8], pay[8]; // Cached rocket safety, quantity, & thrust
    char Name[8][12];          // Cached rocket names
    char ButOn;

    LoadMIVals();
    music_start(M_HARDWARE);

    // FutureCheck brings up the Launch Pad menu, displaying the
    // missions assigned to each of the pads, and allows the player
    // to select a launch pad (or exit). It returns the chosen pad
    // index (or exit code).
    while ((mis = FutureCheck(plr, 1)) < MAX_MISSIONS) {

        // If a manned mission's Primary & Backup flight crews are
        // unavailable, scrub the mission.
        if (CheckCrewOK(plr, mis) == 1) { // found mission no crews
            ClrMiss(plr, mis + 3);
            continue;
        }

        helpText = "i016";

        // When reassembling Hardware, any hardware previously assigned to
        // the mission should be unassigned so it may be used (or not) in
        // reassembly.
        FreeMissionHW(plr, mis);

        BuildVAB(plr, mis, 0, 0, 0); // now holds the mission info

        // Rocket Display Data --------------------------
        for (int i = 0; i < 7; i++) {
            if (i > 3) {
                isDamaged[i] = Data->P[plr].Rocket[i - 4].Damage != 0 ? 1 : 0;
                sf[i] = RocketBoosterSafety(Data->P[plr].Rocket[i - 4].Safety, Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Safety);
                strcpy(&Name[i][0], "B/");
                strcat(&Name[i][0], &Data->P[plr].Rocket[i - 4].Name[0]);
                qty[i] = Data->P[plr].Rocket[i - 4].Num - Data->P[plr].Rocket[i - 4].Spok;
                int tmp = Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Num - Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok;

                if (tmp < qty[i]) {
                    qty[i] = tmp;
                }

                pay[i] = (Data->P[plr].Rocket[i - 4].MaxPay + Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].MaxPay);
            } else {
                isDamaged[i] = Data->P[plr].Rocket[i].Damage != 0 ? 1 : 0;
                sf[i] = Data->P[plr].Rocket[i].Safety;
                strcpy(&Name[i][0], &Data->P[plr].Rocket[i].Name[0]);
                qty[i] = Data->P[plr].Rocket[i].Num - Data->P[plr].Rocket[i].Spok;
                pay[i] = Data->P[plr].Rocket[i].MaxPay;
            }
        }

        DispVAB(plr, mis);

        if (Data->P[plr].Mission[mis].MissionCode) {
            ButOn = 1;
        } else {
            ButOn = 0;
            InBox(245, 5, 314, 17);
        }

        wgt = 0;

        for (int i = 0; i < 4; i++) {
            wgt += VAS[1][i].wt;
        }

        rk = 0;

        while (pay[rk] < wgt) {
            rk++;
        }

        ccc = 1;
        ShowVA(ccc);
        ShowRkt(&Name[rk][0], sf[rk], qty[rk], pay[rk] < wgt, isDamaged[rk]);
        DispRck(plr, rk);
        DispVA(plr, ccc);
        cwt = 0;

        for (int i = 0; i < 4; i++) {
            cwt += VAS[ccc][i].wt;
        }

        DispWts(cwt, pay[rk]);
        //display cost (XX of XX)
        ShowAutopurchase(plr, ccc, rk, &qty[0]);

        FadeIn(2, 10, 0, 0);
        WaitForMouseUp();

        while (1) {
            key = 0;
            GetMouse();

            if (mousebuttons <= 0 && key <= 0) {
                continue;
            }

            // Game Play
            if ((x >= 6 && y >= 86 && x <= 163 && y <= 94 && mousebuttons > 0) || key == 'A') {
                // AUTOPURCHASE
                InBox(6, 86, 163, 94);
                key = 0;
                // NEED A DELAY CHECK
                ac = ChkDelVab(plr, ccc);

                if (ac != 0) {
                    ac = ChkVabRkt(plr, rk, &qty[0]);
                }

                ab = FillVab(plr, ccc, 0);
                ab += BuyVabRkt(plr, rk, &qty[0], 0);

                if (Data->P[plr].Cash >= ab && ac != 0) {
                    FillVab(plr, ccc, 1);
                    BuyVabRkt(plr, rk, &qty[0], 1);
                    BuildVAB(plr, mis, 0, 0, 1);

                    // Rocket Display Data --------------------------
                    for (int i = 0; i < 7; i++) {
                        if (i > 3) {
                            isDamaged[i] = Data->P[plr].Rocket[i - 4].Damage != 0 ? 1 : 0;
                            sf[i] = RocketBoosterSafety(Data->P[plr].Rocket[i - 4].Safety, Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Safety);
                            strcpy(&Name[i][0], "B/");
                            strcat(&Name[i][0], &Data->P[plr].Rocket[i - 4].Name[0]);
                            qty[i] = Data->P[plr].Rocket[i - 4].Num - Data->P[plr].Rocket[i - 4].Spok;
                            int tmp = Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Num - Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok;

                            if (tmp < qty[i]) {
                                qty[i] = tmp;
                            }

                            pay[i] = (Data->P[plr].Rocket[i - 4].MaxPay + Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].MaxPay);
                        } else {
                            isDamaged[i] = Data->P[plr].Rocket[i].Damage != 0 ? 1 : 0;
                            sf[i] = Data->P[plr].Rocket[i].Safety;
                            strcpy(&Name[i][0], &Data->P[plr].Rocket[i].Name[0]);
                            qty[i] = Data->P[plr].Rocket[i].Num - Data->P[plr].Rocket[i].Spok;
                            pay[i] = Data->P[plr].Rocket[i].MaxPay;
                        }
                    }

                    //display cost (XX of XX)
                    ShowAutopurchase(plr, ccc, rk, &qty[0]);
                } else if (ac == 0) {
                    Help("i135");    // delay on purchase
                } else {
                    Help("i137");    // not enough money
                }

                ShowVA(ccc);
                ShowRkt(&Name[rk][0], sf[rk], qty[rk], pay[rk] < wgt, isDamaged[rk]);
                OutBox(6, 86, 163, 94);
            } else if ((x >= 177 && y >= 185 && x <= 242 && y <= 195 && mousebuttons > 0) || (key == K_ESCAPE || key == 'E')) {
                // CONTINUE/EXIT/DO NOTHING
                InBox(177, 185, 242, 195);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(177, 185, 242, 195);

                // Clear mission hardware
                for (int i = Mission_Capsule; i <= Mission_PrimaryBooster; i++) {
                    Data->P[plr].Mission[mis].Hard[i] = 0;
                }

                break;
            } else if (((x >= 249 && y >= 185 && x <= 314 && y <= 195 && mousebuttons > 0) || key == 'S') && Data->P[plr].Mission[mis].MissionCode) {
                // SCRUB The whole mission
                InBox(249, 185, 314, 195);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(249, 185, 314, 195);
                ClrMiss(plr, mis);
                break;
            } else if (((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) && ccc != 0 && ButOn == 1 && cwt <= pay[rk]) {
                int j = 0;

                if (Mis.EVA == 1 && Data->P[plr].Misc[MISC_HW_EVA_SUITS].Num == PROGRAM_NOT_STARTED) {
                    Help("i118");
                } else if (Mis.Doc == 1 && Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num == PROGRAM_NOT_STARTED) {
                    Help("i119");
                } else {
                    if ((Mis.mVab[0] & 0x10) == 0x10 && Data->P[plr].DockingModuleInOrbit <= 0) {
                        Help("i155");    // No docking module in orbit
                    }

                    int j2 = 0;

                    if (strncmp(VAS[ccc][0].name, "NONE", 4) != 0) {
                        j++;

                        if ((Data->P[plr].Manned[VAS[ccc][0].dex].Num - Data->P[plr].Manned[VAS[ccc][0].dex].Spok) > 0) {
                            j2++;
                        }
                    }

                    if (strncmp(VAS[ccc][1].name, "NONE", 4) != 0) {
                        j++;

                        if ((Data->P[plr].Misc[VAS[ccc][1].dex].Num - Data->P[plr].Misc[VAS[ccc][1].dex].Spok) > 0) {
                            j2++;
                        }
                    }

                    if (strncmp(VAS[ccc][2].name, "NONE", 4) != 0) {
                        j++;

                        if ((Data->P[plr].Manned[VAS[ccc][2].dex].Num - Data->P[plr].Manned[VAS[ccc][2].dex].Spok) > 0) {
                            j2++;
                        }
                    }

                    if (strncmp(VAS[ccc][3].name, "NONE", 4) != 0) {
                        j++;

                        if (((Data->P[plr].Probe[VAS[ccc][3].dex].Num - Data->P[plr].Probe[VAS[ccc][3].dex].Spok) > 0)
                            || ((Data->P[plr].Misc[VAS[ccc][3].dex].Num - Data->P[plr].Misc[VAS[ccc][3].dex].Spok) > 0)) {
                            j2++;
                        }
                    }

                    j++;

                    if (qty[rk] > 0) {
                        j2++;
                    }

                    if (j == j2) {
                        InBox(245, 5, 314, 17);
                        WaitForMouseUp();

                        if (key > 0) {
                            delay(150);
                        }

                        OutBox(245, 5, 314, 17);

                        for (int i = Mission_Capsule; i <= Mission_Probe_DM; i++) {
                            Data->P[plr].Mission[mis].Hard[i] = VAS[ccc][i].dex;

                            if (VAS[ccc][i].dex >= 0) {
                                switch (i) {
                                case Mission_Capsule:
                                case Mission_LM:  // Manned+LM
                                    Data->P[plr].Manned[VAS[ccc][i].dex].Spok++;
                                    break;

                                case Mission_Kicker:  // Kicker
                                    Data->P[plr].Misc[VAS[ccc][i].dex].Spok++;
                                    break;

                                case Mission_Probe_DM:  // DM+Probes
                                    if (VAS[ccc][i].dex == 4) {
                                        Data->P[plr].Misc[Mission_PrimaryBooster].Spok++;
                                    } else {
                                        Data->P[plr].Probe[VAS[ccc][i].dex].Spok++;
                                    }

                                    break;
                                }
                            }
                        }

                        Data->P[plr].Mission[mis].Hard[Mission_PrimaryBooster] = rk + 1;
                        Data->P[plr].Rocket[rk % 4].Spok++;

                        if (rk > 3) {
                            Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok++;
                        }

                        break;
                    }
                }
            } else if ((x >= 64 && y >= 181 && x <= 161 && y <= 191 && mousebuttons > 0) || key == 'R') {
                // Choose Rocket
                InBox(64, 181, 161, 191);
#define Misdef(a)     Data->P[plr].Mission[(a)].MissionCode
                rk++;

                if (rk > 6) {
                    rk = 0;
                }

                if (((Misdef(mis) >= 42 && Misdef(mis) <= 57) ||
                     (Misdef(mis) >= 7 && Misdef(mis) <= 13)) &&
                    (rk == 4 || rk == 0)) {
                    if (options.cheat_altasOnMoon == 0) {
                        rk++;
                    }
                }

                //display cost (XX of XX)
                ShowAutopurchase(plr, ccc, rk, &qty[0]);
                ShowRkt(&Name[rk][0], sf[rk], qty[rk], pay[rk] < wgt, isDamaged[rk]);
                DispWts(cwt, pay[rk]);
                DispRck(plr, rk);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(64, 181, 161, 191);
            } else if ((x >= 64 && y >= 129 && x <= 161 && y <= 175 && mousebuttons > 0) || key == 'P') {
                // Cycle through payload selections
                InBox(64, 129, 161, 175);
                ccc++;

                if (ccc > VASqty) {
                    ccc = 0;
                }

                cwt = 0;

                for (int i = 0; i < 4; i++) {
                    cwt += VAS[ccc][i].wt;
                }

                ShowVA(ccc);
                DispWts(cwt, pay[rk]);
                DispVA(plr, ccc);
                //display cost (XX of XX)
                ShowAutopurchase(plr, ccc, rk, &qty[0]);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(64, 129, 161, 175);
            }
        }
    }

    Vab_Spot = (Data->P[plr].Mission[0].Hard[Mission_PrimaryBooster] > 0) ? 1 : 0;
    music_stop();
}


//----------------------------------------------------------------------
// VAB Autobuild Functions
//----------------------------------------------------------------------


/* Generates the set of possible distinct vehicle payloads for the
 * mision and stores the payload information in the global variable
 * VAS.
 *
 * This function is used by the AI, so beware of making changes without
 * understanding what the AI is doing!
 *
 * \param plr  The player assembling the launch vehichle.
 * \param mis  The pad the mission is on.
 * \param ty   >0 if called by the AI.
 * \param pa   0 if the primary half of a joint mission.
 * \param pr   The hardware program to use on mission (used by AI).
 */
void BuildVAB(char plr, char mis, char ty, char pa, char pr)
{
    char i, j, part, mcode, prog, ext = 0;
    unsigned char VX;

    memset(VAS, 0x00, sizeof VAS);

    if (ty == 0) {
        part = Data->P[plr].Mission[mis].part;
        mcode = Data->P[plr].Mission[mis].MissionCode;
        prog = Data->P[plr].Mission[mis].Prog - 1;

        if (part == 0) {
            if (Data->P[plr].Mission[mis].Joint == 1) {
                ext = Data->P[plr].Mission[mis + 1].Prog - 1;
            }
        } else {
            ext = prog;
        }
    } else {                              // ty>0 for AI code
        part = pa;                          // Mission Part to Check
        mcode = mis;                        // Mission to Check
        prog = ext = pr;                    // Manned Program to Check
    }

    GetMisType(mcode);

    VX = Mis.mVab[part];

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 4; j++) {
            strcpy(&VAS[i][j].name[0], "NONE");
            VAS[i][j].qty = VAS[i][j].sf = VAS[i][j].wt = VAS[i][j].dmg = 0;
            VAS[i][j].dex = VAS[i][j].img = -1;
        }
    }

    VASqty = 0;

    if (VX & 0x80) { // Capsule
        j = (part == 0) ? prog : ext;

        for (i = 1; i < 6; i++) { // Fill all parts with CAP
            VASqty++;
            VVals(plr, 0, &Data->P[plr].Manned[j], j, 13 + j);
        }
    }

    VASqty = 0;

    if (VX == 0x20 && part == 0 && mcode == 1) { // P:Sxx XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Probe[PROBE_HW_ORBITAL], 0, 9);
    }

    if (VX == 0x20 && part == 0 && mcode != 1) { // P:xDM XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
    } else if (VX == 0x04 && part == 0) { // P:INTER XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Probe[PROBE_HW_INTERPLANETARY], 1, 10);
    } else if (VX == 0x02 && part == 0) { // P:PRO XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Probe[PROBE_HW_LUNAR], 2, 11);
    } else if (VX == 0x60 && part == 0) { // P:LM+SDM XX
        LMAdd(plr, ext, -1, 1);
    } else if (VX == 0xe8 && part == 0) { // P:LM+SDM+EVA XX
        LMAdd(plr, prog, -1, 1);
    } else if (VX == 0x61 && part == 0) { // P:LM+SDM+KIC XX
        LMAdd(plr, ext, 0, 1);
        LMAdd(plr, ext, 1, 1);
    } else if (VX == 0x21 && part == 0) { // P:SDM+KIC-C XX
        VASqty++;
        VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_C], 2, 22);
    }

    else if (VX == 0x80) {
        VASqty = 1;    // P/S:CAP XX
    }

    else if (VX == 0x88) { // P/S:CAP+EVA XX
        VASqty = 1;
        // EVA Check
    }

    else if (VX == 0xa0 && part == 0) { // P:CAP+SDM XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
    }

    else if (VX == 0x90 && part == 0) { // P:CAP+DMO XX
        VASqty = 1; // DMO Check
    }

    else if (VX == 0xa8 && part == 0) { // P:CAP+SDM+EVA XX
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
        // EVA Check
    }

    else if (VX == 0x98 && part == 0) { // P:CAP+DMO+EVA XX
        VASqty = 1;
        // EVA Check
        // DMO Check
    }

    else if (VX == 0xe0 && part == 0) { // P:CAP+LM+SDM XX
        LMAdd(plr, prog, -1, 0);
    }

    else if (VX == 0x81) { // P/S:CAP+KIC XX
        if (prog == 1 || prog == 3) {
            if (mcode != 52) {  ///Special Case EOR LM Test
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_A], 0, 20);
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_B], 1, 21);
            } else {
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_B], 1, 20);
            }
        } else {
            VASqty = 1;
        }
    }

    else if (VX == 0xe1 && part == 0) { // P:CAP+LM+SDM+KIC XX
        LMAdd(plr, prog, 1, 0);
    }

    else if (VX == 0xe9 && part == 0) { // P:CAP+LM+SDM+EVA XX
        LMAdd(plr, prog, 1, 0);
        // EVA Check
    }

    else if (VX == 0x89 && part == 1) { // S:CAP+EVA+KIC
        if (prog != 2) {
            if (mcode != 55) { ///Special Case EOR Lunar Landing
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_A], 0, 20);
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_B], 1, 21);
            } else {
                VASqty++;
                VVals(plr, 1, &Data->P[plr].Misc[MISC_HW_KICKER_B], 1, 20);
            }
        } else {
            VASqty = 1;
        }

        // EVA Check
    }

    return;
}


void LMAdd(char plr, char prog, char kic, char part)
{
    if (prog == 1) {
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
        VVals(plr, 2, &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE], 6, 19);

        if (kic >= 0) {
            VVals(plr, 1, &Data->P[plr].Misc[kic], kic, 20 + kic);
        }

    }

    else if (prog == 2) {
        VASqty++;
        VVals(plr, 2, &Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE], 5, 18);

        if (part == 1 && kic >= 0) {
            VVals(plr, 1, &Data->P[plr].Misc[kic], kic, 20 + kic);
        }

        VASqty++;
        VVals(plr, 2, &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE], 6, 19);

        if (part == 1 && kic >= 0) {
            VVals(plr, 1, &Data->P[plr].Misc[kic], kic, 20 + kic);
        }
    }

    else if (prog == 3) { // Minishuttle
        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
        VVals(plr, 2, &Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE], 5, 18);

        if (kic >= 0) {
            VVals(plr, 1, &Data->P[plr].Misc[kic], kic, 20 + kic);
        }

        VASqty++;
        VVals(plr, 3, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], 4, 12);
        VVals(plr, 2, &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE], 6, 19);

        if (kic >= 0) {
            VVals(plr, 1, &Data->P[plr].Misc[kic], kic, 20 + kic);
        }
    }

    return;
}


void VVals(char plr, char tx, Equipment *EQ, char v4, char v5)
{
    strcpy(&VAS[VASqty][tx].name[0], &EQ->Name[0]);
    VAS[VASqty][tx].qty = EQ->Num;
    VAS[VASqty][tx].ac = EQ->Spok;
    VAS[VASqty][tx].wt = EQ->UnitWeight;

    if (tx == 3 && v4 == 4 && AI[plr] == 1) {
        VAS[VASqty][tx].sf = EQ->MSF;
    } else {
        VAS[VASqty][tx].sf = EQ->Safety;
    }

    VAS[VASqty][tx].dex = v4;
    VAS[VASqty][tx].img = v5;
    VAS[VASqty][tx].dmg = EQ->Damage != 0 ? 1 : 0;
    return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
