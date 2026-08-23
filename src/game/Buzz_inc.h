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

#ifndef RIS_BUZZ_INC_H
#define RIS_BUZZ_INC_H 1

extern "C" {
    int game_main(int argc, char *argv[]);
}

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <SDL_config.h> // declares some of the same symbols as our config

#include "raceintospace_config.h"
#include "logging.h"
#include "gamedata.h"
#include "proto.h"    // prototypes and general defines
#include "music.h"    // defines for music names

#include "data.h"     // main data structures

#include "macros.h"     // Collected Macros
#include "fs.h"

#endif // RIS_BUZZ_INC_H
