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
        LOG_WARNING("unable to decode stream");
        return;
    }

    LOG_TRACE("trying theora decoder...");
    init_theora(); // if it's a video file, is_video() will be returning true
    if (!good) {
        LOG_WARNING("unable to decode stream");
        return;
    }

    LOG_TRACE("trying vorbis decoder...");
    init_vorbis(); // if it's an audio file, is_audio() will be returning true
    if (!good) {
        LOG_WARNING("unable to decode stream");
        return;
    }

    if (is_audio()) {
        LOG_TRACE("audio %u channel(s) at %u Hz", channels(), audio_rate());
    }

    if (is_video()) {
        LOG_INFO("video %ux%u pixels at %g fps", w(), h(), fps());
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

    return *this;
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
        LOG_WARNING("requested decode but AUDIO is set to ignore");
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
                LOG_WARNING("packet does not contain a valid vorbis frame");
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
        LOG_WARNING("requested decode but VIDEO is set to ignore");
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
            LOG_WARNING("packet does not contain theora frame");
            /* get next packet */
        }
    }

    yuv_buffer yuv;
    theora_decode_YUVout(mmf.video_ctx.get(), &yuv);

    uint8_t* yp = yuv.y;
    uint8_t* up;
    uint8_t* vp;
    switch (ovl.format) {
    case SDL_IYUV_OVERLAY:
        up = yuv.u;
        vp = yuv.v;
        break;
        
    case SDL_YV12_OVERLAY:
        up = yuv.v;
        vp = yuv.u;
        break;

    default:
        LOG_WARNING("only IYUV and YV12 SDL overlay formats supported");
        return false;
    }

    if (mmf.video_info.get()->pixelformat != OC_PF_420) {
        LOG_WARNING("unknown/unsupported theora pixel format");
        return false;
    }

    if (SDL_LockYUVOverlay(&ovl) < 0) {
        LOG_WARNING("unable to lock overlay");
        return false;
    }

    unsigned h = std::min(mmf.video_info.get()->frame_height, (unsigned)ovl.h);
    unsigned w = std::min(mmf.video_info.get()->frame_width, (unsigned) ovl.w);
    unsigned xoff = mmf.video_info.get()->offset_x;
    unsigned yoff = mmf.video_info.get()->offset_y;
    
    /* luna goes first */
    for (unsigned i = 0; i < h; ++i) {
        memcpy(ovl.pixels[0] + i * ovl.pitches[0],
               yp + (i + yoff) * yuv.y_stride + xoff, w);
    }

    xoff /= 2;
    yoff /= 2;
    /* round up */
    w = w / 2 + w % 2;
    h = h / 2 + h % 2;

    /* handle 2x2 subsampled u and v planes */
    for (unsigned i = 0; i < h; ++i) {
        memcpy(ovl.pixels[1] + i * ovl.pitches[1],
               up + (i + yoff) * yuv.uv_stride + xoff, w);
        memcpy(ovl.pixels[2] + i * ovl.pitches[2],
               vp + (i + yoff) * yuv.uv_stride + xoff, w);
    }

    SDL_UnlockYUVOverlay(&ovl);
    return true;
}

// loads data from file through sync buffer into ogg page last_read
void Multimedia::get_page()
{
    while (true) {
        int res = ogg_sync_pageout(&mmf.sync, &last_read); // attempt loading from the buffer
        if (res < 0) { // catastropic error
            good = false;
            return;
        }
        if (res > 0) { // load succesful
        /* XXX: following may segfault if non-ogg file is read */
            if (ogg_page_version(&last_read) != 0) {good = false;} // idk
            return;
        }   
        // otherwise load into sync buffer
        const int bufsize = 8192;
        char* p = ogg_sync_buffer(&mmf.sync, bufsize); // give us space to write into
        if (p == nullptr) {
            LOG_ERROR("ogg buffer synchronization failed");
            good = false;
            return;
        }

        int n = fread(p, 1, bufsize, mmf.file); // write into buffer
        if (n == 0) { // either error or EOF
            good = false;
            return;
        }

        if (ogg_sync_wrote(&mmf.sync, n)) { // notify buffer how much we have written
            LOG_ERROR("buffer overflow in ogg_sync_wrote");
            good = false;
            return;
        }
    }
}

// video parser
void Multimedia::init_theora()
{
    Ogg_stream_raii stream{&last_read};

    // check that we're reading header of a new logical ogg stream
    if (ogg_page_packets(&last_read) != 1 || ogg_page_granulepos(&last_read) != 0) {
        return;
    }

    if (ogg_stream_pagein(stream.get(), &last_read)) // load into ogg stream
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
            if (ogg_stream_pagein(stream.get(), &last_read) < 0) {
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
            LOG_INFO("incompatible theora file");
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
    Ogg_stream_raii stream{&last_read};

    // check that we're at the start of a new logical ogg stream
    if (ogg_page_packets(&last_read) != 1 || ogg_page_granulepos(&last_read) != 0) {
        return;
    }

    if (ogg_stream_pagein(stream.get(), &last_read) < 0)
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
            
            if (ogg_stream_pagein(stream.get(), &last_read) < 0) { // and feed to the stream buffer
                good = false; return;
            }
        }

        // try parsing the header
        switch (vorbis_synthesis_headerin(vo_info.get(), vo_comm.get(), &pkt)) {
        case 0:
            break;

        case OV_EBADHEADER:
            LOG_INFO("bad vorbis header");

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
        if (ogg_stream_pagein(stream.get(), &last_read) >= 0) continue; // if no error, continue in the loop
        // otherwise check if it's an ogg page from the other stream
        if (other.get() != nullptr && ogg_stream_pagein(other.get(), &last_read) == 0) {
            // If user needs it, move on - page gets buffered in other
            if (!ignore_stream[other_type]) continue;
            
            // otherwise just drop the packets
            ogg_packet packet;
            while (ogg_stream_packetout(other.get(), &packet));
        } else {
            LOG_INFO("got page not associated with any stream, "
                     "serial 0x%x", ogg_page_serialno(&last_read));
        }
    }

    if (pkt.e_o_s) stream_has_ended[media] = true;
    return pkt;
}

/* vi: set noet ts=4 sw=4 tw=78: */
