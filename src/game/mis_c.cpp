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

// This file handles missions in progress, particularly mission failures and their results

#include "mis_c.h"

#include <cassert>
#include <vector>

#include "display/graphics.h"
#include "display/surface.h"

#include "gamedata.h"
#include "Buzz_inc.h"
#include "bzanim.h"
#include "draw.h"
#include "mmfile.h"
#include "utils.h"
#include "game_main.h"
#include "mc.h"
#include "mis_m.h"
#include "sdlhelper.h"
#include "newmis.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "ioexception.h"
#include "place.h"

#define FRM_Delay 22

#define NORM_TABLE 397
#define CLIF_TABLE 240
#define SCND_TABLE 486

LOG_DEFAULT_CATEGORY(mission)

struct Infin {
    char Code[9], Qty;
    int16_t List[10];
};

struct OF {
    char Name[8];
    int16_t idx;
};

char SHTS[4];

char STEPnum;
char daysAMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void Tick(char plr);
void Clock(char plr, char clck, char mode, char tm);
void DoPack(char plr, FILE *ffin, char mode, char *cde, char *fName,
            const std::vector<struct Infin> &Mob,
            const std::vector<struct OF> &Mob2);
void InRFBox(int a, int b, int c, int d, int col);
void GuyDisp(int xa, int ya, struct Astros *Guy);
char DrawMoonSelection(char plr, char nauts, const struct MisEval &step);
int ImportInfin(FILE *fin, struct Infin &target);
int ImportOF(FILE *fin, struct OF &target);


/** Finds the video fitting to the current mission step and plays it.
 *
 * The function does handle variations for the videos
 * and also finds the proper babypics to display.
 *
 * \param plr Player structure
 * \param step Missions step ID
 * \param Seq Sequence-Code for the movies (Mev[STEP].Name + XFails.fail)
 * \param mode mode 1 branches to fseq.dat so it's probably failure. However mode 2 is defined by a female 'naut's presence
 */
void PlaySequence(char plr, int step, const char *InSeq, char mode)
{
    //DEBUG1("->PlaySequence()");
    DEBUG4("->PlaySequence(plr, step %d, Seq %s, mode %d)", step, InSeq, mode);
    int keep_going;
    int wlen, i, j, k;
    unsigned int fres, max;
    char lnch = 0, AEPT, BABY, Tst2, Tst3;
    unsigned char sts = 0, fem = 0;
    FILE *fout, *ffin, *nfin;
    struct oGROUP *bSeq, aSeq;
    //    struct oFGROUP *dSeq, cSeq;
    struct Table *F;
    char sName[20], err = 0;
    mm_file vidfile;
    FILE *mmfp;
    float fps;
    int hold_count;
    const int SCRATCH_SIZE = 64000;
    char scratch[SCRATCH_SIZE];
    std::vector<struct Infin> Mob;
    std::vector<struct OF> Mob2;
    char *fname;
    std::string ID;

    // since Seq apparently needs to be mutable, copy the input parameter
    char Seq[128];
    strncpy(Seq, InSeq, sizeof(Seq));

    F = NULL; /* XXX check uninitialized */
    i = j = k = 0; /* XXX check uninitialized */

    SHTS[0] = brandom(10);
    SHTS[1] = brandom(10);
    SHTS[2] = brandom(10);
    SHTS[3] = brandom(10);

    if (fEarly && step != 0) {
        return;    //Specs: unmanned mission cut short
    }

    //Specs: female 'naut kludge
    if (mode == 2) {
        fem = 1;  //Spec: additional search param.
        mode = 0;
    } else {
        fem = 0;
    }

    //Specs: LEM Activities Kludge
    if (Seq[0] == 'h') {
        assert(step > 0);

        if (Mev[step - 1].Name[0] == 'S') {
            Seq[0] = 'Q';
        } else {
            Seq[0] = 'i';
            strncpy(Mev[step].FName, "F034", 4);
        }
    }

    if (Seq[0] == 'Q') {
        assert(step > 0);

        if (Mev[step - 1].Name[0] != 'S') {
            if (mode == 0) {
                Seq[0] = 'i';
            } else if (mode == 1) {
                Seq[0] = 'i';
                strncpy(Mev[step].FName, "F034", 4);
            }
        }

        if ((Seq[1] == 'U' || Seq[1] == 'S') && Seq[2] == 'C') {
            if (Seq[3] == '5') {
                Seq[3] = '6';
            } else if (Seq[3] == '6') {
                Seq[3] = '5';
            }
        }
    }

    //Specs: LM act lunar liftoff failure kludge and failed landing LPL
    if (mode == 1) {
        if (Seq[0] == 'T') {
            if (Seq[3] == '6' || Seq[3] == '5') {
                strncpy(Mev[step].FName, "F019", 4);
            }
        }

        if (Seq[0] == 'Q') {
            if (Seq[3] == '6' || Seq[3] == '5') {
                strncpy(Mev[step].FName, "F216", 4);
            }
        }

        if (Seq[0] == 'S') {
            if (Seq[2] == 'P') {
                strncpy(Mev[step].FName, "F118", 4);
            }
        }

        if (Seq[0] == 'P') {
            //TC changero kludge
            if (Seq[5] == '6') {
                Tst2 = Seq[4];
                Tst3 = Seq[5];
                Seq[4] = Seq[2];
                Seq[5] = Seq[3];
                Seq[2] = Tst2;
                Seq[3] = Tst3;
                strncpy(Mev[step].FName, "F115", 4);
            }
        }
    }

    //Specs: launch sync
    if (Seq[0] == '#') {
        lnch = 1;
    } else {
        lnch = 0;
    }

    if (Seq[0] == 'A' || Seq[0] == 'E' || Seq[0] == 'P' || Seq[0] == 'T' || Seq[0] == '#') {
        AEPT = 1;
    } else {
        AEPT = 0;
    }

    if (mode == 1) {

        /* i: the first element in fSeq belonging to the right step */
        for (i = 0; i < Assets->fSeq.size(); i++) {
            if (strncmp(Assets->fSeq.at(i).MissionStep.c_str(), Mev[step].FName, 4) == 0) {
                break;
            }
        }

        if (i == Assets->fSeq.size()) {
            err = 1;
        }

    }

    if (mode == 0) {
        j = 0;
        ID = Assets->sSeq.at(j).MissionIdSequence;

        while (strncmp(&ID[3], Seq, strlen(&ID[3])) != 0) {
            j++;
            if (j == Assets->sSeq.size()) {
                err = 1;
                break;
            }
            else {
                ID = Assets->sSeq.at(j).MissionIdSequence;
            }
        }

        if (ID[2] - 0x30 == 1) {
            if (fem == 0) {
                j++;
                ID = Assets->sSeq.at(j).MissionIdSequence;
            }
        }
    } else if (err == 0) {
        j = i;
        ID = Assets->fSeq.at(j).MissionIdSequence;

        while (strncmp(&ID[3], Seq, strlen(&ID[3])) != 0) {
            j++;
            if (j == Assets->fSeq.size()) {
                err = 1;
                break;
            }
            else {
                if (strncmp(Assets->fSeq.at(j).MissionStep.c_str(), Mev[step].FName, 4) != 0) {
                    // j is already entering the next MissionStep
                    err = 1;
                    break;
                }
                else {
                    ID = Assets->fSeq.at(j).MissionIdSequence;
                }
            }
        }
    }

    if (err) {
        j = 0;
        ID = "110DEFAULT";
    }

    //::::::::::::::::::::::::::::
    // Specs: Sequence Variation :
    //::::::::::::::::::::::::::::
    if (ID[0] - 0x30 != 1) {
        max = (unsigned)(ID[0] - 0x30);
        j += Mev[step].rnum % max;
        if (mode == 0) {
            ID = Assets->sSeq.at(j).MissionIdSequence;
        } else {
            ID = Assets->fSeq.at(j).MissionIdSequence;
        }
    }

    BABY = 0;

    if (mode == 0) {
        if (j >= 1 && j <= 22) {
            BABY = 1;
        }
    }

    if (mode == 0) {
        interimData.tempReplay.at((plr * 100) + Data->P[plr].PastMissionCount).push_back({false, ID});
    } else {
        interimData.tempReplay.at((plr * 100) + Data->P[plr].PastMissionCount).push_back({true, ID});
    }

    if (AI[plr] == 1) {
        return;
    }

    ffin = open_gamedat("BABYPICX.CDR");

    if (AEPT && !mode) {
        // BABYCLIF.CDR consists of two tables:
        //  * 240 Infin entries, each 40 bytes (7200 bytes)
        //  * 486 OF entries, each 10 bytes (4860 bytes)
        // totaling 12060 bytes.
        if ((nfin = open_gamedat("BABYCLIF.CDR")) == NULL) {
            return;
        }

        Mob.reserve(CLIF_TABLE);

        // Specs: First Table
        for (i = 0; i < CLIF_TABLE; i++) {
            struct Infin entry;
            ImportInfin(nfin, entry);
            Mob.push_back(entry);
        }

        fseek(nfin, CLIF_TABLE * sizeof(struct Infin), SEEK_SET);
        Mob2.reserve(SCND_TABLE);

        // Specs: Second Table
        for (i = 0; i < SCND_TABLE; i++) {
            struct OF entry;
            ImportOF(nfin, entry);
            entry.Name[MIN(sizeof(entry.Name), strlen(entry.Name)) - 3] = '-'; // patch
            Mob2.push_back(entry);
        }

        fclose(nfin);
    } else {
        nfin = open_gamedat("BABYNORM.CDR");
        Mob.reserve(NORM_TABLE);

        for (i = 0; i < NORM_TABLE; i++) {
            struct Infin entry;
            ImportInfin(nfin, entry);
            Mob.push_back(entry);
        }

        fclose(nfin);
    }

    i = 0;

    max = ID[1] - '0';
    INFO2("playing sequence ID `%s'", ID.c_str());

    keep_going = 1;
    k = j;

    while (keep_going && i < (int)max) {
        int aidx=0, sidx=0;
        char seq_name[20];
        char name[20]; /** \todo assumption about seq_filename len */

        if (mode == 0) {
            play_audio(Assets->sSeq.at(k).audio.at(i), mode);
        } else {
            play_audio(Assets->fSeq.at(k).audio.at(i), mode);
        }

        if (mode == 0) {
            strntcpy(seq_name, Assets->sSeq.at(k).video.at(i).c_str(), sizeof(seq_name));
        } else {
            strntcpy(seq_name, Assets->fSeq.at(k).video.at(i).c_str(), sizeof(seq_name));
        }

        snprintf(name, sizeof(name), "%s.ogg", seq_name);
                
        mmfp = sOpen(name, "rb", FT_VIDEO);

        INFO2("opening video file `%s'", name);

        if (mm_open_fp(&vidfile, mmfp) <= 0) {
            break;
        }

        /** \todo do not ignore width/height */
        if (mm_video_info(&vidfile, NULL, NULL, &fps) <= 0) {
            break;
        }

        j = 0;

        hold_count = 0;

        while (keep_going) {
            av_step();

            if (BABY == 0 && !fullscreenMissionPlayback) {
                Tick(plr);
            }

            if (hold_count == 0) {

                /** \todo track decoding time and adjust delays */
                if (mm_decode_video(&vidfile, display::graphics.videoOverlay()) <= 0) {
                    break;
                }

                /* XXX I don't get the fancy "hold" thing so I left it out */

            } else if (hold_count < 8) {
                //Specs: single frame hold
                idle_loop(FRM_Delay);

                if (!BABY && !fullscreenMissionPlayback) {
                    Tick(plr);
                }

                idle_loop(FRM_Delay);

                if (!BABY && !fullscreenMissionPlayback) {
                    Tick(plr);
                }

                idle_loop(FRM_Delay);

                if (!BABY && !fullscreenMissionPlayback) {
                    Tick(plr);
                }

                idle_loop(FRM_Delay);
                hold_count++;
            } else {
                DEBUG1("need to come out of hold");
            }

            display::graphics.videoRect().w = 160;
            display::graphics.videoRect().h = 100;

            if (!fullscreenMissionPlayback) {
                display::graphics.videoRect().x = 80;
                display::graphics.videoRect().y = 3 + plr * 10;
            } else {
                display::graphics.screen()->clear();
                display::graphics.videoRect().x = MAX_X / 4;
                display::graphics.videoRect().y = MAX_Y / 4;
                display::graphics.videoRect().h = MAX_Y / 2;
                display::graphics.videoRect().w = MAX_X / 2;
            }

            /** \todo idle_loop is too inaccurate for this */
            idle_loop_secs(1.0 / fps);

            if (sts < 23) {
                if (BABY == 0 && !fullscreenMissionPlayback) {
                    DoPack(plr, ffin, (AEPT && !mode) ? 1 : 0, Seq,
                           seq_name, Mob, Mob2);
                }

                ++sts;

                if (sts == 23) {
                    sts = 0;
                }

                if (bioskey(0) || grGetMouseButtons()) {
                    av_silence(AV_SOUND_CHANNEL);
                    keep_going = 0;
                }

                if (Data->Def.Anim) {
                    idle_loop(FRM_Delay * 3);
                }

                j++;
            }
        }

        mm_close(&vidfile);

        i++;
    }

    if (!IsChannelMute(AV_SOUND_CHANNEL)) {
        if (lnch == 0) {
            PlayAudio("wh.ogg", 0);
        }

        keep_going = 1;

        while (keep_going) {
            if (AnimSoundCheck()) {
                keep_going = 0;
            }

            if (bioskey(0) || grGetMouseButtons()) {
                av_silence(AV_SOUND_CHANNEL);
                keep_going = 0;
            }

            av_block();

            if (Data->Def.Sound == 1) {
                UpdateAudio();
            }

            if (!BABY && !fullscreenMissionPlayback) {
                Tick(plr);
                gr_sync();
            }
        }
    }

    fclose(ffin);  // Specs: babypicx.cdr
    mm_close(&vidfile);
    display::graphics.videoRect().h = 0;
    display::graphics.videoRect().w = 0;
    DEBUG1("<-PlaySequence()");
}

void Tick(char plr)
{
    static int Sec = 1, Min = 0, Hour = 5, Day = 5;
    int g, change = 0;
    double now;
    static double last;

//: Specs: reset clocks
    if (plr == 2) {
        Sec = 1;
        Min = 0;
        Hour = 5;
        Day = 5;
        return;
    }

    now = get_time();

    if (now - last < .1) {
        return;
    }

    last = now;

    for (g = 3; g > -1; g--) {
        change = 0;

        if (g == 3 && Hour == 7) {
            change = 1;
        } else if (g == 2 && Min == 7) {
            change = 1;
        } else if (g == 1 && Sec == 7) {
            change = 1;
        } else if (g == 0) {
            change = 1;
        }

        if (change == 1) {
            switch (g) {
            case 3:
                Clock(plr, 3, 0, Day);

                if (Day == 7) {
                    Day = 0;
                } else {
                    Day++;
                }

                Clock(plr, 3, 1, Day);

            case 2:
                Clock(plr, 2, 0, Hour);

                if (Hour == 7) {
                    Hour = 0;
                } else {
                    Hour++;
                }

                Clock(plr, 2, 1, Hour);

            case 1:
                Clock(plr, 1, 0, Min);

                if (Min == 7) {
                    Min = 0;
                } else {
                    Min++;
                }

                Clock(plr, 1, 1, Min);

            default:
                Clock(plr, 0, 0, Sec);

                if (Sec == 7) {
                    Sec = 0;
                } else {
                    Sec++;
                }

                Clock(plr, 0, 1, Sec);
                break;
            }
        }
    }
}

void Clock(char plr, char clck, char mode, char tm)
{
    unsigned sx, sy;

    sx = 0; /* XXX check uninitialized */

//: Specs: clock y value
    if (plr == 0) {
        sy = 108;
    } else {
        sy = 121;
    }

//: Specs: color
    if (mode == 0) {
        mode = 3;
    } else {
        mode = 4;
    }

//: Specs: clock x_value
    if (clck == 0) {
        sx = 147;
    } else if (clck == 1) {
        sx = 157;
    } else if (clck == 2) {
        sx = 168;
    } else if (clck == 3) {
        sx = 178;
    }

    switch (tm) {
    case 0:
        display::graphics.legacyScreen()->setPixel(sx, sy - 1, mode);
        display::graphics.legacyScreen()->setPixel(sx, sy - 2, mode);
        break;

    case 1:
        display::graphics.legacyScreen()->setPixel(sx + 1, sy - 1, mode);
        break;

    case 2:
        display::graphics.legacyScreen()->setPixel(sx + 1, sy, mode);
        display::graphics.legacyScreen()->setPixel(sx + 2, sy, mode);
        break;

    case 3:
        display::graphics.legacyScreen()->setPixel(sx + 1, sy + 1, mode);
        break;

    case 4:
        display::graphics.legacyScreen()->setPixel(sx, sy + 1, mode);
        display::graphics.legacyScreen()->setPixel(sx, sy + 2, mode);
        break;

    case 5:
        display::graphics.legacyScreen()->setPixel(sx - 1, sy + 1, mode);
        break;

    case 6:
        display::graphics.legacyScreen()->setPixel(sx - 1, sy, mode);
        display::graphics.legacyScreen()->setPixel(sx - 2, sy, mode);
        break;

    case 7:
        display::graphics.legacyScreen()->setPixel(sx - 1, sy - 1, mode);
        break;
    }
}

void DoPack(char plr, FILE *ffin, char mode, char *cde, char *fName,
            const std::vector<struct Infin> &Mob,
            const std::vector<struct OF> &Mob2)
{
    int x, y, attempt, which, mx2, mx1;
    uint16_t *bot, off = 0;
    int32_t locl;
    static char kk = 0, bub = 0;
    char Val1[12], Val2[12], loc;

    memset(Val1, 0x00, sizeof Val1);
    memset(Val2, 0x00, sizeof Val2);
    strcpy(Val1, cde);

    if (Val1[0] == 'W') {
        Val1[2] = 'P';    // for planetary steps
    }

    if (bub < 2) { //4
        mode = 3;
        loc = kk;
    } else if (bub == 2) { //4
        ++bub;
        loc = kk;
    } else {
        bub = 0;
        SHTS[0]++;
        SHTS[1]++;
        SHTS[2]++;
        SHTS[3]++;
        mx1 = MAX(SHTS[0], SHTS[2]);
        mx2 = MAX(SHTS[1], SHTS[3]);

        if (mx1 > mx2) {
            loc = (SHTS[0] > SHTS[2]) ? 0 : 2 ;
        } else {
            loc = (SHTS[1] > SHTS[3]) ? 1 : 3 ;
        }

        SHTS[loc] = brandom(3);
        kk = loc;
        return;
    }

    x = (loc == 0 || loc == 1) ? 6 : 246;

    y = (loc == 0 || loc == 2) ? 5 + 2 * plr : 57 + plr * 9;

    off = 64 + loc * 16;

    // TODO: Rename variable to indicate purpose.
    display::LegacySurface boob(68, 46);

    bot = (uint16_t *) boob.pixels();

    //:::::::::::::::::::::::::::::::
    //Specs: which holds baby frame :
    //:::::::::::::::::::::::::::::::
    if (mode == 3) {
        which = 580 + bub; //Specs: static frames
        ++bub;
    } else if (mode == 1) {
        attempt = 0;
        which = 0;

        while (attempt < SCND_TABLE && attempt < Mob2.size()) {
            int maxlength = MIN(strlen(Mob2[attempt].Name),
                                sizeof(Mob2[attempt].Name));

            if (xstrncasecmp(fName, Mob2[attempt].Name, maxlength) == 0) {
                break;
            } else {
                attempt++;
            }
        }

        if (attempt >= SCND_TABLE || attempt >= Mob2.size()) {
            which = 415 + brandom(25);
        } else {
            if (Val1[0] != '#') {
                switch (Mob2[attempt].idx) {
                case 0:
                    strcat(Val1, "0\0");
                    break;

                case 1:
                    strcat(Val1, "1\0");
                    break;

                case 2:
                    strcat(Val1, "2\0");
                    break;

                case 3:
                    strcat(Val1, "3\0");
                    break;

                case 4:
                    strcat(Val1, "0\0");
                    break;

                case 5:
                    strcat(Val1, "1\0");
                    break;

                case 6:
                    strcat(Val1, "2\0");
                    break;

                case 7:
                    strcat(Val1, "0\0");
                    break;

                case 8:
                    strcat(Val1, "1\0");
                    break;

                case 9:
                    strcat(Val1, "2\0");
                    break;

                case 10:
                    strcat(Val1, "0\0");
                    break;

                case 11:
                    strcat(Val1, "1\0");
                    break;

                default:
                    which = 415 + brandom(25);
                }
            }

            if (which == 0) {
                attempt = 0;
                assert(sizeof(Val2) >= sizeof(Mob[0].Code));

                while (attempt < CLIF_TABLE && attempt < Mob.size()) {
                    strncpy(Val2, &Mob[attempt].Code[0],
                            sizeof(Mob[attempt].Code));
                    int maxlength = MIN(strlen(Val1), sizeof(Val1));

                    if (xstrncasecmp(Val1, Val2, maxlength) == 0) {
                        break;
                    } else {
                        attempt++;
                    }
                }

                if (attempt >= CLIF_TABLE || attempt >= Mob.size()) {
                    which = 415 + brandom(25);
                } else {
                    which = brandom(Mob[attempt].Qty);

                    if (which >= 10) {
                        which = Mob[attempt].List[which % 10];
                    } else {
                        which = Mob[attempt].List[which];
                    }
                }
            }
        }
    } else {
        attempt = 0;
        assert(sizeof(Val2) >= sizeof(Mob[0].Code));

        while (attempt < NORM_TABLE && attempt < Mob.size()) {
            strncpy(Val2, &Mob[attempt].Code[0],
                    sizeof(Mob[attempt].Code));
            int maxlength = MIN(strlen(Val2), sizeof(Val2));

            if (strncmp(Val1, Val2, strlen(Val2)) == 0) {
                break;
            } else {
                attempt++;
            }
        }

        if (attempt >= NORM_TABLE || attempt >= Mob.size()) {
            which = 415 + brandom(25);
        } else {
            which = brandom(Mob[attempt].Qty);

            if (which >= 10) {
                which = Mob[attempt].List[which % 10];
            } else {

                which = Mob[attempt].List[which];
            }
        }
    }

    //Specs: which holds baby num
    locl = (int32_t) 1612 * which;

    if (which < 580) {
        display::AutoPal p(display::graphics.legacyScreen());
        memset(&p.pal[off * 3], 0x00, 48);
    }

    if (loc != 0 && which < 580) {
        VBlank();
    }

    fseek(ffin, (int32_t)locl, SEEK_SET);
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(&p.pal[off * 3], 48, 1, ffin);
    }
    fread(boob.pixels(), 1564, 1, ffin);

    for (int i = 0; i < 782; i++) {
        bot[i + 782] = ((bot[i] & 0xF0F0) >> 4);
        bot[i] = (bot[i] & 0x0F0F);
    }

    for (int i = 0; i < 1564; i++) {
        boob.pixels()[i] += off;
        boob.pixels()[1564 + i] += off;
    }

    VBlank();

    boob.copyTo(display::graphics.legacyScreen(), x, y);

    VBlank();
}



void InRFBox(int a, int b, int c, int d, int col)
{
    InBox(a, b, c, d);
    fill_rectangle(a + 1, b + 1, c - 1, d - 1, col);
    return;
}

void
GuyDisp(int xa, int ya, struct Astros *Guy)
{
    display::graphics.setForegroundColor(1);
    assert(Guy != NULL);

    if (Guy->Sex == 1) {
        display::graphics.setForegroundColor(6);    // Display female 'nauts in navy blue, not white  -Leon
    }

    draw_string(xa, ya, Guy->Name);
    draw_string(0, 0, ": ");

    switch (Guy->Status) {
    case AST_ST_DEAD:
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "DEAD");
        break;

    case AST_ST_RETIRED:
        display::graphics.setForegroundColor(12);
        draw_string(0, 0, "INJ");   // TODO: Why INJ not RET?
        break;

    case AST_ST_INJURED:
        display::graphics.setForegroundColor(12);
        draw_string(0, 0, "INJ");
        break;

    default:
        display::graphics.setForegroundColor(13);
        draw_string(0, 0, "OK");
        break;
    }

    return;
}


/**
 * Interface for the Mission Step Failure report.
 *
 * \param plr    0 for the USA, 1 for the USSR.
 * \param prelim
 * \param text   description of the problem encountered.
 * \return  0 to continue the mission, 1 to scrub.
 */
char FailureMode(char plr, int prelim, char *text)
{
    int i, j, k;
    FILE *fin;
    double last_secs;
    Equipment *e;
    display::LegacySurface saveScreen(display::graphics.screen()->width(), display::graphics.screen()->height());

    FadeOut(2, 10, 0, 0);

    // this destroys what's in the current page frames
    saveScreen.copyFrom(display::graphics.legacyScreen(), 0, 0, display::graphics.screen()->width() - 1, display::graphics.screen()->height() - 1);

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    IOBox(243, 3, 316, 19);
    InBox(3, 3, 30, 19);
    draw_small_flag(plr, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    ShBox(0, 24, 319, 199);

    InRFBox(4, 27, 153, 58, 0);

    display::graphics.setForegroundColor(1);
    MisStep(9, 34, Mev[STEP].loc);
    e = GetEquipment(Mev[STEP]);
    draw_string(9, 41, "MISSION STEP: ");
    draw_number(0, 0, STEP);
    draw_string(9, 48, e->Name);
    draw_string(0, 0, " CHECK");
    draw_number(9, 55, Mev[STEP].dice);
    draw_string(0, 0, " VS. ");
    draw_number(0, 0, StepSafety(Mev[STEP]));
    // draw_string(0, 0, "%");

    draw_heading(45, 5, "STEP FAILURE", 0, -1);

//  InRFBox(4,61,153,109,0); // Image, Small Left Side

    // Status/Scrub
    InRFBox(4, 112, 153, 128, 0);

    // Display Result of Mission
    if (prelim == 3) {
        ShBox(6, 114, 151, 126);
        display::graphics.setForegroundColor(9);
        draw_string(15, 122, "RECOMMEND MISSION ");
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, "S");
        display::graphics.setForegroundColor(9);
        draw_string(0, 0, "CRUB");
        draw_string(7, 108, "(TEMP. PENALTY TO HARDWARE)");
    } else {
        display::graphics.setForegroundColor(9);

        switch (prelim) {
        case 0:
            display::graphics.setForegroundColor(15);
            draw_string(30, 122, "ALL SYSTEMS ARE GO");
            break;

        case 1:
            draw_string(20, 122, "FAILURE: USE ALTERNATE");
            break;

        case 2:
            draw_string(10, 122, "FURTHER PROBLEMS: RECHECK");
            break;

        case 4:
            draw_string(19, 122, "CREW STRANDED IN SPACE");
            break;

        case 5:
            draw_string(39, 122, "MISSION FAILURE");
            break;

        case 6:
            display::graphics.setForegroundColor(15);
            draw_string(33, 121, "MISSION SUCCESS");
            break;

        case 7:
            draw_string(45, 122, "STEP FAILURE");
            break;

        case 8:
            draw_string(23, 122, "CATASTROPHIC FAILURE");
            break;

        case 9:
            draw_string(44, 122, "CREW INJURIES");
            break;
        }
    }


    // Flight Crew Info
    InRFBox(4, 131, 315, 151, 0); // Astro List/Crew

    display::graphics.setForegroundColor(12);

    draw_string(15, 139, "CREW");

    draw_string(9, 146, "STATUS");


    if (MANNED[Mev[STEP].pad] > 0) {
        GuyDisp(49, 139, MA[Mev[STEP].pad][0].A);
    }

    if (MANNED[Mev[STEP].pad] > 1) {
        GuyDisp(49, 146, MA[Mev[STEP].pad][1].A);
    }

    if (MANNED[Mev[STEP].pad] > 2) {
        GuyDisp(182, 139, MA[Mev[STEP].pad][2].A);
    }

    if (MANNED[Mev[STEP].pad] > 3) {
        GuyDisp(182, 146, MA[Mev[STEP].pad][3].A);
    }

    if (MANNED[Mev[STEP].pad] == 0) {
        if (((e->ID[1] == 0x35 || e->ID[1] == 0x36) && STEP > 5)) {  // if LEMS
            GuyDisp(49, 138, MA[1][LM[1]].A);

            if (EVA[1] != LM[1]) {
                GuyDisp(49, 146, MA[1][EVA[1]].A);
            }
        } else if (strncmp(e->ID, "M2", 2) == 0) {
            GuyDisp(49, 138, MA[other(Mev[STEP].pad)][0].A);
            GuyDisp(49, 146, MA[other(Mev[STEP].pad)][1].A);
            GuyDisp(182, 138, MA[other(Mev[STEP].pad)][2].A);
        } else if (strncmp(e->ID, "M3", 2) == 0) {  // EVA
            GuyDisp(49, 138, MA[1][EVA[1]].A);
        } else {
            display::graphics.setForegroundColor(1);
            draw_string(49, 139, "UNMANNED");
        }
    }

    display::graphics.setForegroundColor(11);  // Specialist

    if (MANNED[Mev[STEP].pad] > 0) {
        display::graphics.setForegroundColor(1);
        int xloc = (Mev[STEP].dice > 99) ? 62 : 57;

        switch (Mev[STEP].ast) {
        case 0:
            draw_string(xloc, 55, "(");
            draw_string(0, 0, MA[Mev[STEP].pad][0].A->Name);
            draw_string(0, 0, ")");
            break;

        case 1:
            draw_string(xloc, 55, "(");
            draw_string(0, 0, MA[Mev[STEP].pad][1].A->Name);
            draw_string(0, 0, ")");
            break;

        case 2:
            draw_string(xloc, 55, "(");
            draw_string(0, 0, MA[Mev[STEP].pad][2].A->Name);
            draw_string(0, 0, ")");
            break;

        case 3:
            draw_string(xloc, 55, "(");
            draw_string(0, 0, MA[Mev[STEP].pad][3].A->Name);
            draw_string(0, 0, ")");
            break;
        }
    }

    // Display Failure Text
    InRFBox(4, 154, 315, 196, 0);

    display::graphics.setForegroundColor(11);

    j = 0;

    k = 163;

    grMoveTo(12, k);

    for (i = 0; i < 200; i++) {
        if (j > 40 && text[i] == ' ') {
            k += 7;
            j = 0;
            grMoveTo(12, k);
        } else {
            draw_character(text[i]);
        }

        j++;

        if (text[i] == '\0') {
            break;
        }
    }


    // Failure Diagram
    InRFBox(162, 28, 312, 42, 10);

    display::graphics.setForegroundColor(11);

    draw_string(194, 37, "EQUIPMENT DETAIL");

    InRFBox(162, 46, 312, 127, 0); // Image is 188,49

    // Place Image Here
    // Build Name
    memset(Name, 0x00, sizeof Name);

    if (plr == 0) {
        strcat(Name, "US");
    } else {
        strcat(Name, "SV");
    }

    strncat(Name, e->ID, 2);

    if (Mev[STEP].Class == Mission_PhotoRecon) {
        strcpy(&Name[0], "XCAM\0");
    }

    strcat(Name, ".BZ\0");

    BZAnimation::Ptr modelAnim;

    try {
        modelAnim = BZAnimation::load("LIFTOFF.ABZ", Name, 188, 47);
    } catch (IOException &err) {
        ERROR4("Unable to load animation %s in file %s: %s",
               Name, "LIFTOFF.ABZ", err.what());
    }

    if (modelAnim) {
        last_secs = get_time();
        modelAnim->advance();
    }

    FadeIn(2, 10, 0, 0);


    WaitForMouseUp();
    key = 0;

    while (bioskey(1)) {
        bioskey(0);
    }

    while (1) {
        if (modelAnim && get_time() - last_secs > .55) {
            last_secs = get_time();
            modelAnim->advance();
        }

        GetMouse();

        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();
            OutBox(245, 5, 314, 17);
            delay(10);
            FadeOut(2, 10, 0, 0);
            //  DrawControl(plr);

            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;
            return 0;  /* Continue */
        }

        if ((x >= 6 && y >= 114 && x <= 151 && y <= 126 && prelim == 3 && mousebuttons > 0) || (prelim == 7 && key == 'S')) {
            InBox(6, 114, 151, 126);
            WaitForMouseUp();
            OutBox(6, 114, 151, 126);
            delay(10);
            FadeOut(2, 10, 0, 0);
            //   DrawControl(plr);

            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;

            return Help("i165") >= 0;
        }
    }
}


void FirstManOnMoon(char plr, char isAI, char misNum,
                    const struct MisEval &step)
{
    int nautsOnMoon = 0;
    Equipment *e = GetEquipment(step);

    dayOnMoon = brandom(daysAMonth[Data->P[plr].Mission[step.pad].Month]) + 1;

    if (misNum == Mission_Soyuz_LL && plr == 1) {
        nautsOnMoon = 3;
    }

    //Direct Ascent
    if (strcmp(e->Name, Data->P[plr].Manned[MANNED_HW_FOUR_MAN_CAPSULE].Name) == 0) {
        nautsOnMoon = 4;
    }

    //2 men LL
    if (strcmp(e->Name, Data->P[plr].Manned[MANNED_HW_TWO_MAN_MODULE].Name) == 0) {
        nautsOnMoon = 2;
    }

    //1 man LL
    if (strcmp(e->Name, Data->P[plr].Manned[MANNED_HW_ONE_MAN_MODULE].Name) == 0) {
        nautsOnMoon = 1;
    }


    if (nautsOnMoon == 1) {
        manOnMoon = 2;
        return;
    }

    if (!AI[plr]) {
        manOnMoon = DrawMoonSelection(plr, nautsOnMoon, step);
    } else {
        manOnMoon = brandom(nautsOnMoon) + 1;
    }

    EVA[0] = EVA[1] = manOnMoon - 1;

    return;
}


// TODO: Move drawing the interface into its own function distinct
// from the main control loop.
char DrawMoonSelection(char plr, char nauts, const struct MisEval &step)
{
    // TODO: Using MX as a copy of MA to avoid modifying it is nice,
    // but it never gets modified and a global var (MA) is still being
    // directly accessed.
    struct MisAst MX[2][4];
    FILE *fin;
    double last_secs;
    Equipment *e;
    display::LegacySurface saveScreen(display::graphics.screen()->width(), display::graphics.screen()->height());

    memcpy(MX, MA, 8 * sizeof(struct MisAst));
    char cPad;

    if (MX[step.pad][0].A != NULL) {
        cPad = step.pad;
    } else if (MX[other(step.pad)][0].A != NULL) {
        cPad = other(step.pad);
    } else {
        return 2;
    }

    FadeOut(2, 10, 0, 0);
    saveScreen.copyFrom(display::graphics.legacyScreen(), 0, 0, display::graphics.screen()->width() - 1, display::graphics.screen()->height() - 1);

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    draw_small_flag(plr, 4, 4);
    ShBox(0, 24, 319, 199);
    draw_heading(42, 5, "FIRST LUNAR EVA", 0, -1);

    InRFBox(162, 28, 312, 42, 10);
    display::graphics.setForegroundColor(11);
    draw_string(194, 37, "EQUIPMENT DETAIL");

    InRFBox(162, 46, 312, 127, 0); // Image is 188,49

    // Place Image Here
    // Build Name
    memset(Name, 0x00, sizeof Name);

    if (plr == 0) {
        strcat(Name, "US");
    } else {
        strcat(Name, "SV");
    }

    // TODO: Simpler to remove the intermediary var 'e'.
    e = GetEquipment(step);
    strncat(Name, e->ID, 2);

    if (step.Class == Mission_PhotoRecon) {
        strcpy(&Name[0], "XCAM\0");
    }

    strcat(Name, ".BZ\0");

    BZAnimation::Ptr moonAnim;

    try {
        moonAnim = BZAnimation::load("LIFTOFF.ABZ", Name, 188, 47);
    } catch (IOException &err) {
        ERROR4("Unable to load animation %s in file %s: %s",
               Name, "LIFTOFF.ABZ", err.what());
    }

    if (moonAnim) {
        last_secs = get_time();
        moonAnim->advance();
    }

    InRFBox(25, 31, 135, 45, 10);
    display::graphics.setForegroundColor(11);
    // TODO: Use TextDisplayLength instead of strlen * 3 for better
    // centering?
    draw_string(83 - strlen(Data->P[plr].Mission[step.pad].Name) * 3, 40,
                Data->P[plr].Mission[step.pad].Name);
    InRFBox(162, 161, 313, 175, 10);
    display::graphics.setForegroundColor(11);
    draw_number(198, 170, dayOnMoon);
    draw_string(0, 0, " ");
    draw_string(0, 0, Month[Data->P[plr].Mission[step.pad].Month]);
    draw_string(0, 0, "19");
    draw_number(0, 0, Data->Year);

    InRFBox(25, 51, 135, 85, 10);
    display::graphics.setForegroundColor(11);
    draw_string(35, 60, "Who should be the");
    draw_string(31, 70, plr == 0 ?
                "first astronaut to" :
                "first cosmonaut to");
    draw_string(35, 80, "walk on the moon?");

    int i;
    // TODO: This does nothing? If so, remove.
    char str;

    for (i = 0; i < nauts; i++) {
        IOBox(25, 100 + i * 25, 135, 115 + i * 25);
        display::graphics.setForegroundColor(12);
        GuyDisp(71 - TextDisplayLength(MX[cPad][i].A->Name) / 2, 109 + i * 25, MX[cPad][i].A);
    }

    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();
    key = 0;

    while (1) {
        if (moonAnim && get_time() - last_secs > .55) {
            last_secs = get_time();
            moonAnim->advance();
        }

        GetMouse();

        if (MX[cPad][0].A->Status != AST_ST_DEAD &&
            (key == '1' || (x >= 25 && x <= 135 && y >= 100 && y <= 115 && mousebuttons > 0))) {
            InBox(27, 102, 133, 113);
            WaitForMouseUp();
            OutBox(27, 102, 133, 113);
            delay(10);
            FadeOut(2, 10, 0, 0);
            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;
            return 1;
        }

        if (MX[cPad][1].A->Status != AST_ST_DEAD &&
            (key == '2' || (x >= 25 && x <= 135 && y >= 125 && y <= 140 && mousebuttons > 0))) {
            InBox(27, 127, 133, 138);
            WaitForMouseUp();
            OutBox(27, 127, 133, 138);
            delay(10);
            FadeOut(2, 10, 0, 0);
            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;
            return 2;
        }

        if (nauts >= 3 && MX[cPad][2].A->Status != AST_ST_DEAD &&
            (key == '3' || (x >= 25 && x <= 135 && y >= 150 && y <= 165 && mousebuttons > 0))) {
            InBox(27, 152, 133, 163);
            WaitForMouseUp();
            OutBox(27, 152, 133, 163);
            delay(10);
            FadeOut(2, 10, 0, 0);
            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;
            return 3;
        }

        if (nauts >= 4 && MX[cPad][3].A->Status != AST_ST_DEAD &&
            (key == '4' || (x >= 25 && x <= 135 && y >= 175 && y <= 190 && mousebuttons > 0))) {
            InBox(27, 177, 133, 188);
            WaitForMouseUp();
            OutBox(27, 177, 133, 188);
            delay(10);
            FadeOut(2, 10, 0, 0);
            display::graphics.legacyScreen()->palette().copy_from(saveScreen.palette());
            display::graphics.screen()->draw(saveScreen, 0, 0);

            FadeIn(2, 10, 0, 0);
            key = 0;
            return 4;
        }
    }
}


/**
 * Read an Infin struct stored in a file as raw data.
 *
 * \param fin  Pointer to a FILE object that specifies an input stream.
 * \param target  The destination for the read data.
 */
int ImportInfin(FILE *fin, struct Infin &target)
{
    // struct Infin {
    //     char Code[9], Qty;
    //     int16_t List[10];
    // };
    fread(&target.Code[0], sizeof(target.Code), 1, fin);
    fread(&target.Qty, sizeof(target.Qty), 1, fin);
    fread(&target.List[0], sizeof(target.List), 1, fin);

    for (int i = 0; i < 10; i++) {
        Swap16bit(target.List[i]);
    }

    return 0;
}


/**
 * Read in an OF struct stored in a file as raw data.
 *
 * \param fin  Pointer to a FILE object that specifies an input stream.
 * \param target  The destination for the read data
 * \return
 */
int ImportOF(FILE *fin, struct OF &target)
{
    // struct OF {
    //     char Name[8];
    //     int16_t idx;
    // };
    fread(&target.Name[0], sizeof(target.Name), 1, fin);
    fread(&target.idx, sizeof(target.idx), 1, fin);
    Swap16bit(target.idx);
    return 0;
}

