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

// This file helps with processing A/V.
#include "sdlhelper.h"

#include <cassert>

#include <SDL.h>
#include "display/graphics.h"
#include "display/surface.h"
#include "raceintospace_config.h"

#include "Buzz_inc.h"
#include "options.h"

#define MAX_X   320
#define MAX_Y   200

LOG_DEFAULT_CATEGORY(sdl)

#define KEYBUF_SIZE 256
static int keybuf[KEYBUF_SIZE];
static int keybuf_in_idx, keybuf_out_idx;

int av_mouse_cur_x;
int av_mouse_cur_y;
int av_mouse_pressed_x;
int av_mouse_pressed_y;
int av_mouse_pressed_cur;
int av_mouse_pressed_latched;

static SDL_Color pal_colors[256];

static audio_channel Channels[AV_NUM_CHANNELS];

/* information about current fading operation */
static struct {
    unsigned from;
    unsigned to;
    unsigned step;
    unsigned steps;
    unsigned force_black;
    int inc;
    unsigned end;
} fade_info;

/** Assume we have audio until we try to initialize and find that we can't */
static int have_audio = 1;

static int do_fading;

static SDL_AudioSpec audio_desired;

void audio_callback(void* userdata, Uint8* stream, int len)
{
    int ch = 0;

    memset(stream, 0, len);

    for (ch = 0; ch < AV_NUM_CHANNELS; ++ch) {
        audio_channel* chp = &Channels[ch];
        if(chp->mute) continue;
        if (chp->volume == 0) continue;
        
        int pos = 0;
        audio_chunk* ac = chp->chunk;
        while (ac) {
            int bytes =
                MIN(len - pos, (int) ac->size - (int) chp->offset);
    
            int i = 0;
            int16_t* dst = (int16_t*)(stream + pos);
            const int16_t* src = (int16_t*)((uint8_t*)ac->data + chp->offset);

            for (i = 0; i < bytes / 2; ++i) {
                dst[i] += src[i] * chp->volume / AV_MAX_VOLUME / AV_NUM_CHANNELS;
            }

            pos += bytes;
            chp->offset += bytes;
                
            if (chp->offset == ac->size) {
                chp->offset = 0;
                
                if (!ac->loop) {
                    ac = chp->chunk = chp->chunk->next;
                
                    if (!chp->chunk) {
                        chp->chunk_tailp = &chp->chunk;
                    }
        
                    /* why this tailp?? */
                }
            }
                
            if (pos == len) {
                    break;
            }
        }
    }
}

/** Check if animation sound playback is in progress.
 * Currently #AV_SOUND_CHANNEL is used only for animation sounds.
 * \return 0 means busy playing audio; 1 means idle
 */
char AnimSoundCheck()
{
    /* assume sound channel */
    av_step();

    if (Channels[AV_SOUND_CHANNEL].chunk) {
        return 0;
    }

    return 1;
}

int IsChannelMute(int channel)
{
    assert(channel >= 0 && channel < AV_NUM_CHANNELS);

    if (!have_audio) {
        return 1;
    }

    return Channels[channel].mute;
}

void play(audio_chunk* new_chunk, int channel)
{
    assert(channel >= 0 && channel < AV_NUM_CHANNELS);

    if (!have_audio) {
        return;
    }

    SDL_LockAudio();

    audio_channel* chp = &Channels[channel];
    for (audio_chunk* cp = chp->chunk; cp; cp = cp->next) {
        if (cp != new_chunk) continue;
        
        LOG_DEBUG("attempt to do add duplicate chunk");
        av_silence(channel);
        break;
    }

    new_chunk->next = nullptr;
    *chp->chunk_tailp = new_chunk;
    SDL_UnlockAudio();
}

void av_silence(int channel)
{
    if (channel == AV_ALL_CHANNELS) {
        for (int i = 0; i < AV_NUM_CHANNELS; ++i) {
            av_silence(i);
        }
        return;
    }
    
    assert(channel >= 0 && channel < AV_NUM_CHANNELS);

    if (Channels[channel].chunk == nullptr) return;
    
    SDL_LockAudio();
    Channels[channel].chunk = nullptr;
    Channels[channel].chunk_tailp = &Channels[channel].chunk;
    Channels[channel].offset = 0;
    SDL_UnlockAudio();
}

Uint32 sdl_timer_callback(Uint32 interval, void* param)
{
    static SDL_Event tick;

    tick.type = SDL_USEREVENT;
    SDL_PushEvent(&tick);
    return interval;
}

/**
 * Set up SDL audio, video and window subsystems.
 */
void av_setup()
{
    std::string title(PACKAGE_STRING " " PACKAGE_VERSION);
#ifdef PACKAGE_BUILD

    if (!std::string(PACKAGE_BUILD).empty()) {
        title += " build " PACKAGE_BUILD;
    }

#endif


    display::graphics.create(title, (options.want_fullscreen == 1));


#ifdef SET_SDL_ICON
    std::string icon_path = locate_file("moon_32x32.bmp", FT_IMAGE);

    if (!icon_path.empty()) {
        SDL_Surface *icon = SDL_LoadBMP(icon_path.c_str());

        if (icon != nullptr) {
            SDL_WM_SetIcon(icon, nullptr);
        } else {
            LOG_INFO("setting icon failed: %s\n", SDL_GetError());
        }
    }

#endif


    fade_info.step = 1;
    fade_info.steps = 1;
    do_fading = 1;

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                        SDL_DEFAULT_REPEAT_INTERVAL);

    if (have_audio) {
        audio_desired.freq = 44100;
        audio_desired.format = AUDIO_S16SYS;
        audio_desired.channels = 2;
        /* audio was unresponsive on win32 so let's use shorter buffer */
        audio_desired.samples = 2048;   /* was 8192 */
        audio_desired.callback = audio_callback;

        /* initialize audio channels */
        for (int i = 0; i < AV_NUM_CHANNELS; ++i) {
            Channels[i].volume = AV_MAX_VOLUME;
            Channels[i].mute = 0;
            Channels[i].chunk = nullptr;
            Channels[i].chunk_tailp = &Channels[i].chunk;
            Channels[i].offset = 0;
        }

        /* we don't care what we got, library will convert for us */
        if (SDL_OpenAudio(&audio_desired, nullptr) < 0) {
            LOG_ERROR("SDL_OpenAudio error: %s", SDL_GetError());
            LOG_NOTICE("disabling audio");
            have_audio = 0;
        } else {
            SDL_PauseAudio(0);
        }
    }

    SDL_AddTimer(30, sdl_timer_callback, nullptr);
}

void av_process_event(SDL_Event* evp)
{
    int c;

    switch (evp->type) {
    case SDL_QUIT:
        LOG_TRACE("av_event %04x", evp->type);
        exit(0);
        break;

    case SDL_USEREVENT:
        /* LOG_TRACE("event %04x", evp->type); */
        break;

    case SDL_KEYDOWN:
        switch (evp->key.keysym.sym) {
        case SDLK_UP:
            c = UP_ARROW;
            break;

        case SDLK_DOWN:
            c = DN_ARROW;
            break;

        case SDLK_RIGHT:
            c = RT_ARROW;
            break;

        case SDLK_LEFT:
            c = LT_ARROW;
            break;

        case SDLK_HOME:
            c = K_HOME;
            break;

        case SDLK_END:
            c = K_END;
            break;

        case SDLK_PAGEUP:
            c = K_PGUP;
            break;

        case SDLK_PAGEDOWN:
            c = K_PGDN;
            break;

        case SDLK_F1:
            c = 0x3B00;
            break;

        case SDLK_F2:
            c = 0x3C00;
            break;

        case SDLK_F3:
            c = 0x3D00;
            break;

        default:
            c = evp->key.keysym.unicode;
            break;
        }

        LOG_TRACE("event %04x %04x %04x", evp->type,
               evp->key.keysym.sym, evp->key.keysym.unicode);

        if (c != 0) {
            keybuf[keybuf_in_idx] = c;
            keybuf_in_idx = (keybuf_in_idx + 1) % KEYBUF_SIZE;
        }

        break;

    case SDL_MOUSEBUTTONDOWN:
        av_mouse_pressed_cur = 1;
        av_mouse_pressed_latched = 1;
        av_mouse_pressed_x = evp->button.x;
        av_mouse_pressed_y = evp->button.y;
        LOG_TRACE("event %04x %04x %04x %04x", evp->type,
               evp->button.x, evp->button.y, evp->button.button);
        break;

    case SDL_MOUSEBUTTONUP:
        {
        LOG_TRACE("event %04x %04x %04x %04x", evp->type,
               evp->button.x, evp->button.y, evp->button.button);
        av_mouse_pressed_cur = 0;

        /* if we get a mouse wheel event then translate it to arrow keypress 
           otherwise we're done*/
        if (evp->button.button != SDL_BUTTON_WHEELUP
            && evp->button.button != SDL_BUTTON_WHEELDOWN) break;
        
        SDL_Event ev;
        int up = evp->button.button == SDL_BUTTON_WHEELUP;
        SDLMod mod = SDL_GetModState();
        SDLKey key;

        if (mod & KMOD_SHIFT) {
            key = up ? SDLK_LEFT : SDLK_RIGHT;
        } else {
            key = up ? SDLK_UP : SDLK_DOWN;
        }

        ev.type = SDL_KEYDOWN;
        ev.key.type = SDL_KEYDOWN;
        ev.key.state = SDL_RELEASED;
        ev.key.keysym.scancode = 0;
        ev.key.keysym.mod = mod;
        ev.key.keysym.unicode = 0;
        ev.key.keysym.sym = key;
        av_process_event(&ev);

        break;
        }

    case SDL_MOUSEMOTION:
        av_mouse_cur_x = evp->motion.x;
        av_mouse_cur_y = evp->motion.y;
        break;

    /* ignore these events */
    case SDL_KEYUP:
    case SDL_ACTIVEEVENT:
        break;

    default:
        LOG_TRACE("got unknown event %d", evp->type);
        break;
    }
}

/* non-blocking */
void av_step()
{
    SDL_Event ev;

    /* Have the music system update itself as required */
    music_pump();

    while (SDL_PollEvent(&ev)) {
        av_process_event(&ev);
    }
}

/**
 * Block until an SDL event comes in.
 *
 * We have a 30ms timer going, so that is the
 * maximum wait time.
 */
void av_block()
{
    SDL_Event ev;

    if (SDL_WaitEvent(&ev)) {
        av_process_event(&ev);
        av_step();                 /* soak up any other currently available events */
    }
}

int bioskey(int peek)
{
    av_step();

    if (peek) {
        if (keybuf_in_idx != keybuf_out_idx) {
            return 1;
        }

        return 0;
    }

    if (keybuf_in_idx == keybuf_out_idx) {
        return 0;
    }

    int c = keybuf[keybuf_out_idx];
    keybuf_out_idx = (keybuf_out_idx + 1) % KEYBUF_SIZE;

    return c;
}

void UpdateAudio()
{
//  av_step ();
}

void NUpdateVoice()
{
    av_step();
}

SDL_Surface* SDL_Scale2x(SDL_Surface* src, SDL_Surface* dst)
{
    assert(src);
    assert(src != dst);

    SDL_PixelFormat* pf = src->format;

    if (dst == nullptr)
        dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   2 * src->w, 2 * src->h,
                                   pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);

    if (!dst) {
        return nullptr;
    }

    int bpp = pf->BytesPerPixel;
    if (2 * src->h != dst->h
        || 2 * src->w != dst->w 
        || bpp != dst->format->BytesPerPixel) {
        SDL_SetError("dst surface size or bpp mismatch (%d vs %d)",
                     bpp, dst->format->BytesPerPixel);
        return nullptr;
    }

    if (bpp == 1) {
        SDL_SetColors(dst, pf->palette->colors, 0, pf->palette->ncolors);
    }

    if (SDL_MUSTLOCK(src)) {
        SDL_LockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)) {
        SDL_LockSurface(dst);
    }

    SDL_Rect clp;
    SDL_GetClipRect(dst, &clp);

    for (int y = clp.y / 2; y < clp.y / 2 + clp.h / 2; ++y) {
        for (int x = clp.x / 2; x < clp.x / 2 + clp.w / 2; ++x) {
            uint8_t* from = ((uint8_t *) src->pixels) + y * src->pitch + x * bpp;
            uint8_t* to = ((uint8_t *) dst->pixels) + 2 * y * dst->pitch +
                 2 * x * bpp;

            switch (bpp) {
#define ASSIGN(TYPE) do { \
                    *(TYPE (to)) = *(TYPE from); \
                    *(TYPE (to+bpp)) = *(TYPE from); \
                    *(TYPE (to+dst->pitch)) = *(TYPE from); \
                    *(TYPE (to+dst->pitch+bpp)) = *(TYPE from); \
                } while (0)

            case 1:
                ASSIGN(uint8_t*);
                break;

            case 2:
                ASSIGN(uint16_t*);
                break;

            case 3:
                ASSIGN(uint8_t*);
                to++;
                from++;
                ASSIGN(uint8_t*);
                to++;
                from++;
                ASSIGN(uint8_t*);
                to++;
                from++;
                break;

            case 4:
                ASSIGN(uint32_t*);
                break;
#undef ASSIGN
            }
        }
    }

    if (SDL_MUSTLOCK(dst)) {
        SDL_UnlockSurface(dst);
    }

    if (SDL_MUSTLOCK(src)) {
        SDL_UnlockSurface(src);
    }

    return dst;
}

SDL_Surface* SDL_Scale4x(SDL_Surface* src, SDL_Surface* dst)
{
    assert(src);
    assert(src != dst);
    
    SDL_PixelFormat* pf = src->format;
    if (!dst)
        dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   4 * src->w, 4 * src->h,
                                   pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
    if (!dst) {
        return nullptr;
    }

    int bpp = pf->BytesPerPixel;
    if (4 * src->h != dst->h || 4 * src->w != dst->w || bpp != dst->format->BytesPerPixel) {
        SDL_SetError("dst surface size or bpp mismatch (%d vs %d)", bpp, dst->format->BytesPerPixel);
        return nullptr;
    }

    if (bpp == 1) {
        SDL_SetColors(dst, pf->palette->colors, 0, pf->palette->ncolors);
    }

    if (SDL_MUSTLOCK(src)) {
        SDL_LockSurface(src);
    }

    if (SDL_MUSTLOCK(dst)) {
        SDL_LockSurface(dst);
    }

    SDL_Rect clp;
    SDL_GetClipRect(dst, &clp);

    int src_pitch = src->pitch;
    int dst_pitch = dst->pitch;

    for (int y = 0; y < src->h; ++y) {
        uint8_t* from = (uint8_t *)src->pixels + y * src_pitch;
        unit8_t* to = (uint8_t *)dst->pixels + 4 * y * dst_pitch;

        for (int x = 0; x < src->w; ++x) {
            uint8_t* src_pixel = from + x * bpp;
            uint8_t* dst_pixel = to + 4 * x * bpp;

            for(int i=0; i<4; ++i)
            {
                for(int j=0; j<4; ++j)
                {
                    for(int k=0; k<bpp; ++k)
                    {
                        int byte_idx = (dst_pitch*i+j)*bpp;
                        dst_pixel[byte_idx+k] = src_pixel[k];
                        // we simply copy pixel data into 4x4 pixel data slots
                        // this whole code (together with 2x version) needs replacement anyway,
                        // whenever that will arrive (SDL3, gpu, float-point arbitrary resizing done by the library for us)
                    }
                }
            }
        }
    }

    if (SDL_MUSTLOCK(dst)) {
        SDL_UnlockSurface(dst);
    }

    if (SDL_MUSTLOCK(src)) {
        SDL_UnlockSurface(src);
    }

    return dst;
}

void transform_palette()
{
    struct range {
        unsigned start, end;
    } ranges[] = {{0, fade_info.from}, {fade_info.to, 256}};

    display::AutoPal p(display::graphics.legacyScreen());

    for (int j = 0; j < ARRAY_LENGTH(ranges); ++j) {
        for (int i = ranges[j].start; i < ranges[j].end; ++i) {
            if (!fade_info.force_black) {
                pal_colors[i].r = p.pal[3 * i] * 4;
                pal_colors[i].g = p.pal[3 * i + 1] * 4;
                pal_colors[i].b = p.pal[3 * i + 2] * 4;
            } else {
                pal_colors[i].r = 0;
                pal_colors[i].g = 0;
                pal_colors[i].b = 0;
            }
        }
    }

    int step = fade_info.step;
    int steps = fade_info.steps;
    /* sanity checks */
    assert(steps != 0 && step <= steps);

    for (int i = fade_info.from; i < fade_info.to; ++i) {
        /*
         * This should be done this way, but unfortunately some image files
         * have palettes for which pal * 4 overflows single byte. They display
         * correctly in game, but not when multiplication factor varies.
        pal_colors[i].r = pal[3 * i] * 4 * step / steps;
        pal_colors[i].g = pal[3 * i + 1] * 4 * step / steps;
        pal_colors[i].b = pal[3 * i + 2] * 4 * step / steps;
         */

        pal_colors[i].r = p.pal[3 * i] * 4;
        pal_colors[i].r = pal_colors[i].r * step / steps;
        pal_colors[i].g = p.pal[3 * i + 1] * 4;
        pal_colors[i].g = pal_colors[i].g * step / steps;
        pal_colors[i].b = p.pal[3 * i + 2] * 4;
        pal_colors[i].b = pal_colors[i].b * step / steps;
    }
}

void av_sync()
{
    SDL_Rect r;

#ifdef PROFILE_GRAPHICS
    float tot_area = 0;
    int i = 0;
    Uint32 ticks = SDL_GetTicks();
#endif

    //Screen scaling Options
    if (!options.want_4xscale) {
        SDL_Scale2x(display::graphics.screen()->surface(), display::graphics.scaledScreenSurface());
    }
    else {
        SDL_Scale4x(display::graphics.screen()->surface(), display::graphics.scaledScreenSurface());
    }
    
    transform_palette();
    SDL_SetColors(display::graphics.scaledScreenSurface(), pal_colors, 0, 256);
    SDL_BlitSurface(display::graphics.scaledScreenSurface(), nullptr, display::graphics.displaySurface(), nullptr);

    if (display::graphics.videoRect().h && display::graphics.videoRect().w) {
        r.h = display::graphics.SCALE * display::graphics.videoRect().h;
        r.w = display::graphics.SCALE * display::graphics.videoRect().w;
        r.x = display::graphics.SCALE * display::graphics.videoRect().x;
        r.y = display::graphics.SCALE * display::graphics.videoRect().y;
        SDL_DisplayYUVOverlay(display::graphics.videoOverlay(), &r);
    }

    if (display::graphics.newsRect().h && display::graphics.newsRect().w) {
        r.h = display::graphics.SCALE * display::graphics.newsRect().h;
        r.w = display::graphics.SCALE * display::graphics.newsRect().w;
        r.x = display::graphics.SCALE * display::graphics.newsRect().x;
        r.y = display::graphics.SCALE * display::graphics.newsRect().y;
        SDL_DisplayYUVOverlay(display::graphics.newsOverlay(), &r);
    }

    //TODO: Since we're not always tracking the right dirty area (due to the graphics refactoring)
    // for now we update the entire display every time.
    SDL_UpdateRect(display::graphics.displaySurface(), 0, 0, display::graphics.WIDTH * display::graphics.SCALE, display::graphics.HEIGHT * display::graphics.SCALE);
}

void MuteChannel(int channel, int mute)
{
    if (channel == AV_ALL_CHANNELS) {
        for (int i = 0; i < AV_NUM_CHANNELS; ++i) {
            MuteChannel(i, mute);
        }
        return;
    }
    
    assert(channel >= 0 && channel < AV_NUM_CHANNELS);
    Channels[channel].mute = mute;
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
 * \param preserve whether to preserve rest of palette colors or not
 */
/*
 * \note A hack, but hey, it works :)
 * Adding periodic timer won't work, because we can't call av_sync from timer.
 * The only thing allowed is SDL_PushEvent, and we don't have event-driven
 * setup. So for now either this or nothing.
 */
void av_set_fading(int type, int from, int to, int steps, int preserve)
{
    int dir = (type == AV_FADE_IN) ? 1 : -1;
    unsigned st;
    unsigned st_end;

    if (!do_fading) {
        return;
    }

    if (steps <= 0) {
        steps = 5;
    }

    st = (type == AV_FADE_IN) ? 0 : steps;
    st_end = steps - st;

    fade_info.from = from;
    fade_info.to = to;
    fade_info.steps = steps;
    fade_info.step = st;
    fade_info.force_black = !preserve;
    fade_info.inc = dir;
    fade_info.end = st_end;

    for (; fade_info.step != fade_info.end; fade_info.step += fade_info.inc) {
        av_sync();
        SDL_Delay(10);
    }

    av_sync();
}
