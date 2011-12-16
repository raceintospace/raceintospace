#ifndef _AV_H

#define _AV_H

#include <SDL.h>

#define AV_NUM_CHANNELS     2
#define AV_ALL_CHANNELS     -1
#define AV_SOUND_CHANNEL    0
#define AV_MUSIC_CHANNEL    1
#define AV_MIN_VOLUME       0
#define AV_MAX_VOLUME       SDL_MIX_MAXVOLUME

#define AV_FADE_IN          0
#define AV_FADE_OUT         1

/*
 * 5 => 10x6 px rectangles
 * 4 => 20x12 ... etc
 * minimal = 1
 */
#define AV_DTREE_DEPTH       5
/* 
 * Whole rectangle will be marked as dirty if fill area is higher than this
 */
#define AV_DTREE_FILL_RATIO .8

extern void av_step (void);
extern void av_block (void);
extern void av_setup (void);
extern void av_silence (int channel);
extern void av_sync (void);

struct audio_chunk {
	struct audio_chunk *next;
	void *data;
	unsigned size;
	int loop;
};
extern void play (struct audio_chunk *cp, int channel);

struct audio_channel {
    unsigned                volume;
    unsigned                mute;
#if 0
    unsigned                fade;            /* actual fade value */
    int                     fade_inc;        /* fade inc or dec */
#endif
    struct audio_chunk*     chunk;           /**< played chunk */
    struct audio_chunk**    chunk_tailp;     /**< tail of chunk list? */
    unsigned                offset;          /**< data offset in chunk */
};

extern char AnimSoundCheck(void);
extern void UpdateAudio (void);
extern void MuteChannel(int channel, int mute);

extern double get_time (void);
extern int bioskey (int peek);

extern int av_mouse_cur_x, av_mouse_cur_y;
extern int av_mouse_pressed_x, av_mouse_pressed_y;
extern int av_mouse_pressed_cur;
extern int av_mouse_pressed_latched;

extern SDL_Overlay *video_overlay;
extern SDL_Rect video_rect;
extern SDL_Overlay *news_overlay;
extern SDL_Rect news_rect;
extern void av_need_update(SDL_Rect *r);
extern void av_need_update_xy(int x1, int y1, int x2, int y2);

extern void av_set_fading(int type, int from, int to, int steps, int preserve);

#endif /* ifndef _AV_H */
