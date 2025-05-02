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

#ifndef MUSIC_H
#define MUSIC_H

enum music_track {
    M_ASSEMBLY,
    M_ASTTRNG,
    M_BADNEWS,
    M_DRUMSM,
    M_ELEPHANT,
    M_FILLER,
    M_FUTURE,
    M_GOOD,
    M_HARDWARE,
    M_HISTORY,
    M_INTEL,
    M_INTELLIG,
    M_INTERLUD,
    M_LIFTOFF,
    M_MISSPLAN,
    M_NEW1950,
    M_NEW1970,
    M_PRES,
    M_PRGMTRG,
    M_RD,
    M_SOVTYP,
    M_SUCCESS,
    M_SVFUN,
    M_SVLOGO,
    M_SVPORT,
    M_THEME,
    M_UNSUCC,
    M_USFUN,
    M_USMIL,
    M_USPORT,
    M_USSRMIL,
    M_VICTORY,
    M_MAX_MUSIC
};

// Start a given track, and indicate if the track should loop when complete
#define music_start(track) music_start_loop(track, 1)
void music_start_loop(enum music_track track, int loop);

// Stop a given track from playing, or all playing tracks
void music_stop();
void music_stop_all();

// Checks to see if a particular track_name is playing, or if any track is playing
int music_is_playing();
int music_is_track_playing(enum music_track track);

// Do any periodic work that the music system might need
// Called at least 20 times per second
void music_pump();

// Toggle muting of the music system
void music_set_mute(int muted);
#endif // MUSIC_H
