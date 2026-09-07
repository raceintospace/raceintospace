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

// This page handles the Launch Pad screen.

#include "radar.h"

#include <cassert>

#include "display/graphics.h"
#include "display/palettized_surface.h"

#include "ast0.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "gamedata.h"
#include "game_main.h"
#include "mission_util.h"
#include "news_suq.h"
#include "pace.h"
#include "place.h"
#include "start.h"
#include "state_utils.h"


void PadDraw(char plr, char pad);
void PadPict(char poff);
void DrawCrew(int x_coord, int y_coord, int plr, int Capsule, int crew);

void DrawCrew(int x_coord, int y_coord, int plr, int Capsule, int crew)
{
    auto& pData = Data->P[plr];
    for (int i = 0; i < pData.CrewCount[Capsule][crew]; i++) {
        int Crew_idx = pData.Crew[Capsule][crew][i] - 1;
        const Astros& CrewMember = pData.Pool[Crew_idx];
        
        // Draw a morale box for each crew member -Leon
        fill_rectangle(x_coord, y_coord + 7 * i, x_coord+8, y_coord + 6 + 7 * i, 4);
        fill_rectangle(x_coord, y_coord + 7 * i, x_coord+7, y_coord + 5 + 7 * i, 2);
        
        int color = MoodColor(CrewMember.Mood);
        fill_rectangle(x_coord + 1, y_coord + 1 + 7 * i, x_coord + 7, y_coord + 5 + 7 * i, color);

        int colors[] = {1, 5,
                        0, 7};
        display::graphics.setForegroundColor(colors[CrewMember.Sex + 2 * (CrewMember.RetirementDelay > 0)]);
        draw_string(x_coord + 12, y_coord + 5 + 7 * i, CrewMember.Name);
        
        int missions = CrewMember.Missions;
        if (missions > 0) {
            draw_string(0, 0, " (");
            draw_number(0, 0, missions);
            draw_string(0, 0, ")");
        }
    }
}


void PadDraw(char plr, char pad)
{
    MissionType& mission = Data->P[plr].Mission[pad];
    LaunchFacility_Status launchpad_status = Data->P[plr].LaunchFacility[pad];
    int MissionCode = mission.MissionCode;
    int primary_crew = mission.PCrew - 1;
    int backup_crew = mission.BCrew - 1;
    int Capsule = mission.Prog;

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 198);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    InBox(167, 27, 316, 176);
    fill_rectangle(168, 28, 315, 175, 0);
    if (launchpad_status >= LAUNCHPAD_DAMAGED_MARGIN) { 
        IOBox(167, 179, 316, 195);  // Button to fix damaged/destroyed pad
    } else {
        if (MissionCode != Mission_None &&
            MissionTimingOk(MissionCode, Data->Year, Data->Season)) {
            IOBox(167, 179, 240, 195);  // Delay button disabled because mission can't be delayed
            IOBox(242, 179, 316, 195);  // Scrub button
        } else {
            InBox(167, 179, 240, 195);  // Delay button
            IOBox(242, 179, 316, 195);  // Scrub button
        }
    }
    draw_number(170, 100, launchpad_status);
    ShBox(4, 28, 162, 43);
    InBox(6, 30, 160, 41);
    ShBox(4, 46, 162, 61);
    InBox(6, 48, 160, 59);
    ShBox(4, 68, 162, 97);
    InBox(6, 70, 160, 95);
    ShBox(55, 64, 109, 74);  // Box for MISSION name
    InBox(56, 65, 108, 73);  // Box for MISSION name
    ShBox(4, 180, 162, 195);
    InBox(6, 182, 160, 193);  //sched. duration
    InBox(6, 99, 160, 178);
    display::graphics.setForegroundColor(9);
    draw_string(18, 190, "SCHEDULED DURATION: ");
    display::graphics.setForegroundColor(7);

    if ((MissionCode > 24 && MissionCode < 37) || MissionCode == 40 || MissionCode == 41 || MissionCode == 43 || MissionCode == 44 || MissionCode > 45)
        // Show Duration level for manned missions with Duration steps: this keeps the Mission[pad].Duration
        // variable from continuing to show Duration level if mission is scrubbed or downgraded - Leon
    {
        if (mission.Duration == 0) {
            draw_string(0, 0, "NONE");
        } else {
            draw_character('A' + mission.Duration - 1);
        }
    } else {
        if (primary_crew >= 0 || backup_crew >= 0) {
            draw_string(0, 0, "A");
        } else {
            draw_string(0, 0, "NONE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(64, 71, "MISSION");
    draw_small_flag(plr, 4, 4);

    if (launchpad_status == LAUNCHPAD_OPERATIONAL && MissionCode != Mission_None) {
        PadPict(2 + plr);
    } else if (launchpad_status == LAUNCHPAD_OPERATIONAL && MissionCode == Mission_None) {
        PadPict(4 + plr);
    } else if (launchpad_status >= LAUNCHPAD_DESTROYED_MARGIN) {  // Destroyed Pad
        PadPict(6 + plr);
    } else if (launchpad_status >= LAUNCHPAD_DAMAGED_MARGIN) {  // Damaged Pad
        PadPict(0 + plr);
    }

    display::graphics.setForegroundColor(1);
    draw_string(15, 37, "STATUS: ");
    display::graphics.setForegroundColor(9);

    if (launchpad_status == LAUNCHPAD_OPERATIONAL) {
        draw_string(0, 0, "OPERATIONAL");
    } else if (launchpad_status >= LAUNCHPAD_DESTROYED_MARGIN) {
        draw_string(0, 0, "DESTROYED");
    } else {
        draw_string(0, 0, "DAMAGED");
    }

    display::graphics.setForegroundColor(1);

    if (launchpad_status >= LAUNCHPAD_DAMAGED_MARGIN) {
        draw_string(15, 56, "REPAIR COST: ");
        display::graphics.setForegroundColor(9);
        draw_number(0, 0, launchpad_status);
        draw_string(0, 0, "MB");
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, " (OF ");
        draw_number(0, 0, Data->P[plr].Cash);
        draw_string(0, 0, ")");

        if (Data->P[plr].Cash < launchpad_status) {
            InBox(169, 181, 314, 193);
        }
    } else {
        display::graphics.setForegroundColor(9);

        if (MissionCode == Mission_None) {
            draw_string(15, 56, "NO LAUNCH SCHEDULED");
            //InBox(168, 179, 312, 193);
        } else {
            draw_string(15, 56, "PRE-MISSION CHECK");
        }
    }

    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    if (launchpad_status == LAUNCHPAD_OPERATIONAL) {
        display::graphics.setForegroundColor(9);
        draw_string(189, 189, "D");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "ELAY");
        display::graphics.setForegroundColor(9);
        draw_string(264, 189, "S");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "CRUB");
    } else {
        display::graphics.setForegroundColor(9);
        draw_string(205, 189, "F");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "IX LAUNCH PAD");
    }

    draw_heading(37, 5, "LAUNCH FACILITY", 0, -1);

    switch (pad) {
    case 0:
        draw_heading(203, 5, "A", 0, -1);
        break;

    case 1:
        draw_heading(203, 5, "B", 0, -1);
        break;

    case 2:
        draw_heading(203, 5, "C", 0, -1);
        break;
    }

    display::graphics.setForegroundColor(6);
    DrawMissionName(MissionCode, 10, 81, 20);
    display::graphics.setForegroundColor(1);

    // joint mission part
    if (mission.Joint == 1) {
        ShBox(38, 91, 131, 101);
        InBox(39, 92, 130, 100);
        display::graphics.setForegroundColor(1);

        if (mission.part == 0) {
            draw_string(53, 98, "PRIMARY PART");
        } else {
            draw_string(44, 98, "SECONDARY PART");
        }
    }

    // Crews
    display::graphics.setForegroundColor(7);
    draw_string(13, 107, "PRIMARY CREW  ");

    if (primary_crew >= 0) {
        assert(primary_crew < 8);
        
        display::graphics.setForegroundColor(11);  // Now display the crew number, for reference -Leon        
        draw_string(0,0, "(CREW ");
        draw_string(0,0, RomanNumeral(primary_crew+1).c_str());
        draw_string(0,0, ")");

        DrawCrew(13, 110, plr, Capsule, primary_crew);

        if (backup_crew == -1) {
            draw_string(25, 174, "UNAVAILABLE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(13, 145, "BACKUP CREW  ");

    if (backup_crew >= 0) {
        assert(backup_crew < 8);
        
        display::graphics.setForegroundColor(11);  // Now display the crew number, for player's reference -Leon
        draw_string(0,0, "(CREW ");
        draw_string(0,0, RomanNumeral(backup_crew+1).c_str());
        draw_string(0,0, ")");

        DrawCrew(13, 148, plr, Capsule, backup_crew);
        
        if (primary_crew == -1) {
            draw_string(25, 136, "UNAVAILABLE");
        }
    }

    if (mission.Prog > 0) {
        PatchMe(plr, 126, 40, mission.Prog - 1,
                mission.Patch);
    }

    FadeIn(2, 10, 0, 0);
}


/**
 * Draw an image of the Launch Pad facility in the Launch Pad menu.
 *
 * Launch Pad images have their own 256-color palette that is exported
 * to the main display. The first 32 colors are shared with the Port
 * palettes.
 *
 * The pad images (poff) are:
 *   0: USA Launch Pad, damaged
 *   1: USSR Launch Pad, damaged
 *   2: USA Launch Pad, scheduled launch
 *   3: USSR Launch Pad, scheduled launch
 *   4: USA Launch Pad, no mission
 *   5: USSR Launch Pad, no mission
 *   6: USA Launch Pad, destroyed
 *   7: USSR Launch Pad, destroyed
 *
 * \param poff  which of the pad images to display (0-6).
 * \throws runtime_error if Filesystem cannot load the image.
 */
void PadPict(char poff)
{
    assert(poff >= 0 && poff <= 7);

    char filename[128];
    snprintf(filename, sizeof(filename),
             "images/lfacil.but.%d.png", (int) poff);
    boost::shared_ptr<display::PalettizedSurface> image(
        Filesystem::readImage(filename));

    image->exportPalette();
    display::graphics.screen()->draw(image, 168, 28);
}


void ShowPad(char plr, char pad)
{
    MissionType& mission = Data->P[plr].Mission[pad];
    LaunchFacility_Status launchpad_status = Data->P[plr].LaunchFacility[pad];
    int MissionCode = mission.MissionCode;
    
    music_start((plr == 0) ? M_USMIL : M_USSRMIL);
    PadDraw(plr, pad);

    if (MissionCrewCantFly(plr, pad)) {  //found mission no crews
        ScrubMission(plr, pad);
        return;
    }

    helpText = "i028";
    keyHelpText = "k028";
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0)
            || (key == 'S')) {
            if (launchpad_status != LAUNCHPAD_OPERATIONAL) continue;
            if (MissionCode == Mission_None) continue;
            // Scrub Mission
            InBox(244, 181, 314, 193);
            key = 0;
            WaitForMouseUp();

            if (ScrubMissionQuery(plr, pad)) {
                ScrubMission(plr, pad);
            }

            OutBox(244, 181, 314, 193);
            key = 0;

            if (MissionCode == Mission_None) {
                return;
            }
        } else if ((x >= 169 && y >= 181 && x <= 238 && y <= 193 && mousebuttons > 0)
                   || (key == 'D')) {
            if (launchpad_status != LAUNCHPAD_OPERATIONAL) continue;
            if (MissionCode == Mission_None) continue;
            // Delay Mission

            // There are restrictions on Mars/Jupiter/Saturn Flybys, so
            // check that this mission _could_ be launched at this time.

            if (! MissionTimingOk(MissionCode, Data->Year, Data->Season)) continue;
            
            InBox(169, 181, 238, 193);
            WaitForMouseUp();

            if (key > 0) {
                delay(100);
            }

            bool conflict = false;

            // Check if there's a Future Mission which would be
            // displaced by delaying the mission.
            if (Data->P[plr].Future[pad].MissionCode != Mission_None) {
                conflict = true;
            } else if (mission.Joint) {
                int other = (mission.part) ?
                            pad - 1 : pad + 1;

                if (Data->P[plr].Future[other].MissionCode) {
                    conflict = true;
                }
            }

            // Confirm that the mission should be delayed.
            if (Help(conflict ? "i163" : "i162") > 0) {
                DelayMission(plr, pad);
                OutBox(169, 181, 238, 193);
                break;
            } else {
                OutBox(169, 181, 238, 193);

            }
        } else if ((x >= 169 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0)
                   || (key == 'F')) {
            if (launchpad_status < LAUNCHPAD_DAMAGED_MARGIN) continue;
            if (Data->P[plr].Cash < launchpad_status) continue;
            // Fix launchpad
            InBox(169, 181, 314, 193);
            key = 0;
            WaitForMouseUp();

            bool temp = false;
            if (Data->P[plr].Cash >= launchpad_status) {
                temp = Help("i115");

                if (temp == 1) {
                    Data->P[plr].Cash -= launchpad_status;
                    Data->P[plr].LaunchFacility[pad] = LAUNCHPAD_OPERATIONAL;
                }
            } else {
                Help("i116");
            }

            OutBox(169, 181, 314, 193);
            key = 0;

            if (temp) {
                return;
            }
        } else if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER || key == K_ESCAPE) {
            InBox(245, 5, 314, 17);
            key = 0;
            WaitForMouseUp();
            OutBox(245, 5, 314, 17);

            if (key > 0) {
                delay(150);
            }

            key = 0;
            return;  /* Done */
        }
    }
}

// EOF
