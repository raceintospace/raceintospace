/* Copyright (C) 2019 Race Into Space
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "state_utils.h"

#include <stdexcept>
#include <cstdio>

#include "Buzz_inc.h"
#include "game_main.h"


namespace
{
void RemoveCrew(char plr, struct MissionType &mission, CrewType crew);
};


//----------------------------------------------------------------------
// Header function definitions
//----------------------------------------------------------------------

/**
 * Removes crew from a Future mission and leaves them available for
 * new mission assignments.
 *
 * This synchronizes delisting the crew from the mission and setting
 * the crew's status to be open. Routing crew access through this
 * function manages access to global values.
 *
 * \throws invalid_argument  if pad < 0 or pad >= MAX_MISSIONS.
 */
void ClearFutureCrew(const char plr, const int pad, const CrewType crew)
{
    if (pad < 0 || pad >= MAX_MISSIONS) {
        char buffer[70];
        sprintf(buffer, "ClearFutureCrew argument pad=%d, "
                "must be 0 <= pad < %d", pad, MAX_MISSIONS);
        throw std::invalid_argument(buffer);
    }

    RemoveCrew(plr, Data->P[plr].Future[pad], crew);
}


/**
 * Removes crew from a prepped mission and leaves them available for
 * new mission assignments.
 *
 * This synchronizes delisting the crew from the mission and setting
 * the crew's status to be open. Routing crew access through this
 * function manages access to global values.
 *
 * \throws invalid_argument  if pad < 0 or pad >= MAX_MISSIONS.
 */
void ClearMissionCrew(const char plr, const int pad, const CrewType crew)
{
    if (pad < 0 || pad >= MAX_MISSIONS) {
        char buffer[70];
        sprintf(buffer, "ClearFutureCrew argument pad=%d, "
                "must be 0 <= pad < %d", pad, MAX_MISSIONS);
        throw std::invalid_argument(buffer);
    }

    RemoveCrew(plr, Data->P[plr].Mission[pad], crew);
}


/* Frees up hardware that was assigned to a specific mission launch.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \param pad  The index of the launch (pad 0, 1, or 2).
 * \throws invalid_argument  if pad < 0 or pad >= MAX_LAUNCHPADS
 */
void FreeLaunchHardware(const char plr, const int pad)
{
    if (pad < 0 || pad >= MAX_LAUNCHPADS) {
        char buffer[70];
        sprintf(buffer, "FreeLaunchHardware argument pad=%d"
                "must be 0 <= pad < %d", pad, MAX_LAUNCHPADS);
        throw std::invalid_argument(buffer);
    }

    struct MissionType &launch = Data->P[plr].Mission[pad];

    if (launch.Hard[Mission_PrimaryBooster] <= 0) {
        return;
    }

    for (int i = Mission_Capsule; i <= Mission_Probe_DM; i++) {
        const uint8_t hardwareID = launch.Hard[i];

        switch (i) {
        case Mission_Capsule:
        case Mission_LM:  // Manned+LM
            Data->P[plr].Manned[hardwareID].Spok--;
            break;

        case Mission_Kicker:  // Kicker
            Data->P[plr].Misc[hardwareID].Spok--;
            break;

        case Mission_Probe_DM:  // DM+Probes
            if (hardwareID == MISC_HW_DOCKING_MODULE) {
                Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Spok--;
            } else {
                Data->P[plr].Probe[hardwareID].Spok--;
            }

            break;
        }
    }

    const uint8_t rocket = (launch.Hard[Mission_PrimaryBooster] - 1) % 4;
    Data->P[plr].Rocket[rocket].Spok--;

    if (launch.Hard[Mission_PrimaryBooster] > 3) {
        Data->P[plr].Rocket[ROCKET_HW_BOOSTERS].Spok--;
    }
}


//----------------------------------------------------------------------
// Local definitions
//----------------------------------------------------------------------

namespace   // Start of local namespace
{

/**
 */
void RemoveCrew(const char plr, struct MissionType &mission,
                const CrewType crew)
{
    char prog = mission.Prog;
    char men = mission.Men;

    if ((crew == CREW_PRIMARY || crew == CREW_ALL) && mission.PCrew > 0) {
        char prime = mission.PCrew - 1;

        for (int i = 0; i < men; i++) {
            Data->P[plr].Pool[
                Data->P[plr].Crew[prog][prime][i] - 1].Prime = 0;
        }

        mission.PCrew = 0;
        mission.Crew = (mission.BCrew > 0) ? mission.BCrew : 0;
    }

    if ((crew == CREW_BACKUP || crew == CREW_ALL) && mission.BCrew > 0) {
        char backup = mission.BCrew - 1;

        for (int i = 0; i < men; i++) {
            Data->P[plr].Pool[
                Data->P[plr].Crew[prog][backup][i] - 1].Prime = 0;
        }

        mission.BCrew = 0;
        // If Crew is set, should already be at Primary crew if they
        // are availble.
        // mission.Crew = (mission.PCrew > 0) ? mission.PCrew : 0;
    }

    // This makes logical sense, but would need better control over
    // mission fields.
    // if (mission.PCrew <= 0 && mission.BCrew <= 0) {
    //     mission.Crew = 0;
    //     mission.Men = 0;
    // }
}

}; // End of local namespace
