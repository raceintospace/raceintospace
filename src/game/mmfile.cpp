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

/* Multimedia constructor. Reads from file pointer, decides if it's video or audio file
 * Check is_good() afterwards to see if there's been any error
 */
Multimedia::Multimedia(FILE* fp)
{
    if (fp == nullptr) { // nullptr means some caller issue, just error out
        good = false;
        return;
    }
    mmf.file = fp;
    
    ogg_sync_init(&mmf.sync); // sets up ogg lib's file reading buffer

    /* get first page to start things up */
    get_page(); // read from the file through sync buffer into ogg_page
    if (!good) {
        WARNING1("unable to decode stream");
        return;
    }

    TRACE1("trying theora decoder...");
    init_theora(); // if it's a video file, is_video() will be returning true
    if (!good) {
        WARNING1("unable to decode stream");
        return;
    }

    TRACE1("trying vorbis decoder...");
    init_vorbis(); // if it's an audio file, is_audio() will be returning true
    if (!good) {
        WARNING1("unable to decode stream");
        return;
    }

    if (is_audio()) {
        TRACE3("audio %u channel(s) at %u Hz", channels(), audio_rate());
    }

    if (is_video()) {
        INFO4("video %ux%u pixels at %g fps", w(), h(), fps());
    }

    good = is_audio() || is_video(); // if both parsers rejected the file we are no good
}

// It's easier to replace contents via assigning to temporary than deal with all destructors manually
Multimedia& Multimedia::operator=(Multimedia&& other)
{
    std::swap(ignore_stream, other.ignore_stream);
    std::swap(good, other.good);
    std::swap(stream_has_ended, other.stream_has_ended);
    std::swap(mmf, other.mmf);
    std::swap(last_read, other.last_read);
}

// Destructor, deals with member variables that aren't wrapped in raii
Multimedia::~Multimedia()
{
    if (mmf.file != nullptr) {
        fclose(mmf.file);
    }

    if (mmf.video_ctx != nullptr) {
        theora_clear(mmf.video_ctx.get());
    }

    if (mmf.audio_blk != nullptr) {
        vorbis_block_clear(mmf.audio_blk.get());
    }

    if (mmf.audio_ctx != nullptr) {
        vorbis_dsp_clear(mmf.audio_ctx.get());
    }

    ogg_sync_clear(&mmf.sync);
}

// returns false if some catastrophic parsing error occured
bool Multimedia::is_good() {return good;}

bool Multimedia::is_audio() {return mmf.audio.get() != nullptr;}
int Multimedia::channels() {return (is_audio())? mmf.audio_info.get()->channels : -1;}
int Multimedia::audio_rate() {return (is_audio())? mmf.audio_info.get()->rate : -1;}

// returns number of bytes written to buffer
int Multimedia::decode_audio(void* buf, int buflen)
{
    if (!good) return -1;

    // can't decode audio if we aren't one
    if (!is_audio()) return -1;

    if (ignore_stream[AUDIO]) {
        WARNING1("requested decode but AUDIO is set to ignore");
        return -1;
    }
    
    // only dealing with 16-bit samples (todo?)
    int16_t* audio_buf = (int16_t*)buf;

    /* convert buflen [bytes] to left [samples] */
    const int bytes_per_sample = 2;
    int space_left = buflen / channels() / bytes_per_sample;

    int samples_written = 0;
    while (space_left > 0) {
        /* output samples in audio_ctx (including those left from last decoding) */
        while (true) {
            // ask for audio samples to read
            float** pcm;
            int samples = std::min(vorbis_synthesis_pcmout(mmf.audio_ctx.get(), &pcm), space_left);
            // if we run out of audio samples or buffer space - exit the loop
            if (samples <= 0) break;

            for (int i = 0; i < samples; ++i) {
                for (unsigned ch = 0; ch < channels(); ++ch) {
                    // normalize to int16_t range
                    int val = lrint(pcm[ch][i] * std::numeric_limits<int16_t>::max());

                    // todo: std::clamp
                    if (val > std::numeric_limits<int16_t>::max()) {
                        val = std::numeric_limits<int16_t>::max();
                    }
                    if (val < std::numeric_limits<int16_t>::min()) {
                        val = std::numeric_limits<int16_t>::min();
                    }

                    // maybe replace this with an iterator?
                    audio_buf[(samples_written + i) * channels() + ch] = val;
                }
            }
            // update our counters
            samples_written += samples;
            space_left -= samples;
            // notify parser how many samples we've read
            vorbis_synthesis_read(mmf.audio_ctx.get(), samples);
        }

        /* grab new packets if we need more */
        for (;;) {
            ogg_packet pkt = get_packet(AUDIO);
            if (!good) return -1;
            if (stream_has_ended[AUDIO]) {
                return samples_written * channels() * bytes_per_sample;
            }

            /* have packet, synthesize */
            if (vorbis_synthesis(mmf.audio_blk.get(), &pkt) == 0) {
                vorbis_synthesis_blockin(mmf.audio_ctx.get(), mmf.audio_blk.get());
                break;
            } else {
                WARNING1("packet does not contain a valid vorbis frame");
                /* get next packet */
            }
        }
    }

    return samples_written * channels() * bytes_per_sample;
}

bool Multimedia::is_video() { return mmf.video.get() != nullptr;}
int Multimedia::w() { return (is_video())? mmf.video_info.get()->frame_width : -1;}
int Multimedia::h() { return (is_video())? mmf.video_info.get()->frame_height : -1;}
float Multimedia::fps() { return (is_video())? mmf.video_info.get()->fps_numerator / mmf.video_info.get()->fps_denominator : -1;}

// returns true if there's still more frames to draw
bool Multimedia::draw_video_frame(SDL_Overlay& ovl)
{
    if (!good) return false;

    // can't draw frames if we aren't a video
    if (!is_video()) return false;

    if (ignore_stream[VIDEO]) {
        WARNING1("requested decode but VIDEO is set to ignore");
        return false;
    }

    for (;;) {
        ogg_packet pkt = get_packet(VIDEO);
        if (!good) return false;
        if (stream_has_ended[VIDEO]) return false;

        /* we got packet, decode */
        if (theora_decode_packetin(mmf.video_ctx.get(), &pkt) == 0) {
            break;
        } else {
            WARNING1("packet does not contain theora frame");
            /* get next packet */
        }
    }

    yuv_buffer yuv;
    theora_decode_YUVout(mmf.video_ctx.get(), &yuv);

    uint8_t* yp = &yuv.y;
    uint8_t* up;
    uint8_t* vp;
    switch (ovl.format) {
    case SDL_IYUV_OVERLAY:
        up = &yuv.u;
        vp = &yuv.v;
        break;
        
    case SDL_YV12_OVERLAY:
        up = &yuv.v;
        vp = &yuv.u;
        break;

    default:
        WARNING1("only IYUV and YV12 SDL overlay formats supported");
        return false;
    }

    if (mmf.video_info->pixelformat != OC_PF_420) {
        WARNING1("unknown/unsupported theora pixel format");
        return false;
    }

    if (SDL_LockYUVOverlay(ovl) < 0) {
        WARNING1("unable to lock overlay");
        return false;
    }

    unsigned h = std::min(mmf.video_info.get()->frame_height, (unsigned)ovl->h);
    unsigned w = std::min(mmf.video_info.get()->frame_width, (unsigned) ovl->w);
    unsigned xoff = mmf.video_info.get()->offset_x;
    unsigned yoff = mmf.video_info.get()->offset_y;
    
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
    return true;
}

// loads data from file through sync buffer into ogg page pg
void Multimedia::get_page()
{
    while (true) {
        int res = ogg_sync_pageout(&mmf.sync, &pg); // attempt loading from the buffer
        if (res < 0) { // catastropic error
            good = false;
            return;
        }
        if (res > 0) { // load succesful
        /* XXX: following may segfault if non-ogg file is read */
            if (ogg_page_version(pg) != 0) {good = false;} // idk
            return;
        }   
        // otherwise load into sync buffer
        const int bufsize = 8192;
        char* p = ogg_sync_buffer(&mmf.sync, bufsize); // give us space to write into
        if (p == nullptr) {
            ERROR1("ogg buffer synchronization failed");
            good = false;
            return;
        }

        int n = fread(p, 1, bufsize, mmf.file); // write into buffer
        if (n == 0) { // either error or EOF
            good = false;
            return;
        }

        if (ogg_sync_wrote(&mmf.sync, n)) { // notify buffer how much we have written
            ERROR1("buffer overflow in ogg_sync_wrote");
            good = false;
            return;
        }
    }
}

// video parser
void Multimedia::init_theora()
{
    Ogg_stream_raii stream{&pg};

    // check that we're reading header of a new logical ogg stream
    if (ogg_page_packets(&pg) != 1 || ogg_page_granulepos(&pg) != 0) {
        return;
    }

    if (ogg_stream_pagein(stream.get(), &pg)) // load into ogg stream
        /* should not error */
    {
        return;
    }

    // try parsing first 3 packets as theora header
    Theora_info_raii th_info{};
    Theora_comment_raii th_comm{};
    for (int pkts = 0; pkts < 3; ++pkts) {
        ogg_packet pkt;
        while (true) {
            int res = ogg_stream_packetpeek(stream.get(), &pkt); // check if packet is ready
            if (res == 1) break; // if it is, move on to reading it
            
            if (res < 0) { // catastropic error, abort
                good = false;
                return;
            }

            // otherwise we need to load from the file more
            get_page();
            if (!good) return;

            // and load into the stream
            if (ogg_stream_pagein(stream.get(), &pg) < 0) {
                good = false;
                return;
            }
        }

        //once packet is ready, decode it
        switch (theora_decode_header(th_info.get(), th_comm.get(), &pkt)) {
        case 0: // all good
            break;

        case OC_VERSION:
        case OC_NEWPACKET:
            INFO1("incompatible theora file");
            // [[fallthrough]]
        case OC_BADHEADER:
        default:
            return;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(stream.get(), &pkt);
    }

    /* maybe print something about comment or etc? */

    // if we succeded, move everything into Multimedia member variables
    mmf.video = std::move(stream);
    mmf.video_info = std::move(th_info);
    mmf.video_ctx.reset(new theora_state{});
    theora_decode_init(mmf.video_ctx.get(), mmf.video_info.get());
}

// audio parser
void Multimedia::init_vorbis()
{
    Ogg_stream_raii stream{&pg};

    // check that we're at the start of a new logical ogg stream
    if (ogg_page_packets(&pg) != 1 || ogg_page_granulepos(&pg) != 0) {
        return;
    }

    if (ogg_stream_pagein(stream.get(), &pg) < 0)
        /* should not happen */
    {
        return;
    }

    /*
     * Try reading the 3 header packets
     */
    Vorbis_info_raii vo_info{};
    Vorbis_comment_raii vo_comm{};
    for (int pkts = 0; pkts < 3; ++pkts) {
        ogg_packet pkt;
        while (true) {
            int res = ogg_stream_packetpeek(stream.get(), &pkt); // check if there's enough data to form a packet
            if (res == 1) break; // if pkt successfully formed, continue to reading
            
            if (res < 0) { // catastrophic error
                good = false; return;
            }
            
            get_page(); // load page from the file
            if (!good) return;
            
            if (ogg_stream_pagein(stream.get(), &pg) < 0) { // and feed to the stream buffer
                good = false; return;
            }
        }

        // try parsing the header
        switch (vorbis_synthesis_headerin(vo_info.get(), vo_comm.get(), &pkt)) {
        case 0:
            break;

        case OV_EBADHEADER:
            INFO1("bad vorbis header");

        case OV_ENOTVORBIS:
        default:
            return;
        }

        /* decode successful so grab packet */
        ogg_stream_packetout(stream.get(), &pkt);
    }

    /* maybe print something about comment or etc? */
    
    // if we succeded, move everything into Multimedia member variables
    mmf.audio = std::move(stream);
    mmf.audio_info = std::move(vo_info);
    mmf.audio_ctx.reset(new vorbis_dsp_state{});
    vorbis_synthesis_init(mmf.audio_ctx.get(), mmf.audio_info.get());
    mmf.audio_blk.reset(new vorbis_block{});
    vorbis_block_init(mmf.audio_ctx.get(), mmf.audio_blk.get());
}

// returns next packet for the selected media stream
// responsible for setting stream_has_ended
ogg_packet Multimedia::get_packet(enum media_type media)
{
    ogg_packet pkt;
    
    Ogg_stream_raii& stream = (media==VIDEO)? mmf.video : mmf.audio;
    Ogg_stream_raii& other = (media==VIDEO)? mmf.audio : mmf.video;
    media_type other_type = (media_type) !media;

    if (stream_has_ended[media]) {
        return pkt;
    }

    while (true) {
        // try reading pkt from the stream
        if (0 != ogg_stream_packetout(stream.get(), &pkt)) break; // if it was in buffer, exit the loop
        // otherwise we need to load more
        get_page(); // read next ogg page
        if (!good) { // check that it succeded
            return pkt;
        }

        // read from page into stream
        if (ogg_stream_pagein(stream.get(), &pg) >= 0) continue; // if no error, continue in the loop
        // otherwise check if it's an ogg page from the other stream
        if (other.get() != nullptr && ogg_stream_pagein(other.get(), &pg) == 0) {
            // If user needs it, move on - page gets buffered in other
            if (!ignore_stream[other_type]) continue;
            
            // otherwise just drop the packets
            ogg_packet packet;
            while (ogg_stream_packetout(other.get(), &packet));
        } else {
            INFO2("got page not associated with any stream, "
                  "serial 0x%x", ogg_page_serialno(&pg));
        }
    }

    if (pkt->e_o_s) stream_has_ended[media] = true;
    return pkt;
}

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
