/*
 Copyright (C) 2007 Will Glynn
 
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreMIDI/CoreMIDI.h>

#include "race.h"
#include "fs.h"
#include "music.h"
#include "logging.h"

LOG_DEFAULT_CATEGORY(music);

// A map of music_tracks to filenames
struct music_key {
	enum music_track track;
	char *name;
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
	{ M_INTELLEG, "intelleg" },
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

// Whether or not the music should be globally muted
int mute_music = 0;

// This structure defines each track
struct music_file {
	// Pointer to the audio buffer, once loaded from disk
	char * buf;
	size_t buf_size;
	
	// Can this track be played? i.e., does it exist?
	int unplayable;
	
	// Is this track playing?
	int playing;
	
	// Was this track playing when the mute happened?
	int muted;
	
	// Should this track loop?
	int loop;
	
	// The sequence representing the MIDI file
	// These are loaded once and re-used
	MusicSequence sequence;
	int sequence_loaded;
	
	// The measured track length, in beats
	MusicTimeStamp track_length;
	
	// The current music player, if any
	// This is initialized in music_start() and schedulde for destruction in music_stop()
	MusicPlayer player;
};
struct music_file music_files[M_MAX_MUSIC];

// Keep track of MusicPlayers that need to be disposed of
// Don't tear them down right away, since they'd still be reverberating, and it would sound bad
struct disposable_track {
	struct disposable_track * next;
	MusicPlayer player;
	time_t dispose_after;
};
struct disposable_track * disposable_tracks;

// Ensure that the specified music track is in memory, loading it if required
void music_load(enum music_track track)
{
	char name[80] = "";
	FILE * midi_f;
	char * midi_data = NULL;
	int len;
	int i;
	OSStatus rv;
	
	// Don't (try to) load the track twice
	if (music_files[track].sequence_loaded || music_files[track].unplayable)
		return;
	
	// Find the name for this track
	for (i = 0; music_key[i].track != M_MAX_MUSIC; i++) {
		if (music_key[i].track == track && music_key[i].name) {
			snprintf(name, sizeof(name), "%s.MID", music_key[track].name);
			break;
		}
	}
	
	// Bail out if this track isn't known
	if (strlen(name) == 0) {
		music_files[track].unplayable = 1;
		return;		
	}
	
	// Read the MIDI file into memory
	midi_f = sOpen(name, "rb", FT_MIDI);
	if (midi_f == NULL) {
		len = -1;
	} else {
		len = fread_dyn(&midi_data, &len, midi_f);
		fclose(midi_f);
	}
	
	if (len < 0 || midi_data == NULL) {
		WARNING2("could not read MIDI file `%s'", name);
		music_files[track].unplayable = 1;
		return;
	}
	
	// Initialize the sequence
	rv = NewMusicSequence(&music_files[track].sequence);
	if (rv == 0) {
		// Try to load the sequence out of this buffer
		rv = MusicSequenceLoadSMFData(music_files[track].sequence, CFDataCreate(NULL, (UInt8*)midi_data, len));		
	}
		
	// Regardless, free the buffer we read off disk, if any
	free(midi_data);
	
	// Did it work?
	if (rv) {
		WARNING2("could not understand MIDI file `%s'", name);
		music_files[track].unplayable = 1;		
	} else {
		music_files[track].sequence_loaded = 1;
	}
}

// Start playing the given track
void music_start_loop(enum music_track track, int loop)
{
	OSStatus result;
	
	// Load as necessary
	music_load(track);
	
	// Ensure we can we play this
	if (!music_files[track].sequence_loaded)
		return;
	
	// Keep track of if we should loop or not
	music_files[track].loop = loop;
	
	// Is this already playing, or would it be playing if we weren't muted?
	if (music_files[track].playing || music_files[track].muted)
		return;
	
	// TODO: fix the code that calls the music system so that we don't have two tracks running at once
	// For now, stop any other tracks
	music_stop();
	
	// If we're muted, mark this track as muted, and bail out
	if (mute_music) {
		music_files[track].muted = 1;
		return;
	}
	
	// Initialize the music player and attach the sequence
	require_noerr(result = NewMusicPlayer(&music_files[track].player), fail);
	require_noerr(result = MusicPlayerSetSequence(music_files[track].player, music_files[track].sequence), fail);
	
	// Determine the track length if we don't know it already
	if (music_files[track].track_length <= 1.0) {
		UInt32 track_count, i;
		require_noerr(result = MusicSequenceGetTrackCount(music_files[track].sequence, &track_count), fail);

		for (i = 0; i < track_count; i++) {
			MusicTrack midi_track;
			MusicTimeStamp track_length;
			UInt32 property_size = sizeof(MusicTimeStamp);
			
			// Get the track within the MIDI file
			require_noerr(result = MusicSequenceGetIndTrack(music_files[track].sequence, i, &midi_track), fail);
			
			// Determine its length
			require_noerr(result = MusicTrackGetProperty(midi_track, kSequenceTrackProperty_TrackLength, &track_length, &property_size), fail);
			
			// Update the music track length if this MIDI track is longer
			if (track_length > music_files[track].track_length)
				music_files[track].track_length = track_length;
		}		
	}
	
	// Seek and pre-roll to the beginning
	require_noerr(result = MusicPlayerSetTime(music_files[track].player, 0), fail);
	require_noerr(result = MusicPlayerPreroll(music_files[track].player), fail);
	
	// Start playing
	require_noerr(result = MusicPlayerStart(music_files[track].player), fail);
	
	// Mark as playing
	music_files[track].playing = 1;
	
	return;
		
fail:
	WARNING2("unexpected error playing MIDI track; result=%ld", result);
	music_files[track].unplayable = 1;
	return;	
}

// Stop a specific track
void music_stop_track(enum music_track track)
{
	struct disposable_track *d, *cur;
	
	// Do nothing unless the track is playing
	if (!music_files[track].playing)
		return;
	
	// Seek past the end of the file to stop new music from coming out
	MusicPlayerSetTime(music_files[track].player, music_files[track].track_length + 1);
	
	// Move the MusicPlayer to a new "dispose me" entry
	d = (struct disposable_track *)malloc(sizeof(struct disposable_track));
	d->next = NULL;
	d->player = music_files[track].player;
	d->dispose_after = time(NULL) + 5;
	
	// Add to the end of the list, or make a new list as required
	cur = disposable_tracks;
	while (cur && cur->next)
		cur = cur->next;
	if (cur)
		cur->next = d;
	else
		disposable_tracks = d;
	
	// Mark as not playing and not muted
	music_files[track].playing = 0;
	music_files[track].muted = 0;
}

// Stop all tracks
void music_stop()
{
	int i;
	
	// Iterate through the list and stop any playing tracks by calling music_stop_track()
	for (i = 0; i < M_MAX_MUSIC; i ++) {
		if (music_files[i].playing || music_files[i].muted) {
			music_stop_track(i);
		}
	}
}

int music_is_playing()
{
	int i;
	for (i = 0; i < M_MAX_MUSIC; i ++) {
		if (music_files[i].playing || music_files[i].muted) {
			return 1;
		}
	}
	
	return 0;
}

int music_is_track_playing(enum music_track track)
{
	return music_files[track].playing || music_files[track].muted;
}

void music_pump()
{
	int i;
	MusicTimeStamp current_beat;

	// Dispose up to one MusicPlayer that wants disposing
	if (disposable_tracks && disposable_tracks->dispose_after < time(NULL)) {
		struct disposable_track * next;
		
		// Stop and free
		MusicPlayerStop(disposable_tracks->player);
		DisposeMusicPlayer(disposable_tracks->player);
		
		// Free this record, and move the list to the next record, if any
		next = disposable_tracks->next;
		free(disposable_tracks);
		disposable_tracks = next;
	}
	
	// Iterate over all tracks
	for (i = 0; i < M_MAX_MUSIC; i ++) {	
		// Check to see if we need to loop this track
		if (music_files[i].playing && music_files[i].loop) {
			// Check to see where we are on this track
			MusicPlayerGetTime(music_files[i].player, &current_beat);
			
			if (current_beat > music_files[i].track_length) {
				// We've played past the end of the track, but are supposed to loop
				// Seek back to the beginning
				MusicPlayerSetTime(music_files[i].player, 1);
			}
		}
	}
}

void music_set_mute(int muted)
{
	int i;
	
	if (muted && !mute_music) {
		// Mute all playing tracks
		for (i = 0; i < M_MAX_MUSIC; i++) {
			if (music_files[i].playing) {
				// Found a track that should be muted
				// Stop the track
				music_stop_track(i);
				
				// Mark the track as muted
				music_files[i].muted = 1;
			}
		}
		
		// Set mute flag
		mute_music = 1;
	}
	
	if (!muted && mute_music) {
		// Un-set mute flag
		mute_music = 0;

		// Start all muted tracks
		for (i = 0; i < M_MAX_MUSIC; i++) {
			if (music_files[i].muted) {
				// Found a track that should be playing
				// Mark the track as no longer muted
				music_files[i].muted = 0;
				
				// Start the track, preserving the loop flag
				music_start_loop(i, music_files[i].loop);
			}
		}
	}
	
}
