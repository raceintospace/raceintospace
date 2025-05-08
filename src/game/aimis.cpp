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
// AI Master Routines

// This file seems to control the planning and execution of AI missions

#include "aimis.h"

#include "aimast.h"
#include "aipur.h"
#include "Buzz_inc.h"
#include "downgrader.h"
#include "game_main.h"
#include "hardware.h"
#include "ioexception.h"
#include "logging.h"
#include "mc.h"
#include "mission_util.h"
#include "state_utils.h"
#include "vab.h"

LOG_DEFAULT_CATEGORY(mission)

struct {
    int16_t cost, sf, i;
} Mew[5];
int whe[2], rck[2];
char pc[2], bc[2], Alt_A[2] = {0, 0}, Alt_B[2] = {0, 0};
void Strategy_One(char plr, int *m_1, int *m_2, int *m_3);
void Strategy_Two(char plr, int *m_1, int *m_2, int *m_3);
void Strategy_Thr(char plr, int *m_1, int *m_2, int *m_3);

void AIVabCheck(char plr, char mis, char prog);
char Best(void);
int ICost(char plr, char h, char i);
void CalcSaf(char plr, char vs);
char Panic_Level(char plr, int *m_1, int *m_2);




void AIVabCheck(char plr, char mis, char prog)
{
    VASqty = 0;
    //prog=1; 0=UnM : 1=1Mn ...
    const struct mStr plan = GetMissionPlan(mis);
    whe[0] = whe[1] = -1;

    if (prog == 5) {
        if (plan.Jt == 0 && plan.LM == 0 && plan.Doc == 0) {
            BuildVAB(plr, mis, 1, 0, prog - 1);
            CalcSaf(plr, VASqty);
            whe[0] = Best();

            if (Mew[whe[0]].i < 60) {
                whe[0] = 0;    // Weed out low safeties
            }
        }
    } else if (prog >= plan.mEq && (prog != 0)) { // && (plan.mVab[0]&0x80 || plan.mVab[1]&0x80)) )
        if (plan.Jt == 1) {                      // Joint mission
            BuildVAB(plr, mis, 1, 0, prog - 1);     // first launch
            CalcSaf(plr, VASqty);
            whe[0] = Best();

            if (Mew[whe[0]].i < 60) {
                whe[0] = 0;
            }

            BuildVAB(plr, mis, 1, 1, prog - 1);
            CalcSaf(plr, VASqty);
            whe[1] = Best();

            if (Mew[whe[1]].i < 60) {
                whe[1] = 0;    // Weed out low safeties
            }
        } else {
            // fill_rectangle(100,100,200,190,5);
            // draw_string(110,110,"MIS ");draw_number(0,0,mis);
            // draw_string(110,120,"PRG ");draw_number(0,0,prog);
            // PauseMouse();

            BuildVAB(plr, mis, 1, 0, prog - 1);
//        draw_string(110,130,"VAS ");draw_number(0,0,VASQTY);
            CalcSaf(plr, VASqty);
            whe[0] = Best();

            if (Mew[whe[0]].i < 60) {
                whe[0] = 0;
            }
        }
    } else if (prog == 0 && prog == plan.mEq) { // Unmanned Vechicle
        BuildVAB(plr, mis, 1, 0, prog);         //  plr,mcode,ty=1,part,prog
        CalcSaf(plr, VASqty);
        whe[0] = Best();
        // if (Mew[whe[0]].i<30) whe[0]=0;
        // ShowVA(whe[0]);
    }

    return;
}

char Best(void)
{
    int i, j, ct, ct1;
    char valid[5];

    for (i = 0; i < 5; i++) {
        valid[i] = 0;
    }

    for (i = 1; i < VASqty + 1; i++) {
        ct = ct1 = 0;

        for (j = 0; j < 4; j++) {
            if (strncmp("NONE", &VAS[i][j].name[0], 4) != 0) {
                ct1++;
            }

            if (VAS[i][j].qty >= 0) {
                ct += VAS[i][j].sf;
            }
        }

        if (ct1 > 0) {
            valid[i] = ct / ct1;
        }
    }

    ct1 = 0;

    for (i = 1; i < VASqty + 1; i++) {
        ct1 = (valid[i] > valid[ct1]) ? i : ct1;
    }

    return ct1;
}


int ICost(char plr, char h, char i)
{
    int cost = 0;

    switch (h) {
    case Mission_Capsule:
    case Mission_LM:
        cost = cost + Data->P[plr].Manned[i].MaxRD - Data->P[plr].Manned[i].Safety;
        cost = cost / 3.5;
        cost = cost * Data->P[plr].Manned[i].RDCost;

        if (Data->P[plr].Manned[i].Num < 0) {
            cost += Data->P[plr].Manned[i].InitCost;
        }

        if (Data->P[plr].Manned[i].Num == 0) {
            cost += Data->P[plr].Manned[i].UnitCost;
        }

        break;

    case Mission_Kicker:
        cost = cost + Data->P[plr].Misc[i].MaxRD - Data->P[plr].Misc[i].Safety;
        cost = cost / 3.5;
        cost = cost * Data->P[plr].Misc[i].RDCost;

        if (Data->P[plr].Misc[i].Num < 0) {
            cost += Data->P[plr].Misc[i].InitCost;
        }

        if (Data->P[plr].Misc[i].Num == 0) {
            cost += Data->P[plr].Misc[i].UnitCost;
        }

        break;

    case Mission_Probe_DM:
        if (i < 4) {
            cost = cost + Data->P[plr].Probe[i].MaxRD - Data->P[plr].Probe[i].Safety;
            cost = cost / 3.5;
            cost = cost * Data->P[plr].Probe[i].RDCost;

            if (Data->P[plr].Probe[i].Num < 0) {
                cost += Data->P[plr].Probe[i].InitCost;
            }

            if (Data->P[plr].Probe[i].Num == 0) {
                cost += Data->P[plr].Probe[i].UnitCost;
            }
        } else {
            if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num < 0) {
                cost += Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].InitCost;
            }

            if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num == 0) {
                cost += Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].UnitCost;
            }
        }

        break;

    default:
        break;
    }

    return cost;
}


void CalcSaf(char plr, char vs)
{
    int i, j, k, sum = 0, co = 0, t = 0;

    for (i = 0; i < 5; i++) {
        Mew[i].cost = Mew[i].sf = 0;    // Clear thing
    }

    // Do first part

    for (j = 1; j < vs + 1; j++) {
        for (k = 0; k < 4; k++) {
            if (VAS[j][k].qty >= 0) {
                sum += VAS[j][k].sf;
            }

            if (strncmp("NONE", &VAS[j][k].name[0], 4) != 0) {
                t++;
            }

            if (VAS[j][k].wt > 0)  {
                co += ICost(plr, k, VAS[j][k].dex);
            }
        }

        Mew[j].cost += co;
        co = 0;
        Mew[j].sf += sum;
        sum = 0;

        if (t > 0) {
            Mew[j].i = Mew[j].sf / t;
        }

        t = 0;
    }

    return;
}

char Panic_Level(char plr, int *m_1, int *m_2)
{
// PANIC level manned docking/EVA/duration
    if (Alt_B[plr] <= 1 &&
        Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] == 4 &&
        PrestigeCheck(plr, Prestige_MannedDocking) == 0 &&
        PrestigeCheck(plr, Prestige_Spacewalk) == 0 &&
        Data->P[plr].Mission[0].MissionCode != Mission_U_Orbital_D &&
        Data->P[plr].Mission[1].MissionCode != Mission_Manned_Orbital_Docking_EVA) {
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_Manned_Orbital_Docking_EVA;
        ++Alt_B[plr];
        return 1;
    }

// PANIC lunar pass/probe landing/lunar flyby
    if (Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] == 5 &&
        !PrestigeCheck(plr, Prestige_LunarFlyby) &&
        !PrestigeCheck(plr, Prestige_LunarProbeLanding) &&
        Cur_Status == Ahead &&
        Alt_A[plr] <= 2) {
        *m_1 = Mission_LunarFlyby;

        if (Data->P[plr].DurationLevel <= 2) {
            *m_2 = Mission_Orbital_Duration;
        } else {
            *m_2 = Mission_Lunar_Probe;
        }

        ++Alt_A[plr];

        return 1;
    }

    // PANIC level duration/pass/lunar orbital/LM_pts
    return 0;
}

void Strategy_One(char plr, int *m_1, int *m_2, int *m_3)
{
//AI version 12/26/92
    switch (Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:// mission 26 -> if manned docking and eva  -> DurationLevel+1
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        *m_3 = Mission_LunarFlyby;
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;

        if (PrestigeCheck(plr, Prestige_Spacewalk) == 0) {
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
        } else {
            *m_2 = Mission_Orbital_Docking;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];

        *m_3 = Mission_LunarFlyby;
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        }

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        break;

    case 3:
        *m_1 = Mission_Orbital_Docking_Duration;

        if (Cur_Status == Behind) {
            *m_2 = Mission_Orbital_Duration;
        }

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (Data->P[plr].DurationLevel) {
        case 0:
        case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety >= Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3:
        case 4:
        case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (Data->P[plr].Cash <= 0) {
            Data->P[plr].Cash = 0;
        }

        Data->P[plr].Cash += Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        } else {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        }

        if (Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            Data->P[plr].AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5:
        *m_1 = Mission_LunarPass;

        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }

        //lunar pass
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6:
        if (Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Safety > Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_LunarOrbital;    //lunar orbit
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && Data->P[plr].Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (Data->P[plr].LMpts == 0 && Data->P[plr].Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (Data->P[plr].LMpts) {
            case 0:
            case 1:
                if (Data->P[plr].Mission[0].MissionCode == Mission_Lunar_Orbital) {
                    *m_1 = Mission_HistoricalLanding;
                } else {
                    *m_1 = Mission_Lunar_Orbital;
                }

                break;

            case 2:
            case 3:
                *m_1 = Mission_HistoricalLanding;
                break;

            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;

            if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety < 60) {
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }

        break;

    default:
        break;
    }

    return;
}

void Strategy_Two(char plr, int *m_1, int *m_2, int *m_3)
{
// AI version 12/28/92
    switch (Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        *m_3 = Mission_LunarFlyby;
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_Orbital_Docking;
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        *m_3 = Mission_LunarFlyby;
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        }

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        break;

    case 3:
        *m_1 = Mission_Orbital_Duration;
        *m_2 = Mission_Orbital_Docking_Duration;

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        } else {
            *m_3 = Mission_LunarFlyby;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (Data->P[plr].DurationLevel) {
        case 0:
        case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            *m_3 = Mission_LunarFlyby;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety >= Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3:
        case 4:
        case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (Data->P[plr].Cash <= 0) {
            Data->P[plr].Cash = 0;
        }

        Data->P[plr].Cash += Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        } else {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        }

        if (Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            Data->P[plr].AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5:
        *m_1 = Mission_LunarPass;

        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }

        //lunar pass
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6:
        if (Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Safety > Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10) {
            *m_1 = Mission_LunarOrbital;    //lunar orbit
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && Data->P[plr].Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (Data->P[plr].LMpts == 0 && Data->P[plr].Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (Data->P[plr].LMpts) {
            case 0:
            case 1:
                if (Data->P[plr].Mission[0].MissionCode == Mission_Lunar_Orbital) {
                    *m_1 = Mission_HistoricalLanding;
                } else {
                    *m_1 = Mission_Lunar_Orbital;
                }

                break;

            case 2:
            case 3:
                *m_1 = Mission_HistoricalLanding;
                break;

            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;

            if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety < 60) {
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }

        break;

    default:
        break;
    }

    return;
}

void Strategy_Thr(char plr, int *m_1, int *m_2, int *m_3)
{
//new version undated
    switch (Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:// mission 26 -> if manned docking and eva  -> DurationLevel+1
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        *m_3 = Mission_LunarFlyby;
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;

        if (PrestigeCheck(plr, Prestige_Spacewalk) == 0) {
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
        } else {
            *m_2 = Mission_Orbital_Docking;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];

        *m_3 = Mission_LunarFlyby;
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        }

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        break;

    case 3:
        *m_1 = Mission_Orbital_Docking_Duration;

        if (Cur_Status == Behind) {
            *m_2 = Mission_Orbital_Duration;
        }

        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (Data->P[plr].DurationLevel) {
        case 0:
        case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety >= Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3:
        case 4:
        case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (Data->P[plr].Cash <= 0) {
            Data->P[plr].Cash = 0;
        }

        Data->P[plr].Cash += Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        } else {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        }

        if (Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            Data->P[plr].AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5:
        *m_1 = Mission_LunarPass;

        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }

        //lunar pass
        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6:
        if (Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Safety > Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10) {
            *m_1 = Mission_LunarOrbital;    //lunar orbit
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && Data->P[plr].Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (Data->P[plr].LMpts == 0 && Data->P[plr].Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }

        ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (Data->P[plr].LMpts) {
            case 0:
            case 1:
                if (Data->P[plr].Mission[0].MissionCode == Mission_Lunar_Orbital) {
                    *m_1 = Mission_HistoricalLanding;
                } else {
                    *m_1 = Mission_Lunar_Orbital;
                }

                break;

            case 2:
            case 3:
                *m_1 = Mission_HistoricalLanding;
                break;

            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;

            if (Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Safety < 60) {
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }

        break;

    default:
        break;
    }

    return;
}

void NewAI(char plr, char frog)
{
    char i, spc[2], prg[2], primaryPad, secondaryPad, hsf, Panic_Check = 0;
    int mis1, mis2, mis3, val;

    spc[0] = 0; /* XXX check uninitialized */

    prg[0] = frog;
    mis1 = mis2 = mis3 = Mission_None;
    primaryPad = secondaryPad = PAD_NONE;
    GenPur(plr, MANNED_HARDWARE, frog - 1);

    if (Data->P[plr].AILunar < 4) {
        mis1 = Mission_None;
        mis2 = Mission_None;
        mis3 = Mission_None;
        hsf = 0;

        for (i = 0; i < 3; i++) {
            if (Data->P[plr].Probe[hsf].Safety <= Data->P[plr].Probe[i].Safety) {
                hsf = i;
            }
        }

        RDafford(plr, PROBE_HARDWARE, hsf);

        if (Data->P[plr].Probe[hsf].Safety < 90) {
            if (GenPur(plr, PROBE_HARDWARE, hsf)) {
                RDafford(plr, PROBE_HARDWARE, hsf);
            } else {
                RDafford(plr, PROBE_HARDWARE, hsf);
            }
        }

        Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num = 2;
        Panic_Check = Panic_Level(plr, &mis1, &mis2);

        if (!Panic_Check) {
            if (Data->P[plr].AIStrategy[AI_STRATEGY] == 1) {
                Strategy_One(plr, &mis1, &mis2, &mis3);
            } else if (Data->P[plr].AIStrategy[AI_STRATEGY] == 2) {
                Strategy_Two(plr, &mis1, &mis2, &mis3);
            } else {
                Strategy_Thr(plr, &mis1, &mis2, &mis3);
            }

            if (mis1 == Mission_HistoricalLanding)
                switch (Data->P[plr].AILunar) {
                case 1:
                    mis1 = Mission_HistoricalLanding; //Apollo behind Gemini

                    if (frog == 2 && (Data->P[plr].AISec == Mission_Lunar_Probe || Data->P[plr].AISec == Mission_VenusFlyby)) {
                        val = Data->P[plr].AISec;

                        if (val < 7) {
                            val = val - 4;
                        } else {
                            val = val - 5;
                        }

                        if (Data->P[plr].Manned[val - 1].Safety >= Data->P[plr].Manned[val - 1].MaxRD - 10) {
                            mis2 = Mission_HistoricalLanding;
                            spc[0] = val;
                        }
                    }

                    break;

                case 2:
                    mis1 = Mission_Jt_LunarLanding_EOR;
                    mis2 = Mission_None;
                    break;

                case 3:
                    mis1 = Mission_Jt_LunarLanding_LOR;
                    mis2 = Mission_None;
                    break;

                default:
                    break;
                }
        }
    } else {
        switch (Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION]) {
        case 0:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_Manned_Orbital_Docking_EVA;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 1:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_Orbital_Duration;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 2:
            if (Data->P[plr].Cash <= 0) {
                Data->P[plr].Cash = 0;
            }

            Data->P[plr].Cash += Data->P[plr].Rocket[ROCKET_HW_MEGA_STAGE].InitCost + 25;

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            if (Data->P[plr].Rocket[ROCKET_HW_MEGA_STAGE].Num >= 0) {
                Data->P[plr].AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
            }

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            mis1 = Mission_Orbital_Duration;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 3:
            switch (Data->P[plr].DurationLevel) {
            case 0:
            case 1:
                mis1 = Mission_Orbital_Duration;
                mis2 = Mission_Orbital_Duration;
                break;

            case 2:
                mis1 = Mission_Orbital_Duration;
                mis2 = Mission_LunarFlyby;
                ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
                break;

            case 3:
            case 4:
            case 5:
                mis1 = Mission_LunarFlyby;
                mis2 = Mission_LunarFlyby;
                ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
                break;

            default:
                break;
            }

            break;

        case 4:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_LunarFlyby;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 5:
            switch (Data->P[plr].DurationLevel) {
            case 0:
            case 1:
            case 2:
                mis1 = Mission_Orbital_Duration;
                break;

            case 3:
                mis1 = (PrestigeCheck(plr, Prestige_MannedOrbital) == 0) ? Mission_Orbital_EVA_Duration : Mission_Orbital_Duration;
                break;

            case 4:
            case 5:
                if (PrestigeCheck(plr, Prestige_LunarFlyby) == plr || PrestigeCheck(plr, Prestige_LunarProbeLanding) == plr) {
                    mis1 = Mission_LunarPass;
                } else {
                    mis1 = Mission_LunarFlyby;
                    mis2 = Mission_Lunar_Probe;
                }

                break;

            default:
                break;
            }

            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 6:
            mis1 = Mission_LunarPass;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 7:
            if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
                mis1 = Mission_LunarPass;
            } else {
                mis1 = Mission_LunarOrbital;
            }

            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 8:
            mis1 = Mission_LunarOrbital;
            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 9:
            if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
                mis1 = Mission_LunarOrbital;
            }

            ++Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION];

            break;

        case 10:
            if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
                mis1 = Mission_LunarOrbital;
            } else {
                mis1 = Mission_DirectAscent_LL;
            }

            break;

        default:
            break;
        }
    };

// unmanned/manned kludge
    if (mis1 == Mission_Orbital_Docking && mis2 == Mission_U_Orbital_D) {
        mis2 = Mission_U_Orbital_D;
        mis1 = Mission_Orbital_Docking;
    };

//lunar flyby/probe landing kludge
    if (mis1 == Mission_LunarFlyby && mis2 == Mission_LunarFlyby)
        if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 15) {
            mis2 = Mission_Lunar_Probe;
        }

    const struct mStr plan = GetMissionPlan(mis1);

// deal with lunar modules
    if (plan.LM == 1) {
        if (Data->P[plr].AIStrategy[AI_LUNAR_MODULE] > 0) {
            if (GenPur(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE])) {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            } else {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            }
        } else {
            Data->P[plr].AIStrategy[AI_LUNAR_MODULE] = 6;

            if (GenPur(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE])) {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            } else {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            }
        }
    }

    if (plan.Jt == 1) {
        // JOINT LAUNCH
        if (Data->P[plr].Future[0].MissionCode == Mission_None && Data->P[plr].LaunchFacility[0] == LAUNCHPAD_OPERATIONAL &&
            Data->P[plr].Future[1].MissionCode == Mission_None && Data->P[plr].LaunchFacility[1] == LAUNCHPAD_OPERATIONAL) {
            primaryPad = PAD_A;
        }

        if (mis1 > 0)
            if (primaryPad != PAD_NONE) {
                AIFuture(plr, mis1, primaryPad, (char *)&prg);
            }
    } else {
        // SINGLE LAUNCH
        if (mis1 == Mission_DirectAscent_LL) {
            prg[0] = 5;
        }

        if (mis1 == Mission_LunarFlyby || mis1 == Mission_Lunar_Probe) {
            prg[0] = 0;
        }

        if (Data->P[plr].Future[0].MissionCode == Mission_None && Data->P[plr].LaunchFacility[0] == LAUNCHPAD_OPERATIONAL) {
            primaryPad = PAD_A;
        }

        if (Data->P[plr].Future[1].MissionCode == Mission_None && Data->P[plr].LaunchFacility[1] == LAUNCHPAD_OPERATIONAL) {
            if (primaryPad == PAD_A) {
                secondaryPad = PAD_B;
            } else {
                primaryPad = PAD_B;
            }
        }

        if (Data->P[plr].Future[2].MissionCode == Mission_None && Data->P[plr].LaunchFacility[2] == LAUNCHPAD_OPERATIONAL) {
            if (primaryPad != PAD_A && secondaryPad != PAD_B) {
                if (primaryPad == PAD_B) {
                    secondaryPad = PAD_C;
                } else if (primaryPad == PAD_A && secondaryPad == PAD_NONE) {
                    secondaryPad = PAD_C;
                } else {
                    primaryPad = PAD_C;
                }
            }
        };

        if (mis1 > 0) {
            if (primaryPad != PAD_NONE) {
                AIFuture(plr, mis1, primaryPad, (char *)&prg);
            }
        }

        if (mis2 > 0) {
            if (mis2 == Mission_LunarFlyby || mis2 == Mission_Lunar_Probe) {
                prg[0] = 0;
            } else {
                prg[0] = frog;
            }

            if (mis2 == Mission_HistoricalLanding) {
                prg[0] = spc[0];
            }

            if (secondaryPad != -1) {
                AIFuture(plr, mis2, secondaryPad, (char *)&prg);
            }
        }

        if (mis3 > 0) {
            prg[0] = frog;

            if (secondaryPad != -1) {
                AIFuture(plr, mis3, 2, (char *)&prg);
            }
        }
    }

    if (Data->P[plr].Future[2].MissionCode == Mission_None &&
        Data->P[plr].LaunchFacility[2] == LAUNCHPAD_OPERATIONAL) {
        if ((mis1 == 0 && frog == 2 && (
                 Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety >= Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].MaxRD - 10)) ||
            (Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Safety >= Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].MaxRD - 10)) {
            if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 &&
                PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 0) {
                mis3 = Mission_SubOrbital;
            } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 &&
                       PrestigeCheck(other(plr), Prestige_MannedOrbital) == 0) {
                mis3 = Mission_Earth_Orbital;
            }

            if (mis3 == Mission_None) {
                if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 &&
                    PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 1) {
                    mis3 = Mission_SubOrbital;
                } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 &&
                           PrestigeCheck(other(plr), Prestige_MannedOrbital) == 1) {
                    mis3 = Mission_Earth_Orbital;
                }
            }
        }

        if (mis3 == Mission_None)
            if (mis1 != Mission_LunarFlyby && mis1 != Mission_Lunar_Probe) {
                if (mis1 == Mission_LunarFlyby) {
                    mis3 = Mission_Lunar_Probe;
                } else if (mis1 == Mission_Lunar_Probe) {
                    mis3 = Mission_LunarFlyby;
                }

                if (Data->P[plr].Probe[PROBE_HW_LUNAR].Safety > Data->P[plr].Probe[PROBE_HW_LUNAR].MaxRD - 15)
                    if (PrestigeCheck(plr, Prestige_LunarProbeLanding) == 0 ||
                        Data->P[plr].Misc[MISC_HW_PHOTO_RECON].Safety < 85) {
                        if (mis3 == Mission_None) {
                            mis3 = Mission_Lunar_Probe;
                        }
                    }

                if ((Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety > Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].MaxRD - 15) &&
                    mis3 == Mission_None) {
                    if (PrestigeCheck(plr, Prestige_LunarFlyby) == 0 &&
                        PrestigeCheck(other(plr), Prestige_LunarFlyby) == 0 &&
                        Data->P[plr].Mission[2].MissionCode != Mission_LunarFlyby) {
                        mis3 = Mission_LunarFlyby;
                    } else if (PrestigeCheck(plr, Prestige_MercuryFlyby) == 0 &&
                               PrestigeCheck(other(plr), Prestige_MercuryFlyby) == 0 &&
                               Data->P[plr].Mission[2].MissionCode != Mission_MercuryFlyby) {
                        mis3 = Mission_MercuryFlyby;
                    } else if (PrestigeCheck(plr, Prestige_VenusFlyby) == 0 &&
                               PrestigeCheck(other(plr), Prestige_VenusFlyby) == 0 &&
                               Data->P[plr].Mission[2].MissionCode != Mission_VenusFlyby) {
                        mis3 = Mission_VenusFlyby;
                    } else if (PrestigeCheck(plr, Prestige_MarsFlyby) == 0 &&
                               PrestigeCheck(other(plr), Prestige_MarsFlyby) == 0 &&
                               Data->P[plr].Mission[2].MissionCode != Mission_MarsFlyby) {
                        mis3 = Mission_MarsFlyby;
                    } else if (PrestigeCheck(plr, Prestige_JupiterFlyby) == 0 &&
                               PrestigeCheck(other(plr), Prestige_JupiterFlyby) == 0 &&
                               Data->P[plr].Mission[2].MissionCode != Mission_JupiterFlyby) {
                        mis3 = Mission_JupiterFlyby;
                    } else if (PrestigeCheck(plr, Prestige_SaturnFlyby) == 0 &&
                               PrestigeCheck(other(plr), Prestige_SaturnFlyby) == 0 &&
                               Data->P[plr].Mission[2].MissionCode != Mission_SaturnFlyby) {
                        mis3 = Mission_SaturnFlyby;
                    }

                    if (mis3 == Mission_None) {
                        if (PrestigeCheck(plr, Prestige_LunarFlyby) == 0 &&
                            PrestigeCheck(other(plr), Prestige_LunarFlyby) == 1 &&
                            Data->P[plr].Mission[2].MissionCode != Mission_LunarFlyby) {
                            mis3 = Mission_LunarFlyby;
                        } else if (PrestigeCheck(plr, Prestige_MercuryFlyby) == 0 &&
                                   PrestigeCheck(other(plr), Prestige_MercuryFlyby) == 1 &&
                                   Data->P[plr].Mission[2].MissionCode != Mission_MercuryFlyby) {
                            mis3 = Mission_MercuryFlyby;
                        } else if (PrestigeCheck(plr, Prestige_VenusFlyby) == 0 &&
                                   PrestigeCheck(other(plr), Prestige_VenusFlyby) == 1 &&
                                   Data->P[plr].Mission[2].MissionCode != Mission_VenusFlyby) {
                            mis3 = Mission_VenusFlyby;
                        } else if (PrestigeCheck(plr, Prestige_MarsFlyby) == 0 &&
                                   PrestigeCheck(other(plr), Prestige_MarsFlyby) == 1 &&
                                   Data->P[plr].Mission[2].MissionCode != Mission_MarsFlyby) {
                            mis3 = Mission_MarsFlyby;
                        } else if (PrestigeCheck(plr, Prestige_JupiterFlyby) == 0 &&
                                   PrestigeCheck(other(plr), Prestige_JupiterFlyby) == 1 &&
                                   Data->P[plr].Mission[2].MissionCode != Mission_JupiterFlyby) {
                            mis3 = Mission_JupiterFlyby;
                        } else if (PrestigeCheck(plr, Prestige_SaturnFlyby) == 0 &&
                                   PrestigeCheck(other(plr), Prestige_SaturnFlyby) == 1 &&
                                   Data->P[plr].Mission[2].MissionCode != Mission_SaturnFlyby) {
                            mis3 = Mission_SaturnFlyby;
                        }
                    }
                }
            }

        if (mis3 == Mission_None) {
            if (GenPur(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL)) {
                RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
            } else {
                RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            }

            if (Data->P[plr].Probe[PROBE_HW_ORBITAL].Num >= 1 && Data->P[plr].Rocket[ROCKET_HW_ONE_STAGE].Num >= 1) {
                mis3 = Mission_Orbital_Satellite;
            }
        }

        if (mis3 != Mission_SubOrbital && mis3 != 4) {
            prg[0] = 0;
        }

        if (mis3 > 0) {
            AIFuture(plr, mis3, 2, (char *)&prg);
        }
    }

    AILaunch(plr);
    return;
}

void AIFuture(char plr, char mis, char pad, char *prog)
{
    int i, j;
    char prime, back, max, men;
    char fake_prog[2];

    if (prog == NULL) {
        memset(fake_prog, 0, sizeof fake_prog);
        prog = fake_prog;
    }

    if (prog[1] < 0 || prog[1] > 5) {
        prog[1] = prog[0];
    }

    const struct mStr plan = GetMissionPlan(mis);

    for (i = 0; i < (plan.Jt + 1); i++) {
        Data->P[plr].Future[pad + i].MissionCode = mis;
        Data->P[plr].Future[pad + i].part = i;

        // duration
        if (Data->P[plr].DurationLevel <= 5 && Data->P[plr].Future[pad + i].Duration == 0) {
            if (plan.Dur == 1) Data->P[plr].Future[pad + i].Duration =
                    MAX(plan.Days, MIN(Data->P[plr].DurationLevel + 1, 6));
            else {
                Data->P[plr].Future[pad + i].Duration = plan.Days;
            }
        }

        if (Data->P[plr].Mission[0].Duration == Data->P[plr].Future[pad + i].Duration ||
            Data->P[plr].Mission[1].Duration == Data->P[plr].Future[pad + i].Duration) {
            ++Data->P[plr].Future[pad + i].Duration;
        }

        if (pad == 1 && Data->P[plr].Future[0].Duration == Data->P[plr].Future[pad + i].Duration) {
            ++Data->P[plr].Future[pad + i].Duration;
        }

        if (Data->P[plr].Future[pad + i].Duration >= 6) {
            Data->P[plr].Future[pad + i].Duration = 6;
        }

        // one-man capsule duration kludge
        if (Data->P[plr].Future[pad + i].Prog == 1) {
            if (Data->P[plr].DurationLevel == 0) {
                Data->P[plr].Future[pad + i].Duration = 1;
            } else {
                Data->P[plr].Future[pad + i].Duration = 2;
            }
        }; // limit duration 'C' one-man capsule

        // lunar mission kludge
        if (plan.Lun == 1 ||
            Data->P[plr].Future[pad + i].MissionCode == Mission_Jt_LunarLanding_EOR ||
            Data->P[plr].Future[pad + i].MissionCode == Mission_Jt_LunarLanding_LOR ||
            Data->P[plr].Future[pad + i].MissionCode == Mission_HistoricalLanding) {
            Data->P[plr].Future[pad + i].Duration = 4;
        }

        // unmanned duration kludge
        if (plan.Days == 0) {
            Data->P[plr].Future[pad + i].Duration = 0;
        }

        Data->P[plr].Future[pad + i].Joint = plan.Jt;
        Data->P[plr].Future[pad + i].Month = 0;

        if (mis == 1) {
            prog[i] = 0;
        }

        Data->P[plr].Future[pad + i].Prog = prog[0];

        if (prog[i] > 0 && plan.Days > 0) {
            for (j = 1; j < 6; j++) {
                DumpAstro(plr, j);
            }

            TransAstro(plr, prog[i]); //indexed OK

            if (Data->P[plr].Future[pad + i].PCrew != 0) {
                prime = Data->P[plr].Future[pad + i].PCrew - 1;
            } else {
                prime = -1;
            }

            if (Data->P[plr].Future[pad + i].BCrew != 0) {
                back = Data->P[plr].Future[pad + i].BCrew - 1;
            } else {
                back = -1;
            }

            max = prog[i];

            if (prog[i] > 3) {
                max = prog[i] - 1;
            }

            Data->P[plr].Future[pad + i].Men = max;
            men = Data->P[plr].Future[pad + i].Men;

            if (prime != -1)
                for (j = 0; j < men; j++) {
                    Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][prime][j] - 1].Prime = 0;
                }

            if (back != -1)
                for (j = 0; j < men; j++) {
                    Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][back][j] - 1].Prime = 0;
                }

            Data->P[plr].Future[pad + i].PCrew = 0;
            Data->P[plr].Future[pad + i].BCrew = 0;
            pc[i] = -1;
            bc[i] = -1;

            for (j = 0; j < 8; j++) {
                if (pc[i] == -1 &&
                    Data->P[plr].Crew[prog[i]][j][0] != 0 &&
                    Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][j][0] - 1].Prime == 0) {
                    pc[i] = j;
                }
            }

            if (pc[i] == -1) {
                // astronaut/duration kludge
                if (plan.Days > 0) {
                    Data->P[plr].Future[pad + i].Men = max;
                }

                // no astronauts available have to go unmanned
                Data->P[plr].Future[pad + i].Men = 0;
                Data->P[plr].Future[pad + i].PCrew = 0;
                Data->P[plr].Future[pad + i].BCrew = 0;

                Downgrader::Options options = LoadJsonDowngrades("DOWNGRADES.JSON");
                Downgrader replace(Data->P[plr].Future[pad + i], options);
                char mcode = -1;

                //  Find a mission that can be flown unmanned
                try {
                    std::vector<struct mStr> missionData = GetMissionData();

                    while (mcode < 0) {

                        std::string cName = missionData.at(replace.current().MissionCode).Name;
                        std::size_t pos = cName.find("MANNED");

                        if (pos != std::string::npos) {

                            // "MANNED" -> "UNMANNED"
                            std::string uName = cName.replace(pos, 0, "UN");

                            // Check whether unmanned counterpart exists
                            for (int i = 0; i < missionData.size(); i++) {
                                if (!uName.compare(missionData[i].Name)) {
                                    mcode = i;
                                    break;
                                }
                            }
                        }

                        replace.next();

                        if (mcode < 0 && replace.current().MissionCode == Mission_None) {
                            // Fly Unmanned Earth Orbital as a last resort
                            mcode = Mission_Unmanned_Earth_Orbital;
                        }
                    }

                    TRACE3("AI replacing mission code %i by %i", Data->P[plr].Future[pad + i].MissionCode, mcode);
                    Data->P[plr].Future[pad + i].MissionCode = mcode;

                } catch (IOException &err) {
                    // TODO: Can't download to Earth Orbital if Joint mission.
                    CRITICAL2("Error loading data file: %s", err.what());
                    WARNING1("Defaulting to Unmanned Earth Orbital.");
                    Data->P[plr].Future[pad + i].MissionCode = Mission_Unmanned_Earth_Orbital;
                }

                return;
            }

            Data->P[plr].Future[pad + i].PCrew = pc[i] + 1;
            bc[i] = -1;

            for (j = 0; j < 8; j++) {
                if (bc[i] == -1 &&
                    j != pc[i] &&
                    Data->P[plr].Crew[prog[i]][j][0] != 0 &&
                    Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][j][0] - 1].Prime == 0) {
                    bc[i] = j;
                }
            }

            Data->P[plr].Future[pad + i].BCrew = bc[i] + 1;

            for (j = 0; j < men; j++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][pc[i]][j] - 1].Prime = 4;
            }

            for (j = 0; j < men; j++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][bc[i]][j] - 1].Prime = 2;
            }
        } else {
            Data->P[plr].Future[pad + i].Men = 0;
            Data->P[plr].Future[pad + i].PCrew = 0;
            Data->P[plr].Future[pad + i].BCrew = 0;
        }
    }

// joint mission Mission_Jt_LunarLanding_EOR and Mission_Jt_LunarLanding_LOR men kludge
    if (mis == Mission_Jt_LunarLanding_EOR || mis == Mission_Jt_LunarLanding_LOR) {
        Data->P[plr].Future[pad + 1].Men = Data->P[plr].Future[pad].Men;
        Data->P[plr].Future[pad + 1].PCrew = Data->P[plr].Future[pad].PCrew;
        Data->P[plr].Future[pad + 1].BCrew = Data->P[plr].Future[pad].BCrew;
        Data->P[plr].Future[pad + 1].Prog = Data->P[plr].Future[pad].Prog;
        Data->P[plr].Future[pad].Men = 0;
        Data->P[plr].Future[pad].PCrew = 0;
        Data->P[plr].Future[pad].BCrew = 0;
        Data->P[plr].Future[pad].Prog = 0;
        Data->P[plr].Future[pad + 1].Duration = Data->P[plr].Future[pad].Duration;
        Data->P[plr].Future[pad].Duration = 0;
    }

    return;
}

void AILaunch(char plr)
{
    int i, j, k = 0, l = 0, JR = 0, wgt, bwgt[7];
    char boos[7], bdex[7];


    for (i = 0; i < 7; i++) {
        bdex[i] = i;
        boos[i] = (i > 3) ?
                  RocketBoosterSafety(Data->P[plr].Rocket[i - 4].Safety, Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Safety)
                  : Data->P[plr].Rocket[i].Safety;
        bwgt[i] = (i > 3) ?
                  (Data->P[plr].Rocket[i - 4].MaxPay + Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].MaxPay)
                  : Data->P[plr].Rocket[i].MaxPay;

        if (boos[i] < 60) {
            boos[i] = -1;    // Get Rid of any Unsafe rocket systems
        }

        if (Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Num < 1) for (j = 4; j < 7; j++) {
                boos[j] = -1;
            }

        for (j = 0; j < 4; j++) if (Data->P[plr].Rocket[j].Num < 1) {
                boos[j] = -1;
            }
    }

    for (i = 0; i < 3; i++) {
        if (Data->P[plr].Mission[i].MissionCode == Mission_Orbital_DockingInOrbit_Duration && Data->P[plr].DockingModuleInOrbit == 0) {
            Data->P[plr].Mission[i].MissionCode = Mission_None;
            return;
        }

        if (Data->P[plr].Mission[i].MissionCode && Data->P[plr].Mission[i].part == 0) {
            whe[0] = whe[1] = -1;

            if (Data->P[plr].Mission[i].Joint == 1) {
                AIVabCheck(plr, Data->P[plr].Mission[i].MissionCode, Data->P[plr].Mission[i + 1].Prog);
            } else {
                AIVabCheck(plr, Data->P[plr].Mission[i].MissionCode, Data->P[plr].Mission[i].Prog);
            }

            if (whe[0] > 0) {
                if (Data->P[plr].Mission[i].Prog == 0) {
                    BuildVAB(plr, Data->P[plr].Mission[i].MissionCode, 1, 0, Data->P[plr].Mission[i].Prog);
                } else {
                    BuildVAB(plr, Data->P[plr].Mission[i].MissionCode, 1, 0, Data->P[plr].Mission[i].Prog - 1);
                }

                for (j = Mission_Capsule; j <= Mission_Probe_DM; j++) {
                    Data->P[plr].Mission[i].Hard[j] = VAS[whe[0]][j].dex;
                }

                wgt = 0;

                for (j = 0; j < 4; j++) {
                    wgt += VAS[whe[0]][j].wt;
                }

                rck[0] = -1;

                for (k = 0; k < 7; k++) {
                    if (boos[k] != -1 && bwgt[k] >= wgt) {
                        if (rck[0] == -1) {
                            rck[0] = bdex[k];
                        } else if (boos[k] >= boos[rck[0]]) {
                            rck[0] = bdex[k];
                        }
                    }
                }

                if (rck[0] == -1) {
                    ScrubMission(plr, i - Data->P[plr].Mission[i].part);
                } else {
                    if (Data->P[plr].Mission[i].MissionCode == Mission_Orbital_Satellite) {
                        rck[0] = 0;
                    }

                    if (Data->P[plr].Mission[i].MissionCode >= Mission_LunarFlyby &&
                        Data->P[plr].Mission[i].MissionCode <= Mission_SaturnFlyby) {
                        rck[0] = 1;
                    }

                    if (Data->P[plr].Mission[i].MissionCode == Mission_U_SubOrbital) {
                        rck[0] = 1;
                    }

                    if (Data->P[plr].Mission[i].MissionCode == Mission_U_Orbital_D) {
                        rck[0] = 1;
                    }

                    Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] = rck[0] + 1;
                }
            } else {
                // Clear Mission
                Data->P[plr].Mission[i].MissionCode = Mission_None;
            }

            // joint mission part
            if (whe[1] > 0 && Data->P[plr].Mission[i + 1].part == 1) {
                if (Data->P[plr].Mission[i].Prog == 0) {
                    BuildVAB(plr, Data->P[plr].Mission[i].MissionCode, 1, 1, Data->P[plr].Mission[i].Prog);
                } else {
                    BuildVAB(plr, Data->P[plr].Mission[i].MissionCode, 1, 1, Data->P[plr].Mission[i].Prog - 1);
                }

                for (j = Mission_Capsule ; j <= Mission_Probe_DM; j++) {
                    Data->P[plr].Mission[i + 1].Hard[j] = VAS[whe[1]][j].dex;
                }

                wgt = 0;

                for (j = 0; j < 4; j++) {
                    wgt += VAS[whe[1]][j].wt;
                }

                rck[1] = -1;

                for (k = 0; k < 7; k++) {
                    if (boos[k] != -1 && bwgt[k] >= wgt) {
                        if (rck[1] == -1) {
                            rck[1] = bdex[k];
                        } else if (boos[k] >= boos[rck[1]]) {
                            rck[1] = bdex[k];
                        }
                    }
                }

                if (rck[1] == -1) {
                    rck[1] = Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] - 1;
                }

                Data->P[plr].Mission[i + 1].Hard[Mission_PrimaryBooster] = rck[1] + 1;
            }
        }
    }

// JOINT MISSION KLUDGE MISSION Mission_Jt_LunarLanding_EOR & Mission_Jt_LunarLanding_LOR
    if (Data->P[plr].Mission[0].MissionCode == Mission_Jt_LunarLanding_EOR) {
        Data->P[plr].Mission[1].Hard[Mission_Capsule] = Data->P[plr].Mission[1].Prog - 1;
        Data->P[plr].Mission[0].Hard[Mission_LM] = 6; // LM
        Data->P[plr].Mission[0].Hard[Mission_Probe_DM] = 4; // DM
        Data->P[plr].Misc[MISC_HW_KICKER_B].Safety = MAX(Data->P[plr].Misc[MISC_HW_KICKER_B].Safety, Data->P[plr].Misc[MISC_HW_KICKER_B].MaxRD);
        Data->P[plr].Mission[1].Hard[Mission_Kicker] = 1; // kicker second part
    };

    if (Data->P[plr].Mission[0].MissionCode == Mission_Jt_LunarLanding_LOR) {
        Data->P[plr].Mission[1].Hard[Mission_Capsule] = Data->P[plr].Mission[1].Prog - 1;
        Data->P[plr].Mission[0].Hard[Mission_LM] = 6; // LM
        Data->P[plr].Mission[0].Hard[Mission_Probe_DM] = 4; // DM
        Data->P[plr].Misc[MISC_HW_KICKER_B].Safety = MAX(Data->P[plr].Misc[MISC_HW_KICKER_B].Safety, Data->P[plr].Misc[MISC_HW_KICKER_B].MaxRD);
        Data->P[plr].Mission[0].Hard[Mission_Kicker] = 1;
        Data->P[plr].Mission[1].Hard[Mission_Kicker] = 1;
    };

    // lunar module kludge
    for (i = 0; i < 3; i++) {
        if (Data->P[plr].Mission[i].Hard[Mission_LM] >= 5) {
            Data->P[plr].Mission[i].Hard[Mission_LM] = Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE].Safety >= Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Safety ? 5 : 6;
        }
    }

    JR = 0;
    k = 0;

    for (l = 0; l < 3; l++) {
        if (Data->P[plr].Mission[l].Joint == 1) {
            JR = 1;
        }

        if (Data->P[plr].Mission[l].MissionCode &&
            Data->P[plr].Mission[l].part == 0) {
            k++;
        }

        Data->P[plr].Mission[l].Rushing = 0; // Clear Data
    }

    if (k == 3) { // Three non-joint missions
        Data->P[plr].Mission[0].Month = 2 + Data->Season * 6;
        Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
        Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
    };

    if (k == 2 && JR == 0) { // Two non-joint missions
        l = 3;

        if (Data->P[plr].Mission[0].MissionCode) {
            Data->P[plr].Mission[0].Month = l + Data->Season * 6;
            l += 2;
        };

        if (Data->P[plr].Mission[1].MissionCode) {
            Data->P[plr].Mission[1].Month = l + Data->Season * 6;
            l += 2;
        };

        if (Data->P[plr].Mission[2].MissionCode) {
            Data->P[plr].Mission[2].Month = l + Data->Season * 6;
        }
    };

    if (k == 1 && JR == 0) { // Single Mission Non-joint
        if (Data->P[plr].Mission[0].MissionCode) {
            Data->P[plr].Mission[0].Month = 4 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[1].MissionCode) {
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[2].MissionCode) {
            Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
        }
    };

    if (k == 2 && JR == 1) { // Two launches, one Joint;
        if (Data->P[plr].Mission[1].part == 1) { // Joint first
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        };

        if (Data->P[plr].Mission[2].part == 1) { // Joint second
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 5 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        };
    };

    if (k == 1 && JR == 1) { //  Single Joint Launch
        if (Data->P[plr].Mission[1].part == 1) { // found on pad 1+2
            Data->P[plr].Mission[0].Month = 4 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
        } else {   // found on pad 2+3
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
        };
    }

    return;
}

/* EOF */

