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

#include <assert.h>

#include "display/image.h"
#include "display/graphics.h"
#include "display/surface.h"

#include "place.h"
#include "gamedata.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "utils.h"
#include "game_main.h"
#include "museum.h"
#include "port.h"
#include "replay.h"
#include "mc.h"
#include "mis_c.h"
#include "endgame.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"

void BCDraw(int y);
void DispHelp(char top, char bot, char *txt);
void writePrestigeFirst(char index);


void BCDraw(int y)
{
    ShBox(23, y, 54, 20 + y); //ShBox(56,y,296,20+y);
    return;
}

int MainMenuChoice()
{
    struct {
        const char *label;
        int y;
        const char *hotkeys;
    } const menu_options[] = {
        { "NEW GAME", 9, "N" },
        { "OLD GAME", 36, "O" },
        { "CREDITS", 63, "C" },
        { "EXIT", 90, "EXQ" }
    };
    const int menu_option_count = sizeof(menu_options) / sizeof(menu_options[0]);

    int selected_option = -1;

    {
        FILE *fp = sOpen("main_menu.png", "rb", FT_IMAGE);
        display::Image image(fp);
        fclose(fp);

        image.export_to_legacy_palette();
        display::graphics.screen()->draw(&image, 0, 0);
    }

    for (int i = 0; i < menu_option_count; i++) {
        ShBox(21, menu_options[i].y, 180, menu_options[i].y + 25);
        draw_heading(34, menu_options[i].y + 6, menu_options[i].label, 1, 0);
    }

    FadeIn(2, display::graphics.palette(), 30, 0, 0);
    WaitForMouseUp();

    while (selected_option == -1) {
        GetMouse();

        if (key) {
            // Check for hotkeys
            for (int i = 0; i < menu_option_count; i++) {
                // Iterate over each hotkey
                for (const char *hotkey = menu_options[i].hotkeys; *hotkey; hotkey++) {
                    if (*hotkey == key) {
                        // Match!
                        selected_option = i;
                        break;
                    }
                }
            }
        }

        if (mousebuttons) {
            for (int i = 0; i < menu_option_count; i++) {
                if (x >= 21 && x <= 180 && y >= menu_options[i].y && y <= menu_options[i].y + 27) {
                    selected_option = i;
                    break;
                }
            }
        }
    }

    // Draw a highlighted box
    InBox(21, menu_options[selected_option].y, 180, menu_options[selected_option].y + 25);

    // Let it sink in
    delay(50);

    // Pass back the selected option
    key = 0;
    return selected_option;
}

int BChoice(char plr, char qty, char *Name, char *Imx) // Name[][22]
{
    int i, j, starty = 100;
    FILE *fin = sOpen("PORTBUT.BUT", "rb", 0);

    //FadeOut(2,pal,10,0,0);

    display::Surface local(30, 19);

    starty -= (qty * 23 / 2);

    /* hard-coded magic numbers, yuck */
    for (i = 0; i < qty; i++) {
        BCDraw(starty + 23 * i);
        draw_heading(60, starty + 4 + 23 * i, &Name[i * 22], 1, 0);
        fseek(fin, Imx[i] * 570, SEEK_SET);
        fread(local.pixels(), 570, 1, fin);
        local.copyTo(display::graphics.screen(), 24, starty + 1 + 23 * i);
    }

    fclose(fin);

    // FadeIn(2,pal,10,0,0);
    WaitForMouseUp();
    j = -1;

    while (j == -1) {
        av_block();
        GetMse(plr, 0);

        for (i = 0; i < qty; i++) // keyboard stuff
            if ((char)key == Name[i * 22]) {

                InBox(23, starty + 23 * i, 54, starty + 20 + 23 * i);

                delay(50);
                j = i + 1;
                key = 0;
            }

        if (mousebuttons != 0) {
            for (i = 0; i < qty; i++)
                if ((x >= 23 && x <= 54 && y >= starty + 23 * i && y <= starty + 20 + 23 * i) ||
                    (x > 56 && y > starty + 23 * i && x < 296 && y < starty + 20 + 23 * i)) {

                    InBox(23, starty + 23 * i, 54, starty + 20 + 23 * i);

                    delay(50);
                    j = i + 1;
                }
        }

    }

    return j;
}

void PatchMe(char plr, int x, int y, char prog, char poff, unsigned char coff)
{
    /*
     * XXX: HACK WARNING.  In my datafiles some Patch entries have
     * errors in widths. That causes malformed images, though the data
     * is there.  I corrected the problem here (look for do_fix).
     * Someone else might also run into this problem, but maybe only
     * my data files are corrupted. For correct data behavior is not
     * changed.
     */
    PatchHdrSmall P;
    unsigned int j, do_fix = 0;
    FILE *in;
    in = sOpen("PATCHES.BUT", "rb", 0);
    fread(&display::graphics.palette()[coff * 3], 96, 1, in);
    fseek(in, (50 * plr + prog * 10 + poff) * (sizeof P), SEEK_CUR);
    fread(&P, sizeof P, 1, in);
    SwapPatchHdrSmall(&P);
    fseek(in, P.offset, SEEK_SET);

    if (P.w * P.h != P.size) {
        /* fprintf(stderr, "PatchMe(): w*h != size (%hhd*%hhd == %d != %hd)\n",
                P.w, P.h, P.w*P.h, P.size); */
        if ((P.w + 1) * P.h == P.size) {
            /* fprintf(stderr, "PatchMe(): P.w++ saves the day!\n"); */
            P.w++;
            do_fix = 1;
        }

        P.size = P.w * P.h;
    }

    display::Surface local(P.w, P.h);
    display::Surface local2(P.w, P.h);
    local2.copyFrom(display::graphics.screen(), x, y, x + P.w - 1, y + P.h - 1);

    fread(local.pixels(), P.size, 1, in);
    fclose(in);

    //RLED(buffer+20000,local.vptr,P.size);
    for (j = 0; j < P.size; j++)
        if (local.pixels()[j] != 0) {
            if (do_fix && ((j % P.w) + 1 == (unsigned char)P.w)) {
                continue;
            }

            local2.pixels()[j] = local.pixels()[j] + coff;
        }

    local2.copyTo(display::graphics.screen(), x, y);
}

void
AstFaces(char plr, int x, int y, char face)
{
    int32_t offset;
    int fx, fy;
    int face_offset = 0;
    FILE *fin;

    memset(&display::graphics.palette()[192], 0x00, 192);
    fin = sOpen("FACES.BUT", "rb", 0);
    fseek(fin, 87 * sizeof(int32_t), SEEK_SET);
    fread(&display::graphics.palette()[192], 96, 1, fin);
    face_offset = ((int)face) * sizeof(int32_t);
    fseek(fin, face_offset, SEEK_SET);  // Get Face
    fread(&offset, sizeof(int32_t), 1, fin);
    Swap32bit(offset);
    fseek(fin, offset, SEEK_SET);

    display::Surface local(18, 15);
    fread(local.pixels(), 18 * 15, 1, fin);


    face_offset = ((int)(85 + plr)) * sizeof(int32_t);
    fseek(fin, face_offset, SEEK_SET);  // Get Helmet
    fread(&offset, sizeof(int32_t), 1, fin);
    Swap32bit(offset);
    fseek(fin, offset, SEEK_SET);
    display::Surface local2(80, 50);
    display::Surface local3(80, 50);
    fread(local2.pixels(), 80 * 50, 1, fin);
    fclose(fin);
    local3.clear(0);

    if (plr == 0)   {
        fx = 32;
        fy = 17;
    } else            {
        fx = 33;
        fy = 21;
    }

    local3.copyFrom(&local, 0, 0, local.width() - 1, local.height() - 1, fx, fy);

    local2.maskCopy(&local3, 0, display::Surface::DestinationEqual);

    local3.copyFrom(display::graphics.screen(), x, y, x + 79, y + 49);

    local3.maskCopy(&local2, 0, display::Surface::SourceNotEqual);

    local3.filter((7 + plr * 3), -160, display::Surface::NotEqual);

    local3.copyTo(display::graphics.screen(), x, y);
}


void SmHardMe(char plr, int x, int y, char prog, char planet, unsigned char coff)
{
    PatchHdrSmall P;
    unsigned int j;
    int do_fix = 0;
    FILE *in;

    in = sOpen("MHIST.BUT", "rb", 0);
    fread(&display::graphics.palette()[coff * 3], 64 * 3, 1, in);

    if (planet > 0) {
        fseek(in, (planet - 1) * (sizeof P), SEEK_CUR);
    } else {
        fseek(in, (7 + plr * 8 + prog) * (sizeof P), SEEK_CUR);
    }

    fread(&P, sizeof P, 1, in);
    SwapPatchHdrSmall(&P);
    fseek(in, P.offset, SEEK_SET);

    if (P.w * P.h != P.size) {
        /* fprintf(stderr, "SmHardMe(): w*h != size (%hhd*%hhd == %d != %hd)\n",
                P.w, P.h, P.w*P.h, P.size); */
        if ((P.w + 1) * P.h == P.size) {
            /* fprintf(stderr, "SmHardMe(): P.w++ saves the day!\n"); */
            P.w++;
            do_fix = 1;
        }

        P.size = P.w * P.h;
    }

    display::Surface local(P.w, P.h);
    display::Surface local2(P.w, P.h);

    local2.copyFrom(display::graphics.screen(), x, y, x + P.w - 1, y + P.h - 1);
    fread(local.pixels(), P.size, 1, in);
    fclose(in);

    //RLED(buffer+20000,local.vptr,P.size);
    for (j = 0; j < P.size; j++)
        if (local.pixels()[j] != 0) {
            if (do_fix && ((j % P.w) + 1 == (unsigned char)P.w)) {
                continue;
            }

            local2.pixels()[j] = local.pixels()[j] + coff;
        }

    local2.copyTo(display::graphics.screen(), x, y);

    if (planet > 0 && prog == 6) {
        SmHardMe(plr, x + planet * 2, y + 5, prog, 0, coff);
    }

    if (planet == 7 || planet == 6) {
        SmHardMe(plr, x + planet * 2, y + 5, prog, 0, coff);
    }

    return;
}

void BigHardMe(char plr, int x, int y, char hw, char unit, char sh, unsigned char coff)
{
    SimpleHdr table;
    char ch;
    int32_t size;
    unsigned int n;
    FILE *in, *fin;
    struct TM {
        char ID[4];
        int32_t offset;
        int32_t size;
    } AIndex;

    if (sh == 0) {
        size = (plr * 32) + (hw * 8) + unit;
        in = sOpen("RDFULL.BUT", "rb", 0);
        fseek(in, size * sizeof_SimpleHdr, SEEK_CUR);
        fread_SimpleHdr(&table, 1, in);
        fseek(in, table.offset, SEEK_SET);
        display::Surface local(104, 77);
        display::Surface local2(104, 77);
        fread(&display::graphics.palette()[coff * 3], 96 * 3, 1, in); // Individual Palette
        fread(local2.pixels(), table.size, 1, in); // Get Image
        fclose(in);
        RLED_img(local2.pixels(), local.pixels(), table.size, local.width(), local.height());


        local.filter(0, coff, display::Surface::Any);
        //TODO: Determine why the last pixel needed to be 0?
        /*
        n = 104 * 77;
        local.pixels()[n - 1] = 0;
        */

        local.copyTo(display::graphics.screen(), x, y);
    } else {
        memset(Name, 0x00, sizeof Name);

        if (plr == 0) {
            strcat(Name, "US");
        } else {
            strcat(Name, "SV");
        }

        ch = 0x30 + unit;

        switch (hw) {
        case 0:
            strcat(Name, "P");
            break;

        case 1:
            strcat(Name, "R");
            break;

        case 2:
            strcat(Name, "C");
//       if (strncmp(Data->P[plr].Manned[MANNED_HW_TWO_MAN_CAPSULE].Name,"ZOND",4)==0) ch=0x37;
            break;

        case 3:
            strcat(Name, "M");
            break;
        }

        Name[3] = ch;

        fin = sOpen("LIFTOFF.ABZ", "rb", 0);
        fread(&AIndex, sizeof AIndex, 1, fin);

        while (strncmp(AIndex.ID, Name, 4) != 0) {
            fread(&AIndex, sizeof AIndex, 1, fin);
        }

        Swap32bit(AIndex.offset);
        Swap32bit(AIndex.size);
        fseek(fin, AIndex.offset, SEEK_SET);

        fread(&AHead, sizeof AHead, 1, fin);
        Swap16bit(AHead.w);
        Swap16bit(AHead.h);
        fread(&display::graphics.palette()[coff * 3], 64 * 3, 1, fin);
        fseek(fin, 3 * (AHead.cNum - 64), SEEK_CUR);
        display::Surface local(AHead.w, AHead.h);

        fread(&BHead, sizeof BHead, 1, fin);
        Swap32bit(BHead.fSize);
        fread(vhptr->pixels(), BHead.fSize, 1, fin);
        RLED_img(vhptr->pixels(), local.pixels(), BHead.fSize, local.width(), local.height());
        n = (AHead.w * AHead.h); //gxVirtualSize(gxVGA_13, AHead.w, AHead.h);

        local.filter(0, -(128 - coff), display::Surface::NotEqual);

        //TODO: Determine why the first pixel needed to be zero?
        //local.pixels()[0] = 0x00;

        local.copyTo(display::graphics.screen(), 0, 0, x + 1, y, x + 102, y + 76);
        fclose(fin);
    }
}

void
DispHelp(char top, char bot, char *txt)
{
    int i, pl = 0;

    i = 0;

    while (i++ < top) {
        if (txt[i * 42] == (char) 0xcc) {
            display::graphics.setForegroundColor(txt[i * 42 + 1]);
        }
    }

    i = top;
    fill_rectangle(38, 49, 260, 127, 3);

    while (i <= bot && pl < 11) {
        if (txt[i * 42] == (char) 0xCC) {
            display::graphics.setForegroundColor(txt[i * 42 + 1]);
            draw_string(45, 55 + 7 * pl, &txt[i * 42 + 2]);
        } else {
            draw_string(45, 55 + 7 * pl, &txt[i * 42]);
        }

        pl++;
        i++;
    }

    return;
}

int Help(const char *FName)
{
    int i, j, line, top = 0, bot = 0, plc = 0;
    char *Help, *NTxt, mode;
    int fsize;
    FILE *fin;
    int32_t count;
    struct Help {
        char Code[6];
        int32_t offset;
        int16_t size;
    } Pul;

    mode = 0; /* XXX check uninitialized */
    NTxt = NULL; /* XXX check uninitialized */

    if (strncmp(&FName[1], "000", 3) == 0) {
        return 0;
    }

    fin = sOpen("HELP.CDR", "rb", 0);
    fread(&count, sizeof count, 1, fin);
    fread(&Pul, sizeof Pul, 1, fin);
    Swap32bit(count);

    i = 0;

    while (xstrncasecmp(Pul.Code, FName, 4) != 0 && i < count) {
        fread(&Pul, sizeof Pul, 1, fin);
        i++;
    }

    if (i == count) {
        fclose(fin);
        return 0;
    }

    Swap32bit(Pul.offset);
    Swap16bit(Pul.size);

    AL_CALL = 1;
    Help = (char *)xmalloc(Pul.size);
    fseek(fin, Pul.offset, SEEK_SET);
    fread(Help, Pul.size, 1, fin);
    fclose(fin);

    // Process File
    i = 0;
    j = 0;
    line = 0;
    fsize = 1;

    while (line < fsize) {
        if (Help[i] == 0x3B) while (Help[i++] != 0x0a); // Remove Comments
        else if (Help[i] == 0x25) { // Percent for line qty
            i++;
            fsize = 10 * (Help[i] - 0x30) + (Help[i + 1] - 0x30) + 1;
            bot = fsize;

            NTxt = (char *)xmalloc((unsigned int)(42 * fsize));
            memset(NTxt, 0x00, (unsigned int)(42 * fsize));
            j = line * 42; // should be 0
            mode = Help[i + 3] - 0x30;
            i += 6;
        } else if (Help[i] == 0x2e) { // Period
            i++;
            assert(NTxt != NULL);
            NTxt[j++] = (char) 0xcc;
            NTxt[j++] = (Help[i] - 0x30) * 100 + (Help[i + 1] - 0x30) * 10 + (Help[i + 2] - 0x30);
            i += 5;
        } else { // Text of Line
            assert(NTxt != NULL);

            while (Help[i] != 0x0d) {
                NTxt[j++] = Help[i++];
            }

            NTxt[j] = 0x00;
            i += 2;
            line++;
            j = line * 42;
        }
    }

    free(Help);

    key = 0;
    display::Surface local(250, 128);
    local.copyFrom(display::graphics.screen(), 34, 32, 283, 159);

    ShBox(34, 32, 283, 159);
    InBox(37, 35, 279, 45);
    InBox(37, 48, 261, 128);
    InBox(264, 48, 279, 128);
    fill_rectangle(265, 49, 278, 127, 0);
    ShBox(266, 50, 277, 87);
    draw_up_arrow(268, 56);
    ShBox(266, 89, 277, 126);
    draw_down_arrow(268, 95);
    fill_rectangle(38, 36, 278, 44, 7);

    if (mode == 0) {
        IOBox(83, 131, 241, 156);
        draw_heading(119, 137, "CONTINUE", 1, 0);
    } else {
        IOBox(83, 131, 156, 156);
        IOBox(168, 131, 241, 156);
        draw_heading(103, 137, "YES", 1, 0);
        draw_heading(192, 137, "NO", 1, 0);
    }

    // Display Title
    display::graphics.setForegroundColor(NTxt[1]);
    fsize = strlen(&NTxt[2]);
    draw_string(157 - fsize * 3, 42, &NTxt[2]);
    top = plc = 1;
    DispHelp(plc, bot - 1, &NTxt[0]);
    av_sync();

    WaitForMouseUp();
    i = 2;

    while (i == 2) {
        GetMouse();

        if (mode == 0 && ((x > 85 && y >= 133 && x <= 239 && y <= 154 && mousebuttons > 0) || (key == 'C' || key == K_ENTER))) {
            InBox(85, 133, 239, 154);
            i = 0;
            WaitForMouseUp();
            key = 0;
            // Continue Response
        }

        if (mode == 1 && ((x >= 85 && y >= 133 && x <= 154 && y <= 154 && mousebuttons > 0) || key == 'Y')) {
            InBox(85, 133, 154, 154);
            i = 1;
            WaitForMouseUp();
            key = 0;
            // Yes Response
        }

        if (mode == 1 && ((x > 170 && y >= 133 && x <= 239 && y <= 154 && mousebuttons > 0) || key == 'N')) {
            InBox(170, 133, 239, 154);
            i = -1;
            WaitForMouseUp();
            key = 0;
            // No Response
        }

        if (plc > top && ((x >= 266 && y > 50 && x <= 277 && y <= 87 && mousebuttons > 0) || (key >> 8) == 72)) {
            InBox(266, 50, 277, 87);
            // WaitForMouseUp();
            plc--;
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 50, 277, 87);
            key = 0;
        }  // Up

        if ((plc + 11) < bot && ((x >= 266 && y > 89 && x <= 277 && y <= 126 && mousebuttons > 0) || (key >> 8) == 80)) {
            InBox(266, 89, 277, 126);
            // WaitForMouseUp();
            plc++;
            DispHelp(plc, bot, &NTxt[0]);
            OutBox(266, 89, 277, 126);
            key = 0;
        }  // Down

    }

    local.copyTo(display::graphics.screen(), 34, 32);
    free(NTxt);

    AL_CALL = 0;
    return i;
}

void writePrestigeFirst(char index)   ///index==plr
{
    char w = 0, i, draw = 0;

    for (i = 0; i < 28; i++) {
        //Preestige First
        if (w < 6 && Data->Prestige[i].Place == index && Data->PD[index][i] == 0) {
            if (draw == 0) {
                ShBox(6, 170, 314, 197);
                fill_rectangle(10, 173, 310, 194, 7);
                InBox(9, 172, 311, 195);
                ShBox(216, 156, 314, 172);
                fill_rectangle(220, 160, 310, 168, 9);
                InBox(219, 159, 311, 169);
                fill_rectangle(216, 171, 216, 171, 3);
                fill_rectangle(312, 172, 313, 172, 3);
                display::graphics.setForegroundColor(11);
                draw_string(224, 166, "PRESTIGE FIRSTS");
                draw = 1;
            }

            display::graphics.setForegroundColor(11);
            draw_string(w > 2 ? 170 : 14,
                        w > 2 ? 179 + (w - 3) * 7 : 179 + w * 7,
                        &PF[i][0]);
            ++w;
            Data->PD[index][i] = 1;

            switch (i) {
            case 8:
                if (Data->Prestige[Prestige_Duration_E].Place == index && Data->PD[index][9] == 0) {
                    draw_string(0, 0, ", E");
                    Data->PD[index][9] = 1;
                }

            case 9:
                if (Data->Prestige[Prestige_Duration_D].Place == index && Data->PD[index][10] == 0) {
                    draw_string(0, 0, ", D");
                    Data->PD[index][10] = 1;
                }

            case 10:
                if (Data->Prestige[Prestige_Duration_C].Place == index && Data->PD[index][11] == 0) {
                    draw_string(0, 0, ", C");
                    Data->PD[index][11] = 1;
                }

            case 11:
                if (Data->Prestige[Prestige_Duration_B].Place == index && Data->PD[index][12] == 0) {
                    draw_string(0, 0, ", B");
                    Data->PD[index][12] = 1;
                }

            case 12:
                i = 12;

            default:
                break;
            }
        }
    }

    for (i = 0; i < 28; i++) {
        //Prestige Seconds
        if (w < 6 && Data->Prestige[i].mPlace == index && Data->PD[index][i] == 0) {
            if (draw == 0) {
                ShBox(6, 170, 314, 197);
                fill_rectangle(10, 173, 310, 194, 7);
                InBox(9, 172, 311, 195);
                ShBox(216, 156, 314, 172);
                fill_rectangle(220, 160, 310, 168, 9);
                InBox(219, 159, 311, 169);
                fill_rectangle(216, 171, 216, 171, 3);
                fill_rectangle(312, 172, 313, 172, 3);
                display::graphics.setForegroundColor(11);
                draw_string(224, 166, "PRESTIGE FIRSTS");
                draw = 1;
            }

            display::graphics.setForegroundColor(11);
            draw_string(w > 2 ? 170 : 14,
                        w > 2 ? 179 + (w - 3) * 7 : 179 + w * 7,
                        &PF[i][0]);
            ++w;
            Data->PD[index][i] = 1;

            switch (i) {
            case 8:
                if (Data->Prestige[Prestige_Duration_E].mPlace == index && Data->PD[index][9] == 0) {
                    draw_string(0, 0, ", E");
                    Data->PD[index][9] = 1;
                }

            case 9:
                if (Data->Prestige[Prestige_Duration_D].mPlace == index && Data->PD[index][10] == 0) {
                    draw_string(0, 0, ", D");
                    Data->PD[index][10] = 1;
                }

            case 10:
                if (Data->Prestige[Prestige_Duration_C].mPlace == index && Data->PD[index][11] == 0) {
                    draw_string(0, 0, ", C");
                    Data->PD[index][11] = 1;
                }

            case 11:
                if (Data->Prestige[Prestige_Duration_B].mPlace == index && Data->PD[index][12] == 0) {
                    draw_string(0, 0, ", B");
                    Data->PD[index][12] = 1;
                }

            case 12:
                i = 12;

            default:
                break;
            }

            draw_string(0, 0, " (2ND)");
        }
    }
}


void Draw_Mis_Stats(char plr, char index, int *where, char mode)
{
    int j, k, mcode;
    int let;


    if (mode == 0) {
        InBox(245, 5, 314, 17);
    }

    ShBox(6, 30, 209, 168); // Left Side
    fill_rectangle(10, 34, 205, 44, 9);
    InBox(9, 33, 206, 45);
    fill_rectangle(10, 49, 205, 119, 7);
    InBox(9, 48, 206, 120);
    fill_rectangle(10, 124, 205, 164, 7);
    InBox(9, 123, 206, 165);
    ShBox(62, 117, 154, 127);
    display::graphics.setForegroundColor(1);
    draw_string(79, 124, "FLIGHT CREW");
    display::graphics.setForegroundColor(11);
    draw_string(58, 41, "MISSION INFORMATION");
    draw_string(12, 104, "MISSION DURATION: ");
    Name[0] = Data->P[plr].History[index].Duration + 'A' - 1;
    Name[1] = 0;

    if (Name[0] >= 'A') {
        draw_string(0, 0, Name);
    } else {
        draw_string(0, 0, "NONE");
    }

    draw_string(12, 112, "PRESTIGE EARNED: ");

    if ((MAIL == -1 && Option == -1) || mode == 0) {
        draw_number(0, 0, Data->P[plr].History[index].Prestige);
    } else {
        draw_string(0, 0, "PENDING");
    }

    mcode = Data->P[plr].History[index].MissionCode;

    GetMisType(mcode);

    display::graphics.setForegroundColor(1);
    draw_string(12, 56, "MISSION NAME: ");
    draw_string(0, 0, (char *)Data->P[plr].History[index].MissionName);
    draw_string(12, 64, "MISSION TYPE:");
    display::graphics.setForegroundColor(11);
    draw_string(15, 72, Mis.Abbr);

    display::graphics.setForegroundColor(1);
    draw_string(12, 80, "RESULT: ");

    if (Data->P[plr].History[index].Duration != 0) {

        if (Data->P[plr].History[index].spResult == 4197) {
            draw_string(0, 0, "PRIMARY CREW DEAD");
        } else if (Data->P[plr].History[index].spResult == 4198) {
            draw_string(0, 0, "SECONDARY CREW DEAD");
        } else if (Data->P[plr].History[index].spResult == 4199) {
            draw_string(0, 0, "BOTH CREWS DEAD");
        } else draw_string(0, 0, ((Data->P[plr].History[index].spResult < 500) || (Data->P[plr].History[index].spResult >= 5000)) ? "SUCCESS" :
                               (Data->P[plr].History[index].spResult < 1999) ? "PARTIAL FAILURE" :
                               (Data->P[plr].History[index].spResult == 1999) ? "FAILURE" :
                               (Data->P[plr].History[index].spResult < 3000) ? "MISSION INJURY" :
                               (Data->P[plr].History[index].spResult < 4000) ? "MISSION DEATH" :
                               "ALL DEAD");
    } else if (Data->P[plr].History[index].Event == 0) {
        draw_string(0, 0, (Data->P[plr].History[index].spResult == 1) ? "SUCCESS" : "FAILURE");
    } else if (Data->P[plr].History[index].Event > 0) {
        draw_string(0, 0, "ARRIVE IN ");
        draw_number(0, 0, Data->P[plr].History[index].Event);
        draw_string(0, 0, " SEASON");

        if (Data->P[plr].History[index].Event >= 2) {
            draw_string(0, 0, "S");
        }
    }

    fill_rectangle(210, 30, 211, 168, 0); // Clear any leftover stuff
    ShBox(211, 30, 313, 168); // Right Side
    fill_rectangle(215, 34, 309, 44, 9);
    InBox(214, 33, 310, 45);
    display::graphics.setForegroundColor(11);
    draw_string(225, 41, "MISSION REPLAY");
    InBox(214, 55, 310, 116);

    AbzFrame(plr, index, 215, 56, 94, 60, "OOOO", mode);

    IOBox(214, 134, 310, 148);
    display::graphics.setForegroundColor(9);
    draw_string(224, 143, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "EPLAY MISSION");

    if (mode == 0) {
        IOBox(214, 151, 310, 165);
        display::graphics.setForegroundColor(1);
        draw_string(250, 160, "EXIT");
    }

    // Crew Stuff
    display::graphics.setForegroundColor(11);

    if (Data->P[plr].History[index].Man[PAD_A][0] == -1 && Data->P[plr].History[index].Man[PAD_B][0] == -1) {
        draw_string(13, 137, "UNMANNED MISSION");
    } else {
        // First Part -- Men
        if (Data->P[plr].History[index].Man[PAD_A][0] != -1) {
            for (j = 0; j < 4; j++) {
                k = Data->P[plr].History[index].Man[PAD_A][j];

                if (Data->P[plr].Pool[k].Sex == 1) {
                    display::graphics.setForegroundColor(18);    // Display women in blue, not yellow
                }

                if (k != -1) {
                    draw_string(13, 137 + j * 7, Data->P[plr].Pool[k].Name);
                }

                display::graphics.setForegroundColor(11);
            }

            let = 1; // Men on Part 1
        } else {
            let = 0;    // Men not on Part 1
        }

        // Second Part -- Men
        if (Data->P[plr].History[index].Man[PAD_B][0] != -1) {
            for (j = 0; j < 4; j++) {
                k = Data->P[plr].History[index].Man[PAD_B][j];

                if (Data->P[plr].Pool[k].Sex == 1) {
                    display::graphics.setForegroundColor(18);    // Display women in blue, not yellow
                }

                if (k != -1) {
                    draw_string(13 + let * 100, 137 + j * 7, Data->P[plr].Pool[k].Name);
                }

                display::graphics.setForegroundColor(11);
            }
        }

    }

    writePrestigeFirst(plr);

    if (mode == 1) {
        FadeIn(2, display::graphics.palette(), 10, 0, 0);
    };

    WaitForMouseUp();

    while (1) {
        GetMouse();

        if (mode == 0 && ((x >= 216 && y >= 153 && x <= 308 && y <= 163 && mousebuttons == 1) || (key == K_ENTER || key == 'E'))) {
            InBox(216, 153, 308, 153);
            OutBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            DrawMisHist(plr, where);
            key = 0;
            break;
        } else if (mode == 1 && ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons == 1) || key == K_ENTER)) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);
            delay(10);

            if (!AI[plr]) {
                music_stop();
            }

            FadeOut(2, display::graphics.palette(), 10, 0, 0);
            key = 0;
            break;
        } else if ((x >= 216 && y >= 136 && x <= 308 && y <= 146 && mousebuttons == 1) || (key == 'R')) {
            InBox(216, 136, 308, 146);

            if (mode == 0) {
                InBox(216, 153, 309, 163);
            }

            display::graphics.setForegroundColor(11);
            draw_string(225, 125, "PLAYING...");

            WaitForMouseUp();
            display::graphics.setForegroundColor(1);

            if (x == 0 && y == 0) {
                FILE *tin;

                tin = sOpen("REPL.TMP", "wb", 1); // Create temp image file
                fwrite(display::graphics.palette(), sizeof display::graphics.palette(), 1, tin);
                fwrite(display::graphics.screen()->pixels(), 64000, 1, tin);
                fclose(tin);
                FadeOut(2, display::graphics.palette(), 10, 0, 0);
                display::graphics.screen()->clear(0);
                FadeIn(2, display::graphics.palette(), 10, 0, 0);

                if (Data->P[plr].History[index].MissionCode == Mission_MarsFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_JupiterFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_SaturnFlyby) {
                    if (Data->P[plr].History[index].Event == 0 &&
                        Data->P[plr].History[index].spResult == 1)
                        switch (Data->P[plr].History[index].MissionCode) {
                        case 10:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUM1" : "WSM1");
                            break;

                        case 12:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUJ1" : "WSJ1");
                            break;

                        case 13:
                            Replay(plr, index, 0, 0, 320, 200, (plr == 0) ? "WUS1" : "WSS1");
                            break;

                        default:
                            break;
                        };
                } else {
                    Replay(plr, index, 0, 0, 320, 200, "OOOO");
                }

                FadeOut(2, display::graphics.palette(), 10, 0, 0);
                tin = sOpen("REPL.TMP", "rb", 1); // replad temp image file
                fread(display::graphics.palette(), sizeof display::graphics.palette(), 1, tin);
                fread(display::graphics.screen()->pixels(), 64000, 1, tin);
                fclose(tin);
                FadeIn(2, display::graphics.palette(), 10, 0, 0);
                key = 0;
                remove_savedat("REPL.TMP");

            } else {
                //Specs: Planetary Mission Klugge
                if (Data->P[plr].History[index].MissionCode == Mission_MarsFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_JupiterFlyby ||
                    Data->P[plr].History[index].MissionCode == Mission_SaturnFlyby) {
                    if (Data->P[plr].History[index].Event == 0 &&
                        Data->P[plr].History[index].spResult == 1) {
                        switch (Data->P[plr].History[index].MissionCode) {
                        case 10:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUM1" : "WSM1");
                            break;

                        case 12:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUJ1" : "WSJ1");
                            break;

                        case 13:
                            Replay(plr, index, 215, 56, 94, 60, (plr == 0) ? "WUS1" : "WSS1");
                            break;
                        }
                    } else {
                        Replay(plr, index, 215, 56, 94, 60, "OOOO");
                    }
                } else {
                    Replay(plr, index, 215, 56, 94, 60, "OOOO");
                }

                AbzFrame(plr, index, 215, 56, 94, 60, "OOOO", mode);
            }

            OutBox(216, 136, 308, 146);

            if (mode == 0) {
                OutBox(216, 153, 309, 163);
            }

            fill_rectangle(212, 119, 312, 127, 3);
            key = 0;

        }; // if

    }; // while

    display::graphics.videoRect().w = 0;

    display::graphics.videoRect().h = 0;

}


// EOF
