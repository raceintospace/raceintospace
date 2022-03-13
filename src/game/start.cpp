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

#include "Buzz_inc.h"
#include "astros.h"
#include "game_main.h"
#include "options.h"
#include "pace.h"
#include "prest.h"


/** \todo This function fills Data->Events, but how it's a mystery... */

int AstroConflictsMod(int player, struct Astros &astro);
void AstroTurn(void);
int  CrewConflicts(int player, const struct Astros &astro);
void UpdAll(char side);
void TestFMis(int plr, int i);
void UpdateHardTurn(char plr);


#define EVENT_PERIODS 6
#define EVENT_PERIOD_POOL 5

/* Initialize the event cards */
void InitializeEvents(void)
{
    int cardCount = 2;  // Starting event card index
    int j, selectedCard;
    bool eventSelected[MAXIMUM_NEWS_EVENTS];  // used to track which event cards have been selected
    char quantityInSet[EVENT_PERIODS] = {2, 8, 8, 12, 16, 52};  // Number of events in each period
    char poolPercentages[EVENT_PERIODS][EVENT_PERIOD_POOL] = {
        {100, 0, 0, 0, 0},
        {0, 100, 0, 0, 0},
        {0, 62, 38, 0, 0},
        {0, 50, 20, 30, 0},
        {0, 35, 20, 20, 25},
        {0, 25, 25, 25, 25}
    };
    struct EventCardQueue {
        char firstCard;
        char numberInSet;
        char pic;
    } eventCardSet[EVENT_PERIOD_POOL] = {
        {1, 3, 0},
        {4, 40, 0},
        {44, 19, 0},
        {63, 17, 0},
        {80, 20, 0}
    };

    // Initialize base event data to 0's
    for (int i = 0; i < MAXIMUM_NEWS_EVENTS; i++) {
        eventSelected[i] =  false;
        Data->Events[i] = 0;
    }

    for (int period = 0; period < EVENT_PERIODS; period++) {
        for (int set = 0; set < quantityInSet[period]; set++) {
random_card:
            // Pick a random number
            int randomCard = brandom(100) + 1;
            j = selectedCard = 0;

            // This is just wrong...
            while (randomCard > selectedCard) {
                selectedCard += poolPercentages[period][j++];
            }

            j--;

            // If we've reached the end of the card subset then pick another card
            if (eventCardSet[j].numberInSet == eventCardSet[j].pic) {
                goto random_card;
            }

            selectedCard = brandom(eventCardSet[j].numberInSet) + eventCardSet[j].firstCard;

            while (eventSelected[selectedCard]) {
                if (selectedCard == (eventCardSet[j].numberInSet + eventCardSet[j].firstCard - 1)) {
                    selectedCard = eventCardSet[j].firstCard;
                } else {
                    selectedCard = selectedCard + 1;
                }
            }

            eventSelected[selectedCard] = true;
            Data->Events[cardCount++] = selectedCard;
            eventCardSet[j].pic++;
        }
    }

    // Set defaults for Prestige
    for (int i = 0; i < MAXIMUM_PRESTIGE_NUM; i++) {
        Data->Prestige[i].Place = Data->Prestige[i].mPlace = -1;
    }

    // Initialize in Memory REPLAY.DAT
    interimData.replaySize = sizeof(REPLAY) * MAX_REPLAY_ITEMS;
    memset(interimData.tempReplay, 0, interimData.replaySize);

    // Initialize in memory for Event Saves
    interimData.eventSize = 84 * sizeof(OLDNEWS);
    interimData.eventBuffer = (char *) malloc(interimData.eventSize);
    memset(interimData.eventBuffer, 0, interimData.eventSize);
    interimData.tempEvents = (OLDNEWS *) interimData.eventBuffer;
}

/**
 * Handle astronaut mood and training
 */
static void
updateAstronautSkills(unsigned plr, struct Astros *astro)
{
    /* Constants related to training */
    const unsigned NUM_SKILLS = 5;
    char *skills[5] = {
        &astro->Cap,
        &astro->LM,
        &astro->EVA,
        &astro->Docking,
        &astro->Endurance,
    };
    const char skillMax = 4;
    const char skillMin = 0;

    if ((astro->Moved == 0)
        && (astro->oldAssign < astro->Assign))
        /* Moved to better prog, increase morale */
    {
        astro->Mood += 5;
    }

    // Determine Astronaut/Cosmonaut Level setting
    int AstLevel;
    if ((plr == 1 && Data->Def.Ast2 == 2) || (plr == 0 && Data->Def.Ast1 == 2)) { AstLevel = 3; }
    else if ((plr == 1 && Data->Def.Ast2 == 1) || (plr == 0 && Data->Def.Ast1 == 1)) { AstLevel = 2; }
    else { AstLevel = 1; }

    /* TODO: Moved has to be reset somewhere, right? */

    /* Increase number of seasons astronaut was in active duty */
    switch (astro->Status) {
    case AST_ST_DEAD:
    case AST_ST_RETIRED:
        break;

    default:
        astro->Active++;
    }

    /* Move All unassigned astros to limbo */
    if ((astro->Unassigned == 0)
        && (astro->Status == AST_ST_ACTIVE)
        && (astro->Assign != 0)) {
        astro->Assign = 0;
        astro->Moved = 0;
        astro->Special = 11 + plr;  /* WTF? */
    }

    /* Update skills after training */
    switch (astro->Status) {
    case AST_ST_TRAIN_BASIC_1:
    case AST_ST_TRAIN_BASIC_2:
    case AST_ST_TRAIN_BASIC_3: {
        /* FIXME: factor out a separate function? */
        if (astro->Status == AST_ST_TRAIN_BASIC_3) {
            astro->Special = 7;
            astro->TrainingLevel = AST_ST_TRAIN_BASIC_3;
            astro->Status = AST_ST_ACTIVE;
            astro->Assign = 0;          /* Put in Limbo */
        } else {
            astro->TrainingLevel = astro->Status;
            astro->Status++;
        }

        /* 70% for increase by 1, 30% for increase by 2 */
        char delta = (brandom(100) > 70) ? 2 : 1;

        /* Find skills that are below maximum */
        unsigned choices[NUM_SKILLS];
        unsigned i = 0;
        unsigned j = 0;

        for (i = 0; i < NUM_SKILLS; ++i) {
            if (*skills[i] < skillMax) {
                choices[j++] = i;
            }
        }

        if (j > 0) {
            /* If found, pick one skill at random */
            char *skill = skills[choices[brandom(j)]];
            *skill = MIN(*skill + delta, skillMax);
        }

        /* Not sure why do it here, but let's keep it */
        for (i = 0; i < NUM_SKILLS; ++i) {
            if (*skills[i] < skillMin) {
                *skills[i] = skillMin;
            }
        }

        break;
    }

    case AST_ST_TRAIN_ADV_1:
        astro->TrainingLevel = AST_ST_TRAIN_ADV_1;
        astro->Status = AST_ST_TRAIN_ADV_2;
        astro->Mood += 6 - AstLevel;
        break;

    case AST_ST_TRAIN_ADV_2:
        astro->TrainingLevel = AST_ST_TRAIN_ADV_2;

        if (options.feat_shorter_advanced_training) {
            astro->Status = AST_ST_TRAIN_ADV_4;
        } else {
            astro->Status = AST_ST_TRAIN_ADV_3;
        }
        astro->Mood += 6 - AstLevel;

        // Block created to localize 'skill' declaration
        {
            assert((unsigned) astro->Focus <= NUM_SKILLS);
            /* Increase trained skill by 1 ('naut is halfway through Adv Training) */
            char *skill = skills[astro->Focus - 1];
            *skill = MIN(*skill + 1, skillMax);
        }

        break;

    case AST_ST_TRAIN_ADV_3:
        astro->TrainingLevel = AST_ST_TRAIN_ADV_3;
        astro->Status = AST_ST_TRAIN_ADV_4;
        astro->Mood += 6 - AstLevel;
        break;

    case AST_ST_TRAIN_ADV_4: {
        astro->Special = 8;
        astro->TrainingLevel = astro->Status;
        astro->Status = AST_ST_ACTIVE;
        astro->Assign = 0;  /* Put in Limbo */
        astro->Mood += 6 - AstLevel;

        assert((unsigned) astro->Focus <= NUM_SKILLS);

        /* Increase trained skill by 1 */
        char *skill = skills[astro->Focus - 1];
        *skill = MIN(*skill + 1, skillMax);

        break;
    }

    default:
        break;
    }
}


/**
 * Modifier to astronaut/cosmonaut's morale due to flight crew conflicts.
 */
int AstroConflictsMod(int player, struct Astros &astro)
{
    //-2 for each conflict in Jupiter/Minishuttle, -3 in others
    return -((astro.Assign == 5 || astro.Assign == 4) ? 2 : 3) *
           CrewConflicts(player, astro);
}


/** \todo: This code must be split... it's cluttered beyond hope */
void
AstroTurn(void)
{
    int i, j, k, num, temp;
    int ActTotal[2] = {0, 0};  /* Count of active astronauts per player */

    /* Count total number of active astronauts */
    for (j = 0; j < NUM_PLAYERS; j++) {
        for (i = 0; i < Data->P[j].AstroCount; i++) {
            if (Data->P[j].Pool[i].Status == AST_ST_ACTIVE) {
                ActTotal[j]++;
            }
        }
    }

    /* Update All Astronauts */
    for (j = 0; j < NUM_PLAYERS; j++) {
        for (i = 0; i < Data->P[j].AstroCount; i++) {
            updateAstronautSkills(j, &Data->P[j].Pool[i]);
        }
    }

    for (j = 0; j < NUM_PLAYERS; j++) {  /* Player Analysis */
        int retirements = 0;

        for (i = 0; i < Data->P[j].AstroCount; i++) {
            int prog = Data->P[j].Pool[i].Assign;  // Program index
            int crew = Data->P[j].Pool[i].Crew - 1;  // Crew index
            // Work with mood as a temporary value rather than storing
            // an illegal value. Also stops int8_t overflow.
            int mood = Data->P[j].Pool[i].Mood;

            /* Injury Resolution */
            if (Data->P[j].Pool[i].Status == AST_ST_INJURED) {
                Data->P[j].Pool[i].InjuryDelay--;

                if (Data->P[j].Pool[i].InjuryDelay == 0) {
                    Data->P[j].Pool[i].Status = AST_ST_ACTIVE;
                    prog = Data->P[j].Pool[i].Assign = 0;
                    Data->P[j].Pool[i].Special = 5;
                }
            }

            /* Mustering Out - even seasons after 8 */
            if ((Data->P[j].Pool[i].Active >= 8)
                && Data->P[j].Pool[i].Active % 2 == 0
                && Data->P[j].Pool[i].Status == AST_ST_ACTIVE
                && Data->P[j].Pool[i].RetirementDelay == 0) {
                num = brandom(100);

                if (num > 89) {
                    /* Guy retires */
                    if (j == 0) {
                        Data->P[j].Pool[i].RetirementDelay = 3;  /* US Guy Retires in 2 */
                        Data->P[j].Pool[i].Special = 1;
                    }

                    if (j == 1) {
                        Data->P[j].Pool[i].RetirementDelay = 2;  /* URS Guy Retires in 1 */
                        Data->P[j].Pool[i].Special = 1;
                    }

                    Data->P[j].Pool[i].RetirementReason = brandom(6) + 1;  /* Reason for Retirement */
                }
            }

            if (Data->P[j].MissionCatastrophicFailureOnTurn & 1 &&
                Data->P[j].Pool[i].RetirementDelay == 0 &&
                Data->P[j].Pool[i].Status == AST_ST_ACTIVE) {
                /* Catastrophic Failure */
                num = brandom(100);

                if (j == 1) {
                    temp = 89;
                } else {
                    temp = 79;
                }

                // TODO: The cap on retirements means it's more likely
                // for astronauts earlier in the pool to retire. This
                // is justifiable for earlier groups vs later ones,
                // but since the pool is subordered alphabetically...
                if (num > temp && retirements < (ActTotal[j] * .4)) {
                    /* Guy retires due to being scared */
                    if (j == 0) {
                        Data->P[j].Pool[i].RetirementDelay = 3;  /* US Guy Retires in 2 */
                        Data->P[j].Pool[i].Special = 1;
                    }

                    // TODO: Either the comment is wrong or the delay
                    // is wrong -- rnyoakum
                    if (j == 1) {
                        Data->P[j].Pool[i].RetirementDelay = 2;  /* URS Guy Retires Now */
                        Data->P[j].Pool[i].Special = 1;
                    }

                    Data->P[j].Pool[i].RetirementReason = 11;    /* Reason=Scared */

                    retirements++;
                }
            }

            /* Training Washout */
            if (Data->P[j].Pool[i].Status >= AST_ST_TRAIN_BASIC_1
                && Data->P[j].Pool[i].Status <= AST_ST_TRAIN_BASIC_3
                && strncmp(Data->P[j].Pool[i].Name, "ALDRIN", 6) != 0) {
                num = brandom(100);

                if (num > 94) {
                    num = brandom(100);
                    int enduranceFactor =
                        options.feat_use_endurance
                        ? 10 * MAX(0, Data->P[j].Pool[i].Endurance)
                        : 0;

                    if (num > (74 - enduranceFactor)) {
                        Data->P[j].Pool[i].Status = AST_ST_INJURED;
                        Data->P[j].Pool[i].InjuryDelay = 2;
                        Data->P[j].Pool[i].Special = 9;
                    } else {
                        Data->P[j].Pool[i].Status = AST_ST_RETIRED;
                        Data->P[j].Pool[i].Special = 10;
                        Data->P[j].Pool[i].RetirementReason = 12;    /* Washout */
                    }

                    if (Data->P[j].Pool[i].Cap < 0) {
                        Data->P[j].Pool[i].Cap = 0;    //modified from Data->P[j].Pool[i].Endurance, as it's probably a bug
                    }

                    if (Data->P[j].Pool[i].LM < 0) {
                        Data->P[j].Pool[i].LM = 0;
                    }

                    if (Data->P[j].Pool[i].EVA < 0) {
                        Data->P[j].Pool[i].EVA = 0;
                    }

                    if (Data->P[j].Pool[i].Docking < 0) {
                        Data->P[j].Pool[i].Docking = 0;
                    }

                    if (Data->P[j].Pool[i].Endurance < 0) {
                        Data->P[j].Pool[i].Endurance = 0;
                    }
                }
            }

            if (Data->P[j].Pool[i].RetirementDelay >= 1
                && (Data->P[j].Pool[i].Status > AST_ST_INJURED
                    || Data->P[j].Pool[i].Status == AST_ST_ACTIVE)) {
                /* Actual retirement */
                Data->P[j].Pool[i].RetirementDelay--;

                if (Data->P[j].Pool[i].RetirementDelay == 0) {
                    Data->P[j].Pool[i].Status = AST_ST_RETIRED;
                    prog = Data->P[j].Pool[i].Assign = 0;
                    Data->P[j].Pool[i].Special = 2;
                }
            }

            // TODO: Shouldn't adjust mood if the astronaut / cosmonaut
            // is no longer in service.

            /* END OF SEASON - Positive */
            if (Data->P[j].MissionCatastrophicFailureOnTurn & 4) {
                /* Program First */
                mood += 5;

                if (Data->P[j].Pool[i].currentMissionStatus == ASTRO_MISSION_FAILURE) {
                    mood += 20;    /* Self */
                }
            }

            if (Data->P[j].Pool[i].currentMissionStatus == ASTRO_MISSION_SUCCESS) {
                if (j == 0 && Data->Def.Ast1 == 0) {
                    mood += 20;
                } else {
                    mood += 15;
                }

                if (j == 1 && Data->Def.Ast2 == 0) {
                    mood += 20;
                } else {
                    mood += 15;
                }
            }

            if (Data->Season == 1) {
                /* End of turn what the hell 5% happy */
                num = brandom(100);

                if (num > 94) {
                    mood += 5;
                }
            }

            if (prog > 0) {
                temp = 0;

                for (k = 0; k < Data->P[j].CrewCount[prog][crew]; k++) {
                    if (Data->P[j].Pool[
                            Data->P[j].Crew[prog][crew][k] - 1].Hero == 1) {
                        temp++;
                    }
                }

                if (temp > 1) {
                    mood += 5;    /* Hero Mod */
                }
            }

            /* END OF SEASON - Negative */

            /* In Merc for too long */
            if (Data->P[j].Pool[i].Assign == 1
                && Data->P[j].Pool[i].Moved >= 6) {
                mood -= 4;
            }

            /* Moved Around */
            if (Data->P[j].Pool[i].Moved == 0) {
                mood -= 4;
            }

            // Mission Stuff
            if (Data->P[j].Pool[i].Prime == 3
                || Data->P[j].Pool[i].Prime == 1) {
                Data->P[j].Pool[i].Prime = 0;
            }

            if (Data->P[j].Pool[i].Prime == 4
                || Data->P[j].Pool[i].Prime == 2) {
                Data->P[j].Pool[i].Prime--;
            }

            if (Data->P[j].Pool[i].Status != AST_ST_INJURED) {
                if (Data->P[j].Pool[i].Prime == 0) {
                    mood -= 6;
                }

                if (Data->P[j].Pool[i].Prime > 0) {
                    mood -= 3;
                }

                /* scrubbed mission */
                if (Data->P[j].Pool[i].currentMissionStatus == ASTRO_MISSION_SCRUBBED) {
                    mood -= 5;
                }

                /* successful mission */
            } else {
                mood -= 4;
            }

            /* catastrophic death */
            if (Data->P[j].MissionCatastrophicFailureOnTurn & 1) {
                mood -= 5;
            }

            /* card death */
            if (Data->P[j].MissionCatastrophicFailureOnTurn & 2) {
                mood -= brandom(2) + 1;
            }

            /* Compatibility */
            if (prog > 0) {
                mood += AstroConflictsMod(j, Data->P[j].Pool[i]);
            }

            /* Final record updating */
            Data->P[j].Pool[i].Mood = MAX(0, MIN(100, mood));

            Data->P[j].Pool[i].Moved++;

            /* Retirement stuff */

            if (Data->P[j].Pool[i].Mood < 20
                && Data->P[j].Pool[i].RetirementDelay == 0
                && Data->P[j].Pool[i].Status == AST_ST_ACTIVE) {
                if (j == 0) {
                    Data->P[j].Pool[i].RetirementDelay = 2;  /* US Guy Retires in 2 */
                    Data->P[j].Pool[i].Special = 1;
                }

                if (j == 1) {
                    Data->P[j].Pool[i].Status = AST_ST_RETIRED; /* URS Guy Retires Now */
                    Data->P[j].Pool[i].Special = 2;
                }

                Data->P[j].Pool[i].RetirementReason = 13;    /* Reason=Unhappy */
            }

            Data->P[j].Pool[i].currentMissionStatus = ASTRO_MISSION_CLEAR;
        }

        Data->P[j].MissionCatastrophicFailureOnTurn = 0;
    }

    // Break all groups with dead, injured or retired folks.
    for (j = 0; j < NUM_PLAYERS; j++) {  // for each player
        CheckFlightCrews(j);
    }                          /* for j */

    UpdateHardTurn(0);
    UpdateHardTurn(1);

    return;
}


/**
 * Count how many personality conflicts the astronaut/cosmonaut has
 * with their flight crew.
 */
int CrewConflicts(const int player, const struct Astros &astro)
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


void Update(void)
{
    int i, j, k;
    char p0, p1;
    char tName[20];

    for (j = 0; j < NUM_PLAYERS; j++) {

        // If the Docking module is in orbit, reduce the usable seasons
        if (Data->P[j].DockingModuleInOrbit) {
            Data->P[j].DockingModuleInOrbit--;
        }

        // Decrement the Astronaut delays counter
        if (Data->P[j].AstroDelay) {
            Data->P[j].AstroDelay--;
        }

        for (i = 0; i < MAX_MISSIONS; i++) {
            memcpy(&Data->P[j].Mission[i], &Data->P[j].Future[i], sizeof(struct MissionType));
            memset(&Data->P[j].Future[i], 0x00, sizeof(struct MissionType));
            strcpy(Data->P[j].Future[i].Name, "UNDETERMINED");

        }  /* End j for loop */
    }  /* End i for loop */

    // Name the Missions

    for (j = 0; j < NUM_PLAYERS; j++) {
        for (i = 0; i < MAX_MISSIONS; i++) {
            if (Data->P[j].Mission[i].MissionCode == Mission_Orbital_Satellite) {
                Data->P[j].Mission[i].Patch = -1;
                strcpy(&tName[0], &Data->P[j].Probe[PROBE_HW_ORBITAL].Name[0]);
                strcat(&tName[0], " ");
                strcat(&tName[0], RomanNumeral(Data->P[j].Probe[PROBE_HW_ORBITAL].Code + 1).c_str());
                strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                Data->P[j].Probe[PROBE_HW_ORBITAL].Code++;  // Increase Planned Mission Count
            } else if (Data->P[j].Mission[i].MissionCode == Mission_Lunar_Probe) {
                Data->P[j].Mission[i].Patch = -1;
                strcpy(&tName[0], &Data->P[j].Probe[PROBE_HW_LUNAR].Name[0]);
                strcat(&tName[0], " ");
                strcat(&tName[0], RomanNumeral(Data->P[j].Probe[PROBE_HW_LUNAR].Code + 1).c_str());
                strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                Data->P[j].Probe[PROBE_HW_LUNAR].Code++;  // Increase Planned Mission Count
            } else if (Data->P[j].Mission[i].MissionCode == Mission_LunarFlyby ||
                       (Data->P[j].Mission[i].MissionCode >= Mission_VenusFlyby &&
                        Data->P[j].Mission[i].MissionCode <= Mission_SaturnFlyby)) {
                Data->P[j].Mission[i].Patch = -1;
                strcpy(&tName[0], &Data->P[j].Probe[PROBE_HW_INTERPLANETARY].Name[0]);
                strcat(&tName[0], " ");
                strcat(&tName[0], RomanNumeral(Data->P[j].Probe[PROBE_HW_INTERPLANETARY].Code + 1).c_str());
                strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                Data->P[j].Probe[PROBE_HW_INTERPLANETARY].Code++;  // Increase Planned Mission Count
            } else if (Data->P[j].Mission[i].MissionCode > 0) {
                if (Data->P[j].Mission[i].Joint == 0) {
                    k = Data->P[j].Mission[i].Prog - 1;
                    Data->P[j].Mission[i].Patch = Data->P[j].Manned[k].Code % 10;
                    strcpy(&tName[0], &Data->P[j].Manned[k].Name[0]);
                    strcat(&tName[0], " ");
                    strcat(&tName[0], RomanNumeral(Data->P[j].Manned[k].Code + 1).c_str());
                    strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                    Data->P[j].Manned[k].Code++;  // Increase Planned Mission Count
                } else {
                    if (Data->P[j].Mission[i].Prog == 0) {
                        k = Data->P[j].Mission[i + 1].Prog - 1;
                        Data->P[j].Mission[i].Patch = Data->P[j].Manned[k].Code % 10;
                        strcpy(&tName[0], &Data->P[j].Manned[k].Name[0]);
                        strcat(&tName[0], " ");
                        strcat(&tName[0], RomanNumeral(Data->P[j].Manned[k].Code + 1).c_str());
                        strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                    } else {
                        k = Data->P[j].Mission[i].Prog - 1;
                        Data->P[j].Mission[i].Patch = Data->P[j].Manned[k].Code % 10;
                        strcpy(&tName[0], &Data->P[j].Manned[k].Name[0]);
                        strcat(&tName[0], " ");
                        strcat(&tName[0], RomanNumeral(Data->P[j].Manned[k].Code + 1).c_str());
                        strcpy(&Data->P[j].Mission[i].Name[0], &tName[0]);  // copy into struct
                        Data->P[j].Manned[k].Code++;  // Increase Planned Mission Count
                    }
                }
            }
        }
    }

    // Reset R&D Purchasing Ability
    memset(Data->P[0].Buy, 0x00, sizeof(Data->P[0].Buy));
    memset(Data->P[1].Buy, 0x00, sizeof(Data->P[1].Buy));

    AstroTurn();   /* Process all astronauts */

    for (j = 0; j < NUM_PLAYERS; j++) {
        Data->P[j].RD_Mods_For_Turn = 0;

        if (Data->P[j].RD_Mods_For_Year > 0) {
            Data->P[j].RD_Mods_For_Turn = Data->P[j].RD_Mods_For_Year;
            Data->P[j].RD_Mods_For_Year = 0;
        }

        Data->P[j].TurnOnly = Data->P[j].MissionCatastrophicFailureOnTurn = Data->P[j].Block = 0;
    }

    // Update any delayed Missions
    p0 = p1 = 0;

    while (p0 < Data->P[0].PastMissionCount && p1 < Data->P[1].PastMissionCount) {
        if (Data->P[0].History[p0].MissionYear < Data->P[1].History[p1].MissionYear) {
            TestFMis(0, p0);
            p0++;
        } else if (Data->P[0].History[p0].MissionYear > Data->P[1].History[p1].MissionYear) {
            TestFMis(1, p1);
            p1++;
        } else if (Data->P[0].History[p0].MissionYear == Data->P[1].History[p1].MissionYear) {
            if (Data->P[0].History[p0].Month < Data->P[1].History[p1].Month) {
                TestFMis(0, p0);
                p0++;
            } else if (Data->P[0].History[p0].Month > Data->P[1].History[p1].Month) {
                TestFMis(1, p1);
                p1++;
            } else if (Data->P[0].History[p0].Month == Data->P[1].History[p1].Month) {
                if (Data->P[0].Budget < Data->P[1].Budget && (p0 < Data->P[0].PastMissionCount)) {
                    TestFMis(0, p0);
                    p0++;
                } else if (Data->P[0].Budget > Data->P[1].Budget && (p1 < Data->P[1].PastMissionCount)) {
                    TestFMis(1, p1);
                    p1++;
                } else if ((p0 < Data->P[0].PastMissionCount) && (p1 < Data->P[1].PastMissionCount)) {
                    if (brandom(100) < 50) {
                        TestFMis(0, p0);
                        p0++;
                    } else {
                        TestFMis(1, p1);
                        p1++;
                    }
                }
            }
        }
    }  // end while

    memset(pNeg, 0x00, sizeof pNeg);

    while (p0 < Data->P[0].PastMissionCount) {
        TestFMis(0, p0);
        p0++;
    }

    while (p1 < Data->P[1].PastMissionCount) {
        TestFMis(1, p1);
        p1++;
    }

    // Fix Prestige Values for Mars, Jup, Sat.
    for (j = 0; j < NUM_PLAYERS; j++) {
        Data->Prestige[Prestige_MarsFlyby].Goal[j] = 0;  // Clear Mars
        Data->Prestige[Prestige_JupiterFlyby].Goal[j] = 0;  // Clear Jupiter
        Data->Prestige[Prestige_SaturnFlyby].Goal[j] = 0;  // Clear Saturn
        Data->P[j].Probe[PROBE_HW_ORBITAL].Failures = Data->P[j].Probe[PROBE_HW_LUNAR].Failures = 0;
        Data->P[j].Probe[PROBE_HW_ORBITAL].Used = Data->P[j].Probe[PROBE_HW_LUNAR].Used = 0;

        for (i = 0; i < Data->P[j].PastMissionCount; i++) {
            if (Data->P[j].History[i].Event == 0) {
                switch (Data->P[j].History[i].MissionCode) {
                case Mission_MarsFlyby:
                    Data->Prestige[Prestige_MarsFlyby].Goal[j]++;
                    break;  // Mars

                case Mission_JupiterFlyby:
                    Data->Prestige[Prestige_JupiterFlyby].Goal[j]++;
                    break;  // Jupiter

                case Mission_SaturnFlyby:
                    Data->Prestige[Prestige_SaturnFlyby].Goal[j]++;
                    break;  // Saturn

                default:
                    break;
                }
            }

            switch (Data->P[j].History[i].MissionCode) {
            case Mission_Orbital_Satellite:
                if (Data->P[j].History[i].spResult != 1) {
                    Data->P[j].Probe[PROBE_HW_ORBITAL].Failures++;
                }

                Data->P[j].Probe[PROBE_HW_ORBITAL].Used++;
                break;

            case Mission_Lunar_Probe:
                if (Data->P[j].History[i].spResult != 1) {
                    Data->P[j].Probe[PROBE_HW_LUNAR].Failures++;
                }

                Data->P[j].Probe[PROBE_HW_LUNAR].Used++;
                break;
            }

        }
    }

    return;
}

void UpdAll(char side)
{
    int i, k;
    char p0 = 0, p1 = 0;
    char tName[20];

    if (Data->P[side].DockingModuleInOrbit > 0) {
        Data->P[side].DockingModuleInOrbit--;
    }

    if (Data->P[side].AstroDelay != 0) {
        Data->P[side].AstroDelay -= 1;
    }

    for (i = 0; i < 3; i++) {
        memcpy(&Data->P[side].Mission[i], &Data->P[side].Future[i], sizeof(struct MissionType));
        memset(&Data->P[side].Future[i], 0x00, sizeof(struct MissionType));
        strcpy(Data->P[side].Future[i].Name, "UNDETERMINED");
    }

    for (i = 0; i < 3; i++) {
        if (Data->P[side].Mission[i].MissionCode == Mission_Orbital_Satellite) {
            Data->P[side].Mission[i].Patch = -1;
            strcpy(&tName[0], &Data->P[side].Probe[PROBE_HW_ORBITAL].Name[0]);
            strcat(&tName[0], " ");
            strcat(&tName[0], RomanNumeral(Data->P[side].Probe[PROBE_HW_ORBITAL].Code + 1).c_str());
            strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]);  // copy into struct
            Data->P[side].Probe[PROBE_HW_ORBITAL].Code++;  // Increase Planned Mission Count
        } else if (Data->P[side].Mission[i].MissionCode == Mission_Lunar_Probe) {
            Data->P[side].Mission[i].Patch = -1;
            strcpy(&tName[0], &Data->P[side].Probe[PROBE_HW_LUNAR].Name[0]);
            strcat(&tName[0], " ");
            strcat(&tName[0], RomanNumeral(Data->P[side].Probe[PROBE_HW_LUNAR].Code + 1).c_str());
            strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]);  // copy into struct
            Data->P[side].Probe[PROBE_HW_LUNAR].Code++;  // Increase Planned Mission Count
        } else if (Data->P[side].Mission[i].MissionCode == Mission_LunarFlyby ||
                   (Data->P[side].Mission[i].MissionCode >= Mission_VenusFlyby &&
                    Data->P[side].Mission[i].MissionCode <= Mission_SaturnFlyby)) {
            Data->P[side].Mission[i].Patch = -1;
            strcpy(&tName[0], &Data->P[side].Probe[PROBE_HW_INTERPLANETARY].Name[0]);
            strcat(&tName[0], " ");
            strcat(&tName[0], RomanNumeral(Data->P[side].Probe[PROBE_HW_INTERPLANETARY].Code + 1).c_str());
            strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]);  // copy into struct
            Data->P[side].Probe[PROBE_HW_INTERPLANETARY].Code++;  // Increase Planned Mission Count
        } else if (Data->P[side].Mission[i].MissionCode > 0) {
            if (Data->P[side].Mission[i].Joint == 0) {
                k = Data->P[side].Mission[i].Prog - 1;
                Data->P[side].Mission[i].Patch = Data->P[side].Manned[k].Code % 10;
                strcpy(&tName[0], &Data->P[side].Manned[k].Name[0]);
                strcat(&tName[0], " ");
                strcat(&tName[0], RomanNumeral(Data->P[side].Manned[k].Code + 1).c_str());
                strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]);  // copy into struct
                Data->P[side].Manned[k].Code++;  // Increase Planned Mission Count
            } else {
                if (Data->P[side].Mission[i].Prog == 0) {
                    k = Data->P[side].Mission[i + 1].Prog - 1;
                    Data->P[side].Mission[i].Patch = Data->P[side].Manned[k].Code % 10;
                    strcpy(&tName[0], &Data->P[side].Manned[k].Name[0]);
                    strcat(&tName[0], " ");
                    strcat(&tName[0], RomanNumeral(Data->P[side].Manned[k].Code + 1).c_str());
                    strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]);  // copy into struct
                } else {
                    k = Data->P[side].Mission[i].Prog - 1;
                    Data->P[side].Mission[i].Patch = Data->P[side].Manned[k].Code % 10;
                    strcpy(&tName[0], &Data->P[side].Manned[k].Name[0]);
                    strcat(&tName[0], " ");
                    strcat(&tName[0], RomanNumeral(Data->P[side].Manned[k].Code + 1).c_str());
                    strcpy(&Data->P[side].Mission[i].Name[0], &tName[0]); // copy into struct
                    Data->P[side].Manned[k].Code++;  // Increase Planned Mission Count
                }
            }
        }
    }

    // Reset R&D Purchasing Ability
    memset(Data->P[side].Buy, 0x00, sizeof(Data->P[side].Buy));

    AstroTurn();   /* Process all astronauts */

    Data->P[side].RD_Mods_For_Turn = 0;

    if (Data->P[side].RD_Mods_For_Year > 0) {
        Data->P[side].RD_Mods_For_Turn = Data->P[side].RD_Mods_For_Year;
        Data->P[side].RD_Mods_For_Year = 0;
    }

    Data->P[side].TurnOnly = Data->P[side].MissionCatastrophicFailureOnTurn = Data->P[side].Block = 0;


    if (side == 1) {
        p0 = p1 = 0;

        while (p0 < Data->P[0].PastMissionCount && p1 < Data->P[1].PastMissionCount) {
            if (Data->P[0].History[p0].MissionYear < Data->P[1].History[p1].MissionYear) {
                TestFMis(0, p0);
                p0++;
            } else if (Data->P[0].History[p0].MissionYear > Data->P[1].History[p1].MissionYear) {
                TestFMis(1, p1);
                p1++;
            } else if (Data->P[0].History[p0].MissionYear == Data->P[1].History[p1].MissionYear) {
                if (Data->P[0].History[p0].Month < Data->P[1].History[p1].Month) {
                    TestFMis(0, p0);
                    p0++;
                } else if (Data->P[0].History[p0].Month > Data->P[1].History[p1].Month) {
                    TestFMis(1, p1);
                    p1++;
                } else if (Data->P[0].History[p0].Month == Data->P[1].History[p1].Month) {
                    if (Data->P[0].Budget < Data->P[1].Budget && (p0 < Data->P[0].PastMissionCount)) {
                        TestFMis(0, p0);
                        p0++;
                    } else if (Data->P[0].Budget > Data->P[1].Budget && (p1 < Data->P[1].PastMissionCount)) {
                        TestFMis(1, p1);
                        p1++;
                    } else if ((p0 < Data->P[0].PastMissionCount) && (p1 < Data->P[1].PastMissionCount)) {
                        if (brandom(100) < 50) {
                            TestFMis(0, p0);
                            p0++;
                        } else {
                            TestFMis(1, p1);
                            p1++;
                        }
                    }
                }
            }
        }  // end while
    }

    memset(pNeg, 0x00, sizeof pNeg);

    if (side == 1) {
        while (p0 < Data->P[0].PastMissionCount) {
            TestFMis(0, p0);
            p0++;
        }

        while (p1 < Data->P[1].PastMissionCount) {
            TestFMis(1, p1);
            p1++;
        }
    }

    // Fix Prestige Values for Mars, Jup, Sat.
    Data->Prestige[Prestige_MarsFlyby].Goal[side] = 0;  // Clear Mars
    Data->Prestige[Prestige_JupiterFlyby].Goal[side] = 0;  // Clear Jupiter
    Data->Prestige[Prestige_SaturnFlyby].Goal[side] = 0;  // Clear Saturn
    Data->P[side].Probe[PROBE_HW_ORBITAL].Failures = Data->P[side].Probe[PROBE_HW_LUNAR].Failures = 0;
    Data->P[side].Probe[PROBE_HW_ORBITAL].Used = Data->P[side].Probe[PROBE_HW_LUNAR].Used = 0;

    for (i = 0; i < Data->P[side].PastMissionCount; i++) {
        if (Data->P[side].History[i].Event == 0) {
            switch (Data->P[side].History[i].MissionCode) {
            case Mission_MarsFlyby:
                Data->Prestige[Prestige_MarsFlyby].Goal[side]++;
                break;  // Mars

            case Mission_JupiterFlyby:
                Data->Prestige[Prestige_JupiterFlyby].Goal[side]++;
                break;  // Juptier

            case Mission_SaturnFlyby:
                Data->Prestige[Prestige_SaturnFlyby].Goal[side]++;
                break;  // Saturn

            default:
                break;
            }
        }

        switch (Data->P[side].History[i].MissionCode) {
        case Mission_Orbital_Satellite:
            if (Data->P[side].History[i].spResult != 1) {
                Data->P[side].Probe[PROBE_HW_ORBITAL].Failures++;
            }

            Data->P[side].Probe[PROBE_HW_ORBITAL].Used++;
            break;

        case Mission_Lunar_Probe:
            if (Data->P[side].History[i].spResult != 1) {
                Data->P[side].Probe[PROBE_HW_LUNAR].Failures++;
            }

            Data->P[side].Probe[PROBE_HW_LUNAR].Used++;
            break;
        }
    }

    return;
}


/**
 * Tests a Flyby mission to decide if it was successful.
 *
 * \param plr
 * \param i
 */
void TestFMis(int plr, int i)
{
    int misCode;

    if (Data->P[plr].History[i].Saf == 0) {
        return;
    }

    if (Data->P[plr].History[i].Event > 0) {
        Data->P[plr].History[i].Event--;

        if (brandom(100) > Data->P[plr].History[i].Saf) {
            /* Failed Mission */
            misCode = Data->P[plr].History[i].MissionCode;
            Data->P[plr].History[i].Event = Data->P[plr].History[i].Saf = 0;
            Data->P[plr].History[i].Prestige =
                PrestNeg(plr, (misCode == Mission_MarsFlyby) ? 4 : (misCode == Mission_JupiterFlyby) ? 5 : 6);
            Data->P[plr].Plans |= (misCode == Mission_MarsFlyby) ? 0x01 : (misCode == Mission_JupiterFlyby) ? 0x02 : 0x04;
            Data->P[plr].History[i].spResult = 5000;
        }

        if (Data->P[plr].History[i].Event == 0
            && Data->P[plr].History[i].Prestige == 0) {
            misCode = Data->P[plr].History[i].MissionCode;
            Data->P[plr].History[i].Prestige =
                Set_Goal(plr, (misCode == Mission_MarsFlyby) ? 4 : (misCode == Mission_JupiterFlyby) ? 5 : 6, 3);
            Data->P[plr].Plans |= (misCode == Mission_MarsFlyby) ? 0x10 : (misCode == Mission_JupiterFlyby) ? 0x20 : 0x40;
            Data->P[plr].History[i].spResult = 1;
            Data->P[plr].History[i].Saf = 0;
        }
    }
}

/** End of turn equipment accounting update
 */
void
UpdateHardTurn(char plr)
{
    BuzzData *p = &Data->P[plr];
    Equipment *e;

    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            e = &p->Probe[0];
        } else if (i == 1) {
            e = &p->Rocket[0];
        } else if (i == 2) {
            e = &p->Manned[0];
        } else if (i == 3) {
            e = &p->Misc[0];
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
