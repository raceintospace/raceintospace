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

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "prefs.h"
#include "gamedata.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "options.h"
#include "game_main.h"
#include "randomize.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "filesystem.h"

#include <ctype.h>

void DrawPrefs(int where, char a1, char a2);
void HModel(char mode, char tx);
void Levels(char plr, char which, char x);
void BinT(int x, int y, char st);
void PLevels(char side, char wh);
void CLevels(char side, char wh);

void DrawPrefs(int where, char a1, char a2)
{
    int mode = 0;

    FadeOut(2, 10, 0, 0);
    helpText = "i013";
    keyHelpText = "K013";

    boost::shared_ptr<display::PalettizedSurface> prefs_image(Filesystem::readImage("images/preferences.png"));
    // fixme: don't use vhptr to draw this screen
    vhptr->palette().copy_from(prefs_image->palette());
    vhptr->draw(prefs_image, 0, 0);

    display::graphics.screen()->clear();
    prefs_image->exportPalette();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 89, 199);
    ShBox(91, 24, 228, 107);
    IOBox(98, 28, 137, 63);
    IOBox(98, 68, 137, 103);
    IOBox(144, 28, 221, 63);
    /* This draws disabled button around camera */
    /* IOBox(144,68,221,103); */
    InBox(144, 68, 221, 103);

    ShBox(91, 109, 228, 199);
    InBox(95, 113, 224, 195);
    fill_rectangle(96, 114, 223, 194, 0);
    ShBox(230, 24, 319, 199);

    if (where == 2) {
        where = mode = 1;    //modem klugge
    } else if (where == 3) {
        where = mode = 2;    //play-by-mail
    }

    if (where == 0 || where == 2) {
        music_start(M_SOVTYP);
        IOBox(6, 105, 83, 140);
        IOBox(6, 158, 83, 193);
        IOBox(236, 105, 313, 140);
        IOBox(236, 158, 313, 193);
        InBox(6, 52, 83, 87);
        InBox(236, 52, 313, 87);
        fill_rectangle(7, 53, 82, 86, 0);
        fill_rectangle(237, 53, 312, 86, 0);
        OutBox(8, 77, 18, 85);
        OutBox(238, 77, 248, 85);
        //BinT(8,54,0);BinT(238,54,0);  // Old way with buttons
        BinT(8, 54, 1);
        BinT(238, 54, 1);  // No select Buttons
        fill_rectangle(237, 35, 312, 41, 0);
        fill_rectangle(7, 35, 82, 41, 0);
    } else {
        music_start(M_DRUMSM);
        InBox(8, 107, 81, 138);
        InBox(8, 160, 81, 191);
        InBox(238, 107, 311, 138);
        InBox(238, 160, 311, 191);
        InBox(8, 77, 18, 85);
        InBox(238, 77, 248, 85);
        BinT(8, 54, 1);
        BinT(238, 54, 1);
        fill_rectangle(237, 35, 312, 41, 0);
        fill_rectangle(7, 35, 82, 41, 0);
    };

    if (mode == 0) {
        draw_heading(6, 5, "PREFERENCES SELECTIONS", 0, -1);
    } else if (mode == 2) {
        draw_heading(3, 5, "PLAY BY MAIL SELECTIONS", 0, -1);
    } else {
        draw_heading(6, 5, "MODEM GAME SELECTIONS", 0, -1);
    }

    IOBox(243, 3, 316, 19);
    InBox(236, 34, 313, 42);
    InBox(6, 34, 83, 42);
    PLevels(0, Data->Def.Plr1);
    CLevels(0, a1);
    PLevels(1, Data->Def.Plr2);
    CLevels(1, a2);
    Levels(0, Data->Def.Lev1, 1);
    Levels(0, Data->Def.Ast1, 0);
    Levels(1, Data->Def.Lev2, 1);
    Levels(1, Data->Def.Ast2, 0);

    if (where == 0 || where == 2) {
        display::graphics.setForegroundColor(9);
    } else {
        display::graphics.setForegroundColor(34);
    }

    draw_string(23, 30, "PLAYER 1");
    display::graphics.setForegroundColor(34);
    draw_string(253, 30, "PLAYER 2");
    display::graphics.setForegroundColor(5);
    draw_string(23, 49, "COUNTRY");
    draw_string(254, 49, "COUNTRY");
    draw_string(17, 101, "GAME LEVEL");
    draw_string(247, 101, "GAME LEVEL");
    draw_string(249, 148, "COSMONAUT");
    draw_string(250, 155, "SELECTION");
    draw_string(19, 148, "ASTRONAUT");
    draw_string(20, 155, "SELECTION");
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    draw_string(8, 40, &Data->P[ Data->Def.Plr1 ].Name[0]);
    draw_string(238, 40, &Data->P[ Data->Def.Plr2 ].Name[0]);

    // todo: convert to draw commands
    vhptr->copyTo(display::graphics.legacyScreen(), 153 + 34 * (Data->Def.Music), 0, 101, 31, 134, 60);
    vhptr->copyTo(display::graphics.legacyScreen(), 221 + 34 * (Data->Def.Sound), 0, 101, 71, 134, 100);

    vhptr->copyTo(display::graphics.legacyScreen(), 216, 30, 147, 31, 218, 60);
    vhptr->copyTo(display::graphics.legacyScreen(), 72 * (Data->Def.Anim), 90, 147, 71, 218, 100);
    HModel(Data->Def.Input, 1);

    // if (where==0 || where==2)
    FadeIn(2, 10, 0, 0);
    return;
}

void HModel(char mode, char tx)
{
    SimpleHdr table;

    FILE *in;

    in = sOpen("PRFX.BUT", "rb", 0);
    fseek(in, (mode == 0 || mode == 1 || mode == 4)*sizeof_SimpleHdr, SEEK_CUR);
    fread_SimpleHdr(&table, 1, in);
    fseek(in, table.offset, SEEK_SET);
    display::LegacySurface local(127, 80);
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(&p.pal[112 * 3], 96 * 3, 1, in); // Individual Palette
    }
    fread(buffer, table.size, 1, in); // Get Image
    fclose(in);

    vhptr->palette().copy_from(display::graphics.legacyScreen()->palette());

    RLED_img(buffer, local.pixels(), table.size, local.width(), local.height());

    local.filter(0, 112, display::LegacySurface::Any);

    fill_rectangle(96, 114, 223, 194, 0);

    local.copyTo(display::graphics.legacyScreen(), 97, 115);
    display::graphics.setForegroundColor(11);

    if (mode == 2 || mode == 3) {
        draw_string(100, 122, "HISTORICAL MODEL");
    } else if (mode == 0 || mode == 1) {
        draw_string(100, 122, "BASIC MODEL");
    } else if (mode == 4 || mode == 5) {
        draw_string(100, 122, "RANDOM MODEL");
    }

    display::graphics.setForegroundColor(9);

    if (mode == 0 || mode == 2 || mode == 4) {
        draw_string(100, 128, "HISTORICAL ROSTER");
    } else {
        draw_string(100, 128, "CUSTOM ROSTER");
    }

    return;
}


void Levels(char plr, char which, char x)
{
    unsigned char v[2][2] = {{9, 239}, {161, 108}};

    vhptr->copyTo(display::graphics.legacyScreen(), 0 + which * 72, 30 + x * 30, v[0][plr], v[1][x], v[0][plr] + 71, v[1][x] + 29);

    return;
}

void BinT(int x, int y, char st)
{
    char sta[2][2] = {{2, 4}, {4, 2}};

    display::graphics.setForegroundColor(sta[st][0]);
    grMoveTo(0 + x, y + 20);
    grLineTo(0 + x, y + 0);
    grLineTo(72 + x, y + 0);
    grMoveTo(12 + x, y + 21);
    grLineTo(12 + x, y + 30);
    display::graphics.setForegroundColor(sta[st][1]);
    grMoveTo(0 + x, y + 21);
    grLineTo(11 + x, y + 21);
    grMoveTo(12 + x, y + 31);
    grLineTo(73 + x, y + 31);
    grLineTo(73 + x, y + 0);

    return;
}

void PLevels(char side, char wh)
{

    if (side == 0) {
        vhptr->copyTo(display::graphics.legacyScreen(), 0 + wh * 72,     0,   9,  55,  20,  74);
        vhptr->copyTo(display::graphics.legacyScreen(), 0 + wh * 72 + 11,  0,  21,  55,  80,  84);
    } else {
        vhptr->copyTo(display::graphics.legacyScreen(), 0 + wh * 72,     0, 239,  55, 250,  74);
        vhptr->copyTo(display::graphics.legacyScreen(), 0 + wh * 72 + 11,  0, 250,  55, 310,  84);
    }

    return;
}

void CLevels(char side, char wh)
{

    if (side == 0) {
        vhptr->copyTo(display::graphics.legacyScreen(), 144, wh * 7, 9, 78, 17, 84);
    } else {
        vhptr->copyTo(display::graphics.legacyScreen(), 144, wh * 7, 239, 78, 247, 84);
    }

    return;
}


/* Pref Levels:
  Player Select : 0 = USA
          1 = USSR
          2 = USA AI
          3 = USSR AI
          4 = FOREIGN (maybe)
          5 = FOREIGN AI (maybe)

       6 = USA (modem play)
       7 = USSR (modem play)


  Game Level:  0 = Easy
           1 = Medium
           2 = Hard
  Astro Level: 0 = Easy
           1 = Meduim
           2 = Hard
  Input:       0 = Mouse
           1 = Keyboard
           2 = Joystick (maybe)
  Anim Level:  0 = Full
           1 = Partial
           2 = Results Only
  Music Level: 0 = Full
           1 = Partial
           2 = None
  Sound FX:    0 = On
           1 = Off
  */

void Prefs(int where)
{
    int num, hum1 = 0, hum2 = 0;
    FILE *fin;
    char ch, Name[20], ksel = 0;
    int32_t size;

    if (where != 3) {
        if (where == 0) {
            plr[0] = 0;
            plr[1] = 1;
            Data->Def.Plr2 = 1;
            Data->Def.Plr1 = 0;
            hum1 = 0, hum2 = 1;
            Data->Def.Lev1 = Data->Def.Ast1 = Data->Def.Ast2 = 0;
            Data->Def.Lev2 = 2; //start computer level 3
            Data->Def.Input = 0;
            Data->Def.Sound = Data->Def.Music = 1;
            MuteChannel(AV_ALL_CHANNELS, 0);
        }

        if (Data->Def.Plr1 > 1) {
            Data->Def.Plr1 -= 2;
            hum1 = 1;
        }

        if (Data->Def.Plr2 > 1) {
            Data->Def.Plr2 -= 2;
            hum2 = 1;
        }
    }

    /* Data->Def.Sound=Data->Def.Music=1; */
    DrawPrefs(where, hum1, hum2);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if (mousebuttons > 0 || key > 0) { /* Game Play */
            if (((x >= 245 && y >= 5 && x <= 314 && y <= 17) || key == K_ENTER) && !(hum1 == 1 && hum2 == 1)) {
                InBox(245, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                OutBox(245, 5, 314, 17);

                if (!(Data->Def.Input == 2 || Data->Def.Input == 3)) {
                    if (options.feat_eq_new_name && hum1 != 1) {
                        SetEquipName(0);
                    }

                    if (options.feat_eq_new_name && hum2 != 1) {
                        SetEquipName(1);
                    }
                } //Change Name, if basic mode and for human players

                if (Data->Def.Plr1 != Data->Def.Plr2) {
                    if (Data->Def.Plr1 == 1) {
                        int tmp;

                        strcpy(&Name[0], &Data->P[0].Name[0]);
                        strcpy(&Data->P[0].Name[0], &Data->P[1].Name[0]);
                        strcpy(&Data->P[1].Name[0], &Name[0]);
                        tmp = Data->Def.Lev1;
                        Data->Def.Lev1 = Data->Def.Lev2;
                        Data->Def.Lev2 = tmp;
                        tmp = Data->Def.Ast1;
                        Data->Def.Ast1 = Data->Def.Ast2;
                        Data->Def.Ast2 = tmp;
                    }

                    Data->Def.Plr1 += hum1 * 2;
                    Data->Def.Plr2 += hum2 * 2;

                    if (where == 0 || where == 3) {
                        FadeOut(2, 10, 0, 0);
                    }

                    key = 0;

                    if ((where == 0 || where == 3) && (Data->Def.Input == 2 || Data->Def.Input == 3)) {
                        fin = sOpen("HIST.DAT", "rb", 0);
                        fread(&Data->P[0].Probe[PROBE_HW_ORBITAL], 28 * (sizeof(Equipment)), 1, fin);
                        fread(&Data->P[1].Probe[PROBE_HW_ORBITAL], 28 * (sizeof(Equipment)), 1, fin);
                        fclose(fin);
                    }

                    ///Random Equipment
                    if ((where == 0 || where == 3) && (Data->Def.Input == 4 || Data->Def.Input == 5)) {
                        RandomizeEq();
                    }

                    int i, k;

                    for (i = 0; i < NUM_PLAYERS; i++)
                        for (k = 0; k < 7; k++) {
                            Data->P[i].Probe[k].MSF = Data->P[i].Probe[k].MaxRD;
                            Data->P[i].Rocket[k].MSF = Data->P[i].Rocket[k].MaxRD;
                            Data->P[i].Manned[k].MSF = Data->P[i].Manned[k].MaxRD;
                            Data->P[i].Misc[k].MSF = Data->P[i].Misc[k].MaxRD;
                        }

                    if (Data->Def.Input == 0 || Data->Def.Input == 2 || Data->Def.Input == 4) {
                        // Hist Crews
                        fin = sOpen("CREW.DAT", "rb", 0);
                        size = fread(buffer, 1, BUFFER_SIZE, fin);
                        fclose(fin);
                        fin = sOpen("MEN.DAT", "wb", 1);
                        fwrite(buffer, size, 1, fin);
                        fclose(fin);
                    } else if (Data->Def.Input == 1 || Data->Def.Input == 3 || Data->Def.Input == 5) {
                        // User Crews
                        fin = sOpen("USER.DAT", "rb", FT_SAVE);

                        if (!fin) {
                            fin = sOpen("USER.DAT", "rb", FT_DATA);
                        }

                        size = fread(buffer, 1, BUFFER_SIZE, fin);
                        fclose(fin);
                        fin = sOpen("MEN.DAT", "wb", 1);
                        fwrite(buffer, size, 1, fin);
                        fclose(fin);
                    }

                    music_stop();
                    return;
                }
            } else if (key == 'P' && (where == 0 || where == 3)) {
                fill_rectangle(59, 26, 68, 31, 3);
                fill_rectangle(290, 26, 298, 31, 3);

                if (ksel == 0) {
                    ksel = 1;
                    display::graphics.setForegroundColor(9);
                    draw_string(253, 30, "PLAYER 2");
                    display::graphics.setForegroundColor(34);
                    draw_string(23, 30, "PLAYER 1");
                } else {
                    ksel = 0;
                    display::graphics.setForegroundColor(34);
                    draw_string(253, 30, "PLAYER 2");
                    display::graphics.setForegroundColor(9);
                    draw_string(23, 30, "PLAYER 1");
                }
            } else if ((x >= 146 && y >= 30 && x <= 219 && y <= 61 && mousebuttons > 0) || key == 'E') {
                // Edit astronauts has been ripped out

            } else if (((x >= 96 && y >= 114 && x <= 223 && y <= 194 && mousebuttons > 0) || key == K_SPACE) && (where == 3 || where == 0)) { // Hist
                char maxHModels;
                maxHModels = options.feat_random_eq > 0 ? 5 : 3;
                WaitForMouseUp();
                Data->Def.Input++;

                if (Data->Def.Input > maxHModels) {
                    Data->Def.Input = 0;
                }

                HModel(Data->Def.Input, 0);
            } else if ((x >= 146 && y >= 70 && x <= 219 && y <= 101 && mousebuttons > 0) || key == 'A') {
                /* disable this option right now */
            } else if ((x >= 100 && y >= 30 && x <= 135 && y <= 61 && mousebuttons > 0) || key == 'M') {
                InBox(100, 30, 135, 61);
                WaitForMouseUp();
                Data->Def.Music = !Data->Def.Music;
                // SetMusicVolume((Data->Def.Music==1)?100:0);
                music_set_mute(!Data->Def.Music);
                vhptr->copyTo(display::graphics.legacyScreen(), 153 + 34 * (Data->Def.Music), 0, 101, 31, 134, 60);
                OutBox(100, 30, 135, 61);
                /* Music Level */
            } else if ((x >= 100 && y >= 70 && x <= 135 && y <= 101 && mousebuttons > 0) || key == 'S') {
                InBox(100, 70, 135, 101);
                WaitForMouseUp();
                Data->Def.Sound = !Data->Def.Sound;
                MuteChannel(AV_SOUND_CHANNEL, !Data->Def.Sound);
                vhptr->copyTo(display::graphics.legacyScreen(), 221 + 34 * (Data->Def.Sound), 0, 101, 71, 134, 100);
                OutBox(100, 70, 135, 101);
                /* Sound Level */
            }

            else if ((x >= 8 && y >= 77 && x <= 18 && y <= 85 && where == 0 && mousebuttons > 0) ||
                     (where == 0 && ksel == 0 && key == 'H')) {
                InBox(8, 77, 18, 85);
                WaitForMouseUp();
                hum1++;

                if (hum1 > 1) {
                    hum1 = 0;
                }

                CLevels(0, hum1);
                OutBox(8, 77, 18, 85);

                /* P1: Human/Computer */
                //change human to dif 1 and comp to 3
                if (hum1 == 1) {
                    Data->Def.Lev1 = 2;
                } else {
                    Data->Def.Lev1 = 0;
                }

                Levels(0, Data->Def.Lev1, 1);
            } else if ((x >= 8 && y >= 107 && x <= 81 && y <= 138 && (where == 0 || where == 3) && mousebuttons > 0) ||
                       ((where == 3 || where == 0) && ksel == 0 && key == 'G')) {
                InBox(8, 107, 81, 138);
                WaitForMouseUp();
                OutBox(8, 107, 81, 138);
                Data->Def.Lev1++;

                if (Data->Def.Lev1 > 2) {
                    Data->Def.Lev1 = 0;
                }

                Levels(0, Data->Def.Lev1, 1);
                /* P1: Game Level */
            } else if ((x >= 8 && y >= 160 && x <= 81 && y <= 191 && ((where == 0 || where == 3) && mousebuttons > 0)) ||
                       ((where == 3 || where == 0) && ksel == 0 && key == 'L')) {
                InBox(8, 160, 81, 191);
                WaitForMouseUp();
                OutBox(8, 160, 81, 191);
                Data->Def.Ast1++;

                if (Data->Def.Ast1 > 2) {
                    Data->Def.Ast1 = 0;
                }

                Levels(0, Data->Def.Ast1, 0);
                /* P1: Astro Level */
            }

            else if ((x >= 238 && y >= 77 && x <= 248 && y <= 85 && where == 0 && mousebuttons > 0) ||
                     (where == 0 && ksel == 1 && key == 'H')) {
                InBox(238, 77, 248, 85);
                WaitForMouseUp();
                hum2++;

                if (hum2 > 1) {
                    hum2 = 0;
                }

                CLevels(1, hum2);
                OutBox(238, 77, 248, 85);

                /* P2:Human/Computer */
                //change human to dif 1 and comp to 3
                if (hum2 == 1) {
                    Data->Def.Lev2 = 2;
                } else {
                    Data->Def.Lev2 = 0;
                }

                Levels(1, Data->Def.Lev2, 1);
            } else if ((x >= 238 && y >= 107 && x <= 311 && y <= 138 && (where == 0 || where == 3) && mousebuttons > 0) ||
                       ((where == 0 || where == 3) && ksel == 1 && key == 'G')) {
                InBox(238, 107, 311, 138);
                WaitForMouseUp();
                OutBox(238, 107, 311, 138);
                Data->Def.Lev2++;

                if (Data->Def.Lev2 > 2) {
                    Data->Def.Lev2 = 0;
                }

                Levels(1, Data->Def.Lev2, 1);
                /* P2: Game Level */
            } else if ((x >= 238 && y >= 160 && x <= 311 && y <= 191 && (where == 0 || where == 3) && mousebuttons > 0) ||
                       ((where == 0 || where == 3) && ksel == 1 && key == 'L')) {
                InBox(238, 160, 311, 191);
                WaitForMouseUp();
                OutBox(238, 160, 311, 191);
                Data->Def.Ast2++;

                if (Data->Def.Ast2 > 2) {
                    Data->Def.Ast2 = 0;
                }

                Levels(1, Data->Def.Ast2, 0);
                /* P2: Astro Level */
            } else if ((x >= 6 && y >= 34 && x <= 83 && y <= 42 && (where == 3 || where == 0) && mousebuttons > 0) ||
                       ((where == 3 || where == 0) && ksel == 0 && key == 'N')) {
                fill_rectangle(7, 35, 82, 41, 0);

                for (int i = 0; i < 20; i++) {
                    Data->P[0].Name[i] = 0x00;
                }

                num = 0;
                ch = 0;
                display::graphics.setForegroundColor(1);
                grMoveTo(8, 40);
                draw_character(0x14);
                av_sync();

                while (ch != K_ENTER) {
                    ch = getch();

                    if (ch != (ch & 0xff)) {
                        ch = 0x00;
                    }

                    if (ch >= 'a' && ch <= 'z') {
                        ch -= 0x20;
                    }

                    if (ch == 0x08 && num > 0) {
                        Data->P[0].Name[--num] = 0x00;
                    } else if (num < 12 && (isupper(ch) || isdigit(ch) || ch == 0x20)) {
                        Data->P[0].Name[num++] = ch;
                    }

                    fill_rectangle(7, 35, 82, 41, 0);
                    display::graphics.setForegroundColor(1);
                    draw_string(8, 40, &Data->P[0].Name[0]);
                    draw_character(0x14);
                    av_sync();
                }

                Data->P[0].Name[num] = 0x00;
                fill_rectangle(7, 35, 82, 41, 0);
                display::graphics.setForegroundColor(1);
                draw_string(8, 40, &Data->P[0].Name[0]);
                av_sync();
                /* P1: Director Name */
            } else if ((x >= 236 && y >= 34 && x <= 313 && y <= 42 && (where == 3 || where == 0) && mousebuttons > 0) ||
                       ((where == 3 || where == 0) && ksel == 1 && key == 'N')) {
                fill_rectangle(237, 35, 312, 41, 0);

                for (int i = 0; i < 20; i++) {
                    Data->P[1].Name[i] = 0x00;
                }

                num = 0;
                ch = 0;
                display::graphics.setForegroundColor(1);
                grMoveTo(238, 40);
                draw_character(0x14);
                av_sync();

                while (ch != K_ENTER) {
                    ch = getch();

                    if (ch != (ch & 0xff)) {
                        ch = 0x00;
                    }

                    if (ch >= 'a' && ch <= 'z') {
                        ch -= 0x20;
                    }

                    if (ch == 0x08 && num > 0) {
                        Data->P[1].Name[--num] = 0x00;
                    } else if (num < 12 && (isupper(ch) || isdigit(ch) || ch == 0x20)) {
                        Data->P[1].Name[num++] = ch;
                    }

                    fill_rectangle(237, 35, 312, 41, 0);
                    display::graphics.setForegroundColor(1);
                    draw_string(238, 40, &Data->P[1].Name[0]);
                    draw_character(0x14);
                    av_sync();
                }

                Data->P[1].Name[num] = 0x00;
                fill_rectangle(237, 35, 312, 41, 0);
                display::graphics.setForegroundColor(1);
                draw_string(238, 40, &Data->P[1].Name[0]);
                av_sync();
                /* P2: Director Name */
            };
        }
    };
}
