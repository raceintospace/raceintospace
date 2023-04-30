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
// Programmed by Michael K McCarty & Morgan Roarty
//
// Pentagon / KGB Headquarters Main Files

// This page handles the Intel Library and CIA/KGB Statistics

#include "intel.h"

#include <cassert>
#include <stdexcept>
#include <numeric>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "gamedata.h"
#include "Buzz_inc.h"
#include "future.h"
#include "draw.h"
#include "game_main.h"
#include "mission_util.h"
#include "place.h"
#include "port.h"
#include "rdplex.h"
#include "mc.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "filesystem.h"
#include "hardware.h"
#include "hardware_buttons.h"
#include "logging.h"


/**
 * Intelligence Briefings
 *
 * The Intel briefings are tracked by 'program' and 'index'. The
 * program keeps track of what class of intel is being reviewed,
 * while the index keeps track of the specific mission, hardware, etc.
 *
 * Program
 * 0: Probes / (Astro/Cosmo)nauts / Launch Facilities
 * 1: Rockets
 * 2: Manned Craft (Capsules, LMs)
 * 3: Misc. Hardware
 * 4: - None -
 * 5: Planned Mission
 *
 * The first four entries correspond to the EquipmentIndex for the
 * most part, which is logical. But while Space Program / Administration
 * stuff like 'nauts and launch facilities would logically be classified
 * as their own program (4), they are lumped in with Probes, and their
 * space left empty.
 *
 * This appears to be connected to how the legacy image storage,
 * INTEL.BUT, stored the briefing images. The position of the image
 * within the file would be calculated based on the program and index
 * values, plus an offset if a USSR player was viewing reports on US
 * programs. Combining Admin & Probes saved disk space in this
 * calculation.
 */


// imported from CODENAME.DAT
const char *code_names[] = {
    "BASEBALL",
    "TUMBLEWEED",
    "BUMBLEBEE",
    "SPIKE",
    "POSTHOLE",
    "BULLET",
    "MECHTA",
    "LUNAYA",
    "PRANDA",
    "BUTTERFLY",
    "BUTTERFLY",
    "BUTTERFLY",
    "TINKERTOY",
    "FOOTBALL",
    "DANDELION",
    "SPIDER",
    "GRASSHOPPER",
    "SPIDER",
    "ERECTOR SET",
    "ERECTOR SET",
    "ERECTOR SET",
    "SQUID",
    "SQUID",
    "SQUID",
    "LEGO",
    "LEGO",
    "LEGO",
    "OCTOPUS",
    "OCTOPUS",
    "TENTACLE",
    "L-7",
    "SANDCASTLE",
    "GANGSTER",
    "SEPTOR",
    "SEPTOR",
    "SEPTOR",
    "INNINGS",
    "SPECTOR",
    "NINE LIVES",
    "TIGER",
    "PANTHER",
    "LION",
    "SAPWOOD",
    "SISYPHUS",
    "SINKHOLE",
    "POTEMKIN",
    "BALLOON",
    "GASBAG",
    "SIX-SHOOTER",
    "SUNBURN",
    "STARFIRE",
    "WAR HAMMER",
    "LIGHTNING",
    "DRAGON",
    "LENIN",
    "SURE-FIRE",
    "SAMSON",
    "THUNDER",
    "VON BRAUN",
    "JUNO VI",
    "SATAN",
    "STUPENDOUS",
    "SUPER",
    "GENGHIS",
    "SIBERIA",
    "JUNO VII",
    "SANDALWOOD",
    "STYX",
    "SIDEARM",
    "TWINS",
    "TWINS",
    "TWINS",
    "CANNONBALL",
    "KAMIKAZE",
    "BRIQUET",
    "SPLASH",
    "SOLO",
    "SHOE HORN",
    "VOLLEYBALL",
    "SLAM DUNK",
    "BATHTUB",
    "DUO",
    "BABUSHKA",
    "DECADENCE",
    "BELL BOTTOM",
    "HAMSTER CAGE",
    "BIKINI",
    "TROIKA",
    "TRIPLET",
    "TRIO",
    "WEDGE",
    "FLIPPER",
    "HAWK",
    "MANTA",
    "DINGHY",
    "ROWBOAT",
    "SPUD",
    "GOLIATH",
    "LONG HAUL",
    "BORSCHT",
    "BABA YAGA",
    "ATILLA",
    "FALCON",
    "TANGO",
    "TWOSOME",
    "BOLSHOI",
    "TARANTULA",
    "BOLSHOI",
    "SOLOIST",
    "JUMPBUG",
    "POGO STICK",
    "SIKORSKY",
    "HUMMINGBIRD",
    "SIKORSKY",
    "NECKLACE",
    "NECKLACE",
    "NECKLACE",
    "STRIKE",
    "STRIKE",
    "STRIKE",
    "BOOMERANG",
    "BOOMERANG",
    "BOOMERANG",
    "RAINBOW",
    "RAINBOW",
    "RAINBOW",
    "MORNING STAR",
    "MORNING STAR",
    "MORNING STAR",
    "SCOUT",
    "SCOUT",
    "SCOUT",
    "LONGSHOT",
    "PASS BALL",
    "LONGSHOT",
    "DAGGER",
    "SWORD",
    "FID",
    "FLEETFOOT",
    "ACHILLES",
    "ROADRUNNER",
    "SEAGULL",
    "SPHINX",
    "HOT FOOT",
    "ADONIS",
    "CUPID",
    "AMOR",
    "RAVEN",
    "EROS",
    "TWIN BROTHER",
    "WARGOD",
    "BARSOON",
    "RADISH",
    "RED STAR",
    "RUBY",
    "GARNET",
    "ZEUS",
    "BIG GUY",
    "GALILEO",
    "ZEUS",
    "FALCON",
    "RED SPOT",
    "HALO",
    "WANDERER",
    "CAROUSEL",
    "PROPHECY",
    "AURORA",
    "JOLTE",
    "OSCAR",
    "SQUIRT",
    "DUSTER",
    "COURIER",
    "CONTACT",
    "IMPACT",
    "DOUBLE",
    "CATAPULT",
    "SNAPSHOT",
    "SLINGSHOT",
    "SLEIGH RIDE",
    "JOURNEY",
    "TRIPLE",
    "LEGEND",
    "LEGEND",
    "CRESCENT",
    "TWILIGHT",
    "PIROUETTE",
    "CHECKERED FLAG",
    "HOME RUN",
    "FIRST AND GOAL",
    "MONUMENT",
    "SWAN SONG",
    "LENIN",
    "",
    "",
    "",
    "",
    "",
    ""
};

struct DisplayContext {
    boost::shared_ptr<display::LegacySurface> intel;
};


inline unsigned int BriefingIndex();
inline unsigned int BriefingIndex(char year, char season);
int MissionIntelFake(char plr);
int MissionIntelReal(char plr);
void MissionIntel(char plr, bool acc);
void XSpec(char plr, char mis, char year);
void Special(char plr, int ind);
void BackIntel(char plr, char year);
void HarIntel(char plr, char acc);
void SaveIntel(char plr, char prg, char ind);
void ImpHard(char plr, char hd, char dx);
void UpDateTable(char plr);
void DrawBre(char plr);
void Bre(char plr);
void DrawIStat(char plr);
void IStat(char plr);
void IInfo(char plr, char loc, char w, const DisplayContext &dctx);

void ClearIntelReportText();
void DrawIntelImage(char plr, char poff);
void DrawIntelBackground();


/* Returns the index into the record of Intel Briefings for the
 * intel gathered the current turn.
 *
 * \return  the index into PastIntel.
 */
unsigned int BriefingIndex()
{
    return Data->Year - 58;
}


/* Returns the index into the record of Intel Briefings for the
 * intel gathered on a given turn.
 *
 * \param year    years since 1900.
 * \param season  0 for Spring, 1 for Fall.
 * \return  the index into PastIntel.
 */
unsigned int BriefingIndex(const char year, const char season)
{
    return year - 58;
}


/**
 * The interface for the main Intel center (CIA/KGB) menu.
 *
 * \param plr  The player viewing the intel.
 */
void Intel(char plr)
{
    char IName[3][22] = {"LIBRARY", "CIA STATISTICS", "EXIT INTELLIGENCE"};
    char IImg[3] = {15, 17, 0};
    int i, beg;

    // FadeOut(2,pal,10,0,0);
    if (plr == 1) {
        music_start(M_INTELLIG);
        strncpy((char *)&IName[1], "KGB", 3);
    } else {
        music_start(M_INTEL);
    }

    beg = 1;

    do {
        if (beg) {
            beg = 0;
        } else {
            FadeOut(2, 10, 0, 0);
            DrawSpaceport(plr);
            FadeIn(2, 10, 0, 0);
        }

        helpText = (plr == 0) ? "i704" : "i705";
        keyHelpText = (plr == 0) ? "k605" : "k606";

        i = BChoice(plr, 3, &IName[0][0], &IImg[0]);

        switch (i) {
        case 1:
            helpText = "i024";
            keyHelpText = "k024";
            Bre(plr);
            break;

        case 2:
            helpText = "i026";
            keyHelpText = "k026";
            IStat(plr);
            break;

        default:
            break;
        }
    } while (i == 1 || i == 2);

    music_stop();
}



/**
 * Generate a fake mission report detailing an era-appropriate mission.
 *
 * While the plr variable is not used, it is reserved in case a future
 * implementation wishes to return nation-specific missions such as the
 * Soyuz Lunar Landing.
 *
 * \param plr  the active player.
 * \return  the mission code or Mission_None on failure.
 */
int MissionIntelFake(char plr)
{
    int era, k, total = 0, roll = 0, sumWeight;
    static const char F[3][15] = {
        {6, 1, 2, 3, 4, 5},  // 58 & 59
        {14, 6, 25, 7, 9, 10, 11, 12, 13, 8, 14, 15, 18, 16},  // 60 to 64
        {11, 43, 38, 48, 53, 54, 55, 56, 42, 49, 50}  // 65 and up
    };
    static const char Weights[3][15] = {
        {6, 3, 1, 1, 2, 2}, // 58 & 59
        {14, 1, 1, 3, 3, 3, 2, 2, 2, 2, 1, 3, 1, 1},  // 60 to 64
        {11, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1}  // 65 and up
    };

    if (Data->Year <= 59) {
        era = 0;
    } else if (Data->Year <= 64) {
        era = 1;
    } else {
        era = 2;
    }

    sumWeight = std::accumulate(&Weights[era][1],
                                Weights[era] + Weights[era][0],
                                0);

    roll = brandom(sumWeight) + 1;
    k = 1;
    total = 0;

    do {
        total += Weights[era][k];
    } while (roll > total && ++k < Weights[era][0]);

    return (k < Weights[era][0]) ? F[era][k] : Mission_None;
}


/**
 * Identify an upcoming mission planned by an opposing player.
 *
 * \param plr  the active player.
 * \return  the mission code or Mission_None if no candidate found.
 */
int MissionIntelReal(char plr)
{
    int save[2 * MAX_MISSIONS];
    int found = 0;

    for (int i = 0; i < MAX_MISSIONS; i++) {
        if (Data->P[other(plr)].Future[i].MissionCode) {
            save[found++] = Data->P[other(plr)].Future[i].MissionCode;
        }
    }

    for (int i = 0; i < MAX_MISSIONS; i++) {
        if (Data->P[other(plr)].Mission[i].MissionCode) {
            save[found++] = Data->P[other(plr)].Mission[i].MissionCode;
        }
    }

    return found ? save[brandom(found)] : Mission_None;
}


/**
 * Create a mission intel report describing a planned launch.
 *
 * \param plr  the active player (0 for USA, 1 for USSR).
 * \param acc  true if the intel is accurate.
 */
void MissionIntel(char plr, bool acc)
{
    bool prevReported = false;
    int mis = Mission_None;

    if (acc) {
        mis = MissionIntelReal(plr);

        if (mis == Mission_None) {
            mis = MissionIntelFake(plr);
        }
    } else {
        mis = MissionIntelFake(plr);
    }

    // Soviet side has 57 missions (Soyuz L.L.)
    assert(mis >= 0 && mis <= 56 + (1 ^ plr));

    for (int i = 0; i < Data->P[plr].PastIntel[0].cur; i++) {
        if (Data->P[plr].PastIntel[i].prog == 5 &&
            Data->P[plr].PastIntel[i].index == mis) {
            prevReported = true;
        }
    }

    // TODO: It would be nice to pass acc to HarIntel, but that sets
    // up the possibility of an infinite loop.
    if (mis == Mission_None || prevReported) {
        HarIntel(plr, 0);
        return;
    }

    SaveIntel(plr, 5, (char) mis);
}

/**
 * Display the Intel Library report for a planned mission.
 *
 * \param plr  the player index
 * \param mis  the mission code (mStr::Index)
 * \param year the year of the intelligence report
 */
void XSpec(char plr, char mis, char year)
{
    const struct mStr plan = GetMissionPlan(mis);
    display::graphics.setForegroundColor(6);
    draw_string(17, 75, "CLASS: ");
    display::graphics.setForegroundColor(9);

    if (plan.Days >= 1) {
        draw_string(39, 81, "MANNED");
    } else {
        draw_string(39, 81, "UNMANNED");
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 96, "TYPE: ");
    display::graphics.setForegroundColor(9);

    if (plan.Jt == 1) {
        draw_string(0, 0, "JOINT LAUNCH");
    } else {
        draw_string(0, 0, "SINGLE LAUNCH");
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 112, "DOCKING: ");
    display::graphics.setForegroundColor(9);

    if (plan.Doc == 1) {
        draw_string(0, 0, "YES");
    } else {
        draw_string(0, 0, "NO");
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 128, "DURATION: ");
    display::graphics.setForegroundColor(9);

    if (plan.Dur >= 1) {
        draw_string(0, 0, "YES");
    } else {
        draw_string(0, 0, "NO");
    }

    display::graphics.setForegroundColor(1);
    draw_string(33, 155, "THE ");

    if (plr == 0) {
        draw_string(0, 0, "CIA ");
    } else {
        draw_string(0, 0, "KGB ");
    }

    draw_string(0, 0, "REPORTS THAT THE ");

    if (plr == 0) {
        draw_string(0, 0, "SOVIET UNION IS");
    } else {
        draw_string(0, 0, "UNITED STATES IS");
    }

    draw_string(33, 169, "PLANNING A ");
    display::graphics.setForegroundColor(9);
    MissionName(mis, 93, 169, 30);
    display::graphics.setForegroundColor(1);
    draw_string(33, 183, "SOMETIME IN THE NEXT YEAR.");
    DrawIntelImage(plr, 37 + Data->P[plr].PastIntel[year].SafetyFactor);
}


/**
 * View an intel report discussing astronaut/cosmonaut recruitment
 * or a launch facility purchase.
 *
 * \param plr  the active player.
 * \param ind  3/4 for a launch pad B/C, 5/6 for recruit class I/II.
 */
void Special(char plr, int ind)
{

    display::graphics.setForegroundColor(6);

    if (ind >= 5) {
        draw_string(17, 96, "GROUP: ");
    } else {
        draw_string(17, 96, "FACILITY: ");
    }

    display::graphics.setForegroundColor(9);

    switch (ind) {
    case 3:
        draw_string(0, 0, "TWO");
        break;

    case 4:
        draw_string(0, 0, "THREE");
        break;

    case 5:
        draw_string(0, 0, "ONE");
        break;

    case 6:
        draw_string(0, 0, "TWO");
        break;

    default:
        break;
    }

    if (ind >= 5) {
        display::graphics.setForegroundColor(6);
        draw_string(17, 112, "STATUS: ");
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "TRAINING");
    } else {
        display::graphics.setForegroundColor(6);
        draw_string(17, 112, "STATUS: ");
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "OPERATIONAL");
    }

    display::graphics.setForegroundColor(1);
    draw_string(33, 155, "THE ");

    if (plr == 0) {
        draw_string(0, 0, "CIA ");
    } else {
        draw_string(0, 0, "KGB ");
    }

    draw_string(0, 0, "REPORTS THAT THE ");

    if (plr == 0) {
        draw_string(0, 0, "SOVIET UNION HAS ");
    } else {
        draw_string(0, 0, "UNITED STATES HAS ");
    }

    draw_string(33, 169, "PURCHASED ");
    draw_string(0, 0, "A NEW");
    display::graphics.setForegroundColor(9);

    if (ind >= 5) {
        draw_string(0, 0, " GROUP OF ");

        if (plr == 0) {
            draw_string(0, 0, "COSMO");
        } else {
            draw_string(0, 0, "ASTRO");
        }

        draw_string(0, 0, "NAUTS ");
    }

    else {
        draw_string(0, 0, " LAUNCH FACILITY ");
    }

    display::graphics.setForegroundColor(1);
    draw_string(33, 183, "FOR ITS SPACE PROGRAM");
    DrawIntelImage(plr, ind);
}


/**
 * View an intelligence report for a given turn.
 *
 * The task of drawing mission briefings is passed off to XSpec()
 * and briefings for Launch facilities or (Astro/Cosmo)nauts is handed
 * off to the Special() function. This method draws the intelligence
 * briefing for hardware projects, which includes opponent probes,
 * rockets, manned craft, and miscellaneous hardware.
 *
 * \param plr   player index.
 * \param year  the report's index (# of years since 1958).
 */
void BackIntel(char plr, char year)
{
    int prg, ind, xc, yc;
    char code, w;

    prg = Data->P[plr].PastIntel[year].prog;
    ind = Data->P[plr].PastIntel[year].index;

    display::graphics.setForegroundColor(6);
    draw_string(17, 37, "CODE: ");
    display::graphics.setForegroundColor(9);
    draw_number(0, 0, Data->P[plr].PastIntel[year].num);
    draw_character(Data->P[plr].PastIntel[year].code);
    draw_string(0, 0, "-");

    // TODO: BackIntel uses the current season even when displaying
    // old intel. Since intel is only collected in the fall (due to
    // PastIntel size limitations), that should be used.
    if (Data->Season == 0) {
        draw_string(0, 0, "S");
    } else {
        draw_string(0, 0, "F");
    }

    if (year < Data->Year - 58) {  // TODO: Should use PastIntel[0].cur
        draw_up_arrow_highlight(137, 42);
    } else {
        draw_up_arrow(137, 42);
    }

    if (year > 0) {
        draw_down_arrow_highlight(137, 95);
    } else {
        draw_down_arrow(137, 95);
    }


    draw_number(0, 0, 58 + year);
    display::graphics.setForegroundColor(6);
    draw_string(17, 51, "CODE NAME: ");
    display::graphics.setForegroundColor(1);
    xc = 39;
    yc = 59;
// CODE NAME GOES HERE
    code = -1;

    if (prg == 0) {
        code = ind;
    } else if (prg == 1) {
        code = ind + 7;
    } else if (prg == 2) {
        code = ind + 12;
    } else if (prg == 5) {
        code = Data->P[plr].PastIntel[year].SafetyFactor - 1;
    }

    if (code == -1) {
        draw_string(xc, yc, "TOP SECRET");
    } else {
        w = Data->P[plr].PastIntel[year].cdex;

        int code_name_index = code * 6 + w;
        assert(code_name_index >= 0);
        assert(code_name_index < (sizeof(code_names) / sizeof(code_names[0])));
        draw_string(xc, yc, code_names[code_name_index]);
    }

    if (prg == 5) {
        XSpec(plr, ind, year);
        return;
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 75, "CLASS: ");
    display::graphics.setForegroundColor(9);

    switch (prg) {
    case 0:
        if (ind >= 5 && ind <= 6) {
            if (plr == 0) {
                draw_string(39, 82, "COSMO");
            } else {
                draw_string(39, 81, "ASTRO");
            }

            draw_string(0, 0, "NAUTS");
        } else if (ind >= 3 && ind <= 4) {
            draw_string(39, 81, "LAUNCH FACILITY");
        } else {
            draw_string(39, 81, "PROBE");
        }

        break;

    case 1:
        draw_string(39, 82, "ROCKET");
        break;

    case 2:
        if (ind < 5) {
            draw_string(39, 81, "CAPSULE");
        } else {
            draw_string(39, 81, "LUNAR MODULE");
        }

        break;

    case 3:
        if (ind < 3) {
            draw_string(39, 81, "KICKER");
        } else if (ind == 3) {
            draw_string(39, 81, "EVA SUITS");
        } else if (ind == 4) {
            draw_string(39, 81, "DOCKING MODULES");
        }

        break;

    default:
        break;
    }

    if (prg == 0 && ind >= 3) {
        Special(plr, ind);
        return;
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 96, "CREW: ");
    display::graphics.setForegroundColor(9);

    if (prg == 2) {
        if (ind >= 0 && ind <= 2) {
            draw_number(0, 0, ind + 1);
        } else if (ind == 3 || ind == 4) {
            draw_number(0, 0, ind);
        } else {
            draw_number(0, 0, ind - 4);
        }

        draw_string(0, 0, " PERSON");
    } else {
        draw_string(0, 0, "NONE");
    }

    display::graphics.setForegroundColor(6);
    draw_string(17, 112, "PROGRAM: ");
    display::graphics.setForegroundColor(9);

    Equipment &hardware = HardwareProgram(abs(plr - 1), prg, ind);
    draw_string(0, 0, &hardware.Name[0]);

    display::graphics.setForegroundColor(6);
    draw_string(17, 128, "DURATION: ");
    display::graphics.setForegroundColor(9);

    if (! hardware.Duration) {
        draw_string(0, 0, "NONE");
    } else {
        draw_number(0, 0, hardware.Duration);
        draw_string(0, 0, " DAYS");
    }

    display::graphics.setForegroundColor(1);
    draw_string(33, 155, "THE ");

    if (plr == 0) {
        draw_string(0, 0, "CIA ");
    } else {
        draw_string(0, 0, "KGB ");
    }

    draw_string(0, 0, "REPORTS THAT THE ");

    if (plr == 0) {
        draw_string(0, 0, "SOVIET UNION IS");
    } else {
        draw_string(0, 0, "UNITED STATES IS");
    }

    draw_string(33, 169, "DEVELOPING THE ");
    display::graphics.setForegroundColor(9);
    draw_string(0, 0, &hardware.Name[0]);
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, " AND RATES THE");
    draw_string(33, 183, "RELIABILITY AT ABOUT ");
    draw_number(0, 0, Data->P[plr].PastIntel[year].SafetyFactor);
    draw_string(0, 0, " PERCENT.");

    if (prg != 5) {
        DrawIntelImage(plr, prg * 7 + ind);
    }
}


/**
 * Create an intel report detailing the opponent's hardware
 * capabilities.
 *
 * An 'inaccurate' report selects a program reasonably plausible for
 * the time period and reports on it. An 'accurate' reports details
 * a program the opponent *does* have. Both generate a random safety
 * rating.
 *
 * \param plr  the active player (0 for USA, 1 for USSR).
 * \param acc  true if the intel report is accurate.
 */
void HarIntel(char plr, char acc)
{
    int mr, i, prg = 0, ind = 0, j = 0, k = 0, save[28], lo = 0, hi = 28, tot = 0, nf = 0, seg = 0;

    static char F[10][11] = {
        {7, 0, 7, 8, 11, 14, 15, 12, 12, 12},  //58
        {4, 3, 5, 17, 12, 12, 12, 12, 12, 12},  //59
        {3, 16, 24, 12, 12, 12, 12, 12, 12, 12},  //60
        {9, 1, 4, 6, 8, 11, 14, 15, 17, 24},  //61
        {10, 1, 2, 4, 6, 8, 11, 15, 16, 17, 18},  //62
        {5, 19, 20, 9, 12, 12, 12, 12, 12, 12},  //63
        {5, 9, 10, 18, 12, 12, 12, 12, 12, 12},  //64
        {5, 9, 10, 18, 12, 12, 12, 12, 12, 12},  //65
        {5, 9, 10, 18, 12, 12, 12, 12, 12, 12}  //66
    };
    static char W[10][15] = {
        {9, 8, 3, 2, 1, 1, 3, 1},  // 58
        {7, 11, 3, 1, 1, 2, 1, 1, 3, 1, 1},  // 59
        {5, 13, 1, 1, 2, 2, 2, 2, 3, 2, 1, 2, 2},  // 60
        {5, 12, 2, 2, 2, 2, 2, 1, 3, 2, 2, 2},  // 61
        {5, 10, 3, 3, 2, 3, 2, 3, 2, 2},  // 62
        {3, 14, 3, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3},  // 63
        {3, 15, 3, 3, 2, 3, 3, 1, 2, 2, 2, 2, 1, 2, 3},  // 64
        {3, 15, 3, 3, 2, 3, 3, 1, 2, 2, 2, 2, 1, 2, 3},  // 65
        {3, 15, 3, 3, 2, 3, 3, 1, 2, 2, 2, 2, 1, 2, 3}  // 66
    };

    for (i = 0; i < 28; i++) {
        save[i] = 0;
    }

    if (acc == 0) {
        switch (Data->Year) {
        case 58:
            for (i = 1; i < F[0][0]; i++) {
                save[F[0][i]] = 1;
            }

            break;

        case 59:
            for (i = 1; i < F[0][0]; i++) {
                save[F[0][i]] = 1;
            }

            for (i = 1; i < F[1][0]; i++) {
                save[F[1][i]] = 1;
            }

            break;

        case 60:
            for (i = 1; i < F[0][0]; i++) {
                save[F[0][i]] = 1;
            }

            for (i = 1; i < F[1][0]; i++) {
                save[F[1][i]] = 1;
            }

            for (i = 1; i < F[2][0]; i++) {
                save[F[2][i]] = 1;
            }

            break;

        case 61:
            for (i = 1; i < F[2][0]; i++) {
                save[F[2][i]] = 1;
            }

            for (i = 1; i < F[3][0]; i++) {
                save[F[3][i]] = 1;
            }

            break;

        case 62:
            for (i = 1; i < F[4][0]; i++) {
                save[F[4][i]] = 1;
            }

            break;

        case 63:
            for (i = 1; i < F[4][0]; i++) {
                save[F[4][i]] = 1;
            }

            for (i = 1; i < F[5][0]; i++) {
                save[F[5][i]] = 1;
            }

            break;

        case 64:
            for (i = 1; i < F[4][0]; i++) {
                save[F[4][i]] = 1;
            }

            for (i = 1; i < F[5][0]; i++) {
                save[F[5][i]] = 1;
            }

            for (i = 1; i < F[6][0]; i++) {
                save[F[6][i]] = 1;
            }

            break;

        case 65:
            for (i = 1; i < F[4][0]; i++) {
                save[F[4][i]] = 1;
            }

            for (i = 1; i < F[5][0]; i++) {
                save[F[5][i]] = 1;
            }

            for (i = 1; i < F[7][0]; i++) {
                save[F[7][i]] = 1;
            }

            break;

        default:
            for (i = 1; i < F[4][0]; i++) {
                save[F[4][i]] = 1;
            }

            for (i = 1; i < F[5][0]; i++) {
                save[F[5][i]] = 1;
            }

            for (i = 1; i < F[8][0]; i++) {
                save[F[8][i]] = 1;
            }

            break;
        }

        i = Data->Year - 58;

        if (Data->Year >= 66) {
            i = 8;
        }

        seg = W[i][0];
        j = brandom(100);
        k = 2;
        nf = 0;
        tot = 0;

        while (nf == 0 && k < W[i][1]) {
            tot = tot + W[i][k] * seg;

            if (j <= tot) {
                nf = 1;
            } else {
                k++;
            }
        }

        nf = 0;
        j = 0;
        tot = 0;
        k = k - 2;

        while (nf == 0 && j < 28) {
            if (tot == k) {
                nf = 1;
            } else if (save[j] == 1) {
                tot++;
            }

            if (nf == 0) {
                j++;
            }
        }

        if (j > 0) {
            j = j - 1;    // adjust
        }

    } else {
        // accurate programs pick one
        for (i = 0; i < 7; i++) {
            if (Data->P[abs(plr - 1)].Probe[i].Num >= 0) {
                save[i] = 1;
            }

            if (Data->P[abs(plr - 1)].Rocket[i].Num >= 0) {
                save[i + 7] = 1;
            }

            if (Data->P[abs(plr - 1)].Manned[i].Num >= 0) {
                save[i + 14] = 1;
            }

            if (Data->P[abs(plr - 1)].Misc[i].Num >= 0) {
                save[i + 21] = 1;
            }
        }

        save[3] = save[4] = save[5] = save[6] = save[12] = save[13] = save[26] = save[27] = 0;

        if (Data->P[abs(plr - 1)].LaunchFacility[1] == 1) {
            save[3] = 1;
        }

        if (Data->P[abs(plr - 1)].LaunchFacility[2] == 1) {
            save[4] = 1;
        }

        if (Data->P[abs(plr - 1)].AstroLevel == 0) {
            save[5] = 1;
        }

        if (Data->P[abs(plr - 1)].AstroLevel == 1) {
            save[6] = 1;
        }

        for (i = lo; i < hi; i++) {
            if (save[i] > 0) {
                j++;    // Check if event is good.
            }
        }

        if (j <= 2) {
            HarIntel(plr, 0);
            return;
        }

        j = brandom(hi - lo);
        k = 0;

        while ((k < (hi - lo)) && (save[j] != 1)) {  // finds candidate
// draw_number(100,5+k*6,j);
            j = brandom(hi - lo);
            k++;
        }

        if (k >= 28) {
            HarIntel(plr, 0);
            return;
        }
    }  // end else

    if (j >= 0 && j < 7) {
        prg = 0;
        ind = j;
    } else if (j >= 7 && j < 14) {
        prg = 1;
        ind = j - 7;
    } else if (j >= 14 && j < 21) {
        prg = 2;
        ind = j - 14;
    } else if (j >= 21 && j < 28) {
        prg = 3;
        ind = j - 21;
    }

    mr = Data->P[plr].PastIntel[0].cur;
    nf = 0;

    for (i = 0; i < mr; i++) {
        if (Data->P[plr].PastIntel[i].prog == prg && Data->P[plr].PastIntel[i].index == ind) {
            nf = 1;
        }
    }

    if (nf == 1 || (prg == 1 && ind == 5) || (prg == 1 && ind == 6) ||
        (prg == 3 && ind == 5) || (prg == 3 && ind == 6)) {
        MissionIntel(plr, false);
        return;
    }

    SaveIntel(plr, prg, ind);
}


/* Clears the Intel image area and draws the intel background layer.
 */
void DrawIntelBackground()
{
    fill_rectangle(153, 32, 310, 131, 0);
    boost::shared_ptr<display::PalettizedSurface> background(Filesystem::readImage("images/intel_background.png"));
    background->exportPalette();
    display::graphics.screen()->draw(background, 153, 32);
}


/* Draws the image for an Intelligence Briefing.
 *
 * Because program indices begin at 0, and png image files are indexed
 * starting at 1 (intel.but.0.png was the intel background) the values
 * do not correspond. This is handled by the function internally.
 * The program/mission offset should be used.
 *
 * \param plr Player side (0 for US, 1 for USSR)
 * \param poff Program/Mission index
 */
void DrawIntelImage(char plr, char poff)
{
    DrawIntelBackground();

    if (poff < 56) {
        if (plr == 1) {
            poff = poff + 28;
        }
    }

    assert(poff >= 0 && poff <= 68);

    char filename[128];
    snprintf(filename,
             sizeof(filename),
             "images/intel.but.%d.png",
             (int)poff + 1);

    // If the image cannot be written, report the error and continue.
    // These images use transparent backgrounds, overlaid on the
    // Intel background. It won't have the right appearance, but
    // it's preferable to crashing.
    boost::shared_ptr<display::PalettizedSurface> image;

    try {
        image = Filesystem::readImage(filename);
    } catch (const std::runtime_error &err) {
        CERROR4(filesys, "error loading %s: %s", filename, err.what());
        return;
    }

    display::graphics.screen()->draw(image, 153, 32);
}


/**
 * Save an Intel Library briefing to record.
 *
 * Each Intel briefing is given a unique ID, its record # within
 * the CIA/KGB's filing system. It takes the form
 *    {num}{code}-{Season}{Year}
 * where Season (F or S) and Year (ex: 62) are the date when the
 * intel was collected. For example, 7183V-F59.
 *
 * Intel reports are stored in BuzzData::PastIntel[].
 * PastIntel structs have seven fields:
 *
 *   char code
 *   int16_t num
 *   char prog
 *   char cur
 *   char index
 *   char SafetyFactor
 *   char cdex
 *
 * 'code' and 'num' are components of the intel briefing's id.
 * 'code' is a random letter and 'num' a random four-digit value.
 *
 * 'prog' records the type of intel recorded. Options are
 *   0: Hardware intel for a probe, or
 *      a new astronaut/cosmonaut recruit class, or
 *      a launch facility purchase.
 *   1: Hardware intel for a rocket program.
 *   2: Hardware intel for a manned program.
 *   3: Hardware intel for a kicker, EVA suit, or docking module.
 *   5: Mission intel (an upcoming launch)
 *
 * PastIntel[0].cur is used to store how many entries of the array
 * have been filled (i.e. its length).
 *
 * 'index' is contextually dependent on 'prog'.
 *   - For hardware intel, it's the program index for the type
 *     specified by 'prog'.
 *   - For mission intel, it's the mission code.
 *   - For a launch facility it's 3 or 4 for Pad B or C, respectively.
 *   - For a class of recruits, 'index' is 5 or 6 for the first or
 *     second class, respectively.
 *
 * 'SafetyFactor' is also contextually dependent on 'prog'.
 *   - For hardware intel, the program's safety rating.
 *   - For mission intel, designates the correct intel image.
 *   - Not used for launch facility / recruit classes.
 *
 * 'cdex' is a random value [0, 5] used as part of an index into
 * the code_names array to determine the briefing's Code Name.
 *
 * TODO: Even 'accurate' hardware reports use a completely random
 * SafetyFactor for the hardware program.
 *
 * \param plr
 * \param prg  the type of intel briefing.
 * \param ind  the program index/mission code/recruit class/facility.
 */
void SaveIntel(char plr, char prg, char ind)
{
    char Op[61] = {
        0, 19, 20, 20, 21, 21, 21, 22, 28, 24, 25,
        23, 26, 27, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 28, 28, 28, 30, 30, 30, 30, 30, 30,
        30, 30, 31, 31, 31, 31, 31
    };
    int mr, safetyFactor, j, k;
    char ky;

    if (prg == 5 && ind == 0) {
        ind = 1;
    }

    mr = brandom(9998) + 1;
    ky = 65 + brandom(26);
    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].code = ky;
    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].num = mr;
    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].prog = prg;
    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].index = ind;
    j = brandom(100);

    // TODO: Why not k = brandom(3)? Or k = brandom(99) / 33?
    if (j < 33) {
        k = 0;
    } else if (j < 66) {
        k = 1;
    } else {
        k = 2;
    }

    if (plr == 1) {
        k = k + 3;
    }

    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].cdex = k;

    if (prg == 5) {
        safetyFactor = Op[ind];
    } else {
        safetyFactor = brandom(22) + 77;
    }

    Data->P[plr].PastIntel[Data->P[plr].PastIntel[0].cur].SafetyFactor = safetyFactor;

    if (prg != 5) {
        Data->P[plr].IntelHardwareTable[prg][ind] = safetyFactor;
    }
}


/**
 * Updates the Intel hardware table.
 *
 * TODO: Explain _why_ the algorithm chosen is used...
 * TODO: This overwrites the previous value, even if that value is a
 * just-researched Intel briefing. Not that one random value is better
 * than the other...
 *
 * \param plr
 * \param hd   The hardware field (per EquipmentIndex enum).
 * \param dx   The hardware program index (per EquipProbeIndex, etc.).
 */
void ImpHard(char plr, char hd, char dx)
{
    // based on the hardware improve safety factor
    Equipment &program = HardwareProgram(plr, hd, dx);
    Data->P[plr].IntelHardwareTable[hd][dx] =
        program.MaxRD - brandom(program.MaxSafety - program.MaxRD);
}


/* Updates the hardware statistics table in the Intelligence section.
 *
 * TODO: Should ImpHard be called with plr or other(plr)?
 * If plr, document the reasoning behind the choice.
 *
 * \param plr  Player side (0 for USA, 1 for USSR)
 */
void UpDateTable(char plr)
{
    // based on prestige
    char j, p;
    p = other(plr);

    if (Data->P[p].LMpts > 0) {
        j = brandom(100);

        if (j < 60) {
            ImpHard(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_MODULE);
        } else {
            ImpHard(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_MODULE);
        }
    }

    for (int i = 0; i < MAXIMUM_PRESTIGE_NUM; i++) {
        if (Data->Prestige[i].Place == p || Data->Prestige[i].mPlace == p) {
            switch (i) {
            case Prestige_OrbitalSatellite:
                ImpHard(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
                break;

            case Prestige_LunarFlyby:
            case Prestige_MercuryFlyby:
            case Prestige_VenusFlyby:
            case Prestige_MarsFlyby:
            case Prestige_JupiterFlyby:
            case Prestige_SaturnFlyby:
                ImpHard(plr, PROBE_HARDWARE, PROBE_HW_INTERPLANETARY);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
                break;

            case Prestige_LunarProbeLanding:
                ImpHard(plr, PROBE_HARDWARE, PROBE_HW_LUNAR);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
                break;

            case Prestige_OnePerson:
                ImpHard(plr, MANNED_HARDWARE, MANNED_HW_ONE_MAN_CAPSULE);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_TWO_STAGE);
                break;

            case Prestige_TwoPerson:
                ImpHard(plr, MANNED_HARDWARE, MANNED_HW_TWO_MAN_CAPSULE);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
                break;

            case Prestige_ThreePerson:
                ImpHard(plr, MANNED_HARDWARE, MANNED_HW_THREE_MAN_CAPSULE);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
                break;

            case Prestige_Minishuttle:
                ImpHard(plr, MANNED_HARDWARE, MANNED_HW_MINISHUTTLE);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_THREE_STAGE);
                break;

            case Prestige_FourPerson:
                ImpHard(plr, MANNED_HARDWARE, MANNED_HW_FOUR_MAN_CAPSULE);
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_MEGA_STAGE);
                break;

            case Prestige_MannedOrbital:
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
                break;

            case Prestige_MannedLunarPass:
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
                j = brandom(100);

                if (j < 70) {
                    ImpHard(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
                } else {
                    ImpHard(plr, MISC_HARDWARE, MISC_HW_KICKER_A);
                }

                break;

            case Prestige_MannedLunarOrbit:
                ImpHard(plr, ROCKET_HARDWARE, ROCKET_HW_BOOSTERS);
                j = brandom(100);

                if (j < 70) {
                    ImpHard(plr, MISC_HARDWARE, MISC_HW_KICKER_B);
                } else {
                    ImpHard(plr, MISC_HARDWARE, MISC_HW_KICKER_A);
                }

                break;

            case Prestige_MannedDocking:
                ImpHard(plr, MISC_HARDWARE, MISC_HW_DOCKING_MODULE);
                break;

            case Prestige_Spacewalk:
            case Prestige_MannedSpaceMission:
                ImpHard(plr, MISC_HARDWARE, MISC_HW_EVA_SUITS);
                break;

            default:
                break;
            }  // switch
        }  // if
    }  // for
}


/* Updates the current intelligence information for a player about
 * opponent plans and capabilities.
 *
 * Intelligence gathering consists of 1) generating a new Intel briefing
 * about the opponent's mission plans, hardware status, or space
 * program development (such as hiring astronauts or building launch
 * facilities), and 2) updating the CIA/KGB statistics table.
 *
 * Intelligence gathered about opponent plans and/or capabilities is
 * notoriously unreliable, being slightly more than half accurate on
 * lower difficulty levels and less accurate on higher.
 *
 * Due to limited space for saving intelligence briefings, they can
 * only be generated at a rate of one each year, so updating is
 * disabled in the spring.
 *
 * \param plr  Player side (0 for USA, 1 for USSR)
 * \param pt   vaguely named & never used, with no clues to purpose.
 */
void IntelPhase(char plr, char pt)
{
    int i, splt, acc, Plr_Level, Acc_Coef;

    if (Data->Year == 57 || (Data->Year == 58 && Data->Season == 0)) {
        return;
    }

    // Protect against Spring intel phases, because the intel system
    // assumes all intelligence briefings are created in the fall.
    if (Data->Season == 0) {
        return;
    }

    // Protect against repeating Intel phase (only 30 briefing slots!).
    // Each intelligence briefing is generated in the fall post-1957,
    // which gives an index it should occupy. The Intel system uses
    // Data->P[plr].PastIntel[0].cur to track the index of the
    if (BriefingIndex() < Data->P[plr].PastIntel[0].cur) {
        return;
    }

    splt = brandom(1000);
    i = brandom(1000);

    if (plr == 0) {
        Plr_Level = Data->Def.Lev1;
    } else {
        Plr_Level = Data->Def.Lev2;
    }

    // stagger accuracy for player levels
    if (Plr_Level == 0) {
        Acc_Coef = 600;
    } else if (Plr_Level == 1) {
        Acc_Coef = 500;
    } else {
        Acc_Coef = 400;
    }

    if (i < Acc_Coef) {
        acc = 1;
    } else {
        acc = 0;    // 40% accurate
    }

    if (splt < 500) {
        HarIntel(plr, acc);
    } else {
        MissionIntel(plr, acc);
    }

    Data->P[plr].PastIntel[0].cur++;
    UpDateTable(plr);
}


/**
 * Draws the standard background layout for the Intelligence Library
 * screen.
 *
 * Draws the Library interface including buttons and text but
 * excluding entry-specific text.
 *
 * \param plr  the active player's country index.
 */
void DrawBre(char plr)
{
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(242, 3, 315, 19);
    ShBox(0, 24, 319, 199);
    fill_rectangle(5, 28, 314, 195, 0);
    InBox(4, 27, 315, 196);
    ShBox(150, 29, 313, 133);
    InBox(152, 31, 310, 131);
    fill_rectangle(153, 32, 310, 131, 0);
    ShBox(132, 29, 148, 80);
    ShBox(132, 82, 148, 133);
    ShBox(6, 29, 130, 41);
    InBox(8, 31, 128, 39);  // code
    ShBox(6, 43, 130, 63);
    InBox(8, 45, 128, 61);  // code name
    ShBox(6, 65, 130, 85);
    InBox(8, 67, 128, 83);  // class
    ShBox(6, 87, 130, 101);
    InBox(8, 89, 128, 99);  // crew
    ShBox(6, 103, 130, 118);
    InBox(8, 105, 128, 116);  // program
    ShBox(6, 120, 130, 133);
    InBox(8, 122, 128, 131);  // duration
    IOBox(133, 30, 147, 79);
    IOBox(133, 83, 147, 132);
    ShBox(6, 135, 313, 194);
    fill_rectangle(7, 136, 312, 193, 9);
    InBox(11, 139, 308, 190);
    fill_rectangle(12, 140, 307, 189, 7);
    draw_heading(33, 5, "INTELLIGENCE BRIEFING", 1, -1);
    draw_small_flag(plr, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(256, 13, "CONTINUE");
    draw_up_arrow(137, 42);
    draw_down_arrow(137, 95);
}


/**
 * Clear the text in the Intel Library screen so a new report may be
 * displayed.
 *
 * Scrub the entered data from the Briefing template for reuse.
 * This method is quicker than redrawing the entire Library template,
 * as done in DrawBre. The values used depend upon the specific
 * positioning of text strings in other draw functions, and may need
 * adjusting if those are altered.
 *
 * This does not clear the briefing image, which is handled
 * separately.
 */
void ClearIntelReportText()
{
    fill_rectangle(45, 32, 98, 38, 3);
    fill_rectangle(38, 54, 127, 60, 3);
    fill_rectangle(38, 76, 127, 82, 3);
    fill_rectangle(16, 91, 127, 98, 3);
    fill_rectangle(16, 107, 127, 113, 3);
    fill_rectangle(16, 123, 127, 129, 3);
    fill_rectangle(30, 148, 300, 186, 7);
}


/**
 * Interface for the Intel Library.
 *
 * Navigates the Intel briefings, ordered chronologically.
 *
 * \param plr  the active player index.
 */
void Bre(char plr)
{
    int year = Data->P[plr].PastIntel[0].cur - 1;
    DrawBre(plr);
    DrawIntelBackground();  // just the blue background
    BackIntel(plr, year);
    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {  /* Gameplay */
            if ((x >= 135 && y > 32 && x <= 145 && y <= 77 && mousebuttons > 0) || key == UP_ARROW) {
                InBox(135, 32, 145, 77);
                WaitForMouseUp();

                if (year >= 0 && year + 1 <= Data->P[plr].PastIntel[0].cur - 1) {
                    ClearIntelReportText();
                    year++;
                    BackIntel(plr, year);
                }

                OutBox(135, 32, 145, 77);

            } else if (key == K_HOME) {
                ClearIntelReportText();
                year = Data->Year - 58;
                BackIntel(plr, year);

            } else if ((x >= 135 && y > 85 && x <= 145 && y <= 130 && mousebuttons > 0) || key == DN_ARROW) {
                InBox(135, 85, 145, 130);
                WaitForMouseUp();

                if (year - 1 >= 0) {
                    ClearIntelReportText();
                    year--;
                    BackIntel(plr, year);
                }

                OutBox(135, 85, 145, 130);

            } else if (key == K_END) {
                ClearIntelReportText();
                year = 0;
                BackIntel(plr, year);

            } else if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
                InBox(244, 5, 313, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(244, 5, 313, 17);
                break;  /* Done */
            }
        }
    }
}


/**
 * Load the CIA hardware icons into a buffer image.
 *
 * \throws runtime_error  if Filesystem is unable to load the sprite.
 */
boost::shared_ptr<display::LegacySurface> LoadCIASprite()
{
    boost::shared_ptr<display::LegacySurface> surface(new display::LegacySurface(320, 200));

    std::string filename("images/cia.but.0.png");
    boost::shared_ptr<display::PalettizedSurface> sprite =
        Filesystem::readImage(filename);

    // try {
    //     sprite = Filesystem::readImage(filename);
    // } catch (const std::runtime_error &err) {
    //     CERROR4(filesys, "Error loading %s: %s", filename.c_str(),
    //             err.what());
    //     return;
    // }

    surface->palette().copy_from(sprite->palette());
    surface->draw(sprite, 0, 0);
    return surface;
}


/**
 * Draw the CIA/KGB Statistics screen (excluding the hardware buttons).
 */
void DrawIStat(char plr)
{
    int i;

    FadeOut(2, 10, 0, 0);

    display::graphics.screen()->clear();

    ShBox(0, 0, 319, 199);
    IOBox(242, 3, 315, 19);
    InBox(3, 22, 316, 167);
    InBox(3, 3, 30, 19);

    ShBox(0, 160, 319, 199);
    IOBox(5, 162, 77, 197);
    IOBox(81, 162, 158, 197);
    IOBox(162, 162, 239, 197);
    IOBox(243, 162, 315, 197);
    GradRect(4, 23, 315, 159, 0);

    for (i = 4; i < 316; i += 2) {
        display::graphics.legacyScreen()->setPixel(i, 57, 11);
        display::graphics.legacyScreen()->setPixel(i, 91, 11);
        display::graphics.legacyScreen()->setPixel(i, 125, 11);
    }

    display::graphics.setForegroundColor(9);
    draw_number(5, 55, 75);
    draw_string(17, 55, "%");
    draw_number(5, 89, 50);
    draw_string(17, 89, "%");
    draw_number(5, 123, 25);
    draw_string(17, 123, "%");

    display::graphics.setForegroundColor(6);
    draw_heading(40, 5, "INTELLIGENCE STATS", 1, -1);
    draw_small_flag(plr, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(256, 13, "CONTINUE");
    FadeIn(2, 10, 0, 0);

}


/**
 * Interface function for the CIA/KGB Statistics screen.
 */
void IStat(char plr)
{
    int place = -1;
    DisplayContext dctx;

    HardwareButtons hardware_buttons(165, plr);

    dctx.intel = LoadCIASprite();
    DrawIStat(plr);
    hardware_buttons.drawButtons();

    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) {  /* Gameplay */
            if (((x >= 7 && y >= 164 && x <= 75 && y <= 195 && mousebuttons > 0) || key == 'U') && place != 0) {
                InBox(7, 164, 75, 195);
                WaitForMouseUp();
                OutBox(7, 164, 75, 195);
                place = 0;
                hardware_buttons.drawButtons(place);
                IInfo(plr, place, 0, dctx);
                /* Unmanned */
            }

            if (((x >= 83 && y >= 164 && x <= 156 && y <= 195 && mousebuttons > 0) || key == 'R') && place != 1) {
                InBox(83, 164, 156, 195);
                WaitForMouseUp();
                OutBox(83, 164, 156, 195);
                place = 1;
                hardware_buttons.drawButtons(place);
                IInfo(plr, place, 0, dctx);
                /* Rocket */
            }

            if (((x >= 164 && y >= 164 && x <= 237 && y <= 195 && mousebuttons > 0) || key == 'C') && place != 2) {
                InBox(164, 164, 237, 195);
                WaitForMouseUp();
                OutBox(164, 164, 237, 195);
                /* Manned */
                place = 2;
                hardware_buttons.drawButtons(place);
                IInfo(plr, place, 0, dctx);
            }

            if (((x >= 245 && y >= 164 && x <= 313 && y <= 195 && mousebuttons > 0) || key == 'M') && place != 3) {
                InBox(245, 164, 313, 195);
                WaitForMouseUp();
                OutBox(245, 164, 313, 195);
                place = 3;
                hardware_buttons.drawButtons(place);
                IInfo(plr, place, 0, dctx);
                /* Misc */
            }

            if ((x >= 244 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
                InBox(244, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(244, 5, 314, 17);
                break;  /* Done */
            }
        }
    }
}


/**
 * Copies an image segment from the CIA sprite to the display buffer.
 *
 * \param dctx  buffer with the CIA hardware sprite
 * \param x1    top-left x coordinate of the image in the sprite
 * \param y1    top-left y coordinate of the image in the sprite
 * \param x2    bottom-right x coordinate of the image in the sprite
 * \param y2    bottom-right y coordinate of the image in the sprite
 * \param s     top-left x coordinate of the destination in the display
 * \param t     top-left y coordinate of the destination in the display
 */
void DispIt(const DisplayContext &dctx, int x1, int y1, int x2, int y2, int s, int t)
{
    int w;
    int h;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    display::LegacySurface local(w, h);
    local.copyFrom(dctx.intel.get(), x1, y1, x2, y2, 0, 0);
    local.setTransparentColor(0);
    display::graphics.screen()->draw(local, s, t);
}


/**
 * Illustrate the known hardware statistics in the CIA/KGB Statistics
 * view.
 *
 * Draws a chart showing the known safety factors of the existing
 * hardware programs. Values shown for a side's own hardware are the
 * current value, while safety values for opponent's hardware are
 * pulled from intelligence findings (and thus may be inaccurate).
 *
 * Because the chart has to handle a side-by-side comparison of US
 * and USSR hardware, LMs are displayed with Probes rather than
 * Manned craft. Otherwise, Probes would have 6 items to display
 * while Manned craft would have 14. As rocket illustrations are
 * especially large, rocket boosters are shown in Miscellaneous.
 *
 * \param plr   the active player's country index.
 * \param loc   hardware class to display (per EquipmentIndex enum).
 * \param w     false if the graph area should be redrawn.
 * \param dctx  the sprite with the hardware models.
 */
void IInfo(char plr, char loc, char w, const DisplayContext &dctx)
{
    int i, sfu, sfs;

    // Redraw the chart background
    if (w == 0) {
        GradRect(4, 23, 315, 159, 0);

        for (i = 4; i < 316; i += 2) {
            display::graphics.legacyScreen()->setPixel(i, 57, 11);
            display::graphics.legacyScreen()->setPixel(i, 91, 11);
            display::graphics.legacyScreen()->setPixel(i, 125, 11);
        }

        display::graphics.setForegroundColor(9);
        draw_number(5, 55, 75);
        draw_string(17, 55, "%");
        draw_number(5, 89, 50);
        draw_string(17, 89, "%");
        draw_number(5, 123, 25);
        draw_string(17, 123, "%");
    }

    display::graphics.setForegroundColor(1);

    switch (loc) {
    case ROCKET_HARDWARE:  //draw_string(137,150,"ROCKETS");
        for (i = 0; i < 4; i++) {
            sfu = -1;
            sfs = -1;

            if (plr == 0) {
                if (Data->P[0].Rocket[i].Num >= 0) {
                    sfu = Data->P[0].Rocket[i].Safety;
                }

                sfs = Data->P[0].IntelHardwareTable[ROCKET_HARDWARE][i];
            } else if (plr == 1) {
                if (Data->P[1].Rocket[i].Num >= 0) {
                    sfs = Data->P[1].Rocket[i].Safety;
                }

                sfu = Data->P[1].IntelHardwareTable[ROCKET_HARDWARE][i];
            }

            switch (i) {
            case ROCKET_HW_ONE_STAGE:
                if (sfu > 0) {
                    fill_rectangle(19, 159 - sfu * 136 / 100, 27, 159, 6);
                    fill_rectangle(19, 159 - sfu * 136 / 100, 26, 158, 5);
                    DispIt(dctx, 101, 1, 115, 57, 11, 104);
                }

                if (sfs > 0) {
                    fill_rectangle(50, 159 - sfs * 136 / 100, 58, 159, 9);
                    fill_rectangle(50, 159 - sfs * 136 / 100, 57, 158, 8);
                    DispIt(dctx, 125, 1, 149, 85, 33, 75);
                }

                break;

            case ROCKET_HW_TWO_STAGE:
                if (sfu > 0) {
                    fill_rectangle(78, 159 - sfu * 136 / 100, 86, 159, 6);
                    fill_rectangle(78, 159 - sfu * 136 / 100, 85, 158, 5);
                    DispIt(dctx, 115, 0, 124, 68, 73, 92);
                }

                if (sfs > 0) {
                    fill_rectangle(103, 159 - sfs * 136 / 100, 111, 159, 9);
                    fill_rectangle(103, 159 - sfs * 136 / 100, 110, 158, 8);
                    DispIt(dctx, 151, 1, 170, 95, 88, 65);
                }

                break;

            case ROCKET_HW_THREE_STAGE:
                if (sfu > 0) {
                    fill_rectangle(159, 159 - sfu * 136 / 100, 167, 159, 6);
                    fill_rectangle(159, 159 - sfu * 136 / 100, 166, 158, 5);
                    DispIt(dctx, 172, 1, 209, 133, 130, 27);
                }

                if (sfs > 0) {
                    fill_rectangle(200, 159 - sfs * 136 / 100, 208, 159, 9);
                    fill_rectangle(200, 159 - sfs * 136 / 100, 207, 158, 8);
                    DispIt(dctx, 211, 1, 243, 133, 172, 27);
                }

                break;

            case ROCKET_HW_MEGA_STAGE:
                if (sfu > 0) {
                    fill_rectangle(260, 159 - sfu * 136 / 100, 268, 159, 6);
                    fill_rectangle(260, 159 - sfu * 136 / 100, 267, 158, 5);
                    DispIt(dctx, 245, 1, 285, 137, 231, 23);
                }

                if (sfs > 0) {
                    fill_rectangle(302, 159 - sfs * 136 / 100, 310, 159, 9);
                    fill_rectangle(302, 159 - sfs * 136 / 100, 309, 158, 8);
                    DispIt(dctx, 287, 1, 318, 132, 274, 28);
                }

                break;

            default:
                break;
            }
        }

        break;

    case MANNED_HARDWARE:  // draw_string(137,150,"CAPSULES");
        for (i = 0; i < 5; i++) {
            sfu = -1;
            sfs = -1;

            if (plr == 0) {
                if (Data->P[0].Manned[i].Num >= 0) {
                    sfu = Data->P[0].Manned[i].Safety;
                }

                sfs = Data->P[0].IntelHardwareTable[MANNED_HARDWARE][i];
            } else if (plr == 1) {
                if (Data->P[1].Manned[i].Num >= 0) {
                    sfs = Data->P[1].Manned[i].Safety;
                }

                sfu = Data->P[1].IntelHardwareTable[MANNED_HARDWARE][i];
            }

            switch (i) {
            case MANNED_HW_ONE_MAN_CAPSULE:
                if (sfu > 0) {
                    fill_rectangle(13, 159 - sfu * 136 / 100, 21, 159, 6);
                    fill_rectangle(13, 159 - sfu * 136 / 100, 20, 158, 5);
                    DispIt(dctx, 12, 91, 25, 116, 11, 137);
                }

                if (sfs > 0) {
                    fill_rectangle(41, 159 - sfs * 136 / 100, 49, 159, 9);
                    fill_rectangle(41, 159 - sfs * 136 / 100, 48, 158, 8);
                    DispIt(dctx, 0, 56, 26, 89, 27, 123);
                }

                break;

            case MANNED_HW_TWO_MAN_CAPSULE:
                if (sfu > 0) {
                    fill_rectangle(70, 159 - sfu * 136 / 100, 78, 159, 6);
                    fill_rectangle(70, 159 - sfu * 136 / 100, 77, 158, 5);
                    DispIt(dctx, 27, 98, 49, 127, 59, 127);
                }

                if (sfs > 0) {
                    fill_rectangle(97, 159 - sfs * 136 / 100, 105, 159, 9);
                    fill_rectangle(97, 159 - sfs * 136 / 100, 104, 158, 8);
                    DispIt(dctx, 28, 62, 49, 96, 84, 122);
                }

                break;

            case MANNED_HW_THREE_MAN_CAPSULE:
                if (sfu > 0) {
                    fill_rectangle(132, 159 - sfu * 136 / 100, 140, 159, 6);
                    fill_rectangle(132, 159 - sfu * 136 / 100, 139, 158, 5);
                    DispIt(dctx, 95, 77, 117, 127, 117, 106);
                }

                if (sfs > 0) {
                    fill_rectangle(174, 159 - sfs * 136 / 100, 182, 159, 9);
                    fill_rectangle(174, 159 - sfs * 136 / 100, 181, 158, 8);
                    DispIt(dctx, 119, 97, 170, 140, 144, 113);
                }

                break;

            case MANNED_HW_MINISHUTTLE:
                if (sfu > 0) {
                    fill_rectangle(210, 159 - sfu * 136 / 100, 218, 159, 6);
                    fill_rectangle(210, 159 - sfu * 136 / 100, 217, 158, 5);
                    DispIt(dctx, 3, 1, 16, 54, 203, 103);
                }

                if (sfs > 0) {
                    fill_rectangle(232, 159 - sfs * 136 / 100, 240, 159, 9);
                    fill_rectangle(232, 159 - sfs * 136 / 100, 239, 158, 8);
                    DispIt(dctx, 18, 1, 32, 48, 223, 109);
                }

                break;

            case MANNED_HW_FOUR_MAN_CAPSULE:
                if (sfu > 0) {
                    fill_rectangle(269, 159 - sfu * 136 / 100, 277, 159, 6);
                    fill_rectangle(269, 159 - sfu * 136 / 100, 276, 158, 5);
                    DispIt(dctx, 34, 1, 65, 60, 248, 97);
                }

                if (sfs > 0) {
                    fill_rectangle(305, 159 - sfs * 136 / 100, 313, 159, 9);
                    fill_rectangle(305, 159 - sfs * 136 / 100, 312, 158, 8);
                    DispIt(dctx, 67, 1, 100, 60, 281, 97);
                }

                break;

            default:
                break;
            }
        }

        break;

    case PROBE_HARDWARE:  // draw_string(100,150,"SATELLITES & LM'S");
        sfu = -1;
        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Manned[MANNED_HW_ONE_MAN_MODULE].Num >= 0) {
                sfu = Data->P[0].Manned[MANNED_HW_ONE_MAN_MODULE].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MANNED_HARDWARE][MANNED_HW_ONE_MAN_MODULE];
        } else if (plr == 1) {
            if (Data->P[1].Manned[MANNED_HW_ONE_MAN_MODULE].Num >= 0) {
                sfs = Data->P[1].Manned[MANNED_HW_ONE_MAN_MODULE].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MANNED_HARDWARE][MANNED_HW_ONE_MAN_MODULE];
        }

        if (sfu > 0) {
            fill_rectangle(25, 159 - sfu * 136 / 100, 33, 159, 6);
            fill_rectangle(25, 159 - sfu * 136 / 100, 32, 158, 5);
            DispIt(dctx, 60, 153, 88, 176, 9, 132);
        }

        if (sfs > 0) {
            fill_rectangle(61, 159 - sfs * 136 / 100, 69, 159, 9);
            fill_rectangle(61, 159 - sfs * 136 / 100, 68, 158, 8);
            DispIt(dctx, 31, 153, 56, 182, 41, 126);
        }

        sfu = -1;

        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Manned[MANNED_HW_TWO_MAN_MODULE].Num >= 0) {
                sfu = Data->P[0].Manned[MANNED_HW_TWO_MAN_MODULE].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE];
        } else if (plr == 1) {
            if (Data->P[1].Manned[MANNED_HW_TWO_MAN_MODULE].Num >= 0) {
                sfs = Data->P[1].Manned[MANNED_HW_TWO_MAN_MODULE].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MANNED_HARDWARE][MANNED_HW_TWO_MAN_MODULE];
        }

        if (sfu > 0) {
            fill_rectangle(101, 159 - sfu * 136 / 100, 109, 159, 6);
            fill_rectangle(101, 159 - sfu * 136 / 100, 108, 158, 5);
            DispIt(dctx, 1, 153, 29, 182, 83, 128);
        }

        if (sfs > 0) {
            fill_rectangle(132, 159 - sfs * 136 / 100, 140, 159, 9);
            fill_rectangle(132, 159 - sfs * 136 / 100, 139, 158, 8);
            DispIt(dctx, 90, 151, 119, 176, 112, 131);
        }

        for (i = 0; i < 3; i++) {
            sfu = -1;
            sfs = -1;

            if (plr == 0) {
                if (Data->P[0].Probe[i].Num >= 0) {
                    sfu = Data->P[0].Probe[i].Safety;
                }

                sfs = Data->P[0].IntelHardwareTable[PROBE_HARDWARE][i];
            } else if (plr == 1) {
                if (Data->P[1].Probe[i].Num >= 0) {
                    sfs = Data->P[1].Probe[i].Safety;
                }

                sfu = Data->P[1].IntelHardwareTable[PROBE_HARDWARE][i];
            }

            switch (i) {
            case PROBE_HW_ORBITAL:
                if (sfu > 0) {
                    fill_rectangle(152, 159 - sfu * 136 / 100, 160, 159, 6);
                    fill_rectangle(152, 159 - sfu * 136 / 100, 159, 158, 5);
                    DispIt(dctx, 58, 180, 71, 196, 147, 138);
                }

                if (sfs > 0) {
                    fill_rectangle(173, 159 - sfs * 136 / 100, 181, 159, 9);
                    fill_rectangle(173, 159 - sfs * 136 / 100, 180, 158, 8);
                    DispIt(dctx, 73, 180, 89, 195, 165, 139);
                }

                break;

            case PROBE_HW_INTERPLANETARY:
                if (sfu > 0) {
                    fill_rectangle(212, 159 - sfu * 136 / 100, 220, 159, 6);
                    fill_rectangle(212, 159 - sfu * 136 / 100, 219, 158, 5);
                    DispIt(dctx, 91, 178, 115, 195, 198, 139);
                }

                if (sfs > 0) {
                    fill_rectangle(237, 159 - sfs * 136 / 100, 245, 159, 9);
                    fill_rectangle(237, 159 - sfs * 136 / 100, 244, 158, 8);
                    DispIt(dctx, 153, 142, 176, 166, 227, 132);
                }

                break;

            case PROBE_HW_LUNAR:
                if (sfu > 0) {
                    fill_rectangle(272, 159 - sfu * 136 / 100, 280, 159, 6);
                    fill_rectangle(272, 159 - sfu * 136 / 100, 279, 158, 5);
                    DispIt(dctx, 121, 142, 151, 166, 253, 132);
                }

                if (sfs > 0) {
                    fill_rectangle(302, 159 - sfs * 136 / 100, 310, 159, 9);
                    fill_rectangle(302, 159 - sfs * 136 / 100, 309, 158, 8);
                    DispIt(dctx, 178, 142, 201, 160, 284, 138);
                }

                break;

            default:
                break;
            }
        }

        break;

    case MISC_HARDWARE:  // draw_string(100,150,"ADDITIONAL PROGRAMS");
        sfu = -1;
        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Misc[MISC_HW_EVA_SUITS].Num >= 0) {
                sfu = Data->P[0].Misc[MISC_HW_EVA_SUITS].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MISC_HARDWARE][MISC_HW_EVA_SUITS];
        } else if (plr == 1) {
            if (Data->P[1].Misc[MISC_HW_EVA_SUITS].Num >= 0) {
                sfs = Data->P[1].Misc[MISC_HW_EVA_SUITS].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MISC_HARDWARE][MISC_HW_EVA_SUITS];
        }

        if (sfu > 0) {
            fill_rectangle(19, 159 - sfu * 136 / 100, 27, 159, 6);
            fill_rectangle(19, 159 - sfu * 136 / 100, 26, 158, 5);
            DispIt(dctx, 68, 65, 76, 75, 17, 145);
        }

        if (sfs > 0) {
            fill_rectangle(30, 159 - sfs * 136 / 100, 38, 159, 9);
            fill_rectangle(30, 159 - sfs * 136 / 100, 37, 158, 8);
            DispIt(dctx, 78, 65, 86, 75, 31, 145);
        }

        sfu = -1;

        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Misc[MISC_HW_DOCKING_MODULE].Num >= 0) {
                sfu = Data->P[0].Misc[MISC_HW_DOCKING_MODULE].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MISC_HARDWARE][MISC_HW_DOCKING_MODULE];
        } else if (plr == 1) {
            if (Data->P[1].Misc[MISC_HW_DOCKING_MODULE].Num >= 0) {
                sfs = Data->P[1].Misc[MISC_HW_DOCKING_MODULE].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MISC_HARDWARE][MISC_HW_DOCKING_MODULE];
        }

        if (sfu > 0) {
            fill_rectangle(72, 159 - sfu * 136 / 100, 80, 159, 6);
            fill_rectangle(72, 159 - sfu * 136 / 100, 79, 158, 5);
            DispIt(dctx, 88, 62, 100, 75, 64, 143);
        }

        if (sfs > 0) {
            fill_rectangle(91, 159 - sfs * 136 / 100, 99, 159, 9);
            fill_rectangle(91, 159 - sfs * 136 / 100, 98, 158, 8);
            DispIt(dctx, 102, 66, 114, 75, 84, 147);
        }

        sfu = -1;

        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Rocket[ROCKET_HW_BOOSTERS].Num >= 0) {
                sfu = Data->P[0].Rocket[ROCKET_HW_BOOSTERS].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[ROCKET_HARDWARE][ROCKET_HW_BOOSTERS];
        } else if (plr == 1) {
            if (Data->P[1].Rocket[ROCKET_HW_BOOSTERS].Num >= 0) {
                sfs = Data->P[1].Rocket[ROCKET_HW_BOOSTERS].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[ROCKET_HARDWARE][ROCKET_HW_BOOSTERS];
        }

        if (sfu > 0) {
            fill_rectangle(118, 159 - sfu * 136 / 100, 126, 159, 6);
            fill_rectangle(118, 159 - sfu * 136 / 100, 125, 158, 5);
            DispIt(dctx, 1, 120, 14, 151, 113, 125);
        }

        if (sfs > 0) {
            fill_rectangle(143, 159 - sfs * 136 / 100, 151, 159, 9);
            fill_rectangle(143, 159 - sfs * 136 / 100, 150, 158, 8);
            DispIt(dctx, 16, 130, 31, 151, 134, 135);
        }

        sfu = -1;

        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Misc[MISC_HW_KICKER_A].Num >= 0) {
                sfu = Data->P[0].Misc[MISC_HW_KICKER_A].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MISC_HARDWARE][MISC_HW_KICKER_A];
        } else if (plr == 1) {
            if (Data->P[1].Misc[MISC_HW_KICKER_A].Num >= 0) {
                sfs = Data->P[1].Misc[MISC_HW_KICKER_A].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MISC_HARDWARE][MISC_HW_KICKER_A];
        }

        if (sfu > 0) {
            fill_rectangle(173, 159 - sfu * 136 / 100, 181, 159, 6);
            fill_rectangle(173, 159 - sfu * 136 / 100, 180, 158, 5);
            DispIt(dctx, 33, 140, 47, 151, 165, 145);
        }

        if (sfs > 0) {
            fill_rectangle(195, 159 - sfs * 136 / 100, 203, 159, 9);
            fill_rectangle(195, 159 - sfs * 136 / 100, 202, 158, 8);
            DispIt(dctx, 49, 138, 61, 151, 188, 143);
        }

        sfu = -1;

        sfs = -1;

        if (plr == 0) {
            if (Data->P[0].Misc[MISC_HW_KICKER_B].Num >= 0) {
                sfu = Data->P[0].Misc[MISC_HW_KICKER_B].Safety;
            }

            sfs = Data->P[0].IntelHardwareTable[MISC_HARDWARE][MISC_HW_KICKER_B];
        } else if (plr == 1) {
            if (Data->P[1].Misc[MISC_HW_KICKER_B].Num >= 0) {
                sfs = Data->P[1].Misc[MISC_HW_KICKER_B].Safety;
            }

            sfu = Data->P[1].IntelHardwareTable[MISC_HARDWARE][MISC_HW_KICKER_B];
        }

        if (sfu > 0) {
            fill_rectangle(226, 159 - sfu * 136 / 100, 234, 159, 6);
            fill_rectangle(226, 159 - sfu * 136 / 100, 233, 158, 5);
            DispIt(dctx, 63, 131, 75, 151, 219, 136);
        }

        if (sfs > 0) {
            fill_rectangle(246, 159 - sfs * 136 / 100, 254, 159, 9);
            fill_rectangle(246, 159 - sfs * 136 / 100, 253, 158, 8);
            DispIt(dctx, 77, 129, 88, 151, 240, 134);
        }

        sfs = -1;

        if (plr == 1) {
            if (Data->P[1].Misc[MISC_HW_KICKER_C].Num >= 0) {
                sfs = Data->P[1].Misc[MISC_HW_KICKER_C].Safety;
            }
        } else if (plr == 0) {
            sfs = Data->P[0].IntelHardwareTable[MISC_HARDWARE][MISC_HW_KICKER_C];
        }

        if (sfs > 0) {
            fill_rectangle(296, 159 - sfs * 136 / 100, 304, 159, 9);
            fill_rectangle(296, 159 - sfs * 136 / 100, 303, 158, 8);
            DispIt(dctx, 51, 77, 93, 127, 266, 106);
        }

        break;
    }
}

/* EOF */
