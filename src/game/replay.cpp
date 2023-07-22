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

#include "display/graphics.h"

#include "gamedata.h"
#include "Buzz_inc.h"
#include "mmfile.h"
#include "game_main.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

void
Replay(char plr, int num, int dx, int dy, int width, int height,
       std::string Type)
{
    int j;
    std::vector<REPLAY> Rep;
    std::vector<struct MissionSequenceKey> sSeq, fSeq;
    char *fname;

    if (Type == "OOOO") {
        Rep = interimData.tempReplay.at((plr * 100) + num);
    } else {
        Rep.push_back({false, Type});
    }

    mm_file vidfile;
    float fps;

    // TODO: Free up memory in string returned by locate_file.
    fname = locate_file("seq.json", FT_DATA);
    DESERIALIZE_JSON_FILE(&sSeq, fname);

    fname = locate_file("fseq.json", FT_DATA);
    DESERIALIZE_JSON_FILE(&fSeq, fname);

    WaitForMouseUp();

    DEBUG2("video sequence: %d segments", Rep.size());

    for (int kk = 0; kk < Rep.size(); kk++) {
        DEBUG3("playing segment %d: %s", kk, Rep.at(kk).seq.c_str());

        if (Rep.at(kk).Failure) {
            for (j = 0; j < fSeq.size(); j++) {
                if (fSeq.at(j).MissionIdSequence == Rep.at(kk).seq) {
                    break;
                }
            }

            if (j == fSeq.size()) {
                return;
            }
        } else {
            for (j = 0; j < sSeq.size(); j++) {
                if (sSeq.at(j).MissionIdSequence == Rep.at(kk).seq) {
                    break;
                }
            }

            if (j == sSeq.size()) {
                return;
            }
        }

        int max = Rep.at(kk).seq.at(1) - '0';
        bool keep_going = true;

        //  update_map = 0;
        for (int i = 0; i < max && keep_going; i++) {
            char seq_name[20];
            char fname[20];  // TODO: Don't reuse name within function!

            if (Rep.at(kk).Failure) {
                strntcpy(seq_name, fSeq.at(j).video.at(i).c_str(), sizeof(seq_name));
            } else {
                strntcpy(seq_name, sSeq.at(j).video.at(i).c_str(), sizeof(seq_name));
            }

            // TODO: I added this because there are video sequences that
            // do not have a numerical prefix (ex: training videos in
            // ast3.cpp). They should be modified or this check
            // maintained.  -- rnyoakum
            if (strncmp(seq_name, "NONE", MIN(4, sizeof(seq_name))) == 0) {
                break;
            }

            /* here we should create YUV Overlay, but we can't use it on
             * pallettized surface, so we use a global Overlay initialized in
             * sdl.c. */

            /** \todo assumption on file extension */
            snprintf(fname, sizeof(fname), "%s.ogg", seq_name);

            INFO2("opening video file `%s'", fname);

            if (mm_open_fp(&vidfile, sOpen(fname, "rb", FT_VIDEO)) <= 0) {
                goto done;
            }

            /** \todo do not ignore width/height */
            if (mm_video_info(&vidfile, NULL, NULL, &fps) <= 0) {
                goto done;
            }

            while (keep_going) {
                int pressed = 0;
                display::graphics.videoRect().x = dx;
                display::graphics.videoRect().y = dy;
                display::graphics.videoRect().w = width;
                display::graphics.videoRect().h = height;

                /** \todo track decoding time and adjust delays */
                if (mm_decode_video(&vidfile, display::graphics.videoOverlay()) <= 0) {
                    break;
                }

                if ((pressed = bioskey(0)) || grGetMouseButtons()) {
                    keep_going = false;

                    if (pressed == K_ESCAPE) {
                        kk = Rep.size();
                    }
                }

                /** \todo idle_loop is too inaccurate for this */
                idle_loop_secs(1.0 / fps);
            }

            mm_close(&vidfile);
        }
    }

done:
    mm_close(&vidfile);
    display::graphics.videoRect().w = 0;
    display::graphics.videoRect().h = 0;
    return;
}

void
DispBaby(int x, int y, int loc, char neww)
{
    int i;
    FILE *fin;
    uint16_t *bot, off = 0;
    int32_t locl;
    display::AutoPal p(display::graphics.legacyScreen());

    off = 224;

    display::LegacySurface boob(68, 46);
    bot = (uint16_t *) boob.pixels();

    fin = sOpen("BABYPICX.CDR", "rb", FT_DATA);
    locl = (int32_t) 1612 * loc;  // First Image

    fseek(fin, locl, SEEK_SET);

    for (i = 0; i < 48; i++) {
        p.pal[off * 3 + i] = 0;
    }

    fread(&p.pal[off * 3], 48, 1, fin);
    fread(boob.pixels(), 1564, 1, fin);
    fclose(fin);

    for (i = 0; i < 782; i++) {
        bot[i + 782] = ((bot[i] & 0xF0F0) >> 4);
        bot[i] = (bot[i] & 0x0F0F);
    }

    for (i = 0; i < 1564; i++) {
        boob.pixels()[i] += off;
        boob.pixels()[1564 + i] += off;
    }

    boob.copyTo(display::graphics.legacyScreen(), x, y);
}

/**
 * This function is used to display the first frame of the replay in
 * the mission review screen.
 */
void
AbzFrame(char plr, int num, int dx, int dy, int width, int height,
         const char *Type, char mode)
{
    int j = 0;
    std::vector<struct MissionSequenceKey> sSeq;
    char *fname;
    char vname[20];
    mm_file vidfile;

    fname = locate_file("seq.json", FT_DATA);
    DESERIALIZE_JSON_FILE(&sSeq, fname);

    for (j = 0; j < sSeq.size(); j++) {
        if (sSeq.at(j).MissionIdSequence == interimData.tempReplay.at((plr * 100) + num).at(0).seq) {
            break;
        }
    }

    if (j == sSeq.size()) {
        return;
    }

    /* XXX use a generic function */
    snprintf(vname, sizeof(vname), "%s.ogg", sSeq.at(j).video.at(0).c_str());

    INFO2("opening video file `%s'", vname);

    if (mm_open_fp(&vidfile, sOpen(vname, "rb", FT_VIDEO)) <= 0) {
        return;
    }

    if (mm_video_info(&vidfile, NULL, NULL, NULL) <= 0) {
        goto done;
    }

    if (mm_decode_video(&vidfile, display::graphics.videoOverlay()) <= 0) {
        goto done;
    }

    display::graphics.videoRect().x = dx;
    display::graphics.videoRect().y = dy;
    display::graphics.videoRect().w = width;
    display::graphics.videoRect().h = height;

done:
    mm_close(&vidfile);
}
