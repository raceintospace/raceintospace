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
//Equipment Randomization
//Enabling this option will change game balance, and can possibly break the AI

/* Unit Weight is based in the previous equipment (e.g. Gemini's weight is Mercury's + random number)
    some units have fixed ranges, loosely based on the Basic Model
Max Safety is a random number between 80 and 98
Max RD is 2 to 10 lower than Max Safety
Initial Cost is a random number, loosely based on the Basic Model initial cost
Unit Cost is loosely based on the Basic Model, but depends on initial cost
RD cost is loosely based on the Basic Model
*/
#include "randomize.h"

#include <cassert>
#include <cctype>

#include "display/graphics.h"

#include "Buzz_inc.h"
#include "draw.h"
#include "game_main.h"
#include "gamedata.h"
#include "gr.h"
#include "pace.h"
#include "sdlhelper.h"


/*random_number divides the randomization in 2,
 so there is a lower chance of getting an extreme */
/*random_min is used to give more chances of getting a 1 */


char GetNewName(char name[20]);


static inline int
random_aux(int x, int y)
{
    return (brandom((y) - (x) + 1) + (x));
}

static inline int
random_number(int x, int y)
{
    return (random_aux((x) / 2, ((y) + 1) / 2) + random_aux(((x) + 1) / 2, (y) / 2));
}

static inline int
random_min(int n)
{
    return (MAX(1, random_number(0, n)));
}


void RandomizeEq(void)
{
    int i;

    for (i = 0; i < 2; i++) {  // for each player
        //Satellite
        Data->P[i].Probe[PROBE_HW_ORBITAL].UnitWeight  = random_number(100, 500);
        Data->P[i].Probe[PROBE_HW_ORBITAL].MaxSafety   = random_number(86, 98);
        Data->P[i].Probe[PROBE_HW_ORBITAL].MaxRD   = Data->P[i].Probe[PROBE_HW_ORBITAL].MaxSafety - random_number(2, 10);
        Data->P[i].Probe[PROBE_HW_ORBITAL].InitCost    = random_number(3, 9);
        Data->P[i].Probe[PROBE_HW_ORBITAL].UnitCost    = random_min(Data->P[i].Probe[PROBE_HW_ORBITAL].InitCost / 2 - 1);
        Data->P[i].Probe[PROBE_HW_ORBITAL].RDCost  = random_min(2);

        //Lunar Satellite
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].UnitWeight  = Data->P[i].Probe[PROBE_HW_ORBITAL].UnitWeight + random_number(200, 600);
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].MaxSafety   = random_number(86, 98);
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].MaxRD   = Data->P[i].Probe[PROBE_HW_INTERPLANETARY].MaxSafety - random_number(2, 10);
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].InitCost    = random_number(18, 30);
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].UnitCost    = random_number(2, Data->P[i].Probe[PROBE_HW_INTERPLANETARY].InitCost / 4);
        Data->P[i].Probe[PROBE_HW_INTERPLANETARY].RDCost  = random_number(2, 4);

        //Lunar Probe
        Data->P[i].Probe[PROBE_HW_LUNAR].UnitWeight  = Data->P[i].Probe[PROBE_HW_INTERPLANETARY].UnitWeight + random_number(300, 700);
        Data->P[i].Probe[PROBE_HW_LUNAR].MaxSafety   = random_number(86, 98);
        Data->P[i].Probe[PROBE_HW_LUNAR].MaxRD   = Data->P[i].Probe[PROBE_HW_LUNAR].MaxSafety - random_number(2, 10);
        Data->P[i].Probe[PROBE_HW_LUNAR].InitCost    = random_number(20, 40);
        Data->P[i].Probe[PROBE_HW_LUNAR].UnitCost    = random_number(4, 2 + Data->P[i].Probe[PROBE_HW_LUNAR].InitCost / 5);
        Data->P[i].Probe[PROBE_HW_LUNAR].RDCost  = random_number(2, 6);

        //Mercury/Vostok
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].UnitWeight = random_number(400, 600);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].MaxRD  = Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].InitCost   = random_number(12, 24);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].UnitCost   = random_min(Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].InitCost / 4);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].RDCost = random_min(2);

        //Gemini/Voskhod
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].UnitWeight = Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].UnitWeight + random_number(500, 900);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD  = Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].InitCost   = random_number(18, 30);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].UnitCost   = random_number(4, Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].InitCost / 3);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].RDCost = random_number(1, 3);

        //Apollo/Soyuz
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].UnitWeight = Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].UnitWeight + random_number(300, 500);
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].MaxRD  = Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].InitCost   = random_number(28, 44);
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].UnitCost   = random_number(8, Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].InitCost / 2 - 2);
        Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].RDCost = random_number(3, 7);

        //Minishuttle
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].UnitWeight = Data->P[i].Manned[MANNED_HW_TWO_MAN_CAPSULE].UnitWeight + random_number(200, 400);
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].MaxRD  = Data->P[i].Manned[MANNED_HW_MINISHUTTLE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].InitCost   = random_number(45, 75);
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].UnitCost   = random_number(20, 10 + Data->P[i].Manned[MANNED_HW_MINISHUTTLE].InitCost / 2);
        Data->P[i].Manned[MANNED_HW_MINISHUTTLE].RDCost = random_number(5, 9);

        //Jupiter/Kvartet
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].UnitWeight = random_number(Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].UnitWeight * 2, Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].UnitWeight * 3.25);
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].MaxRD  = Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].InitCost   = random_number(45, 75);
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].UnitCost   = random_number(20, 10 + Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].InitCost / 2);
        Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].RDCost = random_number(6, 10);

        //Eagle/Duet
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].UnitWeight = random_number(1200, 1800);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].MaxRD  = Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].InitCost   = random_number(24, 36);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].UnitCost   = random_number(4, Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].InitCost / 2 - 3);
        Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].RDCost = random_min(4);

        //Cricket/L-3
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].UnitWeight = random_number(800, 1200);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].MaxSafety  = random_number(86, 98);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD  = Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].MaxSafety - random_number(2, 10);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].InitCost   = random_number(36, 48);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].UnitCost   = random_number(4, Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].InitCost / 3 - 2);
        Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].RDCost = random_number(2, 6);

        //1-Stage Rocket (Max Payload is at least the satellite and the 1-man capsule)
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].MaxPay = MAX(Data->P[i].Manned[MANNED_HW_ONE_MAN_CAPSULE].UnitWeight, Data->P[i].Probe[PROBE_HW_ORBITAL].UnitWeight) + random_min(200);
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].MaxSafety  = random_number(86, 98);
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].MaxRD  = Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].MaxSafety - random_number(2, 10);
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].InitCost   = random_number(18, 30);
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].UnitCost   = random_number(2, Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].InitCost / 6);
        Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].RDCost = random_number(1, 3);

        //2-Stage Rocket
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].MaxPay = Data->P[i].Rocket[ROCKET_HW_ONE_STAGE].MaxPay + random_number(700, 1100);
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].MaxSafety  = random_number(86, 98);
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].MaxRD  = Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].MaxSafety - random_number(2, 10);
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].InitCost   = random_number(45, 75);
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].UnitCost   = random_number(6, Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].InitCost / 3 - 2);
        Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].RDCost = random_number(2, 6);

        //3-Stage Rocket
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].MaxPay = Data->P[i].Rocket[ROCKET_HW_TWO_STAGE].MaxPay + random_number(1400, 2000);
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].MaxSafety  = random_number(86, 98);
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].MaxRD  = Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].MaxSafety - random_number(2, 10);
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].InitCost   = random_number(68, 100);
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].UnitCost   = random_number(14, Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].InitCost / 4 + 1);
        Data->P[i].Rocket[ROCKET_HW_THREE_STAGE].RDCost = random_number(4, 8);

        //Mega Rocket (Max Payload is at least the 4-man capsule, but maxed at 9999, because otherwise it can show strange symbols)
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].MaxPay = MIN(Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].UnitWeight + random_min(Data->P[i].Manned[MANNED_HW_FOUR_MAN_CAPSULE].UnitWeight), 9999);
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].MaxSafety  = random_number(86, 98);
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].MaxRD  = Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].MaxSafety - random_number(2, 10);
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].InitCost   = random_number(120, 170);
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].UnitCost   = random_number(20, Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].InitCost / 3 - 10);
        Data->P[i].Rocket[ROCKET_HW_MEGA_STAGE].RDCost = random_number(6, 10);

        //Boosters
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].MaxPay = random_number(750, 1250);
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].MaxSafety  = random_number(86, 98);
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].MaxRD  = Data->P[i].Rocket[ROCKET_HW_BOOSTERS].MaxSafety - random_number(2, 10);
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].InitCost   = random_number(10, 14);
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].UnitCost   = random_min(Data->P[i].Rocket[ROCKET_HW_BOOSTERS].InitCost / 2);
        Data->P[i].Rocket[ROCKET_HW_BOOSTERS].RDCost = random_min(4);

        //A-Kicker
        Data->P[i].Misc[MISC_HW_KICKER_A].UnitWeight   = random_number(200, 400);
        Data->P[i].Misc[MISC_HW_KICKER_A].MaxSafety    = random_number(86, 98);
        Data->P[i].Misc[MISC_HW_KICKER_A].MaxRD    = Data->P[i].Misc[MISC_HW_KICKER_A].MaxSafety - random_number(2, 10);
        Data->P[i].Misc[MISC_HW_KICKER_A].InitCost     = random_number(9, 15);
        Data->P[i].Misc[MISC_HW_KICKER_A].UnitCost = random_min(Data->P[i].Misc[MISC_HW_KICKER_A].InitCost / 2);
        Data->P[i].Misc[MISC_HW_KICKER_A].RDCost   = random_min(2);

        //B-Kicker (Unit Weight is 2 times A-Kicker weight)
        Data->P[i].Misc[MISC_HW_KICKER_B].UnitWeight   = Data->P[i].Misc[MISC_HW_KICKER_A].UnitWeight * 2;
        Data->P[i].Misc[MISC_HW_KICKER_B].MaxSafety    = random_number(86, 98);
        Data->P[i].Misc[MISC_HW_KICKER_B].MaxRD    = Data->P[i].Misc[MISC_HW_KICKER_B].MaxSafety - random_number(2, 10);
        Data->P[i].Misc[MISC_HW_KICKER_B].InitCost     = random_number(15, 21);
        Data->P[i].Misc[MISC_HW_KICKER_B].UnitCost = random_number(3, Data->P[i].Misc[MISC_HW_KICKER_B].InitCost / 2);
        Data->P[i].Misc[MISC_HW_KICKER_B].RDCost   = 1 + random_min(2);

        //C-Kicker
        Data->P[i].Misc[MISC_HW_KICKER_C].UnitWeight   = random_number(2000, 3000);
        Data->P[i].Misc[MISC_HW_KICKER_C].MaxSafety    = random_number(86, 98);
        Data->P[i].Misc[MISC_HW_KICKER_C].MaxRD    = Data->P[i].Misc[MISC_HW_KICKER_C].MaxSafety - random_number(2, 10);
        Data->P[i].Misc[MISC_HW_KICKER_C].InitCost     = random_number(35, 45);
        Data->P[i].Misc[MISC_HW_KICKER_C].UnitCost = random_number(16, Data->P[i].Misc[MISC_HW_KICKER_C].InitCost / 2 + 4);
        Data->P[i].Misc[MISC_HW_KICKER_C].RDCost   = random_number(3, 5);

        //EVA suits (Unit weight, unit cost and RD cost are not randomized)
        Data->P[i].Misc[MISC_HW_EVA_SUITS].UnitWeight   = 0;
        Data->P[i].Misc[MISC_HW_EVA_SUITS].MaxSafety    = random_number(86, 98);
        Data->P[i].Misc[MISC_HW_EVA_SUITS].MaxRD    = Data->P[i].Misc[MISC_HW_EVA_SUITS].MaxSafety - random_number(2, 10);
        Data->P[i].Misc[MISC_HW_EVA_SUITS].InitCost     = random_number(15, 21);
        Data->P[i].Misc[MISC_HW_EVA_SUITS].UnitCost = 0;
        Data->P[i].Misc[MISC_HW_EVA_SUITS].RDCost   = 1;

    }
}


void SetEquipName(char plr)
{
    char skip = 0;
    fill_rectangle(95, 50, 227, 96, 0);
    OutBox(96, 51, 226, 95);
    fill_rectangle(97, 52, 225, 94, 3);
    display::graphics.setForegroundColor(9);
    draw_string(103, 59, "EQUIPMENT NAME CHANGE");
    draw_string(123, 69, "(ESCAPE TO SKIP)");
    int n;

    for (n = 0; skip == 0 && n < 3; n++) {
        skip = GetNewName(Data->P[plr].Probe[n].Name);
    }

    for (n = 0; skip == 0 && n < 7; n++) {
        skip = GetNewName(Data->P[plr].Manned[n].Name);
    }

    for (n = 0; skip == 0 && n < 4; n++) {
        skip = GetNewName(Data->P[plr].Rocket[n].Name);
    }
}


char GetNewName(char old_name[20])
{
    char ch = 0, num = 0, name[20];
    int i;

    for (i = 0; i < 20; i++) {
        name[i] = 0x00;
    }

    fill_rectangle(100, 74, 223, 91, 3);
    display::graphics.setForegroundColor(6);
    draw_string(162 - 3 * strlen(old_name), 80, &old_name[0]);
    InBox(99, 84, 223, 92);
    fill_rectangle(100, 85, 222, 91, 0);
    display::graphics.setForegroundColor(1);
    draw_string(162 - 3 * strlen(old_name), 90, &old_name[0]);
    draw_character(0x14);
    av_sync();

    while (ch != K_ENTER && ch != K_ESCAPE) {
        ch = getch();

        if (ch != (ch & 0xff)) {
            ch = 0x00;
        }

        if (ch >= 'a' && ch <= 'z') {
            ch -= 0x20;
        }

        if (ch == 0x08 && num > 0) {
            name[--num] = 0x00;
        } else if (num < 10 && (isupper(ch) || isdigit(ch) || ch == 0x20 || ch == '-')) {
            name[num++] = ch;
        }

        fill_rectangle(100, 85, 222, 91, 0);
        display::graphics.setForegroundColor(1);
        draw_string(162 - 3 * strlen(name), 90, &name[0]);
        draw_character(0x14);
        av_sync();
    }

    if (ch == K_ESCAPE) {
        return 1;
    }

    if (ch == K_ENTER) {
        if (name[0] == '\0') {
            return 0;
        } else {
            strcpy(&old_name[0], &name[0]);
        }
    }

    delay(100);
    return 0;
}
