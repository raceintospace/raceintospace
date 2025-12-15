/**
 * Copyright (C) 2005 Michael K. McCarty & Fritz Bronner,
 * Copyright (c) 2020 Ryan Yoakum
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

// This file contains utiliity functions for astronaut / cosmonaut data

#include "astros.h"

#include "data.h"
#include "game_main.h"
#include "options.h"


/**
 * Count how many flight crews are available for mission assignment.
 *
 * \param plr  the player index.
 * \param program  the manned program index (per EquipMannedIndex).
 * \return  the number of assignable crews.
 */
int AvailableCrewsCount(int plr, int program)
{
    int count = 0;

    for (int i = 0; i < MAX_CREWS_IN_PROGRAM; i++) {
        const int commanderIndex = Data->P[plr].Crew[program][i][0];
        if (commanderIndex == 0) continue;
        
        const Astros& commander  = Data->P[plr].Pool[commanderIndex - 1];

        if (commander.Prime == 0 &&
            (options.feat_no_cTraining != 0 || commander.Moved == 0)) {
            count++;
        }
    }

    return count;
}


/**
 * Break all flight crews containing invalid astronauts/cosmonauts.
 *
 * \param player  the player index.
 */
void CheckFlightCrews(int player)
{
    for (int k = 0; k < ASTRONAUT_POOLS + 1; k++) {
        for (int j = 0; j < MAX_CREWS_IN_PROGRAM; j++) {
            int crew_count = Data->P[player].CrewCount[k][j];
            if (crew_count <= 0) continue;
            
            auto& crew = Data->P[player].Crew[k][j];

            bool invalid_crew = false;
            for (int i = 0; i < crew_count + 1; i++) {
                const auto& spaceman = Data->P[player].Pool[crew[i] - 1];
                if (spaceman.Status == AST_ST_DEAD ||
                    spaceman.Status == AST_ST_RETIRED ||
                    spaceman.Status == AST_ST_INJURED) {
                    invalid_crew = true;
                    break;
                }
            }
            if (!invalid_crew) continue;

            for (int i = 0; i < crew_count + 1; i++) {
                auto& spaceman = Data->P[player].Pool[crew[i] - 1];
                spaceman.oldAssign = spaceman.Assign;
                spaceman.Assign = 0;
                spaceman.Crew = 0;
                spaceman.Prime = 0;
                spaceman.Task = 0;
                spaceman.Moved = 0;

                if (spaceman.Special == 0) {
                    spaceman.Special = 6;
                }

                crew[i] = 0;
            }

            Data->P[player].CrewCount[k][j] = 0;
        }
    }
}


/*
 * Is an astronaut happy with their crewmate?
 *
 * Astronaut compatibility is not transitive. An astronaut may dislike
 * a crewmate who is fine with them.
 *
 * Astronauts / Cosmonauts have a compatibility score of 1-10. They
 * feel compatible with others who have a score within
 * [compat - cl, compat + cr], wrapping.
 *
 * \param ast  the astronaut / cosmonaut whose feelings these are.
 * \param peer  their companion astronaut / cosmonaut.
 * \return  true if ast works well with peer, false otherwise.
 */
bool Compatible(const struct Astros &ast, const struct Astros &peer)
{
    int low = ast.Compat - ast.CL;
    int high = ast.Compat + ast.CR;

    return ((peer.Compat >= low && peer.Compat <= high) ||
            (peer.Compat + 10 >= low && peer.Compat + 10 <= high) ||
            (peer.Compat - 10 >= low && peer.Compat - 10 <= high));
}
