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
#include "Buzz_inc.h"
#include "options.h"
#include "game_main.h"
#include "mc.h"
#include "prest.h"
#include "pace.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

void MissionParse(char plr, char *MCode, char *LCode, char pad);
char WhichPart(char plr, int which);
void MissionSteps(char plr, int mcode, int Mgoto, int step, int pad);


//********************************************************************
// Setup for the Mission
// Routines used by MControl and Future Missions

void MissionCodes(char plr, char val, char pad)
{
    GetMisType(val);
    MissionParse(plr, Mis.Code, Mis.Alt, pad);
    return;
}

void
MissionParse(char plr, char *MCode, char *LCode, char pad)
{
    int i, loc, j;

    STEP = 0;
    loc = pad;

    for (i = 0; MCode[i] != '|'; ++i) {
        switch (MCode[i]) {
        case '@':
            i++;
            MCode[i] = 'b';    // duration step
            MissionSteps(plr, MCode[i], LCode[STEP], STEP, loc - pad);
            break;

        case '~':              //printf("      :Delay of %d seasons\n",MCode[i+1]-0x30);
            for (j = 0; j < (MCode[i + 1] - 0x30); j++) {
                MissionSteps(plr, MCode[i + 2], LCode[STEP], STEP,
                             loc - pad);
            }

            i += 2;
            break;

        case '+':
            i++;
            loc = MCode[i] - 0x30 + pad - 1;
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
            MissionSteps(plr, MCode[i], LCode[STEP], STEP, loc - pad);
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

            MissionSteps(plr, MCode[i], LCode[STEP], STEP, loc - pad);
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

void MissionSteps(char plr, int mcode, int Mgoto, int step, int pad)
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
        if (MH[pad][Mission_Probe_DM] != NULL) {
            Mev[step].Class = Mission_Probe_DM;  // DM
        } else {
            // TODO: Huh? Satellite should be Probe/DM i.e. 3, so
            // comment doesn't make sense. Make sure this is correct.
            Mev[step].Class = 2;    // Satellite
        }

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

    case 'E':
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

    case 'M':
        Mev[step].PComp = WhichPart(plr, Mev[step].Prest = -20);
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

    case 'T': // Done on lunar launch for good reason
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

    if (PastBANG == 1 && Mev[step].PComp > 0) {
        Mev[step].PComp = 5;
    }

    if (mcode == 'd') {
        // Alternative path Mission.Alt
        if (Mgoto == 0) {
            Mev[step - 1].sgoto = Mev[step - 1].fgoto = 100;
        } else {
            Mev[step - 1].sgoto = Mev[step - 1].fgoto = Mgoto;
        }
    } else {
        Mev[step].asf = 0;

        if (MANNED[pad] > 0)
            switch (Mev[step].Class) {
            case Mission_Capsule:
                Mev[step].ast = CAP[pad]; // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->Cap;
                }

                break;

            case Mission_LM:
                Mev[step].ast = LM[pad]; // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->LM;
                }

                break;

            case Mission_Probe_DM:  // docking
                Mev[step].ast = DOC[pad]; // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->Docking;
                }

                break;

            case Mission_EVA:  // eva
                Mev[step].ast = EVA[pad]; // index into MA

                if (Mev[step].ast >= 0) {
                    Mev[step].asf = MA[pad][Mev[step].ast].A->EVA;
                }

                break;

            case 7:  // covers power-on for docking module
                Mev[step].ast = -1;
                Mev[step].asf = 0;
                break;

            case 10: // durations
                Mev[step].Class = Mission_Capsule;
                Mev[step].ast = -1;
                Mev[step].asf = 0;
                break;

            default: // remaining
                Mev[step].ast = -1;
                Mev[step].asf = 0;
                break;
            };


//      if (step==0 && Data->P[plr].TurnOnly==5)
//         *Mev[step].sf=MIN(*Mev[step].sf,50);

        Mev[step].step = step;

        if ((Data->Def.Lev1 == 0 && plr == 0) || (Data->Def.Lev2 == 0 && plr == 1)) {
            Mev[STEP].dice = MisRandom();
        } else {
            Mev[STEP].dice = brandom((AI[plr]) ? 98 : 100) + 1;
        }

        Mev[step].rnum = brandom(10000) + 1;
        Mev[step].sgoto = 0;

        Mev[step].fgoto = (Mgoto == -2) ? step + 1 : Mgoto;  // prevents mission looping
        Mev[step].dgoto = Mis.AltD[STEP];  // death branching (tm)
        Mev[step].E = MH[pad][Mev[step].Class];

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
                strncat(Mev[step].Name, "M2", 2);    // Kicker-C
            } else if (MH[pad][Mission_Capsule] &&
                       MH[pad][Mission_Capsule]->ID[1] == 0x34) {
                strncat(Mev[step].Name, "C4", 2);    // FourMan
            } else { // standard LMs
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

            if (Mev[step].loc == 32 && Mis.Lun) {
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

    if (Mev[step].loc == 32 || Mev[step].loc == 29) { // Fix _g special case #48010
        Mev[step].FName[2] = '0';
        Mev[step].FName[3] = '1';
    }

    // Special Cases for the Failure Mode Charts
    if ((Mev[step].loc == 0) && // MS Failure Launch
        MH[pad][Mev[step].Class] &&
        strncmp(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name, MH[pad][Mev[step].Class]->Name, 5) == 0) {
        Mev[step].FName[1] = '1';
    } else if (Mev[step].loc == 4 && // MS Failure Landing
               MH[pad][Mev[step].Class] &&
               strncmp(Data->P[plr].Manned[MANNED_HW_MINISHUTTLE].Name, MH[pad][Mev[step].Class]->Name, 5) == 0) {
        Mev[step].FName[1] = '3';
    } else if (plr == 1 && Mev[step].loc == 4) {
        if ((Mev[step].loc == 4) && // Soviet Capsules: Vostok
            MH[pad][Mev[step].Class] &&
            strncmp(Data->P[plr].Manned[MANNED_HW_ONE_MAN_CAPSULE].Name, MH[pad][Mev[step].Class]->Name, 5) == 0) {
            Mev[step].FName[1] = '1';
        } else {
            Mev[step].FName[1] = '2';    // Other Capsules
        }
    }


    // New expanded cases for failure mode charts
    if (Mev[step].Name[3] == 'C' && Mev[step].loc == 2) {
        Mev[step].FName[1] = '1';
    }

    if (strncmp(&Mev[step].Name[2], "M3C0", 4) == 0) {
        Mev[step].FName[1] = '1';
    }

    if (Mev[step].FName[3] != 'P') { // exclude any probes
        if (Mev[step].loc == 15 && Mev[step].Name[6] == 0x36) {
            Mev[step].FName[1] = '1';
        }

        if (Mev[step].loc == 16 && STEP > 8) {
            if (Mev[step - 1].loc == 18 && Mev[step].Name[4] >= 0x35) {
                Mev[step].FName[1] = '2';
            } else if (Mev[step - 1].loc == 18 && Mev[step].Name[4] <= 0x34) {
                Mev[step].FName[1] = '3';
            } else {
                Mev[step].FName[1] = '1';
            }
        }

        if ((Mev[step].loc >= 17 && Mev[step].loc <= 19) && (Mev[step].Name[4] <= 0x34)) {
            Mev[step].FName[1] = '1';
        }

        //if (Mev[step].loc==18 && Mev[step].Name[3]=='P') Mev[step].FName[1]='0';

        if (Mev[step].loc == 18 && (Mev[step].Name[3] != '4' && Mev[step].Name[2] != 'M')) {
            Mev[step].FName[1] = '0';
        }


        if (Mev[step].loc == 20 && (Mev[step].Name[4] == 0x34 || Mev[step].Name[4] == 0x33)) {
            Mev[step].FName[1] = '1';
        }

        Mev[step].StepInfo = 0;

    }

    //if (mcode!='d')
    STEP++;
    return;
}

void MisPrt(void)
{
    int i;

    for (i = 0; i < STEP - 1; i++) {
        Mev[i].dice = 100;
        Mev[0].E->MisSaf = 5;
        Mev[0].rnum = 9999;
    }

    return;
}

void MissionSetup(char plr, char mis)
{
    char i, j, t;
    DMFake = 0;
    GetMisType(Data->P[plr].Mission[mis].MissionCode);

    for (j = 0; j < (1 + Data->P[plr].Mission[mis].Joint); j++) {

        if ((Mis.mVab[j] & 0x10) > 0 && Data->P[plr].DockingModuleInOrbit > 0) { // DMO
            Data->P[plr].Mission[mis + j].Hard[Mission_Probe_DM] = 4;
            DMFake = 1;
        }

        // TODO: Or, rather than checking j == 0 and j == 1, just
        // use mis + j and use the loop.
        if (j == 0) { // Apollo (1)  + LM (1)
            if (Data->P[plr].Mission[mis].Hard[Mission_Capsule] == 2 && Data->P[plr].Mission[mis].Hard[Mission_LM] >= 0) {
                Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
                DMFake = 1;
            }
        }

        if (j == 1) { // Apollo (2)  + LM (1)
            if (Data->P[plr].Mission[mis + j].Hard[Mission_Capsule] == 2 && Data->P[plr].Mission[mis].Hard[Mission_LM] >= 0) {
                Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
                DMFake = 1;
            }
        }

        if (Data->P[plr].Mission[mis].MissionCode == Mission_Soyuz_LL) { // Soyuz Kicker-C
            Data->P[plr].Mission[mis].Hard[Mission_Probe_DM] = 4;
            DMFake = 1;
        }

        for (i = Mission_Capsule; i <= Mission_PhotoRecon; i++) {
            t = Data->P[plr].Mission[mis + j].Hard[i];
            MH[j][i] = NULL; // Clear Pointers

            if (t >= 0) {
                switch (i) {
                case Mission_Capsule:
                case Mission_LM:   // Cap - LM
                    MH[j][i] = &Data->P[plr].Manned[t];

                    if (MH[j][i]->Num && t != MANNED_HW_MINISHUTTLE) {
                        MH[j][i]->Num--;
                    }

                    MH[j][i]->Used++;
                    break;

                case Mission_Kicker:           // Kicker
                    MH[j][i] = &Data->P[plr].Misc[t];

                    if (MH[j][i]->Num) {
                        MH[j][i]->Num--;
                    }

                    MH[j][i]->Used++;
                    break;

                case Mission_Probe_DM:           // Secondary Equipment
                    if (t != 4) {
                        MH[j][i] = &Data->P[plr].Probe[t];
                        MH[j][i]->Used++;

                        if (Data->P[plr].Probe[t].Num > 0) {
                            Data->P[plr].Probe[t].Num -= 1;
                        }
                    } else {
                        MH[j][i] = &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE];

                        if (DMFake == 0) {
                            if (MH[j][i]->Num > 0) {
                                MH[j][i]->Num--;
                            }

                            MH[j][i]->Used++;
                        }
                    }

                    break;

                case Mission_PrimaryBooster:            // Rocket XX
                    t--;

                    if (t < 4) {
                        MH[j][i] = &Data->P[plr].Rocket[t];

                        if (MH[j][i]->Num > 0) {
                            MH[j][i]->Num--;
                        }

                        MH[j][i]->Used++;
                    } else {
                        MH[j][i] = &Data->P[plr].Rocket[t - 4];
                        MH[j][Mission_SecondaryBooster] =
                            &Data->P[plr].Rocket[ROCKET_HW_BOOSTERS];
                        MH[j][i]->Used++;
                        MH[j][Mission_SecondaryBooster]->Used++;

                        if (MH[j][i]->Num > 0) {
                            MH[j][i]->Num--;
                        }

                        if (MH[j][Mission_SecondaryBooster]->Num > 0) {
                            MH[j][Mission_SecondaryBooster]->Num--;
                        }
                    }

                    break;

                case Mission_EVA:  // EVA
                    MH[j][i] = &Data->P[plr].Misc[MISC_HW_EVA_SUITS];
                    break;

                case Mission_PhotoRecon:  // Photo Recon
                    MH[j][i] = &Data->P[plr].Misc[MISC_HW_PHOTO_RECON];
                    break;
                }

                if (MH[j][i] != NULL) {
                    MH[j][i]->SMods += MH[j][i]->Damage;    //Damaged Equipment, Nikakd, 10/8/10
                    MH[j][i]->MisSaf = MH[j][i]->Safety + MH[j][i]->SMods;

                    if (MH[j][i]->ID[1] >= 0x35 && i == Mission_LM &&
                        Data->P[plr].Mission[mis].MissionCode >= 53) {
                        switch (Data->P[plr].LMpts) {
                        case 0:
                            MH[j][i]->MisSaf -= 9;
                            break;

                        case 1:
                            MH[j][i]->MisSaf -= 6;
                            break;

                        case 2:
                            MH[j][i]->MisSaf -= 3;
                            break;
                        }
                    }
                }
            } // if t>=0
        } // for (0<7)
    } // for (0<2)

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
        for (i = 0; i < 7; i++) { // Ignore Boosters
            if (MH[j][i] != NULL && (MH[j][i]->MisSucc > 0 || MH[j][i]->MisFail > 0)) {

                MH[j][i]->SMods = MH[j][i]->Damage = MH[j][i]->DCost = 0;

                if (strncmp(MH[j][i]->Name, (i == Mission_Probe_DM) ? "DOC" : "PHO", 3) != 0 && MH[j][i]->MisSucc > 0) {
                    MH[j][i]->Safety++;

                    if (options.cheat_addMaxS) {
                        MH[j][i]->MaxRD++;
                    }

                    if (MH[j][i]->MaxRD > MH[j][i]->MaxSafety - 2) {
                        MH[j][i]->MaxRD = MH[j][i]->MaxSafety - 2;
                    }
                }

                if (strncmp(MH[j][i]->Name, "DOC", 3) == 0 && (MH[j][i]->MisFail + MH[j][i]->MisSucc) == 0) {
                    MH[j][i]->MisFail = 1;
                }

                if ((MH[j][i]->MisFail + MH[j][i]->MisSucc) == 0 && (strncmp(MH[j][i]->ID, "M3", 2) != 0)) {
                    MH[j][i]->MisFail++;
                }

                if ((MH[j][i]->MisFail + MH[j][i]->MisSucc) == 0 && MH[j][i]->ID[0] == 'P') {
                    MH[j][i]->MisFail++;
                }


                MH[j][i]->Failures += MH[j][i]->MisFail;

                MH[j][i]->Steps += (MH[j][i]->MisFail + MH[j][i]->MisSucc);

                if (i == Mission_PrimaryBooster &&
                    MH[j][Mission_SecondaryBooster] != NULL) { // Boosters
                    if (MH[j][Mission_PrimaryBooster]->MisSucc > 0)  {
                        MH[j][Mission_SecondaryBooster]->Safety++;

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
                        MH[j][Mission_PrimaryBooster]->MisFail;
                    MH[j][Mission_SecondaryBooster]->Steps +=
                        (MH[j][Mission_PrimaryBooster]->MisFail +
                         MH[j][Mission_PrimaryBooster]->MisSucc);
                    MH[j][Mission_SecondaryBooster]->MisSucc = 0;
                    MH[j][Mission_SecondaryBooster]->MisFail = 0;
                }

                MH[j][i]->MisSucc = MH[j][i]->MisFail = 0;
            } // if
        } // for i

        VerifySF(j);
    } // for j

    return;
}

/**
 * Apply duration penalty to mission steps in manned missions.
 *
 * TODO: This functionality appears to have been appropriated by
 * MisSkip, via PrestMin(). This needs to be verified, and if so, it
 * should be removed completely.
 * TODO: Addendum - PrestMin() has since been replaced with
 * AchievementPenalty(). This function is now out-of-date, and should
 * be removed or updated to use the latest Penalty functions.
 *
 * \param plr current player
 * \param dur mission duration in duration level (A-F).
 */
void
MisDur(char plr, char dur)
{
    int i, j, diff;
    int manned = 0;

    diff = dur - Data->P[plr].DurationLevel;

    if (Data->P[plr].DurationLevel == 0) {
        diff--;
    }

    if (diff <= 2) {  // Changed from "diff <= 0" to disable broken Duration milestone system  -Leon
        return;
    }

    diff = 5 * diff;

    if ((MH[0][Mission_Capsule] && MH[0][Mission_Capsule]->ID[0] == 'C') ||
        (MH[1][Mission_Capsule] && MH[1][Mission_Capsule]->ID[0] == 'C')) {
        manned = 1;
    }

    /* Don't give negs to unmanned */
    /* Don't give negs to duration missions */
    /* ??? will handle individual durations later */
    if (!manned || !Mis.Dur) {
        return;
    }

    if (!AI[plr]) {
        INFO2("applying duration penalty %d to mission safety", -diff);
    }

    for (i = 0; i < (int) ARRAY_LENGTH(MH); i++) {
        for (j = 0; j < (int) ARRAY_LENGTH(MH[0]); j++) {
            if (MH[i][j] != NULL) {
                MH[i][j]->MisSaf -= diff;
            }
        }
    }
}

// #define Coml(a,b) (!(Data->Prestige[b].Place==(a) || Data->Prestige[b].mPlace==(a)))


/**
 * Compute and apply safety penalties to mission steps.
 *
 *
 * TODO: This takes a parameter ms, used originally to determine the
 * maximum prestige requirement of the mission. However, it's no
 * longer used, and should be removed.
 *
 * \note  This assumes the global variable Mis is correctly filled.
 * \param plr current player
 */
void
MisSkip(char plr, char ms)
{
    int i, j, diff;

    diff = AchievementPenalty(plr, Mis);

    diff = MAX(diff, 0);

    if (!AI[plr]) {
        INFO2("applying general penalty %d to mission safety", -diff);
    }

    if (diff != 0) {
        for (i = 0; i < (int) ARRAY_LENGTH(MH); i++) {
            for (j = 0; j < (int) ARRAY_LENGTH(MH[0]); j++) {
                if (MH[i][j] != NULL) {
                    MH[i][j]->MisSaf -= diff;
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
    int i, j, diff;

    diff = 3 * rush_level;

    if (!AI[plr]) {
        INFO2("applying rushing penalty %d to mission safety", -diff);
    }

    if (diff != 0) {
        for (i = 0; i < (int) ARRAY_LENGTH(MH); i++) {
            for (j = 0; j < (int) ARRAY_LENGTH(MH[0]); j++) {
                if (MH[i][j] != NULL) {
                    MH[i][j]->MisSaf -= diff;
                }
            }
        }
    }
}

/* vim: set noet ts=4 sw=4 tw=77: */
