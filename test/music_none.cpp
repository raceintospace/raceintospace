// No-op music stubs for the game_test binary.  The test target links
// game_sources but has no audio back-end, so every music call is a
// silent no-op.
#include "music.h"

void music_start_loop(enum music_track, int) {}
void music_stop() {}
void music_stop_all() {}
int  music_is_playing() { return 0; }
int  music_is_track_playing(enum music_track) { return 0; }
void music_pump() {}
void music_set_mute(int) {}
