/*
    Copyright (C) 2007 Pace Willison
    Copyright (C) 2007 Krzysztof Kosciuszkiewicz

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

/**
 * \file sdl.c Simple DirectMedia Layer subsystem.
 *
 * Low-level event processing, graphic, audio and video routines.
 * For API and general overview visit http://www.libsdl.org/.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <SDL.h>
#include <Buzz_inc.h>
#include <macros.h>
#include <options.h>
#include <utils.h>
#include <logging.h>
#include <av.h>
#define MAX_X	320
#define MAX_Y	200

LOG_DEFAULT_CATEGORY(sdl)

#define KEYBUF_SIZE 256
static int keybuf[KEYBUF_SIZE];
static int keybuf_in_idx, keybuf_out_idx;

int av_mouse_cur_x, av_mouse_cur_y;
int av_mouse_pressed_x, av_mouse_pressed_y;
int av_mouse_pressed_cur;
int av_mouse_pressed_latched;

unsigned char *screen;

SDL_Surface *display;
SDL_Overlay *video_overlay;
SDL_Overlay *news_overlay;
SDL_Rect video_rect;
SDL_Rect news_rect;
static SDL_Surface *screen_surf;
static SDL_Surface *screen_surf2x;

static SDL_Color pal_colors[256];

static struct audio_channel Channels[AV_NUM_CHANNELS];

/** RGB color palette. */
unsigned char pal[3 * 256];

/* information about current fading operation */
static struct fade_information {
    unsigned from;
    unsigned to;
    unsigned step;
    unsigned steps;
    unsigned force_black;
    int inc;
    unsigned end;
} fade_info;

/** Indicates that screen redraw is required. */
int screen_dirty;

static int have_audio;

static int do_fading;

static SDL_AudioSpec audio_desired;

static unsigned char *dirty_tree;
static unsigned dirty_tree_length;
static SDL_Rect *dirty_rect_list;

static void
alloc_dirty_tree(void)
{
    int depth = AV_DTREE_DEPTH + 1;
    int ratio = 1;
    int bytes = 0;

    /* use power series formula, S = (1 - q**(n+1))/(1 - q) */
    while (depth--)
        ratio *= 4;
    bytes = (1 - ratio) / (1 - 4);
    dirty_tree = xcalloc(bytes, 1);
    dirty_tree_length = bytes;

    ratio /= 4;
    dirty_rect_list = xcalloc(ratio, sizeof(SDL_Rect));
}

static int get_dirty_rect_list();

static void
audio_callback(void *userdata, Uint8 * stream, int len)
{
	int ch = 0;

	memset(stream, 0, len);

	for (ch = 0; ch < AV_NUM_CHANNELS; ++ch)
	{
		int pos = 0;
		struct audio_channel *chp = &Channels[ch];

		if (!chp->mute && chp->volume)
		{
			struct audio_chunk *ac = chp->chunk;

			while (ac)
			{
				int bytes =
					min(len - pos, (int) ac->size - (int) chp->offset);

				int i = 0;
				int16_t* dst = (int16_t*) (stream + pos);
				const int16_t* src = (int16_t*)((uint8_t*) ac->data + chp->offset);

				for (i = 0; i < bytes/2; ++i)
				{
					dst[i] += src[i] * chp->volume / AV_MAX_VOLUME / AV_NUM_CHANNELS;
				}

				pos += bytes;
				chp->offset += bytes;

				if (chp->offset == ac->size)
				{
					chp->offset = 0;
					if (!ac->loop)
					{
						ac = chp->chunk = chp->chunk->next;
						if (!chp->chunk)
							chp->chunk_tailp = &chp->chunk;
						/* why this tailp?? */
					}
				}

				if (pos == len)
					break;

			}
		}
	}
}

/** Check if animation sound playback is in progress.
 * Currently #AV_SOUND_CHANNEL is used only for animation sounds.
 * \return 0 means busy playing audio; 1 means idle
 */
char
AnimSoundCheck(void)
{
	/* assume sound channel */
	av_step();
	if (Channels[AV_SOUND_CHANNEL].chunk)
		return (0);
	return (1);
}

int
IsChannelMute(int channel)
{
	assert(channel >= 0 && channel < AV_NUM_CHANNELS);
	if (!have_audio)
		return 1;
	return Channels[channel].mute;
}

void
play(struct audio_chunk *new_chunk, int channel)
{
	struct audio_chunk *cp;
	struct audio_channel *chp;

	assert(channel >= 0 && channel < AV_NUM_CHANNELS);

	chp = &Channels[channel];

	if (!have_audio)
		return;

	SDL_LockAudio();
	for (cp = chp->chunk; cp; cp = cp->next)
	{
		if (cp == new_chunk)
		{
			DEBUG1("attempt to do add duplicate chunk");
			av_silence(channel);
			break;
		}
	}

	new_chunk->next = NULL;
	*chp->chunk_tailp = new_chunk;
	SDL_UnlockAudio();
}

void
av_silence(int channel)
{
	int i = channel;

	if (channel == AV_ALL_CHANNELS)
	{
		for (i = 0; i < AV_NUM_CHANNELS; ++i)
			av_silence(i);
	}
	else
	{
		assert(channel >= 0 && channel < AV_NUM_CHANNELS);
		if (Channels[channel].chunk)
		{
			SDL_LockAudio();
			Channels[channel].chunk = NULL;
			Channels[channel].chunk_tailp = &Channels[channel].chunk;
			Channels[channel].offset = 0;
			SDL_UnlockAudio();
		}
	}
}

Uint32
sdl_timer_callback(Uint32 interval, void *param)
{
	static SDL_Event tick;

	tick.type = SDL_USEREVENT;
	SDL_PushEvent(&tick);
	return (interval);
}

/**
 * Setup SDL audio, video and window subsystems.
 */
void
av_setup(void)
{
	unsigned video_flags = SDL_SWSURFACE;

#ifndef CONFIG_MACOSX
	char *icon_path = NULL;
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		CRITICAL2("SDL_Init error: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	atexit(SDL_Quit);

	if (options.want_audio)
	{
#ifdef CONFIG_WIN32
		/*
		 * default direct-audio-something has got unreasonably long audio buffers,
		 * but if user knows what he's doing then no problemo...
		 */
		if (!SDL_getenv("SDL_AUDIODRIVER"))
		{
			INFO1("fixing WIN32 audio driver setup");
			SDL_putenv("SDL_AUDIODRIVER=waveout");
		}
		/*
		 * also some sources mention that on win audio needs to be initialised
		 * together with video. Maybe, it works for me as it is now.
		 */
#endif
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			ERROR2("audio initialization failed: %s", SDL_GetError());
		}
		else
		{
			INFO1("audio subsystem initialized");
			have_audio = 1;
		}
	}
	else
		NOTICE1("no audio");

	if (options.want_fullscreen)
	{
		video_flags |= SDL_FULLSCREEN;
		NOTICE1("fullscreen mode enabled");
	}

#ifndef CONFIG_MACOSX
	if ((icon_path = locate_file("moon_32x32.bmp", FT_IMAGE)))
	{
		SDL_Surface *icon = SDL_LoadBMP(icon_path);

		if (icon != NULL)
			SDL_WM_SetIcon(icon, NULL);
		else
			INFO2("setting icon failed: %s\n", SDL_GetError());
		free(icon_path);
	}
#endif

#ifdef PACKAGE_BUILD
	SDL_WM_SetCaption(PACKAGE_NAME " " PACKAGE_VERSION
			" build " PACKAGE_BUILD, NULL);
#else
	SDL_WM_SetCaption(PACKAGE_STRING, NULL);
#endif

	if ((display = SDL_SetVideoMode(MAX_X * 2, MAX_Y * 2, 24, video_flags))
			== NULL)
	{
		CRITICAL2("SDL_SetVideoMode failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	screen = xcalloc(MAX_X * MAX_Y, 1);
	screen_surf = SDL_CreateRGBSurfaceFrom(screen, MAX_X, MAX_Y, 8, MAX_X,
			0, 0, 0, 0);
	if (!screen_surf)
	{
		CRITICAL2("can't create screen surface: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	screen_surf2x = SDL_CreateRGBSurface(SDL_SWSURFACE, MAX_X * 2, MAX_Y * 2,
			8, ~0, ~0, ~0, 0);
	if (!screen_surf2x)
	{
		CRITICAL2("can't create screen_2x surface: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	/* XXX: Hardcoded video width & height */
	video_overlay = SDL_CreateYUVOverlay(160, 100, SDL_YV12_OVERLAY, display);
	if (!video_overlay)
	{
		CRITICAL2("can't create video_overlay: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	news_overlay = SDL_CreateYUVOverlay(312, 106, SDL_YV12_OVERLAY, display);
	/* XXX: Hardcoded video width & height */
	if (!news_overlay)
	{
		CRITICAL2("can't create news_overlay: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	fade_info.step = 1;
	fade_info.steps = 1;
	do_fading = 1;

	alloc_dirty_tree();

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		SDL_DEFAULT_REPEAT_INTERVAL);

	if (have_audio)
	{
		int i = 0;

		audio_desired.freq = 11025;
		audio_desired.format = AUDIO_S16SYS;
		audio_desired.channels = 1;
		/* audio was unresponsive on win32 so let's use shorter buffer */
		audio_desired.samples = 2048;	/* was 8192 */
		audio_desired.callback = audio_callback;

		/* initialize audio channels */
		for (i = 0; i < AV_NUM_CHANNELS; ++i)
		{
			Channels[i].volume = AV_MAX_VOLUME;
			Channels[i].mute = 0;
			Channels[i].chunk = NULL;
			Channels[i].chunk_tailp = &Channels[i].chunk;
			Channels[i].offset = 0;
		}

		/* we don't care what we got, library will convert for us */
		if (SDL_OpenAudio(&audio_desired, NULL) < 0)
		{
			ERROR2("SDL_OpenAudio error: %s", SDL_GetError());
			NOTICE1("disabling audio");
			have_audio = 0;
		}
		else
			SDL_PauseAudio(0);
	}

	SDL_AddTimer(30, sdl_timer_callback, NULL);
}

static void
av_process_event(SDL_Event * evp)
{
	int c;

	switch (evp->type)
	{
		case SDL_QUIT:
			TRACE2("event %04x", evp->type);
			exit(0);
			break;

		case SDL_USEREVENT:
			/* TRACE2("event %04x", evp->type); */
			break;

		case SDL_KEYDOWN:
			switch (evp->key.keysym.sym)
			{
				case SDLK_UP:    c = UP_ARROW; break;
				case SDLK_DOWN:  c = DN_ARROW; break;
				case SDLK_RIGHT: c = RT_ARROW; break;
				case SDLK_LEFT:  c = LT_ARROW; break;
				case SDLK_HOME:  c = K_HOME;   break;
				case SDLK_END :  c = K_END;    break;
				case SDLK_F1:    c = 0x3B00;   break;
				case SDLK_F2:    c = 0x3C00;   break;
				case SDLK_F3:    c = 0x3D00;   break;
				default:
					c = evp->key.keysym.unicode;
					break;
			}
			TRACE4("event %04x %04x %04x", evp->type,
					evp->key.keysym.sym, evp->key.keysym.unicode);
			if (c)
			{
				keybuf[keybuf_in_idx] = c;
				keybuf_in_idx = (keybuf_in_idx + 1) % KEYBUF_SIZE;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			av_mouse_pressed_cur = 1;
			av_mouse_pressed_latched = 1;
			av_mouse_pressed_x = evp->button.x;
			av_mouse_pressed_y = evp->button.y;
			TRACE5("event %04x %04x %04x %04x", evp->type,
					evp->button.x, evp->button.y, evp->button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			TRACE5("event %04x %04x %04x %04x", evp->type,
					evp->button.x, evp->button.y, evp->button.button);
			av_mouse_pressed_cur = 0;

			/* if we get a mouse wheel event then translate it to arrow keypress */
			if (evp->button.button == SDL_BUTTON_WHEELUP
					|| evp->button.button == SDL_BUTTON_WHEELDOWN)
			{
				SDL_Event ev;
				int up = evp->button.button == SDL_BUTTON_WHEELUP;
				SDLMod mod = SDL_GetModState();
				SDLKey key;
				
				if (mod & KMOD_SHIFT)
					key = up ? SDLK_LEFT : SDLK_RIGHT;
				else
					key = up ? SDLK_UP : SDLK_DOWN;

				ev.type = SDL_KEYDOWN;
				ev.key.type = SDL_KEYDOWN;
				ev.key.state = SDL_RELEASED;
				ev.key.keysym.scancode = 0;
				ev.key.keysym.mod = mod;
				ev.key.keysym.unicode = 0;
				ev.key.keysym.sym = key;
				av_process_event(&ev);
			}
			break;

		case SDL_MOUSEMOTION:
			av_mouse_cur_x = evp->motion.x;
			av_mouse_cur_y = evp->motion.y;
			break;

			/* ignore these events */
		case SDL_KEYUP:
		case SDL_ACTIVEEVENT:
			break;
		default:
			DEBUG2("got unknown event %d", evp->type);
			break;
	}
}

/* non-blocking */
void
av_step(void)
{
	SDL_Event ev;
	
	/* Have the music system update itself as required */
	music_pump();

	while (SDL_PollEvent(&ev))
		av_process_event(&ev);
}

/**
 * Block until an SDL event comes in.
 *
 * We have a 30ms timer going, so that is the
 * maximum wait time.
 */
void
av_block(void)
{
	SDL_Event ev;

	if (SDL_WaitEvent(&ev))
	{
		av_process_event(&ev);
		av_step();				   /* soak up any other currently available events */
	}
}

int
bioskey(int peek)
{
	int c;

	av_step();

	if (peek)
	{
		if (keybuf_in_idx != keybuf_out_idx)
			return (1);
		return (0);
	}

	if (keybuf_in_idx == keybuf_out_idx)
		return (0);

	c = keybuf[keybuf_out_idx];
	keybuf_out_idx = (keybuf_out_idx + 1) % KEYBUF_SIZE;

	return (c);
}

void
UpdateAudio(void)
{
//  av_step ();
}

void
NUpdateVoice(void)
{
	av_step();
}

static SDL_Surface *
SDL_Scale2x(SDL_Surface * src, SDL_Surface * dst)
{
	int x, y, bpp;
	uint8_t *from, *to;
	SDL_Rect clp;
	SDL_PixelFormat *pf;

	assert(src);
	assert(src != dst);

	pf = src->format;

	if (!dst)
		dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
			2 * src->w, 2 * src->h,
			pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);

	if (!dst)
		return NULL;

	bpp = pf->BytesPerPixel;

	if (2 * src->h != dst->h
		|| 2 * src->w != dst->w || bpp != dst->format->BytesPerPixel)
	{
		SDL_SetError("dst surface size or bpp mismatch (%d vs %d)",
			bpp, dst->format->BytesPerPixel);
		return NULL;
	}

	if (bpp == 1)
		SDL_SetColors(dst, pf->palette->colors, 0, pf->palette->ncolors);

	if (SDL_MUSTLOCK(src))
		SDL_LockSurface(src);
	if (SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);

	SDL_GetClipRect(dst, &clp);

	for (y = clp.y / 2; y < clp.y / 2 + clp.h / 2; ++y)
	{
		for (x = clp.x / 2; x < clp.x / 2 + clp.w / 2; ++x)
		{
			from = ((uint8_t *) src->pixels) + y * src->pitch + x * bpp;
			to = ((uint8_t *) dst->pixels) + 2 * y * dst->pitch +
				2 * x * bpp;
			switch (bpp)
			{
#define ASSIGN do { \
					*(TYPE (to)) = *(TYPE from); \
					*(TYPE (to+bpp)) = *(TYPE from); \
					*(TYPE (to+dst->pitch)) = *(TYPE from); \
					*(TYPE (to+dst->pitch+bpp)) = *(TYPE from); \
				} while (0)

				case 1:
#define TYPE (uint8_t *)
					ASSIGN;
					break;
#undef TYPE
				case 2:
#define TYPE (uint16_t *)
					ASSIGN;
					break;
#undef TYPE
				case 3:
#define TYPE (uint8_t *)
					ASSIGN;
					to++;
					from++;
					ASSIGN;
					to++;
					from++;
					ASSIGN;
					to++;
					from++;
					break;
#undef TYPE
				case 4:
#define TYPE (uint32_t *)
					ASSIGN;
					break;
#undef TYPE
#undef ASSIGN
			}
		}
	}

	if (SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
	if (SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);

	return dst;
}

static void
transform_palette(void)
{
    unsigned i, j, step, steps;
    struct range {
        unsigned start, end;
    } ranges[] = {{0, fade_info.from}, {fade_info.to, 256}};

    for (j = 0; j < ARRAY_LENGTH(ranges); ++j)
        for (i = ranges[j].start; i < ranges[j].end; ++i)
        {
            if (!fade_info.force_black)
            {
                pal_colors[i].r = pal[3 * i] * 4;
                pal_colors[i].g = pal[3 * i + 1] * 4;
                pal_colors[i].b = pal[3 * i + 2] * 4;
            }
            else
            {
                pal_colors[i].r = 0;
                pal_colors[i].g = 0;
                pal_colors[i].b = 0;
            }
        }
    step = fade_info.step;
    steps = fade_info.steps;
    /* sanity checks */
    assert(steps != 0 && step <= steps);
    for (i = fade_info.from; i < fade_info.to; ++i)
    {
        /* 
         * This should be done this way, but unfortunately some image files
         * have palettes for which pal * 4 overflows single byte. They display
         * correctly in game, but not when multiplication factor varies.
        pal_colors[i].r = pal[3 * i] * 4 * step / steps;
        pal_colors[i].g = pal[3 * i + 1] * 4 * step / steps;
        pal_colors[i].b = pal[3 * i + 2] * 4 * step / steps;
         */

        pal_colors[i].r = pal[3 * i] * 4;
        pal_colors[i].r = pal_colors[i].r * step / steps;
        pal_colors[i].g = pal[3 * i + 1] * 4;
        pal_colors[i].g = pal_colors[i].g * step / steps;
        pal_colors[i].b = pal[3 * i + 2] * 4;
        pal_colors[i].b = pal_colors[i].b * step / steps;
    }
}

void
av_sync(void)
{
	int num_rect = 0;
	SDL_Rect r;

#ifdef PROFILE_GRAPHICS
	float tot_area = 0;
	int i = 0;
	Uint32 ticks = SDL_GetTicks();
#endif

	SDL_Scale2x(screen_surf, screen_surf2x);
	/* copy palette and handle fading! */
	transform_palette();
	SDL_SetColors(screen_surf2x, pal_colors, 0, 256);
	SDL_BlitSurface(screen_surf2x, NULL, display, NULL);
	if (video_rect.h && video_rect.w)
	{
		av_need_update(&video_rect);
		r.h = 2 * video_rect.h;
		r.w = 2 * video_rect.w;
		r.x = 2 * video_rect.x;
		r.y = 2 * video_rect.y;
		SDL_DisplayYUVOverlay(video_overlay, &r);
	}
	if (news_rect.h && news_rect.w)
	{
		av_need_update(&news_rect);
		r.h = 2 * news_rect.h;
		r.w = 2 * news_rect.w;
		r.x = 2 * news_rect.x;
		r.y = 2 * news_rect.y;
		SDL_DisplayYUVOverlay(news_overlay, &r);
	}
	num_rect = get_dirty_rect_list();
	SDL_UpdateRects(display, num_rect, dirty_rect_list);
#ifdef PROFILE_GRAPHICS
	for (i = 0; i < num_rect; ++i)
		tot_area += dirty_rect_list[i].w * dirty_rect_list[i].h;
	tot_area = tot_area * 100 / (2 * MAX_X) / (2 * MAX_Y);
	TRACE4("%3d rects (%6.2f%%) updated in ~%3ums\n",
		num_rect, tot_area, SDL_GetTicks() - ticks);
#endif
	screen_dirty = 0;
}

void
MuteChannel(int channel, int mute)
{
	int i;

	if (channel == AV_ALL_CHANNELS)
	{
		for (i = 0; i < AV_NUM_CHANNELS; ++i)
			MuteChannel(i, mute);
	}
	else
	{
		assert(channel >= 0 && channel < AV_NUM_CHANNELS);
		Channels[channel].mute = mute;
	}
}

/**
 * Set up screen fade effect.  Fading applies only to a range of palette
 * color indexes. Rest of colors in the palette can be preserved or
 * forced to black.
 *
 * \param type #AV_FADE_IN or #AV_FADE_OUT
 * \param from index of first affected color
 * \param to index of last affected color
 * \param steps how many color change steps to perform
 * \param preserve whether preserve rest of palette colors or not
 */
/* 
 * \note A hack, but hey, it works :)
 * Adding periodic timer won't work, because we can't call av_sync from timer.
 * The only thing allowed is SDL_PushEvent, and we don't have event-driven
 * setup. So for now either this or nothing.
 */
void
av_set_fading(int type, int from, int to, int steps, int preserve)
{
    int dir = (type == AV_FADE_IN) ? 1 : -1;
    unsigned st;  
    unsigned st_end;
    SDL_Rect r = {0, 0, MAX_X, MAX_Y};

    if (!do_fading)
        return;

    if (!steps > 0)
        steps = 5;
    st = (type == AV_FADE_IN) ? 0 : steps;
    st_end = steps - st;

    fade_info.from = from;
    fade_info.to = to;
    fade_info.steps = steps;
    fade_info.step = st;
    fade_info.force_black = !preserve;
    fade_info.inc = dir;
    fade_info.end = st_end;

    for (;fade_info.step != fade_info.end; fade_info.step += fade_info.inc)
    {
        av_need_update(&r);
        av_sync();
        SDL_Delay(10);
    }
    av_need_update(&r);
    av_sync();
}

/** compute area of intersection of rectangles */
inline static int
intersect_area(SDL_Rect *first, SDL_Rect *second)
{
    int isect_h = 0, isect_w = 0;
    SDL_Rect *t;
    /* 
     * Treat dimensions separately. Sort accroding to start point,
     * then compute amount of overlap.
     */
    if (first->x > second->x)
    {
        t = first; first = second; second = t;
    }
    if (first->x + first->w < second->x)
        return 0;
    else
        isect_w = min(second->w, first->x + first->w - second->x);

    if (first->y > second->y)
    {
        t = first; first = second; second = t;
    }
    if (first->y + first->h < second->y)
        return 0;
    else
        isect_h = min(second->h, first->y + first->h - second->y);

    return isect_h * isect_w;
}

static void
update_rect(SDL_Rect *fill, int x, int y, int w, int h, int idx, int level)
{
    SDL_Rect r = {x, y, w, h};
    int nw = w / 2;
    int nh = h / 2;
    int area = 0;

    assert((unsigned)idx < dirty_tree_length);

    /* PRUNING: see if already dirty */
    if (dirty_tree[idx])
        return;

    /* PRUNING: check if covered area > AV_DTREE_FILL_RATIO */
    area = intersect_area(fill, &r);
    if (area == 0)
        return;
    else if (level == AV_DTREE_DEPTH
            || area > AV_DTREE_FILL_RATIO * h * w)
    {
        dirty_tree[idx] = 1;
        return;
    }

    idx *= 4;
    level += 1;

    update_rect(fill, x,      y,      nw,     nh,     idx + 1, level);
    update_rect(fill, x + nw, y,      w - nw, nh,     idx + 2, level);
    update_rect(fill, x,      y + nh, nw,     h - nh, idx + 3, level);
    update_rect(fill, x + nw, y + nh, w - nw, h - nh, idx + 4, level);
}

static void
fill_rect_list(SDL_Rect *arr, int *len, int x, int y, int w, int h,
        int idx, int level)
{
    int nw = w / 2;
    int nh = h / 2;

    if (level > AV_DTREE_DEPTH)
        return;

    assert((unsigned)idx < dirty_tree_length);

    if (dirty_tree[idx])
    {
        /* XXX multiply by 2 because of scaling */
        SDL_Rect r = {2*x, 2*y, 2*w, 2*h};
        memcpy(&arr[(*len)++], &r, sizeof(r));
        return;
    }

    idx *= 4;
    level += 1;

    fill_rect_list(arr, len, x,      y,      nw,     nh,     idx + 1, level);
    fill_rect_list(arr, len, x + nw, y,      w - nw, nh,     idx + 2, level);
    fill_rect_list(arr, len, x,      y + nh, nw,     h - nh, idx + 3, level);
    fill_rect_list(arr, len, x + nw, y + nh, w - nw, h - nh, idx + 4, level);
}

static int
get_dirty_rect_list(void)
{
    int len = 0;
    fill_rect_list(dirty_rect_list, &len, 0, 0, MAX_X, MAX_Y, 0, 0);
    memset(dirty_tree, 0, dirty_tree_length);
    return len;
}

/**
 * Notify graphic subsystem that rectangle has to be redrawn.
 * \param r rectangle coordinates
 */
void
av_need_update(SDL_Rect *r)
{
    update_rect(r, 0, 0, MAX_X, MAX_Y, 0, 0);
    screen_dirty = 1;
}

/**
 * Notify graphic subsystem that rectangle has to be redrawn.
 * \param x1 screen coord. of upper left corner
 * \param y1 screen coord. of upper left corner
 * \param x2 screen coord. of bottom right corner
 * \param y2 screen coord. of bottom right corner
 */
void
av_need_update_xy(int x1, int y1, int x2, int y2)
{
	SDL_Rect r = { x1, y1, x2 - x1 + 1, y2 - y1 + 1 };
	av_need_update(&r);
}
/* vim: set noet ts=4 sw=4 tw=77: */
