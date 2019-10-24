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

#include <fstream>
#include <string>
#include <cassert>

#include <json/json.h>

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
#include "gr.h"
#include "pace.h"
#include "filesystem.h"
#include "radar.h"

// Function Prototype

char Mon[12][4] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// TODO: Remove this.
// It is preferable to supply downgrade options to the constructor
// via a JSON-formatted stream, as this allows for an elastic mission
// count with any number of downgrade choices per mission.
// However, dg is used in newmis.cpp/MisAnn(char plr, char pad).
char dg[62][6] = {
    {00, 00, 00, 00, 00, 00}, // 0
    {00, 00, 00, 00, 00, 00}, // 1
    {00, 00, 00, 00, 00, 00}, // 2
    {00, 00, 00, 00, 00, 00}, // 3
    {02, 00, 00, 00, 00, 00}, // 4
    {03, 00, 00, 00, 00, 00}, // 5
    {04, 02, 00, 00, 00, 00}, // 6
    {00, 00, 00, 00, 00, 00}, // 7
    {07, 00, 00, 00, 00, 00}, // 8
    {00, 00, 00, 00, 00, 00}, // 9
    {00, 00, 00, 00, 00, 00}, // 10
    {00, 00, 00, 00, 00, 00}, // 11
    {00, 00, 00, 00, 00, 00}, // 12
    {00, 00, 00, 00, 00, 00}, // 13
    {04, 02, 00, 00, 00, 00}, // 14
    {00, 00, 00, 00, 00, 00}, // 15
    {00, 00, 00, 00, 00, 00}, // 16
    {06, 04, 02, 00, 00, 00}, // 17
    {00, 00, 00, 00, 00, 00}, // 18
    {00, 00, 00, 00, 00, 00}, // 19
    {14, 04, 00, 00, 00, 00}, // 20
    {16, 00, 00, 00, 00, 00}, // 21
    {19, 00, 00, 00, 00, 00}, // 22
    {23, 19, 00, 00, 00, 00}, // 23
    {17, 06, 04, 02, 00, 00}, // 24
    {04, 02, 00, 00, 00, 00}, // 25
    {25, 06, 04, 02, 00, 00}, // 26
    {25, 14, 04, 02, 00, 00}, // 27
    {25, 24, 17, 04, 02, 00}, // 28
    {28, 26, 25, 24, 17, 06}, // 29
    {22, 19, 37, 00, 00, 00}, // 30
    {16, 00, 00, 00, 00, 00}, // 31
    {37, 19, 00, 00, 00, 00}, // 32
    {26, 27, 25, 14, 06, 00}, // 33
    {31, 21, 16, 00, 00, 00}, // 34
    {22, 37, 19, 00, 00, 00}, // 35
    {32, 37, 22, 19, 00, 00}, // 36
    {19, 00, 00, 00, 00, 00}, // 37
    {04, 00, 00, 00, 00, 00}, // 38
    {16, 00, 00, 00, 00, 00}, // 39
    {38, 27, 25, 14, 06, 04}, // 40
    {39, 21, 16, 00, 00, 00}, // 41
    {00, 00, 00, 00, 00, 00}, // 42
    {04, 00, 00, 00, 00, 00}, // 43
    {31, 16, 00, 00, 00, 00}, // 44
    {42, 00, 00, 00, 00, 00}, // 45
    {43, 04, 00, 00, 00, 00}, // 46
    {44, 31, 16, 00, 00, 00}, // 47
    {46, 43, 38, 04, 00, 00}, // 48
    {47, 38, 16, 00, 00, 00}, // 49
    {48, 46, 43, 38, 04, 00}, // 50
    {49, 39, 16, 00, 00, 00}, // 51
    {47, 44, 38, 16, 00, 00}, // 52
    {50, 48, 46, 43, 00, 00}, // 53
    {46, 43, 00, 00, 00, 00}, // 54
    {52, 47, 44, 00, 00, 00}, // 55
    {51, 49, 00, 00, 00, 00}, // 56
    {51, 49, 00, 00, 00, 00}, // 57
    {00, 00, 00, 00, 00, 00}, // 58
    {00, 00, 00, 00, 00, 00}, // 59
    {00, 00, 00, 00, 00, 00}, // 60
    {00, 00, 00, 00, 00, 00} // 61
};

void Downgrade(char plr, int pad, const struct MissionType &mission);
void DrawMissionEntry(char plr, int pad, const struct MissionType &mission);
void DrawRush(char plr);
Downgrader::Options LoadJsonDowngrades(std::string filename);
void ResetRush(int mode, int pad);
void SetLaunchDates(char plr);
void SetRush(int mode, int pad);


/* Replaces the launch scheduled for the end of the turn with a
 * different mission at the same time.
 *
 * Attempts to replace a Joint mission with a single launch mission,
 * or vice versa, are not allowed and will be ignored.
 *
 * TODO: This function does not support downgrading a manned mission
 * to an unmanned mission.
 *
 * TODO: Duration penalty system is currently disabled.
 *
 * \param plr
 * \param pad
 * \param mission
 */
void Downgrade(const char plr, const int pad,
               const struct MissionType &mission)
{
    if (Data->P[plr].Mission[pad].Joint != mission.Joint) {
        CERROR3(baris,
                "Downgrade attempt to change a mission's Joint status"
                " on pad %d", pad);
        return;
    } else if (mission.MissionCode == Mission_None) {
        ClrMiss(plr, pad);
    }

    Data->P[plr].Mission[pad] = mission;
    // Need to mark mission to show it is downgraded, but this is
    // a bad system that will need to be replaced.
    // Setting a global var will not survive an autosave load.
    // pNeg[plr][pad] = 1;
    // Data->P[plr].Mission[pad].Name[24] = 1;
}


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

    fill_rectangle(191, 71 + pad * 58, 270, 78 + pad * 58, 3);
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

            GetMisType(Data->P[plr].Mission[i].MissionCode);

            draw_string(96, 48 + 58 * i, Mis.Abbr);

            // Show duration level only on missions with a
            // Duration step - Leon
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
        } /* End if */
    }

    return;
}


/* Read the mission downgrade options from a file.
 *
 * The Json format is:
 * {
 *   "missions": [
 *     { "mission": <Code>, "downgrades": [<Codes>] },
 *     ...
 *   ]
 * }
 *
 * NOTE: I would prefer to put this in downgrader.h/cpp, but the
 *   pragma packing makes that a pain... -- rnyoakum
 *
 * \param filename  A Json-formatted data file.
 * \return  A collection of MissionType.MissionCode-indexed downgrade
 *          options.
 * \throws IOException  If filename is not a readable Json file.
 */
Downgrader::Options LoadJsonDowngrades(std::string filename)
{
    char *path = locate_file(filename.c_str(), FT_DATA);

    if (path == NULL) {
        free(path);
        throw IOException(std::string("Unable to open path to ") +
                          filename);
    }

    std::ifstream input(path);
    Json::Value doc;
    Json::Reader reader;
    bool success = reader.parse(input, doc);

    if (! success) {
        free(path);
        throw IOException("Unable to parse JSON input stream");
    }

    assert(doc.isObject());
    Json::Value &missionList = doc["missions"];
    assert(missionList.isArray());

    Downgrader::Options options;

    for (int i = 0; i < missionList.size(); i++) {
        Json::Value &missionEntry = missionList[i];
        assert(missionEntry.isObject());

        int missionCode = missionEntry.get("mission", -1).asInt();
        assert(missionCode >= 0);
        // assert(missionCode >= 0 && missionCode <= 61);

        Json::Value &codeGroup = missionEntry["downgrades"];
        assert(codeGroup.isArray());

        for (int j = 0; j < codeGroup.size(); j++) {
            options.add(missionCode, codeGroup[j].asInt());
        }
    }

    input.close();
    free(path);
    return options;
}


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

        Data->P[plr].Mission[pad].Rushing = 0; // Clear Data
    }

    SetLaunchDates(plr);

    Downgrader::Options downgrades;
   
    try {
        downgrades = LoadJsonDowngrades("DOWNGRADES.JSON");
    } catch (IOException & err) {
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
                && pRush && Data->P[plr].Mission[0].MissionCode && Data->P[plr].Mission[0].part != 1) { /* L1: Row One */
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
                       && pRush && Data->P[plr].Mission[1].MissionCode && Data->P[plr].Mission[1].part != 1) { /* L2: Row One */
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
                       && pRush && Data->P[plr].Mission[2].MissionCode && Data->P[plr].Mission[2].part != 1) { /* L3: Row One */
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
                    && Data->P[plr].Mission[i].part != 1) { // Downgrade

                    InBox(91, 41 + i * 58, 264, 59 + i * 58);

                    DrawMissionEntry(plr, i, downgradeList[i].next());

                    WaitForMouseUp();
                    OutBox(91, 41 + i * 58, 264, 59 + i * 58);
                }
            }

            if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) { //  CONTINUE
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(245, 5, 314, 17);
                delay(10);

                for (int i = 0; i < 3; i++) {
                    if (Data->P[plr].Mission[i].MissionCode &&
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

    // Currently, can only handles 3 missions.
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

    if (missionCount == 3) { // Three non joint missions
        Data->P[plr].Mission[0].Month = 2 + Data->Season * 6;
        Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
        Data->P[plr].Mission[2].Month = 4 + Data->Season * 6;
    }

    if (missionCount == 2 && joint == false) { // Two non joint missions
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

    if (missionCount == 1 && joint == false) { // Single Mission Non joint
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

    if (missionCount == 2 && joint == true) { // Two launches, one Joint;
        if (Data->P[plr].Mission[1].part == 1) { // Joint first
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        }

        if (Data->P[plr].Mission[2].part == 1) { // Joint second
            Data->P[plr].Mission[0].Month = 3 + Data->Season * 6;
            Data->P[plr].Mission[1].Month = 5 + Data->Season * 6;
            Data->P[plr].Mission[2].Month = 5 + Data->Season * 6;
        }
    }

    if (missionCount == 1 && joint == true) { //  Single Joint Launch
        if (Data->P[plr].Mission[1].part == 1) { // found on pad 1+2
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
