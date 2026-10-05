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
#include "mission_util.h"
#include "state_utils.h"
#include "vab.h"

LOG_DEFAULT_CATEGORY(mission)

struct {
    int16_t cost, sf, i;
} Mew[5];
int whe[2], rck[2];
char pc[2], bc[2], Alt_A[2] = {0, 0}, Alt_B[2] = {0, 0};
void Strategy_One(char plr, int* m_1, int* m_2, int* m_3);
void Strategy_Two(char plr, int* m_1, int* m_2, int* m_3);
void Strategy_Thr(char plr, int* m_1, int* m_2, int* m_3);

void AIVabCheck(char plr, char mis, char prog);
char Best();
int ICost(char plr, char h, char i);
void CalcSaf(char plr, char vs);
char Panic_Level(char plr, int* m_1, int* m_2);




void AIVabCheck(char plr, char mis, char prog)
{
    VASqty = 0;
    //prog=1; 0=UnM : 1=1Mn ...
    const mStr plan = GetMissionPlan(mis);
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
}

char Best()
{
    char valid[5]{};

    for (int i = 1; i < VASqty + 1; i++) {
        int ct = 0;
        int ct1 = 0;

        for (int j = 0; j < 4; j++) {
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

    int ct1 = 0;
    for (int i = 1; i < VASqty + 1; i++) {
        ct1 = (valid[i] > valid[ct1]) ? i : ct1;
    }

    return ct1;
}


int ICost(char plr, char h, char i)
{
    auto& pData = Data->P[plr];
    int cost = 0;

    auto next_price = [](auto& part) -> int {
        if (part.Num < 0) return part.InitCost;
        return part.UnitCost;
    };
    
    switch (h) {
    case Mission_Capsule:
    case Mission_LM:
        {
        auto& MannedCapsule = pData.Manned[i];
        cost += MannedCapsule.MaxRD - MannedCapsule.Safety;
        cost /= 3.5;
        cost *= MannedCapsule.RDCost;
        cost += next_price(MannedCapsule);
        break;
        }

    case Mission_Kicker:
        {
        auto& Kicker = pData.Misc[i];
        cost += Kicker.MaxRD - Kicker.Safety;
        cost /= 3.5;
        cost *= Kicker.RDCost;
        cost += next_price(Kicker);
        break;
        }

    case Mission_Probe_DM:
        if (i < 4) {
            auto& Probe = pData.Probe[i];
            cost += Probe.MaxRD - Probe.Safety;
            cost /= 3.5;
            cost *= Probe.RDCost;
            cost += next_price(Probe);
        } else {
            auto& DockingModule = pData.Misc[MISC_HW_DOCKING_MODULE];
            cost += next_price(DockingModule);
        }
        break;

    default:
        break;
    }

    return cost;
}


void CalcSaf(char plr, char vs)
{
    for (int i = 0; i < 5; i++) {
        Mew[i].cost = Mew[i].sf = 0;    // Clear thing
    }

    // Do first part

    for (int j = 1; j < vs + 1; j++) {
        int t = 0;
        for (int k = 0; k < 4; k++) {
            if (VAS[j][k].qty >= 0) {
                Mew[j].sf += VAS[j][k].sf;
            }

            if (strncmp("NONE", &VAS[j][k].name[0], 4) != 0) {
                t++;
            }

            if (VAS[j][k].wt > 0)  {
                Mew[j].cost += ICost(plr, k, VAS[j][k].dex);
            }
        }
        
        if (t > 0) {
            Mew[j].i = Mew[j].sf / t;
        }
    }
}

char Panic_Level(char plr, int* m_1, int* m_2)
{
    auto& pData = Data->P[plr];
// PANIC level manned docking/EVA/duration
    if (Alt_B[plr] <= 1 &&
        pData.AIStrategy[AI_END_STAGE_LOCATION] == 4 &&
        PrestigeCheck(plr, Prestige_MannedDocking) == 0 &&
        PrestigeCheck(plr, Prestige_Spacewalk) == 0 &&
        pData.Mission[0].MissionCode != Mission_U_Orbital_D &&
        pData.Mission[1].MissionCode != Mission_Manned_Orbital_Docking_EVA
       ) {
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_Manned_Orbital_Docking_EVA;
        ++Alt_B[plr];
        return 1;
    }

// PANIC lunar pass/probe landing/lunar flyby
    if (pData.AIStrategy[AI_END_STAGE_LOCATION] == 5 &&
        !PrestigeCheck(plr, Prestige_LunarFlyby) &&
        !PrestigeCheck(plr, Prestige_LunarProbeLanding) &&
        Cur_Status == Ahead &&
        Alt_A[plr] <= 2
       ) {
        *m_1 = Mission_LunarFlyby;
        *m_2 = (pData.DurationLevel <= 2)? Mission_Orbital_Duration
                                         : Mission_Lunar_Probe;
        ++Alt_A[plr];
        return 1;
    }

    // PANIC level duration/pass/lunar orbital/LM_pts
    return 0;
}

void Strategy_One(char plr, int* m_1, int* m_2, int* m_3)
{
//AI version 12/26/92
    auto& pData = Data->P[plr];
    switch (pData.AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:// mission 26 -> if manned docking and eva  -> DurationLevel+1
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = (PrestigeCheck(plr, Prestige_Spacewalk) == 0)? Mission_Manned_Orbital_Docking_EVA
                                                            : Mission_Orbital_Docking;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        }

        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        break;

    case 3:
        *m_1 = Mission_Orbital_Docking_Duration;

        if (Cur_Status == Behind) {
            *m_2 = Mission_Orbital_Duration;
        }

        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }

        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (pData.DurationLevel) {
        case 0: case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (pData.Probe[PROBE_HW_INTERPLANETARY].Safety >= pData.Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby 
                                                                                                                    : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3: case 4: case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (pData.Cash <= 0) {
            pData.Cash = 0;
        }

        pData.Cash += pData.Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

        if (pData.Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            pData.AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        pData.Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5: //lunar pass
        *m_1 = Mission_LunarPass;
        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6: //lunar orbit
        *m_1 = (pData.Manned[MANNED_HW_ONE_MAN_MODULE].Safety > pData.Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10)? Mission_Lunar_Orbital
                                                                                                                  : Mission_LunarOrbital; 
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            pData.AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && pData.Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (pData.LMpts == 0 && pData.Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (pData.LMpts) {
            case 0: case 1:
                *m_1 = (pData.Mission[0].MissionCode == Mission_Lunar_Orbital)? Mission_HistoricalLanding
                                                                              : Mission_Lunar_Orbital;
                break;

            case 2: case 3:
                *m_1 = Mission_HistoricalLanding;
                break;

            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;
            if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety < 60) { // bug?
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }
        break;

    default:
        break;
    }
}

void Strategy_Two(char plr, int* m_1, int* m_2, int* m_3)
{
// AI version 12/28/92
    auto& pData = Data->P[plr];
    switch (pData.AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_Orbital_Docking;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
        }
        
        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 3:
        *m_1 = Mission_Orbital_Duration;
        *m_2 = Mission_Orbital_Docking_Duration;
        *m_3 = (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10)? Mission_Lunar_Probe
                                                                                            : Mission_LunarFlyby;

        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (pData.DurationLevel) {
        case 0: case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            *m_3 = Mission_LunarFlyby;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (pData.Probe[PROBE_HW_INTERPLANETARY].Safety >= pData.Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby 
                                                                                                                    : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3: case 4: case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (pData.Cash <= 0) {
            pData.Cash = 0;
        }

        pData.Cash += pData.Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

        if (pData.Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            pData.AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        pData.Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5: //lunar pass
        *m_1 = Mission_LunarPass;
        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6: //lunar orbit
        if (pData.Manned[MANNED_HW_ONE_MAN_MODULE].Safety > pData.Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10) {
            *m_1 = Mission_LunarOrbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            pData.AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && pData.Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (pData.LMpts == 0 && pData.Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (pData.LMpts) {
            case 0: case 1:
                *m_1 = (pData.Mission[0].MissionCode == Mission_Lunar_Orbital)? Mission_HistoricalLanding
                                                                              : Mission_Lunar_Orbital;
                break;

            case 2: case 3:
                *m_1 = Mission_HistoricalLanding;
                break;

            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;
            if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety < 60) { // bug?
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }
        break;

    default:
        break;
    }
}

void Strategy_Thr(char plr, int* m_1, int* m_2, int* m_3)
{
//new version undated
    auto& pData = Data->P[plr];
    switch (pData.AIStrategy[AI_END_STAGE_LOCATION]) {
    case 0:// mission 26 -> if manned docking and eva  -> DurationLevel+1
        *m_1 = Mission_U_Orbital_D;
        *m_2 = Mission_U_Orbital_D;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 1:
        *m_1 = Mission_U_Orbital_D;
        *m_2 = (PrestigeCheck(plr, Prestige_Spacewalk) == 0)? Mission_Manned_Orbital_Docking_EVA
                                                            : Mission_Orbital_Docking;
        *m_3 = Mission_LunarFlyby;
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 2:
        if (PrestigeCheck(plr, Prestige_MannedDocking) && PrestigeCheck(plr, Prestige_Spacewalk)) {
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_Docking_Duration;
        } else {
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_Manned_Orbital_Docking_EVA;
        }

        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 3:
        *m_1 = Mission_Orbital_Docking_Duration;
        if (Cur_Status == Behind) {
            *m_2 = Mission_Orbital_Duration;
        }
        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 10) {
            *m_3 = Mission_Lunar_Probe;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 4:
        switch (pData.DurationLevel) {
        case 0: case 1:
            *m_1 = Mission_Orbital_Duration;
            *m_2 = Mission_Orbital_EVA_Duration;
            break;

        case 2:
            *m_1 = Mission_Orbital_Docking_Duration;
            *m_2 = (pData.Probe[PROBE_HW_INTERPLANETARY].Safety >= pData.Probe[PROBE_HW_INTERPLANETARY].MaxRD - 10) ? Mission_LunarFlyby 
                                                                                                                    : Mission_U_Orbital_D;
            *m_3 = Mission_LunarFlyby;
            break;

        case 3: case 4: case 5:
            *m_1 = Mission_U_Orbital_D;
            *m_2 = Mission_LunarFlyby;
            *m_3 = Mission_Lunar_Probe;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        default:
            break;
        }

        if (pData.Cash <= 0) {
            pData.Cash = 0;
        }

        pData.Cash += pData.Rocket[ROCKET_HW_THREE_STAGE].InitCost + 25;

        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

        if (pData.Rocket[ROCKET_HW_THREE_STAGE].Num >= 0) {
            pData.AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
        }

        pData.Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        break;

    case 5: //lunar pass
        *m_1 = Mission_LunarPass;
        if (Cur_Status == Behind) {
            *m_2 = Mission_LunarOrbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 6: //lunar orbit
        if (pData.Manned[MANNED_HW_ONE_MAN_MODULE].Safety > pData.Manned[MANNED_HW_ONE_MAN_MODULE].MaxRD - 10) {
            *m_1 = Mission_LunarOrbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 7:
        if (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0) {
            *m_1 = Mission_LunarPass;
            pData.AIStrategy[AI_END_STAGE_LOCATION] = 6;
        } else if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0 && pData.Mission[0].MissionCode != Mission_LunarOrbital) {
            *m_1 = Mission_LunarOrbital;
        } else {
            *m_1 = Mission_Lunar_Orbital;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 8:
        if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
            if (Cur_Status == Behind) {
                *m_1 = Mission_Lunar_Orbital;
            } else {
                *m_1 = Mission_LunarOrbital;
            }
        } else if (pData.LMpts == 0 && pData.Mission[0].MissionCode != Mission_Lunar_Orbital) {
            *m_1 = Mission_Lunar_Orbital;
        } else {
            *m_1 = Mission_HistoricalLanding;
        }
        ++pData.AIStrategy[AI_END_STAGE_LOCATION];
        break;

    case 9:
        if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety >= 80) {
            switch (pData.LMpts) {
            case 0: case 1:
                *m_1 = (pData.Mission[0].MissionCode == Mission_Lunar_Orbital)? Mission_HistoricalLanding
                                                                              : Mission_Lunar_Orbital;
                break;

            case 2: case 3:
            default:
                *m_1 = Mission_HistoricalLanding;
                break;
            }
        } else {
            *m_1 = Mission_U_Orbital_D;

            if (pData.Misc[MISC_HW_DOCKING_MODULE].Safety < 60) { // bug?
                *m_2 = Mission_Orbital_Docking;
            } else {
                *m_2 = Mission_U_Orbital_D;
            }
        }
        break;

    default:
        break;
    }
}

void NewAI(char plr, char frog)
{
    auto& pData = Data->P[plr];
    char hsf, spc[2]{}, prg[2]{}, primaryPad, secondaryPad, Panic_Check = 0;
    int mis1, mis2, mis3, val;

    prg[0] = frog;
    mis1 = mis2 = mis3 = Mission_None;
    primaryPad = secondaryPad = PAD_NONE;
    GenPur(plr, MANNED_HARDWARE, frog - 1);

    if (pData.AILunar < 4) {
        hsf = 0;

        for (int i = 0; i < 3; i++) {
            if (pData.Probe[hsf].Safety <= pData.Probe[i].Safety) {
                hsf = i;
            }
        }

        RDafford(plr, PROBE_HARDWARE, hsf);

        if (pData.Probe[hsf].Safety < 90) {
            GenPur(plr, PROBE_HARDWARE, hsf);
            RDafford(plr, PROBE_HARDWARE, hsf);
        }

        pData.Misc[MISC_HW_DOCKING_MODULE].Num = 2;
        Panic_Check = Panic_Level(plr, &mis1, &mis2);

        if (!Panic_Check) {
            if (pData.AIStrategy[AI_STRATEGY] == 1) {
                Strategy_One(plr, &mis1, &mis2, &mis3);
            } else if (pData.AIStrategy[AI_STRATEGY] == 2) {
                Strategy_Two(plr, &mis1, &mis2, &mis3);
            } else {
                Strategy_Thr(plr, &mis1, &mis2, &mis3);
            }

            if (mis1 == Mission_HistoricalLanding) {
                switch (pData.AILunar) {
                case 1:
                    mis1 = Mission_HistoricalLanding; //Apollo behind Gemini
                    if (frog != 2) break;
                    if (pData.AISec != Mission_Lunar_Probe && pData.AISec != Mission_VenusFlyby) break;
                    
                    val = pData.AISec;

                    if (val < 7) {
                        val = val - 4;
                    } else {
                        val = val - 5;
                    }

                    if (pData.Manned[val - 1].Safety >= pData.Manned[val - 1].MaxRD - 10) {
                        mis2 = Mission_HistoricalLanding;
                        spc[0] = val;
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
        }
    } else {
        switch (pData.AIStrategy[AI_END_STAGE_LOCATION]) {
        case 0:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_Manned_Orbital_Docking_EVA;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 1:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_Orbital_Duration;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 2:
            if (pData.Cash <= 0) {
                pData.Cash = 0;
            }

            pData.Cash += pData.Rocket[ROCKET_HW_MEGA_STAGE].InitCost + 25;

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            if (pData.Rocket[ROCKET_HW_MEGA_STAGE].Num >= 0) {
                pData.AIStrategy[AI_LARGER_ROCKET_STRATEGY] = 1;
            }

            pData.Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            mis1 = Mission_Orbital_Duration;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 3:
            switch (pData.DurationLevel) {
            case 0: case 1:
                mis1 = Mission_Orbital_Duration;
                mis2 = Mission_Orbital_Duration;
                break;

            case 2:
                mis1 = Mission_Orbital_Duration;
                mis2 = Mission_LunarFlyby;
                ++pData.AIStrategy[AI_END_STAGE_LOCATION];
                break;

            case 3: case 4: case 5:
                mis1 = Mission_LunarFlyby;
                mis2 = Mission_LunarFlyby;
                ++pData.AIStrategy[AI_END_STAGE_LOCATION];
                break;

            default:
                break;
            }

            break;

        case 4:
            mis1 = Mission_Orbital_Duration;
            mis2 = Mission_LunarFlyby;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 5:
            switch (pData.DurationLevel) {
            case 0: case 1: case 2:
                mis1 = Mission_Orbital_Duration;
                break;

            case 3:
                mis1 = (PrestigeCheck(plr, Prestige_MannedOrbital) == 0) ? Mission_Orbital_EVA_Duration 
                                                                         : Mission_Orbital_Duration;
                break;

            case 4: case 5:
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
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 6:
            mis1 = Mission_LunarPass;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 7:
            mis1 = (PrestigeCheck(plr, Prestige_MannedLunarPass) == 0)? Mission_LunarPass
                                                                      : Mission_LunarOrbital;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 8:
            mis1 = Mission_LunarOrbital;
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];
            break;

        case 9:
            if (PrestigeCheck(plr, Prestige_MannedLunarOrbit) == 0) {
                mis1 = Mission_LunarOrbital;
            }
            ++pData.AIStrategy[AI_END_STAGE_LOCATION];

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
        if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 15) {
            mis2 = Mission_Lunar_Probe;
        }

    const mStr plan = GetMissionPlan(mis1);

// deal with lunar modules
    if (plan.LM == 1) {
        if (pData.AIStrategy[AI_LUNAR_MODULE] > 0) {
            GenPur(plr, MANNED_HARDWARE, pData.AIStrategy[AI_LUNAR_MODULE]);
            RDafford(plr, MANNED_HARDWARE, pData.AIStrategy[AI_LUNAR_MODULE]);
        } else {
            pData.AIStrategy[AI_LUNAR_MODULE] = 6;

            GenPur(plr, MANNED_HARDWARE, pData.AIStrategy[AI_LUNAR_MODULE]);
            RDafford(plr, MANNED_HARDWARE, pData.AIStrategy[AI_LUNAR_MODULE]);
        }
    }

    if (plan.Jt == 1) {
        // JOINT LAUNCH
        if (pData.Future[0].MissionCode == Mission_None && pData.LaunchFacility[0] == LAUNCHPAD_OPERATIONAL &&
            pData.Future[1].MissionCode == Mission_None && pData.LaunchFacility[1] == LAUNCHPAD_OPERATIONAL) {
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

        if (pData.Future[0].MissionCode == Mission_None && pData.LaunchFacility[0] == LAUNCHPAD_OPERATIONAL) {
            primaryPad = PAD_A;
        }

        if (pData.Future[1].MissionCode == Mission_None && pData.LaunchFacility[1] == LAUNCHPAD_OPERATIONAL) {
            if (primaryPad == PAD_A) {
                secondaryPad = PAD_B;
            } else {
                primaryPad = PAD_B;
            }
        }

        if (pData.Future[2].MissionCode == Mission_None && pData.LaunchFacility[2] == LAUNCHPAD_OPERATIONAL) {
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

    if (pData.Future[2].MissionCode == Mission_None 
        && pData.LaunchFacility[2] == LAUNCHPAD_OPERATIONAL) 
    {
        auto& ThreeMan = pData.Manned[MANNED_HW_THREE_MAN_CAPSULE];
        auto& Minishuttle = pData.Manned[MANNED_HW_MINISHUTTLE];
        if ( (mis1 == 0 && frog == 2 
              && (ThreeMan.Safety >= ThreeMan.MaxRD - 10)
             ) 
            || 
             (Minishuttle.Safety >= Minishuttle.MaxRD - 10)
           ) {
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

                if (pData.Probe[PROBE_HW_LUNAR].Safety > pData.Probe[PROBE_HW_LUNAR].MaxRD - 15) {
                    if (PrestigeCheck(plr, Prestige_LunarProbeLanding) == 0 
                        || pData.Misc[MISC_HW_PHOTO_RECON].Safety < 85) {
                        if (mis3 == Mission_None) {
                            mis3 = Mission_Lunar_Probe;
                        }
                    }
                }

                if ((pData.Probe[PROBE_HW_INTERPLANETARY].Safety > pData.Probe[PROBE_HW_INTERPLANETARY].MaxRD - 15) 
                    && mis3 == Mission_None
                   ) {
                    int prest[] = {
                        Prestige_LunarFlyby,Prestige_MercuryFlyby,Prestige_VenusFlyby
                       ,Prestige_MarsFlyby,Prestige_JupiterFlyby,Prestige_SaturnFlyby
                    };
                    int miss[] = {
                        Mission_LunarFlyby, Mission_MercuryFlyby, Mission_VenusFlyby
                       ,Mission_MarsFlyby, Mission_JupiterFlyby, Mission_SaturnFlyby
                    };
                    for (int i=0; i < sizeof(prest)/sizeof(prest[0]); ++i) {
                        if (PrestigeCheck(plr, prest[i]) != 0) continue;
                        if (PrestigeCheck(other(plr), prest[i]) != 0) continue;
                        if (pData.Mission[2].MissionCode == miss[i]) continue;
                        
                        mis3 = miss[i];
                        break;
                    }
                    if (mis3 == Mission_None) {
                        for (int i=0; i < sizeof(prest)/sizeof(prest[0]); ++i) {
                            if (PrestigeCheck(plr, prest[i]) != 0) continue;
                            if (PrestigeCheck(other(plr), prest[i]) != 1) continue;
                            if (pData.Mission[2].MissionCode == miss[i]) continue;
                            
                            mis3 = miss[i];
                            break;
                        }
                    }
                }
            }

        if (mis3 == Mission_None) {
            GenPur(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
            RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);

            if (pData.Probe[PROBE_HW_ORBITAL].Num >= 1 && pData.Rocket[ROCKET_HW_ONE_STAGE].Num >= 1) {
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
}

void AIFuture(char plr, char mis, char pad, char* prog)
{    
    auto& pData = Data->P[plr];
    char fake_prog[2]{};
    if (prog == nullptr) {
        prog = fake_prog;
    }

    if (prog[1] < 0 || prog[1] > 5) {
        prog[1] = prog[0];
    }

    const mStr plan = GetMissionPlan(mis);
    auto& Future = pData.Future;

    for (int i = 0; i < (plan.Jt + 1); i++) {
        auto& launch = Future[pad + i];
        launch.MissionCode = mis;
        launch.part = i;

        // duration
        if (pData.DurationLevel <= 5 && launch.Duration == 0) {
            if (plan.Dur == 1) launch.Duration =
                    MAX(plan.Days, MIN(pData.DurationLevel + 1, 6));
            else {
                launch.Duration = plan.Days;
            }
        }

        if (pData.Mission[0].Duration == launch.Duration ||
            pData.Mission[1].Duration == launch.Duration) {
            ++launch.Duration;
        }

        if (pad == 1 && Future[0].Duration == launch.Duration) {
            ++launch.Duration;
        }

        if (launch.Duration >= 6) {
            launch.Duration = 6;
        }

        // one-man capsule duration kludge
        if (launch.Prog == 1) {
            if (pData.DurationLevel == 0) {
                launch.Duration = 1;
            } else {
                launch.Duration = 2;
            }
        }; // limit duration 'C' one-man capsule

        // lunar mission kludge
        if (plan.Lun == 1 
            || launch.MissionCode == Mission_Jt_LunarLanding_EOR 
            || launch.MissionCode == Mission_Jt_LunarLanding_LOR 
            || launch.MissionCode == Mission_HistoricalLanding) {
            launch.Duration = 4;
        }

        // unmanned duration kludge
        if (plan.Days == 0) {
            launch.Duration = 0;
        }

        launch.Joint = plan.Jt;
        launch.Month = 0;

        if (mis == 1) {
            prog[i] = 0;
        }

        launch.Prog = prog[0];

        if (prog[i] > 0 && plan.Days > 0) {
            for (int j = 1; j < 6; j++) {
                DumpAstro(plr, j);
            }

            TransAstro(plr, prog[i]); //indexed OK

            int primary_crew = -1;
            if (launch.PCrew != 0) {
                primary_crew = launch.PCrew - 1;
            }

            int backup_crew = -1;
            if (launch.BCrew != 0) {
                backup_crew = launch.BCrew - 1;
            }

            int max = prog[i];

            if (prog[i] > 3) {
                max = prog[i] - 1;
            }

            launch.Men = max;
            int men = launch.Men;

            if (primary_crew != -1) {
                for (int j = 0; j < men; j++) {
                    int pool_idx = pData.Crew[prog[i]][primary_crew][j] - 1;
                    pData.Pool[pool_idx].Prime = 0;
                }
            }

            if (backup_crew != -1) {
                for (int j = 0; j < men; j++) {
                    int pool_idx = pData.Crew[prog[i]][backup_crew][j] - 1;
                    pData.Pool[pool_idx].Prime = 0;
                }
            }

            launch.PCrew = 0;
            launch.BCrew = 0;
            bc[i] = -1;

            pc[i] = -1;
            for (int j = 0; j < 8; j++) {
                if (pData.Crew[prog[i]][j][0] == 0) continue;
                if (pData.Pool[pData.Crew[prog[i]][j][0] - 1].Prime != 0) continue;
                
                pc[i] = j;
                break;
            }

            if (pc[i] == -1) {
                // astronaut/duration kludge
                if (plan.Days > 0) {
                    launch.Men = max;
                }

                // no astronauts available have to go unmanned
                launch.Men = 0;
                launch.PCrew = 0;
                launch.BCrew = 0;

                Downgrader::Options options = LoadJsonDowngrades("DOWNGRADES.JSON");
                Downgrader replace{launch, options};
                char mcode = -1;

                //  Find a mission that can be flown unmanned
                try {
                    std::vector<mStr> missionData = GetMissionData();

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

                    LOG_TRACE("AI replacing mission code %i by %i", launch.MissionCode, mcode);
                    launch.MissionCode = mcode;

                } catch (IOException &err) {
                    // TODO: Can't download to Earth Orbital if Joint mission.
                    LOG_CRITICAL("Error loading data file: %s", err.what());
                    LOG_WARNING("Defaulting to Unmanned Earth Orbital.");
                    launch.MissionCode = Mission_Unmanned_Earth_Orbital;
                }
                return;
            }

            launch.PCrew = pc[i] + 1;
            
            bc[i] = -1;
            for (int j = 0; j < 8; j++) {
                if (j == pc[i]) continue;
                if (pData.Crew[prog[i]][j][0] == 0) continue;
                if (pData.Pool[pData.Crew[prog[i]][j][0] - 1].Prime != 0) continue;
                
                bc[i] = j;
                break;
            }

            launch.BCrew = bc[i] + 1;

            for (int j = 0; j < men; j++) {
                pData.Pool[pData.Crew[prog[i]][pc[i]][j] - 1].Prime = 4;
            }

            for (int j = 0; j < men; j++) {
                pData.Pool[pData.Crew[prog[i]][bc[i]][j] - 1].Prime = 2;
            }
        } else {
            launch.Men = 0;
            launch.PCrew = 0;
            launch.BCrew = 0;
        }
    }

// joint mission Mission_Jt_LunarLanding_EOR and Mission_Jt_LunarLanding_LOR men kludge
    if (mis == Mission_Jt_LunarLanding_EOR || mis == Mission_Jt_LunarLanding_LOR) {
        Future[pad + 1].Men = Future[pad].Men;
        Future[pad + 1].PCrew = Future[pad].PCrew;
        Future[pad + 1].BCrew = Future[pad].BCrew;
        Future[pad + 1].Prog = Future[pad].Prog;
        Future[pad + 1].Duration = Future[pad].Duration;
        
        Future[pad].Men = 0;
        Future[pad].PCrew = 0;
        Future[pad].BCrew = 0;
        Future[pad].Prog = 0;
        Future[pad].Duration = 0;
    }
}

void AILaunch(char plr)
{
    auto& pData = Data->P[plr];
    int bwgt[7];
    char boos[7]; // safety of first stage combination?
    
    for (int i = 0; i < 7; i++) {
        auto& RocketData = pData.Rocket;
        
        boos[i] = (i > 3) ?
                  RocketBoosterSafety(RocketData[i - 4].Safety, RocketData[ROCKET_HW_BOOSTERS].Safety)
                  : RocketData[i].Safety;
        bwgt[i] = (i > 3) ?
                  (RocketData[i - 4].MaxPay + RocketData[ROCKET_HW_BOOSTERS].MaxPay)
                  : RocketData[i].MaxPay;

        if (boos[i] < 60) {
            boos[i] = -1;    // Get Rid of any Unsafe rocket systems
        }

        if (RocketData[ROCKET_HW_BOOSTERS].Num < 1) {
            for (int j = 4; j < 7; j++) {
                boos[j] = -1;
            }
        }

        for (int j = 0; j < 4; j++) {
            if (RocketData[j].Num < 1) {
                boos[j] = -1;
            }
        }
    }

    // iterate over planned launches
    for (int i = 0; i < 3; i++) {
        auto& PlannedMission = pData.Mission[i];
        if (PlannedMission.MissionCode == Mission_Orbital_DockingInOrbit_Duration 
            && pData.DockingModuleInOrbit == 0
           ) {
            PlannedMission.MissionCode = Mission_None;
            continue;
        }

        if (PlannedMission.MissionCode == Mission_None || PlannedMission.part != 0) continue;
        
        whe[0] = whe[1] = -1;

        if (PlannedMission.Joint == 1) {
            auto& JoinedMissionSecond = pData.Mission[i + 1];
            AIVabCheck(plr, PlannedMission.MissionCode, JoinedMissionSecond.Prog);
        } else {
            AIVabCheck(plr, PlannedMission.MissionCode, PlannedMission.Prog);
        }

        if (whe[0] > 0) {
            BuildVAB(plr, PlannedMission.MissionCode, 1, 0, (PlannedMission.Prog == 0)? PlannedMission.Prog
                                                                                      : PlannedMission.Prog - 1);

            for (int j = Mission_Capsule; j <= Mission_Probe_DM; j++) {
                PlannedMission.Hard[j] = VAS[whe[0]][j].dex;
            }

            int wgt = 0;
            for (int j = 0; j < 4; j++) {
                wgt += VAS[whe[0]][j].wt;
            }

            rck[0] = -1;

            for (int k = 0; k < 7; k++) {
                if (boos[k] == -1) continue;
                if (bwgt[k] < wgt) continue;
                
                if (rck[0] == -1) {
                    rck[0] = k;
                } else if (boos[k] >= boos[rck[0]]) {
                    rck[0] = k;
                }
            }

            if (rck[0] == -1) {
                ScrubMission(plr, i - PlannedMission.part);
            } else {
                if (PlannedMission.MissionCode == Mission_Orbital_Satellite) {
                    rck[0] = 0;
                }

                if (PlannedMission.MissionCode >= Mission_LunarFlyby &&
                    PlannedMission.MissionCode <= Mission_SaturnFlyby) {
                    rck[0] = 1;
                }

                if (PlannedMission.MissionCode == Mission_U_SubOrbital) {
                    rck[0] = 1;
                }

                if (PlannedMission.MissionCode == Mission_U_Orbital_D) {
                    rck[0] = 1;
                }

                PlannedMission.Hard[Mission_PrimaryBooster] = rck[0] + 1;
            }
        } else {
            // Clear Mission
            PlannedMission.MissionCode = Mission_None;
        }

        // joint mission part
        if (whe[1] > 0 && pData.Mission[i + 1].part == 1) {
            auto& JoinedMissionSecond = pData.Mission[i + 1];
            BuildVAB(plr, PlannedMission.MissionCode, 1, 1, (PlannedMission.Prog == 0)? PlannedMission.Prog
                                                                                      : PlannedMission.Prog - 1);

            for (int j = Mission_Capsule ; j <= Mission_Probe_DM; j++) {
                JoinedMissionSecond.Hard[j] = VAS[whe[1]][j].dex;
            }

            int wgt = 0;
            for (int j = 0; j < 4; j++) {
                wgt += VAS[whe[1]][j].wt;
            }

            rck[1] = -1;

            for (int k = 0; k < 7; k++) {
                if (boos[k] == -1) continue;
                if (bwgt[k] < wgt) continue;
                
                if (rck[1] == -1) {
                    rck[1] = k;
                } else if (boos[k] >= boos[rck[1]]) {
                    rck[1] = k;
                }
            }

            if (rck[1] == -1) {
                rck[1] = PlannedMission.Hard[Mission_PrimaryBooster] - 1;
            }

            JoinedMissionSecond.Hard[Mission_PrimaryBooster] = rck[1] + 1;
        }
    }
    
    auto& MisData = pData.Mission;

// JOINT MISSION KLUDGE MISSION Mission_Jt_LunarLanding_EOR & Mission_Jt_LunarLanding_LOR
    if (MisData[0].MissionCode == Mission_Jt_LunarLanding_EOR) {
        MisData[1].Hard[Mission_Capsule] = MisData[1].Prog - 1;
        MisData[0].Hard[Mission_LM] = 6; // LM
        MisData[0].Hard[Mission_Probe_DM] = 4; // DM
        pData.Misc[MISC_HW_KICKER_B].Safety = MAX(pData.Misc[MISC_HW_KICKER_B].Safety, pData.Misc[MISC_HW_KICKER_B].MaxRD);
        MisData[1].Hard[Mission_Kicker] = 1; // kicker second part
    };

    if (MisData[0].MissionCode == Mission_Jt_LunarLanding_LOR) {
        MisData[1].Hard[Mission_Capsule] = MisData[1].Prog - 1;
        MisData[0].Hard[Mission_LM] = 6; // LM
        MisData[0].Hard[Mission_Probe_DM] = 4; // DM
        pData.Misc[MISC_HW_KICKER_B].Safety = MAX(pData.Misc[MISC_HW_KICKER_B].Safety, pData.Misc[MISC_HW_KICKER_B].MaxRD);
        MisData[0].Hard[Mission_Kicker] = 1;
        MisData[1].Hard[Mission_Kicker] = 1;
    };

    // lunar module kludge
    for (int i = 0; i < 3; i++) {
        if (MisData[i].Hard[Mission_LM] < 5) continue;
        
        int two_man = pData.Manned[MANNED_HW_TWO_MAN_MODULE].Safety;
        int one_man = pData.Manned[MANNED_HW_ONE_MAN_MODULE].Safety;
        MisData[i].Hard[Mission_LM] = (two_man >= one_man) ? 5 : 6;
    }

    int number_of_missions = 0;
    for (int i = 0; i < 3; i++) {
        if (MisData[i].MissionCode != Mission_None 
            && MisData[i].part == 0) {
            number_of_missions++;
        }

        MisData[i].Rushing = 0; // Clear Data
    }

    int launch_months[][3] = {
        {}, // 0 launches
        {4}, // 1 launch
        {3,5}, // 2 launches
        {2,3,4}, // 3 launches
    };

    for(int mission_idx = -1, pad = 0; pad < 3; ++pad) {
        auto& launch = MisData[pad];
        if (launch.MissionCode == Mission_None) continue;
        if (launch.part == 0) mission_idx++;
        MisData[pad].Month = launch_months[number_of_missions][mission_idx];
    }
}

/* EOF */

