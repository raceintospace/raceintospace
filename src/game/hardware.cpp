/* Copyright (C) 2019
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "hardware.h"

#include <cassert>
#include <stdexcept>

#include "game_main.h"
#include "macros.h"
#include "options.h"


/**
 * Check the crew capacity for a Capsule/Minishuttle or Lunar Module.
 *
 * \param program  a Manned hardware program.
 * \return  number of crew allowed/required (0 if unmanned).
 */
int CrewSize(const Equipment &program)
{
    MissionHardwareType type = HardwareType(program);

    if (type != Mission_Capsule && type != Mission_LM) {
        return 0;
    }

    int mannedIndex = int(program.ID[1] - '0');

    switch (mannedIndex) {
    case MANNED_HW_ONE_MAN_CAPSULE:
        return 1;

    case MANNED_HW_TWO_MAN_CAPSULE:
        return 2;

    case MANNED_HW_THREE_MAN_CAPSULE:
    case MANNED_HW_MINISHUTTLE:
        return 3;

    case MANNED_HW_FOUR_MAN_CAPSULE:
        return 4;

    case MANNED_HW_TWO_MAN_MODULE:
        return 2;

    case MANNED_HW_ONE_MAN_MODULE:
        return 1;

    default:
        return 0;
    }

    return 0;
}


/**
 * TODO: Describe.
 *
 * If the argument is not a recognized hardware program, the
 * result is undefined.
 */
MissionHardwareType HardwareType(const Equipment &program)
{
    int categoryIndex = int(program.ID[1] - '0');

    switch (program.ID[0]) {
    case 'P':
        return Mission_Probe_DM;

    case 'C':
        return (categoryIndex <= MANNED_HW_FOUR_MAN_CAPSULE)
               ? Mission_Capsule : Mission_LM;

    case 'R':
        return (categoryIndex != ROCKET_HW_BOOSTERS)
               ? Mission_PrimaryBooster : Mission_SecondaryBooster;

    case 'M':
        if (categoryIndex <= MISC_HW_KICKER_C) {
            return Mission_Kicker;
        } else if (categoryIndex == MISC_HW_EVA_SUITS) {
            return Mission_EVA;
        } else if (categoryIndex == MISC_HW_DOCKING_MODULE) {
            return Mission_Probe_DM;
        } else if (categoryIndex == MISC_HW_PHOTO_RECON) {
            return Mission_PhotoRecon;
        } else {
            throw std::invalid_argument(
                "Error in function HardwareType: "
                "argument program is not recognized Misc hardware");
        }

    default:
        throw std::invalid_argument(
            "Error in function HardwareType: "
            "program's hardware index is not recognized.");
        break;
    }

    throw std::invalid_argument(
        "Error in function HardwareType: "
        "program's hardware index is not recognized.");
}


/**
 * Accessor for the player hardware programs.
 *
 * \param player  0 for the USA, 1 for the USSR
 * \param type    which category of hardware program, per EquipmentIndex
 * \param program the index of the hardware program, per the relevant
 *        Equip<type>Index enum
 */
Equipment &HardwareProgram(int player, int type, int program)
{
    assert(player >= 0 && player < NUM_PLAYERS);
    assert(program >= 0 && program < 7);
    assert(type >= 0 && type < 4);

    switch (type) {
    case PROBE_HARDWARE:
        return Data->P[player].Probe[program];
        break;

    case ROCKET_HARDWARE:
        return Data->P[player].Rocket[program];
        break;

    case MANNED_HARDWARE:
        return Data->P[player].Manned[program];
        break;

    case MISC_HARDWARE:
        return Data->P[player].Misc[program];
        break;

    default:
        throw std::invalid_argument(
            "Error in argument to HardwareProgram: type must be part of "
            "the EquipmentIndex enum");
        break;
    };
}


/**
 * Calculates the safety rating for a rocket with boosters attachment.
 *
 * \param safetyRocket  The rocket program's Safety factor.
 * \param safetyBooster  The booster program's Safety factor.
 * \return  the Safety test value for the rocket/booster combination.
 */
int RocketBoosterSafety(int safetyRocket, int safetyBooster)
{
    if (options.boosterSafety == 0) {
        return ((safetyRocket * safetyBooster) / 100);
    }

    if (options.boosterSafety == 1) {
        return (MIN(safetyRocket, safetyBooster));
    } else {
        return ((safetyRocket + safetyBooster) / 2);
    }
}
int RocketMaxRD(int MaxRDRocket, int MaxRDBooster)
{
    if (options.boosterSafety == 0) {
        return ((MaxRDRocket * MaxRDBooster) / 100);
    }

    if (options.boosterSafety == 1) {
        return (MIN(MaxRDRocket, MaxRDBooster));
    } else {
        return ((MaxRDRocket + MaxRDBooster) / 2);
    }
}
