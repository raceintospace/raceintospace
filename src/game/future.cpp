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
/**
 * \file future.cpp This is responsible for the Future Missions screen.
 */

#include "future.h"

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"
#include "display/legacy_surface.h"

#include <assert.h>

#include "Buzz_inc.h"
#include "admin.h"
#include "crew.h"
#include "draw.h"
#include "filesystem.h"
#include "futbub.h"
#include "game_main.h"
#include "gr.h"
#include "ioexception.h"
#include "logging.h"
#include "mc.h"
#include "mc2.h"
#include "mission_util.h"
#include "pace.h"
#include "place.h"
#include "prest.h"
#include "sdlhelper.h"


LOG_DEFAULT_CATEGORY(future)


namespace
{
/**
 * Hold the mission parameters used for searching mission by their
 * characteristics.
 */
struct MissionNavigator {
    struct NavButton {
        int value;
        bool lock;
    };

    NavButton duration, docking, EVA, LM, joint;
};

enum FMFields {
    FM_Duration = 1,
    FM_Docking,
    FM_EVA,
    FM_LM,
    FM_Joint
};

// TODO: Localize these. Too many global/file global variables.

bool JointFlag, MarsFlag, JupiterFlag, SaturnFlag;
display::LegacySurface *vh;
// missionData is used in SetParameters, PianoKey, UpSearchRout,
// DownSearchRout, and Future.
std::vector<struct mStr> missionData;
} // End unnamed namespace


void LoadFutureButtons(void);
void LoadFuturePalette(void);
bool JointMissionOK(char plr, char pad);
void DrawFuture(char plr, int mis, char pad, MissionNavigator &nav);
void ClearDisplay(void);
void DrawPenalty(char plr, const struct mStr &mission);
void DrawPenaltyPopup(char plr, const struct mStr &mission);
void SetParameters(void);
void DrawLocks(const MissionNavigator &nav);
void Toggle(FMFields button, int state);
void TogBox(int x, int y, int st);
void PianoKey(int X, MissionNavigator &nav);
void DrawPie(int s);
void PlaceRX(FMFields button);
void ClearRX(FMFields button);
bool NavMatch(const MissionNavigator &nav, const struct mStr &mission);
void NavReset(MissionNavigator &nav);
int UpSearchRout(int num, char plr, const MissionNavigator &navigator);
int DownSearchRout(int num, char plr, const MissionNavigator &navigator);
void PrintDuration(int duration, int color);
void DrawMission(char plr, int X, int Y, int val, MissionNavigator &nav);
void MissionPath(char plr, int val, int pad);
bool FutureMissionOk(char plr, const MissionNavigator &nav, int mis);


/**
 * Loads the Future console graphics into the vh global display buffer.
 *
 * nfutbut.but.0.png is 240x90 pixels.
 *
 * Palette information:
 * - The image nfutbut.but.0.png has a 255-color palette.
 * - The palette is identical to the color palette used by as fmin.but.
 * - There is an empty 32-color space from [208,240).
 */
void LoadFutureButtons(void)
{
    boost::shared_ptr<display::PalettizedSurface> console(
        Filesystem::readImage("images/nfutbut.but.0.png"));
    vh->palette().copy_from(console->palette());
    vh->draw(console, 0, 0);

    return;
}


/**
 * Load the 255-color Future Missions palette to the main display.
 *
 * \throws runtime_error  if Filesystem is unable to read images.
 */
void LoadFuturePalette(void)
{
    // May be done via fmin.img.0.png or nfutbut.but.0.png.
    {
        boost::shared_ptr<display::PalettizedSurface> planets(
            Filesystem::readImage("images/fmin.img.0.png"));
        planets->exportPalette();
    }
    // Also possible to use the nfutbut.but data file to get the
    // palette information quickly, but remember we're trying to
    // use the Filesystem class and **absolutely no packing**!
}


/* Is there room among the launch pads to schedule a joint mission,
 * with the given pad being the first part?
 *
 * \param plr  The player scheduling the launch
 * \param pad  0, 1, or 2 for the primary, secondary, or tertiary pad.
 * \return     True if a joint mission may be scheduled on the pad.
 */
bool JointMissionOK(char plr, char pad)
{
    return (pad >= 0 && pad < MAX_LAUNCHPADS - 1) &&
           (Data->P[plr].LaunchFacility[pad + 1] == 1) &&
           ((Data->P[plr].Future[pad + 1].MissionCode == Mission_None) ||
            (Data->P[plr].Future[pad + 1].part == 1));
}


/* Draws the entire Future Missions display, including the mission-
 * specific information. Used to initialize the mission selector
 * interface.
 *
 * This relies on the global buffer vh, which must have been created
 * prior. The Future Missions button art is loaded into vh by this
 * function.
 *
 * This modifies the global variable Mis, via DrawMission().
 *
 * \param plr  The player scheduling the mission's design scheme.
 * \param mis  The mission type.
 * \param pad  0, 1, or 2 depending on which pad is being used.
 */
void DrawFuture(char plr, int mis, char pad, MissionNavigator &nav)
{
    FadeOut(2, 10, 0, 0);

    LoadFuturePalette();
    display::graphics.screen()->clear();

    gr_sync();

    fill_rectangle(1, 1, 318, 21, 3);
    fill_rectangle(317, 22, 318, 198, 3);
    fill_rectangle(1, 197, 316, 198, 3);
    fill_rectangle(1, 22, 2, 196, 3);
    OutBox(0, 0, 319, 199);
    InBox(3, 3, 30, 19);
    InBox(3, 22, 316, 196);
    IOBox(242, 3, 315, 19);
    ShBox(5, 24, 183, 47);
    ShBox(5, 24, 201, 47); //name box
    ShBox(5, 74, 41, 82); // RESET
    ShBox(5, 49, 53, 72); //dur/man
    ShBox(43, 74, 53, 82);   // Duration lock
    ShBox(80, 74, 90, 82);   // Docking lock
    ShBox(117, 74, 127, 82); // EVA lock
    ShBox(154, 74, 164, 82); // LM lock
    ShBox(191, 74, 201, 82); // Joint mission lock
    ShBox(5, 84, 16, 130); //arrows up
    ShBox(5, 132, 16, 146); //middle box
    ShBox(5, 148, 16, 194); //    down
    ShBox(203, 24, 238, 31); // new right boxes

    // Mission penalty numerical display
    fill_rectangle(206, 36, 235, 44, 7);
    ShBox(203, 33, 238, 47);
    InBox(205, 35, 236, 45);

    // Mission scroll arrows
    draw_up_arrow(8, 95);
    draw_down_arrow(8, 157);

    // Display mission steps toggle
    vh->copyTo(display::graphics.legacyScreen(), 140, 5, 5, 132, 15, 146);

    // Draw the mission specification toggle buttons
    Toggle(FM_Duration, 1);
    DrawPie(0);
    OutBox(5, 49, 53, 72);
    Toggle(FM_Docking, 1);
    TogBox(55, 49, 0);
    Toggle(FM_EVA, 1);
    TogBox(92, 49, 0);
    Toggle(FM_LM, 1);
    TogBox(129, 49, 0);
    Toggle(FM_Joint, 1);

    if (JointFlag == false) {
        InBox(191, 74, 201, 82);
        TogBox(166, 49, 1);
    } else {
        OutBox(191, 74, 201, 82);
        TogBox(166, 49, 0);
    }

    gr_sync();

    DrawMission(plr, 8, 37, mis, nav);

    display::graphics.setForegroundColor(5);

    /* lines of text are 1:8,30  2:8,37   3:8,44    */
    switch (pad) { // These used to say Pad 1, 2, 3  -Leon
    case 0:
        draw_string(8, 30, "PAD A:");
        break;

    case 1:
        draw_string(8, 30, "PAD B:");
        break;

    case 2:
        draw_string(8, 30, "PAD C:");
        break;
    }

    display::graphics.setForegroundColor(1);

    draw_string(9, 80, "RESET");
    draw_string(256, 13, "CONTINUE");

    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        draw_string(200, 9, "SPRING");
    } else {
        draw_string(205, 9, "FALL");
    }

    draw_string(206, 16, "19");
    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(1);
    draw_small_flag(plr, 4, 4);
    draw_heading(40, 5, "FUTURE MISSIONS", 0, -1);
    FadeIn(2, 10, 0, 0);

    return;
}


/* Draws the mission starfield. The background depicts any heavenly
 * bodies reachable by an interplanetary mission. Earth, the Moon,
 * Venus, and Mercury are always shown. Depending on the current year
 * and season, some combination of Mars, Jupiter, and Saturn may be
 * depicted.
 */
void ClearDisplay(void)
{
    boost::shared_ptr<display::PalettizedSurface> background(Filesystem::readImage("images/fmin.img.0.png"));

    display::graphics.screen()->draw(background, 202, 48, 40, 35, 202, 48);
    display::graphics.screen()->draw(background, 17, 83, 225, 113, 17, 83);
    display::graphics.screen()->draw(background, 242, 23, 74, 173, 242, 23);

    if (MarsFlag == true) {
        display::graphics.screen()->draw(background, 1, 1, 12, 11, 198, 153);
    }

    if (JupiterFlag == true) {
        display::graphics.screen()->draw(background, 14, 1, 51, 54, 214, 130);
    }

    if (SaturnFlag == true) {
        display::graphics.screen()->draw(background, 66, 1, 49, 53, 266, 135);
    }

    return;
}


/* Displays the total mission penalty for the current selected mission.
 *
 * This calculates the sum of the prestige, duration, and new mission
 * penalties and reports it as the current mission penalty.
 *
 * This relies on graphics loaded into the file variable vh.
 *
 * \param plr
 * \param mission
 */
void DrawPenalty(char plr, const struct mStr &mission)
{
    int u;

    char penalty = AchievementPenalty(plr, mission);
    fill_rectangle(206, 36, 235, 44, 7);

    if (penalty < 3) {
        u = 1;     //ok
    } else if (penalty < 9) {
        u = 10;    //caution
    } else {
        u = 19;    //danger
    }

    vh->copyTo(display::graphics.legacyScreen(), 203, u, 203, 24, 238, 31);
    display::graphics.setForegroundColor(11);

    if (penalty > 0) {
        draw_string(210, 42, "-");
    } else {
        grMoveTo(210, 42);
    }

    draw_number(0, 0, penalty);
    display::graphics.setForegroundColor(1);

    return;
}


/**
 * Create a popup detailing the sources of the mission penalties.
 *
 * Launches a popup explaining how the projected mission penalty
 * is calculated, and if there is any LM penalty. The popup starts
 * it own control loop to handle input.
 *
 * TODO: Modify the color for each penalty line depending on how
 *   large the penalty is.
 * TODO: Create a keyHelp setting for the popup box explaining how
 *   to close it via the keyboard.
 *
 * \param plr  the player index
 * \param mission  the selected mission
 */
void DrawPenaltyPopup(char plr, const struct mStr &mission)
{
    int milestonePenalty = MilestonePenalty(plr, mission);
    int durationPenalty = DurationPenalty(plr, mission);
    int newMissionPenalty = NewMissionPenalty(plr, mission);

    // Buffer the screen contents behing the popup for quick restoration.
    display::LegacySurface local(165, 124);
    local.copyFrom(display::graphics.legacyScreen(), 85, 52, 249, 175);

    ShBox(85, 92, 249, 175);
    InBox(92, 98, 243, 144);
    display::graphics.setForegroundColor(11);
    draw_string(97, 105, "REQUIREMENT PENALTIES:");

    display::graphics.setForegroundColor(1);
    draw_string(99, 116, "MILESTONE PENALTY");
    draw_string(220, 116, "-");

    if (milestonePenalty > 0) {
        draw_number(0, 0, milestonePenalty);
    } else {
        draw_string(0, 0, "-");
    }

    draw_string(99, 124, "DURATION PENALTY");
    draw_string(220, 124, "-");

    if (durationPenalty > 0) {
        draw_number(0, 0, durationPenalty);
    } else {
        draw_string(0, 0, "-");
    }

    draw_string(99, 132, "NEW MISSION PENALTY");
    draw_string(220, 132, "-");

    if (newMissionPenalty > 0) {
        draw_number(0, 0, newMissionPenalty);
    } else {
        draw_string(0, 0, "-");
    }

    if (mission.LM && IsLunarLanding(mission.Index)) {
        int lunarTestPenalty = 3 * MIN(Data->P[plr].LMpts - 3, 0);

        display::graphics.setForegroundColor(24);
        draw_string(99, 140, "PENALTY ON LM STEPS");

        if (lunarTestPenalty >= 0) {
            draw_string(220, 140, "--");
        } else {
            draw_number(220, 140, lunarTestPenalty);
        }
    }

    IOBox(91, 151, 243, 172);
    display::graphics.setForegroundColor(5);
    draw_string(167 - TextDisplayLength("CONTINUE") / 2, 163,
                "CONTINUE");

    // Primitive control loop. Don't like using localized control loops,
    // but it's the way RIS is written. -- rnyoakum
    bool close = false;

    while (! close) {
        key = 0;
        GetMouse();

        if ((x >= 93 && y >= 153 && x <= 241 && y <= 170 && mousebuttons > 0) ||
            (key == K_ENTER || key == K_ESCAPE)) {
            InBox(93, 153, 241, 170);
            WaitForMouseUp();

            if (key > 0) {
                delay(300);
                key = 0;
            }

            close = true;
            OutBox(93, 153, 241, 170);
            delay(50);
        }
    }

    display::graphics.screen()->draw(local, 85, 52);
}


/**
 * Cache a subset of mission data in a local array.
 *
 * Populates the global array missionData with stored mission data.
 */
void SetParameters(void)
{
    if (! missionData.empty()) {
        return;
    }

    FILE *fin = sOpen("MISSION.DAT", "rb", 0);

    if (fin == NULL) {
        throw IOException("Could not open MISSION.DAT");
    }

    for (int i = 0; i < 62; i++) {
        struct mStr entry;

        if (fread(&entry, sizeof entry, 1, fin) != 1) {
            missionData.clear();
            fclose(fin);
            throw IOException("Error reading entry in MISSION.DAT");
        }

        missionData.push_back(entry);
    }

    fclose(fin);
    return;
}


/* Illustrate all of the mission parameter "locks" in their respective
 * settings.
 *
 * \param nav TODO.
 */
void DrawLocks(const MissionNavigator &nav)
{
    if (nav.duration.lock) {
        PlaceRX(FM_Duration);
    } else {
        ClearRX(FM_Duration);
    }

    if (nav.docking.lock) {
        PlaceRX(FM_Docking);
    } else {
        ClearRX(FM_Docking);
    }

    if (nav.EVA.lock) {
        PlaceRX(FM_EVA);
    } else {
        ClearRX(FM_EVA);
    }

    if (nav.LM.lock) {
        PlaceRX(FM_LM);
    } else {
        ClearRX(FM_LM);
    }

    if (nav.joint.lock) {
        PlaceRX(FM_Joint);
    } else {
        ClearRX(FM_Joint);
    }

    return;
}


/* Draws the illustration on a mission parameter button. Each button
 * has two illustrations, one for the selected state and another for
 * the unselected state.
 *
 * The illustrations are stored in a buffer via the global pointer vh,
 * which reads the information in LoadFutureButtons().
 *
 * \param button  the button index.
 * \param state  1 if selected, 0 if unselected.
 */
void Toggle(FMFields button, int state)
{
    TRACE3("->Toggle(button %d, state %d)", button, state);

    switch (button) {
    case FM_Docking:
        if (state == 1) {
            vh->copyTo(display::graphics.legacyScreen(), 1, 21, 55, 49, 89, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 1, 56, 55, 49, 89, 81);
        }

        break;

    case FM_EVA:
        if (state == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 38, 21, 92, 49, 126, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 38, 56, 92, 49, 126, 81);
        }

        break;

    case FM_LM:
        if (state == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 75, 21, 129, 49, 163, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 75, 56, 129, 49, 163, 81);
        }

        break;

    case FM_Joint:

        // FM_Joint is an exception to the normal image coordinates.
        // In the button image file, the inactive joint image is on
        // the row with the active button images, and vice versa.
        if (state == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 112, 56, 166, 49, 200, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 112, 21, 166, 49, 200, 81);
        }

        break;

    case FM_Duration:
        if (state == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 153, 1, 5, 49, 52, 71);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 153, 26, 5, 49, 52, 71);
        }

        break;

    default:
        ERROR3("Invalid argument to Toggle(button = %d, state = %d)",
               button, state);
        break;
    }

    TRACE1("<-Toggle()");
    return;
}


/* Draws a notched box outline for a mission parameter button.
 *
 * This is a custom form of the standard InBox/OutBox functions, using
 * the same color choices.
 *
 * \param x   The x coordinate of the upper-left corner.
 * \param y   The y coordinate of the upper-left corner.
 * \param st  1 if the button is depressed, 0 otherwise
 */
void TogBox(int x, int y, int st)
{
    TRACE4("->TogBox(x %d, y %d, st %d)", x, y, st);
    char sta[2][2] = {{2, 4}, {4, 2}};

    display::graphics.setForegroundColor(sta[st][0]);
    grMoveTo(0 + x, y + 32);
    grLineTo(0 + x, y + 0);
    grLineTo(34 + x, y + 0);
    display::graphics.setForegroundColor(sta[st][1]);
    grMoveTo(x + 0, y + 33);
    grLineTo(23 + x, y + 33);
    grLineTo(23 + x, y + 23);
    grLineTo(x + 35, y + 23);
    grLineTo(x + 35, y + 0);

    TRACE1("<-TogBox()");
    return;
}


/* Set the mission navigation buttons to match the parameters of the
 * chosen mission.
 *
 * \param X  the mission code (mStr.Index or MissionType.MissionCode).
 * \param nav TODO.
 */
void PianoKey(int X, MissionNavigator &nav)
{
    TRACE2("->PianoKey(X %d)", X);

    if (! nav.docking.lock) {
        nav.docking.value = missionData[X].Doc;
        Toggle(FM_Docking, nav.docking.value);
    }

    if (! nav.EVA.lock) {
        nav.EVA.value = missionData[X].EVA;
        Toggle(FM_EVA, nav.EVA.value);
    }

    if (! nav.LM.lock) {
        nav.LM.value = missionData[X].LM;
        Toggle(FM_LM, nav.LM.value);
    }

    if (! nav.joint.lock) {
        nav.joint.value = missionData[X].Jt;
        Toggle(FM_Joint, nav.joint.value);
    }

    if (! nav.duration.lock) {
        nav.duration.value = missionData[X].Days;
        assert(nav.duration.value >= 0);
        Toggle(FM_Duration, nav.duration.value ? 1 : 0);

        if (nav.duration.value) {
            DrawPie(nav.duration.value);
        }
    }

    DrawLocks(nav);
    TRACE1("<-PianoKey()");
    return;
}


/* Draw a piechart with 0-6 pieces, filled in clockwise starting at the
 * top.
 *
 * This relies on the global buffer vh, which must have been created
 * prior. The Future Missions button art is loaded into vh by this
 * function.
 *
 * TODO: Check to ensure 0 <= s <= 6.
 *
 * \param s  How many slices are filled in on the piechart.
 */
void DrawPie(int s)
{
    int off;

    if (s == 0) {
        off = 1;
    } else {
        off = s * 20;
    }

    vh->copyTo(display::graphics.legacyScreen(), off, 1, 7, 51, 25, 69);
    return;
}


/* Draws a restriction (lock) button in its active (restricted) state.
 * The restriction button indicates whether the linked mission parameter
 * setting may be modified.
 *
 * The button index is:
 *   1: Mission Duration
 *   2: Docking status
 *   3: EVA status
 *   4: Lunar Module status
 *   5: Joint Mission status
 *
 * \param button  The button index.
 */
void PlaceRX(FMFields button)
{
    switch (button) {
    case FM_Duration:
        fill_rectangle(44, 75, 52, 81, 8);
        break;

    case FM_Docking:
        fill_rectangle(81, 75, 89, 81, 8);
        break;

    case FM_EVA:
        fill_rectangle(118, 75, 126, 81, 8);
        break;

    case FM_LM:
        fill_rectangle(155, 75, 163, 81, 8);
        break;

    case FM_Joint:
        fill_rectangle(192, 75, 200, 81, 8);
        break;

    default:
        ERROR2("Invalid argument passed to PlaceRX(button = %d)", button);
        break;
    }

    return;
}


/* Draws a restriction (lock) button in its inactive (unrestricted)
 * state. The restriction button indicates whether the linked mission
 * parameter setting may be modified.
 *
 * The button index is:
 *   1: Mission Duration
 *   2: Docking status
 *   3: EVA status
 *   4: Lunar Module status
 *   5: Joint Mission status
 *
 * \param button  The button index.
 */
void ClearRX(FMFields button)
{
    switch (button) {
    case FM_Duration:
        fill_rectangle(44, 75, 52, 81, 3);
        break;

    case FM_Docking:
        fill_rectangle(81, 75, 89, 81, 3);
        break;

    case FM_EVA:
        fill_rectangle(118, 75, 126, 81, 3);
        break;

    case FM_LM:
        fill_rectangle(155, 75, 163, 81, 3);
        break;

    case FM_Joint:
        fill_rectangle(192, 75, 200, 81, 3);
        break;

    default:
        ERROR2("Invalid argument passed to ClearRX(button = %d)", button);
        break;
    }

    return;
}


/**
 * Determine if the mission is compatible with the requirements locked
 * in the navigator.
 *
 * \param nav a navigator object with locked values for required fields.
 * \param mission a mission template.
 * \return false if any of the locked navigator values contradict a
 *         mission parameter, true otherwise.
 */
bool NavMatch(const MissionNavigator &nav, const struct mStr &mission)
{
    return (! nav.docking.lock || nav.docking.value == mission.Doc) &&
           (! nav.EVA.lock || nav.EVA.value == mission.EVA) &&
           (! nav.LM.lock || nav.LM.value == mission.LM) &&
           (! nav.joint.lock || nav.joint.value == mission.Jt) &&
           (! nav.duration.lock || nav.duration.value == mission.Days ||
            (mission.Dur && nav.duration.value >= mission.Days));
}


/**
 * Reset all values in the mission navigator to 0 and release all locks.
 *
 * \param nav A set of mission parameters to be cleared.
 */
void NavReset(MissionNavigator &nav)
{
    nav.duration.value = 0;
    nav.docking.value = 0;
    nav.EVA.value = 0;
    nav.LM.value = 0;
    nav.joint.value = 0;
    nav.duration.lock = false;
    nav.docking.lock = false;
    nav.EVA.lock = false;
    nav.LM.lock = false;
    nav.joint.lock = false;
}



/* Find the next mission that matches the given parameters, searching
 * by ascending mission code
 *
 * TODO: This can be tightened up...
 *
 * \param num  The mission code of the currently selected mission.
 * \param plr  The current player (0 for USA, 1 for USSR).
 * \param navigator  The required mission parameters.
 * \return  The code of the next matching mission, or 0 if no other
 *          mission is found.
 */
int UpSearchRout(int num, char plr, const MissionNavigator &navigator)
{
    bool found = false;
    int orig = num;

    if (++num >= 56 + plr) {
        num = 0;
    }

    while (! found) {

        if (num == Mission_MarsFlyby && MarsFlag == false ||
            num == Mission_JupiterFlyby && JupiterFlag == false ||
            num == Mission_SaturnFlyby && SaturnFlag == false) {
            found = false;
        } else {
            found = NavMatch(navigator, missionData[num]);
        }

        if (num == orig) {
            return 0;
        }

        if (found == false) {
            if (++num > 56 + plr) {
                num = 0;
            }
        }
    } /* end while */

    return num;
}


/* Find the next mission that matches the given parameters, searching
 * by descending mission code
 *
 * TODO: This can be tightened up...
 *
 * \param num  The mission code of the currently selected mission.
 * \param plr  The current player (0 for USA, 1 for USSR).
 * \param navigator  The required mission parameters.
 * \return  The code of the next matching mission, or 0 if no other
 *          mission is found.
 */
int DownSearchRout(int num, char plr, const MissionNavigator &navigator)
{
    bool found = false;
    int orig = num;

    if (--num < 0) {
        num = 56 + plr;
    }

    // TODO: Redo while loop so finding match immediately returns num?
    while (! found) {

        if (num == Mission_MarsFlyby && MarsFlag == false ||
            num == Mission_JupiterFlyby && JupiterFlag == false ||
            num == Mission_SaturnFlyby && SaturnFlag == false) {
            found = false;
        } else {
            found = NavMatch(navigator, missionData[num]);
        }

        if (num == orig) {
            return 0;
        }

        if (! found) {
            if (--num < 0) {
                num = 56 + plr;
            }
        }
    } /* end while */

    return num;
}


/* The main control loop for the Future Missions feature.
 *
 * This function calls functions which modify the global vars Mis
 * and Mev, among others.
 *
 * \param plr  The player (0 for USA, 1 for USSR) planning the mission.
 */
void Future(char plr)
{
    /** \todo the whole Future()-function is 500 >lines and unreadable */
    TRACE1("->Future(plr)");
    const int MaxDur = 6;
    int pad = 0;
    int setting = -1, prev_setting = -1;

    display::LegacySurface local(166, 9);
    display::LegacySurface local2(177, 197);
    vh = new display::LegacySurface(240, 90);
    LoadFutureButtons();

    unsigned int year = Data->Year;
    unsigned int season = Data->Season;
    TRACE3("--- Setting year=Year (%d), season=Season (%d)", year, season);

    try {
        SetParameters();
    } catch (IOException &err) {
        CRITICAL1(err.what());
        return;
    }

    MarsFlag = MissionTimingOk(Mission_MarsFlyby, year, season);
    JupiterFlag = MissionTimingOk(Mission_JupiterFlyby, year, season);
    SaturnFlag = MissionTimingOk(Mission_SaturnFlyby, year, season);

    while ((pad = FutureCheck(plr, 0)) != 5) {
        keyHelpText = "k011";
        helpText = "i011";
        char misType = 0;
        ClrFut(plr, pad);

        JointFlag = JointMissionOK(plr, pad); // initialize Joint flag
        MissionNavigator nav;
        NavReset(nav);

        if (! JointFlag) {
            nav.joint.value = 0;
            nav.joint.lock = true;
        }

        DrawFuture(plr, misType, pad, nav);

        while (1) {
            key = 0;
            GetMouse();

            prev_setting = setting;
            setting = -1;

            if (key == '-') {
                DecreasePathResolution();
            } else if (key == '+') {
                IncreasePathResolution();
            } else if (key >= 65 && key < MissionStepsDrawn() + 65) {
                setting = key - 65;
            }

            // If the mouse is over one of the Mission Step bubbles,
            // display the step information.
            char selectedBubble = GetBubbleAt(x, y);

            if (selectedBubble >= 0) {
                setting = selectedBubble;
            }

            if (setting >= 0) {
                if (prev_setting < 0) {
                    local.copyFrom(display::graphics.legacyScreen(),
                                   18, 186, 183, 194);
                }

                if (prev_setting != setting) {
                    ShBox(18, 186, 183, 194);
                    display::graphics.setForegroundColor(1);
                    MisStep(21, 192, Mev[setting].loc);
                }
            } else if (setting < 0 && prev_setting >= 0) {
                local.copyTo(display::graphics.legacyScreen(), 18, 186);
            }

            if (((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) ||
                 key == K_ENTER)) {
                InBox(244, 5, 313, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(300);
                }

                key = 0;

                if (! FutureMissionOk(plr, nav, misType)) {
                    OutBox(244, 5, 313, 17);
                    continue;
                }

                OutBox(244, 5, 313, 17);

                // Copy the screen contents to a buffer. If the mission
                // requires a capsule to be assigned, a pop-up will be
                // created listing the options. Once the pop-up is
                // dismissed the screen may be redrawn from the buffer.
                local2.copyFrom(display::graphics.legacyScreen(), 74, 3, 250, 199);
                int duration = missionData[misType].Dur ?
                               nav.duration.value : missionData[misType].Days;
                int NewType = missionData[misType].mCrew;
                Data->P[plr].Future[pad].Duration = duration;

                int Ok = HardCrewAssign(plr, pad, misType, NewType);

                local2.copyTo(display::graphics.legacyScreen(), 74, 3);

                if (Ok == 1) {
                    Data->P[plr].Future[pad].Duration = duration;
                    break;        // return to launchpad loop
                } else {
                    ClrFut(plr, pad);
                    // Set the Mission code after being cleared.
                    Data->P[plr].Future[pad].MissionCode = misType;
                    continue;
                }
            } else if ((x >= 43 && y >= 74 && x <= 53 && y <= 82 && mousebuttons > 0) ||
                       key == '!') {  // Duration restriction lock
                nav.duration.lock = (! nav.duration.lock);

                if (nav.duration.lock) {
                    InBox(43, 74, 53, 82);
                    PlaceRX(FM_Duration);
                } else {
                    OutBox(43, 74, 53, 82);
                    ClearRX(FM_Duration);
                }

                WaitForMouseUp();

            } else if (nav.duration.lock != true &&
                       ((x >= 5 && y >= 49 && x <= 53 && y <= 72 && mousebuttons > 0) ||
                        key == '1')) {  // Duration toggle
                InBox(5, 49, 53, 72);

                if (nav.duration.value == MaxDur) {
                    nav.duration.value = 0;
                    Toggle(FM_Duration, 0);
                } else {
                    nav.duration.value++;

                    if (nav.duration.value == 1) {
                        Toggle(FM_Duration, 1);
                    }

                    DrawPie(nav.duration.value);
                }

                // If a duration mission, update the duration & mission
                // penalty displays
                if (missionData[misType].Dur) {
                    struct mStr mission = missionData[misType];
                    int duration = MAX(nav.duration.value,
                                       missionData[misType].Days);
                    bool valid =
                        (nav.duration.value >= missionData[misType].Days);
                    PrintDuration(duration, valid ? 5 : 9);

                    mission.Days = duration;
                    DrawPenalty(plr, mission);
                }

                WaitForMouseUp();

                // Why was this line here? Foreground gets set in OutBox
                display::graphics.setForegroundColor(34);
                OutBox(5, 49, 53, 72);
            } else if ((x >= 5 && y >= 74 && x <= 41 && y <= 82 && mousebuttons > 0) ||
                       (key == K_ESCAPE)) {  // Reset mission selection
                InBox(5, 74, 41, 82);

                WaitForMouseUp();

                misType = 0;
                NavReset(nav);

                if (JointFlag == false) {
                    nav.joint.value = 0;
                    nav.joint.lock = true;
                    InBox(191, 74, 201, 82);
                    TogBox(166, 49, 1);
                } else {
                    OutBox(191, 74, 201, 82);
                }

                OutBox(5, 49, 53, 72);
                OutBox(43, 74, 53, 82);
                OutBox(80, 74, 90, 82);
                OutBox(117, 74, 127, 82);
                OutBox(154, 74, 164, 82);

                ClrFut(plr, pad);
                DrawMission(plr, 8, 37, misType, nav);
                OutBox(5, 74, 41, 82);

            } else if ((x >= 80 && y >= 74 && x <= 90 && y <= 82 && mousebuttons > 0) ||
                       key == '@') {  // Docking restriction lock
                nav.docking.lock = (! nav.docking.lock);

                if (nav.docking.lock) {
                    InBox(80, 74, 90, 82);
                    PlaceRX(FM_Docking);
                } else {
                    OutBox(80, 74, 90, 82);
                    ClearRX(FM_Docking);
                }

                WaitForMouseUp();

            } else if (nav.docking.lock == false &&
                       (((x >= 55 && y >= 49 && x <= 90 && y <= 82) && mousebuttons > 0) ||
                        key == '2')) {  // Docking toggle
                TogBox(55, 49, 1);

                nav.docking.value = nav.docking.value ? 0 : 1;
                Toggle(FM_Docking, nav.docking.value);

                WaitForMouseUp();
                TogBox(55, 49, 0);

            } else if ((x >= 117 && y >= 74 && x <= 127 && y <= 82 && mousebuttons > 0) ||
                       key == '#') {  // EVA Restriction button
                nav.EVA.lock = (! nav.EVA.lock);

                if (nav.EVA.lock) {
                    InBox(117, 74, 127, 82);
                    PlaceRX(FM_EVA);
                } else {
                    OutBox(117, 74, 127, 82);
                    ClearRX(FM_EVA);
                }

                WaitForMouseUp();

            } else if (nav.EVA.lock == false &&
                       ((x >= 92 && y >= 49 && x <= 127 && y <= 82 && mousebuttons > 0) ||
                        key == '3')) {  // EVA toggle
                TogBox(92, 49, 1);

                nav.EVA.value = nav.EVA.value ? 0 : 1;
                Toggle(FM_EVA, nav.EVA.value);

                WaitForMouseUp();
                TogBox(92, 49, 0);

            } else if ((x >= 154 && y >= 74 && x <= 164 && y <= 82 && mousebuttons > 0) ||
                       key == '$') {  // Lunar Module Restriction button
                nav.LM.lock = (! nav.LM.lock);

                if (nav.LM.lock) {
                    InBox(154, 74, 164, 82);
                    PlaceRX(FM_LM);
                } else {
                    OutBox(154, 74, 164, 82);
                    ClearRX(FM_LM);
                }

                WaitForMouseUp();

            } else if (nav.LM.lock == false &&
                       ((x >= 129 && y >= 49 && x <= 164 && y <= 82 && mousebuttons > 0) ||
                        key == '4')) {  // LEM toggle
                TogBox(129, 49, 1);

                nav.LM.value = nav.LM.value ? 0 : 1;
                Toggle(FM_LM, nav.LM.value);

                WaitForMouseUp();
                TogBox(129, 49, 0);

            } else if (JointFlag == true &&
                       ((x > 191 && y >= 74 && x <= 201 && y <= 82 && mousebuttons > 0) ||
                        key == '%')) {  // Joint Mission Restriction button
                nav.joint.lock = (! nav.joint.lock);

                if (nav.joint.lock) {
                    InBox(191, 74, 201, 82);
                    PlaceRX(FM_Joint);
                } else {
                    OutBox(191, 74, 201, 82);
                    ClearRX(FM_Joint);
                }

                WaitForMouseUp();

            } else if (nav.joint.lock == false && JointFlag == true &&
                       ((x >= 166 && y >= 49 && x <= 201 && y <= 82 && mousebuttons > 0) ||
                        key == '5')) {  // Joint Mission
                TogBox(166, 49, 1);

                nav.joint.value = nav.joint.value ? 0 : 1;
                Toggle(FM_Joint, nav.joint.value);

                WaitForMouseUp();
                TogBox(166, 49, 0);

            } else if (x >= 203 && y >= 24 && x <= 241 && y <= 44 && mousebuttons > 0) {
                // Penalties popup
                DrawPenaltyPopup(plr, missionData[misType]);
            } else if ((x >= 5 && y >= 84 && x <= 16 && y <= 130 && mousebuttons > 0) ||
                       (key == UP_ARROW)) {
                // Scroll up among Mission Types
                InBox(5, 84, 16, 130);

                for (int i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {
                        misType = UpSearchRout(misType, plr, nav);
                        Data->P[plr].Future[pad].MissionCode = misType;
                        i = 51;
                    }
                }

                // Keep scrolling while mouse/key is held down.
                while (mousebuttons == 1 || key == UP_ARROW) {
                    misType = UpSearchRout(misType, plr, nav);
                    Data->P[plr].Future[pad].MissionCode = misType;
                    DrawMission(plr, 8, 37, misType, nav);
                    delay(100);
                    key = 0;
                    GetMouse();
                }

                DrawMission(plr, 8, 37, misType, nav);
                OutBox(5, 84, 16, 130);
            } else if ((x >= 5 && y >= 132 && x < 16 && y <= 146 && mousebuttons > 0) ||
                       (key == K_SPACE)) {
                // Turn on Mission Steps display
                InBox(5, 132, 16, 146);
                WaitForMouseUp();
                delay(50);
                misType = Data->P[plr].Future[pad].MissionCode;
                assert(0 <= misType);

                if (misType != 0) {
                    MissionPath(plr, misType, pad);
                }

                OutBox(5, 132, 16, 146);
            } else if ((x >= 5 && y >= 148 && x <= 16 && y <= 194 && mousebuttons > 0) ||
                       (key == DN_ARROW)) {
                // Scroll down among Mission Types
                InBox(5, 148, 16, 194);

                for (int i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {
                        misType = DownSearchRout(misType, plr, nav);
                        Data->P[plr].Future[pad].MissionCode = misType;
                        i = 51;
                    }

                }

                // Keep scrolling while mouse/key is held down.
                while (mousebuttons == 1 || key == DN_ARROW) {
                    misType = DownSearchRout(misType, plr, nav);
                    Data->P[plr].Future[pad].MissionCode = misType;
                    DrawMission(plr, 8, 37, misType, nav);
                    delay(100);
                    key = 0;
                    GetMouse();
                }

                DrawMission(plr, 8, 37, misType, nav);
                OutBox(5, 148, 16, 194);
            }
        }                              // Input while loop
    }                              // Launch pad selection loop

    delete vh;
    vh = NULL;
    TRACE1("<-Future()");
}


/** Update the selected mission view with the given duration.
 *
 * \param duration  0 for unmanned, 1-6 for duration A through F
 * \param color  The palette color for printing the duration
 *     (usually 5, 9 for red).
 *
 * \todo Link this at whatever place the duration is actually defined
 */
void PrintDuration(int duration, int color)
{
    fill_rectangle(112, 25, 199, 30, 3);  // Draw over old duration
    display::graphics.setForegroundColor(color);

    switch (duration) {
    case -1:
    case 0:
        draw_string(112, 30, "NO DURATION");
        break;

    case 1:
        draw_string(112, 30, "1 - 2 DAYS (A)");
        break;

    case 2:
        draw_string(112, 30, "3 - 5 DAYS (B)");
        break;

    case 3:
        draw_string(112, 30, "6 - 7 DAYS (C)");
        break;

    case 4:
        draw_string(112, 30, "8 - 12 DAYS (D)");
        break;

    case 5:
        draw_string(112, 30, "13 - 16 DAYS (E)");
        break;

    case 6:
        draw_string(112, 30, "17 - 20 DAYS (F)");
        break;

    default:
        ERROR3("Invalid argument to PrintDuration(duration = %d, color = %d)",
               duration, color);
        break;
    }

    return;
}


/* Prints the name of the selected mission.
 *
 * This writes the name of the mission associated with the given mission
 * code
 *
 * \note This sets the global variable Mis, via GetMisType().
 *
 * \param val  The mission code.
 * \param xx   The x coordinates for the name block's upper-left corner.
 * \param yy   The y coordinates for the name block's upper-left corner.
 * \param len  The number of characters at which to start a new line.
 */
void MissionName(int val, int xx, int yy, int len)
{
    TRACE5("->MissionName(val %d, xx %d, yy %d, len %d)",
           val, xx, yy, len);
    int i, j = 0;

    GetMisType(val);

    grMoveTo(xx, yy);

    for (i = 0; i < 50; i++) {
        if (j > len && Mis.Name[i] == ' ') {
            yy += 7;
            j = 0;
            grMoveTo(xx, yy);
        } else {
            draw_character(Mis.Name[i]);
        }

        j++;

        if (Mis.Name[i] == '\0') {
            break;
        }
    }

    TRACE1("<-MissionName");

    return;
}


/**
 * Update the mission display to reflect the given mission, including
 * the type, name, duration, and navigation toggle buttons.
 *
 * DrawMission handles important cleanup related to changing the
 * mission selection, including
 *  - Display mission name, type, and duration.
 *  - Set unlocked navigation toggles to match the mission parameters.
 *  - Reset the flight path (clear starfield).
 * It should be called whenever the mission selection changes.
 *
 * This modifies the global value Mis. Specifically, it calls
 * MissionName(), which modifies Mis.
 *
 * \param plr Player
 * \param X screen coord for mission name string
 * \param Y screen coord for mission name string
 * \param val the mission type (MissionType.MissionCode / mStr.Index)
 * \param bub if set to 0 or 3 the function will not draw stuff
 * \param nav the set of mission parameters for mission selection.
 */
void DrawMission(char plr, int X, int Y, int val, MissionNavigator &nav)
{
    TRACE4("->DrawMission(plr, X %d, Y %d, val %d, nav)", X, Y, val);

    // PianoKey is used whenever the mission selection changes, to
    // update the mission navigator with parameters matching the
    // newly displayed mission. This ensures the navigation display
    // handles the dual task
    PianoKey(val, nav);   // Should this be moved outside DrawMission?

    ClearDisplay();                     // Redraw solar system display
    fill_rectangle(6, 31, 199, 46, 3);  // Clear mission name
    fill_rectangle(80, 25, 112, 30, 3); // Clear mission type
    display::graphics.setForegroundColor(5);
    draw_string(55, 30, "TYPE: ");
    draw_number(0, 0, val);
    display::graphics.setForegroundColor(5);

    // Creating a copy of the mission to send to DrawPenalty, rather
    // than using Mis, to decrease the reliance on global vars.
    struct mStr mission = missionData[val];

    // If a duration mission, print the selected duration so long as
    // it is greater than the minimum mission duration.
    if (mission.Dur == 1) {
        int duration = MAX(nav.duration.value, mission.Days);
        PrintDuration(duration, duration >= mission.Days ? 5 : 9);
    } else {
        PrintDuration(mission.Days, 5);
    }

    // MissionName calls GetMisType, which sets the global var Mis.
    MissionName(val, X, Y, 24);

    if (mission.Dur == 1 && mission.Days < nav.duration.value) {
        mission.Days = nav.duration.value;
    }

    DrawPenalty(plr, mission);

    gr_sync();
    TRACE1("<-DrawMission()");
}  // end function DrawMission


/**
 * Checks that the mission is legitimately configured before allowing
 * it to be scheduled.
 *
 * This handles any checks to ensure the mission can be scheduled
 * for launch the next turn, namely:
 *  - On duration missions, the scheduled duration must meet the
 *    minimum duration for that mission type.
 *
 * EVA / Docking program requirements could be placed here, but are
 * currently handled in the VAB. Manned equipment requirements,
 * such as at least one capsule program and at least one available
 * crew, are handled by HardCrewAssign and its subordinate functions.
 *
 * \param plr  The player index.
 * \param nav  The current FM navigation settings.
 * \param mis  The mission's index code.
 * \return true if the mission is OK, false otherwise.
 */
bool FutureMissionOk(char plr, const MissionNavigator &nav, int mis)
{
    const struct mStr &mission = missionData[mis];

    if (mission.Dur && nav.duration.value < mission.Days) {
        Help("i160");
        return false;
    }

    /* Different versions of these checks are performed in the VIB/VAB.
     * In 'classic' RIS, EVA/Docking requirements were checked in
     * Future Missions, so these lines are left as a reference so
     * the checks can be easily restored. -- rnyoakum
     */
    // if (mission.EVA && Data->P[plr].Misc[MISC_HW_EVA_SUITS].Num < 0) {
    //     Help("i118");
    //     return false;
    // }

    // if (mission.Doc && Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num < 0) {
    //     Help("i119");
    //     return false;
    // }

    return true;
}


/* vim: set noet ts=4 sw=4 tw=77: */
