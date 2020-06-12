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

// This file handles canceling missions.

#include "state_utils.h"

#include <stdexcept>
#include <cstdio>

#include "Buzz_inc.h"
#include "crew.h"
#include "game_main.h"
#include "mission_util.h"


namespace
{
void RemoveCrew(char plr, struct MissionType &mission, CrewType crew);
void ResetMission(struct MissionType &mission);
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


/**
 * Reschedules a mission planned for launch this turn for next season.
 *
 * Moves a scheduled mission from the Mission pads to the corresponding
 * Future pads. Any scheduled FMs using those pads are cancelled.
 * This does not provide prompts to check if the delay should proceed;
 * before delaying, you may wish to query:
 *
 *   - if there is a future mission already on this pad.
 *   - if the mission can be scheduled this turn. Certain missions
 *     (interplanetary probes) have limited launch windows and
 *     cannot be scheduled during every season or year.
 *   - if a member of the crew is slated to retire. They may be
 *     available this turn, but might not be the next!
 *
 * \param plr  player index
 * \param pad  the index of the launch pad for the mission (0, 1, or 2).
 * \throws invalid_argument  if pad < 0 or pad >= MAX_LAUNCHPADS.
 */
void DelayMission(const char plr, int pad)
{
    /**
     * 1) Check to see if the mission can be rescheduled for the upcoming
     *    launch time.
     *   - TODO Are there 1/2 (consecutive) pads open in Future Missions?
     *   - TODO Can the mission be delayed to another pad?
     *   - Can the mission be scheduled in that time slot (Ex: Jup. Flyby)?
     * 2) Assign mission to Future missions
     * 3) Update Astronauts to reflect delay
     * 4) Free up any associated hardware
     * 5) Unassign mission from current missions.
     */
    if (pad < 0 || pad >= MAX_LAUNCHPADS) {
        char buffer[70];
        snprintf(buffer, 69, "DelayMission argument pad=%d must be "
                 "0 <= pad < %d", pad, MAX_LAUNCHPADS);
        throw std::invalid_argument(buffer);
    }

    // You can't just delay interplanetary probes! There are strict
    // launch windows!
    if (! MissionTimingOk(Data->P[plr].Mission[pad].MissionCode,
                          Data->Year, Data->Season)) {
        char buffer[150];
        snprintf(buffer, 149,
                 "DelayMission cannot be called for mission code=%d"
                 " on pad %d: That mission cannot be scheduled for "
                 " next turn.",
                 Data->P[plr].Mission[pad].MissionCode, pad);
        throw std::logic_error(buffer);
    }

    if (Data->P[plr].Mission[pad].Joint &&
        Data->P[plr].Mission[pad].part == 1) {
        pad--;
    }

    // Must cache Joint value b/c Mission[pad] gets reset in loop.
    int padsUsed = Data->P[plr].Mission[pad].Joint + 1;

    for (int i = pad; i < pad + padsUsed; i++) {
        ClrFut(plr, i);
        Data->P[plr].Future[i] = Data->P[plr].Mission[i];

        const int program = Data->P[plr].Future[i].Prog;
        const int pCrew = Data->P[plr].Future[i].PCrew;
        const int bCrew = Data->P[plr].Future[i].BCrew;

        for (int j = 0; j < Data->P[plr].Future[i].Men; j++) {
            int poolIndex;

            if (pCrew > 0) {
                poolIndex = Data->P[plr].Crew[program][pCrew - 1][j] - 1;
                Data->P[plr].Pool[poolIndex].Prime++;
            }

            if (bCrew > 0) {
                poolIndex = Data->P[plr].Crew[program][bCrew - 1][j] - 1;
                Data->P[plr].Pool[poolIndex].Prime++;
            }
        }

        FreeLaunchHardware(plr, i);
        ResetMission(Data->P[plr].Mission[i]);
    }
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


/**
 * Scrubs a mission assigned for the current turn.
 *
 * Clears all mission data, frees hardware, and unassigns the crew.
 *
 * \param plr  The player index (0 for USA, 1 for USSR).
 * \param pad  The launch pad index (pad 0, 1, or 2)
 */
void ScrubMission(const char plr, int pad)
{
    if (pad < 0 || pad >= MAX_LAUNCHPADS) {
        char buffer[70];
        sprintf(buffer, "ScrubMission argument pad=%d"
                "must be 0 <= pad < %d", pad, MAX_LAUNCHPADS);
        throw std::invalid_argument(buffer);
    }

    if (Data->P[plr].Mission[pad].Joint &&
        Data->P[plr].Mission[pad].part == 1) {
        pad--;
    }

    if (Data->P[plr].Mission[pad].Joint == 1) {
        FreeLaunchHardware(plr, pad + 1);
        ClearMissionCrew(plr, pad + 1, CREW_ALL);
        ResetMission(Data->P[plr].Mission[pad + 1]);
    }

    FreeLaunchHardware(plr, pad);
    ClearMissionCrew(plr, pad, CREW_ALL);
    ResetMission(Data->P[plr].Mission[pad]);
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


/* Clears a mission entry to a blank state.
 *
 * Resets a mission entry without side effects. This does not
 * actively release any resources assigned to the mission, such
 * as hardware or crew, or address the other half of a joint
 * mission.
 *
 * \param misssion  The entry to zero out.
 */
void ResetMission(struct MissionType &mission)
{
    memset(&mission.Name[0], 0x00, sizeof(mission.Name));
    mission.MissionCode = Mission_None;
    mission.Patch = 0;
    mission.part = 0;
    memset(&mission.Hard[0], 0x00, sizeof(mission.Hard));
    mission.Joint = 0;
    mission.Rushing = 0;
    mission.Month = 0;
    mission.Duration = 0;
    mission.Men = 0;
    mission.Prog = 0;
    mission.PCrew = 0;
    mission.BCrew = 0;
    mission.Crew = 0;
}

}; // End of local namespace
