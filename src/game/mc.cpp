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

// This file handles Mission Control during missions

#include "mc.h"

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "Buzz_inc.h"
#include "data.h"
#include "filesystem.h"
#include "game_main.h"
#include "logging.h"
#include "mc2.h"
#include "mis.h"
#include "mis_c.h"
#include "mis_m.h"
#include "mission_util.h"
#include "newmis.h"
#include "news_suq.h"
#include "pace.h"
#include "pbm.h"
#include "prest.h"
#include "records.h"
#include "state_utils.h"
#include "sdlhelper.h"
#include "randomize.h"

LOG_DEFAULT_CATEGORY(mission)

Equipment* MH[2][8];   // Pointer to the hardware
MisAst MA[2][4];  //[2][4]
MisEval Mev[60];  // was *Mev;
REPLAY Rep;

char tMen;

char MANNED[2];
char CAP[2];
char LM[2];
char DOC[2];
char EVA[2];
char STEP;
char FINAL;
char JOINT;
char PastBANG;
char mcc;
char fEarly; /**< kind of a boolean indicating early missions */
char hero;
char DMFake;
/* STEP tracks mission step numbers             */
/* FINAL is the ultimate result of safety check */
/* JOINT signals the joint mission code         */


void DrawControl(char plr);
void SetW(char ch);
void MissionPast(char plr, char pad, int prest);
int MaxFailPad(char which);


/**
 * Draws the Mission Control background image during missions.
 *
 * The background control.img.X.png images are 320x240 pixels.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \throws runtime_error  if Filesystem unable to read the image.
 */
void DrawControl(char plr)
{
    char filename[128];
    snprintf(filename, sizeof(filename), "images/control.img.%d.png", plr);

    boost::shared_ptr<display::PalettizedSurface> background(
        Filesystem::readImage(filename));
    background->exportPalette();
    display::graphics.screen()->draw(background, 0, 0);
}


void SetW(char ch)
{
    int i = 0;

    while (Mev[i].Name[0] != 'W') {
        i++;
    }

    Mev[i].Name[2] = ch;
}

int Launch(char plr, char mis)
{
    LOG_DEBUG("Launch(plr=%i, mis=%i)", plr, mis);
    STEP = FINAL = JOINT = PastBANG = 0;
    tMen = 0x00; // clear mission status flags

    // Don't do missions twice, just update prestige data
    if ((MAIL == 1 && plr == 0) || (MAIL == 2 && plr == 1)) {
        LOG_DEBUG("Mail game replay state detected");
        STEPnum = Data->Step[mis];
        memcpy(Mev, Data->Mev[mis], 60 * sizeof(struct MisEval));
        // Check for Mission death
        int spResult = Data->P[plr].History[Data->P[plr].PastMissionCount].spResult;
        if (spResult >= 3000 && spResult < 5000) {
            death = 1;
        } else {
            death = 0;
        }

        MANNED[0] = Data->P[plr].Mission[mis].Men;
        MANNED[1] = Data->P[plr].Mission[mis].Joint ? Data->P[plr].Mission[mis + 1].Men : 0;

        return Update_Prestige_Data(
                   plr, mis, Data->P[plr].Mission[mis].MissionCode);
    }

    remove_savedat("REPLAY.TMP");  // make sure replay buffer isn't there

    if (Data->P[plr].Mission[mis].part == 1) {
        return 0;
    }

    memset(buffer, 0x00, BUFFER_SIZE); // Clear Buffer
    memset(MH, 0x00, sizeof MH);
    memset(Mev, 0x00, sizeof Mev);
    LOG_DEBUG("buffers cleared to zero");

    if (Data->P[plr].Mission[mis].MissionCode == Mission_SubOrbital) {
        Data->P[plr].Mission[mis].Duration = 1;
    }

    MANNED[0] = Data->P[plr].Mission[mis].Men;
    MANNED[1] = Data->P[plr].Mission[mis].Joint ? Data->P[plr].Mission[mis + 1].Men : 0;

    JOINT = Data->P[plr].Mission[mis].Joint;

    if (CheckCrewOK(plr, mis) == 1) { // found mission no crews
        LOG_DEBUG("Crew is not good, mission scrubbed");
        ScrubMission(plr, mis - Data->P[plr].Mission[mis].part);
    }

    if (!AI[plr] && Data->P[plr].Mission[mis].MissionCode != Mission_None) {
        MisAnn(plr, mis);
    }

    if (Data->P[plr].Mission[mis].MissionCode == Mission_None) {
        return -20;
    }

    MissionSetup(plr, mis);

    // ****************************************
    // Do all Manned Associated Stuff

    memset(MA, 0x00, sizeof MA);

    for (int i = 0; i < (1 + JOINT); i++) {
        // Decide who to use for each pad
        Data->P[plr].Mission[mis + i].Crew = (Data->P[plr].Mission[mis + i].PCrew > 0) ?
                                             Data->P[plr].Mission[mis + i].PCrew :  Data->P[plr].Mission[mis + i].BCrew ;

        for (int j = 0; j < MANNED[i]; j++) {
            int t = Data->P[plr].Mission[mis + i].Prog;
            int k = Data->P[plr].Mission[mis + i].Crew - 1;
            int total = Data->P[plr].Crew[t][k][j] - 1;
            MA[i][j].A = &Data->P[plr].Pool[total];
            MA[i][j].loc = i;
        }
    }

    // 0 here is for unused

    CAP[0] = LM[0] = EVA[0] = DOC[0] = CAP[1] = LM[1] = EVA[1] = DOC[1] = -1;

    for (int i = 0; i < (1 + JOINT); i++) {
        switch (MANNED[i]) {
        case 0:
            CAP[i] = LM[i] = EVA[i] = DOC[i] = -1;
            break;

        case 1:
            CAP[i] = LM[i] = EVA[i] = 0;
            DOC[i] = -1;
            break;

        case 2:
            CAP[i] = DOC[i] = 0;
            LM[i] = EVA[i] = 1;
            break;  // Last in LM

        case 3:
            CAP[i] = 0;
            LM[i] = EVA[i] = 1;
            DOC[i] = 2;
            break;

        case 4:
            CAP[i] = 0;
            LM[i] = 1;
            EVA[i] = 2;
            DOC[i] = 3;
            break;
        }
    }

    // END MEN SETUP ****************************

    // Do actual Missions

    int mcode = Data->P[plr].Mission[mis].MissionCode;
    mcc = Data->P[plr].Mission[mis].MissionCode;
    mStr misType = GetMissionPlan(mcode);

    // Fixup for Mercury Duration C stuff
    if (Data->P[plr].Mission[mis].Hard[Mission_Capsule] == 0) {
        Data->P[plr].Mission[mis].Duration = MIN(2, Data->P[plr].Mission[mis].Duration);
    }

    MissionCodes(plr, mcode, mis);

    /////////////////////////////////////////////////
    // Fix for BARIS CD-ROM Planetary Steps (Step W)
    // E=Moon; M= Mars; S = Saturn; V=Venus; J= Jupiter; R= Mercury
    // Must be at .Name[2]
    //
    // Search for Step 'W' on planetary steps
    //

    if (mcode == Mission_LunarFlyby) {
        SetW('E');
    } else if (mcode == Mission_VenusFlyby) {
        SetW('V');
    } else if (mcode == Mission_MarsFlyby) {
        SetW('M');
    } else if (mcode == Mission_MercuryFlyby) {
        SetW('R');
    } else if (mcode == Mission_JupiterFlyby) {
        SetW('J');
    } else if (mcode == Mission_SaturnFlyby) {
        SetW('S');
    }

    // Exit missions early
    /** \todo The *early* missions should be defined in a file */
    fEarly = (!misType.Days && !(mcode == Mission_Orbital_Satellite ||
                                 mcode == Mission_LunarFlyby ||
                                 mcode == Mission_Lunar_Probe ||
                                 mcode == Mission_VenusFlyby ||
                                 mcode == Mission_MercuryFlyby));

    STEPnum = STEP;

    if (misType.Dur >= 1) {
        misType.Days = Data->P[plr].Mission[mis].Duration;
    }

    // Apply general mission penalties (Duration, Milestone, New Mission)
    // and Rushing penalties. MisSkip requires the second argument to be
    // loaded with the mission data AND corrects Days value for a
    // duration mission.
    LOG_DEBUG("checking penalties");
    MisSkip(plr, misType);
    MisRush(plr, Data->P[plr].Mission[mis].Rushing);
    STEPnum = 0;

    if (!AI[plr] && !fullscreenMissionPlayback) {
        DrawControl(plr);
        FadeIn(2, 10, 0, 0);
    } else if (fullscreenMissionPlayback) {
        display::graphics.screen()->clear();
    }

    // TODO: Should explicitly only apply to Lunar Landing missions,
    // since there are (unimplemented) rescue missions with mission
    // codes greater than the lunar landing missions.
    if (!AI[plr] && mcode >= Mission_HistoricalLanding) {
        LOG_DEBUG("Calculating average chance of succesful lunar mission");
        int avg = 0;
        int temp = 0;

        for (int i = 0; Mev[i].loc != 0x7f; ++i) {
            /* Bugfix -> We need to skip cases when Mev[i].E is NULL */
            /* Same solution as used in mis_m.c (MisCheck):207 */
            if (!GetEquipment(Mev[i])) {
                continue;
            }

            avg += GetEquipment(Mev[i])->MisSaf + Mev[i].asf;
            temp += 1;
        }

        LOG_DEBUG("avg=%i, temp=%i", avg, temp);
        if (temp) {
            avg /= temp;
        } else {
            avg = 0;
        }

        if (avg >= 3 && avg <= 105) {
            SafetyRecords(plr, avg);
        }
    }

//   if (!AI[plr]) {PreLoadMusic(M_ELEPHANT); PlayMusic(0);}

    // TODO: Shouldn't apply easymode if player is on difficulty III
    // and opponent difficulty I.
    if ((Data->Def.Lev1 == 0 && Data->Def.Lev2 == 2) ||
        (Data->Def.Lev2 == 0 && Data->Def.Lev1 == 2)) {
        LOG_DEBUG("applying EASYMODE");
        xMODE |= EASYMODE;    // set easy flag
    }

    if (AI[plr]) {
        LOG_DEBUG("dis=applying EASYMODE(?)");
        xMODE &= ~xMODE_EASYMODE;    // map out computer from really easy level
    }


    LOG_DEBUG("%i", Mev[0].trace);
    MisCheck(plr, mis); // Mission Resolution
    LOG_DEBUG("%i", Mev[0].trace);

    xMODE &= ~xMODE_EASYMODE;

//   if (!AI[plr]) KillMusic();

    if (MAIL == 0 || MAIL == 3) {
        Data->Step[mis] = STEPnum;
    }

    int total = Update_Prestige_Data(plr, mis, misType.Index);

    MissionSetDown(plr, mis);
    MissionPast(plr, mis, total);
    // Update the Astros

    for (int i = 0; i < 1 + JOINT; i++) {
        /* XXX: was MANNED[i]+1, but why? */
        for (int j = 0; j < MANNED[i]; j++) {
            if (MA[i][j].A == nullptr) continue;
            if (FINAL >= 100) {
                MA[i][j].A->currentMissionStatus = ASTRO_MISSION_SUCCESS;    // Successful
            } else if (Data->P[plr].MissionCatastrophicFailureOnTurn & 4) {
                MA[i][j].A->currentMissionStatus = ASTRO_MISSION_SUCCESS;    // Failure
            }
        }
    }

    if (MAIL == 0 || MAIL == 3) {
        memcpy(Data->Mev[mis], Mev, 60 * sizeof(struct MisEval));
    }

    fullscreenMissionPlayback = false;
    return total;
}


void MissionPast(char plr, char pad, int prest)
{
    LOG_DEBUG("MissionPast(plr=%i, pad=%i, prest=%i)", plr, pad, prest);
    char dys[7] = {0, 2, 5, 7, 12, 16, 20};

    int loc = Data->P[plr].PastMissionCount;
    int mc = Data->P[plr].Mission[pad].MissionCode;
    
    PastInfo& hist = Data->P[plr].History[loc];
    memset(&hist, -1, sizeof(struct PastInfo));
    strcpy(&hist.MissionName[0][0], Data->P[plr].Mission[pad].Name);
    hist.Patch[0] = Data->P[plr].Mission[pad].Patch;

    if (Data->P[plr].Mission[pad].Joint == 1) {
        strcpy(&hist.MissionName[1][0], Data->P[plr].Mission[pad + 1].Name);
        hist.Patch[1] = Data->P[plr].Mission[pad + 1].Patch;
    } else {
        hist.MissionName[1][0] = 0;
    }

    // TODO: There's a lot of what appears to be duplicated code here,
    // concerning hist.Event & .Saf

    // Flag for if mission is done
    hist.Event = 0;
    hist.Saf = 0;

    if (mc == Mission_MarsFlyby) {
        hist.Event = 2;
    } else if (mc == Mission_JupiterFlyby) {
        hist.Event = 4;
    } else if (mc == Mission_SaturnFlyby) {
        hist.Event = 7;
    }

    if ((mc == Mission_MarsFlyby || mc == Mission_JupiterFlyby ||
         mc == Mission_SaturnFlyby) && prest != 0) {
        hist.Event = 0;
    }

    if (MH[0][Mission_Probe_DM]) {
        hist.Saf = MH[0][Mission_Probe_DM]->MisSaf;
    }

    if (!(mc == Mission_MarsFlyby || mc == Mission_JupiterFlyby ||
          mc == Mission_SaturnFlyby)) {
        hist.Event = hist.Saf = 0;
    }

    hist.MissionCode = Data->P[plr].Mission[pad].MissionCode;
    hist.MissionYear = Data->Year;
    hist.Month = Data->P[plr].Mission[pad].Month;
    hist.Prestige = MAX(prest, -10);
    hist.Duration = Data->P[plr].Mission[pad].Duration;

    int nd = 0;

    int number_of_launches = Data->P[plr].Mission[pad].Joint + 1;
    for (int loop = 0; loop < number_of_launches; loop++) {
        int launchpad = pad+loop;
        auto& mission = Data->P[plr].Mission[launchpad];
        int i = mission.Prog;
        int j = mission.Crew - 1;

        for(int q=0; q<4; ++q) {
            hist.Man[loop][q] = (mission.Men > 0)? Data->P[plr].Crew[i][j][q] - 1
                                                 : -1;
        }

        if (mission.Men <= 0) continue;
        
        for (int i = 0; i < 4; i++) {
            int j = Data->P[plr].Crew[mission.Prog][mission.Crew - 1][i] - 1;
            if (j < 0) continue;

            Data->P[plr].Pool[j].MissionNum[Data->P[plr].Pool[j].Missions] = loc;

            Data->P[plr].Pool[j].Missions++;
            Data->P[plr].Pool[j].Prestige += prest;
            int tnd = dys[mission.Duration];  // Variables for total # days possible for the mission, actual days spent on the mission

            if (nd == 0 || nd > 20) {  // Don't do the following if nd is already set, or each crew member will get a different # of days
                nd = tnd;
                int miscode = Data->P[plr].Mission[pad].MissionCode;   // Get mission number

                switch (tnd) {
                case 0:   // Unmanned mission
                    break;

                case 2:   // Duration A
                    if (miscode < 14) {
                        nd = 1;  // Suborbital, Orbital, Orbital EVA = 1; docking, LM test = 2 days
                    }

                    break;

                case 5:   // Duration B
                    if (miscode < 27) {
                        nd = 3;  // Duration, DurEVA = 3 days
                    } else if (miscode < 40) {
                        nd = 4;  // Jt docking = 4 days; Jt LM test = 5 days
                    }

                    break;

                case 7:   // Duration C
                    if (miscode < 32) {
                        nd = 6;  // Duration = 6 days; MOL, Lunar Pass 7 days
                    }

                    break;

                case 12:  // Duration D
                    if (miscode < 44) {
                        nd = 8;  // Duration = 8 days
                    } else if (miscode < 44 || miscode == 46 || miscode == 48 || miscode == 50)  {
                        nd = 8 + brandom(4);  // Random 8-11 days for single lunar orbitals
                    } else {
                        nd = 9 + brandom(4);  // Random 9-12 days for Jt lunar orbitals or any lunar landing
                    }

                    break;

                case 16:  // Duration E
                    nd = 13 + brandom(4);  // Random 13-16 days for any Dur E
                    break;

                case 20:  // Duration F
                    nd = 17 + brandom(4);  // Random 17-20 days for any Dur F
                    break;
                }
            }

            Data->P[plr].Pool[j].Days += nd;

            if (hero & 0x01) {
                Data->P[plr].Pool[j].Hero = 1;
            } else if (hero & 0x02 && j == EVA[loop]) {
                Data->P[plr].Pool[j].Hero = 1;
            }
        }
    }

    for (int i = Mission_Capsule; i <= Mission_PrimaryBooster; i++) {
        hist.Hard[PAD_A][i] = Data->P[plr].Mission[pad].Hard[i];

        if (Data->P[plr].Mission[pad].Joint == 1) {
            hist.Hard[PAD_B][i] = Data->P[plr].Mission[pad + 1].Hard[i];
        }
    }

    hist.result = FINAL;
    hist.spResult = MaxFail();

    if (hist.spResult >= 4000 && MANNED[0] > 0 && MANNED[1] > 0) {
        // first -all killed
        if (MaxFailPad(0) >= 4000 && MaxFailPad(1) < 4000) {
            hist.spResult = 4197;
        }
        // second -all killed
        else if (MaxFailPad(0) < 4000 && MaxFailPad(1) >= 4000) {
            hist.spResult = 4198;
        }

        // first and second all killed
        else if (MaxFailPad(0) >= 4000 && MaxFailPad(1) >= 4000) {
            hist.spResult = 4199;
        }

        if (MANNED[0] > 0 && MANNED[1] > 0 && tMen == (MANNED[0] + MANNED[1])) {
            hist.spResult = 4199;
        }

    }

    if (interimData.tempReplay.at((plr * 100) + loc).size() == 1 && hist.spResult < 3000) {
        hist.spResult = 1999;
    }

    Data->P[plr].PastMissionCount++;
    assert(Data->P[plr].PastMissionCount < MAX_MISSION_COUNT);
}


int MaxFailPad(char which)
{
    int i = 0, t = 0;

    while (i != 0x7f) {
        if (Mev[i].pad == which) {
            if (Mev[i].StepInfo == 0) {
                Mev[i].StepInfo = 1003;
            }

            t = MAX(Mev[i].StepInfo, t);
        }

        i = Mev[i].trace;
    }

    return t;
}

/* vim: set noet ts=4 sw=4 tw=77: */
