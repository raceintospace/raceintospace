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

// This file handles Mission Control in the spaceport, for Downgrading and Rushing missions.

#include "rush.h"

#include <algorithm>
#include <stdexcept>
#include <string_view>
using namespace std::literals::string_view_literals;

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "downgrader.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "ioexception.h"
#include "logging.h"
#include "mission_util.h"
#include "pace.h"
#include "place.h"
#include "prest.h"
#include "state_utils.h"

// Function Prototype

char Mon[12][4] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};


namespace   // Unnamed namespace part 1
{

void DrawMissionEntry(char plr, int pad, const MissionType& mission);
void DrawRush(char plr);
void ResetRush(int mode, int pad);
void SetLaunchDates(char plr);
void SetRush(int mode, int pad);
void DrawPenaltyPopup(char plr, const MissionType& mission);
void DrawPenaltyPopup(char plr, const mStr& mission);

}; // End of Unnamed namespace part 1


/* Rules for mission downgrades
 * 1. Joint Mission *cannot* be downgraded to single launch missions,
 *    and vice versa.
 * 2. Unmanned missions *cannot* be downgraded to manned missions.
 * 3. There *must not* be downgrading to, or from, a Probe Mission.
 *    (Exception: a Lunar Probe Landing should be downgradable to a
 *     Lunar Flyby.)
 * 4. Mission hardware requirements *must not* be added.
 * 5. Mission hardware requirements *should not* be removed.
 * 6. Manned mission downgrades *should* be listed ahead of unmanned
 *    alternatives.
 *
 * NOTE: Rule 5 will probably be broken at times, but it results in
 * the extra hardware being inaccessible until the end of the turn
 * (possibly consumed by the mission, no guarantee).
 */


/* Replaces the launch scheduled for the end of the turn with a
 * different mission at the same time.
 *
 * Attempts to replace a Joint mission with a single launch mission,
 * or vice versa, are not allowed and will be ignored.
 * Replacing an Unmanned launch with a Manned launch is also blocked.
 *
 * TODO: Downgrade penalty system is currently disabled.
 * Add it with a configuration toggle.
 *
 * TODO: Compare hardware requirements and release any hardware
 * not in the downgraded version of the mission.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \param pad  0 <= pad < MAX_LAUNCHPADS.
 * \param mission  The replacement mission.
 * \throws invalid_argument  if pad < 0 or >= MAX_LAUNCHPADS.
 * \throws logic_error  if a joint mission part 0 is on the last pad.
 */
void Downgrade(const char plr, const int pad,
               const MissionType& mission)
{
    auto& pData = Data->P[plr];
    auto& pad_mission = pData.Mission[pad];
    // Make sure there's no memory access problems.
    if (pad < 0 || pad >= MAX_LAUNCHPADS) {
        throw std::invalid_argument(
            "Launch pad value must be between 0 and MAX_LAUNCHPADS");
    } else if (pad_mission.Joint == 1 && pad_mission.part == 0
               && pad + 1 >= MAX_LAUNCHPADS) {
        throw std::logic_error(
            "Cannot have the first part of a Joint mission on the last"
            "launchpad.");
    }

    // Block downgrades for Joint <--> Single, Unmanned --> Manned
    if (pad_mission.Joint != mission.Joint) {
        CAT_ERROR(baris,
                "Downgrade attempt to change a mission's Joint status"
                " on pad %d", pad);
        return;
    } else if (pad_mission.Men == 0 && mission.Men > 0) {
        CAT_ERROR(baris,
                "Downgrade attempt to change a mission from Unmanned"
                " to Manned on pad %d", pad);
        return;
    } else if (mission.MissionCode == Mission_None) {
        // TODO: Should launch a prompt before scrubbing a mission,
        // but cancelling means this function cannot fulfill its
        // mandate, and the decision of how to handle that must be
        // dealt with at a higher level.
        ScrubMission(plr, pad);
        return;
    }

    // On a manned Joint mission, there is always crew on the second
    // part. Remove them, if relevant, first.
    if (pad_mission.Joint == 1
        && pad_mission.part == 0
        && pData.Mission[pad + 1].Men > 0) {
        bool manned = true;

        try {
            mStr type = GetMissionPlan(mission.MissionCode);
            // mCrew == 5 means Unmanned Joint mission
            manned = (type.mCrew == 5) ? false : true;
        } catch (IOException &err) {
            CAT_CRITICAL(baris,
                       "Unable to read mission information from file,"
                       " cancelling downgrade on pad %d: %s",
                       pad, err.what());
            return;
        }

        if (! manned) {
            pData.Mission[pad + 1].Men = 0;
            pData.Mission[pad + 1].Crew = 0;
            ClearMissionCrew(plr, pad + 1, CREW_ALL);
        }
    }

    // If the new mission is unmanned, free up the crew...
    if (pad_mission.Men > 0 && pad_mission.Men == 0) {
        pad_mission.Crew = 0;
        ClearMissionCrew(plr, pad, CREW_ALL);
    }

    pad_mission = mission;

    if (pad_mission.Joint == 1 && pad_mission.part == 0) {
        pData.Mission[pad + 1].MissionCode = mission.MissionCode;
    }

    // Need to mark mission to show it is downgraded, but this is
    // a bad system that will need to be replaced.
    // Setting a global var will not survive an autosave load.
    // pNeg[plr][pad] = 1;
    // pad_mission.Name[24] = 1;
}


namespace   // Unnamed namespace part 2
{

/* Summarize the given mission and its relation to the original mission
 * in the specified pad slot.
 *
 * \param plr  The player index
 * \param pad  The index of the launch pad for the mission (0, 1, or 2).
 * \param mission
 */
void DrawMissionEntry(const char plr, const int pad,
                      const MissionType& mission)
{
    auto& pData = Data->P[plr];
    fill_rectangle(144, 29 + pad * 58, 270, 37 + pad * 58, 3);
    fill_rectangle(93, 43 + pad * 58, 262, 57 + pad * 58, 3);
    display::graphics.setForegroundColor(5);
    mStr plan = GetMissionPlan(mission.MissionCode);
    draw_string(96, 48 + 58 * pad, (plan.Abbr).c_str());

    if (plan.Dur >= 1) {
        int duration = mission.Duration;
        plan.Days = duration;
        draw_string(0, 0, GetDurationParens(duration));
    }

    display::graphics.setForegroundColor(9);

    // The prestige penalty to downgrading is currently disabled.
    // Marking the last character in MissionType.Name is a bad
    // system.
    if (pData.Mission[pad].MissionCode != Mission_None &&
        mission.MissionCode == Mission_None) {
        display::graphics.setForegroundColor(9);
        draw_string(145, 33 + pad * 58, "SCRUBBED MISSION");
        // draw_string(193, 77 + pad * 58, "NO PENALTY");
    } else if (! Equals(pData.Mission[pad], mission)) {
        // } else if (pData.Mission[pad].Name[24] == 1 ||
        //            ! Equals(pData.Mission[pad], mission)) {
        display::graphics.setForegroundColor(9);
        draw_string(145, 33 + pad * 58, "DOWNGRADED MISSION");
        // draw_string(193, 77 + pad * 58, "-3 PRESTIGE");
    } else {
        display::graphics.setForegroundColor(7);
        draw_string(145, 33 + pad * 58, "ORIGINAL MISSION");
        // draw_string(193, 77 + pad * 58, "NO PENALTY");
    }

    // draw_string(88, 77 + pad * 58, "REQUIREMENT PENALTIES: ");
    fill_rectangle(215, 71 + pad * 58, 270, 78 + pad * 58, 3);
    const int penalty = AchievementPenalty(plr, plan);
    display::graphics.setForegroundColor(16);

    if (penalty > 2) {
        display::graphics.setForegroundColor(11);
    }

    if (penalty > 9) {
        display::graphics.setForegroundColor(9);
    }

    draw_string(88, 77 + pad * 58, "REQUIREMENT PENALTIES:");
    draw_number(215, 77 + pad * 58, penalty);
    draw_string(0, 0, "%");
}


/* Draw the Mission Control facility display and the missions planned
 * for launch this turn.
 *
 * \param plr  The player's country (0 for USA, 1 for the USSR).
 */
void DrawRush(char plr)
{
    auto& pData = Data->P[plr];
    FadeOut(2, 10, 0, 0);

    boost::shared_ptr<display::PalettizedSurface> launchPads(Filesystem::readImage("images/lpads.but.1.png"));
    launchPads->exportPalette();

    display::graphics.screen()->clear();

    ShBox(0, 0, 319, 23);
    IOBox(243, 3, 316, 19);
    InBox(3, 3, 30, 19);
    display::graphics.setForegroundColor(1);
    draw_string(263, 13, "ASSIGN");
    draw_small_flag(plr, 4, 4);

    for (int i = 0; i < 3; i++) {
        auto& mission = pData.Mission[i];
        if (mission.MissionCode == Mission_None) continue;
        if (mission.part != 0) continue;
        
        mStr plan = GetMissionPlan(mission.MissionCode);

        ShBox(0, 25 + i * 58, 80, 82 + i * 58 - 1);
        ShBox(83, 25 + i * 58, 319, 82 + i * 58 - 1);

        IOBox(278, 30 + i * 58, 314, 42 + i * 58);
        IOBox(278, 47 + i * 58, 314, 59 + i * 58);
        IOBox(278, 64 + i * 58, 314, 76 + i * 58);

        IOBox(89, 39 + i * 58, 266, 61 + i * 58);
        display::graphics.setForegroundColor(1);
        draw_string(89, 33 + i * 58, "SCHEDULE:");
        draw_string(88, 69 + i * 58, "RUSHING PENALTY: ");
        // draw_string(88, 77 + i * 58, "DOWNGRADE PENALTY: ");
        draw_string(199, 69 + i * 58, "COST:");
        OutBox(11 , 33 + i * 58, 69, 74 + i * 58);
        InBox(20, 38 + i * 58, 60, 69 + i * 58);
        display::graphics.screen()->draw(
            launchPads, 156 * plr, i * 30, 39, 30, 21, 39 + i * 58);

        SetRush(mission.Rushing, i);
        display::graphics.setForegroundColor(1);
        draw_heading(55, 5, "MISSION SCHEDULE", 0, -1);

        display::graphics.setForegroundColor(5);
        draw_string(96, 48 + 58 * i, (plan.Abbr).c_str());

        // Show duration level only on missions with a Duration step -Leon
        if (IsDuration(mission.MissionCode)) {
            int duration = mission.Duration;
            plan.Days = duration;
            draw_string(0, 0, GetDurationParens(duration));
        }

        if (mission.Name[24] == 1) {
            display::graphics.setForegroundColor(9);
            draw_string(145, 33 + i * 58, "DOWNGRADED MISSION");
            // draw_string(193, 77 + i * 58, "-3 PRESTIGE");
        } else {
            display::graphics.setForegroundColor(7);
            draw_string(145, 33 + i * 58, "ORIGINAL MISSION");
            // draw_string(193, 77 + i * 58, "NO PENALTY");
        }

        display::graphics.setForegroundColor(11);
        draw_string(288, 38 + 58 * i,
                    &Mon[mission.Month - 0][0]);
        draw_string(288, 55 + 58 * i,
                    &Mon[mission.Month - 1][0]);
        draw_string(288, 72 + 58 * i,
                    &Mon[mission.Month - 2][0]);

        // Since the Downgrade penalty isn't being used, its
        // screen space is commandeered to display the mission
        // penalty.
        const int penalty = AchievementPenalty(plr, plan);
        display::graphics.setForegroundColor(16);

        if (penalty > 2) {
            display::graphics.setForegroundColor(11);
        }

        if (penalty > 9) {
            display::graphics.setForegroundColor(9);
        }

        draw_string(88, 77 + i * 58, "REQUIREMENT PENALTIES:");
        display::graphics.setForegroundColor(16);

        if (penalty > 2) {
            display::graphics.setForegroundColor(11);
        }

        if (penalty > 9) {
            display::graphics.setForegroundColor(9);
        }

        draw_number(215, 77 + i * 58, penalty);
        draw_string(0, 0, "%");
    }
}


}; // End of Unnamed namespace part 2

/**
 * Initialize and run the Mission Control state which handles mission
 * rushing and downgrading.
 *
 * TODO: Support downgrading from Manned missions to Unmanned missions.
 *
 * \param plr
 */
void Rush(char plr)
{
    auto& pData = Data->P[plr];

    // Reset Rushing status for missions.
    for (int pad = 0; pad < 3; pad++) {
        if (pData.Mission[pad].Rushing == 1) {
            pData.Cash += 3;
        } else if (pData.Mission[pad].Rushing == 2) {
            pData.Cash += 6;
        }

        pData.Mission[pad].Rushing = 0;  // Clear Data
    }

    SetLaunchDates(plr);

    Downgrader::Options downgrades;

    try {
        downgrades = LoadJsonDowngrades("DOWNGRADES.JSON");
    } catch (IOException &err) {
        CAT_CRITICAL(baris, err.what());
    }

    Downgrader downgradeList[3] = {
        Downgrader(pData.Mission[0], downgrades),
        Downgrader(pData.Mission[1], downgrades),
        Downgrader(pData.Mission[2], downgrades)
    };

    DrawRush(plr);
    int fCsh = pData.Cash;
    bool pRush = (pData.Cash >= 3);
    display::graphics.setForegroundColor(1);

    music_start((plr == 0) ? M_USMIL : M_USSRMIL);
    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();

    int R[3]{};
    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons == 0 && key == 0) continue;
    
        for (int pad=0; pad < 3; ++pad) { // rush
            if (! pRush) break;
            if (pData.Mission[pad].MissionCode == Mission_None) continue;
            if (pData.Mission[pad].part == 1) continue;

            if ((x >= 280 && x <= 312 && y >= 32+pad*58 && y <= 74+pad*58 && mousebuttons > 0)
                || (key >= '1'+pad*3 && key <= '3'+pad*3)) {

                for (int rush_level=0; rush_level < 3; ++rush_level) {
                    if ((y >= 32+rush_level*17+pad*58 && y <= 57+rush_level*17+pad*58 && mousebuttons > 0)
                        || key == '1'+rush_level+pad*3) {

                        if (fCsh < 3*(rush_level - R[pad]) {
                            Help("i117");
                            break;
                        }

                        if (R[pad] == rush_level) break;

                        ResetRush(R[pad], pad);
                        SetRush(rush_level, pad);
                        fCsh -= (rush_level - R[pad]) * 3;
                        R[pad] = rush_level;
                    }
                }
            }
        }

        for (int pad = 0; pad < 3; ++pad) { // Penalty explainer
            if (pData.Mission[pad].MissionCode != Mission_None) continue;
            if (pData.Mission[pad].part == 1) continue;

            if (x >= 20 && x <= 60 && y >= 38+pad*58 && y <= 69+pad*58 && mousebuttons > 0) {
                OutBox(20, 38+pad*58, 60, 69+pad*58);
                delay(150);
                InBox(20, 38+pad*58, 60, 69+pad*58);
                DrawPenaltyPopup(plr, downgradeList[pad].current());
            }
        }

        std::string_view keyboard_shortcuts = "QRU"sv;
        for (int pad=0; pad < 3; ++pad) { // downgrade
            if (pData.Mission[pad].MissionCode == Mission_None) continue;
            if (pData.Mission[i].part == 1) continue;

            if ((x >= 91 && x <= 264 && y >= 41 + pad * 58 && y <= 59 + pad * 58 && mousebuttons > 0)
                || key == keyboard_shortcuts[pad]) {
                InBox(91, 41 + pad * 58, 264, 59 + pad * 58);

                DrawMissionEntry(plr, pad, downgradeList[pad].next());

                WaitForMouseUp();
                OutBox(91, 41 + pad * 58, 264, 59 + pad * 58);
            }
        }

        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || (key == K_ENTER || key == K_ESCAPE)) {  // CONTINUE
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);
            delay(10);

            for (int pad = 0; pad < 3; pad++) {
                if (pData.Mission[pad].MissionCode == Mission_None) continue;
                if (pData.Mission[pad].part == 1) continue; 
                if (Equals(pData.Mission[pad], downgradeList[pad].current())) continue;
                
                Downgrade(plr, pad, downgradeList[pad].current());
            } 

            for (int sec_pad = 1; sec_pad < 3; ++sec_pad) {
                if (pData.Mission[sec_pad].part == 1) {
                    R[sec_pad] = R[sec_pad - 1];
                } 
            } 

            pData.Cash = fCsh;
            for (int pad=0; pad < 3; ++pad) {
                if (pData.Mission[pad].MissionCode == Mission_None) continue;
                
                pData.Mission[pad].Month -= R[pad];
                pData.Mission[pad].Rushing = R[pad];
            } 
    
            music_stop();
            return;  // Done
        }
    }
}


namespace   // Unnamed namespace part 3
{

/* Unsets the old launch date on the Mission Control display.
 *
 * Because mission settings are not modified until leaving the Mission
 * Control center, this only updates the display.
 * Updating the display to reflect the new setting is handled by SetRush.
 *
 * \param mode  How many months the mission was being rushed (0, 1, or 2).
 * \param pad   The index of the launch pad for the mission (0, 1, or 2).
 */
void ResetRush(const int mode, const int pad)
{
    OutBox(280, 32 + 17 * mode + pad * 58, 312, 40 + 17 * mode + pad * 58);
}


/* Schedule the default, unrushed launch dates for the planned missions.
 *
 * \param plr  The index of the player launching the missions.
 */
void SetLaunchDates(const char plr)
{
    auto& pData = Data->P[plr];
    int missionCount = std::count_if(pData.Mission, pData.Mission+MAX_MISSIONS, [](auto& mission){return mission.MissionCode != Mission_None
                                                                                                         && mission.part == 0;});

    // Currently, can only handle 3 missions.
    // assert(MAX_MISSIONS == 3);

    int launch_months[3][3] = {
        {4},
        {3,5},
        {2,3,4},
    };
    int launch = -1;
    for (int pad = 0; pad < MAX_MISSIONS; ++pad){
        auto& mission = pData.Mission[pad];
        if (mission.MissionCode == Mission_None) continue;
        if (mission.part == 0) ++launch;
        mission.Month = launch_months[missionCount][launch] + Data->Season*6;
    }
}


/* Update the Mission Control display to show the new Rush status
 * of the mission.
 *
 * \param pad  The index of the launch pad for the mission (0, 1, or 2).
 * \param mode How many months the mission will be rushed (0, 1, or 2).
 */
void SetRush(int mode, int pad)
{
    InBox(280, 32 + 17 * mode + pad * 58, 312, 40 + 17 * mode + pad * 58);
    fill_rectangle(177, 63 + 58 * pad, 192, 70 + 58 * pad, 3);
    fill_rectangle(225, 62 + 58 * pad, 270, 70 + 58 * pad, 3);
    display::graphics.setForegroundColor(11);
    draw_number(179, 69 + 58 * pad, mode * 3);
    draw_character('%');
    display::graphics.setForegroundColor(9);
    draw_number(230, 69 + 58 * pad, mode * 3);
    display::graphics.setForegroundColor(1);
    draw_string(237, 69 + 58 * pad, "MB");
}


void DrawPenaltyPopup(char plr, const MissionType& mission)
{
    mStr plan = GetMissionPlan(mission.MissionCode);

    if (plan.Dur) {
        plan.Days = mission.Duration;
    }

    DrawPenaltyPopup(plr, plan);
}


void DrawPenaltyPopup(char plr, const mStr& mission)
{
    auto& pData = Data->P[plr];
    int milestonePenalty = MilestonePenalty(plr, mission);
    int durationPenalty = DurationPenalty(plr, mission);
    int newMissionPenalty = NewMissionPenalty(plr, mission);

    display::LegacySurface local(165, 124);
    local.copyFrom(display::graphics.legacyScreen(), 85, 52, 249, 175);

    ShBox(85, 68, 249, 151);
    InBox(92, 74, 243, 120);
    display::graphics.setForegroundColor(11);

    if (milestonePenalty + durationPenalty > 2) {
        display::graphics.setForegroundColor(11);
    }

    if (milestonePenalty + durationPenalty > 9) {
        display::graphics.setForegroundColor(9);
    }

    draw_string(99, 81, "REQUIREMENT PENALTIES:");

    display::graphics.setForegroundColor(1);
    draw_string(99, 92, "MILESTONE PENALT");

    if (milestonePenalty == 3) {
        draw_string(0, 0, "Y");
    } else {
        draw_string(0, 0, "IES");
    }

    draw_string(220, 92, "-");

    if (milestonePenalty > 0) {
        draw_number(0, 0, milestonePenalty);
    } else {
        draw_string(0, 0, "-");
    }

    draw_string(99, 100, "DURATION PENALT");

    if (durationPenalty == 5) {
        draw_string(0, 0, "Y");
    } else {
        draw_string(0, 0, "IES");
    }

    draw_string(220, 100, "-");

    if (durationPenalty > 0) {
        draw_number(0, 0, durationPenalty);
    } else {
        draw_string(0, 0, "-");
    }

    draw_string(99, 108, "NEW MISSION PENALTY");
    draw_string(220, 108, "-");

    if (newMissionPenalty > 0) {
        draw_number(226, 108, newMissionPenalty);
    } else {
        draw_string(0, 0, "-");
    }

    if (mission.LM && IsLunarLanding(mission.Index)) {
        int lunarTestPenalty = 3 * MIN(pData.LMpts - 3, 0);

        display::graphics.setForegroundColor(24);
        draw_string(99, 116, "PENALTY ON LM STEPS");

        if (lunarTestPenalty >= 0) {
            draw_string(220, 116, "--");
        } else {
            draw_number(220, 116, lunarTestPenalty);
        }
    }

    IOBox(91, 127, 243, 148);
    display::graphics.setForegroundColor(5);
    draw_heading(123, 131, "CONTINUE", 0, -2);

    while (true) {
        key = 0;
        GetMouse();

        if ((x >= 92 && y >= 126 && x <= 242 && y <= 148 && mousebuttons > 0)
            || (key == K_ENTER || key == K_ESCAPE)) {
            InBox(91, 127, 243, 148);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
                key = 0;
            }

            OutBox(91, 127, 243, 148);
            delay(50);
            break;
        }
    }

    display::graphics.screen()->draw(local, 85, 52);
}

}; // End of unnamed namespace part 3
