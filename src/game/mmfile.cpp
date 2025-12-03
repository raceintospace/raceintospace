/*
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

// This file handles multimedia files

#include "mmfile.h"

#include <stdint.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <limits>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theora.h>
#include <SDL.h>

#include "raceintospace_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include "fake_unistd.h"
#endif

#include "macros.h"
#include "logging.h"
#include "utils.h"

LOG_DEFAULT_CATEGORY(multimedia)

struct Theora_info_raii
{
    std::unique_ptr<theora_info> th_info;

    Theora_info_raii()
    : th_info{new theora_info{}}
    {
        theora_info_init(th_info);
    }

    theora_info* get() {return th_info.get();}

    theora_info* release() {return th_info.release();}

    ~Theora_info_raii()
    {
        if (th_info == nullptr) return;
        
        theora_info_clear(th_info);
    }
};

struct Theora_comment_raii
{
    theora_comment th_comm;

    Theora_comment_raii()
    {
        theora_comment_init(&th_comm);
    }

    theora_comment* get()
    {
        return &th_comm;
    }

    ~Theora_comment_raii()
    {
        theora_comment_clear(&th_comm);
    }
};

struct Ogg_stream_raii
{
    std::unique_ptr<ogg_stream_state> stream;

    Ogg_stream_raii(ogg_page* pg)
    : stream{new ogg_stream_state{}}
    {
        ogg_stream_init(stream.get(), ogg_page_serialno(pg));
    }

    ogg_stream_state* get() {return stream.get();}

    ogg_stream_state* release() {return stream.release();}

    ~Ogg_stream_raii()
    {
        if (stream == nullptr) return;

        ogg_stream_clear(&stream);
    }
};

struct Vorbis_info_raii
{
    std::unique_ptr<vorbis_info> vo_info;

    Vorbis_info_raii()
    : vo_info{new vorbis_info{}}
    {
        vorbis_info_init(vo_info);
    }

    vorbis_info* get() {return vo_info.get();}
    vorbis_info* release() {return vo_info.release();}

    ~Vorbis_info_raii()
    {
        if (vo_info == nullptr) return;
        vorbis_info_clear(vo_info);
    }
};

struct Vorbis_comment_raii
{
    vorbis_comment vo_comm;

    Vorbis_comment_raii()
    {
        vorbis_comment_init(&vo_comm);
    }

    vorbis_comment* get() {return &vo_comm;}

    ~Vorbis_comment_raii()
    {
        vorbis_comment_clear(&vo_comm);
    }
};

/** --
 *
 * \return -1 on error
 * \return  0 on end of file
 * \return  1 on successful page read
 */
static int get_page(mm_file* mf, ogg_page* pg)
{
    assert(mf);

    while (true) {
        int res = ogg_sync_pageout(&mf->sync, pg);
        if (res < 0) return -1;
        if (res > 0) {
        /* XXX: following may segfault if non-ogg file is read */
            if (ogg_page_version(pg) != 0) return -1;
            return 1;
        }   
        
        const int bufsize = 8192;
        char* p = ogg_sync_buffer(&mf->sync, bufsize);
        if (p == nullptr) {
            ERROR1("ogg buffer synchronization failed");
            return -1;
        }

        int n = fread(p, 1, bufsize, mf->file);
        if (n == 0) {
            return (feof(mf->file)) ? 0 : -1;
        }

        if (ogg_sync_wrote(&mf->sync, n)) {
            ERROR1("buffer overflow in ogg_sync_wrote");
            return -1;
        }
    }
}

static int get_packet(mm_file* mf, ogg_packet* pkt, enum stream_type type)
{
    ogg_stream_state *stream, *other;
    enum stream_type other_type;

    assert(mf);
    assert(pkt);

    switch (type) {
    case MEDIA_VIDEO:
        assert(mf->video);
        stream = mf->video;
        other = mf->audio;
        other_type = MEDIA_AUDIO;
        break;

    case MEDIA_AUDIO:
        assert(mf->audio);
        stream = mf->audio;
        other = mf->video;
        other_type = MEDIA_VIDEO;
        break;

    default:
        WARNING2("bad stream type: %d", type);
        return -1;
    }

    if (mf->end_of_stream & type) {
        return 0;
    }

    while (0 == ogg_stream_packetout(stream, pkt)) {
        ogg_page pg;

        int rv = get_page(mf, &pg);
        if (rv <= 0) {
            return rv;
        }

        if (ogg_stream_pagein(stream, &pg) < 0) {
            if (other && ogg_stream_pagein(other, &pg) == 0) {
                /*
                 * Got page from other stream. If user won't ever decode this
                 * then we need to clean up it here - otherwise read but not
                 * decoded packets would accumulate.
                 */
                if (mf->drop_packets & other_type) {
                    ogg_packet packet;

                    while (ogg_stream_packetout(other, &packet))
                        /* just drop packets */ ;
                }
            } else {
                INFO2("got page not associated with any stream, "
                      "serial 0x%x", ogg_page_serialno(&pg));
                /*
                 * drop page. Ogg source code says ogg_page member pointers are
                 * initialized to static buffers, so there is no need to free
                 * anything.
                 */
            }
        }
    }

    mf->end_of_stream |= (!!pkt->e_o_s) * type;
    return 1;
}

static int init_theora(mm_file* mf, ogg_page* pg)
{
    assert(mf);

    Theora_info_raii th_info{};
    Theora_comment_raii th_comm{};
    Ogg_stream_raii stream{pg};

    if (ogg_page_packets(pg) != 1 || ogg_page_granulepos(pg) != 0) {
        mf->video_info = nullptr;
        return 0;
    }

    if (ogg_stream_pagein(stream.get(), pg))
        /* should not happen */
    {
        mf->video_info = nullptr;
        return 0;
    }

    /* Three first packets must go successfully through the loop. */
    for (int pkts = 0; pkts < 3; ++pkts) {
        ogg_packet pkt;
        while (true) {
            int res = ogg_stream_packetpeek(stream.get(), &pkt);
            if (res == 1) break;
            
            if (res < 0) {
                mf->video_info = nullptr;
                return -1;
            }

            if (get_page(mf, pg) <= 0) {
                mf->video_info = nullptr;
                return -1;
            }

            if (ogg_stream_pagein(stream.get(), pg) < 0) {
                mf->video_info = nullptr;
                return -1;
            }
        }

        switch (theora_decode_header(th_info.get(), th_comm.get(), &pkt)) {
        case 0:
            break;

        case OC_VERSION:
        case OC_NEWPACKET:
            INFO1("incompatible theora file");

        /* fall through */
        case OC_BADHEADER:
        default:
            mf->video_info = nullptr;
            return 0;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(stream.get(), &pkt);
    }

    mf->video_ctx = new theora_state{};
    theora_decode_init(mf->video_ctx, th_info.get());
    mf->video = stream.release();
    mf->video_info = th_info.release();
    return 1;
}

static int init_vorbis(mm_file* mf, ogg_page* pg)
{
    assert(mf);
    
    Vorbis_info_raii vo_info{};
    Vorbis_comment_raii vo_comm{};
    Ogg_stream_raii stream{pg};

    if (ogg_page_packets(pg) != 1 || ogg_page_granulepos(pg) != 0) {
        return 0;
    }

    if (ogg_stream_pagein(stream.get(), pg) < 0)
        /* should not happen */
    {
        return 0;
    }

    /*
     * Three first packets must go successfully through the loop.
     */
    for (int pkts = 0; pkts < 3; ++pkts) {
        ogg_packet pkt;
        while (true) {
            int res = ogg_stream_packetpeek(stream.get(), &pkt);
            if (res == 1) break;
            
            if (res < 0) return -1;
            if (get_page(mf, pg) <= 0) return -1;
            if (ogg_stream_pagein(stream.get(), pg) < 0) return -1;
        }

        switch (vorbis_synthesis_headerin(vo_info.get(), vo_comm.get(), &pkt)) {
        case 0:
            break;

        case OV_EBADHEADER:
            INFO1("bad vorbis header");

        case OV_ENOTVORBIS:
        default:
            return 0;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(stream.get(), &pkt);
    }

    /* maybe print something about comment or etc? */

    mf->audio_ctx = new vorbis_dsp_state{};
    mf->audio_blk = new vorbis_block{};
    mf->audio = stream.release();
    vorbis_synthesis_init(mf->audio_ctx, vo_info.get());
    vorbis_block_init(mf->audio_ctx, mf->audio_blk);
    mf->audio_info = vo_info.release();
    return 1;
}

static int yuv_to_overlay(const mm_file* mf, const yuv_buffer* yuv, SDL_Overlay* ovl)
{
    assert(mf);
    assert(yuv);
    assert(ovl);

    uint8_t* up;
    uint8_t* vp;
    switch (ovl->format) {
    case SDL_IYUV_OVERLAY:
        up = yuv->u;
        vp = yuv->v;
        break;

    case SDL_YV12_OVERLAY:
        up = yuv->v;
        vp = yuv->u;
        break;

    default:
        WARNING1("only IYUV and YV12 SDL overlay formats supported");
        return -1;
    }

    uint8_t* yp = yuv->y;

    if (mf->video_info->pixelformat != OC_PF_420) {
        WARNING1("unknown/unsupported theora pixel format");
        return -1;
    }

    if (SDL_LockYUVOverlay(ovl) < 0) {
        WARNING1("unable to lock overlay");
        return -1;
    }

    unsigned h = MIN(mf->video_info->frame_height, (unsigned) ovl->h);
    unsigned w = MIN(mf->video_info->frame_width, (unsigned) ovl->w);
    unsigned xoff = mf->video_info->offset_x;
    unsigned yoff = mf->video_info->offset_y;
    
    /* luna goes first */
    for (unsigned i = 0; i < h; ++i) {
        memcpy(ovl->pixels[0] + i * ovl->pitches[0],
               yp + (i + yoff) * yuv->y_stride + xoff, w);
    }

    xoff /= 2;
    yoff /= 2;
    /* round up */
    w = w / 2 + w % 2;
    h = h / 2 + h % 2;

    /* handle 2x2 subsampled u and v planes */
    for (unsigned i = 0; i < h; ++i) {
        memcpy(ovl->pixels[1] + i * ovl->pitches[1],
               up + (i + yoff) * yuv->uv_stride + xoff, w);
        memcpy(ovl->pixels[2] + i * ovl->pitches[2],
               vp + (i + yoff) * yuv->uv_stride + xoff, w);
    }

    SDL_UnlockYUVOverlay(ovl);
    return 0;
}

/* rval < 0: error, > 0: have audio or video */
int mm_open_fp(mm_file* mf, FILE* file)
{
    assert(mf);
    memset(mf, 0, sizeof(*mf));

    mf->file = file;

    // This is important.  If there is no file
    // then we need to reset the audio and video
    // pointers so that the other functions
    // ignore the file.
    if (!mf->file) {
        mf->audio = nullptr;
        mf->video = nullptr;
        return -1;
    }

    ogg_sync_init(&mf->sync);

    /* get first page to start things up */
    ogg_page pg;
    if (get_page(mf, &pg) <= 0) {
        WARNING1("unable to decode stream");
        mm_close(mf);
        return -1;
    }

    TRACE1("trying theora decoder...");
    int have_theora = 0;
    switch(init_theora(mf, &pg))
    {
    case 0:
        have_theora = 0;
        break;
    case 1:
        have_theora = MEDIA_VIDEO;
        break;
    case -1: default:
        WARNING1("unable to decode stream");
        mm_close(mf);
        return -1;
    }

    TRACE1("trying vorbis decoder...");
    int have_vorbis = 0;
    switch(init_vorbis(mf, &pg))
    {
    case 1:
        have_vorbis = MEDIA_AUDIO;
        break;
    case 0:
        have_vorbis = 0;
        break;
    case -1: default:
        WARNING1("unable to decode stream");
        mm_close(mf);
        return -1;
    }


    if (have_vorbis) {
        unsigned c = 0, r = 0;
        mm_audio_info(mf, &c, &r);
        TRACE3("audio %u channel(s) at %u Hz", c, r);
    }

    if (have_theora) {
        unsigned w, h;
        float fps;
        mm_video_info(mf, &w, &h, &fps);
        INFO4("video %ux%u pixels at %g fps", w, h, fps);
    }

    return have_vorbis | have_theora;
}

int mm_open(mm_file* mf, const char* fname)
{
    assert(mf);
    assert(fname);
    INFO2("opening file `%s'", fname);
    return mm_open_fp(mf, fopen(fname, "rb"));
}

unsigned mm_ignore(mm_file* mf, unsigned mask)
{
    unsigned old = mf->drop_packets;

    mf->drop_packets = mask;
    return old;
}

int mm_close(mm_file* mf)
{
    assert(mf);

    if (mf->file) {
        fclose(mf->file);
        mf->file = nullptr;
    }

    if (mf->audio) {
        ogg_stream_clear(mf->audio);
        delete mf->audio;
        mf->audio = nullptr;
    }

    if (mf->video) {
        ogg_stream_clear(mf->video);
        delete mf->video;
        mf->video = nullptr;
    }

    if (mf->video_ctx) {
        theora_clear(mf->video_ctx);
        delete mf->video_ctx;
        mf->video_ctx = nullptr;
    }

    if (mf->video_info) {
        theora_info_clear(mf->video_info);
        delete mf->video_info;
        mf->video_info = nullptr;
    }

    if (mf->audio_blk) {
        vorbis_block_clear(mf->audio_blk);
        delete mf->audio_blk;
        mf->audio_blk = nullptr;
    }

    if (mf->audio_ctx) {
        vorbis_dsp_clear(mf->audio_ctx);
        delete mf->audio_ctx;
        mf->audio_ctx = nullptr;
    }

    if (mf->audio_info) {
        vorbis_info_clear(mf->audio_info);
        delete mf->audio_info;
        mf->audio_info = nullptr;
    }

    ogg_sync_clear(&mf->sync);
    return 0;
}

/**
 * \return rval < 0: no video in file
 */
int mm_video_info(const mm_file* mf, 
                  unsigned* width, unsigned* height,
                  float* fps)
{
    assert(mf);

    if (!mf->video) {
        return -1;
    }

    if (width) {
        *width = mf->video_info->frame_width;
    }

    if (height) {
        *height = mf->video_info->frame_height;
    }

    if (fps) {
        *fps = mf->video_info->fps_numerator
               / mf->video_info->fps_denominator;
    }

    return 1;
}

/**
 * \return rval < 0: no audio in file
 **/
int mm_audio_info(const mm_file* mf, unsigned* channels, unsigned* rate)
{
    assert(mf);

    if (!mf->audio) {
        return -1;
    }

    if (channels) {
        *channels = mf->audio_info->channels;
    }

    if (rate) {
        *rate = mf->audio_info->rate;
    }

    return 1;
}

int mm_decode_video(mm_file* mf, SDL_Overlay* ovl)
{
    assert(mf);

    if (!mf->video) {
        return -1;
    }

    if (mf->drop_packets & MEDIA_VIDEO) {
        WARNING1("requested decode but MEDIA_VIDEO is set to ignore");
        return -1;
    }

    for (;;) {
        ogg_packet pkt;
        
        int rv = get_packet(mf, &pkt, MEDIA_VIDEO);
        if (rv <= 0) {
            return rv;
        }

        /* we got packet, decode */
        if (theora_decode_packetin(mf->video_ctx, &pkt) == 0) {
            break;
        } else {
            WARNING1("packet does not contain theora frame");
            /* get next packet */
        }
    }

    yuv_buffer yuv;
    theora_decode_YUVout(mf->video_ctx, &yuv);

    if (yuv_to_overlay(mf, &yuv, ovl) < 0) {
        return -1;
    }

    return 1;
}

/* for now just 16bit signed values, mono channels FIXME
 * maybe use SDL_AudioConvert() for this */
int mm_decode_audio(mm_file* mf, void* buf, int buflen)
{
    const int max_val = 32767;
    const int min_val = -32768;
    const int bytes_per_sample = 2;

    assert(mf);

    unsigned channels;
    if (-1 == mm_audio_info(mf, &channels, nullptr)) {
        return -1;
    }

    if (mf->drop_packets & MEDIA_AUDIO) {
        WARNING1("requested decode but MEDIA_AUDIO is set to ignore");
        return -1;
    }

    /* convert buflen [bytes] to left [samples] */
    int left = buflen / channels / bytes_per_sample;

    int total = 0;
    while (left > 0) {
        /* also outputs any samples left from last decoding */
        while (true) {
            float** pcm;
            int samples = MIN(vorbis_synthesis_pcmout(mf->audio_ctx, &pcm), left);
            if (samples <= 0) break;

            for (int i = 0; i < samples; ++i) {
                for (unsigned ch = 0; ch < channels; ++ch) {
                    int val = lrint(pcm[ch][i] * max_val);

                    if (val > max_val) {
                        val = max_val;
                    }

                    if (val < min_val) {
                        val = min_val;
                    }

                    *((int16_t *) buf + (total + i) * channels + ch) = val;
                }
            }

            total += samples;
            left -= samples;
            vorbis_synthesis_read(mf->audio_ctx, samples);
        }

        /* grab new packets if we need more */
        for (;;) {
            ogg_packet pkt;
            
            int rv = get_packet(mf, &pkt, MEDIA_AUDIO);
            if (rv < 0) {
                return rv;
            } else if (rv == 0) {
                return total * channels * bytes_per_sample;
            }

            /* have packet, synthesize */
            if (vorbis_synthesis(mf->audio_blk, &pkt) == 0) {
                vorbis_synthesis_blockin(mf->audio_ctx, mf->audio_blk);
                break;
            } else {
                WARNING1("packet does not contain a valid vorbis frame");
                /* get next packet */
            }
        }
    }

    return total * channels * bytes_per_sample;
}

/* vi: set noet ts=4 sw=4 tw=78: */
