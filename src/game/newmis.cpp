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

// This page controls the last-minute confirmation screen just before each launch

#include "newmis.h"

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "Buzz_inc.h"
#include "downgrader.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "hardware.h"
#include "ioexception.h"
#include "mc.h"
#include "mis_c.h"
#include "mission_util.h"
#include "place.h"
#include "port.h"
#include "rush.h"
#include "sdlhelper.h"
#include "state_utils.h"
#include "pace.h"
#include "pbm.h"

struct order Order[7] ;

char Month[12][11] = {
    "JANUARY ", "FEBRUARY ", "MARCH ", "APRIL ", "MAY ", "JUNE ",
    "JULY ", "AUGUST ", "SEPTEMBER ", "OCTOBER ", "NOVEMBER ", "DECEMBER "
};

namespace   // Unnamed namespace part 1
{

void MisOrd(char num);

}; // End of unnamed namespace part 1


/** Function to compare two Missions
 *
 * Orders by date and budget
 *
 * \param
 * \param
 *
 * \return -1 if first date earlier than second
 * \return 0 if date and budget are the same
 * \return 1 if first budget is lower than second
 */
static int cmp_order(const void* p1, const void* p2)
{
    order* o1 = (order*) p1;
    order* o2 = (order*) p2;

    if (o1->date < o2->date) return -1;
    if (o1->date > o2->date) return 1;
    
    if (o1->budget > o2->budget) return -1;
    if (o1->budget < o2->budget) return 1;
    
    // Never randomly reshuffle missions by the same player as
    // this would break mail games; order by launch pad in
    // this case.
    if (o1->plr != o2->plr) (brandom(2) == 1) ? 1 : -1;
    
    if (o1->loc < o2->loc) {
        return -1;
    } else {
        return 1;
    }
}

char OrderMissions()
{
    memset(Order, 0x00, sizeof Order);
    // Sort Missions for Proper Order
    int k = 0;

    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < MAX_MISSIONS; j++) {
            // Don't run the Soviet missions during the U.S. turn
            if ((MAIL == 0 && i == 1) || (MAIL == 3 && i == 0)) continue;
            if (Data->P[i].Mission[j].MissionCode
                && Data->P[i].Mission[j].part != 1) {
                Order[k].plr = i;
                Order[k].loc = j;
                Order[k].budget = Data->P[i].Budget;
                Order[k].date = Data->P[i].Mission[j].Month;
                k++;
            }
        }
    }

    if (k) {
        qsort(Order, k, sizeof(struct order), cmp_order);
    }

    if (MAIL == -1 && Option == -1 && AI[0] == 0 && AI[1] == 0 && k != 0) {
        MisOrd(k);
    }

    return k;
}

namespace   // Unnamed namespace part 2
{

void MisOrd(char num)
{
    ShBox(63, 19, 257, 173);
    InBox(74, 36, 246, 163);
    display::graphics.setForegroundColor(36);
    draw_string(77, 30, "       LAUNCH ORDER");

    for (int i = 0; i < num; i++) {
        InBox(78, 39 + 21 * i, 105, 55 + 21 * i);
        draw_small_flag(Order[i].plr, 79, 40 + 21 * i);
        display::graphics.setForegroundColor(16);
        draw_string(110, 45 + 21 * i, "SCHEDULED LAUNCH");
        draw_string(110, 52 + 21 * i, "DATE: ");
        display::graphics.setForegroundColor(1);

        draw_string(0, 0,
                    Month[Data->P[Order[i].plr].Mission[Order[i].loc].Month]);

        draw_string(0, 0, " 19");
        draw_number(0, 0, Data->Year);
    }

    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    WaitForKeyOrMouseDown();

    WaitForMouseUp();

    FadeOut(2, 10, 0, 0);
}

}; // End of unnamed namespace part 2


void MisAnn(char plr, char pad)
{
    char pad_str[2] = {static_cast<char>('A' + pad), '\0'};

    display::graphics.screen()->clear();

    helpText = "i175";

    PortPal(plr);
    ShBox(41, 20, 281, 184);
    InBox(46, 25, 276, 179);
    InBox(46, 25, 117, 65);
    draw_flag(47, 26, plr);
    InBox(122, 25, 276, 65);
    display::graphics.setForegroundColor(9);
    draw_string(127, 33, "SCHEDULED LAUNCH");  //was 154,33
    display::graphics.setForegroundColor(16);
    draw_string(127, 40, "LAUNCH FACILITY: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "PAD ");
    draw_string(0, 0, pad_str);
    display::graphics.setForegroundColor(16);
    draw_string(127, 47, "DATE: ");
    display::graphics.setForegroundColor(1);

    draw_string(0, 0, Month[Data->P[plr].Mission[pad].Month]);

    draw_string(0, 0, "19");
    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(1);

    mStr plan = GetMissionPlan(Data->P[plr].Mission[pad].MissionCode);

    // Check to ensure there is a docking module in orbit before
    // allowing a docking mission to proceed.
    // This assumes an unmanned docking mission cannot be attempted
    // without including a docking module.
    bool HelpFlag = false;
    if ((plan.mVab[0] & 0x10) == 0x10 &&
        Data->P[plr].DockingModuleInOrbit <= 0) {
        Downgrader::Options options = LoadJsonDowngrades("DOWNGRADES.JSON");
        Downgrader replace{Data->P[plr].Mission[pad], options};
        MissionType downgrade;

        //  Assumes Mission_None is not a docking mission...
        try {
            while (IsDocking(replace.current().MissionCode)) {
                replace.next();
            }

            downgrade = replace.current();
        } catch (IOException &err) {
            // TODO: Can't download to Earth Orbital if Joint mission.
            CAT_CRITICAL(baris, "Error loading mission downgrades: %s",
                       err.what());
            CAT_WARNING(baris, "Defaulting to Manned Earth Orbital.");
            downgrade = Data->P[plr].Mission[pad];
            downgrade.MissionCode = Mission_Earth_Orbital;
            downgrade.Duration = 1;
        }

        Downgrade(plr, pad, downgrade);
        plan = GetMissionPlan(Data->P[plr].Mission[pad].MissionCode);
        HelpFlag = true;
    }

    draw_string(127, 54, (plan.Abbr).c_str());

    // Show duration level only on missions with a Duration step - Leon
    if (IsDuration(Data->P[plr].Mission[pad].MissionCode)) {
        int duration = Data->P[plr].Mission[pad].Duration;
        draw_string(0, 0, GetDurationParens(duration));
    }

    IOBox(57, 68, 118, 84);
    IOBox(129, 68, 195, 84);
    IOBox(205, 68, 266, 84);
    display::graphics.setForegroundColor(1);
    draw_string(65, 78, "CONTINUE");
    draw_string(137, 78, "PLAY FULL");
    draw_string(221, 78, "SCRUB");
    display::graphics.setForegroundColor(9);
    draw_string(65, 78, "C");
    draw_string(137, 78, "P");
    draw_string(221, 78, "S");

    //IOBox(85,68,158,84);IOBox(172,68,245,84);
    //display::graphics.setForegroundColor(1);draw_string(102,78,"CONTINUE");draw_string(189,78,"SCRUB");
    //display::graphics.setForegroundColor(9);
    //draw_string(102,78,"C");draw_string(189,78,"S");

    if (Data->P[plr].Mission[pad].Joint == 0) {
        draw_string(126, 91, "SINGLE LAUNCH");
    } else {
        display::graphics.setForegroundColor(9);
        draw_string(129, 91, "JOINT LAUNCH");
        display::graphics.setForegroundColor(11);
        draw_string(59, 92, "PART 1: ");
        draw_string(0, 0, "PAD ");

        //draw_number(0,0,pad);
        switch (pad) {
        case 0:
            draw_string(0, 0, "A");
            break;

        case 1:
            draw_string(0, 0, "B");
            break;

        case 2:
            draw_string(0, 0, "C");
            break;
        }

        draw_string(201, 92, "PART 2: ");

        draw_string(0, 0, "PAD ");

        //draw_number(0,0,pad+1);
        switch (pad + 1) {
        case 0:
            draw_string(0, 0, "A");
            break;

        case 1:
            draw_string(0, 0, "B");
            break;

        case 2:
            draw_string(0, 0, "C");
            break;
        }

    }

    for (int i = 0; i < Data->P[plr].Mission[pad].Joint + 1; i++) {
        int k = 0; // some kind of y-coordinate variable
        int bud = (i == 0)?59 :168; // some kind of x-coordinate variable

        for (int j = Mission_Capsule; j <= Mission_EVA; j++) {
            int hold = Data->P[plr].Mission[pad + i].Hard[j];

            switch (j) {
            case Mission_Capsule:
                {
                if (hold <= -1) break;
                    
                if (Data->P[plr].Manned[hold].SaveCard > 0) {
                    display::graphics.setForegroundColor(11);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                display::graphics.setForegroundColor(7);
                draw_string(bud, 100 + 14 * k, "CAPSULE: ");
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &Data->P[plr].Manned[hold].Name[0]);
                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");

                if (Data->P[plr].Manned[hold].Damage != 0) display::graphics.setForegroundColor(9);
                else if (Data->P[plr].Manned[hold].Safety < Data->P[plr].Manned[hold].MaxRD) display::graphics.setForegroundColor(11);
                else display::graphics.setForegroundColor(1);

                draw_number(0, 0, Data->P[plr].Manned[hold].Safety +
                            Data->P[plr].Manned[hold].Damage);
                draw_string(0, 0, "%");
                    
                ++k;
                break;
                }

            case Mission_Kicker:
                {
                if (hold <= -1) break;
                    
                if (Data->P[plr].Misc[hold].SaveCard > 0) {
                    display::graphics.setForegroundColor(11);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                display::graphics.setForegroundColor(7);
                draw_string(bud, 100 + 14 * k, "KICKER: ");
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &Data->P[plr].Misc[hold].Name[0]);
                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");

                if (Data->P[plr].Misc[hold].Damage != 0) display::graphics.setForegroundColor(9);
                else if (Data->P[plr].Misc[hold].Safety < Data->P[plr].Misc[hold].MaxRD) display::graphics.setForegroundColor(11);
                else display::graphics.setForegroundColor(1);
                draw_number(0, 0, Data->P[plr].Misc[hold].Safety +
                            Data->P[plr].Misc[hold].Damage);
                draw_string(0, 0, "%");
                ++k;

                break;
                }

            case Mission_LM:
                {
                if (hold <= -1) break;
                    
                if (Data->P[plr].Manned[hold].SaveCard > 0) {
                    display::graphics.setForegroundColor(11);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                display::graphics.setForegroundColor(7);
                draw_string(bud, 100 + 14 * k, "LM: ");
                    
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &Data->P[plr].Manned[hold].Name[0]);
                    
                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");

                if (Data->P[plr].Manned[hold].Damage != 0) display::graphics.setForegroundColor(9);
                else if (Data->P[plr].Manned[hold].Safety < Data->P[plr].Manned[hold].MaxRD) display::graphics.setForegroundColor(11);
                else display::graphics.setForegroundColor(1);
                draw_number(0, 0, Data->P[plr].Manned[hold].Safety +
                            Data->P[plr].Manned[hold].Damage);
                draw_string(0, 0, "%");
                ++k;

                break;
                }

            case Mission_Probe_DM:
                {
//draw_number(150, 116, Data->P[plr].Probe[hold].MaxRD);
                if (hold <= -1 || hold > 4) break;
                auto& Plr = Data->P[plr];
                    
                int SaveCard = (hold < 3)?Plr.Misc[hold].SaveCard
                                         :Plr.Probe[hold].SaveCard;
                if (SaveCard > 0) {
                    display::graphics.setForegroundColor(11);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                    
                display::graphics.setForegroundColor(7);
                const char* str1 = (hold < 3)?"PROBE: "
                                             :"DOCKING: ";
                draw_string(bud, 100 + 14 * k, str1);
                    
                display::graphics.setForegroundColor(1);
                const char* str2 = (hold < 3)?&Plr.Probe[hold].Name[0]
                                             :&Plr.Misc[hold].Name[0];
                draw_string(0, 0, str2);
                    
                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");
                    
                int Damage = (hold < 3)? Plr.Probe[hold].Damage
                                       : Plr.Misc[hold].Damage;
                int Safety = (hold < 3)? Plr.Probe[hold].Safety
                                       : Plr.Misc[hold].Safety;
                int MaxRD = (hold < 3)? Plr.Probe[hold].MaxRD
                                      : Plr.Misc[hold].MaxRD;
                if (Damage != 0) display::graphics.setForegroundColor(9);
                else if (Safety < MaxRD) display::graphics.setForegroundColor(11);
                else display::graphics.setForegroundColor(1);
                draw_number(0, 0, Safety + Damage);
                draw_string(0, 0, "%");
                    
                ++k;
                break;
                }

            case Mission_PrimaryBooster:
                {
                if (hold <= -1) break;
                auto& Rocket = (hold < 5)? Data->P[plr].Rocket[hold - 1]
                                         : Data->P[plr].Rocket[hold - 5];
                auto& Boosters = Data->P[plr].Rocket[ROCKET_HW_BOOSTERS];
                    
                if (Rocket.SaveCard > 0) {
                    display::graphics.setForegroundColor(11);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                display::graphics.setForegroundColor(7);
                draw_string(bud, 100 + 14 * k, "ROCKET: ");
                    
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &Rocket.Name[0]);
                if (hold >= 5) draw_string(0, 0, " W/B");

                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");

                if (Rocket.Damage != 0 || (hold >= 5 && Boosters.Damage != 0)) {
                    display::graphics.setForegroundColor(9);
                } else if (Rocket.Safety < Rocket.MaxRD 
                           || (hold >= 5 && Boosters.Safety < Boosters.MaxRD)) {
                    display::graphics.setForegroundColor(11);
                }
                else display::graphics.setForegroundColor(1);
                if (hold < 5) {
                    draw_number(0, 0, Rocket.Safety + Rocket.Damage);
                } else {
                    draw_number(0, 0, RocketBoosterSafety(Rocket.Safety + Rocket.Damage, Boosters.Safety + Boosters.Damage));
                }
                draw_string(0, 0, "%");
                ++k;
                break;
                }

            case Mission_EVA:
                {
                // EVA suits are added to _all_ manned missions once
                // developed (for emergencies). Only display if needed.
                if (hold <= -1) break;
                if (!IsEVA(Data->P[plr].Mission[pad].MissionCode)) break;

                auto& EVA_Suit = Data->P[plr].Misc[hold];
                if (EVA_Suit.SaveCard > 0) {
                    display::graphics.setForegroundColor(44);
                    draw_string(bud - 8, 100 + 14 * k, "+");
                }
                display::graphics.setForegroundColor(7);
                draw_string(bud, 100 + 14 * k, "EVA: ");
                    
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &EVA_Suit.Name[0]);
                    
                display::graphics.setForegroundColor(11);
                draw_string(bud, 107 + 14 * k, "SAFETY FACTOR: ");

                if (EVA_Suit.Damage != 0) display::graphics.setForegroundColor(9);
                else if (EVA_Suit.Safety < EVA_Suit.MaxRD) display::graphics.setForegroundColor(11);
                else display::graphics.setForegroundColor(1);
                draw_number(0, 0, EVA_Suit.Safety + EVA_Suit.Damage);
                draw_string(0, 0, "%");
                    
                ++k;
                break;
                }

            default:
                break;
            }
        }
    }

    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    // If the mission had a Docking requirement which could not be
    // fulfilled (due to an absent DM), it is automatically downgraded.
    // If there is no viable downgrade, it will have to be scrubbed.
    if (HelpFlag) {
        if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
            Help("i156");
            ScrubMission(plr, pad);
            FadeOut(2, 10, 0, 0);
            return;
        } else {
            Help("i157");
        }
    }

    while (1) {
        key = 0;
        GetMouse();

        if ((x >= 59 && y >= 70 && x <= 116 && y <= 82 && mousebuttons > 0) || key == K_ENTER || key == 'C') {
            InBox(59, 70, 116, 82);
            WaitForMouseUp();
            OutBox(59, 70, 116, 82);
            FadeOut(2, 10, 0, 0);
            fullscreenMissionPlayback = false;
            return;
        } else if ((x >= 131 && y >= 70 && x <= 195 && y <= 82 && mousebuttons > 0) || key == K_ENTER || key == 'P') {
            InBox(131, 70, 193, 82);
            WaitForMouseUp();
            OutBox(131, 70, 193, 82);
            FadeOut(2, 10, 0, 0);
            fullscreenMissionPlayback = true;
            return;
        } else if ((x >= 207 && y >= 70 && x <= 264 && y <= 82 && mousebuttons > 0) || key == 'S') {
            InBox(207, 70, 264, 82);
            WaitForMouseUp();
            OutBox(207, 70, 264, 82);

            if (ScrubMissionQuery(plr, pad)) {
                ScrubMission(plr, pad);
            }

            if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
                FadeOut(2, 10, 0, 0);
                return;
            }
        }
    }
}


void AI_Begin(char plr)
{
    boost::shared_ptr<display::PalettizedSurface> countrySeals(Filesystem::readImage("images/turn.but.0.png"));
    countrySeals->exportPalette();

    display::graphics.screen()->clear();
    ShBox(0, 60, 319, 80);
    display::graphics.setForegroundColor(6 + plr * 3);

    if (plr == 0) {
        draw_heading(15, 64, "DIRECTOR OF THE UNITED STATES", 0, -1);
    } else {
        draw_heading(30, 64, "CHIEF DESIGNER OF THE USSR", 0, -1);
    }

    display::graphics.setForegroundColor(11);
    grMoveTo(175, 122);

    if (Data->Season == 0) {
        draw_string(0, 0, "SPRING 19");
    } else {
        draw_string(0, 0, "FALL 19");
    }

    draw_number(0, 0, Data->Year);
    display::graphics.screen()->draw(countrySeals, 110 * plr, 0, 107, 95, 30, 85);
    display::graphics.setForegroundColor(11);
    draw_string(60, 58, "COMPUTER TURN:  THINKING...");
    music_start(M_SOVTYP);
    FadeIn(2, 10, 0, 0);
}

void AI_Done()
{
    music_stop();
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
}
