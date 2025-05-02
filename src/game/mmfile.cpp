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

/** --
 *
 * \return -1 on error
 * \return  0 on end of file
 * \return  1 on successful page read
 */
static int
get_page(mm_file *mf, ogg_page *pg)
{
    const int bufsize = 8192;
    char *p = NULL;
    int n = 0;
    int res = 0;

    assert(mf);

    while (0 == (res = ogg_sync_pageout(&mf->sync, pg))) {
        p = ogg_sync_buffer(&mf->sync, bufsize);

        if (!p) {
            ERROR1("ogg buffer synchronization failed");
            return -1;
        }

        if (0 == (n = fread(p, 1, bufsize, mf->file))) {
            return (feof(mf->file)) ? 0 : -1;
        }

        if (ogg_sync_wrote(&mf->sync, n)) {
            ERROR1("buffer overflow in ogg_sync_wrote");
            return -1;
        }
    }

    /* XXX: following may segfault if non-ogg file is read */
    if (res < 0 || ogg_page_version(pg) != 0) {
        return -1;
    }

    return 1;
}

static int
get_packet(mm_file *mf, ogg_packet *pkt, enum stream_type type)
{
    ogg_stream_state *stream, *other;
    ogg_page pg;
    enum stream_type other_type;
    int rv = 0;

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
        rv = get_page(mf, &pg);

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

static int
init_theora(mm_file *mf, ogg_page *pg)
{
    int pkts = 0;
    int res = 0;
    int rval = 0;
    theora_info *th_info = NULL;
    theora_comment th_comm;
    ogg_packet pkt;
    ogg_stream_state stream;

    assert(mf);
    th_info = (theora_info *)xmalloc(sizeof(*mf->video_info));
    theora_info_init(th_info);
    theora_comment_init(&th_comm);
    ogg_stream_init(&stream, ogg_page_serialno(pg));

    if (ogg_page_packets(pg) != 1 || ogg_page_granulepos(pg) != 0) {
        goto end;
    }

    if (ogg_stream_pagein(&stream, pg))
        /* should not happen */
    {
        goto end;
    }

    /* Three first packets must go successfully through the loop. */
    for (pkts = 0; pkts < 3; ++pkts) {
        while ((res = ogg_stream_packetpeek(&stream, &pkt)) != 1) {
            if (res < 0
                || get_page(mf, pg) <= 0
                || ogg_stream_pagein(&stream, pg) < 0) {
                rval = -1;
                goto end;
            }
        }

        switch (theora_decode_header(th_info, &th_comm, &pkt)) {
        case 0:
            break;

        case OC_VERSION:
        case OC_NEWPACKET:
            INFO1("incompatible theora file");

        /* fall through */
        case OC_BADHEADER:
        default:
            goto end;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(&stream, &pkt);
    }

    mf->video_ctx = (theora_state *)xmalloc(sizeof(*mf->video_ctx));
    mf->video = (ogg_stream_state *)xmalloc(sizeof(*mf->video));
    memcpy(mf->video, &stream, sizeof(stream));
    theora_decode_init(mf->video_ctx, th_info);
    mf->video_info = th_info;
    rval = 1;
end:
    theora_comment_clear(&th_comm);

    if (rval <= 0) {
        ogg_stream_clear(&stream);
        theora_info_clear(th_info);
        free(th_info);
        mf->video_info = NULL;
    }

    return rval;
}

static int
init_vorbis(mm_file *mf, ogg_page *pg)
{
    int pkts = 0;
    int res = 0;
    int rval = 0;
    vorbis_block *vo_blk = NULL;
    vorbis_info *vo_info = NULL;
    vorbis_comment vo_comm;
    ogg_packet pkt;
    ogg_stream_state stream;

    assert(mf);
    vo_info = (vorbis_info *)xmalloc(sizeof(*vo_info));
    vorbis_info_init(vo_info);
    vorbis_comment_init(&vo_comm);
    ogg_stream_init(&stream, ogg_page_serialno(pg));

    if (ogg_page_packets(pg) != 1 || ogg_page_granulepos(pg) != 0) {
        goto end;
    }

    if (ogg_stream_pagein(&stream, pg) < 0)
        /* should not happen */
    {
        goto end;
    }

    /*
     * Three first packets must go successfully through the loop.
     */
    for (pkts = 0; pkts < 3; ++pkts) {
        while ((res = ogg_stream_packetpeek(&stream, &pkt)) != 1) {
            if (res < 0
                || get_page(mf, pg) <= 0
                || ogg_stream_pagein(&stream, pg) < 0) {
                rval = -1;
                goto end;
            }
        }

        switch (vorbis_synthesis_headerin(vo_info, &vo_comm, &pkt)) {
        case 0:
            break;

        case OV_EBADHEADER:
            INFO1("bad vorbis header");

        case OV_ENOTVORBIS:
        default:
            goto end;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(&stream, &pkt);
    }

    /* maybe print something about comment or etc? */

    mf->audio_ctx = (vorbis_dsp_state *)xmalloc(sizeof(*mf->audio_ctx));
    mf->audio = (ogg_stream_state *)xmalloc(sizeof(*mf->audio));
    vo_blk = (vorbis_block *)xmalloc(sizeof(*vo_blk));
    memcpy(mf->audio, &stream, sizeof(stream));
    vorbis_synthesis_init(mf->audio_ctx, vo_info);
    vorbis_block_init(mf->audio_ctx, vo_blk);
    mf->audio_info = vo_info;
    mf->audio_blk = vo_blk;
    rval = 1;
end:
    vorbis_comment_clear(&vo_comm);

    if (rval <= 0) {
        ogg_stream_clear(&stream);
        vorbis_info_clear(vo_info);
        free(vo_info);
    }

    return rval;
}

static int
yuv_to_overlay(const mm_file *mf, const yuv_buffer *yuv, SDL_Overlay *ovl)
{
    unsigned i, h, w, xoff, yoff;
    uint8_t *yp, *up, *vp;

    assert(mf);
    assert(yuv);
    assert(ovl);

    h = MIN(mf->video_info->frame_height, (unsigned) ovl->h);
    w = MIN(mf->video_info->frame_width, (unsigned) ovl->w);
    xoff = mf->video_info->offset_x;
    yoff = mf->video_info->offset_y;

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

    yp = yuv->y;

    switch (mf->video_info->pixelformat) {
    case OC_PF_420:
        break;

    case OC_PF_422:
    case OC_PF_444:
    default:
        WARNING1("unknown/unsupported theora pixel format");
        return -1;
    }

    if (SDL_LockYUVOverlay(ovl) < 0) {
        WARNING1("unable to lock overlay");
        return -1;
    }

    /* luna goes first */
    for (i = 0; i < h; ++i) {
        memcpy(ovl->pixels[0] + i * ovl->pitches[0],
               yp + (i + yoff) * yuv->y_stride + xoff, w);
    }

    xoff /= 2;
    yoff /= 2;
    /* round up */
    w = w / 2 + w % 2;
    h = h / 2 + h % 2;

    /* handle 2x2 subsampled u and v planes */
    for (i = 0; i < h; ++i) {
        memcpy(ovl->pixels[1] + i * ovl->pitches[1],
               up + (i + yoff) * yuv->uv_stride + xoff, w);
        memcpy(ovl->pixels[2] + i * ovl->pitches[2],
               vp + (i + yoff) * yuv->uv_stride + xoff, w);
    }

    SDL_UnlockYUVOverlay(ovl);
    return 0;
}

/* rval < 0: error, > 0: have audio or video */
int
mm_open_fp(mm_file *mf, FILE *file)
{
    int retval = -1;
    int res = 0;
    int have_vorbis = 0;
    int have_theora = 0;
    ogg_page pg;

    assert(mf);
    memset(mf, 0, sizeof(*mf));

    mf->file = file;

    // This is important.  If there is no file
    // then we need to reset the audio and video
    // pointers so that the other functions
    // ignore the file.
    if (!mf->file) {
        mf->audio = NULL;
        mf->video = NULL;
        return retval;
    }

    ogg_sync_init(&mf->sync);

    /* get first page to start things up */
    if (get_page(mf, &pg) <= 0) {
        goto err;
    }

    TRACE1("trying theora decoder...");
    res = init_theora(mf, &pg);

    if (res < 0) {
        goto err;
    } else {
        have_theora = !!res * MEDIA_VIDEO;
    }

    TRACE1("trying vorbis decoder...");
    res = init_vorbis(mf, &pg);

    if (res < 0) {
        goto err;
    } else {
        have_vorbis = !!res * MEDIA_AUDIO;
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
err:
    WARNING1("unable to decode stream");
    mm_close(mf);
    return retval;
}

int
mm_open(mm_file *mf, const char *fname)
{
    assert(mf);
    assert(fname);
    INFO2("opening file `%s'", fname);
    return mm_open_fp(mf, fopen(fname, "rb"));
}

unsigned
mm_ignore(mm_file *mf, unsigned mask)
{
    unsigned old = mf->drop_packets;

    mf->drop_packets = mask;
    return old;
}

int
mm_close(mm_file *mf)
{
    assert(mf);

    if (mf->file) {
        fclose(mf->file);
        mf->file = NULL;
    }

    if (mf->audio) {
        ogg_stream_destroy(mf->audio);
        mf->audio = NULL;
    }

    if (mf->video) {
        ogg_stream_destroy(mf->video);
        mf->video = NULL;
    }

    if (mf->video_ctx) {
        theora_clear(mf->video_ctx);
        free(mf->video_ctx);
        mf->video_ctx = NULL;
    }

    if (mf->video_info) {
        theora_info_clear(mf->video_info);
        free(mf->video_info);
        mf->video_info = NULL;
    }

    if (mf->audio_blk) {
        vorbis_block_clear(mf->audio_blk);
        free(mf->audio_blk);
        mf->audio_blk = NULL;
    }

    if (mf->audio_ctx) {
        vorbis_dsp_clear(mf->audio_ctx);
        free(mf->audio_ctx);
        mf->audio_ctx = NULL;
    }

    if (mf->audio_info) {
        vorbis_info_clear(mf->audio_info);
        free(mf->audio_info);
        mf->audio_info = NULL;
    }

    ogg_sync_clear(&mf->sync);
    return 0;
}

/**
 * \return rval < 0: no video in file
 */
int
mm_video_info(const mm_file *mf, unsigned *width, unsigned *height,
              float *fps)
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

    if (fps)
        *fps = mf->video_info->fps_numerator
               / mf->video_info->fps_denominator;

    return 1;
}

/**
 * \return rval < 0: no audio in file
 **/
int
mm_audio_info(const mm_file *mf, unsigned *channels, unsigned *rate)
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

int
mm_decode_video(mm_file *mf, SDL_Overlay *ovl)
{
    int rv = 0;
    ogg_packet pkt;
    yuv_buffer yuv;

    assert(mf);

    if (!mf->video) {
        return -1;
    }

    if (mf->drop_packets & MEDIA_VIDEO) {
        WARNING1("requested decode but MEDIA_VIDEO is set to ignore");
        return -1;
    }

    for (;;) {
        rv = get_packet(mf, &pkt, MEDIA_VIDEO);

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

    theora_decode_YUVout(mf->video_ctx, &yuv);

    if (yuv_to_overlay(mf, &yuv, ovl) < 0) {
        return -1;
    }

    return 1;
}

/* for now just 16bit signed values, mono channels FIXME
 * maybe use SDL_AudioConvert() for this */
int
mm_decode_audio(mm_file *mf, void *buf, int buflen)
{
    const int max_val = 32767;
    const int min_val = -32768;
    const int bytes_per_sample = 2;

    int rv = 0, samples = 0, left = 0, total = 0;
    unsigned channels = 0;

    assert(mf);

    if (-1 == mm_audio_info(mf, &channels, NULL)) {
        return -1;
    }

    if (mf->drop_packets & MEDIA_AUDIO) {
        WARNING1("requested decode but MEDIA_AUDIO is set to ignore");
        return -1;
    }

    /* convert buflen [bytes] to left [samples] */
    left = buflen;
    left = left / channels / bytes_per_sample;

    while (left > 0) {
        float **pcm;
        ogg_packet pkt;

        /* also outputs any samples left from last decoding */
        while (left > 0
               && (samples = vorbis_synthesis_pcmout(mf->audio_ctx, &pcm)) > 0) {
            int i = 0;
            unsigned ch = 0;

            samples = MIN(samples, left);

            for (i = 0; i < samples; ++i) {
                for (ch = 0; ch < channels; ++ch) {
                    // lrint is not available on MSVC
#ifdef HAVE_LRINT
                    int val = lrint(pcm[ch][i] * max_val);
#else
                    int val = (int)floor(pcm[ch][i] * max_val);
#endif

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
            rv = get_packet(mf, &pkt, MEDIA_AUDIO);

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
