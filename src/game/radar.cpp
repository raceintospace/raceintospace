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
#include "gr.h"
#include "mission_util.h"
#include "news_suq.h"
#include "pace.h"
#include "place.h"
#include "sdlhelper.h"
#include "state_utils.h"


void PadDraw(char plr, char pad);
void PadPict(char poff);


void PadDraw(char plr, char pad)
{
    int i, j, k, l;
    int missions;     // Variable for how many missions each 'naut has flown

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 198);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    InBox(167, 27, 316, 176);
    fill_rectangle(168, 28, 315, 175, 0);
    struct MissionType &mission = Data->P[plr].Mission[pad];
    if (Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DAMAGED_MARGIN) { 
        IOBox(167, 179, 316, 195);  // Button to fix damaged/destroyed pad
    } else {
        if (mission.MissionCode &&
            MissionTimingOk(mission.MissionCode, Data->Year, Data->Season)) {
            IOBox(167, 179, 240, 195);  // Delay button disabled because mission can't be delayed
            IOBox(242, 179, 316, 195);  // Scrub button
        } else {
            InBox(167, 179, 240, 195);  // Delay button
            IOBox(242, 179, 316, 195);  // Scrub button
        }
    }
draw_number(170, 100, Data->P[plr].LaunchFacility[pad]);
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
    int MisCod;
    MisCod = Data->P[plr].Mission[pad].MissionCode;

    if ((MisCod > 24 && MisCod < 37) || MisCod == 40 || MisCod == 41 || MisCod == 43 || MisCod == 44 || MisCod > 45)
        // Show Duration level for manned missions with Duration steps: this keeps the Mission[pad].Duration
        // variable from continuing to show Duration level if mission is scrubbed or downgraded - Leon
    {
        switch (Data->P[plr].Mission[pad].Duration) {
        case 1:
            draw_string(0, 0, "A");
            break;

        case 2:
            draw_string(0, 0, "B");
            break;

        case 3:
            draw_string(0, 0, "C");
            break;

        case 4:
            draw_string(0, 0, "D");
            break;

        case 5:
            draw_string(0, 0, "E");
            break;

        case 6:
            draw_string(0, 0, "F");
            break;

        default:
            draw_string(0, 0, "NONE");
            break;
        }
    } else {
        if (Data->P[plr].Mission[pad].PCrew - 1 >= 0 || Data->P[plr].Mission[pad].BCrew - 1 >= 0) {
            draw_string(0, 0, "A");
        } else {
            draw_string(0, 0, "NONE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(64, 71, "MISSION");
    draw_small_flag(plr, 4, 4);

    if (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && Data->P[plr].Mission[pad].MissionCode) {
        PadPict(2 + plr);
    } else if (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && Data->P[plr].Mission[pad].MissionCode == Mission_None) {
        PadPict(4 + plr);
    } else if (Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DESTROYED_MARGIN) {  // Destroyed Pad
        PadPict(6 + plr);
    } else if (Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DAMAGED_MARGIN) {  // Damaged Pad
        PadPict(0 + plr);
    }

    display::graphics.setForegroundColor(1);
    draw_string(15, 37, "STATUS: ");
    display::graphics.setForegroundColor(9);

    if (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL) {
        draw_string(0, 0, "OPERATIONAL");
    } else if (Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DESTROYED_MARGIN) {
        draw_string(0, 0, "DESTROYED");
    } else {
        draw_string(0, 0, "DAMAGED");
    }

    display::graphics.setForegroundColor(1);

    if (Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DAMAGED_MARGIN) {
        draw_string(15, 56, "REPAIR COST: ");
        display::graphics.setForegroundColor(9);
        draw_number(0, 0, Data->P[plr].LaunchFacility[pad]);
        draw_string(0, 0, "MB");

        if (Data->P[plr].Cash < Data->P[plr].LaunchFacility[pad]) {
            InBox(169, 181, 314, 193);
        }
    } else {
        display::graphics.setForegroundColor(9);

        if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
            draw_string(15, 56, "NO LAUNCH SCHEDULED");
            //InBox(168, 179, 312, 193);
        } else {
            draw_string(15, 56, "PRE-MISSION CHECK");
        }
    }

    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    if (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL) {
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
    int code = Data->P[plr].Mission[pad].MissionCode;
    DrawMissionName(code, 10, 81, 20);
    display::graphics.setForegroundColor(1);

    // joint mission part
    if (Data->P[plr].Mission[pad].Joint == 1) {
        ShBox(38, 91, 131, 101);
        InBox(39, 92, 130, 100);
        display::graphics.setForegroundColor(1);

        if (Data->P[plr].Mission[pad].part == 0) {
            draw_string(53, 98, "PRIMARY PART");
        } else {
            draw_string(44, 98, "SECONDARY PART");
        }
    }

    // Hardware to Use

    i = Data->P[plr].Mission[pad].Prog;
    j = Data->P[plr].Mission[pad].PCrew - 1;
    l = Data->P[plr].Mission[pad].BCrew - 1;

    // Crews
    display::graphics.setForegroundColor(7);
    draw_string(13, 107, "PRIMARY CREW  ");

    if (j >= 0) {
        display::graphics.setForegroundColor(11);  // Now display the crew number, for player's easy reference -Leon

        if (j == 0) {
            draw_string(0, 0, "(CREW I)");
        }

        if (j == 1) {
            draw_string(0, 0, "(CREW II)");
        }

        if (j == 2) {
            draw_string(0, 0, "(CREW III)");
        }

        if (j == 3) {
            draw_string(0, 0, "(CREW IV)");
        }

        if (j == 4) {
            draw_string(0, 0, "(CREW V)");
        }

        if (j == 5) {
            draw_string(0, 0, "(CREW VI)");
        }

        if (j == 6) {
            draw_string(0, 0, "(CREW VII)");
        }

        if (j == 7) {
            draw_string(0, 0, "(CREW VIII)");
        }

        for (k = 0; k < Data->P[plr].CrewCount[i][j]; k++) {
            // Draw a morale box for each crew member -Leon
            display::graphics.setForegroundColor(1);
            fill_rectangle(13, 110 + 7 * k, 20, 110 + 7 * k, 2);  // Top
            fill_rectangle(13, 110 + 7 * k, 13, 116 + 7 * k, 2);  // Left
            fill_rectangle(13, 116 + 7 * k, 20, 116 + 7 * k, 4);  // Bottom
            fill_rectangle(21, 110 + 7 * k, 21, 116 + 7 * k, 4);  // Right

            int color = MoodColor(Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood);
            fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, color);

            display::graphics.setForegroundColor(1);

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Sex == 1) {
                display::graphics.setForegroundColor(5);    // Show female 'nauts in blue
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(0);    // Show men who've announced retirement in black
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Sex == 1 && Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(7);
            }

            draw_string(25, 115 + 7 * k, &Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Name[0]);   // Show women who've announced retirement in purple
            missions = Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Missions;

            if (missions > 0) {
                draw_string(0, 0, " (");
                draw_number(0, 0, missions);
                draw_string(0, 0, ")");
            }
        }

        if (l == -1) {
            draw_string(25, 174, "UNAVAILABLE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(13, 145, "BACKUP CREW  ");

    if (l >= 0) {
        display::graphics.setForegroundColor(11);  // Now display the crew number, for player's easy reference -Leon

        if (l == 0) {
            draw_string(0, 0, "(CREW I)");
        }

        if (l == 1) {
            draw_string(0, 0, "(CREW II)");
        }

        if (l == 2) {
            draw_string(0, 0, "(CREW III)");
        }

        if (l == 3) {
            draw_string(0, 0, "(CREW IV)");
        }

        if (l == 4) {
            draw_string(0, 0, "(CREW V)");
        }

        if (l == 5) {
            draw_string(0, 0, "(CREW VI)");
        }

        if (l == 6) {
            draw_string(0, 0, "(CREW VII)");
        }

        if (l == 7) {
            draw_string(0, 0, "(CREW VIII)");
        }

        for (k = 0; k < Data->P[plr].CrewCount[i][l]; k++) {
            // Draw a morale box for each crew member -Leon
            display::graphics.setForegroundColor(1);
            fill_rectangle(13, 148 + 7 * k, 20, 148 + 7 * k, 2);  // Top
            fill_rectangle(13, 148 + 7 * k, 13, 154 + 7 * k, 2);  // Left
            fill_rectangle(13, 154 + 7 * k, 20, 154 + 7 * k, 4);  // Bottom
            fill_rectangle(21, 148 + 7 * k, 21, 154 + 7 * k, 4);  // Right

            int color = MoodColor(Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood);
            fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, color);

            display::graphics.setForegroundColor(1);

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Sex == 1) {
                display::graphics.setForegroundColor(5);    // Show female 'nauts in blue
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(0);    // Show men who've announced retirement in black
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Sex == 1 && Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(7);
            }

            draw_string(25, 153 + 7 * k, &Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Name[0]);   // Show women who've announced retirement in purple
        }

        if (j == -1) {
            draw_string(25, 136, "UNAVAILABLE");
        }
    }

    if (Data->P[plr].Mission[pad].Prog > 0) {
        PatchMe(plr, 126, 40, Data->P[plr].Mission[pad].Prog - 1,
                Data->P[plr].Mission[pad].Patch);
    }

    FadeIn(2, 10, 0, 0);

    return;
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
 * \throws runtime_error  if Filesystem cannot load the image.
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
    char temp;
    music_start((plr == 0) ? M_USMIL : M_USSRMIL);
    PadDraw(plr, pad);
    temp = CheckCrewOK(plr, pad);

    if (temp == 1) {  //found mission no crews
        ScrubMission(plr, pad);
        return;
    }

    helpText = "i028";
    keyHelpText = "k028";
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && x >= 244 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0 && Data->P[plr].Mission[pad].MissionCode)
            || (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && Data->P[plr].Mission[pad].MissionCode && key == 'S')) {
            // Scrub Mission
            InBox(244, 181, 314, 193);
            key = 0;
            WaitForMouseUp();

            if (ScrubMissionQuery(plr, pad)) {
                ScrubMission(plr, pad);
            }

            OutBox(244, 181, 314, 193);
            key = 0;

            if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
                return;
            }
        } else if ((Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && x >= 169 && y >= 181 && x <= 238 && y <= 193 && mousebuttons > 0 && Data->P[plr].Mission[pad].MissionCode)
            || (Data->P[plr].LaunchFacility[pad] == LAUNCHPAD_OPERATIONAL && Data->P[plr].Mission[pad].MissionCode && key == 'D')) {
            // Delay Mission

            // There are restrictions on Mars/Jupiter/Saturn Flybys,
            // so check that this mission _could_ be launched at
            // this time.
            bool validLaunch =
                MissionTimingOk(Data->P[plr].Mission[pad].MissionCode,
                                Data->Year, Data->Season);

            if (validLaunch) {
                InBox(169, 181, 238, 193);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                bool conflict = false;

                // Check if there's a Future Mission which would be
                // displaced by delaying the mission.
                if (Data->P[plr].Future[pad].MissionCode) {
                    conflict = true;
                } else if (Data->P[plr].Mission[pad].Joint) {
                    int other = (Data->P[plr].Mission[pad].part) ?
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
            }
        } else if ((Data->P[plr].LaunchFacility[pad] <= Data->P[plr].Cash && Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DAMAGED_MARGIN && x >= 169 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0)
                   || (key == 'F' && Data->P[plr].LaunchFacility[pad] >= LAUNCHPAD_DAMAGED_MARGIN && Data->P[plr].LaunchFacility[pad] <= Data->P[plr].Cash)) {
            // Scrub Mission
            InBox(169, 181, 314, 193);
            key = 0;
            WaitForMouseUp();

            if (Data->P[plr].Cash >= Data->P[plr].LaunchFacility[pad]) {
                temp = Help("i115");

                if (temp == 1) {
                    Data->P[plr].Cash -= Data->P[plr].LaunchFacility[pad];
                    Data->P[plr].LaunchFacility[pad] = LAUNCHPAD_OPERATIONAL;
                }
            } else {
                Help("i116");
            }

            OutBox(169, 181, 314, 193);
            key = 0;

            if (temp == 1) {
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
