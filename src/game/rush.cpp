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

// This file handles Downgrading and Rushing missions.

#include <stdexcept>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "rush.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "downgrader.h"
#include "draw.h"
#include "ioexception.h"
#include "place.h"
#include "logging.h"
#include "mc.h"
#include "mission_util.h"
#include "sdlhelper.h"
#include "state_utils.h"
#include "gr.h"
#include "pace.h"
#include "prest.h"
#include "filesystem.h"

// Function Prototype

char Mon[12][4] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};


namespace   // Unnamed namespace part 1
{

void DrawMissionEntry(char plr, int pad, const struct MissionType &mission);
void DrawRush(char plr);
void ResetRush(int mode, int pad);
void SetLaunchDates(char plr);
void SetRush(int mode, int pad);

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
               const struct MissionType &mission)
{
    // Make sure there's no memory access problems.
    if (pad < 0 || pad >= MAX_LAUNCHPADS) {
        throw std::invalid_argument(
            "Launch pad value must be between 0 and MAX_LAUNCHPADS");
    } else if (Data->P[plr].Mission[pad].Joint == 1 &&
               Data->P[plr].Mission[pad].part == 0 &&
               pad + 1 >= MAX_LAUNCHPADS) {
        throw std::logic_error(
            "Cannot have the first part of a Joint mission on the last"
            "launchpad.");
    }

    // Block downgrades for Joint <--> Single, Unmanned --> Manned
    if (Data->P[plr].Mission[pad].Joint != mission.Joint) {
        CERROR3(baris,
                "Downgrade attempt to change a mission's Joint status"
                " on pad %d", pad);
        return;
    } else if (Data->P[plr].Mission[pad].Men == 0 && mission.Men > 0) {
        CERROR3(baris,
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
    if (Data->P[plr].Mission[pad].Joint == 1 &&
        Data->P[plr].Mission[pad].part == 0 &&
        Data->P[plr].Mission[pad + 1].Men > 0) {
        bool manned = true;

        try {
            struct mStr type = GetMissionPlan(mission.MissionCode);
            // mCrew == 5 means Unmanned Joint mission
            manned = (type.mCrew == 5) ? false : true;
        } catch (IOException &err) {
            CCRITICAL4(baris,
                       "Unable to read mission information from file,"
                       " cancelling downgrade on pad %d: %s",
                       pad, err.what());
            return;
        }

        if (! manned) {
            Data->P[plr].Mission[pad + 1].Men = 0;
            Data->P[plr].Mission[pad + 1].Crew = 0;
            ClearMissionCrew(plr, pad + 1, CREW_ALL);
        }
    }

    // If the new mission is unmanned, free up the crew...
    if (Data->P[plr].Mission[pad].Men > 0 && mission.Men == 0) {
        Data->P[plr].Mission[pad].Crew = 0;
        ClearMissionCrew(plr, pad, CREW_ALL);
    }

    Data->P[plr].Mission[pad] = mission;

    if (Data->P[plr].Mission[pad].Joint == 1 &&
        Data->P[plr].Mission[pad].part == 0) {
        Data->P[plr].Mission[pad + 1].MissionCode = mission.MissionCode;
    }

    // Need to mark mission to show it is downgraded, but this is
    // a bad system that will need to be replaced.
    // Setting a global var will not survive an autosave load.
    // pNeg[plr][pad] = 1;
    // Data->P[plr].Mission[pad].Name[24] = 1;
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
                      const struct MissionType &mission)
{
    fill_rectangle(144, 29 + pad * 58, 270, 37 + pad * 58, 3);
    fill_rectangle(93, 43 + pad * 58, 262, 57 + pad * 58, 3);
    display::graphics.setForegroundColor(5);
    GetMisType(mission.MissionCode);
    draw_string(96, 48 + 58 * pad, Mis.Abbr);

    if (Mis.Dur >= 1) {
        int duration = mission.Duration;
        draw_string(0, 0, GetDurationParens(duration));
    }

    display::graphics.setForegroundColor(9);

    // The prestige penalty to downgrading is currently disabled.
    // Marking the last character in MissionType.Name is a bad
    // system.
    if (Data->P[plr].Mission[pad].MissionCode != Mission_None &&
        mission.MissionCode == Mission_None) {
        display::graphics.setForegroundColor(9);
        draw_string(145, 33 + pad * 58, "SCRUBBED MISSION");
        // draw_string(193, 77 + pad * 58, "NO PENALTY");
    } else if (! Equals(Data->P[plr].Mission[pad], mission)) {
        // } else if (Data->P[plr].Mission[pad].Name[24] == 1 ||
        //            ! Equals(Data->P[plr].Mission[pad], mission)) {
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
    const int penalty = AchievementPenalty(plr, Mis);
    display::graphics.setForegroundColor(11);
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
        if (Data->P[plr].Mission[i].MissionCode &&
            Data->P[plr].Mission[i].part == 0) {

            GetMisType(Data->P[plr].Mission[i].MissionCode);

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

            SetRush(Data->P[plr].Mission[i].Rushing, i);
            display::graphics.setForegroundColor(1);
            draw_heading(55, 5, "MISSION SCHEDULE", 0, -1);

            display::graphics.setForegroundColor(5);
            draw_string(96, 48 + 58 * i, Mis.Abbr);

            // Show duration level only on missions with a Duration step -Leon
            if (IsDuration(Data->P[plr].Mission[i].MissionCode)) {
                int duration = Data->P[plr].Mission[i].Duration;
                draw_string(0, 0, GetDurationParens(duration));
            }

            if (Data->P[plr].Mission[i].Name[24] == 1) {
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
                        &Mon[Data->P[plr].Mission[i].Month - 0][0]);
            draw_string(288, 55 + 58 * i,
                        &Mon[Data->P[plr].Mission[i].Month - 1][0]);
            draw_string(288, 72 + 58 * i,
                        &Mon[Data->P[plr].Mission[i].Month - 2][0]);

            // Since the Downgrade penalty isn't being used, its
            // screen space is commandeered to display the mission
            // penalty.
            const int penalty = AchievementPenalty(plr, Mis);
            display::graphics.setForegroundColor(1);
            draw_string(88, 77 + i * 58, "REQUIREMENT PENALTIES:");
            display::graphics.setForegroundColor(11);
            draw_number(215, 77 + i * 58, penalty);
            draw_string(0, 0, "%");
        } /* End if */
    }

    return;
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
    int R1, R2, R3, oR1, oR2, oR3;
    char pRush = 0;

    R1 = R2 = R3 = oR1 = oR2 = oR3 = 0;

    // Reset Rushing status for missions.
    for (int pad = 0; pad < 3; pad++) {
        if (Data->P[plr].Mission[pad].Rushing == 1) {
            Data->P[plr].Cash += 3;
        } else if (Data->P[plr].Mission[pad].Rushing == 2) {
            Data->P[plr].Cash += 6;
        }

        Data->P[plr].Mission[pad].Rushing = 0;  // Clear Data
    }

    SetLaunchDates(plr);

    Downgrader::Options downgrades;

    try {
        downgrades = LoadJsonDowngrades("DOWNGRADES.JSON");
    } catch (IOException &err) {
        CCRITICAL2(baris, err.what());
    }

    Downgrader downgradeList[3] = {
        Downgrader(Data->P[plr].Mission[0], downgrades),
        Downgrader(Data->P[plr].Mission[1], downgrades),
        Downgrader(Data->P[plr].Mission[2], downgrades)
    };

    DrawRush(plr);
    pRush = (Data->P[plr].Cash >= 3) ? 1 : 0;
    int fCsh = Data->P[plr].Cash;
    display::graphics.setForegroundColor(1);

    music_start((plr == 0) ? M_USMIL : M_USSRMIL);
    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {
            if (((y >= 32 && y <= 74 && x >= 280 && x <= 312 && mousebuttons > 0) || (key >= '1' && key <= '3'))
                && pRush && Data->P[plr].Mission[0].MissionCode && Data->P[plr].Mission[0].part != 1) {  /* L1: Row One */
                // R1=oR1;
                if (((y >= 49 && y <= 57 && mousebuttons > 0) || key == '2') && oR1 != 1 && fCsh < 3) {
                    Help("i117");
                }

                R1 = (((y >= 49 && y <= 57 && mousebuttons > 0) || key == '2') && fCsh >= 3) ? 1 : R1;

                if (((y >= 66 && y <= 74 && mousebuttons > 0) || key == '3') && oR1 != 2 && fCsh < 6) {
                    Help("i117");
                }

                R1 = (((y >= 66 && y <= 74 && mousebuttons > 0) || key == '3') && fCsh >= 6) ? 2 : R1;
                R1 = ((y >= 32 && y <= 40 && mousebuttons > 0) || key == '1') ? 0 : R1;

                if (oR1 != R1) {
                    ResetRush(oR1, PAD_A);
                    SetRush(R1, PAD_A);
                    fCsh -= (R1 - oR1) * 3;
                    oR1 = R1;
                }
            } else if (((x >= 280 && x <= 312 && y >= 90 && y <= 132 && mousebuttons > 0) || (key >= '4' && key <= '6'))
                       && pRush && Data->P[plr].Mission[1].MissionCode && Data->P[plr].Mission[1].part != 1) {  /* L2: Row One */
                // R2=oR2;
                if (((y >= 107 && y <= 115 && mousebuttons > 0) || key == '5') && oR2 != 1 && fCsh < 3) {
                    Help("i117");
                }

                R2 = (((y >= 107 && y <= 115 && mousebuttons > 0) || key == '5') && fCsh >= 3) ? 1 : R2;

                if (((y >= 124 && y <= 132 && mousebuttons > 0) || key == '6') && oR2 != 2 && fCsh < 6) {
                    Help("i117");
                }

                R2 = (((y >= 124 && y <= 132 && mousebuttons > 0) || key == '6') && fCsh >= 6) ? 2 : R2;
                R2 = ((y >= 90 && y <= 98 && mousebuttons > 0) || key == '4') ? 0 : R2;

                if (oR2 != R2) {
                    ResetRush(oR2, PAD_B);
                    SetRush(R2, PAD_B);
                    fCsh -= (R2 - oR2) * 3;
                    oR2 = R2;
                }
            } else if (((x >= 280 && x <= 312 && y >= 148 && y <= 190 && mousebuttons > 0) || (key >= '7' && key <= '9'))
                       && pRush && Data->P[plr].Mission[2].MissionCode && Data->P[plr].Mission[2].part != 1) {  /* L3: Row One */
                // R3=oR3;
                if (((y >= 165 && y <= 173 && mousebuttons > 0) || key == '8') && oR3 != 1 && fCsh < 3) {
                    Help("i117");
                }

                R3 = (((y >= 165 && y <= 173 && mousebuttons > 0) || key == '8') && fCsh >= 3) ? 1 : R3;

                if (((y >= 182 && y <= 190 && mousebuttons > 0) || key == '9') && oR3 != 2 && fCsh < 6) {
                    Help("i117");
                }

                R3 = (((y >= 182 && y <= 190 && mousebuttons > 0) || key == '9') && fCsh >= 6) ? 2 : R3;
                R3 = ((y >= 148 && y <= 156 && mousebuttons > 0) || key == '7') ? 0 : R3;

                if (oR3 != R3) {
                    ResetRush(oR3, PAD_C);
                    SetRush(R3, PAD_C);
                    fCsh -= (R3 - oR3) * 3;
                    oR3 = R3;
                }
            }

            // DOWNGRADE MISSION KEYBOARD
            if (key == 'Q' || key == 'R' || key == 'U') {
                int i = 0;

                if (key == 'Q') {
                    i = PAD_A;
                } else if (key == 'R') {
                    i = PAD_B;
                } else if (key == 'U') {
                    i = PAD_C;
                } else {
                    i = 0;
                }

                if (Data->P[plr].Mission[i].MissionCode && Data->P[plr].Mission[i].part != 1) {
                    InBox(91, 41 + i * 58, 264, 59 + i * 58);

                    DrawMissionEntry(plr, i, downgradeList[i].next());

                    WaitForMouseUp();
                    OutBox(91, 41 + i * 58, 264, 59 + i * 58);

                }
            }

            for (int i = 0; i < 3; i++) {
                if (x >= 91 && x <= 264 && y >= 41 + i * 59 && y <= 59 + i * 59 && mousebuttons > 0
                    && Data->P[plr].Mission[i].MissionCode
                    && Data->P[plr].Mission[i].part != 1) {  // Downgrade

                    InBox(91, 41 + i * 58, 264, 59 + i * 58);

                    DrawMissionEntry(plr, i, downgradeList[i].next());

                    WaitForMouseUp();
                    OutBox(91, 41 + i * 58, 264, 59 + i * 58);
                }
            }

            if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {  // CONTINUE
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(245, 5, 314, 17);
                delay(10);

                for (int i = 0; i < 3; i++) {
                    if (Data->P[plr].Mission[i].MissionCode &&
                        Data->P[plr].Mission[i].part != 1 &&
                        ! Equals(Data->P[plr].Mission[i],
                                 downgradeList[i].current())) {
                        Downgrade(plr, i, downgradeList[i].current());
                    }
                }

                if (Data->P[plr].Mission[1].part == 1) {
                    R2 = R1;
                }

                if (Data->P[plr].Mission[2].part == 1) {
                    R3 = R2;
                }

                if (Data->P[plr].Mission[0].MissionCode &&
                    Data->P[plr].Cash >= 3 * R1) {
                    Data->P[plr].Cash -= 3 * R1;
                    Data->P[plr].Mission[0].Month -= R1;
                    Data->P[plr].Mission[0].Rushing = R1;
                }

                if (Data->P[plr].Mission[1].MissionCode &&
                    Data->P[plr].Cash >= 3 * R2) {
                    Data->P[plr].Cash -= 3 * R2;
                    Data->P[plr].Mission[1].Month -= R2;
                    Data->P[plr].Mission[1].Rushing = R2;
                }

                if (Data->P[plr].Mission[2].MissionCode &&
                    Data->P[plr].Cash >= 3 * R3) {
                    Data->P[plr].Cash -= 3 * R3;
                    Data->P[plr].Mission[2].Month -= R3;
                    Data->P[plr].Mission[2].Rushing = R3;
                }

                music_stop();
                return;  // Done
            }
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

    return;
}


/* Schedule the default, unrushed launch dates for the planned missions.
 *
 * \param plr  The index of the player launching the missions.
 */
void SetLaunchDates(const char plr)
{
    int missionCount = 0;
    bool joint = false;

    // Currently, can only handle 3 missions.
    // assert(MAX_MISSIONS == 3);

    for (int i = 0; i < MAX_MISSIONS; i++) {
        if (Data->P[plr].Mission[i].Joint == 1) {
            joint = true;
        }

        if (Data->P[plr].Mission[i].MissionCode &&
            Data->P[plr].Mission[i].part == 0) {
            missionCount++;
        }
    }

    if (missionCount == 3) {  // Three non-joint missions
        Data->P[plr].Mission[0].Month = 2 + Data->Season * 6;
        Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
        Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
    }

    if (missionCount == 2 && joint == false) {  // Two non-joint missions
        int start = 3;

        if (Data->P[plr].Mission[0].MissionCode) {
            Data->P[plr].Mission[0].Month = start + Data->Season * 6;
            start += 2;
        }

        if (Data->P[plr].Mission[1].MissionCode) {
            Data->P[plr].Mission[1].Month = start + Data->Season * 6;
            start += 2;
        }

        if (Data->P[plr].Mission[2].MissionCode) {
            Data->P[plr].Mission[2].Month = start + Data->Season * 6;
        }
    }

    if (missionCount == 1 && joint == false) {  // Single Mission Non-joint
        if (Data->P[plr].Mission[0].MissionCode) {
            Data->P[plr].Mission[0].Month = 4 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[1].MissionCode) {
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[2].MissionCode) {
            Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
        }
    }

    if (missionCount == 2 && joint == true) {  // Two launches, one Joint;
        if (Data->P[plr].Mission[1].part == 1) {  // Joint first
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[2].part == 1) {  // Joint second
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 5 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        }
    }

    if (missionCount == 1 && joint == true) {  // Single Joint Launch
        if (Data->P[plr].Mission[1].part == 1) {  // found on pad 1+2
            Data->P[plr].Mission[0].Month = 4 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
        } else {   // found on pad 2+3
            Data->P[plr].Mission[1].Month = 4 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
        }
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

    return;
}

}; // End of unnamed namespace part 3
