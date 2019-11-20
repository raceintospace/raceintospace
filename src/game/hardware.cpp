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

#include <cassert>
#include <stdexcept>

#include "Buzz_inc.h"
#include "data.h"
#include "game_main.h"


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
