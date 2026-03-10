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
// Programmed by Michael K McCarty and Morgan Roarty
//
/** \file aimast.c AI Master Routines
 */

// This seems to be the master control for the AI, including selecting its strategy (0-2) and primary & secondary lunar approach

#include "aimast.h"
#include "aimis.h"
#include "aipur.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "pace.h"

char Level_Check;
enum Opponent_Status Cur_Status;

// Track[0] - orbital satellite
// Track[1] - end stage location holder
// Track[2] - holds lunar module
// Track[3] - strategy 0=none 1=one 2=two
// Track[4] - beginning game status
// Track[5] - larger rocket (3-stage)
// AILunar  - way to the moon
// AIPrim   - primary program
// AISec    - secondary program


char NoFail(char plr);
void RDPrograms(char plr);
void CheckVoting(char plr);
void KeepRD(char plr, int m);
void PickModule(char plr);
void MoonVoting(char plr);
int CheckSafety(char plr, char m);
void MoonProgram(char plr, char m);
void ProgramVoting(char plr);




void AIMaster(char plr)
{
    auto& PData = Data->P[plr];

    if (plr == 0) {
        Level_Check = (Data->Def.Lev1 == 0) ? 0 : 1;
    } else  if (plr == 1) {
        Level_Check = (Data->Def.Lev2 == 0) ? 0 : 1;
    }

    // Randomly select the AI strategy
    int P_total = brandom(100);

    if (PData.AIStrategy[AI_STRATEGY] == 0) {
        if (P_total < 33) {
            PData.AIStrategy[AI_STRATEGY] = 1;
        } else if (P_total < 66) {
            PData.AIStrategy[AI_STRATEGY] = 2;
        } else {
            PData.AIStrategy[AI_STRATEGY] = 3;
        }
    }

    Cur_Status = Equal;

// *** check status ***
    for (int i = 0; i < PData.PastMissionCount; i++) {
        P_total += PData.History[i].Prestige;
    }

    int O_total = 0;
    for (int i = 0; i < Data->P[other(plr)].PastMissionCount; i++) {
        O_total += Data->P[other(plr)].History[i].Prestige;
    }

    if (P_total > O_total) {
        Cur_Status = Ahead;
    } else if (P_total == O_total) {
        Cur_Status = Equal;
    } else if (P_total < O_total) {
        Cur_Status = Behind;
    }

    if (Data->Year == 59 && Data->Season == 1) {
        PData.AIStrategy[AI_BEGINNING_STRATEGY] = 0;
    }

    if (PData.AstroDelay <= 0) {
        AIAstroPur(plr);
    }

    if (PData.AIStat == 1)
        if (Data->Prestige[Prestige_OrbitalSatellite].Place == plr || Data->Prestige[Prestige_OrbitalSatellite].mPlace == plr) {
            PData.AIStat = 2;
        }

    if (PData.AIStat == 1 && PrestigeCheck(plr, Prestige_LunarFlyby) == 0 && Cur_Status == Behind) {
        PData.AIStat = 2;
    }

    if (PData.AIStat == 2 && Data->Year >= 61 && Data->Season == 0) {
        PData.AIStat = 3;
    }

    if (PData.AIStat <= 2) {
        GenPur(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
        RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);

        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);

        PData.Buy[PROBE_HARDWARE][PROBE_HW_ORBITAL] = 0;
        PData.Buy[ROCKET_HARDWARE][ROCKET_HW_ONE_STAGE] = 0;
        PData.Buy[MANNED_HARDWARE][MANNED_HW_ONE_MAN_CAPSULE] = 0;

        if (Data->Year <= 59) {
            PData.Probe[PROBE_HW_ORBITAL].Num++;
            PData.Rocket[ROCKET_HW_ONE_STAGE].Num++;
            AIFuture(plr, Mission_Orbital_Satellite, 0, 0);
        }

        KeepRD(plr, 5);
        PData.Buy[PROBE_HARDWARE][PROBE_HW_ORBITAL] = 0;
        PData.Buy[ROCKET_HARDWARE][ROCKET_HW_ONE_STAGE] = 0;
        PData.Buy[MANNED_HARDWARE][MANNED_HW_ONE_MAN_CAPSULE] = 0;
        RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);

        if (PData.AIStat < 2) {
            AIPur(plr);
        }

        char prg[2]{1,0};

        if (NoFail(plr) == 0) {
            if ((Data->Year == 59 && Data->Season == 1) || Data->Year >= 60) {
                switch (PData.AIStrategy[AI_BEGINNING_STRATEGY]) {
                case 0:
                    if (PData.Misc[MISC_HW_EVA_SUITS].Safety > PData.Misc[MISC_HW_EVA_SUITS].MaxRD - 20) {
                        AIFuture(plr, Mission_Earth_Orbital_EVA, 0, (char *)&prg);
                    } else {
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    }

                    PData.Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety += 10;
                    PData.Misc[MISC_HW_EVA_SUITS].Safety += 10;
                    ++PData.AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                case 1:
                    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 0) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 0) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 1) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 1) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    }

                    ++PData.AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                case 2:
                    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 0) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 0) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 1) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 1) { // && PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital_EVA, 0, (char *)&prg);    //2
                    } else {
                        PData.AIStat = 3;
                    }

                    ++PData.AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                default:
                    break;
                }

                if (PData.Manned[MANNED_HW_ONE_MAN_CAPSULE].Num >= 2 && PData.Rocket[ROCKET_HW_ONE_STAGE].Num >= 2) {
                    if (PData.Future[0].MissionCode == Mission_SubOrbital) {
                        AIFuture(plr, Mission_Earth_Orbital, 1, (char *)&prg);
                    } else if (PData.Future[0].MissionCode == Mission_Earth_Orbital) {
                        AIFuture(plr, Mission_SubOrbital, 1, (char *)&prg);
                    } else if (PData.Future[0].MissionCode == Mission_Earth_Orbital_EVA) {
                        AIFuture(plr, Mission_Earth_Orbital, 1, (char *)&prg);
                    }
                }
            } // if (Data->Year)
        } // if (NoFail)

        AILaunch(plr);
    }

    GenPur(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);
    RDafford(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);

    PData.Buy[MISC_HARDWARE][MISC_HW_EVA_SUITS] = 0;
    RDafford(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);

    if (PData.AIStat >= 2) {
        CheckVoting(plr); // gets AIPrim,AISec,AILunar

        if (PData.AIPrim == 0) {
            ProgramVoting(plr);
        }
    }

    // primary/secondary programs
    if (PData.AIPrim == 8) {
        PData.AIPrim = 6;
        PData.AISec = 8;
    }

    if (PData.AIPrim == 6) {
        PData.AISec = 8;
    }

    if (PData.AIPrim == 9) {
        PData.AIPrim = 6;
        PData.AISec = 9;
    }

    if (PData.AILunar == 4) {
        PData.AIPrim = 6;
        int two_man = PData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety;
        int four_man = PData.Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety;
        PData.AISec = (two_man >= four_man) ? 6 : 10;
    }

    // boosters
    if (PData.AIStat >= 2) {
        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
    }

    if (CheckSafety(plr, PData.AIPrim) >= CheckSafety(plr, PData.AISec)) {
        KeepRD(plr, PData.AIPrim);
    } else {
        KeepRD(plr, PData.AISec);
    }

    // larger rocket kludge
    if (PData.AIStrategy[AI_LARGER_ROCKET_STRATEGY] == 1) {
        if (Level_Check != 0) {
            PData.Cash += 25;    // temporary
        }

        if (PData.AILunar < 4) {
            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

            PData.Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        } else {
            if (Level_Check != 0) {
                PData.Cash += 25;
            }

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            PData.Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
        }
    }

    if (Data->Year >= 62)
        if (PData.AIStrategy[AI_LUNAR_MODULE] < 5) {
            PickModule(plr);
        }

    if (PData.AILunar > 0 && PData.AILunar < 4) {
        if (PData.AIStrategy[AI_LUNAR_MODULE] > 0) {
            GenPur(plr, MANNED_HARDWARE, PData.AIStrategy[AI_LUNAR_MODULE]);
            RDafford(plr, MANNED_HARDWARE, PData.AIStrategy[AI_LUNAR_MODULE]);

            PData.Buy[MANNED_HARDWARE][PData.AIStrategy[AI_LUNAR_MODULE]] = 0;
            RDafford(plr, MANNED_HARDWARE, PData.AIStrategy[AI_LUNAR_MODULE]);
        }
    }

    for (int i = 0; i < MAX_LAUNCHPADS; i++) {
        if (PData.LaunchFacility[i] >= LAUNCHPAD_DAMAGED_MARGIN) {
            if (PData.LaunchFacility[i] <= PData.Cash) {
                PData.Cash -= PData.LaunchFacility[i];
                PData.LaunchFacility[i] = LAUNCHPAD_OPERATIONAL;
            }
        }
    }

    PData.LaunchFacility[0] = LAUNCHPAD_OPERATIONAL;
    PData.LaunchFacility[1] = LAUNCHPAD_OPERATIONAL;
    PData.LaunchFacility[2] = LAUNCHPAD_OPERATIONAL;

    if (PData.AIStat == 3) {
        switch (PData.AILunar) {
        case 1:
            MoonProgram(plr, 1);
            break;

        case 2:
            MoonProgram(plr, 2);
            break;

        case 3:
            MoonProgram(plr, 3);
            break;

        case 4:
            MoonProgram(plr, 4);

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            break;

        default:
            break;
        }
    }

    if (PData.AIStat >= 2 && Data->Year >= 61 && PData.AIStrategy[AI_END_STAGE_LOCATION] >= 2) {
        if ((PData.Probe[PROBE_HW_ORBITAL].Safety >= PData.Probe[PROBE_HW_ORBITAL].MaxRD - 20) || PData.Probe[PROBE_HW_INTERPLANETARY].Num >= 0) {
            PData.Cash += PData.Probe[PROBE_HW_INTERPLANETARY].InitCost + 30;

            if (PData.Probe[PROBE_HW_INTERPLANETARY].Num <= 0) {
                GenPur(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
                RDafford(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
            }

            PData.Buy[PROBE_HARDWARE][PROBE_HW_INTERPLANETARY] = 0;
            RDafford(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
        }

        if ((PData.Probe[PROBE_HW_INTERPLANETARY].Safety >= PData.Probe[PROBE_HW_INTERPLANETARY].MaxRD - 20) || PData.Probe[PROBE_HW_LUNAR].Num >= 0) {
            PData.Cash += PData.Probe[PROBE_HW_LUNAR].InitCost + 30;

            if (PData.Probe[PROBE_HW_LUNAR].Num <= 0) {
                GenPur(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
                RDafford(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
            }

            PData.Buy[PROBE_HARDWARE][PROBE_HW_LUNAR] = 0;
            RDafford(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
        }

        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
    }

    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) || PrestigeCheck(plr, Prestige_MannedOrbital)) {
        PData.AIStat = 3;
    }

    // **** end stages ***
    if (PData.AIStat == 3) {
        if (PData.AILunar < 4) {
            int val;
            if (CheckSafety(plr, PData.AIPrim) > CheckSafety(plr, PData.AISec)) {
                val = PData.AIPrim;
            } else {
                val = PData.AISec;
            }

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (PData.Manned[val - 1].Safety >= PData.Manned[val - 1].MaxRD - 15) {
                NewAI(plr, val);
            }
        } else if (PData.AILunar == 4) {
            if (PData.Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety >= PData.Manned[MANNED_HW_FOUR_MAN_CAPSULE].MaxRD - 10) {
                PData.AISec = 10;
            } else {
                PData.AISec = 6;
            }

            int val;
            if (CheckSafety(plr, PData.AIPrim) > CheckSafety(plr, PData.AISec)) {
                val = PData.AIPrim;
            } else {
                val = PData.AISec;
            }

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (PData.Manned[val - 1].Safety >= PData.Manned[val - 1].MaxRD - 15) {
                NewAI(plr, val);
            } else {
                PData.Probe[PROBE_HW_ORBITAL].Num += 2;
                PData.Rocket[ROCKET_HW_ONE_STAGE].Num += 2;
                AIFuture(plr, Mission_Orbital_Satellite, 0, 0);
                AIFuture(plr, Mission_Orbital_Satellite, 1, 0);
                AIFuture(plr, Mission_Orbital_Satellite, 2, 0);
            }
        }

        if (CheckSafety(plr, PData.AIPrim) > CheckSafety(plr, PData.AISec)) {
            int val = PData.AIPrim;

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (CheckSafety(plr, PData.AIPrim) > PData.Manned[val - 1].MaxRD - 10)
            {
                if (PData.Manned[val - 1].Num >= 1) {
                    KeepRD(plr, PData.AISec);
                }
            }
        } else {
            int val = PData.AISec;

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (CheckSafety(plr, PData.AISec) > PData.Manned[val - 1].MaxRD - 10)
            {
                if (PData.Manned[val - 1].Num >= 1) {
                    KeepRD(plr, PData.AIPrim);
                }
            }
        }
    }

    RDPrograms(plr);
}


char NoFail(char plr)
{
    char RT_value = 0;

    for (int i = 0; i < Data->P[plr].PastMissionCount; i++) {
        int code = Data->P[plr].History[i].MissionCode;
        if (code != Mission_Earth_Orbital && code != Mission_Earth_Orbital_EVA) 
            continue;
        
        if (Data->P[plr].History[i].spResult >= 3000) {
            ++RT_value;
        }
    }

    if (RT_value >= 2) {
        RT_value = 1;
        Data->P[plr].AIStat = 3;
    }

    return RT_value;
}


void RDPrograms(char plr)
{
    for (int i = 0; i < 7; i++) {
        if (Data->P[plr].Manned[i].Num >= 0) {
            RDafford(plr, MANNED_HARDWARE, i);
        }

        Data->P[plr].Manned[i].Damage = 0;
        Data->P[plr].Manned[i].DCost = 0;
    };

    for (int i = 0; i < 5; i++) {
        if (Data->P[plr].Rocket[i].Num >= 0) {
            RDafford(plr, ROCKET_HARDWARE, i);
        }

        Data->P[plr].Rocket[i].Damage = 0;
        Data->P[plr].Rocket[i].DCost = 0;
    };

    for (int i = 0; i < 4; i++) {
        if (Data->P[plr].Misc[i].Num >= 0) {
            RDafford(plr, MISC_HARDWARE, i);
        }

        Data->P[plr].Misc[i].Damage = 0;
        Data->P[plr].Misc[i].DCost = 0;
    };

    for (int i = 0; i < 3; i++) {
        if (Data->P[plr].Probe[i].Num >= 0) {
            RDafford(plr, PROBE_HARDWARE, i);
        }

        Data->P[plr].Probe[i].Damage = 0;
        Data->P[plr].Probe[i].DCost = 0;
    };
}

void CheckVoting(char plr)
{
    if (Data->P[plr].AILunar == 0) {
        MoonVoting(plr);
    } else if (Data->P[plr].AILunar == 4) {
        if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE) == 1) {
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            Data->P[plr].AILunar = 4; // lunar landing set
        }
    }
}

void KeepRD(char plr, int m)
{
//reassessing player level
    if (plr == 0) {
        Level_Check = (Data->Def.Lev1 == 0) ? 0 : 1;
    } else  if (plr == 1) {
        Level_Check = (Data->Def.Lev2 == 0) ? 0 : 1;
    }

    if (m < 4) {
        return;    // hasn't voted yet
    }

    switch (m) {
    case 5:
        if (Data->P[plr].Rocket[ROCKET_HW_ONE_STAGE].Num <= Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Num) {
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            }

            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
            }

            RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
        } else {
            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            }

            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);

        }

        break;

    case 6:
        if (Data->P[plr].Rocket[ROCKET_HW_TWO_STAGE].Num <= Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Num) {
            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);

            GenPur(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_TWO_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_CAPSULE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            }
        } else {
            GenPur(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_TWO_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_CAPSULE] = 0;
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            }
        }

        break;

    case 8:
        if (Data->P[plr].Rocket[ROCKET_HW_THREE_STAGE].Num <= Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Num) {
            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

            GenPur(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_THREE_MAN_CAPSULE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            }
        } else {
            GenPur(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_THREE_MAN_CAPSULE] = 0;
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            }
        }

        break;

    case 9:
        if (Data->P[plr].Rocket[ROCKET_HW_MEGA_STAGE].Num <= Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Num) {
            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            GenPur(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_MINISHUTTLE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }
        } else {
            GenPur(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_MINISHUTTLE] = 0;
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            }
        }

        break;

    case 10:
        if (Data->P[plr].Rocket[ROCKET_HW_MEGA_STAGE].Num <= Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Num) {
            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE] = 0;
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
        } else {
            GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);

            GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
        }

        break;

    default:
        break;
    }
}

void PickModule(char plr)
{
    Data->P[plr].AIStrategy[AI_LUNAR_MODULE] = 6;

    if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE)) {
        RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE);
    }

    RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE);
}

void MoonVoting(char plr)
{
    int val = brandom(100) + 1;

    int high;
    if (val < 70) {
        high = 0;
    } else if (val < 78) {
        high = 1;
    } else if (val < 92) {
        high = 2;
    } else {
        high = 3;
    }

    switch (high) {
    case 0:
        Data->P[plr].AILunar = 1;
        PickModule(plr);
        MoonProgram(plr, 1);
        break;

    case 1:
        Data->P[plr].AILunar = 2;
        PickModule(plr);
        MoonProgram(plr, 2);
        break;

    case 2:
        Data->P[plr].AILunar = 3;
        PickModule(plr);
        MoonProgram(plr, 3);
        break;

    case 3:
        Data->P[plr].AILunar = 4;
        MoonProgram(plr, 4);
        break;

    default:
        break;
    }
}

int CheckSafety(char plr, char m)
{
    switch (m) {
    case 5:
        return Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety;

    case 6:
        return Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety;

    case 8:
        return Data->P[plr].Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety;

    case 9:
        return Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Safety;

    case 10:
        return Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety;

    default:
        break;
    }

    return 0;
}

void MoonProgram(char plr, char m)
{
    switch (m) {
    case 1:
        if (GenPur(plr, MISC_HARDWARE, MISC_HW_KICKER_B)) {
            RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        }

        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        Data->P[plr].Buy[MISC_HARDWARE][MISC_HW_KICKER_B] = 0;
        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        break;

    case 2:
        if (GenPur(plr, MISC_HARDWARE, MISC_HW_KICKER_B)) {
            RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        }

        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        Data->P[plr].Buy[MISC_HARDWARE][MISC_HW_KICKER_B] = 0;
        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        break;

    case 3:
        if (GenPur(plr, MISC_HARDWARE, MISC_HW_KICKER_A)) {
            RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_A);
        }

        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_A);
        Data->P[plr].Buy[MISC_HARDWARE][MISC_HW_KICKER_A] = 0;
        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_A);
        break;

    case 4:
        if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE)) {
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
        }

        RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);

        if (GenPur(plr, MISC_HARDWARE, MISC_HW_KICKER_B)) {
            RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        }

        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        Data->P[plr].Buy[MISC_HARDWARE][MISC_HW_KICKER_B] = 0;
        RDafford(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
        break;

    default:
        break;
    }
}

void ProgramVoting(char plr)
{
    if (brandom(100) < 65) {
        Data->P[plr].AIPrim = 8;
    } else {
        Data->P[plr].AIPrim = 9;
    }
}

/* EOF */
