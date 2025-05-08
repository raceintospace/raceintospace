#ifndef SDLHELPER_H
#define SDLHELPER_H

#include <SDL.h>

struct audio_chunk {
    struct audio_chunk *next;
    void *data;
    unsigned size;
    int loop;
};

struct audio_channel {
    unsigned                volume;
    unsigned                mute;
    struct audio_chunk     *chunk;           // played chunk
    struct audio_chunk    **chunk_tailp;     // tail of chunk list?
    unsigned                offset;          // data offset in chunk
};


int IsChannelMute(int channel);
void NUpdateVoice(void);
void av_step(void);
void av_silence(int channel);
void MuteChannel(int channel, int mute);
char AnimSoundCheck(void);
void av_block(void);
void UpdateAudio(void);
void av_set_fading(int type, int from, int to, int steps, int preserve);
void av_sync(void);
void av_setup(void);
void play(struct audio_chunk *cp, int channel);
bool is_new_key_available();
 int get_pressed_key();
void purge_key_buffer();

extern int av_mouse_cur_x;
extern int av_mouse_cur_y;
extern int av_mouse_pressed_x;
extern int av_mouse_pressed_y;
extern int av_mouse_pressed_cur;
extern int av_mouse_pressed_latched;

#define AV_NUM_CHANNELS     2
#define AV_ALL_CHANNELS     -1
#define AV_SOUND_CHANNEL    0
#define AV_MUSIC_CHANNEL    1
#define AV_MAX_VOLUME       SDL_MIX_MAXVOLUME

#define AV_FADE_IN          0
#define AV_FADE_OUT         1

#endif // SDLHELPER_H
