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

// This file is part of mission branching and failure handling

#include <cassert>

#include "display/graphics.h"
#include "display/surface.h"

#include "mis_m.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "utils.h"
#include "options.h"
#include "game_main.h"
#include "mc.h"
#include "mis_c.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"

LOG_DEFAULT_CATEGORY(mission)

char MFlag;
char death;
char durx;
char MPad;
char Unm;
char SCRUBS;
char noDock;
char InSpace;
char Dock_Skip; /**< used for mission branching */

extern uint16_t MisStat;
extern char tMen;
extern bool fullscreenMissionPlayback;

void Tick(char);

void GetFailStat(struct XFails *Now, char *FName, int rnum);
int MCGraph(char plr, int lc, int safety, int val, char prob);
void F_KillCrew(char mode, struct Astros *Victim);
void F_IRCrew(char mode, struct Astros *Guy);
int FailEval(char plr, int type, char *text, int val, int xtra);

/**
 * Load the failure state information explaining a mission step
 * failure.
 *
 * Step Failure reports are represented by an XFails structure,
 * comprising:
 *    per  -
 *    code -
 *    val  -
 *    xtra -
 *    fail -
 *    text - The written description of the failure, displayed in
 *           the step failure report.
 *
 * For unmanned steps, per lies in [-5, -1]. The negative value marks
 * it as unmanned and means it is always less than a manned value. It
 * also produces an even probability distribution. For Manned steps,
 * per is designed so each entry's per sits within [0, 10000) and
 * the relative values create an uneven probability distribution.
 *
 * TODO: Add error handling in case FName does not match a code.
 * At minimum, log an error. Preferably, throw an exception.
 *
 * \param Now    destination for the fail state read from the file
 *               (Not NULL).
 * \param FName  the code giving the step failure type.
 * \param rnum   between -1 and -5 for unmanned steps, 0 and 10000 for
 *               manned.
 */
void GetFailStat(struct XFails *Now, char *FName, int rnum)
{
    DEBUG2("->GetFailStat(struct XFails *Now,char *FName,int rnum %d)",
           rnum);
    FILE *fin;
    int32_t count;
    struct Fdt {
        char Code[6];
        int32_t offset;
        int16_t size;
    } Pul;

    // const size_t sizeof_Pul = 6 + 4 + 2;
    // const size_t sizeof_XFails = 4 + 2 + 2 + 2 + 2 + 200;

    assert(Now != NULL);

    fin = sOpen("FAILS.CDR", "rb", 0);
    count = 44;
    fread(&count, sizeof count, 1, fin);  // never written to file
    Swap32bit(count);

    int i = 0;

    do {
        fread(&Pul.Code[0], sizeof Pul.Code, 1, fin);
        fread(&Pul.offset, sizeof Pul.offset, 1, fin);
        Swap32bit(Pul.offset);
        fread(&Pul.size, sizeof Pul.size, 1, fin);
        Swap16bit(Pul.size);
    } while (xstrncasecmp(Pul.Code, FName, 4) != 0 && ++i < count);

    // This uses short-circuit evaluation to distinguish between
    // string match and iterator comparison.

    if (i == count) {
        fclose(fin);
        return;
    }

    fseek(fin, Pul.offset, SEEK_SET);

    if (rnum < 0) { // Unmanned portion
        do {
            // Only need to swap Now->per since we're checking only that
            fread(&Now->per, sizeof(Now->per), 1, fin);
            fread(&Now->code, sizeof(Now->code), 1, fin);
            fread(&Now->val, sizeof(Now->val), 1, fin);
            fread(&Now->xtra, sizeof(Now->xtra), 1, fin);
            fread(&Now->fail, sizeof(Now->fail), 1, fin);
            fread(&Now->text[0], sizeof(Now->text), 1, fin);
            Swap32bit(Now->per);
            Swap16bit(Now->code);
            Swap16bit(Now->val);
            Swap16bit(Now->xtra);
            Swap16bit(Now->fail);
        } while (Now->per != rnum);
    } else {
        do {
            fread(&Now->per, sizeof(Now->per), 1, fin);
            fread(&Now->code, sizeof(Now->code), 1, fin);
            fread(&Now->val, sizeof(Now->val), 1, fin);
            fread(&Now->xtra, sizeof(Now->xtra), 1, fin);
            fread(&Now->fail, sizeof(Now->fail), 1, fin);
            fread(&Now->text[0], sizeof(Now->text), 1, fin);
            Swap32bit(Now->per);
            Swap16bit(Now->code);
            Swap16bit(Now->val);
            Swap16bit(Now->xtra);
            Swap16bit(Now->fail);
        } while (Now->per <= rnum);
    }

    fclose(fin);

    DEBUG1("<-GetFailStat()");
}

void MisCheck(char plr, char mpad)
{
    int tomflag = 0;  // Tom's checking flag
    int val, safety, save, PROBLEM, i, lc, durxx;
    struct XFails Now;
    unsigned char gork = 0;

    lc = 0; /* XXX check uninitialized */

    STEPnum = STEP;
    FINAL = STEP = MFlag = 0;  // Clear Everything
    Unm = MANNED[0] + MANNED[1];
    Dock_Skip = 0;  // used for mission branching
    MPad = mpad;
    SCRUBS = noDock = InSpace = 0;



    if (!AI[plr] && !fullscreenMissionPlayback) {
        //FadeOut(1,pal,100,128,1);
        if (plr == 1) {
            fill_rectangle(189, 173, 249, 196, 55);

            for (i = 190; i < 250; i += 2) {
                display::graphics.legacyScreen()->setPixel(i, 178, 61);
                display::graphics.legacyScreen()->setPixel(i, 184, 61);
                display::graphics.legacyScreen()->setPixel(i, 190, 61);
            }

            lc = 191;
        } else if (plr == 0) {
            fill_rectangle(73, 173, 133, 196, 55);

            for (i = 73; i < 133; i += 2) {
                display::graphics.legacyScreen()->setPixel(i, 178, 61);
                display::graphics.legacyScreen()->setPixel(i, 184, 61);
                display::graphics.legacyScreen()->setPixel(i, 190, 61);
            }

            lc = 76;
        }
    } // END if (!AI[plr])

    if (!AI[plr] && !fullscreenMissionPlayback) {
        Tick(2);
    }

    Mev[0].trace = 0;
    death = 0;

    durxx = durx = -1;

    if (Data->P[plr].Mission[mpad].Duration > 0) {
        durxx = Data->P[plr].Mission[mpad].Duration - 1;
        Data->P[plr].Mission[mpad].Duration = 0;
    }

    if (JOINT == 1) {
        durxx = MAX(durxx, Data->P[plr].Mission[mpad + 1].Duration - 1);
        Data->P[plr].Mission[mpad + 1].Duration = 0;
    }

    do {
        if (STEP > 30 || STEP < 0) {
            delay(20);
        }

        if (Dock_Skip == 1) {
            if (Mev[Mev[STEP].trace].loc == 8) {
                Mev[STEP].trace++;    // skip over docking.
            }
        }

        if (Mev[STEP].loc == 16 && Mis.PCat[4] == 22) {
            FirstManOnMoon(plr, 0, Mis.Index);
        }

        // Duration Hack Part 1 of 3   (during the Duration stuff)
        if ((Mev[STEP].loc == 27 || Mev[STEP].loc == 28) && durx > 0) {

            if (Mev[STEP].StepInfo != 1) {
                Data->P[plr].Mission[MPad + Mev[STEP].pad].Duration = 1;  // Original code would also return 1
                durx = -1;  // end durations
            } else {
                Data->P[plr].Mission[MPad + Mev[STEP].pad].Duration++;
                durx--;

                if ((Data->Def.Lev1 == 0 && plr == 0) || (Data->Def.Lev2 == 0 && plr == 1)) {
                    Mev[STEP].dice = MisRandom();
                } else {
                    Mev[STEP].dice = brandom(100) + 1;
                }

                Mev[STEP].rnum = brandom(10000);  // reroll failure type
                Mev[STEP].trace = STEP;
            }
        }

        if (Mev[STEP].Name[0] == 'A' || Mev[STEP].StepInfo == 9 || Mev[STEP].StepInfo == 19) {
            STEP = Mev[STEP].trace;
        } else {
            if (Mev[STEP].trace != 0) {
                STEP = Mev[STEP].trace;
            } else {
                Mev[STEP].trace = STEP + 1;
                STEP++;
            }
        }

        // Duration Hack Part 2 of 3    (set up durx for duration use)
        if ((Mev[STEP].loc == 27 || Mev[STEP].loc == 28) && durx == -1) {
            durx = durxx - 1;
            Data->P[plr].Mission[MPad + Mev[STEP].pad].Duration = 2;
        }

        if (Mev[STEP].Name[0] == 'A') {
            display::graphics.setForegroundColor(11);

            if (!AI[plr] && !fullscreenMissionPlayback) {
                if (plr == 0) {
                    x = 5;
                    y = 112;
                    fill_rectangle(2, 107, 140, 115, 3);
                } else {
                    x = 82, y = 8;
                    fill_rectangle(78, 2, 241, 10, 3);
                }

                draw_string(x, y, "COUNTDOWN");

                if (plr == 0) {
                    fill_rectangle(188, 107, 294, 113, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string(190, 112, (Mev[STEP].pad == 0) ? "PRIMARY LAUNCH" : "SECOND LAUNCH");
                } else {
                    fill_rectangle(244, 56, 314, 62, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string(246, 61, (Mev[STEP].pad == 0) ? "PRIMARY PAD" : "SECOND PAD");
                }

            }

            memset(Name, 0x00, sizeof Name);
            strcpy(Name, Mev[STEP].Name);
            Name[0] = '#';  // Launch Code
            PlaySequence(plr, STEP, Name, 0);  // Special Case #47236
        }

        // Necessary to keep code from crashing on bogus mission step
        while (Mev[STEP].E == NULL) {
            STEP++;
        }

        // Draw Mission Step Name
        if (!AI[plr] && !fullscreenMissionPlayback) {
            if (!(fEarly && STEP != 0)) {
                if (plr == 0) {
                    x = 5;
                    y = 112;
                    fill_rectangle(2, 107, 140, 115, 3);
                } else {
                    x = 82, y = 8;
                    fill_rectangle(78, 2, 241, 10, 3);
                }

                display::graphics.setForegroundColor(11);
                MisStep(x, y, Mev[STEP].loc);

                if (plr == 0) {
                    fill_rectangle(188, 107, 294, 113, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string(190, 112, (Mev[STEP].pad == 0) ? "PRIMARY LAUNCH" : "SECOND LAUNCH");
                } else {
                    fill_rectangle(244, 56, 314, 62, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string(246, 61, (Mev[STEP].pad == 0) ? "PRIMARY PAD" : "SECOND PAD");
                }
            }
        }


        // SAFETY FACTOR STUFF

        safety = Mev[STEP].E->MisSaf;

        if ((Mev[STEP].Name[0] == 'A') &&
            MH[Mev[STEP].pad][Mission_SecondaryBooster] != NULL) {
            // boosters involved
            safety = RocketBoosterSafety(safety, MH[Mev[STEP].pad][Mission_SecondaryBooster]->Safety);
        }

        // Duration Hack Part 3 of 3
        if (Mev[STEP].loc == 28 || Mev[STEP].loc == 27) {
            safety = Mev[STEP].E->MisSaf;  // needs to be for both

            // Use average of capsule ratings for Joint duration
            if (InSpace == 2) {
                safety = (MH[0][Mission_Capsule]->MisSaf +
                          MH[1][Mission_Capsule]->MisSaf) / 2;
            }
        }

        if (strncmp(Mev[STEP].E->Name, "DO", 2) == 0) {
            if (Mev[STEP].loc == 1 || Mev[STEP].loc == 2) {
                safety = Mev[STEP].E->MSF;
            }
        }

        val = Mev[STEP].dice;
        safety += Mev[STEP].asf;

        if (safety >= 100) {
            safety = 99;
        }

        save = (Mev[STEP].E->SaveCard == 1) ? 1 : 0;

        PROBLEM = val > safety;

        if (!AI[plr] && options.want_cheats) {
            PROBLEM = 0;
        }

        DEBUG6("step %c:%s safety %d rolled %d%s", Mev[STEP].Name[0], S_Name[Mev[STEP].loc],
               safety, val,
               PROBLEM ? " problem" : (options.want_cheats ? " cheating" : ""));

        if (!AI[plr] && !fullscreenMissionPlayback)
            if (!(fEarly && STEP != 0)) {
                lc = MCGraph(plr, lc, MAX(0, safety), MAX(0, val), PROBLEM);    // Graph Chart
            }

        if (PROBLEM && save == 1) {  // Failure Saved
            Mev[STEP].E->SaveCard--;    // Deduct SCard
            PROBLEM = 0;  // Fix problem
        }


        // Fix wrong anim thing for the Jt Durations
        if (Mev[STEP].loc == 28 || Mev[STEP].loc == 27) {
            strcpy(Mev[STEP].Name, (plr == 0) ? "_BUSC0\0" : "_BSVC0");
            Mev[STEP].Name[5] = Mev[STEP].E->ID[1];
        }

        if (PROBLEM == 1) {  // Step Problem
            // for the unmanned mission
            if (MANNED[Mev[STEP].pad] == 0 && MANNED[other(Mev[STEP].pad)] == 0) {
                Mev[STEP].rnum = (-1) * (brandom(5) + 1);
            }

            // Unmanned also
            if (MANNED[Mev[STEP].pad] == 0 && noDock == 0) {
                Mev[STEP].rnum = (-1) * (brandom(5) + 1);
            }

            memset(&Now, 0x00, sizeof Now);


            //***************TC Special little HMOON EVA FAILURE FIX

            tomflag = 0;

            if (Mev[STEP].Name[0] == 'H' && Mev[STEP].Name[1] == 'M') {

                gork = Data->P[plr].Mission[mpad].Prog;

                if (gork == 2) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "HUM3C1" : "HSM3C1");
                } else if (gork == 3) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "HUM3C2" : "HSM3C2");
                } else {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "HUM3C3" : "HSM3C3");
                }

                tomflag = 1;
            }



            //:::::: Failure docking kludge

            if (Mev[STEP].Name[0] == 'I') {
                gork = Data->P[plr].Mission[mpad].Prog;

                if (gork == 2) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C1" : "ISM4C1");
                } else if (gork == 3) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C2" : "ISM4C2");
                } else {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C3" : "ISM4C3");
                }
            }

// *********** TOM's FAIL HMOON KLUDGE

            // if HMOON FAILURE
            if (tomflag) {
                GetFailStat(&Now, Mev[STEP].FName, 7595);
            } else {
                DEBUG3("Failing !tomflag - calling GetFailStat(&Now, Mev[STEP].FName %s, MEV[STEP].rnum %d))", Mev[STEP].FName, Mev[STEP].rnum);
                GetFailStat(&Now, Mev[STEP].FName, Mev[STEP].rnum);     // all others
            }


            memset(Name, 0x00, sizeof Name);
            memcpy(Name, Mev[STEP].Name, strlen(Mev[STEP].Name)); // copy orig failure
            Name[strlen(Name)] = 0x30 + (Now.fail / 10);
            Name[strlen(Name)] = 0x30 + Now.fail % 10;
            Name[strlen(Name)] = 0x00;




            //:::::: STEP FAILURE :::::::::
            //:::::::::::::::::::::::::::::

            PlaySequence(plr, STEP, Name, 1);

            if (!AI[plr]) {
                Tick(2);    // reset dials
            }

            FailEval(plr, Now.code, Now.text, Now.val, Now.xtra);
        } else {   // Step Success

            if (Mev[STEP].loc == 28 || Mev[STEP].loc == 27) {
                strcpy(Mev[STEP].Name, (plr == 0) ? "bUC0" : "bSC0");
                Mev[STEP].Name[5] = Mev[STEP].E->ID[1];
            }

            if (strncmp(Mev[STEP].E->Name, "DO", 2) == 0) {
                if (Mev[STEP].loc == 2) {
                    Data->P[plr].DockingModuleInOrbit = 2;
                }
            }

            //::::::::::::::::::::::::::::::::::
            //::: SUCCESS: Docking kludge ::::::
            //::::::::::::::::::::::::::::::::::
            if (Mev[STEP].Name[0] == 'I') {
                gork = Data->P[plr].Mission[mpad].Prog;

                if (gork == 2) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C1" : "ISM4C1");
                } else if (gork == 3) {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C2" : "ISM4C2");
                } else {
                    strcpy(Mev[STEP].Name, (plr == 0) ? "IUM4C3" : "ISM4C3");
                }
            }

            if (Mev[STEP].Name[0] == 'e' && Data->P[plr].Mission[mpad].MissionCode == Mission_LunarFlyby) {
                strcpy(Mev[STEP].Name, (plr == 0) ? "VUP1" : "VSP1");
            }

            //:::::: STEP SUCCESS :::::::::
            //:::::::::::::::::::::::::::::
            // FemaleAstronautsAllowed step kludge
            // third parameter (0 -> MALE) (2 -> FEMALE)
            //:::::::::::::::::::::::::::::

            gork = ((MA[0][0].A != NULL && MA[0][0].A->Sex && EVA[0] == 0)
                    || (MA[0][1].A != NULL && MA[0][1].A->Sex && EVA[0] == 1)
                    || (MA[0][2].A != NULL && MA[0][2].A->Sex && EVA[0] == 2)
                    || (MA[0][3].A != NULL && MA[0][3].A->Sex && EVA[0] == 3)
                    || (MA[1][0].A != NULL && MA[1][0].A->Sex && EVA[1] == 0)
                    || (MA[1][1].A != NULL && MA[1][1].A->Sex && EVA[1] == 1)
                    || (MA[1][2].A != NULL && MA[1][2].A->Sex && EVA[1] == 2)
                    || (MA[1][3].A != NULL && MA[1][3].A->Sex && EVA[1] == 3));

            //if (!((mcc==9 || mcc==11) && (Mev[STEP].Name[0]=='W')))
            PlaySequence(plr, STEP, Mev[STEP].Name, (gork == 1) ? 2 : 0);  // Play Animations

            if (Mev[STEP].sgoto == 100) {
                Mev[STEP].trace = 0x7F;
            } else if (Mev[STEP].sgoto != 0) {
                Mev[STEP].trace = Mev[STEP].sgoto;
            } else {
                Mev[STEP].trace = STEP + 1;
            }

            if (!(strncmp(Mev[STEP].E->Name, "DO", 2) == 0 && Mev[STEP].loc == 0x02)) {
                Mev[STEP].E->MisSucc++;  // set for all but docking power on
            }

            Mev[STEP].StepInfo = 1;

            // Bottom of success statement
        }

        if (Mev[STEP].loc == 0x7f || Mev[STEP].sgoto == 100) {  // force mission end
            Mev[STEP].trace = 0x7f;
        }

        if ((mcc == Mission_MarsFlyby || mcc == Mission_JupiterFlyby ||
             mcc == Mission_SaturnFlyby) && STEP == 2) {
            Mev[STEP].trace = 0x7f;
        }

        if (STEP > 30 || STEP < 0) {
            delay(20);
        }

        if (Mev[STEP].sgoto == Mev[STEP].fgoto && Mev[STEP].trace != 0x7f) {
            Mev[STEP].trace = Mev[STEP].sgoto;
        }

        if (Mev[STEP].loc == 8 && Mev[STEP].StepInfo == 1) {
            noDock = 1;
        }

        if (Mev[STEP].loc == 0 && MANNED[Mev[STEP].pad] > 0) {
            InSpace++;
        }

        if (Mev[STEP].loc == 4 && MANNED[Mev[STEP].pad] > 0) {
            InSpace--;
        }

        // these two lines fix a problem with Lab Missions failing on launch and
        // ending the mission.
        // Yet another in the ongoing series of special cases.
        //

        if (Mev[STEP].trace == 0x7f && STEP == 3) {
            switch (Mis.Index) {
            case 19:
            case 22:
            case 23:
            case 30:
            case 32:
            case 35:
            case 36:
            case 37:
                InSpace = 1;
                Mev[STEP].trace = Mev[STEP].dgoto;

            default:
                break;
            }
        }

//  if (Mev[STEP].trace==0x7f && InSpace>0) Mev[STEP].trace=STEP+1;

    } while (Mev[STEP].trace != 0x7f);         // End mission

    //end do
    if (!AI[plr] && death == 0) {
        delay(1000);
    }

    if ((MA[0][0].A != NULL && MA[0][0].A->Status == AST_ST_DEAD)
        || (MA[0][1].A != NULL && MA[0][1].A->Status == AST_ST_DEAD)
        || (MA[0][2].A != NULL && MA[0][2].A->Status == AST_ST_DEAD)
        || (MA[0][3].A != NULL && MA[0][3].A->Status == AST_ST_DEAD)
        || (MA[1][0].A != NULL && MA[1][0].A->Status == AST_ST_DEAD)
        || (MA[1][1].A != NULL && MA[1][1].A->Status == AST_ST_DEAD)
        || (MA[1][2].A != NULL && MA[1][2].A->Status == AST_ST_DEAD)
        || (MA[1][3].A != NULL && MA[1][3].A->Status == AST_ST_DEAD)) {
        // Mission Death
        if (!AI[plr]) {
            if (!fullscreenMissionPlayback) {
                display::AutoPal p(display::graphics.legacyScreen());
                memset(&p.pal[64 * 3], 0x00, 64 * 3);  //Specs: 0x08

                if (plr == 0) {
                    fill_rectangle(2, 107, 140, 115, 3);
                } else {
                    fill_rectangle(78, 2, 241, 10, 3);
                }

                if (plr == 0) {
                    fill_rectangle(188, 107, 294, 113, 3);
                } else {
                    fill_rectangle(244, 56, 314, 62, 3);
                }
            }

            PlaySequence(plr, STEP, (plr == 0) ? "UFUN" : "SFUN", 0);
        }

        if (!AI[plr]) {
            delay(1000);
        }

        death = 1;
    } else {
        death = 0;
    }

    return;
}

/** Draw mission step rectangle
 *
 * The rectangle represents the success or failure rate.
 *
 * \param plr Player data
 * \param lc ??? maybe location of the chart
 * \param safety Safety factor in percent
 * \param val value of the dice checked against safety
 * \param prob is this a problem or not?
 *
 * \return new value of lc
 */
int MCGraph(char plr, int lc, int safety, int val, char prob)
{
    int i;
    TRACE5("->MCGraph(plr, lc %d, safety %d, val %d, prob %c)", lc, safety, val, prob);
    fill_rectangle(lc - 2, 195, lc, 195 - safety * 22 / 100, 11);
    fill_rectangle(lc - 2, 195, lc, 195 - (safety - Mev[STEP].asf) * 22 / 100, 6);

    for (i = 195; i > 195 - val * 22 / 100; i--) {
        fill_rectangle(lc - 2, 195, lc, i, 21);
        delay(15);
    }


    if (plr == 1 && !AI[plr]) {
        if (val > safety && prob == 0) {
            fill_rectangle(lc - 2, 195, lc, 195 - val * 22 / 100, 9);
            lc = 191;
        } else if (val > safety) {
            fill_rectangle(lc - 2, 195, lc, 195 - val * 22 / 100, 9);
            lc += 5;
        } else {
            if (lc >= 241) {
                display::graphics.setForegroundColor(55);
                fill_rectangle(189, 173, 249, 196, 55);

                for (i = 190; i < 250; i += 2) {
                    display::graphics.legacyScreen()->setPixel(i, 178, 61);
                    display::graphics.legacyScreen()->setPixel(i, 184, 61);
                    display::graphics.legacyScreen()->setPixel(i, 190, 61);
                }

                fill_rectangle(189, 195, 191, 195 - safety * 22 / 100, 11);
                fill_rectangle(189, 195, 191, 195 - (safety - Mev[STEP].asf) * 22 / 100, 6);
                fill_rectangle(189, 195, 191, 195 - val * 22 / 100, 21);

                if (Mev[STEP].asf > 0) {
                    fill_rectangle(189, 195 - safety * 22 / 100, 191, 195 - safety * 22 / 100, 11);
                }

                lc = 196;
                /* lc > 241 */
            } else {
                lc += 5;
            }
        } /* check safety and problem */
    } else if (plr == 0 && !AI[plr]) {
        if (val > safety && prob == 0) {
            fill_rectangle(lc - 2, 195, lc, 195 - val * 22 / 100, 9);
            lc = 76;
        } else if (val > safety) {
            fill_rectangle(lc - 2, 195, lc, 195 - val * 22 / 100, 9);
            lc += 5;
        } else {
            if (lc >= 126) {
                fill_rectangle(73, 173, 133, 196, 55);

                for (i = 73; i < 133; i += 2) {
                    display::graphics.legacyScreen()->setPixel(i, 178, 61);
                    display::graphics.legacyScreen()->setPixel(i, 184, 61);
                    display::graphics.legacyScreen()->setPixel(i, 190, 61);
                }

                fill_rectangle(74, 195, 76, 195 - safety * 22 / 100, 11);
                fill_rectangle(74, 195, 76, 195 - (safety - Mev[STEP].asf) * 22 / 100, 6);
                fill_rectangle(74, 195, 76, 195 - val * 22 / 100, 21);

                if (Mev[STEP].asf > 0) {
                    fill_rectangle(74, 195 - safety * 22 / 100, 76, 195 - safety * 22 / 100, 11);
                }

                lc = 81;
            } else {
                lc += 5;
            }
        }
    }

    TRACE1("<-MCGraph()");
    return lc;
}

#define F_ALL 0
#define F_ONE 1

void F_KillCrew(char mode, struct Astros *Victim)
{
    int k = 0, p = 0;
    struct Astros *Guy;

    Guy = NULL; /* XXX check uninitialized */

    // Reset Hardware
    if (Victim >= &Data->P[1].Pool[0]) {
        p = 1;
    }

    if ((Data->Def.Lev1 == 0 && p == 0) || (Data->Def.Lev2 == 0 && p == 1)) {
        Mev[STEP].E->Safety /= 2;
    } else {
        Mev[STEP].E->Safety = Mev[STEP].E->Base;
    }

    Mev[STEP].E->MaxRD = Mev[STEP].E->MSF - 1;

    if (mode == F_ALL) {
        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {  // should work in news
            Guy = MA[Mev[STEP].pad][k].A;

            if (Guy != NULL) {
                Guy->Status = AST_ST_DEAD;
                Guy->Special = 3;
                Guy->RetirementReason = 8;
                Guy->Assign = Guy->Moved = Guy->Crew = Guy->Task = Guy->Unassigned = 0;
                tMen++;
                death = 1;
            }
        }
    } else if (mode == F_ONE) {  // should work in news
        if (Victim == NULL) {
            return;
        }

        Victim->Status = AST_ST_DEAD;
        Victim->Special = 3;
        Victim->RetirementReason = 8;
        Victim->Assign = Victim->Moved = Victim->Crew = Victim->Task = 0;

        /* XXX this code was here, but Guy wsa not initialized */
        // Guy->Unassigned0;

        tMen++;
        //death=1;
    }
}


#define F_RET 0
#define F_INJ 1

void F_IRCrew(char mode, struct Astros *Guy)
{
    if (Guy == NULL || Guy->Status == AST_ST_DEAD) {
        return;
    }

    if (mode == F_RET) {  // should work in news
        Guy->Status = AST_ST_RETIRED;
        Guy->RetirementDelay = 1;  // Retire beginning of next season
        Guy->RetirementReason = 9;
        Guy->Assign = Guy->Moved = Guy->Crew = Guy->Task = Guy->Unassigned = 0;
    } else if (mode == F_INJ) {
        Guy->Status = AST_ST_INJURED;
        Guy->InjuryDelay = 3;  // Injured for a year
        Guy->Special = 4;
        Guy->Assign = Guy->Moved = Guy->Crew = Guy->Task = Guy->Unassigned = 0;
    }
}

int FailEval(char plr, int type, char *text, int val, int xtra)
{
    int FNote = 0, temp, k, ctr = 0;
    char PROBLEM = 0;
    struct Astros *crw;

    temp = 0; /* XXX check uninitialized */

    if (!(strncmp(Mev[STEP].E->Name, "DO", 2) == 0 && Mev[STEP].loc == 0x02)) {
        Mev[STEP].E->MisFail++;  // set failure for all but docking power on
    }

    Mev[STEP].StepInfo = 1003;
    FNote = 5;  // Mission Failure

    if (Unm == 0) {
        Mev[STEP].trace = 0x7f;

        if (type == 12) {
            DestroyPad(plr, MPad + Mev[STEP].pad, 20, 0);
        }

        if (!AI[plr]) {
            FailureMode(plr, FNote, text);
        }

        // Special Case for PhotoRecon with Lunar Probe
        if (Mev[STEP].loc == 20 && mcc == Mission_Lunar_Probe) {
            Mev[STEP - 1].E->MisFail++;
        }

        return 0;
    }

    Mev[STEP].StepInfo = 0;

    switch (type) {

    case 0:   // Failure has no effect on Mission
    case 20:   // don't want to test for crew experience
        FNote = 0;
        Mev[STEP].StepInfo = 50;

        if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;


    case 2:  // End of Mission Failure
        FNote = 5;
        Mev[STEP].StepInfo = 1000 + Mev[STEP].loc;
        Mev[STEP].trace = 0x7f;
        break;


    case 5:  // Stranded Step  (temp)
    case 3:  // Kill ALL Crew and END Mission
        FNote = 8;

        if (InSpace > 0 && MANNED[Mev[STEP].pad] == 0 && strncmp(Mev[STEP].E->ID, "M2", 2) == 0) {
            Mev[STEP].pad = other(Mev[STEP].pad);  // for Kicker-C problems
            F_KillCrew(F_ALL, 0);
            Mev[STEP].pad = other(Mev[STEP].pad);
        } else {
            F_KillCrew(F_ALL, 0);
        }

        Mev[STEP].StepInfo = 4600 + Mev[STEP].loc;
        Mev[STEP].trace = 0x7F;
        break;

    case 4:  // Branch to Alternate Step
        FNote = 1;
        Mev[STEP].StepInfo = 1900 + Mev[STEP].loc;

        if (Mev[STEP].fgoto == -1) {  // End of Mission Flag
            if (Mev[STEP].PComp > 0) {
                Mev[STEP].PComp = 4;
            }

            Mev[STEP].trace = 0x7F;  // End of Mission Signal
            FNote = 5;
        } else if (Mev[STEP].fgoto != -2) {  // Alternate Step is other num
            if (Mev[STEP].PComp > 0) {
                Mev[STEP].PComp = 4;
            }

            Mev[STEP].trace = Mev[STEP].fgoto;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 6:   // Reduce Safety by VAL% temp
        FNote = 0;
        Mev[STEP].E->MisSaf -= abs(val);

        if (Mev[STEP].E->MisSaf <= 0) {
            Mev[STEP].E->MisSaf = 1;
        }

        Mev[STEP].StepInfo = 900 + Mev[STEP].loc;

        if (Mev[STEP].fgoto == -1 && Unm == 0) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 7:   // Reduce Safety by VAL% perm
        FNote = 0;
        Mev[STEP].StepInfo = 1700 + Mev[STEP].loc;

        if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 9:  // Recheck Step
        FNote = 2;
        Mev[STEP].StepInfo = 9;
//         Mev[STEP].rnum=random(10000)+1;  // new failure roll
//         Mev[STEP].dice=random(100)+1;    // new die roll
        Mev[STEP].trace = STEP;          // redo step
        break;

    case 12:  // Subtract VAL% from Safety, repair Pad for XTRA (launch only)
        FNote = 5;
        Mev[STEP].E->MisSaf -= abs(val);

        if (Mev[STEP].E->MisSaf <= 0) {
            Mev[STEP].E->MisSaf = 1;
        }

        Mev[STEP].StepInfo = 1600 + Mev[STEP].loc;

        DestroyPad(plr, MPad + Mev[STEP].pad, abs(xtra), 0);  // Destroy Pad

        Mev[STEP].trace = 0x7F;  // signal end of mission
        break;

    case 13:  // Kill Crew, repair Pad for VAL
        FNote = 8;
        F_KillCrew(F_ALL, 0);
        DestroyPad(plr, Mev[STEP].pad + MPad, (val == 0) ? abs(xtra) : abs(val), 0);  // Destroy Pad
        Mev[STEP].StepInfo = 4500 + Mev[STEP].loc;
        Mev[STEP].trace = 0x7F;
        break;

    case 15:  // Give option to Scrub  1%->20% negative of part
        FNote = 3;
        Mev[STEP].E->MisSaf -= brandom(20) + 1;

        if (Mev[STEP].E->MisSaf <= 0) {
            Mev[STEP].E->MisSaf = 1;
        }

        Mev[STEP].StepInfo = 15;
        break;

    case 16:  // VAL% injury,   XTRA% death
        FNote = 0;
        Mev[STEP].StepInfo = 1100 + Mev[STEP].loc;

        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {
            if (brandom(100) >= val) {
                F_IRCrew(F_INJ, MA[Mev[STEP].pad][k].A);
                Mev[STEP].StepInfo = 2100 + Mev[STEP].loc;
                FNote = 9;
            }
        }  // for

        ctr = 0;

        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {
            if (Data->P[plr].Pool[temp].Status == AST_ST_RETIRED) {
                if (brandom(100) > xtra) {
                    F_KillCrew(F_ONE, MA[Mev[STEP].pad][k].A);
                    Mev[STEP].StepInfo = 3100 + Mev[STEP].loc;
                    FNote = 8;
                    ctr++;
                }
            }
        }

        if (ctr == MANNED[Mev[STEP].pad]) {
            Mev[STEP].StepInfo = 4100 + Mev[STEP].loc;
            Mev[STEP].trace = 0x7F;
        } else if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        if (Mev[STEP].FName[3] == 0x30) {
            Mev[STEP].trace = 0x7f;
        }

        break;



    case 17:  // VAL% survival and XTRA% if injury and retirement
        Mev[STEP].StepInfo = 1300 + Mev[STEP].loc;

        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {
            if (brandom(100) >= xtra) {
                F_IRCrew(F_RET, MA[Mev[STEP].pad][k].A);
                Mev[STEP].StepInfo = 2300 + Mev[STEP].loc;
                FNote = 9;
            }
        }

        ctr = 0;

        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {
            if (brandom(100) >= val) {
                F_KillCrew(F_ONE, MA[Mev[STEP].pad][k].A);
                Mev[STEP].StepInfo = 3300 + Mev[STEP].loc;
                FNote = 8;
                ctr++;
            }
        }  // for

        if (ctr == MANNED[Mev[STEP].pad]) {
            Mev[STEP].StepInfo = 4100 + Mev[STEP].loc;
            Mev[STEP].trace = 0x7F;
        } else if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 18:    // set MFlag from VAL, branch if already set
        if ((MFlag & val) > 0) {
            FNote = 1;
            Mev[STEP].StepInfo = 1800 + Mev[STEP].loc;

            if (Mev[STEP].fgoto == -1) {
                Mev[STEP].trace = 0x7F;
                FNote = 0;
            } else if (Mev[STEP].fgoto != -2) {
                Mev[STEP].trace = Mev[STEP].fgoto;
            } else {
                Mev[STEP].trace = STEP + 1;
            }
        } else {
            FNote = 0;
            Mev[STEP].StepInfo = 18;
            MFlag = MFlag | val;

            if (Mev[STEP].fgoto == -1) {
                Mev[STEP].trace = 0x7F;
            } else {
                Mev[STEP].trace = STEP + 1;
            }
        }

        break;

    case 19:   // Set mission flag and recheck step
        if ((MFlag & val) > 0) {
            Mev[STEP].StepInfo = 1200 + Mev[STEP].loc;
            FNote = 2;
        } else {
            FNote = 2;
            Mev[STEP].StepInfo = 19;
            MFlag = MFlag | val;
        }

        Mev[STEP].trace = STEP;          // recheck step

//         Mev[STEP].rnum=random(10000)+1;  // new failure roll
//         Mev[STEP].dice=random(100)+1;    // new die roll
        break;


    case 22:  // one man % survival :: EVA
        Mev[STEP].StepInfo = 19;

        if (brandom(100) > val) {
            FNote = 8;
            crw = (EVA[Mev[STEP].pad] != -1) ? MA[Mev[STEP].pad][EVA[Mev[STEP].pad]].A : MA[other(Mev[STEP].pad)][EVA[other(Mev[STEP].pad)]].A;
            F_KillCrew(F_ONE, crw);

            if (Mev[STEP].Name[6] == 0x36) {
                death = 1;    // one-man lem
            }

            Mev[STEP].StepInfo = 3200 + Mev[STEP].loc;
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 23:  // VAL% retirement, hardware cut %XTRA perm
        FNote = 0;
        Mev[STEP].StepInfo = 23 + Mev[STEP].loc;

        for (k = 0; k < MANNED[Mev[STEP].pad]; k++) {
            if (brandom(100) >= val) {
                FNote = 9;
                F_IRCrew(F_RET, MA[Mev[STEP].pad][k].A);
                Mev[STEP].StepInfo = 2400 + Mev[STEP].loc;
                ctr++;
            }
        }

        // Used to reduce safety

        if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else if (Mev[STEP].fgoto != -2) {
            Mev[STEP].trace = Mev[STEP].fgoto;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;

    case 24:   // Reduce Safety by VAL% perm :: hardware recovered
        FNote = 5;
        Mev[STEP].E->Safety -= brandom(10);

        if (Mev[STEP].E->Safety <= 0) {
            Mev[STEP].E->Safety = 1;
        }

        Mev[STEP].StepInfo = 800 + Mev[STEP].loc;
        Mev[STEP].trace = 0x7F;

        break;

    case 25:    // Mission Failure recover Minishuttle
        FNote = 5;
        Mev[STEP].StepInfo = 700;
        Mev[STEP].trace = 0x7F;  // End of Mission
        break;

    case 26:  // Subtract VAL% from Equip perm and branch to alternate
        FNote = 1;
        Mev[STEP].StepInfo = 1926;
        Mev[STEP].E->Safety -= brandom(10);

        if (Mev[STEP].E->Safety <= 0) {
            Mev[STEP].E->Safety = 1;
        }

        if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
            FNote = 7;
        } else if (Mev[STEP].fgoto != -2) {
            Mev[STEP].trace = Mev[STEP].fgoto;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;


    case 30:  // Duration Failure
        Data->P[plr].Mission[MPad + Mev[STEP].pad].Duration = 1; //Original code would also return 1
        durx = -1;  // end durations
        FNote = 7;
        Mev[STEP].StepInfo = 1950;
        Mev[STEP].trace = STEP + 1;

        break;

    case 31:  // kill EVA and LM people and branch VAL steps
        //Dock_Skip=1;
        Mev[STEP].trace = Mev[STEP].dgoto;
        Mev[STEP].StepInfo = 3100 + STEP;

        if (strncmp(Mev[STEP].E->ID, "C4", 2) == 0) {
            F_KillCrew(F_ALL, 0);
            Mev[STEP].StepInfo = 4100 + STEP;
        } else {
            if (JOINT == 0) {
                F_KillCrew(F_ONE, MA[0][EVA[0]].A);

                if (LM[0] != EVA[0]) {
                    F_KillCrew(F_ONE, MA[0][LM[0]].A);
                }
            } else {
                F_KillCrew(F_ONE, MA[1][EVA[1]].A);

                if (LM[1] != EVA[1]) {
                    F_KillCrew(F_ONE, MA[1][LM[1]].A);
                }
            }
        }

        death = 1;
        break;

    case 33:  // Kill Crew on All Capsules (happens only on dockings)
        FNote = 8;

        if (MANNED[Mev[STEP].pad] > 0) {
            F_KillCrew(F_ALL, 0);
        }

        if (MANNED[other(Mev[STEP].pad)] > 0)  {
            Mev[STEP].pad = other(Mev[STEP].pad);  // switch pad for a sec
            F_KillCrew(F_ALL, 0);
            Mev[STEP].pad = other(Mev[STEP].pad);  // restore current pad
        }

        Mev[STEP].StepInfo = 4600 + Mev[STEP].loc;
        Mev[STEP].trace = 0x7F;
        break;

    case 1:
    case 8:
    case 10:
    case 11:
    case 14:
    case 21:
    case 27:
    case 28:
    case 29:
    default:
        FNote = 0;
        Mev[STEP].StepInfo = 50;

        if (Mev[STEP].fgoto == -1) {
            Mev[STEP].trace = 0x7F;
        } else {
            Mev[STEP].trace = STEP + 1;
        }

        break;  // nothing : continue steps
    }

    if ((Mev[STEP].Name[0] == 'A') &&
        MH[Mev[STEP].pad][Mission_SecondaryBooster] != NULL) {
        // boosters involved
        if (MH[Mev[STEP].pad][Mission_PrimaryBooster]->Safety ==
            MH[Mev[STEP].pad][Mission_PrimaryBooster]->Base) {
            MH[Mev[STEP].pad][Mission_SecondaryBooster]->Safety =
                MH[Mev[STEP].pad][Mission_SecondaryBooster]->Base;
        }
    }

    VerifySF(plr);  // Keep all safeties within the proper ranges

    // check for all astros that are dead.  End mission if this is the case.
    while (bioskey(1)) {
        bioskey(0);
    }

    key = 0;

    if (!AI[plr]) {
        temp = FailureMode(plr, FNote, text);
    }

    if (temp == 0 && FNote == 3) {
        Mev[STEP].trace = STEP + 1;
    } else if (FNote == 3) {
        Mev[STEP].StepInfo += 1000;
        SCRUBS = 1;
    }

    if (SCRUBS == 1) {
        if (Mev[STEP].loc == 8 && noDock == 1) {
            PROBLEM = 1;
        }

        if (PROBLEM == 0) {
            if (Mev[STEP].fgoto == -1) {  // End of Mission Flag
                if (Mev[STEP].PComp > 0) {
                    Mev[STEP].PComp = 4;
                }

                Mev[STEP].trace = 0x7F;  // End of Mission Signal
                FNote = 5;
            } else if (Mev[STEP].fgoto != -2) {  // Alternate Step is other num
                if (Mev[STEP].PComp > 0) {
                    Mev[STEP].PComp = 4;
                }

                Mev[STEP].trace = Mev[STEP].fgoto;
            } else {
                Mev[STEP].trace = STEP + 1;
            }
        }
    }

    if (strncmp(Mev[STEP].E->ID, "M3", 2) == 0) {
        death = 0;    //what???
    }

    // New death branching code
    if (death == 1) {
        if (Mev[STEP].dgoto == 0) {
            Mev[STEP].trace = 0x7f;
        } else if (Mev[STEP].dgoto > 0) {
            Mev[STEP].trace = Mev[STEP].dgoto;
        }
    }

    if (type == 9 || type == 19) {
        Mev[STEP].trace = STEP;
        Mev[STEP].rnum = brandom(10000) + 1;  // new failure roll
        Mev[STEP].dice = brandom(100) + 1;  // new die roll
    }

    death = 0;
    return FNote;
}
