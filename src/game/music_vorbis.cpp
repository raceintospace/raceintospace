/*
 Copyright (C) 2007 Will Glynn

 Derived from music.c:
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

// This file handles music (as you might guess by the name)

#include "Buzz_inc.h"
#include "music.h"
#include "pace.h"
#include "sdlhelper.h"
#include "utils.h"

// A map of music_tracks to filenames
struct music_key {
    enum music_track track;
    const char *name;
} music_key[] = {
    { M_ASSEMBLY, "assembly" },
    { M_ASTTRNG, "asttrng" },
    { M_BADNEWS, "badnews" },
    { M_DRUMSM, "drumsm" },
    { M_ELEPHANT, "elephant" },
    { M_FILLER, "filler" },
    { M_FUTURE, "future" },
    { M_GOOD, "good" },
    { M_HARDWARE, "hardware" },
    { M_HISTORY, "history" },
    { M_INTEL, "intel" },
    { M_INTELLIG, "intellig" },
    { M_INTERLUD, "interlud" },
    { M_LIFTOFF, "liftoff" },
    { M_MISSPLAN, "missplan" },
    { M_NEW1950, "new1950" },
    { M_NEW1970, "new1970" },
    { M_PRES, "pres" },
    { M_PRGMTRG, "prgmtrg" },
    { M_RD, "r&d" },
    { M_SOVTYP, "sovtyp" },
    { M_SUCCESS, "success" },
    { M_SVFUN, "svfun" },
    { M_SVLOGO, "svlogo" },
    { M_SVPORT, "svport2" },
    { M_THEME, "theme" },
    { M_UNSUCC, "unsucc" },
    { M_USFUN, "usfun" },
    { M_USMIL, "usmil" },
    { M_USPORT, "usport2" },
    { M_USSRMIL, "ussrmil" },
    { M_VICTORY, "victory" },
    { M_MAX_MUSIC, NULL },
};

// This structure defines each track
struct music_file {
    // Pointer to the audio buffer, once loaded from disk
    char *buf;
    size_t buf_size;

    // Can this track be played? i.e., does it exist?
    int unplayable;

    // Is this track playing?
    int playing;

    // The SDL audio chunk describing this track
    struct audio_chunk chunk;
};
struct music_file music_files[M_MAX_MUSIC];

// Ensure that the specified music track is in memory, loading it if required
void music_load(enum music_track track)
{
    char fname[20] = "";
    int i;
    ssize_t bytes;

    // Check to see if the track is already loaded or known broken; if so, we're already done
    if (music_files[track].buf != NULL || music_files[track].unplayable) {
        return;
    }

    // Find the name for this track
    for (i = 0; music_key[i].track != M_MAX_MUSIC; i++) {
        if (music_key[i].track == track && music_key[i].name) {
            snprintf(fname, sizeof(fname), "%s.OGG", music_key[track].name);
            break;
        }
    }

    // Bail out if this track isn't known
    if (strlen(fname) == 0) {
        music_files[track].unplayable = 1;
        return;
    }

    // Load the file
    bytes = load_audio_file(fname, &music_files[track].buf, &music_files[track].buf_size, true);

    // Assign the correct buffer size
    music_files[track].buf_size = bytes;

    // Handle failure gracefully
    if (bytes < 0) {
        free(music_files[track].buf);
        music_files[track].unplayable = 1;
        return;
    }

    // XXX: Trim the last two seconds from the audio file, since it's broken
    // Music files edited for better loop, code disabled
    /*
    / if (bytes > 2 * 44100) {
    /    music_files[track].buf_size -= 2 * 44100;
    /    music_files[track].buf = (char *)xrealloc(music_files[track].buf, music_files[track].buf_size);
    /}
    */
}

// Start playing the given track
void music_start_loop(enum music_track track, int loop)
{
    // Load the track as necessary
    music_load(track);

    // Ensure that this track is playable
    if (music_files[track].unplayable) {
        return;
    }

    // Ignore requests to play a track that's already playing
    if (music_files[track].playing) {
        return;
    }

    // Initialize the audio chunk
    music_files[track].chunk.data = music_files[track].buf;
    music_files[track].chunk.size = music_files[track].buf_size;
    music_files[track].chunk.loop = loop;

    // XXX: Stop the existing music, since we need the music channel
    // This should be changed to dynamic channel allocation, to allow layering music tracks
    music_stop();

    // Play the track, and indicate that it's playing
    play(&music_files[track].chunk, AV_MUSIC_CHANNEL);
    music_files[track].playing = 1;
}

// Stop a specific track
void music_stop_track(enum music_track track)
{
    if (music_files[track].playing) {
        // XXX: stop the global music channel
        // This should be assigned on a per-track basis
        av_silence(AV_MUSIC_CHANNEL);

        music_files[track].playing = 0;
    }
}

// Stop all tracks
void music_stop()
{
    int i;

    // Iterate through the list and stop any playing tracks by calling music_stop_track()
    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing) {
            music_stop_track((music_track)i);
        }
    }
}

int music_is_playing()
{
    int i;

    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing) {
            return 1;
        }
    }

    return 0;
}

int music_is_track_playing(enum music_track track)
{
    return music_files[track].playing;
}

void music_pump()
{
    // TODO: Check to see that all the tracks we think are playing actually are
    // This doesn't apply to looped tracks, since those keep playing forever
}

void music_set_mute(int muted)
{
    MuteChannel(AV_MUSIC_CHANNEL, muted);
}
