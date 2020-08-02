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
/** \file vab.c Vehicle Assembly Building
 *
 */

// This file handles the VAB / VIB  (Vehicle Assembly Building / Vehicle Integration Building)

#include "vab.h"

#include <cassert>

#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "gamedata.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "options.h"
#include "admin.h"
#include "game_main.h"
#include "logging.h"
#include "mis_c.h"
#include "mission_util.h"
#include "news_suq.h"
#include "place.h"
#include "mc.h"
#include "sdlhelper.h"
#include "state_utils.h"
#include "port.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "filesystem.h"
#include "vehicle.h"
#include "vehicle_selector.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);

/* VAS holds all possible payload configurations for the given mission.
 * Each payload consists of four components:
 *   0: Primary (a capsule)
 *   1: Kicker
 *   2: LM
 *   3: Payload (Probe / DM)
 * Any of which may be empty. There are only ever a maximum of seven
 * potential payload combinations available at assembly time, each of
 * which is stored in VAS.
 */
struct VInfo VAS[7][4];
int VASqty;  // How many payload configurations there are

/* MI contains the location of a vehicle equipment image and its
 * positioning when drawn inside a vehicle casing.
 *
 * The images of each component used in vehicle assembly are stored as
 * part of a single VAB superimage containing all the component images.
 * The two coordinates (x1, y1) and (x2, y2) represent the top-left
 * corner and bottom-right corner, respectively, of the component's
 * space in that image. The yOffset value describes how much space a
 * capsule image should be given from the top of the casing image in
 * order to properly align it within the launch vehicle casing.
 * The Mercury capsule is a special case, in that the Mercury capsule
 * has a tower meant to extend above the vehicle casing, and thus the
 * offset should be used to move the casing (rather than capsule)
 * down that much.
 */
struct MDA {
    int16_t x1, y1, x2, y2, yOffset;
} MI[2 * 28];

/* ID for the Vab sprite images. Serves as an index into each player's
 * section of vtable.dat, which contains the struct MDA data for
 * finding each sprite in the vab.img.(0/1).png image.
 */
enum VabSprite {
    VabImg_OneStage = 0,
    VabImg_TwoStage,
    VabImg_ThreeStage,
    VabImg_MegaStage,
    VabImg_OneStageBoosters,
    VabImg_TwoStageBoosters,
    VabImg_ThreeStageBoosters,
    VabImg_CasingSmall,
    VabImg_CasingLarge,
    VabImg_OrbitalSat,
    VabImg_InterplanetaryProbe,
    VabImg_LunarProbe,
    VabImg_DockingModule,
    VabImg_OneManCapsule,
    VabImg_TwoManCapsule,
    VabImg_ThreeManCapsule,
    VabImg_Minishuttle,
    VabImg_FourManCapsule,
    VabImg_TwoManLM,
    VabImg_OneManLM,
    VabImg_KickerA,
    VabImg_KickerB,
    VabImg_KickerC,
    VabImg_Zond,
    VabImg_FillerSmall,  // 24
    VabImg_FillerLarge,  // 25
    VabImg_ShroudSmall,  // 26
    VabImg_ShroudLarge,  // 27
};


void LoadMIVals();
boost::shared_ptr<display::LegacySurface> LoadVABSprite(char plr);
int ChkDelVab(char plr, char f);
bool ChkVabRkt(const Vehicle &rocket);
void GradRect2(int x1, int y1, int x2, int y2, char plr);
void DispVAB(char plr, char pad);
int FillVab(char plr, char f, char mode);
int  BuyVabRkt(char plr, Vehicle &rocket, char mode);
void ShowAutopurchase(char plr, int payload, Vehicle &rocket);
void ShowVA(char f);
void ShowRkt(const Vehicle &rocket, int payloadWeight);
void DispVA(char plr, char f, const display::LegacySurface *hw);
void DispRck(char plr, char wh, const display::LegacySurface *hw);
void DispWts(int payload, int thrust);
Equipment *MissionHardware(char plr, enum MissionHardwareType slot,
                           int program);
bool PayloadReady(int plr, int payload);
void ReserveHardware(int plr, int pad, int payload, Vehicle &rocket);
void LMAdd(char plr, char prog, char kic, char part);
void VVals(char plr, char tx, Equipment *EQ, char v4, char sprite);


/* Load the coordinates of vehicle hardware components' images into the
 * MI global variable.
 *
 * Hardware components (capsules, rockets, etc.) have images used to
 * create a mock-up of the hardware assigned to the mission. This
 * includes a display of the rocket and a cutaway illustration of the
 * payload contained within the rocket casing. These component images
 * are stored together in a pair of larger VAB sprites. The global
 * variable MI stores the coordinates specifying where to find each
 * component in the VAB sprite.
 */
void LoadMIVals()
{
    FILE *file = sOpen("VTABLE.DAT", "rb", 0);

    // Read in the data & perform Endianness swap
    for (int i = 0; i < 2 * 28; i++) {
        // struct MDA {
        //     int16_t x1, y1, x2, y2, yOffset;
        // } MI[2 * 28];
        fread(&MI[i].x1, sizeof(MI[i].x1), 1, file);
        fread(&MI[i].y1, sizeof(MI[i].y1), 1, file);
        fread(&MI[i].x2, sizeof(MI[i].x2), 1, file);
        fread(&MI[i].y2, sizeof(MI[i].y2), 1, file);
        fread(&MI[i].yOffset, sizeof(MI[i].yOffset), 1, file);
        Swap16bit(MI[i].x1);
        Swap16bit(MI[i].y1);
        Swap16bit(MI[i].x2);
        Swap16bit(MI[i].y2);
        Swap16bit(MI[i].yOffset);
    }

    fclose(file);
}


/**
 * Load the VAB hardware icons into a local buffer.
 *
 * Exports the file palette to the global display.
 *
 * \param plr  0 for the USA sprite, 1 for the USSR sprite.
 * \throws runtime_error  if Filesystem unable to load the sprite.
 */
boost::shared_ptr<display::LegacySurface> LoadVABSprite(const char plr)
{
    boost::shared_ptr<display::LegacySurface> surface;

    surface = boost::shared_ptr<display::LegacySurface>(new display::LegacySurface(320, 200));

    char filename[128];
    snprintf(filename, sizeof(filename), "images/vab.img.%d.png", plr);

    boost::shared_ptr<display::PalettizedSurface> sprite(
        Filesystem::readImage(filename));

    surface->palette().copy_from(sprite->palette());
    surface->draw(sprite, 0, 0);

    // Export the sprite palette to the display here to prevent any
    // palette mismatch errors.
    // FIXME: Modifies palette of LegacyScreen, should be moved to drawing
    // place
    sprite->exportPalette();
    return surface;
}


void GradRect2(int x1, int y1, int x2, int y2, char plr)
{
    register int i, j, val;

    val = 3 * plr + 6;

    fill_rectangle(x1, y1, x2, y2, 7 + 3 * plr);

    for (j = x1; j <= x2; j += 3) {
        for (i = y1; i <= y2; i += 3) {
            display::graphics.legacyScreen()->pixels()[j + 320 * i] = val;
        }
    }

    return;
}


/* Draw the Vehicle Assembly / Integration interface layout and print
 * mission-specific information.
 *
 * \param plr  0 for the USA, 1 for the USSR.
 * \param pad  The launch pad to which the mission is assigned.
 */
void DispVAB(char plr, char pad)
{
    struct MissionType &mission = Data->P[plr].Mission[pad];

    FadeOut(2, 10, 0, 0);

    display::graphics.screen()->clear();

    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 170, 99);
    ShBox(0, 101, 170, 199);
    ShBox(172, 24, 319, 199);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    IOBox(175, 183, 220, 197);

    // Disable the Scrub buttons if there is no mission.
    if (mission.MissionCode) {
        IOBox(271, 183, 316, 197);
    } else {
        InBox(271, 183, 316, 197);
    }

    // Disable the Delay button if there is no mission OR if it
    // cannot be delayed.
    if (mission.MissionCode &&
        MissionTimingOk(mission.MissionCode, Data->Year, Data->Season)) {
        IOBox(223, 183, 268, 197);
    } else {
        InBox(223, 183, 268, 197);
    }

    InBox(4, 104, 166, 123);
    IOBox(62, 127, 163, 177);
    IOBox(62, 179, 163, 193);

    InBox(177, 28, 314, 180);
    fill_rectangle(178, 29, 313, 179, 3);

    IOBox(4, 84, 165, 96);

    display::graphics.setForegroundColor(9);
    draw_string(188, 192, "E");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "XIT");
    display::graphics.setForegroundColor(9);
    draw_string(231, 192, "D");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ELAY");
    display::graphics.setForegroundColor(9);
    draw_string(279, 192, "S");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "CRUB");
    draw_string(263, 13, "ASSIGN");
    display::graphics.setForegroundColor(9);
    draw_string(18, 136, "P");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "RIMARY:");
    draw_string(24, 148, "KICKER:");
    draw_string(42, 160, "L.M.:");
    draw_string(16, 172, "PAYLOAD:");
    display::graphics.setForegroundColor(9);
    draw_string(22, 188, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "OCKET:     ");

    InBox(4, 27, 166, 37);
    fill_rectangle(5, 28, 165, 36, 10);
    display::graphics.setForegroundColor(11);
    draw_string(38, 34, "LAUNCH FACILITY: ");
    draw_character('A' + pad);
    display::graphics.setForegroundColor(1);

    if (plr == 0) {
        draw_heading(42, 4, "VEHICLE ASSEMBLY", 0, -1);
    } else {
        draw_heading(37, 4, "VEHICLE INTEGRATION", 0, -1);
    }

    display::graphics.setForegroundColor(5);
    draw_string(5, 45, "MISSION: ");
    draw_string(0, 0, mission.Name);

    display::graphics.setForegroundColor(1);
    draw_string(5, 61, "CREW: ");

    switch (mission.Men) {
    case 0:
        draw_string(0, 0, "UNMANNED");
        break;

    case 1:
        draw_string(0, 0, "ONE PERSON");
        break;

    case 2:
        draw_string(0, 0, "TWO PEOPLE");
        break;

    case 3:
        draw_string(0, 0, "THREE PEOPLE");
        break;

    case 4:
        draw_string(0, 0, "FOUR PEOPLE");
        break;
    }

    fill_rectangle(5, 105, 165, 122, 7 + plr * 3);
    display::graphics.setForegroundColor(11);
    draw_string(40, 111, "MISSION HARDWARE:");
    draw_string(10, 119, "SELECT PAYLOADS AND BOOSTER");

    struct mStr missionPlan = GetMissionPlan(mission.MissionCode);

    display::graphics.setForegroundColor(1);
    draw_string(5, 52, missionPlan.Abbr);

    // Show duration level only on missions with a Duration step  -Leon
    if (IsDuration(mission.MissionCode)) {
        draw_string(0, 0, GetDurationParens(mission.Duration));
    }

    draw_small_flag(plr, 4, 4);

    return;
}


/* Calculate the cost of autopurchasing all of the missing hardware
 * components to create the given payload.
 *
 * If the mode argument is set to true, it will proceed to acquire
 * the missing hardware, spending the player's cash accordingly
 * (even if they do not have enough). This will not initiate any
 * hardware programs which haven't been started.
 *
 * This function does not ensure the player has sufficient cash on
 * hand to make the purchases.
 *
 * \param  plr  The player assembling the hardware.
 * \param  f    The VAS index of the given payload hardware set.
 * \param  mode 1 to autopurchase missing components, 0 otherwise.
 * \return      The total cost of all components that will have to be
 *              purchased (0 if autopurchasing).
 */
int FillVab(char plr, char f, char mode)
{
    int cost = 0;
    bool sale = (Data->P[plr].TurnOnly == 3);

    for (int i = 0; i < 4; i++) {
        Equipment *equip =
            MissionHardware(plr, static_cast<MissionHardwareType>(i),
                            VAS[f][i].dex);

        if (equip == NULL || equip->Num == PROGRAM_NOT_STARTED) {
            continue;
        }

        assert(equip->Num >= equip->Spok);

        if ((equip->Num - equip->Spok) <= 0) {
            int unitCost = sale ? (equip->UnitCost / 2) : equip->UnitCost;
            unitCost = (equip->UnitCost > 0) ? MAX(unitCost, 1) : 0;

            if (mode == 1) {
                Data->P[plr].Cash -= unitCost;
                equip->Num++;
            } else {
                cost += unitCost;
            }
        }
    }

    return cost;
}


/* Checks to see if any of the payload hardware is already fully
 * assigned to other missions (or not on hand) and subject to a delay
 * preventing it from being autopurchased.
 *
 * \param  plr  The player assembling the hardware.
 * \param  f    The payload loadout index in VAS.
 * \return      0 if stopped by delay, 1 otherwise.
 */
int ChkDelVab(char plr, char f)
{
    for (int i = 0; i < 4; i++) {
        Equipment *equip =
            MissionHardware(plr, static_cast<MissionHardwareType>(i),
                            VAS[f][i].dex);

        if (equip != NULL && (equip->Num - equip->Spok) <= 0 &&
            equip->Delay > 0) {
            return 0;
        }
    }

    return 1;
}


/* Calculate the cost of autopurchasing the specified missing rocket.
 *
 * If the mode argument is set, it will proceed to acquire the
 * missing rocket, subtracting the cost from the player's available
 * cash. If the player does not have sufficient cash, the purchase
 * will not be made. This will not initiate any new rocket program
 * which hasn't been started.
 *
 * \param plr    The player index.
 * \param rocket
 * \param mode   0 to query the cost, 1 to purchase the rocket parts.
 * \return       The cost of the rocket, 0 if it was purchased.
 */
int BuyVabRkt(char plr, Vehicle &rocket, char mode)
{
    if (! rocket.started()) {
        return 0;
    }

    int cost = 0;
    bool sale = (Data->P[plr].TurnOnly == 3);
    std::list<Equipment *> components = rocket.needed();

    for (std::list<Equipment *>::iterator it = components.begin();
         it != components.end(); it++) {
        cost += sale ? std::max(1, (*it)->UnitCost / 2) : (*it)->UnitCost;
    }

    if (mode == 1 && cost > Data->P[plr].Cash) {
        ERROR3("Cannot purchase launch vehicle components:"
               " cost of %dMBs, have %dMBs on hand",
               cost, Data->P[plr].Cash);
        return cost;
    }

    if (mode == 1) {
        Data->P[plr].Cash -= cost;

        for (std::list<Equipment *>::iterator it = components.begin();
             it != components.end(); it++) {
            (*it)->Num++;
        }
    }

    return (mode == 1) ? 0 : cost;
}


/* Checks to see if a rocket (and booster) is available for use on
 * this launch.
 *
 * \param rocket  a rocket w/ optional booster.
 * \return  true if the vehicle is in stock or may be purchased.
 */
bool ChkVabRkt(const Vehicle &rocket)
{
    return (rocket.available() > 0) || !rocket.delayed();
}


/* Prints the cost of autopurchasing the missing components of the
 * given payload. It prints the cost compared to the player's available
 * funds (XX of YY) on the autopurchase button.
 *
 * \param plr      The player assembling the hardware.
 * \param payload  The VAS index of the given payload hardware set.
 * \param rocket   A rocket w/ optional booster.
 */
void ShowAutopurchase(const char plr, const int payload, Vehicle &rocket)
{
    int hasDelay, cost;

    hasDelay = ChkDelVab(plr, payload) || ChkVabRkt(rocket);
    cost = FillVab(plr, payload, 0) + BuyVabRkt(plr, rocket, 0);

    fill_rectangle(7, 87, 162, 93, 3);
    display::graphics.setForegroundColor(9);
    draw_string(13, 92, "A");
    display::graphics.setForegroundColor(1);

    //if can't buy (delay, cost>cash) ->red letters
    if (hasDelay == 0 || cost > Data->P[plr].Cash) {
        display::graphics.setForegroundColor(9);
    }

    draw_string(0, 0, "UTOPURCHASE (");
    draw_number(0, 0, cost);
    draw_string(0, 0, " OF ");
    draw_megabucks(0, 0, Data->P[plr].Cash);
    draw_string(0, 0, ")");
}


/* Prints the payload components for the selected payload configuration.
 *
 * The text is printed on the Mission Hardware button found in the
 * lower left corner of the main Vehicle Assembly screen.
 *
 * \param  f   The VAS index of the given payload hardware set.
 */
void ShowVA(char f)
{
    fill_rectangle(65, 130, 160, 174, 3);
    display::graphics.setForegroundColor(1);

    for (int i = 0; i < 4; i++) {
        if (VAS[f][i].qty < 0) {
            display::graphics.setForegroundColor(9);
        } else {
            display::graphics.setForegroundColor(1);
        }

        draw_string(67, 136 + 12 * i, &VAS[f][i].name[0]);

        if ((VAS[f][i].qty - VAS[f][i].ac) < 0) {
            draw_number(152, 136 + 12 * i, 0);
            draw_number(128, 136 + 12 * i, 0);
        } else {
            draw_number(152, 136 + 12 * i, VAS[f][i].qty - VAS[f][i].ac);

            if (VAS[f][i].dmg) {
                display::graphics.setForegroundColor(9);
            }

            draw_number(128, 136 + 12 * i, VAS[f][i].sf);
        }

        draw_string(0, 0, "%");
    }

    return;
}


/* Prints the selected rocket's name, safety factor, and quantity.
 *
 * This adds the text to the small rocket selection button in the
 * Mission Hardware area in the lower-left quadrant of the VAB screen.
 *
 * \param rocket
 * \param payloadWeight
 */
void ShowRkt(const Vehicle &rocket, int payloadWeight)
{
    fill_rectangle(65, 182, 160, 190, 3);

    if (!rocket.started() || rocket.thrust() < payloadWeight) {
        display::graphics.setForegroundColor(9);
    } else {
        display::graphics.setForegroundColor(1);
    }

    draw_string(67, 188, rocket.name().c_str());

    if (!rocket.started() < 0) {
        draw_number(152, 188, 0);
        draw_number(128, 188, 0);
    } else {
        draw_number(152, 188, rocket.available());

        if (rocket.damaged()) {
            display::graphics.setForegroundColor(9);
        }

        draw_number(128, 188, rocket.safety());
    }

    draw_string(0, 0, "%");

    return;
}


/* Draw the launch vehicle illustration in the Vehicle Assembly mock-up
 * screen. Depending on the payload cargo, an appropriate casing is
 * chosen and the payload components are rendered along the length of
 * the casing, as per a cut-out illustration, in order of
 * Primary (capsule), LM, Kicker, and Payload.
 * For larger casings, the remaining space is occupied by a filler image
 * depicting the rocket's fuel supply.
 *
 * This function makes extensive use of two global variables, VAS and
 * MI.
 *
 * \param plr      The assembling player (0 for USA, 1 for USSR).
 * \param payload  The VAS index of the given payload hardware set.
 * \param hw       The VAB loaded hardware bitmap.
 */
void DispVA(char plr, char payload, const display::LegacySurface *hw)
{
    int i, TotY, IncY;
    int casingWidth, casingHeight, x1, y1, x2, y2, w2, h2, cx, off = 0;
    uint8_t casing, images, img;

    images = 0; /**< number of pictures */

    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 0) {
            images++;
        }
    }

    casing = ((images == 1 && VAS[payload][Mission_Probe_DM].img > 0) ||
              images == 0) ? VabImg_CasingSmall : VabImg_CasingLarge;

    // The Mercury capsule has a tower that sticks through the top of
    // the casing. On the large casing image, there isn't space to
    // display this, so the smaller casing image is forced.
    if (plr == 0 &&
        VAS[payload][Mission_Capsule].img == VabImg_OneManCapsule) {
        casing = VabImg_CasingSmall;
    }

    /* TotY: sum of height of all payload images */
    TotY = 0;

    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 1) {
            TotY += MI[plr * 28 + VAS[payload][i].img].y2 -
                    MI[plr * 28 + VAS[payload][i].img].y1 + 1;
        }
    }

    /* Load proper (casing) background into buffer */
    x1 = MI[plr * 28 + casing].x1;
    y1 = MI[plr * 28 + casing].y1;
    x2 = MI[plr * 28 + casing].x2;
    y2 = MI[plr * 28 + casing].y2;
    casingWidth = x2 - x1 + 1;
    casingHeight = y2 - y1 + 1;

    // Mercury capsule has a tower extending outside of the casing
    if (plr == 0 &&
        VAS[payload][Mission_Capsule].img == VabImg_OneManCapsule) {
        casingHeight += 13;
        off = 13;
    }

    display::LegacySurface local(casingWidth, casingHeight);

    local.clear(0);

    local.copyFrom(hw, x1, y1, x2, y2, 0, 0 + off);

    /* Copy area background into buffer underneath casing */
    display::LegacySurface local2(casingWidth, casingHeight);

    fill_rectangle(178, 29, 243, 179, 3);  /* TODO: magic numbers */

    local2.copyFrom(display::graphics.legacyScreen(),
                    210 - casingWidth / 2,
                    103 - casingHeight / 2,
                    210 - casingWidth / 2 + casingWidth - 1,
                    103 - casingHeight / 2 + casingHeight - 1);

    local.maskCopy(&local2, 0, display::LegacySurface::DestinationEqual);

    local2.clear(0);

    /* IncY is the y-axis increment for drawing each of the payload
     * component images. If there is no Primary (capsule) image, the
     * payload components are centered. If there is a capsule, they
     * start at the top, plus the component's offset.
     *
     * Mercury capsules, as usual, start at the very top with extra
     * space allotted for that big tower.
     */
    IncY = (casingHeight - TotY) / 2;

    if (VAS[payload][Mission_Capsule].img > 0) {
        if (plr == 0 &&
            VAS[payload][Mission_Capsule].img == VabImg_OneManCapsule) {
            IncY = 0;
        } else {
            IncY = MI[plr * 28 + VAS[payload][Mission_Capsule].img].yOffset;
        }
    }

    /* Draw each of the component images into the local2 buffer */
    for (i = 0; i < 4; i++) {
        if (VAS[payload][i].img > 0) {
            img = VAS[payload][i].img;
            x1 = MI[plr * 28 + img].x1;
            y1 = MI[plr * 28 + img].y1;
            x2 = MI[plr * 28 + img].x2;
            y2 = MI[plr * 28 + img].y2;
            w2 = x2 - x1 + 1;
            h2 = y2 - y1 + 1;
            cx = casingWidth / 2 - w2 / 2 - 1;  // Center on x-axis

            if (cx + w2 > casingWidth || IncY + h2 > casingHeight) {
                CWARNING3(graphic, "can't fit %s image into spaceship casing!",
                          VAS[payload][i].name);
                continue;
            } else {
                local2.copyFrom(hw, x1, y1, x2, y2, cx, IncY);
            }

            IncY += h2 + 1;
        }
    }

    local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);

    // Overlay the shroud to give the appearance of a cutaway.
    // The shroud image is overlaid on the bottom of the casing image.
    // If using a large casing, fill the unused space at the bottom
    // with fiery rocket power!
    if (casing == VabImg_CasingLarge) {
        x1 = MI[plr * 28 + VabImg_FillerLarge].x1;
        y1 = MI[plr * 28 + VabImg_FillerLarge].y1;
        x2 = MI[plr * 28 + VabImg_FillerLarge].x2;
        y2 = MIN(y1 + (casingHeight - IncY - 1),
                 MI[plr * 28 + VabImg_FillerLarge].y2);
        w2 = x2 - x1 + 1;
        cx = casingWidth / 2 - w2 / 2 - 1;
        local2.copyFrom(hw, x1, y1, x2, y2, cx, IncY);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);

        x1 = MI[plr * 28 + VabImg_ShroudLarge].x1;
        y1 = MI[plr * 28 + VabImg_ShroudLarge].y1;
        x2 = MI[plr * 28 + VabImg_ShroudLarge].x2;
        y2 = MI[plr * 28 + VabImg_ShroudLarge].y2;
        h2 = y2 - y1 + 1;

        local2.copyFrom(hw, x1, y1, x2, y2, 0, casingHeight - h2);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);
    } else {
        // There is no small filler defined, so skip to the shroud.
        x1 = MI[plr * 28 + VabImg_ShroudSmall].x1;
        y1 = MI[plr * 28 + VabImg_ShroudSmall].y1;
        x2 = MI[plr * 28 + VabImg_ShroudSmall].x2;
        y2 = MI[plr * 28 + VabImg_ShroudSmall].y2;
        h2 = y2 - y1 + 1;
        local2.copyFrom(hw, x1, y1, x2, y2, 0, casingHeight - h2);

        local.maskCopy(&local2, 0, display::LegacySurface::SourceNotEqual);
    }

    local.copyTo(display::graphics.legacyScreen(),
                 210 - casingWidth / 2, 103 - casingHeight / 2);
}


/* Draw the rocket illustration in the Vehicle Assembly mock-up screen.
 *
 * The Rocket graphics, as with other VAB images, are stored in a
 * texture atlas, with the global variable MI storing the texture
 * coordinates.
 *
 * This function makes extensive use of the global variable MI.
 *
 * The rockets are indexed in the MI[] array as:
 *   0 / 28:   Atlas  / A-Series
 *   1 / 29:   Titan  / Proton
 *   2 / 30:   Saturn / N-1
 *   3 / 31:   Nova   / Energia
 *   4 / 32:   Atlas + Boosters  / A-Series + Boosters
 *   5 / 33:   Titan + Boosters  / Proton + Boosters
 *   6 / 34:   Saturn + Boosters / N-1 + Boosters
 *
 * \param plr  The assembling player (0 for USA, 1 for USSR).
 * \param wh   The rocket's index in the MI[] array.
 * \param hw   The VAB hardware sprite
 */
void DispRck(char plr, char wh, const display::LegacySurface *hw)
{
    int w;
    int h;
    int x1;
    int y1;
    int x2;
    int y2;
    int middle_w;
    int middle_h;

    x1 = MI[plr * 28 + wh].x1;
    y1 = MI[plr * 28 + wh].y1;
    x2 = MI[plr * 28 + wh].x2;
    y2 = MI[plr * 28 + wh].y2;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    middle_w = 282 - w / 2;
    middle_h = 103 - h / 2;
    display::LegacySurface local(w, h);
    display::LegacySurface local2(w, h);

    local.copyFrom(hw, x1, y1, x2, y2, 0, 0);

    fill_rectangle(247, 29, 313, 179, 3);
    local2.copyFrom(display::graphics.legacyScreen(), middle_w, middle_h, middle_w + w - 1, middle_h + h - 1);

    local.maskCopy(&local2, 0, display::LegacySurface::DestinationEqual);

    local.copyTo(display::graphics.legacyScreen(), middle_w, middle_h);
}


/**
 * Return the hardware program given the payload slot and program index.
 *
 * MissionHardware doesn't handle VIB/VAB rocket indexing by accounting
 * for Rocket+Booster. It's strictly one or the other.
 *
 * TODO: MissionHardware doesn't have protections against array
 * overflow issues. Consider running through HardwareProgram()?
 *
 * \param plr      the player index
 * \param slot     the mission payload slot
 * \param program  the equipment index (EquipProbeIndex, etc.)
 * \return   the equipment entry, or NULL if no match (program < 0).
 */
Equipment *MissionHardware(char plr, enum MissionHardwareType slot,
                           int program)
{
    assert(0 <= plr && plr < NUM_PLAYERS);

    if (program < 0) {
        return NULL;
    }

    switch (slot) {
    case Mission_Capsule:
    case Mission_LM:
        return &Data->P[plr].Manned[program];
        break;

    case Mission_Kicker:
        return &Data->P[plr].Misc[program];
        break;

    case Mission_Probe_DM:
        if (program == MISC_HW_DOCKING_MODULE) {
            return &Data->P[plr].Misc[program];
        } else {
            return &Data->P[plr].Probe[program];
        }

        break;

    case Mission_PrimaryBooster:
        return &Data->P[plr].Rocket[program];
        break;

    case Mission_EVA:
        // Should program == MISC_HW_EVA_SUITS be enforced?
        return &Data->P[plr].Misc[MISC_HW_EVA_SUITS];
        break;

    case Mission_PhotoRecon:
        // Should program == MISC_HW_PHOTO_RECON be enforced?
        return &Data->P[plr].Misc[MISC_HW_PHOTO_RECON];
        break;

    // Mission_SecondaryBooster isn't used, per data.h
    // case Mission_SecondaryBooster:

    default:
        throw std::logic_error(
            "Illegal hardware type passed to MissionHardware");
        break;
    }
}


/**
 * Check if the payload can be assigned to a mission.
 *
 * \param plr      the player index.
 * \param payload  the payload index in VAS.
 * \return  true if there is a free piece of each payload component.
 */
bool PayloadReady(int plr, int payload)
{
    for (int i = 0; i < 4; i++) {
        Equipment *equip =
            MissionHardware(plr, static_cast<MissionHardwareType>(i),
                            VAS[payload][i].dex);

        if (equip && (equip->Num - equip->Spok) <= 0) {
            return false;
        }
    }

    return true;
}


/**
 * Assigns a launch vehicle and payload to a current mission.
 *
 * This reserves both rocket hardware and payload together, because
 * hardware assignment is normally tracked by
 *   Mission.Hard[Mission_PrimaryBooster] > 0
 * so allowing rocket or payload to be assigned without the other
 * could break game assumptions.
 *
 * \param plr      the player index.
 * \param pad      the pad index for the mission.
 * \param payload  the payload index in VAS.
 * \param rocket   the rocket index (+ ROCKET_HW_BOOSTERS if B/Rocket).
 */
void ReserveHardware(int plr, int pad, int payload, Vehicle &rocket)
{
    assert(0 <= plr && plr < NUM_PLAYERS);
    assert(0 <= pad && pad < MAX_MISSIONS);
    assert(0 <= payload && payload < 7);

    for (int i = Mission_Capsule; i <= Mission_Probe_DM; i++) {
        int index = VAS[payload][i].dex;
        Data->P[plr].Mission[pad].Hard[i] = index;
        Equipment *equip =
            MissionHardware(plr, static_cast<MissionHardwareType>(i),
                            index);

        if (index >= 0 && equip) {
            equip->Spok++;
        }
    }

    rocket.assignTo(pad);
}


/* Print the readout of the vehicle payload weight.
 *
 * This displays the current weight of the payload as well as the
 * maximum payload supported by the rocket currently assigned.
 *
 * \param payload  The total weight of the current payload hardware.
 * \param thrust  The maximum payload weight for the current rocket.
 */
void DispWts(int payload, int thrust)
{
    fill_rectangle(5, 65, 140, 83, 3);

    display::graphics.setForegroundColor(1);
    draw_string(5, 77, "MAXIMUM PAYLOAD: ");
    draw_number(0, 0, thrust);

    display::graphics.setForegroundColor(1);
    draw_string(5, 70, "CURRENT PAYLOAD: ");

    if (thrust < payload) {
        display::graphics.setForegroundColor(9);
    }

    draw_number(0, 0, payload);

    return;
}


void VAB(char plr)
{
    int ccc;  // Payload index
    int mis, weight;
    VehicleSelector rocketList(plr);
    char ButOn;
    boost::shared_ptr<display::LegacySurface> hw;

    LoadMIVals();
    music_start(M_HARDWARE);

    // FutureCheck brings up the Launch Pad menu, displaying the
    // missions assigned to each of the pads, and allows the player
    // to select a launch pad (or exit). It returns the chosen pad
    // index (or exit code).
    while ((mis = FutureCheck(plr, 1)) < MAX_MISSIONS) {

        // If a manned mission's Primary & Backup flight crews are
        // unavailable, scrub the mission.
        if (CheckCrewOK(plr, mis) == 1) {  // found mission no crews
            ScrubMission(plr, mis);
            continue;
        }

        helpText = "i016";
        keyHelpText = "k016";

        struct mStr missionPlan =
            GetMissionPlan(Data->P[plr].Mission[mis].MissionCode);

        // When reassembling Hardware, any hardware previously assigned to
        // the mission should be unassigned so it may be used (or not) in
        // reassembly.
        FreeLaunchHardware(plr, mis);

        // Clear mission hardware
        for (int i = Mission_Capsule; i <= Mission_PrimaryBooster; i++) {
            Data->P[plr].Mission[mis].Hard[i] = 0;
        }

        BuildVAB(plr, mis, 0, 0, 0);  // now holds the mission info
        Vehicle rocket = rocketList.start();

        DispVAB(plr, mis);
        hw = LoadVABSprite(plr);

        if (Data->P[plr].Mission[mis].MissionCode) {
            ButOn = 1;
        } else {
            ButOn = 0;
            InBox(245, 5, 314, 17);
        }

        ccc = 1;
        weight = 0;

        for (int i = 0; i < 4; i++) {
            weight += VAS[ccc][i].wt;
        }

        while (rocket.thrust() < weight) {
            rocket = rocketList.next();
        }

        ShowVA(ccc);
        ShowRkt(rocket, weight);
        DispRck(plr, rocket.index(), hw.get());
        DispVA(plr, ccc, hw.get());
        DispWts(weight, rocket.thrust());
        //display cost (XX of XX)
        ShowAutopurchase(plr, ccc, rocket);

        FadeIn(2, 10, 0, 0);
        WaitForMouseUp();

        while (1) {
            key = 0;
            GetMouse();

            if (mousebuttons <= 0 && key <= 0) {
                continue;
            }

            // Gameplay
            if ((x >= 6 && y >= 86 && x <= 163 && y <= 94 && mousebuttons > 0) || key == 'A') {
                // AUTOPURCHASE
                InBox(6, 86, 163, 94);
                key = 0;
                // NEED A DELAY CHECK
                bool ac = ChkDelVab(plr, ccc) &&
                          ChkVabRkt(rocket);

                int cost = FillVab(plr, ccc, 0) +
                           BuyVabRkt(plr, rocket, 0);

                if (Data->P[plr].Cash >= cost && ac == true) {
                    FillVab(plr, ccc, 1);
                    BuyVabRkt(plr, rocket, 1);
                    // Repopulate VAB data to update components with
                    // autopurchased quantities.
                    BuildVAB(plr, mis, 0, 0, 1);


                    //display cost (XX of XX)
                    ShowAutopurchase(plr, ccc, rocket);
                } else if (ac == false) {
                    Help("i135");    // delay on purchase
                } else {
                    Help("i137");    // not enough money
                }

                ShowVA(ccc);
                ShowRkt(rocket, weight);
                OutBox(6, 86, 163, 94);
            } else if ((x >= 177 && y >= 185 && x <= 218 && y <= 195 && mousebuttons > 0) || (key == K_ESCAPE || key == 'E')) {
                // CONTINUE/EXIT/DO NOTHING
                InBox(177, 185, 218, 195);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(177, 185, 218, 195);

                break;
            } else if (((x >= 225 && y >= 185 && x <= 268 && y <= 195 && mousebuttons > 0) || key == 'D') && Data->P[plr].Mission[mis].MissionCode) {
                // DELAY the mission for a turn

                // There are restrictions on Mars/Jupiter/Saturn Flybys,
                // so check that this mission _could_ be launched at
                // this time.
                bool validLaunch =
                    MissionTimingOk(Data->P[plr].Mission[mis].MissionCode,
                                    Data->Year, Data->Season);

                if (validLaunch) {
                    InBox(225, 185, 266, 195);
                    WaitForMouseUp();

                    if (key > 0) {
                        delay(100);
                    }

                    bool conflict = false;

                    // Check if there's a Future Mission which would be
                    // displaced by delaying the mission.
                    if (Data->P[plr].Future[mis].MissionCode) {
                        conflict = true;
                    } else if (Data->P[plr].Mission[mis].Joint) {
                        int other = (Data->P[plr].Mission[mis].part) ?
                                    mis - 1 : mis + 1;

                        if (Data->P[plr].Future[other].MissionCode) {
                            conflict = true;
                        }
                    }

                    // Confirm that the mission should be delayed.
                    if (Help(conflict ? "i163" : "i162") > 0) {
                        DelayMission(plr, mis);
                        OutBox(225, 185, 266, 195);
                        break;
                    } else {
                        OutBox(225, 185, 266, 195);
                    }
                }
            } else if (((x >= 273 && y >= 185 && x <= 314 && y <= 195 && mousebuttons > 0) || key == 'S') && Data->P[plr].Mission[mis].MissionCode) {
                // SCRUB The whole mission
                InBox(273, 185, 314, 195);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(273, 185, 314, 195);

                if (ScrubMissionQuery(plr, mis)) {
                    ScrubMission(plr, mis);
                }

                break;
            } else if (((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) &&
                       ccc != 0 && ButOn == 1 && weight <= rocket.thrust()) {
                if (missionPlan.EVA == 1 &&
                    Data->P[plr].Misc[MISC_HW_EVA_SUITS].Num == PROGRAM_NOT_STARTED) {
                    Help("i158");
                } else if (missionPlan.Doc == 1 &&
                           Data->P[plr].Misc[MISC_HW_DOCKING_MODULE].Num == PROGRAM_NOT_STARTED) {
                    Help("i159");
                } else {
                    if ((missionPlan.mVab[0] & 0x10) == 0x10 &&
                        Data->P[plr].DockingModuleInOrbit <= 0) {
                        Help("i155");    // No docking module in orbit
                    }

                    if (PayloadReady(plr, ccc) && rocket.available() > 0) {
                        InBox(245, 5, 314, 17);
                        WaitForMouseUp();

                        if (key > 0) {
                            delay(150);
                        }

                        OutBox(245, 5, 314, 17);

                        ReserveHardware(plr, mis, ccc, rocket);
                        break;
                    }
                }
            } else if ((x >= 64 && y >= 181 && x <= 161 && y <= 191 && mousebuttons > 0) || key == 'R') {
                // Choose Rocket
                InBox(64, 181, 161, 191);

                rocket = rocketList.next();

                if (! IsLEORegion(Data->P[plr].Mission[mis].MissionCode) &&
                    ! rocket.translunar()) {
                    if (options.cheat_altasOnMoon == 0) {
                        rocket = rocketList.next();
                    }
                }

                //display cost (XX of XX)
                ShowAutopurchase(plr, ccc, rocket);
                ShowRkt(rocket, weight);
                DispWts(weight, rocket.thrust());
                DispRck(plr, rocket.index(), hw.get());
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(64, 181, 161, 191);
            } else if ((x >= 64 && y >= 129 && x <= 161 && y <= 175 && mousebuttons > 0) || key == 'P') {
                // Cycle through payload selections
                InBox(64, 129, 161, 175);
                ccc++;

                if (ccc > VASqty) {
                    ccc = 0;
                }

                weight = 0;

                for (int i = 0; i < 4; i++) {
                    weight += VAS[ccc][i].wt;
                }

                ShowVA(ccc);
                DispWts(weight, rocket.thrust());
                ShowRkt(rocket, weight);
                DispVA(plr, ccc, hw.get());
                //display cost (XX of XX)
                ShowAutopurchase(plr, ccc, rocket);
                WaitForMouseUp();

                if (key > 0) {
                    delay(100);
                }

                OutBox(64, 129, 161, 175);
            }
        }
    }

    Vab_Spot = (Data->P[plr].Mission[PAD_A].Hard[Mission_PrimaryBooster] > 0) ? 1 : 0;
    music_stop();
}


//----------------------------------------------------------------------
// VAB Autobuild Functions
//----------------------------------------------------------------------


/* Generates the set of possible distinct vehicle payloads for the
 * mission and stores the payload information in the global variable
 * VAS.
 *
 * Mission payloads are built based on the mission requirements found
 * in struct mStr.mVab, a single byte encoded with bit flags:
 *   0x80  Capsule (CAP)
 *   0x40  Lunar Module (LM)
 *   0x20  Orbital Satellite / Docking Module (DMO)
 *   0x10  Spoofed Docking Module (SDM)
 *   0x08  EVA Suit (EVA)
 *   0x04  Interplanetary Probe (INTER)
 *   0x02  Lunar Surveyor (PRO)
 *   0x01  Kicker (KIC)
 *
 * This function is used by the AI, so beware of making changes without
 * understanding what the AI is doing!
 *
 * \param plr  The player assembling the launch vehichle.
 * \param mis  The pad the mission is on.
 * \param ty   >0 if called by the AI.
 * \param pa   0 if the primary half of a joint mission.
 * \param pr   The hardware program to use on mission (used by AI).
 */
void BuildVAB(char plr, char mis, char ty, char pa, char pr)
{
    char i, j, part, mcode, prog, ext = 0;
    unsigned char VX;

    memset(VAS, 0x00, sizeof VAS);

    if (ty == 0) {
        part = Data->P[plr].Mission[mis].part;
        mcode = Data->P[plr].Mission[mis].MissionCode;
        prog = Data->P[plr].Mission[mis].Prog - 1;

        if (part == 0) {
            if (Data->P[plr].Mission[mis].Joint == 1) {
                ext = Data->P[plr].Mission[mis + 1].Prog - 1;
            }
        } else {
            ext = prog;
        }
    } else {                              // ty>0 for AI code
        part = pa;                          // Mission Part to Check
        mcode = mis;                        // Mission to Check
        prog = ext = pr;                    // Manned Program to Check
    }

    VX = GetMissionPlan(mcode).mVab[part];

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 4; j++) {
            strcpy(&VAS[i][j].name[0], "NONE");
            VAS[i][j].qty = VAS[i][j].sf = VAS[i][j].wt = VAS[i][j].dmg = 0;
            VAS[i][j].dex = VAS[i][j].img = -1;
        }
    }

    VASqty = 0;

    if (VX & 0x80) {  // Capsule
        j = (part == 0) ? prog : ext;

        for (i = 1; i < 6; i++) {  // Fill all parts with CAP
            VASqty++;
            VVals(plr, Mission_Capsule, &Data->P[plr].Manned[j], j,
                  VabImg_OneManCapsule + j);
        }
    }

    VASqty = 0;

    if (VX == 0x20 && part == 0 && mcode == Mission_Orbital_Satellite) {  // P:Sxx XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Probe[PROBE_HW_ORBITAL], PROBE_HW_ORBITAL, VabImg_OrbitalSat);
    }

    if (VX == 0x20 && part == 0 && mcode != Mission_Orbital_Satellite) {  // P:xDM XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], MISC_HW_DOCKING_MODULE, VabImg_DockingModule);
    } else if (VX == 0x04 && part == 0) {  // P:INTER XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Probe[PROBE_HW_INTERPLANETARY], PROBE_HW_INTERPLANETARY, VabImg_InterplanetaryProbe);
    } else if (VX == 0x02 && part == 0) {  // P:PRO XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Probe[PROBE_HW_LUNAR], PROBE_HW_LUNAR, VabImg_LunarProbe);
    } else if (VX == 0x60 && part == 0) {  // P:LM+SDM XX
        LMAdd(plr, ext, -1, 1);
    } else if (VX == 0xe8 && part == 0) {  // P:LM+SDM+EVA XX
        LMAdd(plr, prog, -1, 1);
    } else if (VX == 0x61 && part == 0) {  // P:LM+SDM+KIC XX
        LMAdd(plr, ext, MISC_HW_KICKER_A, 1);
        LMAdd(plr, ext, MISC_HW_KICKER_B, 1);
    } else if (VX == 0x21 && part == 0) {  // P:SDM+KIC-C XX
        VASqty++;
        VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_C], MISC_HW_KICKER_C, VabImg_KickerC);
    }

    else if (VX == 0x80) {
        VASqty = 1;    // P/S:CAP XX
    }

    else if (VX == 0x88) {  // P/S:CAP+EVA XX
        VASqty = 1;
        // EVA Check
    }

    else if (VX == 0xa0 && part == 0) {  // P:CAP+SDM XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], MISC_HW_DOCKING_MODULE, VabImg_DockingModule);
    }

    else if (VX == 0x90 && part == 0) {  // P:CAP+DMO XX
        VASqty = 1;  // DMO Check
    }

    else if (VX == 0xa8 && part == 0) {  // P:CAP+SDM+EVA XX
        VASqty++;
        VVals(plr, Mission_Probe_DM, &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE], MISC_HW_DOCKING_MODULE, VabImg_DockingModule);
        // EVA Check
    }

    else if (VX == 0x98 && part == 0) {  // P:CAP+DMO+EVA XX
        VASqty = 1;
        // EVA Check
        // DMO Check
    }

    else if (VX == 0xe0 && part == 0) {  // P:CAP+LM+SDM XX
        LMAdd(plr, prog, -1, 0);
    }

    else if (VX == 0x81) {  // P/S:CAP+KIC XX
        if (prog == MANNED_HW_TWO_MAN_CAPSULE ||
            prog == MANNED_HW_MINISHUTTLE) {
            if (mcode != 52) {  ///Special Case EOR LM Test
                VASqty++;
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_A], MISC_HW_KICKER_A, VabImg_KickerA);
                VASqty++;
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_B], MISC_HW_KICKER_B, VabImg_KickerB);
            } else {
                VASqty++;
                // TODO: Check this out - seems like the sprite image
                // should be VabImg_KickerB.
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_B], MISC_HW_KICKER_B, VabImg_KickerA);
            }
        } else {
            VASqty = 1;
        }
    }

    else if (VX == 0xe1 && part == 0) {  // P:CAP+LM+SDM+KIC XX
        LMAdd(plr, prog, MISC_HW_KICKER_B, 0);
    }

    else if (VX == 0xe9 && part == 0) { // P:CAP+LM+SDM+EVA XX
        LMAdd(plr, prog, MISC_HW_KICKER_B, 0);
        // EVA Check
    }

    else if (VX == 0x89 && part == 1) {  // S:CAP+EVA+KIC
        if (prog != MANNED_HW_THREE_MAN_CAPSULE) {
            if (mcode != Mission_Jt_LunarLanding_EOR) {  ///Special Case EOR Lunar Landing
                VASqty++;
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_A], MISC_HW_KICKER_A, VabImg_KickerA);
                VASqty++;
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_B], MISC_HW_KICKER_B, VabImg_KickerB);
            } else {
                VASqty++;
                // TODO: Check this out - seems like sprite image should
                // be VabImg_KickerB.
                VVals(plr, Mission_Kicker, &Data->P[plr].Misc[MISC_HW_KICKER_B], MISC_HW_KICKER_B, VabImg_KickerA);
            }
        } else {
            VASqty = 1;
        }

        // EVA Check
    }

    return;
}


/**
 * \param plr
 * \param prog  the capsule/manned program index (EquipMannedIndex).
 * \param kic   the kicker index (EquipMiscIndex) or -1 if none.
 * \param part  0 for first part of a joint mission, 1 if second pad.
 */
void LMAdd(char plr, char prog, char kic, char part)
{
    if (prog == MANNED_HW_TWO_MAN_CAPSULE) {
        VASqty++;
        VVals(plr, Mission_Probe_DM,
              &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE],
              MISC_HW_DOCKING_MODULE,
              VabImg_DockingModule);
        VVals(plr, Mission_LM,
              &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE],
              MANNED_HW_ONE_MAN_MODULE,
              VabImg_OneManLM);

        if (kic >= 0) {
            VVals(plr, Mission_Kicker,
                  &Data->P[plr].Misc[kic], kic, VabImg_KickerA + kic);
        }

    }

    else if (prog == MANNED_HW_THREE_MAN_CAPSULE) {
        VASqty++;
        VVals(plr, Mission_LM,
              &Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE],
              MANNED_HW_TWO_MAN_MODULE, VabImg_TwoManLM);

        if (part == 1 && kic >= 0) {
            VVals(plr, Mission_Kicker, &Data->P[plr].Misc[kic], kic,
                  VabImg_KickerA + kic);
        }

        VASqty++;
        VVals(plr, Mission_LM,
              &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE],
              MANNED_HW_ONE_MAN_MODULE, VabImg_OneManLM);

        if (part == 1 && kic >= 0) {
            VVals(plr, Mission_Kicker,
                  &Data->P[plr].Misc[kic], kic, VabImg_KickerA + kic);
        }
    }

    else if (prog == MANNED_HW_MINISHUTTLE) {
        VASqty++;
        VVals(plr, Mission_Probe_DM,
              &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE],
              MISC_HW_DOCKING_MODULE, VabImg_DockingModule);
        VVals(plr, Mission_LM,
              &Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE],
              MANNED_HW_TWO_MAN_MODULE, VabImg_TwoManLM);

        if (kic >= 0) {
            VVals(plr, Mission_Kicker,
                  &Data->P[plr].Misc[kic], kic, VabImg_KickerA + kic);
        }

        VASqty++;
        VVals(plr, Mission_Probe_DM,
              &Data->P[plr].Misc[MISC_HW_DOCKING_MODULE],
              MISC_HW_DOCKING_MODULE, VabImg_DockingModule);
        VVals(plr, Mission_LM,
              &Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE],
              MANNED_HW_ONE_MAN_MODULE, VabImg_OneManLM);

        if (kic >= 0) {
            VVals(plr, Mission_Kicker,
                  &Data->P[plr].Misc[kic], kic, VabImg_KickerA + kic);
        }
    }

    return;
}


/**
 * TODO: Documentation.
 *
 * \param plr
 * \param tx   the mission hardware slot (enum MissionHardwareType)
 * \param EQ   the hardware component
 * \param v4   the equipment index (EquipProbeIndex, etc.)
 * \param sprite  the hardware's sprite index
 */
void VVals(char plr, char tx, Equipment *EQ, char v4, char sprite)
{
    strcpy(&VAS[VASqty][tx].name[0], &EQ->Name[0]);
    VAS[VASqty][tx].qty = EQ->Num;
    VAS[VASqty][tx].ac = EQ->Spok;
    VAS[VASqty][tx].wt = EQ->UnitWeight;

    if (tx == Mission_Probe_DM && v4 == MISC_HW_DOCKING_MODULE &&
        AI[plr] == 1) {
        VAS[VASqty][tx].sf = EQ->MSF;
    } else {
        VAS[VASqty][tx].sf = EQ->Safety;
    }

    VAS[VASqty][tx].dex = v4;
    VAS[VASqty][tx].img = sprite;
    VAS[VASqty][tx].dmg = EQ->Damage != 0 ? 1 : 0;
    return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
