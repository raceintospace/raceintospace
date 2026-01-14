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

// This file handles start-of-turn events, including adjusting 'naut morale.

#include "start.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>

#include "astros.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "options.h"
#include "pace.h"
#include "prest.h"


void AssignMissionName(int plr, int pad);
int  AstroConflictsMod(int player, Astros& astro);
void AstroTurn();
int  CrewConflicts(int player, const Astros& astro);
void TestFMis(int plr, int i);
void UpdateFlybys();
void UpdateHardTurn(int plr);


#define EVENT_PERIODS 6
#define EVENT_PERIOD_POOL 5

/**
 * Initialize the event cards.
 */
void InitializeEvents()
{
    std::vector<std::vector<int>> event_cards(EVENT_PERIOD_POOL); // initialize and fill in Event pools
    event_cards[0].resize(3);  std::iota(event_cards[0].begin(), event_cards[0].end(), 1);
    event_cards[1].resize(40); std::iota(event_cards[1].begin(), event_cards[1].end(), 4);
    event_cards[2].resize(19); std::iota(event_cards[2].begin(), event_cards[2].end(), 44);
    event_cards[3].resize(17); std::iota(event_cards[3].begin(), event_cards[3].end(), 63);
    event_cards[4].resize(20); std::iota(event_cards[4].begin(), event_cards[4].end(), 80);
    
    int cardCount = 2;  // Starting event card index
    char quantityInSet[EVENT_PERIODS] = {2, 8, 8, 12, 16, 52};  // Number of events in each period
    char poolPercentages[EVENT_PERIODS][EVENT_PERIOD_POOL] = {
        {100, 100, 100, 100, 100},
        {  0, 100, 100, 100, 100},
        {  0,  62, 100, 100, 100},
        {  0,  50,  70, 100, 100},
        {  0,  35,  55,  75, 100},
        {  0,  25,  50,  75, 100}
    };

    // Initialize base event data to 0's
    for (int i = 0; i < MAXIMUM_NEWS_EVENTS; i++) {
        Data->Events[i] = 0;
    }

    for (int period = 0; period < EVENT_PERIODS; period++) {
        auto& pool_distribution = poolPercentages[period];
        for (int set = 0; set < quantityInSet[period]; set++) {
            int pool_idx; // TODO: take this out into random utility function
            do{
                // Randomly pick which pool to take event from
                // by choosing number [0;100) and finding first pool whose "inclusion zone" is bigger
                int randomPoolChoice = brandom(100);
                auto iter = std::find_if(std::begin(pool_distribution), std::end(pool_distribution), [=](char c){return c > randomPoolChoice;});
                pool_idx = std::distance(std::begin(pool_distribution), iter);
            // If the pool we chose has been exhausted, choose again
            }while(event_cards[pool_idx].empty());
            auto& chosen_pool = event_cards[pool_idx];

            // choose random event from the pool
            int card_idx = brandom(chosen_pool.size());
            // add it to Data->Events
            Data->Events[cardCount++] = chosen_pool[card_idx];
            // and remove from the pool
            chosen_pool.erase(chosen_pool.begin()+card_idx);
        }
    }

    // Set defaults for Prestige
    for (int i = 0; i < MAXIMUM_PRESTIGE_NUM; i++) {
        Data->Prestige[i].Place = Data->Prestige[i].mPlace = -1;
    }

    // Initialize in memory for Event Saves
    interimData.tempEvents.fill("");

    // Empty previous file name suggestions
    interimData.filename.clear();
}


/**
 * Handle astronaut mood and training
 */
static void updateAstronautSkills(unsigned plr, Astros& astro)
{
    /* Constants related to training */
    const unsigned NUM_SKILLS = 5;
    int8_t* skills[5] = {
        &astro.Cap,
        &astro.LM,
        &astro.EVA,
        &astro.Docking,
        &astro.Endurance,
    };
    const char skillMax = 4;
    const char skillMin = 0;

    /* Moved to better prog, increase morale */
    if ((astro.Moved == 0) && (astro.oldAssign < astro.Assign)) {
        astro.Mood += 5;
    }

    // Determine Astronaut/Cosmonaut Level setting
    int AstLevel = (plr == 1) ? Data->Def.Ast2 + 1 : Data->Def.Ast1 + 1;

    /* TODO: Moved has to be reset somewhere, right? */

    /* Increase number of seasons astronaut was in active duty */
    if (astro.Status != AST_ST_DEAD && astro.Status != AST_ST_RETIRED) {
        astro.Active++;
    }

    /* Move All unassigned astros to limbo */
    if ((astro.Unassigned == 0)
        && (astro.Status == AST_ST_ACTIVE)
        && (astro.Assign != 0)) {
        astro.Assign = 0;
        astro.Moved = 0;
        astro.Special = 11 + plr;  /* WTF? */
    }

    /* Update skills after training */
    switch (astro.Status) {
    case AST_ST_TRAIN_BASIC_1:
    case AST_ST_TRAIN_BASIC_2:
    case AST_ST_TRAIN_BASIC_3: {
        /* FIXME: factor out a separate function? */
        if (astro.Status == AST_ST_TRAIN_BASIC_3) {
            astro.Special = 7;
            astro.TrainingLevel = AST_ST_TRAIN_BASIC_3;
            astro.Status = AST_ST_ACTIVE;
            astro.Assign = 0;          /* Put in Limbo */
        } else {
            astro.TrainingLevel = astro.Status;
            astro.Status++;
        }

        /* 70% for increase by 1, 30% for increase by 2 */
        char delta = (brandom(100) > 70) ? 2 : 1;

        /* Find skills that are below maximum */
        unsigned choices[NUM_SKILLS];
        unsigned j = 0;

        for (int i = 0; i < NUM_SKILLS; ++i) {
            if (*skills[i] < skillMax) {
                choices[j++] = i;
            }
        }

        if (j > 0) {
            /* If found, pick one skill at random */
            int8_t* skill = skills[choices[brandom(j)]];
            *skill = std::min(*skill + delta, skillMax);
        }

        /* Not sure why do it here, but let's keep it */
        for (int i = 0; i < NUM_SKILLS; ++i) {
            if (*skills[i] < skillMin) {
                *skills[i] = skillMin;
            }
        }

        break;
    }

    case AST_ST_TRAIN_ADV_1:
        astro.TrainingLevel = AST_ST_TRAIN_ADV_1;
        astro.Status = AST_ST_TRAIN_ADV_2;
        astro.Mood += 6 - AstLevel;
        break;

    case AST_ST_TRAIN_ADV_2:
        astro.TrainingLevel = AST_ST_TRAIN_ADV_2;

        if (options.feat_shorter_advanced_training) {
            astro.Status = AST_ST_TRAIN_ADV_4;
        } else {
            astro.Status = AST_ST_TRAIN_ADV_3;
        }

        astro.Mood += 6 - AstLevel;

        // Block created to localize 'skill' declaration
        {
            assert((unsigned) astro.Focus <= NUM_SKILLS);
            /* Increase trained skill by 1 ('naut is halfway through Adv Training) */
            int8_t *skill = skills[astro.Focus - 1];
            *skill = std::min(*skill + 1, skillMax);
        }

        break;

    case AST_ST_TRAIN_ADV_3:
        astro.TrainingLevel = AST_ST_TRAIN_ADV_3;
        astro.Status = AST_ST_TRAIN_ADV_4;
        astro.Mood += 6 - AstLevel;
        break;

    case AST_ST_TRAIN_ADV_4: {
        astro.Special = 8;
        astro.TrainingLevel = astro.Status;
        astro.Status = AST_ST_ACTIVE;
        astro.Assign = 0;  /* Put in Limbo */
        astro.Mood += 6 - AstLevel;

        assert((unsigned) astro.Focus <= NUM_SKILLS);

        /* Increase trained skill by 1 */
        int8_t* skill = skills[astro.Focus - 1];
        *skill = std::min(*skill + 1, skillMax);

        break;
    }

    default:
        break;
    }
}


/**
 * Assign a designation and patch to a mission.
 *
 * Updates the mission hardware's Code field, which tracks the mission's
 * designation (For example, Gemini V -> Gemini VI). Mission names
 * and codes should not be assigned to Future missions, as they are
 * not finalized.
 *
 * \param plr  the player index.
 * \param pad  the mission's launch pad index.
 */
void AssignMissionName(int plr, int pad)
{
    assert(plr >= 0 && plr < NUM_PLAYERS);
    assert(pad >= 0 && pad < MAX_MISSIONS);

    MissionType& mission = Data->P[plr].Mission[pad];
    
    Equipment* equip;
    if (mission.MissionCode == Mission_Orbital_Satellite) {
        equip = &Data->P[plr].Probe[PROBE_HW_ORBITAL];
        mission.Patch = -1;
    } else if (mission.MissionCode == Mission_Lunar_Probe) {
        equip = &Data->P[plr].Probe[PROBE_HW_LUNAR];
        mission.Patch = -1;
    } else if (mission.MissionCode == Mission_LunarFlyby ||
               (mission.MissionCode >= Mission_VenusFlyby &&
                mission.MissionCode <= Mission_SaturnFlyby)) {
        equip = &Data->P[plr].Probe[PROBE_HW_INTERPLANETARY];
        mission.Patch = -1;
    } else if (mission.MissionCode != Mission_None) {
        int capsule;
        if (mission.Joint != 0 && mission.Prog == 0) {
            assert(pad + 1 < MAX_MISSIONS);
            capsule = Data->P[plr].Mission[pad + 1].Prog - 1;
        } else {
            capsule = mission.Prog - 1;
        }
        equip = &Data->P[plr].Manned[capsule];
        mission.Patch = equip.Code % 10;
    }
    
    std::string name = std::string{equip.Name} + " " + RomanNumeral(equip.Code + 1);
    strncpy(&mission.Name[0], name.c_str(), sizeof(mission.Name) - 1);
    if (mission.MissionCode != Mission_None 
        && !(mission.Joint && mission.Prog == 0))
    {
        equip.Code++;   // Increase Planned Mission Count
    }
}


/**
 * Modifier to astronaut/cosmonaut's morale due to flight crew conflicts.
 */
int AstroConflictsMod(int player, Astros& astro)
{
    //-2 for each conflict in Jupiter/Minishuttle, -3 in others
    return -((astro.Assign == 5 || astro.Assign == 4) ? 2 : 3) *
           CrewConflicts(player, astro);
}


/** \todo: This code must be split... it's cluttered beyond hope */
void AstroTurn()
{
    /* Count total number of active astronauts */
    int ActTotal[2] = {0, 0};  /* Count of active astronauts per player */
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        for (int i = 0; i < Data->P[plr].AstroCount; i++) {
            auto& spaceman = Data->P[plr].Pool[i];
            if (spaceman.Status == AST_ST_ACTIVE) {
                ActTotal[plr]++;
            }
        }
    }

    /* Update All Astronauts */
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        for (int i = 0; i < Data->P[plr].AstroCount; i++) {
            auto& spaceman = Data->P[plr].Pool[i];
            updateAstronautSkills(plr, spaceman);
        }
    }

    for (int plr = 0; plr < NUM_PLAYERS; plr++) {  /* Player Analysis */
        int retirements = 0;

        for (int i = 0; i < Data->P[plr].AstroCount; i++) {
            auto& spaceman = Data->P[plr].Pool[i];
            int prog = spaceman.Assign;  // Program index
            int crew = spaceman.Crew - 1;  // Crew index
            // Work with mood as a temporary value rather than storing
            // an illegal value. Also stops int8_t overflow.
            int mood = spaceman.Mood;

            /* Injury Resolution */
            if (spaceman.Status == AST_ST_INJURED) {
                spaceman.InjuryDelay--;

                if (spaceman.InjuryDelay == 0) {
                    spaceman.Status = AST_ST_ACTIVE;
                    prog = spaceman.Assign = 0;
                    spaceman.Special = 5;
                }
            }

            /* Mustering Out - even seasons after 8 */
            if ((spaceman.Active >= 8)
                && spaceman.Active % 2 == 0
                && spaceman.Status == AST_ST_ACTIVE
                && spaceman.RetirementDelay == 0) {
                int num = brandom(100);

                if (num > 89) {
                    /* Guy retires */
                    if (plr == 0) {
                        spaceman.RetirementDelay = 3;  /* US Guy Retires in 2 */
                        spaceman.Special = 1;
                    }

                    if (plr == 1) {
                        spaceman.RetirementDelay = 2;  /* URS Guy Retires in 1 */
                        spaceman.Special = 1;
                    }

                    /* Reason for Retirement */
                    spaceman.RetirementReason = brandom(6) + 1;
                }
            }

            if (Data->P[plr].MissionCatastrophicFailureOnTurn & 1 &&
                spaceman.RetirementDelay == 0 &&
                spaceman.Status == AST_ST_ACTIVE) {
                /* Catastrophic Failure */
                int num = brandom(100);

                int temp = (plr == 0)? 79 : 89;

                // TODO: The cap on retirements means it's more likely
                // for astronauts earlier in the pool to retire. This
                // is justifiable for earlier groups vs later ones,
                // but since the pool is subordered alphabetically...
                if (num > temp && retirements < (ActTotal[plr] * .4)) {
                    /* Guy retires due to being scared */
                    if (plr == 0) {
                        spaceman.RetirementDelay = 3;  /* US Guy Retires in 2 */
                        spaceman.Special = 1;
                    }

                    // TODO: Either the comment is wrong or the delay
                    // is wrong -- rnyoakum
                    if (plr == 1) {
                        spaceman.RetirementDelay = 2;  /* URS Guy Retires Now */
                        spaceman.Special = 1;
                    }

                    spaceman.RetirementReason = 11;    /* Reason=Scared */

                    retirements++;
                }
            }

            /* Training Washout */
            if (spaceman.Status >= AST_ST_TRAIN_BASIC_1
                && spaceman.Status <= AST_ST_TRAIN_BASIC_3
                && strncmp(spaceman.Name, "ALDRIN", 6) != 0) {
                int num = brandom(100);

                if (num > 94) {
                    int num = brandom(100);
                    int enduranceFactor =
                        options.feat_use_endurance
                        ? 10 * std::max(0, spaceman.Endurance)
                        : 0;

                    if (num > (74 - enduranceFactor)) {
                        spaceman.Status = AST_ST_INJURED;
                        spaceman.InjuryDelay = 2;

                        if (options.feat_use_endurance && (brandom(100) < (spaceman.Endurance - 1) * 25)) {
                            spaceman.InjuryDelay = 1;  // High endurance can shorten time in hospital to half a year
                        }

                        if (options.feat_use_endurance && spaceman.Endurance < 1 && brandom(100) < 25) {
                            spaceman.InjuryDelay = 3;  // An Endurance of 0 can mean 3 turns in the hospital instead
                        }

                        spaceman.Special = 9;
                    } else {
                        // Injury washout and retirement
                        spaceman.Status = AST_ST_RETIRED;
                        spaceman.Special = 10;
                        spaceman.RetirementReason = 12;
                    }

                    if (spaceman.Cap < 0) {
                        spaceman.Cap = 0;
                    }

                    if (spaceman.LM < 0) {
                        spaceman.LM = 0;
                    }

                    if (spaceman.EVA < 0) {
                        spaceman.EVA = 0;
                    }

                    if (spaceman.Docking < 0) {
                        spaceman.Docking = 0;
                    }

                    if (spaceman.Endurance < 0) {
                        spaceman.Endurance = 0;
                    }
                }
            }

            if (spaceman.RetirementDelay >= 1
                && (spaceman.Status > AST_ST_INJURED
                    || spaceman.Status == AST_ST_ACTIVE)) {
                /* Actual retirement */
                spaceman.RetirementDelay--;

                if (spaceman.RetirementDelay == 0) {
                    spaceman.Status = AST_ST_RETIRED;
                    prog = spaceman.Assign = 0;
                    spaceman.Special = 2;
                }
            }

            // TODO: Shouldn't adjust mood if the astronaut / cosmonaut
            // is no longer in service.

            /* END OF SEASON - Positive */
            if (Data->P[plr].MissionCatastrophicFailureOnTurn & 4) {
                /* Program First */
                mood += 5;

                if (spaceman.currentMissionStatus == ASTRO_MISSION_FAILURE) {
                    mood += 20;    /* Self */
                }
            }

            if (spaceman.currentMissionStatus == ASTRO_MISSION_SUCCESS) {
                if (plr == 0 && Data->Def.Ast1 == 0) {
                    mood += 20;
                } else {
                    mood += 15;
                }

                if (plr == 1 && Data->Def.Ast2 == 0) {
                    mood += 20;
                } else {
                    mood += 15;
                }
            }

            if (Data->Season == 1) {
                /* End of turn what the hell 5% happy */
                if (brandom(100) > 94) {
                    mood += 5;
                }
            }

            if (prog > 0 && crew > -1) {
                bool temp = false;
                for (int k = 0; k < Data->P[plr].CrewCount[prog][crew]; k++) {
                    int spaceman_idx = Data->P[plr].Crew[prog][crew][k] - 1;
                    if (Data->P[plr].Pool[spaceman_idx].Hero == 1) {
                        temp = true;
                        break;
                    }
                }

                if (temp) {
                    mood += 5;    /* Hero Mod */
                }
            }

            /* END OF SEASON - Negative */

            /* In Merc for too long */
            if (spaceman.Assign == 1
                && spaceman.Moved >= 6) {
                mood -= 4;
            }

            /* Moved Around */
            if (spaceman.Moved == 0) {
                mood -= 4;
            }

            // Mission Stuff
            if (spaceman.Prime == 3
                || spaceman.Prime == 1) {
                spaceman.Prime = 0;
            }

            if (spaceman.Prime == 4
                || spaceman.Prime == 2) {
                spaceman.Prime--;
            }

            if (spaceman.Status != AST_ST_INJURED) {
                if (spaceman.Prime == 0) {
                    mood -= 6;
                }

                if (spaceman.Prime > 0) {
                    mood -= 3;
                }

                /* scrubbed mission */
                if (spaceman.currentMissionStatus == ASTRO_MISSION_SCRUBBED) {
                    mood -= 5;
                }

                /* successful mission */
            } else {
                mood -= 4;
            }

            /* catastrophic death */
            if (Data->P[plr].MissionCatastrophicFailureOnTurn & 1) {
                mood -= 5;
            }

            /* card death */
            if (Data->P[plr].MissionCatastrophicFailureOnTurn & 2) {
                mood -= brandom(2) + 1;
            }

            /* Compatibility */
            if (prog > 0) {
                mood += AstroConflictsMod(plr, spaceman);
            }

            /* Final record updating */
            spaceman.Mood = std::max(0, std::min(100, mood));

            spaceman.Moved++;

            /* Retirement stuff */

            if (spaceman.Mood < 20
                && spaceman.RetirementDelay == 0
                && spaceman.Status == AST_ST_ACTIVE) {
                if (plr == 0) {
                    spaceman.RetirementDelay = 2;  /* US Guy Retires in 2 */
                    spaceman.Special = 1;
                } else {
                    spaceman.Status = AST_ST_RETIRED; /* URS Guy Retires Now */
                    spaceman.Special = 2;
                }

                spaceman.RetirementReason = 13;    /* Reason=Unhappy */
            }

            spaceman.currentMissionStatus = ASTRO_MISSION_CLEAR;
        }

        Data->P[plr].MissionCatastrophicFailureOnTurn = 0;
    }

    // Break all groups with dead, injured or retired folks.
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        CheckFlightCrews(plr);
    }

    UpdateHardTurn(0);
    UpdateHardTurn(1);
}


/**
 * Count how many personality conflicts the astronaut/cosmonaut has
 * with their flight crew.
 */
int CrewConflicts(const int player, const Astros& astro)
{
    int conflicts = 0;

    if (astro.Assign <= 0 || astro.Crew <= 0) {
        return conflicts;
    }

    const int program = astro.Assign;
    const int crew = astro.Crew - 1;

    for (int i = 0; i < Data->P[player].CrewCount[program][crew]; i++) {
        int mate = Data->P[player].Crew[program][crew][i] - 1;
        assert(mate >= 0);

        if (!Compatible(astro, Data->P[player].Pool[mate])) {
            conflicts++;
        }
    }

    return conflicts;
}


/**
 * Represents a value in Roman numerals.
 *
 * This uses Standard Form notation to represent an value via
 * Roman Numerals, suitable for any number 0 to 3999.
 * The maximum length of the returned string is 15.
 */
std::string RomanNumeral(unsigned int value)
{
    std::string thousands[] = { "", "M", "MM", "MMM" };
    std::string hundreds[] =
    { "", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM" };
    std::string tens[] =
    { "", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC" };
    std::string ones[] =
    { "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };

    if (value > 3999) {
        throw std::runtime_error("Cannot craft a Roman Numeral > 3999");
    }

    return thousands[value / 1000] + hundreds[(value % 1000) / 100] +
           tens[(value % 100) / 10] + ones[value % 10];
}


void Update()
{
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {

        // If the Docking module is in orbit, reduce the usable seasons
        if (Data->P[plr].DockingModuleInOrbit) {
            Data->P[plr].DockingModuleInOrbit--;
        }

        // Decrement the Astronaut delays counter
        if (Data->P[plr].AstroDelay) {
            Data->P[plr].AstroDelay--;
        }

        for (int i = 0; i < MAX_MISSIONS; i++) {
            memcpy(&Data->P[plr].Mission[i], &Data->P[plr].Future[i], sizeof(struct MissionType));
            memset(&Data->P[plr].Future[i], 0x00, sizeof(struct MissionType));
            strcpy(Data->P[plr].Future[i].Name, "UNDETERMINED");

        }  /* End i for loop */
    }  /* End plr for loop */

    // Name the upcoming Missions
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        for (int i = 0; i < MAX_MISSIONS; i++) {
            AssignMissionName(plr, i);
        }
    }

    // Reset R&D Purchasing Ability
    memset(Data->P[0].Buy, 0x00, sizeof(Data->P[0].Buy));
    memset(Data->P[1].Buy, 0x00, sizeof(Data->P[1].Buy));

    AstroTurn();   /* Process all astronauts */

    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        Data->P[plr].RD_Mods_For_Turn = 0;

        if (Data->P[plr].RD_Mods_For_Year > 0) {
            Data->P[plr].RD_Mods_For_Turn = Data->P[plr].RD_Mods_For_Year;
            Data->P[plr].RD_Mods_For_Year = 0;
        }

        Data->P[plr].TurnOnly = Data->P[plr].MissionCatastrophicFailureOnTurn = Data->P[plr].Block = 0;
    }

    UpdateFlybys();

    memset(pNeg, 0x00, sizeof pNeg);

    // Fix Prestige Values for Mars, Jup, Sat.
    for (int plr = 0; plr < NUM_PLAYERS; plr++) {
        Data->Prestige[Prestige_MarsFlyby].Goal[plr] = 0;  // Clear Mars
        Data->Prestige[Prestige_JupiterFlyby].Goal[plr] = 0;  // Clear Jupiter
        Data->Prestige[Prestige_SaturnFlyby].Goal[plr] = 0;  // Clear Saturn
        Data->P[plr].Probe[PROBE_HW_ORBITAL].Failures = Data->P[plr].Probe[PROBE_HW_LUNAR].Failures = 0;
        Data->P[plr].Probe[PROBE_HW_ORBITAL].Used = Data->P[plr].Probe[PROBE_HW_LUNAR].Used = 0;

        for (int i = 0; i < Data->P[plr].PastMissionCount; i++) {
            auto& mission = Data->P[plr].History[i];
            if (mission.Event == 0) {
                switch (mission.MissionCode) {
                case Mission_MarsFlyby:
                    Data->Prestige[Prestige_MarsFlyby].Goal[plr]++;
                    break;  // Mars

                case Mission_JupiterFlyby:
                    Data->Prestige[Prestige_JupiterFlyby].Goal[plr]++;
                    break;  // Jupiter

                case Mission_SaturnFlyby:
                    Data->Prestige[Prestige_SaturnFlyby].Goal[plr]++;
                    break;  // Saturn

                default:
                    break;
                }
            }

            switch (mission.MissionCode) {
            case Mission_Orbital_Satellite:
                if (mission.spResult != 1) {
                    Data->P[plr].Probe[PROBE_HW_ORBITAL].Failures++;
                }

                Data->P[plr].Probe[PROBE_HW_ORBITAL].Used++;
                break;

            case Mission_Lunar_Probe:
                if (mission.spResult != 1) {
                    Data->P[plr].Probe[PROBE_HW_LUNAR].Failures++;
                }

                Data->P[plr].Probe[PROBE_HW_LUNAR].Used++;
                break;
            }

        }
    }
}


/**
 * Tests a Flyby mission to decide if it was successful.
 *
 * \param plr
 * \param i
 */
void TestFMis(int plr, int i)
{
    auto& mission = Data->P[plr].History[i];

    if (mission.Saf == 0) return;
    if (mission.Event <= 0) return;

    mission.Event--;

    if (brandom(100) > mission.Saf) {
        /* Failed Mission */
        int misCode = mission.MissionCode;
        mission.Event = mission.Saf = 0;
        // TODO: Break this line up; it's unreadable!
        mission.Prestige =
            PrestNeg(plr, (misCode == Mission_MarsFlyby) ? Prestige_MarsFlyby 
                            : (misCode == Mission_JupiterFlyby) ? Prestige_JupiterFlyby 
                                : Prestige_SaturnFlyby);
        Data->P[plr].Plans |= (misCode == Mission_MarsFlyby) ? 0x01 
                                : (misCode == Mission_JupiterFlyby) ? 0x02 
                                    : 0x04;
        mission.spResult = 5000;
    }

    if (mission.Event == 0 && mission.Prestige == 0) {
        int misCode = mission.MissionCode;
        mission.Prestige =
            Set_Goal(plr, (misCode == Mission_MarsFlyby) ? Prestige_MarsFlyby 
                              : (misCode == Mission_JupiterFlyby) ? Prestige_JupiterFlyby 
                                  : Prestige_SaturnFlyby
                    , 3);
        Data->P[plr].Plans |= (misCode == Mission_MarsFlyby) ? 0x10 
                                : (misCode == Mission_JupiterFlyby) ? 0x20 
                                    : 0x40;
        mission.spResult = 1;
        mission.Saf = 0;
    }
}


/**
 * Update any outstanding Flyby missions.
 *
 * Missions are tested in chronological order of launch so prestige
 * will be awarded correctly. If both month and year match, the player
 * with the lower budget proceeds first.
 */
void UpdateFlybys()
{
    int p0 = 0, p1 = 0;

    // Missions are tested in chronological order of launch.
    // In case of a tie, the player with the lower budget wins.
    // If budgets are tied, a coin is tossed.
    while (p0 < Data->P[0].PastMissionCount &&
           p1 < Data->P[1].PastMissionCount) {
        auto& hist_us = Data->P[0].History[p0];
        auto& hist_sv = Data->P[1].History[p1];
        
        if (hist_us.MissionYear < hist_sv.MissionYear) { // order by Year
            TestFMis(0, p0++);
        } else if (hist_us.MissionYear > hist_sv.MissionYear) {
            TestFMis(1, p1++);
        } else if (hist_us.Month < hist_sv.Month) { // if equal, order by month
            TestFMis(0, p0++);
        } else if (hist_us.Month > hist_sv.Month) {
            TestFMis(1, p1++);
        } else if (Data->P[0].Budget < Data->P[1].Budget) { // in case of a tie, the player with the lower budget wins.
            TestFMis(0, p0++);
        } else if (Data->P[0].Budget > Data->P[1].Budget) {
            TestFMis(1, p1++);
        } else if (brandom(100) < 50) { // if budgets are tied, a coin is tossed.
            TestFMis(0, p0++);
        } else {
            TestFMis(1, p1++);
        }
    }

    // After updating all of one player's past missions, handle
    // the remainder of the other player's missions, if any.
    while (p0 < Data->P[0].PastMissionCount) {
        TestFMis(0, p0++);
    }

    while (p1 < Data->P[1].PastMissionCount) {
        TestFMis(1, p1++);
    }
}


/** End of turn equipment accounting update
 */
void UpdateHardTurn(int plr)
{
    BuzzData& p = Data->P[plr];
    
    Equipment *e;
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            e = &p.Probe[0];
        } else if (i == 1) {
            e = &p.Rocket[0];
        } else if (i == 2) {
            e = &p.Manned[0];
        } else if (i == 3) {
            e = &p.Misc[0];
        }

        for (int j = 0; j < 7; j++) {
            if (e[j].Delay) {
                e[j].Delay--;
            }

            if (e[j].Num) {
                e[j].Seas++;
            }

            e[j].Spok = 0;
        }
    }
}

/* vim: set noet ts=4 sw=4 tw=77: */
