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

// This file handles part of Mission Control during missions

#include "mc2.h"

#include <cassert>

#include "Buzz_inc.h"
#include "options.h"
#include "game_main.h"
#include "mc.h"
#include "mission_util.h"
#include "prest.h"
#include "pace.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

int CrewEndurance(const struct MisAst *crew, size_t crewSize);
void MissionParse(char plr, struct mStr &misType, char pad);
char WhichPart(char plr, int which);
void MissionSteps(char plr, int mcode, int step, int pad,
                  const struct mStr &mission);


//********************************************************************
// Setup for the Mission
// Routines used by MControl and Future Missions

/**
 * Interprets the mission plan "code" as a series of mission steps.
 *
 * Fills the Mev[] global variable with mission step data.
 * Increments the STEP global via MissionParse to account for how
 * many mission steps are possible.
 *
 * \param plr  the player index.
 * \param val  the mission code (MissionType.MissionCode or mStr.Index).
 * \param pad  the pad index of the mission's first launch.
 */
void MissionCodes(char plr, char val, char pad)
{
    struct mStr plan = GetMissionPlan(val);
    MissionParse(plr, plan, pad);
    return;
}


int CrewEndurance(const struct MisAst *crew, size_t crewSize)
{
    assert(crew);

    int value = 0;

    for (size_t i = 0; i < crewSize; i++) {
        assert(crew[i].A);
        value += crew[i].A->Endurance;
    }

    return int((double(value) / double(crewSize)) + 0.5);
}


void
MissionParse(char plr, struct mStr &misType, char pad)
{
    int i, loc, j;

    STEP = 0;
    loc = pad;
    char *MCode = misType.Code;

    for (i = 0; MCode[i] != '|'; ++i) {
        switch (MCode[i]) {
        case '@':
            i++;
            MCode[i] = 'b';    // duration step
            MissionSteps(plr, MCode[i], STEP++, loc - pad, misType);
            break;

        case '~':

            // printf("      :Delay of %d seasons\n", MCode[i + 1] - 0x30);
            for (j = 0; j < (MCode[i + 1] - 0x30); j++) {
                MissionSteps(plr, MCode[i + 2], STEP++, loc - pad,
                             misType);
            }

            i += 2;
            break;

        case '+':
            i++;
            loc = MCode[i] - 0x30 + pad - 1;
            ASSERT(loc - pad < 3);  // Breaks rescue missions
            break;

        case '^':
            loc = pad + 1;
            break;

        case '&':
            loc = pad;
            break;

        case '%':
            i++;
            MCode[i] = 'c';
            MissionSteps(plr, MCode[i], STEP++, loc - pad, misType);
            break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case '!':
            if (MCode[i] == 'I') {
                loc = pad;
            }

            MissionSteps(plr, MCode[i], STEP++, loc - pad, misType);
            break;
        }
    }
}


char WhichPart(char plr, int which)
{
    int val = 0;

    if (Data->Prestige[abs(which)].Place == -1) {
        val = 1;
    } else if (Data->Prestige[abs(which)].mPlace == -1
               && Data->Prestige[abs(which)].Place != plr) {
        val = 2;
    } else {
        val = 3;
    }

    return val;
}

Equipment *GetEquipment(const struct MisEval &Mev)
{
    Equipment *e = MH[Mev.pad][Mev.Class];

    // Some equipment like docking module can be attached to the other pad
    if (!e) {
        e = MH[other(Mev.pad)][Mev.Class];
    }

    assert(e == Mev.Ep);
    return e;
}

void MissionSteps(char plr, int mcode, int step, int pad,
                  const struct mStr &mission)
{
    switch (mcode) {
    // Booster Programs    :: VAB order for the class
    case 'A':
        Mev[step].Class = Mission_PrimaryBooster;
        break;

    // Manned Programs : Capsule
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'K':
    case 'M':
    case 'N':
    case 'O':
    case 'X':
    case 'Y':
    case 'f':
    case 'g':
        if (MH[pad][Mission_Kicker] &&
            MH[pad][Mission_Kicker]->ID[1] == 0x32) {
            Mev[step].Class = Mission_Kicker;  // Kicker-C
        } else {
            Mev[step].Class = Mission_Capsule;
        }

        break;

    case 'b':
    case 'c':   // Duration Step
        Mev[step].Class = 10;
        break;

    // Manned Programs : LM
    case 'a':
        Mev[step].Class = Mission_LM;  // LM
        break;

    // Unmanned Programs
    case 'C':
        if (MH[pad][Mission_Kicker] &&
            MH[pad][Mission_Kicker]->ID[1] == 0x32) {
            Mev[step].Class = Mission_Kicker;  // Kicker-C
        } else if (MH[pad][Mission_LM] && MH[pad][Mission_LM] != NULL) {
            Mev[step].Class = Mission_LM;
        } else {
            Mev[step].Class = Mission_Probe_DM;
        }

        break;

    case 'V':
    case 'W':
    case 'Z':
    case 'e':
        Mev[step].Class = Mission_Probe_DM;  // Satellite
        break;

    // Misc Programs
    case 'H':
    case 'P':
        Mev[step].Class = Mission_EVA;  // EVA Suits
        break;

    // Photo Recon
    case 'U':
        Mev[step].Class = Mission_PhotoRecon;  // Photo Recon
        break;

    // Special Cases (when is there not one?)
    case 'B':
    case 'J':
    case 'L':
        if (MH[pad][Mission_Kicker] != NULL) {
            Mev[step].Class = Mission_Kicker;  // Kicker
        } else if (MH[pad][Mission_Capsule] != NULL) {
            Mev[step].Class = Mission_Capsule;  // Cap
        } else if (MH[pad][Mission_LM] != NULL) {
            Mev[step].Class = Mission_LM;    // LM
        } else {
            Mev[step].Class = Mission_Probe_DM;  // Satellite
        }

        if (step > 11 && MH[pad][Mission_Capsule] != NULL) {
            Mev[step].Class = Mission_Capsule;  // Cap L->E
        }

        break;

    case 'Q':
    case 'R':
    case 'S':
    case 'T':
        if (MH[pad][Mission_Kicker] &&
            MH[pad][Mission_Kicker]->ID[1] == 0x32) {
            Mev[step].Class = Mission_Kicker;  // Kicker-C
        } else if (MH[pad][Mission_LM] != NULL) {
            Mev[step].Class = Mission_LM;  // LM
        } else if (MH[pad][Mission_Capsule] != NULL) {
            Mev[step].Class = Mission_Capsule;  // Capsule
        } else {
            Mev[step].Class = Mission_Probe_DM;  // Satellite
        }

        break;

    case 'I':
        Mev[step].Class = Mission_Probe_DM;  // DM
        break;

    case 'd':  // Special
    default:
        break;
    }

    Mev[step].Prest = -100;

    switch (mcode) {
    case 'B':
        if (MH[pad][Mission_Capsule] != NULL) {
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -18);    // CAP
        } else {
            Mev[step].Prest = -100;
        }

        Mev[step].PComp = 0;
        break;

    case 'C':
        if (MH[pad][Mission_Probe_DM] &&
            MH[pad][Mission_Probe_DM]->ID[1] == 0x30) {
            Mev[step].Prest = 0;
        } else {
            Mev[step].Prest = -100;
        }

        break;

    case 'D':
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -27);
        break;

    case 'H':
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = 26);
        break;

    case 'I':
        if ((MANNED[0] + MANNED[1]) > 0) {
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest = 24);
        }

        break;

    case 'L':
        // Award Manned Lunar Orbital only if done by a (manned)
        // capsule and not when parking an unmanned LM during an
        // LOR mission
        if (MH[pad][Mission_Capsule] != NULL) {
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -20);    // CAP
        } else {
            Mev[step].Prest = -100;
        }

        break;

    case 'O':
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -19);
        break;

    case 'P':
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = 26);
        break;

    case 'S':
        if (MH[pad][Mission_Probe_DM] &&
            MH[pad][Mission_Probe_DM]->ID[1] == 0x32) {
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -7);
        }

        break;

    case 'T':  // Done on lunar launch for good reason
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -22);
        break;

    case 'W':
        switch (Data->P[plr].Mission[pad].MissionCode) {
        case Mission_LunarFlyby:
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -1);
            break;

        case Mission_MercuryFlyby:
            Mev[step].Prest = Prestige_MercuryFlyby;
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
            break;

        case Mission_VenusFlyby:
            Mev[step].Prest = Prestige_VenusFlyby;
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
            break;

        case Mission_MarsFlyby:
            Mev[step].Prest = Prestige_MarsFlyby;
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
            break;

        case Mission_JupiterFlyby:
            Mev[step].Prest = Prestige_JupiterFlyby;
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
            break;

        case Mission_SaturnFlyby:
            Mev[step].Prest = Prestige_SaturnFlyby;
            Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
            break;
        }

        break;

    default:
        Mev[step].Prest = -100;
        Mev[step].PComp = 0;
        break;
    }

    if (Mev[step].Prest != -100) {
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest);
    }

    // Invalidate any steps on an alternate branch except for Lunar Orbital
    if (PastBANG == 1 && Mev[step].PComp > 0 && !(mcode == 'L')) {
        Mev[step].PComp = 5;
    }

    if (mcode == 'd') {
        // Alternative path Mission.Alt
        if (mission.Alt[step] == 0) {
            Mev[step - 1].sgoto = Mev[step - 1].fgoto = 100;
        } else {
            Mev[step - 1].sgoto = Mev[step - 1].fgoto = mission.Alt[step];
        }
    } else {
        Mev[step].asf = 0;

        if (MANNED[pad] > 0) {
            switch (Mev[step].Class) {
            case Mission_Capsule:
                Mev[step].ast = CAP[pad];  // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->Cap;
                }

                break;

            case Mission_LM:
                Mev[step].ast = LM[pad];  // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->LM;
                }

                break;

            case Mission_Probe_DM:  // docking
                Mev[step].ast = DOC[pad];  // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->Docking;
                }

                break;

            case Mission_EVA:  // eva
                Mev[step].ast = EVA[pad];  // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->EVA;
                }

                break;

            case 7:  // covers power-on for docking module
                Mev[step].ast = -1;
                Mev[step].asf = 0;
                break;

            case 10:  // durations
                Mev[step].Class = Mission_Capsule;
                Mev[step].ast = -1;
                Mev[step].asf = options.feat_use_endurance
                                ? CrewEndurance(MA[pad], MANNED[pad]) : 0;
                break;

            default:  // remaining
                Mev[step].ast = -1;
                Mev[step].asf = 0;
                break;
            };
        }


//      if (step==0 && Data->P[plr].TurnOnly==5)
//         *Mev[step].sf=MIN(*Mev[step].sf,50);

        Mev[step].step = step;

        if ((Data->Def.Lev1 == 0 && plr == 0) || (Data->Def.Lev2 == 0 && plr == 1)) {
            Mev[step].dice = MisRandom();
        } else {
            Mev[step].dice = brandom((AI[plr]) ? 98 : 100) + 1;
        }

        Mev[step].rnum = brandom(10000) + 1;
        Mev[step].sgoto = 0;

        // prevents mission looping
        Mev[step].fgoto =
            (mission.Alt[step] == -2) ? step + 1 : mission.Alt[step];
        Mev[step].dgoto = mission.AltD[step];  // death branching (tm)
        Mev[step].Ep = MH[pad][Mev[step].Class];  // FIXME: << this sets E

        Mev[step].pad = pad;

        memset(&Mev[step].Name[0], 0x00, sizeof(Mev[step].Name));

        if (mcode == 'Q' && Mev[step - 1].loc == 18) {
            Mev[step].Name[0] = 'h';
        } else {
            if (mcode >= 'a' && mcode < 'z') {
                Mev[step].Name[0] = mcode;
            } else {
                Mev[step].Name[strlen(Mev[step].Name)] = mcode;
            }
        }

        if (plr == 0) {
            strcat(Mev[step].Name, "U\0");
        } else {
            strcat(Mev[step].Name, "S\0");
        }

        if (MH[pad][Mev[step].Class]) {
            strncat(Mev[step].Name, MH[pad][Mev[step].Class]->ID, 2);
        }

        /////////////////////////////////////////////////
        // Fix for BARIS CD-ROM Planetary Steps (Step W)
        // E= Moon; M= Mars; S= Saturn; V= Venus; J= Jupiter; R= Mercury
        // Must be at .Name[2]
        //

        if (Mev[step].Name[0] == 'W') {
            Mev[step].Name[2] = '*';    // Placeholder
        }

        if (Mev[step].Class == Mission_EVA) {
            if (MH[0][Mission_Kicker] &&
                MH[0][Mission_Kicker]->ID[1] == 0x32) {
                strcat(Mev[step].Name, "M2");    // Kicker-C
            } else if (MH[pad][Mission_Capsule] &&
                       MH[pad][Mission_Capsule]->ID[1] == 0x34) {
                strcat(Mev[step].Name, "C4");    // FourMan
            } else {  // standard LMs
                if (mcode == 'P') {
                    if (MH[pad][Mission_LM] != NULL) {
                        strncat(Mev[step].Name, MH[pad][Mission_LM]->ID, 2);
                    } else if (MH[1][Mission_LM]) {
                        strncat(Mev[step].Name, MH[1][Mission_LM]->ID, 2);
                    }
                } else {
                    if (MH[pad][Mission_Capsule] != NULL) {
                        strncat(Mev[step].Name, MH[pad][Mission_Capsule]->ID, 2);
                    } else if (MH[1][Mission_Capsule]) {
                        strncat(Mev[step].Name, MH[1][Mission_Capsule]->ID, 2);
                    }
                }
            }
        }

        // Select animation w/ boosters if appropriate
        if (Mev[step].Name[0] == 'A' &&
            MH[pad][Mission_SecondaryBooster] != NULL) {
            Mev[step].Name[3] += 4;
        }

        // Special Cases #54753 and #54754

        if (mcode == 'H' && Data->P[plr].Mission[pad].MissionCode == Mission_LunarOrbital_LM_Test) {
            strcpy(Mev[step].Name, "HMOON\0");
        }

        if (mcode == 'H' && PastBANG == 1) {
            strcpy(Mev[step].Name, "HMOON\0");
        }

        if (mcode == '!' || mcode == '|') {
            Mev[step].loc = 0x7f;

            if (step > 0) {
                Mev[step - 1].sgoto = 100;
            }

            PastBANG = 1;
        } else {
            Mev[step].loc = mcode - 65;

            if (Mev[step].loc > 25) {
                Mev[step].loc -= 6;
            }

            if (Mev[step].loc == 32 && mission.Lun) {
                Mev[step].loc = 29;
            }
        }
    }

    // name the mission step for failures.

    strcpy(Mev[step].FName, "F000");

    Mev[step].FName[3] = (char) 0x30 + (Mev[step].loc % 10);

    if (Mev[step].loc >= 10) {
        Mev[step].FName[2] = '1';
    }

    if (Mev[step].loc >= 20) {
        Mev[step].FName[2] = '2';
    }

    if (Mev[step].loc >= 30) {
        Mev[step].FName[2] = '3';
    }

    if (Mev[step].loc == 32 || Mev[step].loc == 29) {  // Fix _g special case #48010
        Mev[step].FName[2] = '0';
        Mev[step].FName[3] = '1';
    }

    // Special Cases for the Failure Mode Charts
    if ((Mev[step].loc == 0) &&  // MS Failure Launch
        MH[pad][Mission_Capsule] &&
        strncmp(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name,
                MH[pad][Mission_Capsule]->Name, 5) == 0) {
        Mev[step].FName[1] = '1';
    } else if (Mev[step].loc == 4 &&  // MS Failure Landing
               MH[pad][Mev[step].Class] &&
               strncmp(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name, MH[pad][Mev[step].Class]->Name, 5) == 0) {
        Mev[step].FName[1] = '3';
    } else if (plr == 1 && Mev[step].loc == 4) {
        if ((Mev[step].loc == 4) &&  // Soviet Capsules: Vostok
            MH[pad][Mev[step].Class] &&
            strncmp(Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Name, MH[pad][Mev[step].Class]->Name, 5) == 0) {
            Mev[step].FName[1] = '1';
        } else {
            Mev[step].FName[1] = '2';    // Other Capsules
        }
    }


    // New expanded cases for failure mode charts
    if (Mev[step].Name[2] == 'C' && Mev[step].loc == 2) {
        Mev[step].FName[1] = '1';
    }

    if (strncmp(&Mev[step].Name[2], "M3C0", 4) == 0) {
        Mev[step].FName[1] = '1';
    }

    if (Mev[step].Name[2] != 'P') {  // exclude any probes
        if (Mev[step].loc == 15 && Mev[step].Name[5] == 0x36) {
            // Lunar EVA w/ one-man capsule (F115)
            Mev[step].FName[1] = '1';
        }

        // (loc == 16) => LEM Activities, step > 8 => Lunar orbit/surface
        if (Mev[step].loc == 16 && step > 8) {
            // (loc == 18) => Lunar Landing
            if (Mev[step - 1].loc == 18 && Mev[step].Name[3] >= 0x35) {
                Mev[step].FName[1] = '2';
            } else if (Mev[step - 1].loc == 18 && Mev[step].Name[3] <= 0x34) {
                Mev[step].FName[1] = '3';
            } else {
                Mev[step].FName[1] = '1';
            }
        }

        if ((Mev[step].loc >= 17 && Mev[step].loc <= 19) &&
            (Mev[step].Name[3] <= 0x34)) {
            Mev[step].FName[1] = '1';
        }

        // Use F018 error group for Lunar Landing step unless:
        // Kicker-C (Name is "SSM2") or Four-man capsule ("S?C4")
        if (Mev[step].loc == 18 && (Mev[step].Name[3] != '4' && Mev[step].Name[2] != 'M')) {
            Mev[step].FName[1] = '0';
        }

        if (Mev[step].loc == 20 && (Mev[step].Name[4] == 0x34 || Mev[step].Name[4] == 0x33)) {
            Mev[step].FName[1] = '1';
        }
    }

    Mev[step].StepInfo = 0;

    return;
}


/**
 * \param plr  the player index
 * \param mis  the pad index for player's Mission array.
 */
void MissionSetup(char plr, char mis)
{
    char i, j, t;
    DMFake = 0;
    const struct mStr plan =
        GetMissionPlan(Data->P[plr].Mission[mis].MissionCode);

    for (j = 0; j < (1 + Data->P[plr].Mission[mis].Joint); j++) {

        if ((plan.mVab[j] & 0x10) > 0 &&
            Data->P[plr].DockingModuleInOrbit > 0) {  // DMO
            Data->P[plr].Mission[mis + j].Hard[Mission_Probe_DM] = 4;
            DMFake = 1;
        }

        if (j == 0) {
            // Apollo (1) + LM (1)
            if (Data->P[plr].Mission[mis].Hard[Mission_Capsule] == 2 && Data->P[plr].Mission[mis].Hard[Mission_LM] >= 0) {
                Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
                DMFake = 1;
            }
            
            if (Data->P[plr].Mission[mis].Joint) {
                // Apollo (2) + LM (1)
                if (Data->P[plr].Mission[mis + 1].Hard[Mission_Capsule] == 2 && Data->P[plr].Mission[mis].Hard[Mission_LM] >= 0) {
                    Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
                    DMFake = 1;
                }
            }
        }

        if (Data->P[plr].Mission[mis].MissionCode == Mission_Soyuz_LL) {  // Soyuz Kicker-C
            Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
            DMFake = 1;
        }

        for (i = Mission_Capsule; i < Mission_PhotoRecon; i++) {
            Equipment *eq = NULL;  // Clear Pointers

            t = Data->P[plr].Mission[mis + j].Hard[i];

            if (t >= 0) {
                switch (i) {
                case Mission_Capsule:
                case Mission_LM:   // Cap - LM
                    eq = &Data->P[plr].Manned[t];

                    if (eq->Num && t != MANNED_HW_MINISHUTTLE) {
                        eq->Num--;
                    }

                    eq->Used++;
                    break;

                case Mission_Kicker:           // Kicker
                    eq = &Data->P[plr].Misc[t];

                    if (eq->Num) {
                        eq->Num--;
                    }

                    eq->Used++;
                    break;

                case Mission_Probe_DM:           // Secondary Equipment
                    if (t != 4) {
                        eq = &Data->P[plr].Probe[t];
                        eq->Used++;

                        if (Data->P[plr].Probe[t].Num > 0) {
                            Data->P[plr].Probe[t].Num -= 1;
                        }
                    } else {
                        eq = &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE];

                        if (DMFake == 0) {
                            if (eq->Num > 0) {
                                eq->Num--;
                            }

                            eq->Used++;
                        }
                    }

                    break;

                case Mission_PrimaryBooster:            // Rocket XX
                    t--;

                    if (t < 4) {
                        eq = &Data->P[plr].Rocket[t];

                        if (eq->Num > 0) {
                            eq->Num--;
                        }

                        eq->Used++;
                    } else {
                        eq = &Data->P[plr].Rocket[t - 4];
                        MH[j][Mission_SecondaryBooster] =
                            &Data->P[plr].Rocket[ROCKET_HW_BOOSTERS];
                        eq->Used++;
                        MH[j][Mission_SecondaryBooster]->Used++;

                        if (eq->Num > 0) {
                            eq->Num--;
                        }

                        if (MH[j][Mission_SecondaryBooster]->Num > 0) {
                            MH[j][Mission_SecondaryBooster]->Num--;
                        }
                    }

                    break;

                case Mission_EVA:  // EVA
                    eq = &Data->P[plr].Misc[MISC_HW_EVA_SUITS];
                    break;
                }

                if (eq != NULL) {
                    eq->MisSaf = eq->Safety + eq->SMods + eq->Damage;

                    if (eq->ID[1] >= 0x35 && i == Mission_LM &&
                        Data->P[plr].Mission[mis].MissionCode >= 53) {
                        switch (Data->P[plr].LMpts) {
                        case 0:
                            eq->MisSaf -= 9;
                            break;

                        case 1:
                            eq->MisSaf -= 6;
                            break;

                        case 2:
                            eq->MisSaf -= 3;
                            break;
                        }
                    }
                }
            }  // if t>=0

            MH[j][i] = eq;
        }  // for (i<7)

        // Photo Recon isn't included in MissionType.Hard - it's
        // always available.
        MH[j][Mission_PhotoRecon] =
            &Data->P[plr].Misc[MISC_HW_PHOTO_RECON];
        // Photo Recon should never be damaged.
        MH[j][Mission_PhotoRecon]->MisSaf =
            MH[j][Mission_PhotoRecon]->Safety;
    }  // for (j<2)

    if (DMFake == 1) {
        Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = -1;
    }

    return;
}


// Update the Current Hardware for the Mission
void MissionSetDown(char plr, char mis)
{
    char i, j;

    for (j = 0; j < (Data->P[plr].Mission[mis].Joint + 1); j++) {
        for (i = 0; i < 7; i++) {  // Ignore Boosters
            if (MH[j][i] != NULL && (MH[j][i]->MisSucc[j] > 0 || MH[j][i]->MisFail[j] > 0)) {

                MH[j][i]->SMods = MH[j][i]->Damage = MH[j][i]->DCost = 0;

                if (strncmp(MH[j][i]->Name, (i == Mission_Probe_DM) ? "DOC" : "PHO", 3) != 0 && MH[j][i]->MisSucc[j] > 0) {
                    MH[j][i]->Safety = MIN(MH[j][i]->Safety + 1, MH[j][i]->MaxSafety);

                    if (options.cheat_addMaxS) {
                        MH[j][i]->MaxRD++;
                    }

                    if (MH[j][i]->MaxRD > MH[j][i]->MaxSafety - 2) {
                        MH[j][i]->MaxRD = MH[j][i]->MaxSafety - 2;
                    }
                }

                if (strncmp(MH[j][i]->Name, "DOC", 3) == 0 && (MH[j][i]->MisFail[j] + MH[j][i]->MisSucc[j]) == 0) {
                    MH[j][i]->MisFail[j] = 1;
                }

                if ((MH[j][i]->MisFail[j] + MH[j][i]->MisSucc[j]) == 0 && (strncmp(MH[j][i]->ID, "M3", 2) != 0)) {
                    MH[j][i]->MisFail[j]++;
                }

                if ((MH[j][i]->MisFail[j] + MH[j][i]->MisSucc[j]) == 0 && MH[j][i]->ID[0] == 'P') {
                    MH[j][i]->MisFail[j]++;
                }


                MH[j][i]->Failures += MH[j][i]->MisFail[j];

                MH[j][i]->Steps += (MH[j][i]->MisFail[j] + MH[j][i]->MisSucc[j]);

                if (i == Mission_PrimaryBooster &&
                    MH[j][Mission_SecondaryBooster] != NULL) {  // Boosters
                    if (MH[j][Mission_PrimaryBooster]->MisSucc[j] > 0)  {
                        MH[j][Mission_SecondaryBooster]->Safety =
                            MIN(MH[j][Mission_SecondaryBooster]->Safety + 1,
                                MH[j][Mission_SecondaryBooster]->MaxSafety);

                        if (options.cheat_addMaxS) {
                            MH[j][Mission_SecondaryBooster]->MaxRD++;
                        }

                        if (MH[j][Mission_SecondaryBooster]->MaxRD > MH[j][Mission_SecondaryBooster]->MaxSafety - 2) {
                            MH[j][Mission_SecondaryBooster]->MaxRD = MH[j][Mission_SecondaryBooster]->MaxSafety - 2;
                        }
                    }

                    MH[j][Mission_SecondaryBooster]->SMods = 0;
                    MH[j][Mission_SecondaryBooster]->Damage = 0;
                    MH[j][Mission_SecondaryBooster]->DCost = 0;

                    MH[j][Mission_SecondaryBooster]->Failures +=
                        MH[j][Mission_PrimaryBooster]->MisFail[j];
                    MH[j][Mission_SecondaryBooster]->Steps +=
                        (MH[j][Mission_PrimaryBooster]->MisFail[j] +
                         MH[j][Mission_PrimaryBooster]->MisSucc[j]);
                    MH[j][Mission_SecondaryBooster]->MisSucc[j] = 0;
                    MH[j][Mission_SecondaryBooster]->MisFail[j] = 0;
                }

                MH[j][i]->MisSucc[j] = MH[j][i]->MisFail[j] = 0;
            }  // if
        }  // for i

        VerifySF(j);
    }  // for j

    return;
}


/**
 * Compute and apply safety penalties to mission steps.
 *
 * This should only be called after MissionSetup() so the global array
 * MH[][] will be populated.
 *
 * \param plr      the index of the current player
 * \param mission  the mission plan with configured Days duration.
 */
void
MisSkip(const char plr, const struct mStr &mission)
{
    int i, j, k, diff, done;

    // MAX is a macro, not a function, so don't combine...
    diff = AchievementPenalty(plr, mission);
    diff = MAX(diff, 0);

    if (!AI[plr]) {
        INFO2("applying general penalty %d to mission safety", -diff);
    }

    if (diff != 0) {
        for (i = 0; i < (int) ARRAY_LENGTH(MH); i++) {
            for (j = 0; j < (int) ARRAY_LENGTH(MH[0]); j++) {
                if (MH[i][j] != NULL) {
                    // Don't subtract twice if already done
                    done = 0;
                    
                    if (i) {
                        for (k = 0; k < (int) ARRAY_LENGTH(MH[0]); k++) {
                            if (MH[0][k] == MH[i][j]) {
                                done = 1;
                            }
                        }
                    }

                    if (!done) {
                        MH[i][j]->MisSaf -= diff;
                    }
                }
            }
        }
    }
}

/**
 * Apply rushing penalty to individual mission steps.
 *
 * \param plr current player
 * \param rush_level
 */
void
MisRush(char plr, char rush_level)
{
    int i, j, k, diff, done;

    diff = 3 * rush_level;

    if (!AI[plr]) {
        INFO2("applying rushing penalty %d to mission safety", -diff);
    }

    if (diff != 0) {
        for (i = 0; i < (int) ARRAY_LENGTH(MH); i++) {
            for (j = 0; j < (int) ARRAY_LENGTH(MH[0]); j++) {
                if (MH[i][j] != NULL) {
                    // Don't subtract twice if already done
                    done = 0;

                    if (i) {
                        for (k = 0; k < (int) ARRAY_LENGTH(MH[0]); k++) {
                            if (MH[0][k] == MH[i][j]) {
                                done = 1;
                            }
                        }
                    }

                    if (!done) {
                        MH[i][j]->MisSaf -= diff;
                    }
                }
            }
        }
    }
}

/* vim: set noet ts=4 sw=4 tw=77: */
