/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

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
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty
//

// This file handles replay of missions (I think).

#include "replay.h"

#include <cassert>
#include <algorithm>

#include "display/graphics.h"

#include "Buzz_inc.h"
#include "game_main.h"
#include "gamedata.h"
#include "gr.h"
#include "mmfile.h"
#include "pace.h"
#include "sdlhelper.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

/*
 *  Loads and plays video(s) as specified by Type
 *  at position dx dy
 *
 *  Todo: refactor this into a function that plays 1 video and a function that calls first one multiple times if needed
 */
void Replay(char plr, int num, 
            int dx, int dy, 
            int width, int height,
            std::string Type)
{
    std::vector<REPLAY> Rep;

    if (Type == "OOOO") { // replay the whole mission
        Rep = interimData.tempReplay.at((plr * 100) + num);
    } else { // play single video
        Rep.push_back({false, Type});
    }

    // load json files that translate internal video types into filenames
    std::vector<MissionSequenceKey> sSeq, fSeq;
    DESERIALIZE_JSON_FILE(&sSeq, locate_file("seq.json", FT_DATA)); // normal events, training montages
    DESERIALIZE_JSON_FILE(&fSeq, locate_file("fseq.json", FT_DATA)); // failures

    WaitForMouseUp();

    DEBUG2("video sequence: %d segments", Rep.size());

    int dbg_segment_idx = -1;
    for (const REPLAY& r : Rep) {
        ++dbg_segment_idx;
        DEBUG3("playing segment %d: %s", dbg_segment_idx, r.seq.c_str());

        // select correct json
        auto& Sequence_container = (r.Failure)? fSeq : sSeq;

        // find correct step information
        auto Seq_iter = std::find_if(Sequence_container.begin(), 
                                     Sequence_container.end(), 
                                     [&](const MissionSequenceKey& ms){
                                         return ms.MissionIdSequence == r.seq;
                                    });
        // if step information is bogus, bail
        if (Seq_iter == Sequence_container.end()) return;
        
        int max = r.seq.at(1) - '0';
        bool keep_going = true;
        bool exit_replay = false;
        //  update_map = 0;
        for (int i = 0; i < max && keep_going; i++) {
            std::string video_filename = Seq_iter->video.at(i) + ".ogg";

            // TODO: I added this because there are video sequences that
            // do not have a numerical prefix (ex: training videos in
            // ast3.cpp). They should be modified or this check
            // maintained.  -- rnyoakum
            if (strncmp(video_filename.c_str(), "NONE", 4) == 0) {
                // if something went wrong and we are trying to read NONE, move on to the next replay
                break;
            }

            /* here we should create YUV Overlay, but we can't use it on
             * pallettized surface, so we use a global Overlay initialized in
             * sdl.c. */

            INFO2("opening video file `%s'", video_filename.c_str());

            Multimedia vidfile{sOpen(video_filename.c_str(), "rb", FT_VIDEO)};
            if (!vidfile.is_good() || !vidfile.is_video()){
                // if we fail to open video file, stop displaying videos
                exit_replay = true;
                break;
            }

            while (keep_going) {
                int pressed = 0;
                display::graphics.videoRect().x = dx;
                display::graphics.videoRect().y = dy;
                display::graphics.videoRect().w = width;
                display::graphics.videoRect().h = height;

                /** \todo track decoding time and adjust delays */
                if (vidfile.draw_video_frame(*display::graphics.videoOverlay())) {
                    // if video ends (or breaks) - exit loop, move on to the next one
                    break;
                }

                // pressing any key moves to the next video
                if ((pressed = bioskey(0)) || grGetMouseButtons()) {
                    keep_going = false;

                    // pressing Esc leaves replay altogether
                    if (pressed == K_ESCAPE) {
                        exit_replay = true;
                    }
                }

                /** \todo idle_loop is too inaccurate for this */
                idle_loop_secs(1.0 / vidfile.fps());
            }
        }
        if (exit_replay) break;
    }
    display::graphics.videoRect().w = 0;
    display::graphics.videoRect().h = 0;
}

void DispBaby(int x, int y, int loc, char neww)
{
    display::AutoPal p{display::graphics.legacyScreen()};

    display::LegacySurface boob(68, 46);
    uint16_t* bot = (uint16_t*) boob.pixels();

    FILE* fin = sOpen("BABYPICX.CDR", "rb", FT_DATA);
    int32_t locl = (int32_t) 1612 * loc;  // First Image

    fseek(fin, locl, SEEK_SET);

    uint16_t off = 224;
    for (int i = 0; i < 48; i++) {
        p.pal[off * 3 + i] = 0;
    }

    fread(&p.pal[off * 3], 48, 1, fin);
    fread(boob.pixels(), 1564, 1, fin);
    fclose(fin);

    for (int i = 0; i < 782; i++) {
        bot[i + 782] = ((bot[i] & 0xF0F0) >> 4);
        bot[i] = (bot[i] & 0x0F0F);
    }

    for (int i = 0; i < 1564; i++) {
        boob.pixels()[i] += off;
        boob.pixels()[1564 + i] += off;
    }

    boob.copyTo(display::graphics.legacyScreen(), x, y);
}


/**
 * This function is used to display the first frame of the replay in
 * the mission review screen.
 *
 * This relies on the fact that the first video sequence in a mission
 * is a pre-liftoff video that, by its nature, is not a 'failure' and
 * thus is found in seq.json.
 */
void AbzFrame(int plr, int dx, int dy, int width, int height,
              int mission)
{
    AbzFrame(plr, dx, dy, width, height,
             interimData.tempReplay.at(plr * 100 + mission).at(0).seq);
}


void AbzFrame(int plr, int dx, int dy, int width, int height,
              std::string sequence)
{
    // load JSON file that translates from internal sequence code into filename
    std::vector<MissionSequenceKey> sSeq;

    DESERIALIZE_JSON_FILE(&sSeq, locate_file("seq.json", FT_DATA));

    auto Seq_iter = std::find_if(sSeq.begin(), sSeq.end(), 
                                 [&](const MissionSequenceKey& ms){
                                     return ms.MissionIdSequence == sequence;
                                });
    // if step information is bogus, bail
    if (Seq_iter == Sequence_container.end()) return;

    std::string video = Seq_iter->video.at(0) + ".ogg";

    INFO2("opening video file `%s'", video.c_str());

    Multimedia vidfile{sOpen(video.c_str(), "rb", FT_VIDEO)};
    if (!vidfile.is_good() || !vidfile.is_video()) {
        return; // if we failed to open video file, bail
    }

    vidfile.draw_video_frame(display::graphics.videoOverlay());

    display::graphics.videoRect().x = dx;
    display::graphics.videoRect().y = dy;
    display::graphics.videoRect().w = width;
    display::graphics.videoRect().h = height;
}
