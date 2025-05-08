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
    int val, i, P_total = 0, O_total = 0;
    char prg[2];

    if (plr == 0) {
        Level_Check = (Data->Def.Lev1 == 0) ? 0 : 1;
    } else  if (plr == 1) {
        Level_Check = (Data->Def.Lev2 == 0) ? 0 : 1;
    }

    // Randomly select the AI strategy
    P_total = brandom(100);

    if (Data->P[plr].AIStrategy[AI_STRATEGY] == 0) {
        if (P_total < 33) {
            Data->P[plr].AIStrategy[AI_STRATEGY] = 1;
        } else if (P_total < 66) {
            Data->P[plr].AIStrategy[AI_STRATEGY] = 2;
        } else {
            Data->P[plr].AIStrategy[AI_STRATEGY] = 3;
        }
    }

    Cur_Status = Equal;

// *** check status ***
    for (i = 0; i < Data->P[plr].PastMissionCount; i++) {
        P_total += Data->P[plr].History[i].Prestige;
    }

    for (i = 0; i < Data->P[other(plr)].PastMissionCount; i++) {
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
        Data->P[plr].AIStrategy[AI_BEGINNING_STRATEGY] = 0;
    }

    if (Data->P[plr].AstroDelay <= 0) {
        AIAstroPur(plr);
    }

    if (Data->P[plr].AIStat == 1)
        if (Data->Prestige[Prestige_OrbitalSatellite].Place == plr || Data->Prestige[Prestige_OrbitalSatellite].mPlace == plr) {
            Data->P[plr].AIStat = 2;
        }

    if (Data->P[plr].AIStat == 1 && PrestigeCheck(plr, Prestige_LunarFlyby) == 0 && Cur_Status == Behind) {
        Data->P[plr].AIStat = 2;
    }

    if (Data->P[plr].AIStat == 2 && Data->Year >= 61 && Data->Season == 0) {
        Data->P[plr].AIStat = 3;
    }

    if (Data->P[plr].AIStat <= 2) {
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

        Data->P[plr].Buy[PROBE_HARDWARE][PROBE_HW_ORBITAL] = 0;
        Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_ONE_STAGE] = 0;
        Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_ONE_MAN_CAPSULE] = 0;

        if (Data->Year <= 59) {
            Data->P[plr].Probe[PROBE_HW_ORBITAL].Num++;
            Data->P[plr].Rocket[ROCKET_HW_ONE_STAGE].Num++;
            AIFuture(plr, Mission_Orbital_Satellite, 0, 0);
        }

        KeepRD(plr, 5);
        Data->P[plr].Buy[PROBE_HARDWARE][PROBE_HW_ORBITAL] = 0;
        Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_ONE_STAGE] = 0;
        Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_ONE_MAN_CAPSULE] = 0;
        RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);

        if (Data->P[plr].AIStat < 2) {
            AIPur(plr);
        }

        prg[0] = 1;

        if (NoFail(plr) == 0) {
            if ((Data->Year == 59 && Data->Season == 1) || Data->Year >= 60) {
                switch (Data->P[plr].AIStrategy[AI_BEGINNING_STRATEGY]) {
                case 0:
                    if (Data->P[plr].Misc[MISC_HW_EVA_SUITS].Safety > Data->P[plr].Misc[MISC_HW_EVA_SUITS].MaxRD - 20) {
                        AIFuture(plr, Mission_Earth_Orbital_EVA, 0, (char *)&prg);
                    } else {
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    }

                    Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety += 10;
                    Data->P[plr].Misc[MISC_HW_EVA_SUITS].Safety += 10;
                    ++Data->P[plr].AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                case 1:
                    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 0) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 0) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 1) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 1) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    }

                    ++Data->P[plr].AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                case 2:
                    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 0) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_SubOrbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 0) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedSpaceMission) == 0 && PrestigeCheck(other(plr), Prestige_MannedSpaceMission) == 1) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital, 0, (char *)&prg);
                    } else if (PrestigeCheck(plr, Prestige_MannedOrbital) == 0 && PrestigeCheck(other(plr), Prestige_MannedOrbital) == 1) { // && Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety>Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].MaxRD-25)
                        AIFuture(plr, Mission_Earth_Orbital_EVA, 0, (char *)&prg);    //2
                    } else {
                        Data->P[plr].AIStat = 3;
                    }

                    ++Data->P[plr].AIStrategy[AI_BEGINNING_STRATEGY];
                    break;

                default:
                    break;
                }

                if (Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Num >= 2 && Data->P[plr].Rocket[ROCKET_HW_ONE_STAGE].Num >= 2) {
                    if (Data->P[plr].Future[0].MissionCode == Mission_SubOrbital) {
                        AIFuture(plr, Mission_Earth_Orbital, 1, (char *)&prg);
                    } else if (Data->P[plr].Future[0].MissionCode == Mission_Earth_Orbital) {
                        AIFuture(plr, Mission_SubOrbital, 1, (char *)&prg);
                    } else if (Data->P[plr].Future[0].MissionCode == Mission_Earth_Orbital_EVA) {
                        AIFuture(plr, Mission_Earth_Orbital, 1, (char *)&prg);
                    }
                };
            };
        };

        AILaunch(plr);
    }

    if (GenPur(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS)) {
        RDafford(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);
    } else {
        RDafford(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);
    }

    Data->P[plr].Buy[MISC_HARDWARE][MISC_HW_EVA_SUITS] = 0;
    RDafford(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);

    if (Data->P[plr].AIStat >= 2) {
        CheckVoting(plr); // gets AIPrim,AISec,AILunar

        if (Data->P[plr].AIPrim == 0) {
            ProgramVoting(plr);
        }
    }

// primary/secondary programs
    if (Data->P[plr].AIPrim == 8) {
        Data->P[plr].AIPrim = 6;
        Data->P[plr].AISec = 8;
    }

    if (Data->P[plr].AIPrim == 6) {
        Data->P[plr].AISec = 8;
    }

    if (Data->P[plr].AIPrim == 9) {
        Data->P[plr].AIPrim = 6;
        Data->P[plr].AISec = 9;
    }

    if (Data->P[plr].AILunar == 4) {
        Data->P[plr].AIPrim = 6;
        Data->P[plr].AISec = (Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety >= Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety) ? 6 : 10;
    }

// boosters
    if (Data->P[plr].AIStat >= 2) {
        if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS)) {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
        } else {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
        }
    }

    if (CheckSafety(plr, Data->P[plr].AIPrim) >= CheckSafety(plr, Data->P[plr].AISec)) {
        KeepRD(plr, Data->P[plr].AIPrim);
    } else {
        KeepRD(plr, Data->P[plr].AISec);
    }

// larger rocket kludge
    if (Data->P[plr].AIStrategy[AI_LARGER_ROCKET_STRATEGY] == 1) {
        if (Level_Check != 0) {
            Data->P[plr].Cash += 25;    // temporary
        }

        if (Data->P[plr].AILunar < 4) {
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            }

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
        } else {
            if (Level_Check != 0) {
                Data->P[plr].Cash += 25;
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
        }
    }

    if (Data->Year >= 62)
        if (Data->P[plr].AIStrategy[AI_LUNAR_MODULE] < 5) {
            PickModule(plr);
        }

    if (Data->P[plr].AILunar > 0 && Data->P[plr].AILunar < 4) {
        if (Data->P[plr].AIStrategy[AI_LUNAR_MODULE] > 0) {
            if (GenPur(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE])) {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            } else {
                RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
            }

            Data->P[plr].Buy[MANNED_HARDWARE][Data->P[plr].AIStrategy[AI_LUNAR_MODULE]] = 0;
            RDafford(plr, MANNED_HARDWARE, Data->P[plr].AIStrategy[AI_LUNAR_MODULE]);
        }
    }

    for (i = 0; i < MAX_LAUNCHPADS; i++) {
        if (Data->P[plr].LaunchFacility[i] >= LAUNCHPAD_DAMAGED_MARGIN) {
            if (Data->P[plr].LaunchFacility[i] <= Data->P[plr].Cash) {
                Data->P[plr].Cash -= Data->P[plr].LaunchFacility[i];
                Data->P[plr].LaunchFacility[i] = LAUNCHPAD_OPERATIONAL;
            }
        }
    }

    Data->P[plr].LaunchFacility[0] = LAUNCHPAD_OPERATIONAL;
    Data->P[plr].LaunchFacility[1] = LAUNCHPAD_OPERATIONAL;
    Data->P[plr].LaunchFacility[2] = LAUNCHPAD_OPERATIONAL;

    if (Data->P[plr].AIStat == 3) {
        switch (Data->P[plr].AILunar) {
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

    if (Data->P[plr].AIStat >= 2 && Data->Year >= 61 && Data->P[plr].AIStrategy[AI_END_STAGE_LOCATION] >= 2) {
        if ((Data->P[plr].Probe[PROBE_HW_ORBITAL].Safety >= Data->P[plr].Probe[PROBE_HW_ORBITAL].MaxRD - 20) || Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num >= 0) {
            Data->P[plr].Cash += Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].InitCost + 30;

            if (Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Num <= 0) {
                if (GenPur(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY)) {
                    RDafford(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
                } else {
                    RDafford(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
                }
            }

            Data->P[plr].Buy[PROBE_HARDWARE][PROBE_HW_INTERPLANETARY] = 0;
            RDafford(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
        }

        if ((Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].Safety >= Data->P[plr].Probe[PROBE_HW_INTERPLANETARY].MaxRD - 20) || Data->P[plr].Probe[PROBE_HW_LUNAR].Num >= 0) {
            Data->P[plr].Cash += Data->P[plr].Probe[PROBE_HW_LUNAR].InitCost + 30;

            if (Data->P[plr].Probe[PROBE_HW_LUNAR].Num <= 0) {
                if (GenPur(plr, PROBE_HARDWARE, PROBE_HW_LUNAR)) {
                    RDafford(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
                } else {
                    RDafford(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
                }
            }

            Data->P[plr].Buy[PROBE_HARDWARE][PROBE_HW_LUNAR] = 0;
            RDafford(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
        }

        if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE)) {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
        } else {
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
        }
    }

    if (PrestigeCheck(plr, Prestige_MannedSpaceMission) || PrestigeCheck(plr, Prestige_MannedOrbital)) {
        Data->P[plr].AIStat = 3;
    }

// **** end stages ***
    if (Data->P[plr].AIStat == 3) {
        if (Data->P[plr].AILunar < 4) {
            if (CheckSafety(plr, Data->P[plr].AIPrim) > CheckSafety(plr, Data->P[plr].AISec)) {
                val = Data->P[plr].AIPrim;
            } else {
                val = Data->P[plr].AISec;
            }

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (Data->P[plr].Manned[val - 1].Safety >= Data->P[plr].Manned[val - 1].MaxRD - 15) {
                NewAI(plr, val);
            }
        } else if (Data->P[plr].AILunar == 4) {
            if (Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety >= Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].MaxRD - 10) {
                Data->P[plr].AISec = 10;
            } else {
                Data->P[plr].AISec = 6;
            }

            if (CheckSafety(plr, Data->P[plr].AIPrim) > CheckSafety(plr, Data->P[plr].AISec)) {
                val = Data->P[plr].AIPrim;
            } else {
                val = Data->P[plr].AISec;
            }

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (Data->P[plr].Manned[val - 1].Safety >= Data->P[plr].Manned[val - 1].MaxRD - 15) {
                NewAI(plr, val);
            } else {
                Data->P[plr].Probe[PROBE_HW_ORBITAL].Num += 2;
                Data->P[plr].Rocket[ROCKET_HW_ONE_STAGE].Num += 2;
                AIFuture(plr, Mission_Orbital_Satellite, 0, 0);
                AIFuture(plr, Mission_Orbital_Satellite, 1, 0);
                AIFuture(plr, Mission_Orbital_Satellite, 2, 0);
            }
        }

        if (CheckSafety(plr, Data->P[plr].AIPrim) > CheckSafety(plr, Data->P[plr].AISec)) {
            val = Data->P[plr].AIPrim;

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (CheckSafety(plr, Data->P[plr].AIPrim) > Data->P[plr].Manned[val - 1].MaxRD - 10)
                if (Data->P[plr].Manned[val - 1].Num >= 1) {
                    KeepRD(plr, Data->P[plr].AISec);
                }
        } else {
            val = Data->P[plr].AISec;

            if (val < 7) {
                val = val - 4;
            } else {
                val = val - 5;
            }

            if (CheckSafety(plr, Data->P[plr].AISec) > Data->P[plr].Manned[val - 1].MaxRD - 10)
                if (Data->P[plr].Manned[val - 1].Num >= 1) {
                    KeepRD(plr, Data->P[plr].AIPrim);
                }
        }
    }

    RDPrograms(plr);
    return;
}


char NoFail(char plr)
{
    char RT_value = 0, i;

    for (i = 0; i < Data->P[plr].PastMissionCount; i++) {
        if ((Data->P[plr].History[i].MissionCode == Mission_Earth_Orbital || Data->P[plr].History[i].MissionCode == Mission_Earth_Orbital_EVA) && Data->P[plr].History[i].spResult >= 3000) {
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
    int i;

    for (i = 0; i < 7; i++) {
        if (Data->P[plr].Manned[i].Num >= 0) {
            RDafford(plr, MANNED_HARDWARE, i);
        }

        Data->P[plr].Manned[i].Damage = 0;
        Data->P[plr].Manned[i].DCost = 0;
    };

    for (i = 0; i < 5; i++) {
        if (Data->P[plr].Rocket[i].Num >= 0) {
            RDafford(plr, ROCKET_HARDWARE, i);
        }

        Data->P[plr].Rocket[i].Damage = 0;
        Data->P[plr].Rocket[i].DCost = 0;
    };

    for (i = 0; i < 4; i++) {
        if (Data->P[plr].Misc[i].Num >= 0) {
            RDafford(plr, MISC_HARDWARE, i);
        }

        Data->P[plr].Misc[i].Damage = 0;
        Data->P[plr].Misc[i].DCost = 0;
    };

    for (i = 0; i < 3; i++) {
        if (Data->P[plr].Probe[i].Num >= 0) {
            RDafford(plr, PROBE_HARDWARE, i);
        }

        Data->P[plr].Probe[i].Damage = 0;
        Data->P[plr].Probe[i].DCost = 0;
    };

    return;
}

void CheckVoting(char plr)
{
    int st;

    if (Data->P[plr].AILunar == 0) {
        MoonVoting(plr);
    } else if (Data->P[plr].AILunar == 4) {
        st = GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);

        if (st == 1) {
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            Data->P[plr].AILunar = 4; // lunar landing set
        }
    }

    return;
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
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            }

            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            }

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_TWO_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_CAPSULE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            }
        } else {
            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
            }

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
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            }

            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            }

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_THREE_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_THREE_MAN_CAPSULE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            }
        } else {
            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
            }

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
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            }

            if (Level_Check != 0) {
                Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
                Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_MINISHUTTLE] = 0;
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }
        } else {
            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

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
            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            }

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE] = 0;
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
        } else {
            if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE)) {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            } else {
                RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
            }

            if (GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE)) {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            } else {
                RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            }

            Data->P[plr].Buy[ROCKET_HARDWARE][ROCKET_HW_MEGA_STAGE] = 0;
            Data->P[plr].Buy[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE] = 0;
            RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
            RDafford(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
        }

        break;

    default:
        break;
    }

    return;
}

void PickModule(char plr)
{
    Data->P[plr].AIStrategy[AI_LUNAR_MODULE] = 6;

    if (GenPur(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE)) {
        RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE);
    }

    RDafford(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE);
    return;
}

void MoonVoting(char plr)
{
    int high = -1, val;
    val = brandom(100) + 1;

    if (val < 70) {
        high = 0;
    } else if (val < 78) {
        high = 1;
    } else if (val < 92) {
        high = 2;
    } else {
        high = 3;
    }

    if (high != -1) {
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

    return;
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

    return;
}

void ProgramVoting(char plr)
{
    int i = 0;
    i = brandom(100);

    if (i < 65) {
        Data->P[plr].AIPrim = 8;
    } else {
        Data->P[plr].AIPrim = 9;
    }

    return;
}

/* EOF */



