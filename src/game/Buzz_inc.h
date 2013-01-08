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

#ifndef __BUZZ_INC_H__
#define __BUZZ_INC_H__ 1

extern "C" {
    int game_main(int argc, char *argv[]);
}

#ifdef __GNUG__
// suppress literal-strings-as-char* warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#if __clang__
// suppress "char as array subscript" warning
#pragma clang diagnostic ignored "-Wchar-subscripts"
#endif

#ifdef _WIN32
// Note, this cannot be removed until the packing problem (below) is worked out
#include <winsock2.h>
#endif

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <SDL/SDL_config.h> // declares some of the same symbols as our config
#include "raceintospace_config.h"
#include "logging.h"
#include "gamedata.h"
#include "proto.h"    // prototypes and general defines
#include "music.h"    // defines for music names

/* FIXME: non-portable. Used to get struct layout like in DOS days */
#pragma pack( 1 )

#include "data.h"     // main data structures

/* get the alignment back to defaults */
/* #pragma pack() */

/* BIG FIXME: Unfortunately structures defined in some functions rely on tight
 * packing. This setting mainly breaks things in gamedata.h, so we make sure
 * we notice bad order of #includes. That's another good reason to make all
 * code use the gamedata.c interfaces. */
#define ALTERED_STRUCTURE_PACKING


#include "macros.h"     // Collected Macros
#include "fs.h"

#endif /* __BUZZ_INC_H__ */
