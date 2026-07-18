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

// This file executes the results of the newscast (event cards, etc.)

#include "news_suq.h"

#include <algorithm>

#include "astros.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "mission_util.h"
#include "news.h"
#include "news_sup.h"
#include "options.h"
#include "pace.h"
#include "place.h"
#include "state_utils.h"


// Roll a six die
int rollSixDie(int nrolls) {
    int result = 0;
    for (int i = 0; i < nrolls; i++) {
        result += brandom(6) + 1;
    }
    return result;
}

/**
 *
 * \return true (1) if need to scrub mission because of crews
 */
char CheckCrewOK(char plr, char pad)
{
    BuzzData& pData = Data->P[plr];
    auto& mission = pData.Mission[pad];

    int mis = mission.MissionCode;
    if (mis == Mission_None) return 0; // no mission - no problem
    
    int prog = mission.Prog;
    int P_crew = mission.PCrew - 1;
    if (pData.CrewCount[prog][P_crew] != 0) return 0; // primary crew is fine
    
    int B_crew = mission.BCrew - 1;
    if (pData.CrewCount[prog][P_crew] != 0) return 0; // backup crew is fine

    if (mission.Men <= 0) return 0; // don't care about unmanned missions(?)

    if (AI[plr]) return 0; // ???
    
    switch (pad) {              // THE PRIMARY AND SECONDARY CREWS
    case 0:                     // OF THE MISSION ON PAD [] HAVE BEEN
        Help("i138");           // REMOVED FOR ONE REASON OR ANOTHER.
        break;                  // THE MISSION IS SCRUBBED

    case 1:
        Help("i139");
        break;

    case 2:
        Help("i140");
        break;
    }

    return 1;
}


char REvent(char plr)
{
    int NoMods = 1, i = 0, j = 0;

    BuzzData& pData = Data->P[plr];
                
    if (pData.Budget < 40) {
        pData.Budget = 40;
    }

    j = 5; // for Budget over 160
    int range[6] = {0, 50, 90, 110, 140, 160}; // Budget ranges
        
    for (int k = 0; k < 5; k++) {
         if (pData.Budget > range[k] && pData.Budget <= range[k+1]) {
            j = k;
         }
    }

    std::vector<int> nTable; // vector for NTABLE
    DESERIALIZE_JSON_FILE(&nTable, locate_file("ntable.json", FT_DATA));
    //int index =  (plr * 60) + (j * 10) + brandom(10);
    int index =  (j * 10) + brandom(10); // The first table is used for both players
    // TODO; Change the budget table (+60 / +120) according to difficulty
    if (index >= 0 && index < nTable.size()){
        pData.Budget += nTable[index];
    } else {
        std::cerr << "Index out of ntable range: " << index << std::endl;
        return 1;
    }

    if (Data->Year <= 60 && Data->Events[Data->Count] > 44) {
        return 1;
    }

    if (Data->Year <= 62 && Data->Events[Data->Count] > 63) {
        return 1;
    }

    if (Data->Year <= 65 && Data->Events[Data->Count] > 80) {
        return 1;
    }
    
    // evflag defined in news.h
    switch (Data->Events[Data->Count]) {
    case  0: case  1: case 44: case 59: case 63:  // Nothing Events
    case 67: case 68: case 69: case 70: case 71:
    case 72: case 73: case 74: case 75: case 81:
    case 86: case 87: case 88: case 90: case 97:
    case 98:
        break;

    // Cash Modifier Events --------------------------

    case 30: case 32:
        evflag = brandom(10) + 1;
        pData.Cash += evflag;
        break;

    case 31:
        evflag = rollSixDie(3);
        pData.Cash += evflag;
        break;

    case 33:
        if (pData.Cash < 10) {
            return 1;
        }

        evflag = pData.Cash / 10;
        pData.Cash -= evflag;
        break;

    // Budget Modifier Events ------------------------

    case  9: case 76:  // Budget increase of 15 MB
        pData.Budget += 15;
        break;

    case 45:  // increase budget by total two 10-sided dice this year
        evflag = brandom(10) + brandom(10) + 2;
        pData.Budget += evflag;
        break;

    case 46:  // increase budget by 5 MB's
        pData.Budget += 5;
        break;

    case 91:  // increase budget by 20MB's in the budget
        pData.Budget += 20;
        break;

    case 92:  // 20% or 20MB's whichever is less
        evflag = std::min(pData.Budget / 5, 20);
        pData.Budget -= evflag;
        break;

    case 95:  // 30% or 30MB's whichever is less
        pData.Budget -= std::min(pData.Budget * 3 / 10, 30);
        evflag = pData.Budget;
        break;

    case 96:  // 25% or 25MB's whichever is less
        if (Data->Year > 72) {
            return 1;
        }

        pData.Budget -= std::min(pData.Budget / 4, 25);
        evflag = pData.Budget;
        break;

    case 99:  // deduct 5 MB's from budget
        pData.Budget -= 5;
        break;

    // RD Modifier Events ----------------------------

    case  2: case 12: case 13: case 14:  // RD improve by 1: season
    case 15: case 16: case 17: case 18:
        pData.RD_Mods_For_Turn += 1;
        break;

    case  3: case 61: case 62:  // Minus one on all R&D for ONE year
        pData.RD_Mods_For_Turn -= 1;
        pData.RD_Mods_For_Year = -1;
        break;

    case 19:  // minus 2 on each die roll for R&D this turn
        pData.RD_Mods_For_Turn -= 2;
        break;

    case 41:  // R&D will decrease for one year
        pData.RD_Mods_For_Turn -= 2;
        pData.RD_Mods_For_Year -= 2;
        break;

    case 42: case 43:  /* increment R&D cost by 1 on most advanced program */
        evflag = RDMods(plr, 0, 1, 1);

        if (evflag == 0) {
            return 1;
        }
        break;


    // Improve/Subtract Tech to Other players -------------------

    case 5: case 47: // Improve tech of plr prog from rival prog
        evflag = Steal(plr, 0, 1);

        if (!evflag) return 1;
        break;
 
    case 6: case 7: // Lower tech of plr prog from rival prog
        evflag = Steal(plr, 0, -1);

        if (!evflag) return 1;
        break;
    
    
    // Special Events -------------------------------------------

    case  8:  // Allow females into space program
        pData.FemaleAstronautsAllowed = 1;
        break;

    // Launch Affected ------------------------------------------

    case 4:  // Next Launch must be a manned launch.  If the launch
        // is not then -10 on Budget
        //pData.Plans=1;
        break;

    case 10: {  /* Launch window for Launch #x is blocked */
        int choice[3] = {0, 0, 0};

        evflag = 0;

        for (int i = 0; i < 3; i++) {
            if(pData.Mission[i].MissionCode) {
                choice[evflag++] += i + 1;
            }
        }

        if (!evflag) return 1;

        evflag = choice[brandom(evflag)] - 1;
        xMODE |= xMODE_CLOUDS;
        ScrubMission(plr, evflag);
    }
    break;

    case 35:  // may launch only one mission this year ??? Season?!?
        evflag = 0;

        if (pData.Mission[0].Joint == 1 ||
            pData.Mission[1].Joint == 1) {
            return 1;
        }

        for (int i = 0; i < 3; i++) {
            if(pData.Mission[i].MissionCode) {
                evflag += i * 2;
            }
        }

        if (!evflag) return 1;

        switch (evflag) {
        case 1:
        case 2:
        case 4:
            evflag = 0;
            break;

        case 3:
            ScrubMission(plr, 1);
            break;

        case 5:
        case 6:
            ScrubMission(plr, 2);
            break;

        case 7:
            ScrubMission(plr, 1);
            ScrubMission(plr, 2);
            break;
        }

        if (evflag == 0) {
            return 1;
        }
        break;

    case 36:  /* damage launch facility */
        if (pData.Mission[0].MissionCode == Mission_None) {
            pData.LaunchFacility[0] = 15;
        } else {
            return 1;
        }
        break;

    // Program Saving cards ------------------------------------

    case 11:  // Set Safety Card for most Advanced program
        evflag = SaveMods(plr, 0);

        if (!evflag) return 1;
        break;

    case 48:  // Set Safety Card for advanced Rocket program
        evflag = SaveMods(plr, 2);

        if (!evflag) return 1;
        break;

    case 77:  // set Safety Card for advanced Capsule Program
        evflag = SaveMods(plr, 3);

        if (!evflag) return 1;
        break;

    case 93:  // set Safety Card for advanced LEM Program
        evflag = SaveMods(plr, 4);

        if (!evflag) return 1;
        break;

    // Drop Safety  ------------------------------------

    case 20:  /* the most advanced rocket program is affected.
                 drop the safety factor in half. */
        evflag = 0;

        for (i = 3; i >= 0; i--) {
            if (pData.Rocket[i].Num >= 0 && pData.Rocket[i].Safety > 50) {
                evflag++;
            }
        }

        if (!evflag) return 1;

        for (i = 3; i >= 0; i--) {
            if (pData.Rocket[i].Num >= 0 && pData.Rocket[i].Safety > 50) {
                evflag = pData.Rocket[i].Safety / 2;
                pData.Rocket[i].Safety -= evflag;
                strcpy(&Name[0], &pData.Rocket[i].Name[0]);
                break;
            }
        }
        break;

    // Increase Safety   ------------------------------------

    case 21:  // Probes' and Sats' Max SF and RD up 5% */
        for (i = 0; i < 3; i++) {
            pData.Probe[i].MaxRD += 5;
            pData.Probe[i].MaxSafety += 5;
        }

        for (i = 0; i < 3; i++) {
            if (pData.Probe[i].MaxSafety >= 99) {
                pData.Probe[i].MaxSafety = 99;
            }

            if (pData.Probe[i].MaxRD > pData.Probe[i].MaxSafety) {
                pData.Probe[i].MaxRD = pData.Probe[i].MaxSafety;
            }
        }
        break;

    case 22: case 84: case 94:  /*  this applies for the most advanced capsule program. 
                                    roll four 6-sided dice and add to current safety factor. */
        x = rollSixDie(4);
        evflag = NMod(plr, 3, 1, x);

         if (!evflag) return 1;

        evflag = x;
        break;    

    case 23:  /* this applies to the most advanced rocket program.
                 roll six 6-sided dice and add to current safety factor. */
        x = rollSixDie(6);
        evflag = NMod(plr, 2, 1, x);

        if (!evflag) return 1;

        evflag = x;
        break;

    case 24:  /*  this applies for the most advanced satellite program. 
                  roll four 6-sided dice and add to current safety factor. */
        x = rollSixDie(4);
        evflag = NMod(plr, 1, 1, x);

        if (!evflag) return 1;

        evflag = x;
        break;

    case 26:  /* select most advanced capsule program and reduce safety by 25%  */
        evflag = NMod(plr, 3, -1, 25);

        if (!evflag) return 1;

        evflag = 25;
        break;

    case 27:  /* select most advanced probe program and reduce safety by 15%  */
        evflag = NMod(plr, 1, -1, 15);

        if (!evflag) return 1;

        evflag = 15;
        break;

    case 34:  /* 20% loss most advanced capsule program */
        evflag = NMod(plr, 3, -1, 20);

        if (!evflag) return 1;

        evflag = 20;
        break;

    case 79:  /* select most advanced program and reduce safety by 20%  */
        evflag = NMod(plr, 0, -1, 20);

        if (!evflag) return 1;

        evflag = 20;
        break;

    case 28: case 56:  // hardware cost 50% less this season
        pData.TurnOnly = 3;
        break;

    case 29:  // set the delay of the rockets to 2 == one year
        for (i = 0; i < 5; i++) {
            pData.Rocket[i].Delay = 2;
        }
        break;

    // Cost to Repair or Safety Loss   ------------------------------------

    case 25:  /* cost 15MB repair or 20% safety loss on the most advanced rocket program*/
        evflag = DamMod(plr, 2, -20, 15);

        if (!evflag) return 1;
        break;

    case 37:  /* cost 10MB repair or 10% safety loss on the most advanced probe program*/
        evflag = DamMod(plr, 1, -10, 10);

        if (!evflag) return 1;
        break;

    case 38: case 39:  /* cost 10MB repair or 5% safety loss on the most advanced rocket program*/
        evflag = DamMod(plr, 2, -5, 10);

        if (!evflag) return 1;
        break;

    case 40:  /* cost 10MB repair or 10% safety loss on the most advanced program*/
        evflag = DamMod(plr, 0, -10, 10);

        if (!evflag) return 1;
        break;

    case 54:  /* cost 15 MB repair or 20% safety loss on the most advanced rocket program*/
        evflag = DamMod(plr, 2, -20, 15);

        if (!evflag) return 1;
        break;

    case 55:  /* cost 20 MB repair or 10% safety loss on the most advanced capsule program*/
        evflag = DamMod(plr, 3, -10, 20);

        if (!evflag) return 1;
        break;

    case 78:  /* cost 10MB repair or 10% safety loss on the most advanced capsule program*/
        evflag = DamMod(plr, 3, -10, 10);

        if (!evflag) return 1;
        break;

    // Astronaut Portion ------------------------------------

    case 49:  /* pick random astronaut retire, budget minus 5 MB's */
        evflag = 0;

        for (i = 0; i < pData.AstroCount; i++) {
            if (!(pData.Pool[i].Status == AST_ST_DEAD || 
            pData.Pool[i].Status == AST_ST_RETIRED)) {
                evflag++;
            }
        }

        if (evflag) {
          i = brandom(pData.AstroCount);
          pData.Budget -= 5;
        } else {
          return 1;
        }

        while (pData.Pool[i].Status == AST_ST_DEAD || 
        pData.Pool[i].Status == AST_ST_RETIRED) {
            i = brandom(pData.AstroCount);
        }

        pData.Pool[i].RetirementDelay = 2;
        pData.Pool[i].Mood = 10;
        strcpy(&Name[0], &pData.Pool[i].Name[0]);

        if (plr == 1) {
            pData.Pool[i].Status = AST_ST_RETIRED;
            pData.Pool[i].RetirementDelay = 0;
        }

        if (pData.Pool[i].Status == AST_ST_RETIRED) {
            CheckFlightCrews(plr);
        }
        break;

    case 50: case 85:  // backup crew will fly mission
        evflag = 0;

        // Event text specifies the "Primary Crew", so check for a
        // Primary crew rather than manned.
        for (i = 0; i < MAX_MISSIONS; i++) {
            if (pData.Mission[i].MissionCode != Mission_None &&
                pData.Mission[i].PCrew > 0) {
                evflag = i + 1;
                break;
            }
        }

        if (!evflag) return 1;

        ClearMissionCrew(plr, --evflag, CREW_PRIMARY);
        break;

    case 51:  /* astronaut killed delay all manned missons = 1 */
        evflag = 0;

        for (i = 0; i < pData.AstroCount; i++) {
            if (!(pData.Pool[i].Status == AST_ST_DEAD || 
            pData.Pool[i].Status == AST_ST_RETIRED))  {
                evflag++;
            }
        }

        if (!evflag) return 1;

        pData.Budget -= 5;
        i = brandom(pData.AstroCount);

        while (pData.Pool[i].Status == AST_ST_DEAD || 
        pData.Pool[i].Status == AST_ST_RETIRED) {
            i = brandom(pData.AstroCount);
        }

        strcpy(&Name[0], &pData.Pool[i].Name[0]);
        pData.Pool[i].Status = AST_ST_DEAD;
        pData.MissionCatastrophicFailureOnTurn = 2;
        xMODE |= xMODE_SPOT_ANIM;  //trigger spot anim
        CheckFlightCrews(plr);

        //cancel manned missions
        for (size_t pad = 0; pad < MAX_LAUNCHPADS; pad++) {
            if (pData.Mission[pad].Men > 0) {
                ScrubMission(plr, pad);
            }
        }
        break;

    case 52:
        evflag = 0;

        for (i = 0; i < pData.AstroCount; i++) {
            if (!(pData.Pool[i].Status == AST_ST_DEAD || 
            pData.Pool[i].Status == AST_ST_RETIRED))  {
                evflag++;
            }
        }

        if (!evflag) return 1;

        i = brandom(pData.AstroCount);

        while (pData.Pool[i].Status == AST_ST_DEAD || 
        pData.Pool[i].Status == AST_ST_RETIRED) {
            i = brandom(pData.AstroCount);
        }

        strcpy(&Name[0], &pData.Pool[i].Name[0]);
        pData.Pool[i].Status = AST_ST_DEAD;
        pData.MissionCatastrophicFailureOnTurn = 2;
        CheckFlightCrews(plr);
        break;

    case 60: case 53:
        evflag = 0;

        for (i = 0; i < pData.AstroCount; i++) {
            if (!(pData.Pool[i].Status == AST_ST_DEAD || 
            pData.Pool[i].Status == AST_ST_RETIRED))  {
                evflag++;
            }
        }

        if (!evflag) return 1;

        i = brandom(pData.AstroCount);

        while (pData.Pool[i].Status == AST_ST_DEAD || 
        pData.Pool[i].Status == AST_ST_RETIRED) {
            i = brandom(pData.AstroCount);
        }

        strcpy(&Name[0], &pData.Pool[i].Name[0]);
        pData.Pool[i].Status = AST_ST_INJURED;
        pData.Pool[i].InjuryDelay = 2;

        if (options.feat_use_endurance && (brandom(100) < (pData.Pool[i].Endurance - 1) * 25)) {
            pData.Pool[i].InjuryDelay = 1;  // High endurance can shorten time in hospital to half a year
        }

        if (options.feat_use_endurance && pData.Pool[i].Endurance < 1 && brandom(100) < 25) {
            pData.Pool[i].InjuryDelay = 3;  // An Endurance of 0 can mean 3 turns in the hospital instead
        }

        pData.Pool[i].Special = 4;
        CheckFlightCrews(plr);
        break;

    case 57:  /* transfer new nauts to space program */
        NewNauts(plr);
        break;

    case 58:  // 50% chance explosion on pad
        pData.TurnOnly = 5;
        break;

    case 64:  /* launch facility repair 10MB's */
        for (j = 0; j < 20; j++) {
            i = rand() % 3;

            if (pData.LaunchFacility[i] == LAUNCHPAD_OPERATIONAL 
            && pData.Mission[i].MissionCode == Mission_None) {
                break;
            }
        }

        if (j == 20) return 1;

        /* get which of the three facilities damaged */
        pData.LaunchFacility[i] = 10;
        break;

    case 65:  // Gemini or Voskhod will cost additional 2MB's per spacecraft
        if (pData.Manned[MANNED_HW_TWO_MAN_CAPSULE].Num < 0) {
            return 1;
        }

        pData.Manned[MANNED_HW_TWO_MAN_CAPSULE].UnitCost += 2;  // Used to say "InitCost", which effectively disabled this newscast -Leon
        break;

    case 83:  // Apollo or Soyuz will cost additional 3MB's per spacecraft
        if (pData.Manned[MANNED_HW_THREE_MAN_CAPSULE].Num < 0) return 1;

        pData.Manned[MANNED_HW_THREE_MAN_CAPSULE].UnitCost += 3;
        break;

    case 66:  // mission delay
        evflag = 0;

        for (i = 0; i < 3; i++) {
            if (pData.Mission[i].MissionCode) {

                if (!IsJoint(pData.Mission[i].MissionCode) &&
                    MissionTimingOk(pData.Mission[i].MissionCode,
                                    Data->Year, Data->Season)) {
                    evflag++;
                }
            }
        }

        if (!evflag) return 1;

        i = brandom(3);

        while (pData.Mission[i].MissionCode == Mission_None ||
               pData.Mission[i].Joint == 1 ||
               ! MissionTimingOk(pData.Mission[i].MissionCode,
                                 Data->Year, Data->Season)) {
            i = brandom(3);
        }

        evflag = i;
        DelayMission(plr, i);
        break;

    case 80:  // Can't Deliver any Cap/Mods this year
        for (i = 0; i < 5; i++) {
            pData.Manned[i].Delay = 2;
        }
        break;

    case 82:  // Duration E Mission Required
        pData.FuturePlans = 5;
        break;

    case 89:  // random active astronaut leaves program
        evflag = 0;

        for (i = 0; i < pData.AstroCount; i++) {
            if (pData.Pool[i].Status == AST_ST_ACTIVE) {
                evflag++;
            }
        }

        if (!evflag) return 1;

        i = brandom(pData.AstroCount);

        while (pData.Pool[i].Status != AST_ST_ACTIVE) {
            i = brandom(pData.AstroCount);
        }

        pData.Pool[i].RetirementDelay = 2;
        strcpy(&Name[0], &pData.Pool[i].Name[0]);

        if (plr == 1) {
            pData.Pool[i].Status = AST_ST_RETIRED;
            pData.Pool[i].RetirementDelay = 0;
        }

        if (pData.Pool[i].Status == AST_ST_RETIRED) {
            CheckFlightCrews(plr);
        }
        break;

    default:
        break;
    }  // End of Switch

    return 0;
}
