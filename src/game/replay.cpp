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
#include "gamedata.h"
#include "Buzz_inc.h"
#include "externs.h"
#include <assert.h>
#include "mmfile.h"
#include "av.h"
#include "logging.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

/** find and fill REPLAY structure and return 0, or -1 if failed.
 * if grp != NULL and oGROUP at offset rep->off[0] is found, then fill grp too
 *
 * \return -1 on bad sequence
 * \return  0 in all other cases
 */
static int
find_replay(REPLAY *rep, struct oGROUP *grp, char player, int num,
            const char *type)
{
    FILE *fseq = NULL;
    struct oGROUP group;
    size_t offset = 0;
    int retval = 0;

    assert(rep);

    /** \note uses SEQ.DAT */
    fseq = sOpen("SEQ.DAT", "rb", 0);

    if (!fseq) {
        return -1;
    }

    if (strncmp("OOOO", type, 4) == 0) {
        FILE *f = sOpen("REPLAY.DAT", "rb", 1);

        offset = (player * 100) + num;
        fseek(f, offset * (sizeof * rep), SEEK_SET);
        /** \todo Uses fread() here - should be fread_REPLAY(&Rep, 1, f); */
        fread(rep, (sizeof * rep), 1, f);
        fclose(f);

        if (grp && fseek(fseq, sizeof_oGROUP * rep->Off[0], SEEK_SET) == 0) {
            fread_oGROUP(grp, 1, fseq);
        }
    } else {
        int j = 0;

        while (fread_oGROUP(&group, 1, fseq)) {
            if (strncmp(group.ID, "XXXX", 4) == 0) {
                /* bad sequence? */
                retval = -1;
                goto done;
            }

            if (strcmp(&group.ID[3], type) == 0) {
                break;
            }

            j++;
        }

        rep->Qty = 1;
        rep->Off[0] = j;

        if (grp) {
            memcpy(grp, &group, sizeof(group));
        }
    }

done:

    if (fseq) {
        fclose(fseq);
    }

    return retval;
}

/**
 *
 * \returns nothing if find_replay() fails
 * \returns nothing if it can't open the [f]seq.dat file
 */
void
Replay(char plr, int num, int dx, int dy, int width, int height, const char *Type)
{
    int keep_going;
    int i, kk, mode, max;
    FILE *seqf, *fseqf;
    int32_t offset;
    struct oGROUP group;
    struct oFGROUP fgroup;
    struct Table table;
    REPLAY Rep;

    mm_file vidfile;
    float fps;

    if (find_replay(&Rep, NULL, plr, num, Type) < 0) {
        return;
    }

    /** \note uses SEQ.DAT
     *  \note uses FSEQ.DAT
     */
    seqf = sOpen("SEQ.DAT", "rb", 0);
		if (!seqf) {
			return;
		}

    fseqf = sOpen("FSEQ.DAT", "rb", 0);
		if (!fseqf) {
			fclose(seqf);
			return;
		}

    WaitForMouseUp();

    DEBUG2("video sequence: %d segments", Rep.Qty);

    for (kk = 0; kk < Rep.Qty; kk++) {
        DEBUG3("playing segment %d: %d", kk, Rep.Off[kk]);

        if (Rep.Off[kk] < 1000) {  //Specs: success seq
            fseek(seqf, Rep.Off[kk] * sizeof_oGROUP, SEEK_SET);
            fread_oGROUP(&group, 1, seqf);
            max = group.ID[1] - '0';
            mode = 0;
        } else {
            //Specs: failure seq
            int j = 0;
            // MAX 50 Tables
            i = Rep.Off[kk] / 1000;
            j = Rep.Off[kk] % 1000;

            if (i == 0 || i == 50) {
                goto done;
            }

            i--;                   //Specs: offset index klugge
            fseek(fseqf, i * sizeof_Table, SEEK_SET);
            fread_Table(&table, 1, fseqf);
            offset = table.foffset;
            fseek(fseqf, offset + j * sizeof_oFGROUP, SEEK_SET);
            fread_oFGROUP(&fgroup, 1, fseqf);
            mode = 1;
            max = fgroup.ID[1] - '0';
        };

        i = 0;

        keep_going = 1;

        //  update_map = 0;
        while (keep_going && i < max) {
            int frm_idx;
            char *seq_fname = NULL;
            char fname[20];

            if (mode == 1) {       /* failure */
                frm_idx = fgroup.oLIST[i].aIdx;
            } else {
                frm_idx = group.oLIST[i].aIdx;
            }

            /* here we should create YUV Overlay, but we can't use it on
             * pallettized surface, so we use a global Overlay initialized in
             * sdl.c. */
            seq_fname = seq_filename(frm_idx, mode);

            if (!seq_fname) {
                seq_fname = "(unknown)";
            }

            /** \todo assumption on file extension */
            snprintf(fname, sizeof(fname), "%s.ogg", seq_fname);

            INFO2("opening video file `%s'", fname);

            if (mm_open_fp(&vidfile, sOpen(fname, "rb", FT_VIDEO)) <= 0) {
                goto done;
            }

            /** \todo do not ignore width/height */
            if (mm_video_info(&vidfile, NULL, NULL, &fps) <= 0) {
                goto done;
            }

            while (keep_going) {
                video_rect.x = dx;
                video_rect.y = dy;
                video_rect.w = width;
                video_rect.h = height;

                screen_dirty = 1;

                /** \todo track decoding time and adjust delays */
                if (mm_decode_video(&vidfile, video_overlay) <= 0) {
                    break;
                }

                if (bioskey(0) || grGetMouseButtons()) {
                    keep_going = 0;
                }

                /** \todo idle_loop is too inaccurate for this */
                idle_loop_secs(1.0 / fps);
            }

            mm_close(&vidfile);
            i++;
        }
    }

done:
    mm_close(&vidfile);
    video_rect.w = 0;
    video_rect.h = 0;
    fclose(fseqf);
    fclose(seqf);
    return;
}

void
DispBaby(int x, int y, int loc, char neww)
{
    int i;
    FILE *fin;
    GXHEADER boob;
    uint16_t *bot, off = 0;
    int32_t locl;

    off = 224;

    GV(&boob, 68, 46);
    bot = (uint16_t *) boob.vptr;

    fin = sOpen("BABYPICX.CDR", "rb", 0);
    locl = (int32_t) 1612 * loc; // First Image

    fseek(fin, locl, SEEK_SET);

    for (i = 0; i < 48; i++) {
        pal[off * 3 + i] = 0;
    }

    fread(&pal[off * 3], 48, 1, fin);
    fread(boob.vptr, 1564, 1, fin);
    fclose(fin);

    for (i = 0; i < 782; i++) {
        bot[i + 782] = ((bot[i] & 0xF0F0) >> 4);
        bot[i] = (bot[i] & 0x0F0F);
    }

    for (i = 0; i < 1564; i++) {
        boob.vptr[i] += off;
        boob.vptr[1564 + i] += off;
    }

    gxPutImage(&boob, gxSET, x, y, 0);

    DV(&boob);

    return;
}

void
AbzFrame(char plr, int num, int dx, int dy, int width, int height,
         char *Type, char mode)
{
    int idx = 0;
    struct oGROUP grp;
    REPLAY Rep;

    /* force mode to zero */
    mode = 0;

    char fname[100];
    mm_file vidfile;

    memset(&grp, 0, sizeof grp);

    if (find_replay(&Rep, &grp, plr, num, Type) < 0) {
        return;
    }

    idx = grp.oLIST[0].aIdx;

    /* XXX use a generic function */
    snprintf(fname, sizeof(fname), "%s.ogg", seq_filename(idx, mode));

    INFO2("opening video file `%s'", fname);

    if (mm_open_fp(&vidfile, sOpen(fname, "rb", FT_VIDEO)) <= 0) {
        return;
    }

    if (mm_video_info(&vidfile, NULL, NULL, NULL) <= 0) {
        goto done;
    }

    if (mm_decode_video(&vidfile, video_overlay) <= 0) {
        goto done;
    }

    video_rect.x = dx;
    video_rect.y = dy;
    video_rect.w = width;
    video_rect.h = height;

done:
    mm_close(&vidfile);
}
