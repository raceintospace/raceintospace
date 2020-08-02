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

// This file handles the Mission Records screen

#include "display/graphics.h"

#include "Buzz_inc.h"
#include "records.h"
#include "hardef.h"
#include "draw.h"
#include "game_main.h"
#include "endianness.h"
#include "place.h"
#include "port.h"
#include "replay.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

#define MLL(a,b) (Data->P[a].History[b].MissionCode>=53 && Data->P[a].History[b].MissionCode<=56)

void RecChange(int i, int j, int k, int temp, int max, char Rec_Change, char hold);
int ISDOCK(int a);

char NREC[56][3];
Record_Entry rec[56][3];

void Move2rec(char *pos, char *pos2, char val);
void ClearRecord(char *pos2);
void BackTop(char *pos, char *pos2);
void Back1rec(char *pos, char *pos2);
void For1rec(char *pos, char *pos2);
void ForEnd(char *pos, char *pos2);
void Drec(char *pos, char *pos2, char mde);
void WriteRecord(int i, int j, int k, int temp);
void SwapRec(int Rc, int pl1, int pl2);
char CheckSucess(int i, int j);
size_t ImportRecordEntry(FILE *fin, Record_Entry &dst);
size_t ExportRecordEntry(FILE *fout, const Record_Entry &src);

int Pict[56] = {
    411, 2, 1, 177, 272, 275, 409, 501, 504, 507, 414,
    497, 476, 571, 162, 210, 574, 185, 506, 180, 181,
    496, 530, 521, 286, 325, 495, 172, 537, 441, 404,
    326, 572, 324, 301, 3, 17, 4, 18, 22, 19,
    295, 296, 262, 78, 282, 400, 309, 322, 308, 285,
    317, 247, 239, 322, 291
};

char *Record_Names[56] = {
    "ORBITAL SATELLITE",
    "MAN IN SPACE",
    "WOMAN IN SPACE",
    "PERSON IN ORBIT",
    "SPACEWALK (MALE)",
    "SPACEWALK (FEMALE)",
    "LUNAR FLYBY",
    "MERCURY FLYBY",
    "VENUS FLYBY",
    "MARS FLYBY",
    "JUPITER FLYBY",
    "SATURN FLYBY",
    "LUNAR PROBE LANDING",
    "ONE-PERSON CRAFT",
    "TWO-PERSON CRAFT",
    "THREE-PERSON CRAFT",
    "MINISHUTTLE",
    "FOUR-PERSON CRAFT",
    "SPACECRAFT PRESTIGE POINTS",
    "MANNED LUNAR PASS",
    "MANNED LUNAR ORBIT",
    "MANNED LUNAR LANDING",
    "FEWEST CASUALTIES IN A GAME",
    "MOST CASUALTIES IN A GAME",
    "HIGHEST SAFETY AVG LUNAR LANDING",
    "LOWEST SAFETY AVG LUNAR LANDING",
    "FIRST DOCKING",
    "FIRST MANNED DOCKING",
    "FIRST ORBITAL LABORATORY",
    "LONGEST MISSION DURATION",
    "MISSION W/ MOST PRESTIGE POINTS",
    "FEWEST MISSIONS IN GAME",
    "MOST MISSIONS IN GAME",
    "MOST MANNED MISSIONS ATTEMPTED",
    "MOST SUCCESSFUL MANNED MISSIONS",
    "MOST SPACE MISSIONS (MALE)",
    "MOST SPACE MISSIONS (FEMALE)",
    "HIGHEST PRESTIGE POINTS (MALE)",
    "HIGHEST PRESTIGE POINTS (FEMALE)",
    "MOST DAYS IN SPACE (MALE)",
    "MOST DAYS IN SPACE (FEMALE)",
    "LONGEST TERM OF DUTY",
    "HIGHEST DIRECTOR RATING ",
    "HIGHEST AVERAGE BUDGET",
    "LOWEST AVERAGE BUDGET",
    "GREATEST PRESTIGE TOTAL",
    "EARLIEST LOR LANDING",
    "EARLIEST EOR LANDING",
    "EARLIEST DIRECT ASCENT LANDING",
    "EARLIEST HISTORICAL LANDING",
    "UNITED STATES VICTORIES",
    "SOVIET UNION VICTORIES",
    "LOR VICTORIES",
    "EOR VICTORIES",
    "DIRECT ASCENT VICTORIES",
    "HISTORICAL VICTORIES"
};


char *Months[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
                   };

int ISDOCK(int a)
{
    if ((a >= 14 && a <= 24) || (a >= 27 && a <= 41) || a == 44 ||
        a == 47 || (a >= 48 && a <= 52) || a == 53 || a == 55 || a == 56) {
        return 1;
    } else {
        return 0;
    }
}

void MakeRecords(void)
{
    FILE *file;
    int i, j;

    if ((file = sOpen("RECORDS.DAT", "rb", FT_SAVE_CHECK)) == NULL) {
        file = sOpen("RECORDS.DAT", "wb", 1);

        if (file == NULL) {
            /* XXX: very drastic */
            CRITICAL1("can't create required file RECORDS.DAT");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < 56; i++) {
            for (j = 0; j < 3; j++) {
                rec[i][j].country = -1;
                rec[i][j].tag = rec[i][j].month = rec[i][j].yr = rec[i][j].program = rec[i][j].type = 0;
                rec[i][j].place = 0;

                ExportRecordEntry(file, rec[i][j]);
            }
        }
    }

    fclose(file);
}

void Records(char plr)
{
    FILE *file;
    char pos = 0, pos2 = 0;
    file = sOpen("RECORDS.DAT", "rb", 1);

    for (int i = 0; i < 56; i++) {
        for (int j = 0; j < 3; j++) {
            ImportRecordEntry(file, rec[i][j]);
        }
    }

    fclose(file);

    FadeOut(2, 5, 0, 0);
    PortPal(plr);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 199);
    InBox(4, 27, 315, 196);
    fill_rectangle(5, 28, 314, 195, 0);

    ShBox(23, 29, 228, 52);
    ShBox(23, 54, 228, 77);
    ShBox(23, 79, 228, 102);

    IOBox(243, 3, 316, 19);  // Bottom half of the screen stuff
    ShBox(6, 104, 313, 194);
    InBox(10, 127, 309, 191);
    fill_rectangle(11, 128, 308, 190, 0);
    ShBox(297, 129, 307, 158);
    ShBox(297, 160, 307, 189);  // Arrows
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    draw_heading(42, 5, "MISSION RECORDS", 0, -1);
    draw_up_arrow(299, 131);
    draw_down_arrow(299, 162);
    ShBox(6, 29, 21, 102);
    ShBox(230, 29, 313, 102);
    display::graphics.setForegroundColor(1);
    draw_number(12, 42, 1);
    draw_number(12, 66, 2);
    draw_number(12, 90, 3);
    fill_rectangle(70, 109, 266, 121, 4);
    fill_rectangle(72, 111, 264, 119, 0);
    IOBox(234, 84, 309, 99);
    display::graphics.setForegroundColor(1);
    draw_string(238, 94, "CLEAR RECORD");
    InBox(237, 34, 306, 81);
    Drec(&pos, &pos2, 0);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    while (1) {
        GetMouse();

        // Parse Button actions, note that return is embedded in first pButton
        if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(245, 5, 314, 17);

            if (key > 0) {
                delay(300);
                key = 0;
            }

            WaitForMouseUp();

            OutBox(245, 5, 314, 17);

            key = 0;

            return;
        }

        if (key == K_HOME) {
            BackTop(&pos, &pos2);
        }

        if (key == K_PGUP) {
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
            Back1rec(&pos, &pos2);
        }

        if (key == K_PGDN) {
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
            For1rec(&pos, &pos2);
        }

        if (key == K_END) {
            ForEnd(&pos, &pos2);
        }

        pButton(297, 129, 307, 158, Back1rec(&pos, &pos2), key >> 8, 72);
        pButton(297, 160, 307, 189, For1rec(&pos, &pos2), key >> 8, 80);
        pButton(236, 86, 307, 97, ClearRecord(&pos2), key, 'C');
        Button2(15, 129, 160, 133, Move2rec(&pos, &pos2, 0), key, 49);
        Button2(15, 136, 160, 140, Move2rec(&pos, &pos2, 1), key, 50);
        Button2(15, 143, 160, 147, Move2rec(&pos, &pos2, 2), key, 51);
        Button2(15, 150, 160, 154, Move2rec(&pos, &pos2, 3), key, 52);
        Button2(15, 157, 160, 161, Move2rec(&pos, &pos2, 4), key, 53);
        Button2(15, 164, 160, 168, Move2rec(&pos, &pos2, 5), key, 54);
        Button2(15, 171, 160, 175, Move2rec(&pos, &pos2, 6), key, 55);
        Button2(15, 178, 160, 182, Move2rec(&pos, &pos2, 7), key, 56);
        Button2(15, 185, 160, 189, Move2rec(&pos, &pos2, 8), key, 57);
        key = 0;
    }
}

void Move2rec(char *pos, char *pos2, char val)
{
    *pos2 = *pos + val;
    Drec(pos, pos2, 1);
}

void ClearRecord(char *pos2)
{
    FILE *file;
    int choice = Help("i125");

    if (choice == -1) {
        return;
    }

    file = sOpen("RECORDS.DAT", "rb", 1);

    for (int i = 0; i < 56; i++) {
        for (int j = 0; j < 3; j++) {
            ImportRecordEntry(file, rec[i][j]);
        }
    }

    fclose(file);

//clear record
    for (int j = 0; j < 3; j++) {
        NREC[*pos2][j] = 0x00;
        rec[*pos2][j].country = -1;
        rec[*pos2][j].tag = rec[*pos2][j].month = rec[*pos2][j].yr = rec[*pos2][j].program = 0;
        rec[*pos2][j].place = 0;
    }

    ShBox(23, 29, 228, 52);
    ShBox(23, 54, 228, 77);
    ShBox(23, 79, 228, 102);
    fill_rectangle(24, 30, 227, 51, 3);
    fill_rectangle(24, 55, 227, 76, 3);
    fill_rectangle(24, 80, 227, 101, 3);
    display::graphics.setForegroundColor(1);
    draw_number(12, 42, 1);
    draw_number(12, 66, 2);
    draw_number(12, 90, 3);

    file = sOpen("RECORDS.DAT", "wb", 1);

    for (int i = 0; i < 56; i++) {
        for (int j = 0; j < 3; j++) {
            ExportRecordEntry(file, rec[i][j]);
        }
    }

    fclose(file);
    return;
}

void BackTop(char *pos, char *pos2)
{
    *pos = 0;
    *pos2 = 0;
    Drec(pos, pos2, 1);
    return;
}

void Back1rec(char *pos, char *pos2)
{
    if (*pos2 == 0) {
        return;
    }

    *pos2 -= 1;

    if (*pos2 < *pos) {
        *pos -= 1;
    }

    Drec(pos, pos2, 1);
    return;
}

void For1rec(char *pos, char *pos2)
{
    if (*pos2 == 55) {
        return;
    }

    *pos2 += 1;

    if (*pos > 47) ;
    else if (*pos2 > *pos + 8) {
        *pos += 1;
    }

    Drec(pos, pos2, 1);
    return;
}

void ForEnd(char *pos, char *pos2)
{
    *pos = 47;
    *pos2 = 55;
    Drec(pos, pos2, 1);
    return;
}


void Drec(char *pos, char *pos2, char mde)
{
    char i, j = 0, Digit[10];


    memset(Digit, 0x00, sizeof(Digit));
    display::graphics.setForegroundColor(1);
    draw_number(12, 42, 1);
    draw_number(12, 66, 2);
    draw_number(12, 90, 3);
    fill_rectangle(238, 35, 305, 80, 0);

    if (Pict[*pos2] != 0) {
        DispBaby(238, 35, Pict[*pos2], mde);
    }

    fill_rectangle(12, 129, 295, 190, 0);  // Clear bottom text

    if (NREC[*pos2][0] == 0) {
        ShBox(23, 29, 228, 52);
        fill_rectangle(24, 30, 227, 51, 3);
    } else {
        display::graphics.setForegroundColor(8);
        draw_number(12, 42, 1);
        InBox(23, 29, 228, 52);
        fill_rectangle(24, 30, 227, 51, 3);
    }

    if (NREC[*pos2][1] == 0) {
        ShBox(23, 54, 228, 77);
        fill_rectangle(24, 55, 227, 76, 3);
    } else {
        display::graphics.setForegroundColor(8);
        draw_number(12, 66, 2);
        InBox(23, 54, 228, 77);
        fill_rectangle(24, 55, 227, 76, 3);
    }

    if (NREC[*pos2][2] == 0) {
        ShBox(23, 79, 228, 102);
        fill_rectangle(24, 80, 227, 101, 3);
    } else {
        display::graphics.setForegroundColor(8);
        draw_number(12, 90, 3);
        InBox(23, 79, 228, 102);
        fill_rectangle(24, 80, 227, 101, 3);
    }

    display::graphics.setForegroundColor(2);

    for (i = *pos; i < *pos + 9; i++, j++) {
        if (i == *pos2) {
            display::graphics.setForegroundColor(11);
        }

        draw_string(15, 133 + 7 * j, Record_Names[i]);

        if (i == *pos2) {
            display::graphics.setForegroundColor(11);
        } else {
            display::graphics.setForegroundColor(12);
        }

        display::graphics.setForegroundColor(2);
    }

    fill_rectangle(72, 111, 264, 119, 0);
    display::graphics.setForegroundColor(1);
    draw_string(83, 117, Record_Names[*pos2]);

    for (i = 0; i < 3; i++) {
        if (rec[*pos2][i].country == NOT_SET) {

            return;
        }

        if (*pos2 < 52) {
            fill_rectangle(27, 33 + (i * 24), 54, 49 + (i * 24), 4);
            draw_small_flag(rec[*pos2][i].country, 28, 34 + (i * 24));

            if (*pos2 == 50 || *pos2 == 51) {
                fill_rectangle(196, 33 + (i * 24), 223, 49 + (i * 24), 4);
                draw_small_flag(rec[*pos2][i].country, 197, 34 + (i * 24));
            }
        } else {
            fill_rectangle(27, 33 + (i * 24), 54, 49 + (i * 24), 4);
            draw_small_flag(0, 28, 34 + (i * 24));
            fill_rectangle(196, 33 + (i * 24), 223, 49 + (i * 24), 4);
            draw_small_flag(1, 197, 34 + (i * 24));
        }

        if (*pos2 < 50) {
            display::graphics.setForegroundColor(9);
            draw_string(61, 48 + (i * 24), "DIR: ");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, rec[*pos2][i].name);
        }

        switch (rec[*pos2][i].type) {
        case 1:
            display::graphics.setForegroundColor(9);
            draw_string(61, 38 + (i * 24), "DATE: ");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, Months[rec[*pos2][i].month]);
            draw_string(0, 0, " ");
            sprintf(&Digit[0], "%d", rec[*pos2][i].yr + 1900);
            draw_string(0, 0, &Digit[0]);

            if (*pos2 == 29) {
                display::graphics.setForegroundColor(6);
                draw_string(143, 38 + (i * 24), "DURATION: ");
                display::graphics.setForegroundColor(1);

                switch (rec[*pos2][i].tag) {
                case 1:
                    draw_string(0, 0, "A");
                    break;

                case 2:
                    draw_string(0, 0, "B");
                    break;

                case 3:
                    draw_string(0, 0, "C");
                    break;

                case 4:
                    draw_string(0, 0, "D");
                    break;

                case 5:
                    draw_string(0, 0, "E");
                    break;

                case 6:
                    draw_string(0, 0, "F");
                    break;

                default:
                    break;
                }
            }

            break;

        case 2:
            display::graphics.setForegroundColor(9);

            if (rec[*pos2][i].country == 1) {
                draw_string(61, 38 + (i * 24), "COSMONAUT: ");
            } else {
                draw_string(61, 38 + (i * 24), "ASTRONAUT: ");
            }

            display::graphics.setForegroundColor(1);
            draw_string(0, 0, rec[*pos2][i].astro);

            switch (*pos2) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "DATE: ");
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, Months[rec[*pos2][i].month]);
                draw_string(0, 0, " ");
                sprintf(&Digit[0], "%d", rec[*pos2][i].yr + 1900);
                draw_string(0, 0, &Digit[0]);
                break;

            case 35:
            case 36:
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "MISSIONS: ");
                display::graphics.setForegroundColor(1);
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(0, 0, &Digit[0]);
                break;

            case 37:
            case 38:
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "PRESTIGE: ");
                display::graphics.setForegroundColor(1);
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(0, 0, &Digit[0]);
                break;

            case 39:
            case 40:
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "DAYS: ");
                display::graphics.setForegroundColor(1);
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(0, 0, &Digit[0]);
                break;

            case 41:
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "SEASONS: ");
                display::graphics.setForegroundColor(1);
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(0, 0, &Digit[0]);
                break;

            default:
                break;
            }

            break;

        case 3:
            display::graphics.setForegroundColor(9);
            draw_string(61, 38 + (i * 24), "RECORD: ");
            display::graphics.setForegroundColor(1);

            if (*pos2 == 18) {  //special case craft and prestige points
                display::graphics.setForegroundColor(1);
                draw_string(0, 0, &Data->P[rec[*pos2][i].country].Manned[rec[*pos2][i].program].Name[0]);
                display::graphics.setForegroundColor(6);
                draw_string(143, 48 + (i * 24), "PRESTIGE: ");
                display::graphics.setForegroundColor(1);
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(0, 0, &Digit[0]);
            } else {
                sprintf(&Digit[0], "%d", rec[*pos2][i].tag);
                draw_string(101, 38 + (i * 24), &Digit[0]);

                switch (*pos2) {
                case 22:
                case 23:
                    draw_string(0, 0, " CASUALTIES");
                    break;

                case 24:
                case 25:
                    draw_string(0, 0, " PERCENT");
                    break;

                case 31:
                case 32:
                    draw_string(0, 0, " MISSIONS");
                    break;

                case 33:
                case 34:
                    draw_string(0, 0, " MISSIONS");
                    break;

                case 42:
                    draw_string(0, 0, " POINTS");
                    break;

                case 43:
                case 44:
                    draw_string(0, 0, " M.B.");
                    break;

                case 50:
                case 51:
                case 52:
                case 53:
                case 54:
                case 55:
                    draw_string(0, 0, " TIMES");
                    break;

                default:
                    break;
                }
            }

            break;

        default:
            break;
        }
    }

    return;
}

void WriteRecord(int i, int j, int k, int temp)
{
    char t;
    t = rec[k][0].place;
    NREC[k][t] = 1;
    ++rec[k][0].place;

    switch (rec[k][t].type) {
    case 1:
        rec[k][t].yr = Data->P[i].History[j].MissionYear;
        rec[k][t].month = Data->P[i].History[j].Month;
        rec[k][t].program = Data->P[i].History[j].MissionCode;

        if (k == 29 || k == 30) {
            rec[k][t].tag = temp;
        }

        break;

    case 2:
        if (k >= 35 && k <= 41) {
            rec[k][t].tag = temp;
            strcpy(&rec[k][t].astro[0], &Data->P[i].Pool[j].Name[0]);
        } else {
            rec[k][t].yr = Data->P[i].History[j].MissionYear;
            rec[k][t].month = Data->P[i].History[j].Month;
            strcpy(&rec[k][t].astro[0], &Data->P[i].Pool[temp].Name[0]);
        }

        break;

    case 3:
        rec[k][t].tag = temp;

        if (k == 18) {
            rec[k][t].program = j;
        }

        break;

    default:
        break;
    }

    strcpy(&rec[k][t].name[0], &Data->P[i].Name[0]);
    rec[k][t].country = i;
    return;
}

void SwapRec(int Rc, int pl1, int pl2)
{
    int temp;
    char Tmp[20];

    temp = NREC[Rc][pl1];
    NREC[Rc][pl1] = NREC[Rc][pl2];
    NREC[Rc][pl2] = temp;

    temp = rec[Rc][pl1].country;
    rec[Rc][pl1].country = rec[Rc][pl2].country;
    rec[Rc][pl2].country = temp;
    temp = rec[Rc][pl1].yr;
    rec[Rc][pl1].yr = rec[Rc][pl2].yr;
    rec[Rc][pl2].yr = temp;
    temp = rec[Rc][pl1].month;
    rec[Rc][pl1].month = rec[Rc][pl2].month = temp;
    temp = rec[Rc][pl1].program;
    rec[Rc][pl1].program = rec[Rc][pl2].program;
    rec[Rc][pl2].program = temp;
    temp = rec[Rc][pl1].tag;
    rec[Rc][pl1].tag = rec[Rc][pl2].tag;
    rec[Rc][pl2].tag = temp;
// Swap Astro
    strcpy(&Tmp[0], &rec[Rc][pl1].astro[0]);
    strcpy(&rec[Rc][pl1].astro[0], &rec[Rc][pl2].astro[0]);
    strcpy(&rec[Rc][pl2].astro[0], &Tmp[0]);
// Swap Name
    strcpy(&Tmp[0], &rec[Rc][pl1].name[0]);
    strcpy(&rec[Rc][pl1].name[0], &rec[Rc][pl2].name[0]);
    strcpy(&rec[Rc][pl2].name[0], &Tmp[0]);
    return;
}


char CheckSucess(int i, int j)
{
    if (Data->P[i].History[j].Duration != 0) {
        if (Data->P[i].History[j].spResult < 500 || Data->P[i].History[j].spResult >= 5000) {
            return 1;
        } else {
            return 0;
        }
    } else if (Data->P[i].History[j].spResult == 1 && Data->P[i].History[j].Event == 0) {
        return 1;
    } else {
        return 0;
    }
}

void SafetyRecords(char plr, int temp)
{
    int j, k;
    FILE *fin, *bo;
    fin = sOpen("RECORDS.DAT", "rb", 1);

    for (int i = 0; i < 56; i++) {
        for (j = 0; j < 3; j++) {
            ImportRecordEntry(fin, rec[i][j]);
        }
    }

    fclose(fin);
// deal with case highest safety and lowest safety average
    rec[24][0].type = 3;
    rec[24][1].type = 3;
    rec[24][2].type = 3;
    rec[25][0].type = 3;
    rec[25][1].type = 3;
    rec[25][2].type = 3;
    k = 24;
    j = 0;  //starting value

    while (k != -1) {
        switch (rec[k][0].place) {
        case 0:
            WriteRecord(plr, j, k, temp);
            break;

        case 1:
            if ((k == 24 && rec[k][0].tag <= temp) || (k == 25 && rec[k][0].tag > temp)) {
                WriteRecord(plr, j, k, temp);
                SwapRec(k, 1, 0);
            } else {
                WriteRecord(plr, j, k, temp);
            }

            break;

        case 2:
            if ((k == 24 && rec[k][1].tag <= temp) || (k == 25 && rec[k][1].tag > temp)) {
                WriteRecord(plr, j, k, temp);
                SwapRec(k, 2, 1);

                if ((k == 24 && rec[k][0].tag <= temp) || (k == 25 && rec[k][0].tag > temp)) {
                    SwapRec(k, 1, 0);
                }
            } else {
                WriteRecord(plr, j, k, temp);
            }

            break;

        case 3:
            if ((k == 24 && rec[k][2].tag <= temp) || (k == 25 && rec[k][2].tag > temp)) {
                --rec[k][0].place;
                WriteRecord(plr, j, k, temp);

                if ((k == 24 && rec[k][1].tag <= temp) || (k == 25 && rec[k][1].tag > temp)) {
                    SwapRec(k, 2, 1);

                    if ((k == 24 && rec[k][0].tag <= temp) || (k == 25 && rec[k][0].tag > temp)) {
                        SwapRec(k, 1, 0);
                    }
                }
            }

            break;

        default:
            break;
        }

        if (k == 24) {
            k = 25;
        } else if (k == 25) {
            k = -1;
        }
    }  //end while

    bo = sOpen("RECORDS.DAT", "wb", 1);

    for (int i = 0; i < 56; i++) {
        for (j = 0; j < 3; j++) {
            ExportRecordEntry(bo, rec[i][j]);
        }
    }

    fclose(bo);

    return;
}

void UpdateRecords(char Ty)
{
    int i, j, k, m, loop, temp, max;

    struct mStr *Miss;
    char Rec_Change, hold, craft;
    FILE *file;

    hold = 0; /* XXX check uninitialized */

    Miss = (struct mStr *)buffer;

    for (j = 0; j < 56; j++) {
        for (i = 0; i < 3; i++) {
            NREC[j][i] = 0x00;
        }
    }

    file = sOpen("MISSION.DAT", "rb", 0);  //Get Whole File
    fread(Miss, 60 * (sizeof(struct mStr)), 1, file);
    fclose(file);

    file = sOpen("RECORDS.DAT", "rb", 1);

    for (i = 0; i < 56; i++) {
        for (j = 0; j < 3; j++) {
            ImportRecordEntry(file, rec[i][j]);
        }
    }

    fclose(file);

    for (i = 0; i < NUM_PLAYERS; i++) {
        if (!AI[i])
            for (j = 0; j < Data->P[i].PastMissionCount; j++) {
                if (CheckSucess(i, j) == 1)
                    for (k = 0; k < 56; k++) {
                        Rec_Change = -1;
                        temp = 0;
                        max = 0;

                        //GetMisData(Data->P[i].History[j].MissionCode);
                        switch (k) {
                        case 0:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Orbital Satellite
                                if (Data->P[i].History[j].MissionCode == Mission_Orbital_Satellite) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_Orbital_Satellite) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 1:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Data->P[i].History[j].Duration > 0) {
                                for (m = 0; m < 4; m++) {
                                    if (Data->P[i].History[j].Man[PAD_A][m] != -1) {
                                        if (Data->P[i].Pool[Data->P[i].History[j].Man[PAD_A][m]].Sex == 0) {
                                            temp = Data->P[i].History[j].Man[PAD_A][m];

                                            if (rec[k][0].place == 0) {
                                                Rec_Change = 0;
                                            } else {
                                                Rec_Change = 2;
                                            }

                                            RecChange(i, j, k, temp, m, Rec_Change, hold);
                                        }
                                    }
                                }
                            }

                            Rec_Change = -1;
                            break;

                        case 2:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Data->P[i].History[j].Duration > 0) {
                                for (m = 0; m < 4; m++) {
                                    if (Data->P[i].History[j].Man[PAD_A][m] != -1) {
                                        if (Data->P[i].Pool[Data->P[i].History[j].Man[PAD_A][m]].Sex == 1) {
                                            temp = Data->P[i].History[j].Man[PAD_A][m];

                                            if (rec[k][0].place == 0) {
                                                Rec_Change = 0;
                                            } else {
                                                Rec_Change = 2;
                                            }

                                            RecChange(i, j, k, temp, m, Rec_Change, hold);
                                        }
                                    }
                                }
                            }

                            Rec_Change = -1;
                            break;

                        case 3:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Data->P[i].History[j].Duration > 0) {
                                for (m = 0; m < 4; m++) {
                                    if (Data->P[i].History[j].Man[PAD_A][m] != -1) {
                                        temp = Data->P[i].History[j].Man[PAD_A][m];

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 2;
                                        }

                                        RecChange(i, j, k, temp, m, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;
                            break;

                        case 4:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            //EVA record (MALE)
                            switch (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule]) {
                            case MANNED_HW_ONE_MAN_CAPSULE:
                                m = 0;
                                break;

                            case MANNED_HW_TWO_MAN_CAPSULE:
                            case MANNED_HW_THREE_MAN_CAPSULE:
                            case MANNED_HW_MINISHUTTLE:
                                m = 1;
                                break;

                            case MANNED_HW_FOUR_MAN_CAPSULE:
                                m = 2;
                                break;

                            default:
                                m = 0;
                                break;
                            }

                            if (Miss[Data->P[i].History[j].MissionCode].EVA) {
                                if (Data->P[i].History[j].Man[PAD_A][m] != -1)
                                    if (Data->P[i].Pool[Data->P[i].History[j].Man[PAD_A][m]].Sex == 0) {
                                        temp = Data->P[i].History[j].Man[PAD_A][m];

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 2;
                                        }

                                        RecChange(i, j, k, temp, m, Rec_Change, hold);
                                    }
                            }

                            Rec_Change = -1;
                            break;

                        case 5:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            //EVA record (FEMALE)
                            switch (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule]) {
                            case MANNED_HW_ONE_MAN_CAPSULE:
                                m = 0;
                                break;

                            case MANNED_HW_TWO_MAN_CAPSULE:
                            case MANNED_HW_THREE_MAN_CAPSULE:
                            case MANNED_HW_MINISHUTTLE:
                                m = 1;
                                break;

                            case MANNED_HW_FOUR_MAN_CAPSULE:
                                m = 2;
                                break;

                            default:
                                m = 0;
                                break;
                            }

                            if (Miss[Data->P[i].History[j].MissionCode].EVA) {
                                if (Data->P[i].History[j].Man[PAD_A][m] != -1)
                                    if (Data->P[i].Pool[Data->P[i].History[j].Man[PAD_A][m]].Sex == 1) {
                                        temp = Data->P[i].History[j].Man[PAD_A][m];

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 2;
                                        }

                                        RecChange(i, j, k, temp, m, Rec_Change, hold);
                                    }
                            }

                            Rec_Change = -1;
                            break;

                        case 6:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //LUNAR flyby
                                if (Data->P[i].History[j].MissionCode == Mission_LunarFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_LunarFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 7:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //MERCURY flyby
                                if (Data->P[i].History[j].MissionCode == Mission_MercuryFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_MercuryFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 8:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //VENUS flyby
                                if (Data->P[i].History[j].MissionCode == Mission_VenusFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_VenusFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 9:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //MARS flyby
                                if (Data->P[i].History[j].MissionCode == Mission_MarsFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_MarsFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 10:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //JUPITER flyby
                                if (Data->P[i].History[j].MissionCode == Mission_JupiterFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_JupiterFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 11:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //SATURN flyby
                                if (Data->P[i].History[j].MissionCode == Mission_SaturnFlyby) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_SaturnFlyby) {
                                Rec_Change = 1;
                            }

                            break;

                        case 12:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //LUNAR PROBE landing
                                if (Data->P[i].History[j].MissionCode == Mission_Lunar_Probe) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_Lunar_Probe) {
                                Rec_Change = 1;
                            }

                            break;

                        case 13:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //ONE-person craft
                                if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_ONE_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_ONE_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 14:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //TWO-person craft
                                if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_TWO_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_TWO_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 15:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //THREE-person craft
                                if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_THREE_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                    WriteRecord(i, j, k, 0);
                                }
                            } else if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_THREE_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 16:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //MINISHUTTLE craft
                                if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_MINISHUTTLE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_MINISHUTTLE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 17:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //FOUR-person craft
                                if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_FOUR_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].Hard[PAD_A][Mission_Capsule] == MANNED_HW_FOUR_MAN_CAPSULE && Data->P[i].History[j].Man[PAD_A][0] != -1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 18:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {
                                for (craft = 0; craft < 5; craft++) {
                                    max = temp = 0;
                                    Rec_Change = -1;

                                    for (loop = 0; loop < Data->P[i].PastMissionCount; loop++) {
                                        if (Data->P[i].History[loop].Prestige > 0) {
                                            max = Data->P[i].History[loop].Prestige;

                                            if (Data->P[i].History[loop].Hard[PAD_A][Mission_Capsule] == craft) {
                                                temp += max;
                                                Rec_Change = 1;
                                            }
                                        }
                                    }

                                    hold = j;
                                    j = craft;

                                    for (loop = 0; loop < 3; loop++) {
                                        if (rec[k][loop].program == j && rec[k][loop].tag == temp) {
                                            Rec_Change = -1;
                                        }
                                    }

                                    if (Rec_Change == 1)
                                        switch (rec[k][0].place) {
                                        case 0:
                                            WriteRecord(i, j, k, temp);
                                            break;

                                        case 1:
                                            if (rec[k][0].tag <= temp) {
                                                WriteRecord(i, j, k, temp);
                                                SwapRec(k, 1, 0);
                                            } else {
                                                WriteRecord(i, j, k, temp);
                                            }

                                            break;

                                        case 2:
                                            if (rec[k][1].tag <= temp) {
                                                WriteRecord(i, j, k, temp);
                                                SwapRec(k, 2, 1);

                                                if (rec[k][0].tag <= temp) {
                                                    SwapRec(k, 1, 0);
                                                }
                                            } else {
                                                WriteRecord(i, j, k, temp);
                                            }

                                            break;

                                        case 3:
                                            if (rec[k][2].tag <= temp) {
                                                --rec[k][0].place;
                                                WriteRecord(i, j, k, temp);

                                                if (rec[k][1].tag <= temp) {
                                                    SwapRec(k, 2, 1);

                                                    if (rec[k][0].tag <= temp) {
                                                        SwapRec(k, 1, 0);
                                                    }
                                                }
                                            }

                                            break;

                                        default:
                                            break;
                                        }

                                    j = hold;
                                }  //for (cra
                            } // if (Ty

                            Rec_Change = -1;
                            break;  //Spacecraft Prestige Points

                        case 19:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Manned Lunar Pass
                                if (Data->P[i].History[j].MissionCode == Mission_LunarPass) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_LunarPass) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 20:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Manned Lunar Orbit
                                if (Data->P[i].History[j].MissionCode == Mission_LunarOrbital ||
                                    Data->P[i].History[j].MissionCode == Mission_Lunar_Orbital) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_LunarOrbital ||
                                    Data->P[i].History[j].MissionCode == Mission_Lunar_Orbital) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 21:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Manned Lunar Landing
                                if (MLL(i, j) == 1 || (i == 1 && Data->P[i].History[j].MissionCode == Mission_Soyuz_LL)) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (MLL(i, j) == 1 || (i == 1 && Data->P[i].History[j].MissionCode == Mission_Soyuz_LL)) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 22:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Fewest Casualties In A Game
                                temp = 0;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    if (Data->P[i].Pool[loop].Status == AST_ST_DEAD) {
                                        ++temp;
                                    }
                                }

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 4;
                                }
                            }

                            break;

                        case 23:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Most Casualties In A Game
                                temp = 0;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    if (Data->P[i].Pool[loop].Status == AST_ST_DEAD) {
                                        ++temp;
                                    }

                                    if (rec[k][0].place == 0) {
                                        Rec_Change = 0;
                                    } else {
                                        Rec_Change = 3;
                                    }
                                }
                            }

                            break;

                        case 24:
                            break;  //Highest Safety Average (Lunar Mission)

                        case 25:
                            break;  //Lowest Safety Average (Lunar Mission)

                        case 26:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Docking Mission
                                if (ISDOCK(Data->P[i].History[j].MissionCode) == 1) {
                                    Rec_Change = 0;
                                }
                            } else if (ISDOCK(Data->P[i].History[j].MissionCode) == 1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 27:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Manned Docking
                                if (Data->P[i].History[j].MissionCode != Mission_U_Orbital_D && Data->P[i].History[j].MissionCode != Mission_Jt_Unmanned_Orbital_Docking &&
                                    ISDOCK(Data->P[i].History[j].MissionCode) == 1) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode != Mission_U_Orbital_D && Data->P[i].History[j].MissionCode != Mission_Jt_Unmanned_Orbital_Docking &&
                                       ISDOCK(Data->P[i].History[j].MissionCode) == 1) {
                                Rec_Change = 1;
                            }

                            break;

                        case 28:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Orbital Laboratory
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_OrbitingLab || Data->P[i].History[j].MissionCode == Mission_Jt_OrbitingLab_EVA) {
                                    Rec_Change = 0;
                                }
                            } else if (Data->P[i].History[j].MissionCode == Mission_Jt_OrbitingLab || Data->P[i].History[j].MissionCode == Mission_Jt_OrbitingLab_EVA) {
                                Rec_Change = 1;
                            }

                            break;

                        case 29:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;
                            temp = Data->P[i].History[j].Duration;  //Longest Mission Duration

                            if (rec[k][0].place == 0) {
                                if (temp > 0) {
                                    Rec_Change = 0;
                                }
                            } else if (temp > 0) {
                                Rec_Change = 5;
                            } else {
                                Rec_Change = -1;
                            }

                            break;

                        case 30:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;
                            temp = Data->P[i].History[j].Prestige;  //Single Mission Prestige

                            if (rec[k][0].place == 0) {
                                if (temp > 0) {
                                    Rec_Change = 0;
                                }
                            } else if (temp > 0) {
                                Rec_Change = 3;
                            } else {
                                Rec_Change = -1;
                            }

                            break;

                        case 31:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {
                                temp = Data->P[i].PastMissionCount;  //Fewest Missions

                                if (rec[k][0].place == 0) {
                                    if (temp > 0) {
                                        Rec_Change = 0;
                                    }
                                } else if (temp > 0) {
                                    Rec_Change = 4;
                                } else {
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 32:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {
                                temp = Data->P[i].PastMissionCount;  //Most Missions

                                if (rec[k][0].place == 0) {
                                    if (temp > 0) {
                                        Rec_Change = 0;
                                    }
                                } else if (temp > 0) {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 33:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Most Space Missions Attempted
                                temp = 0;

                                for (loop = 0; loop < Data->P[i].PastMissionCount; loop++) {
                                    if (Data->P[i].History[loop].Duration > 0) {
                                        ++temp;
                                    }
                                }

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 34:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Most Successful Space Missions
                                temp = 0;

                                for (loop = 0; loop < Data->P[i].PastMissionCount; loop++) {
                                    if (Data->P[i].History[loop].Duration > 0 && CheckSucess(i, loop) == 1) {
                                        ++temp;
                                    }
                                }

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 35:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;
                            max = 0;

                            if (Ty == 1) {  //Most Missions (MALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Missions >= max && Data->P[i].Pool[loop].Sex == 0) {
                                        max = Data->P[i].Pool[loop].Missions;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;
                            break;

                        case 36:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Most Missions (FEMALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Missions >= max && Data->P[i].Pool[loop].Sex == 1) {
                                        max = Data->P[i].Pool[loop].Missions;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;
                            break;

                        case 37:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Most Prestige (MALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Prestige >= max && Data->P[i].Pool[loop].Sex == 0) {
                                        max = Data->P[i].Pool[loop].Prestige;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;

                            break;

                        case 38:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Most Prestige (FEMALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Prestige >= max && Data->P[i].Pool[loop].Sex == 1) {
                                        max = Data->P[i].Pool[loop].Prestige;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;

                            break;

                        case 39:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Most Days (MALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Days >= max && Data->P[i].Pool[loop].Sex == 0) {
                                        max = Data->P[i].Pool[loop].Days;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;

                            break;

                        case 40:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Most Days (FEMALE)
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Days >= max && Data->P[i].Pool[loop].Sex == 1) {
                                        max = Data->P[i].Pool[loop].Days;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;

                            break;

                        case 41:
                            rec[k][0].type = 2;
                            rec[k][1].type = 2;
                            rec[k][2].type = 2;

                            if (Ty == 1) {  //Longest Term Of Duty
                                temp = 0;
                                max = 1;

                                for (loop = 0; loop < Data->P[i].AstroCount; loop++) {
                                    max = 1;

                                    if (Data->P[i].Pool[loop].Active >= max) {
                                        max = Data->P[i].Pool[loop].Active;

                                        if (rec[k][0].place == 0) {
                                            Rec_Change = 0;
                                        } else {
                                            Rec_Change = 3;
                                        }

                                        RecChange(i, j, k, loop, max, Rec_Change, hold);
                                    }
                                }
                            }

                            Rec_Change = -1;

                            break;

                        case 42:
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;
                            temp = max = 0;  //Highest Rating

                            if (Ty == 1) {
                                if (i == 0) {
                                    temp = (unsigned)CalcScore(0, Data->Def.Lev1, Data->Def.Lev2);
                                } else if (i == 1) {
                                    temp = (unsigned)CalcScore(1, Data->Def.Lev2, Data->Def.Lev1);
                                }

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 43:
                            temp = max = 0;
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Highest Average Budget
                                for (loop = 0; loop < 40; ++loop) {
                                    if (Data->P[i].BudgetHistory[loop] != 0) {
                                        temp += Data->P[i].BudgetHistory[loop];
                                    } else if (loop >= 6 && max == 0) {
                                        max = loop - 1;
                                    }
                                }

                                temp = temp / max;

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 44:
                            temp = max = 0;
                            rec[k][0].type = 3;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Lowest Average Budget
                                for (loop = 0; loop < 40; ++loop) {
                                    if (Data->P[i].BudgetHistory[loop] != 0) {
                                        temp += Data->P[i].BudgetHistory[loop];
                                    } else if (loop >= 6 && max == 0) {
                                        max = loop - 1;
                                    }
                                }

                                temp = temp / max;

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 4;
                                }
                            }

                            break;

                        case 45:
                            rec[k][0].type = 3;
                            temp = 0;
                            rec[k][1].type = 3;
                            rec[k][2].type = 3;

                            if (Ty == 1) {  //Highest Prestige Total
                                for (loop = 0; loop < Data->P[i].PastMissionCount; loop++) {
                                    temp += Data->P[i].History[loop].Prestige;
                                }

                                if (rec[k][0].place == 0) {
                                    Rec_Change = 0;
                                } else {
                                    Rec_Change = 3;
                                }
                            }

                            break;

                        case 46:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest EOR Landing
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_LOR) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_LOR) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 47:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest LOR Landing
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_EOR) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_EOR) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 48:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Direct Ascent Landing
                                if (Data->P[i].History[j].MissionCode == Mission_DirectAscent_LL) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_DirectAscent_LL) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 49:
                            rec[k][0].type = 1;
                            rec[k][1].type = 1;
                            rec[k][2].type = 1;

                            if (rec[k][0].place == 0) {  //Earliest Historical Lunar Landing
                                if (Data->P[i].History[j].MissionCode == Mission_HistoricalLanding) {
                                    Rec_Change = 0;
                                }
                            } else {
                                if (Data->P[i].History[j].MissionCode == Mission_HistoricalLanding) {
                                    Rec_Change = 1;
                                }
                            }

                            break;

                        case 50:
                            if (Ty == 1) {  //Total US Wins
                                if (MLL(i, j) == 1 && Data->Prestige[Prestige_MannedLunarLanding].Place == 0) {
                                    ++rec[k][0].tag;
                                    rec[k][0].place = 1;
                                    rec[k][0].type = 3;
                                    rec[k][0].country = 0;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 51:
                            if (Ty == 1) {  //Total SOVIET Wins
                                if ((MLL(i, j) == 1 || (i == 1 && Data->P[i].History[j].MissionCode == Mission_Soyuz_LL)) && Data->Prestige[Prestige_MannedLunarLanding].Place == 1) {
                                    ++rec[k][0].tag;
                                    rec[k][0].place = 1;
                                    rec[k][0].type = 3;
                                    rec[k][0].country = 1;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 52:
                            if (Ty == 1) {  //Total LOR Wins
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_EOR && CheckSucess(i, j) == 1) {
                                    ++rec[k][0].tag;
                                    rec[k][0].country = 2;
                                    rec[k][0].type = 3;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 53:
                            if (Ty == 1) {  //Total EOR Wins
                                if (Data->P[i].History[j].MissionCode == Mission_Jt_LunarLanding_LOR && CheckSucess(i, j) == 1) {
                                    ++rec[k][0].tag;
                                    rec[k][0].country = 2;
                                    rec[k][0].type = 3;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 54:
                            if (Ty == 1) {  //Total Direct Ascent Wins
                                if (Data->P[i].History[j].MissionCode == Mission_DirectAscent_LL && CheckSucess(i, j) == 1) {
                                    ++rec[k][0].tag;
                                    rec[k][0].country = 2;
                                    rec[k][0].type = 3;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        case 55:
                            if (Ty == 1) {  //Total Historical Wins
                                if (Data->P[i].History[j].MissionCode == Mission_HistoricalLanding && CheckSucess(i, j) == 1) {
                                    ++rec[k][0].tag;
                                    rec[k][0].country = 2;
                                    rec[k][0].type = 3;
                                    Rec_Change = -1;
                                }
                            }

                            break;

                        default:
                            break;
                        }  // end switch

                        if ((k < 35 || k > 41) && k != 18) {
                            RecChange(i, j, k, temp, max, Rec_Change, hold);
                        }
                    }
            }
    }

    //Change and Update Records
    file = sOpen("RECORDS.DAT", "wb", 1);

    for (i = 0; i < 56; i++) {
        for (j = 0; j < 3; j++) {
            ExportRecordEntry(file, rec[i][j]);
        }
    }

    fclose(file);
    return;
}

void RecChange(int i, int j, int k, int temp, int max, char Rec_Change, char hold)
{
    int loop;

    /* XXX: SEGFAULT - some record entries are null (rec[k][loop].astro) */

    if (Rec_Change != 0)
        for (loop = 0; loop < 3; loop++) {
            if (rec[k][loop].type == 3 && rec[k][loop].tag == temp && (rec[k][loop].country == i)) {
                Rec_Change = -1;
            }

            // rec[k][loop].country is sometimes -1
            // guard against this case
            // (is this the issue cited above?)
            if (rec[k][loop].type == 2 && rec[k][loop].country >= 0 && strcmp(&rec[k][loop].astro[0], &Data->P[rec[k][loop].country].Pool[temp].Name[0]) == 0) {
                Rec_Change = -1;
            }
        }

    switch (Rec_Change) {
    case 0:
        if (k >= 35 && k <= 41) {
            j = temp;  // holds astronaut
            temp = max;  // holds record
        }

        WriteRecord(i, j, k, temp);
        break;

    case 1:
    case 2:
        if (Rec_Change == 2) {
            temp = Data->P[i].History[j].Man[PAD_A][max];
        } else {
            temp = Data->P[i].History[j].Man[PAD_A][0];
        }

        switch (rec[k][0].place) {
        case 1:
            if ((rec[k][0].yr > Data->P[i].History[j].MissionYear)  ||
                (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                 rec[k][0].month >= Data->P[i].History[j].Month)) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 1, 0);
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 2:
            if ((rec[k][1].yr > Data->P[i].History[j].MissionYear)  ||
                (rec[k][1].yr == Data->P[i].History[j].MissionYear &&
                 rec[k][1].month >= Data->P[i].History[j].Month)) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 2, 1);

                if ((rec[k][0].yr > Data->P[i].History[j].MissionYear)    ||
                    (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                     rec[k][0].month >= Data->P[i].History[j].Month)) {
                    SwapRec(k, 1, 0);
                }
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 3:
            if ((rec[k][2].yr > Data->P[i].History[j].MissionYear)  ||
                (rec[k][2].yr == Data->P[i].History[j].MissionYear &&
                 rec[k][2].month >= Data->P[i].History[j].Month)) {
                --rec[k][0].place;
                WriteRecord(i, j, k, temp);

                if ((rec[k][1].yr > Data->P[i].History[j].MissionYear)    ||
                    (rec[k][1].yr == Data->P[i].History[j].MissionYear &&
                     rec[k][1].month >= Data->P[i].History[j].Month)) {
                    SwapRec(k, 2, 1);

                    if ((rec[k][0].yr > Data->P[i].History[j].MissionYear)  ||
                        (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                         rec[k][0].month >= Data->P[i].History[j].Month)) {
                        SwapRec(k, 1, 0);
                    }
                }
            }

            break;

        default:
            break;
        }

        break;

    case 3:
        if (k >= 35 && k <= 41) {
            j = temp;  // holds astronaut
            temp = max;  // holds record
        }

        switch (rec[k][0].place) {
        case 1:
            if (rec[k][0].tag <= temp) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 1, 0);
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 2:
            if (rec[k][1].tag <= temp) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 2, 1);

                if (rec[k][0].tag <= temp) {
                    SwapRec(k, 1, 0);
                }
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 3:
            if (rec[k][2].tag <= temp) {
                --rec[k][0].place;
                WriteRecord(i, j, k, temp);

                if (rec[k][1].tag <= temp) {
                    SwapRec(k, 2, 1);

                    if (rec[k][0].tag <= temp) {
                        SwapRec(k, 1, 0);
                    }
                }
            }

            break;

        default:
            break;
        }

        break;

    case 4:
        switch (rec[k][0].place) {
        case 1:
            if (rec[k][0].tag > temp) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 1, 0);
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 2:  // kludge cases 35-40 max holds tag temp holds astro
            if (rec[k][1].tag > temp) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 2, 1);

                if (rec[k][0].tag > temp) {
                    SwapRec(k, 1, 0);
                }
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 3:
            if (rec[k][2].tag > temp) {
                --rec[k][0].place;
                WriteRecord(i, j, k, temp);

                if (rec[k][1].tag > temp) {
                    SwapRec(k, 2, 1);

                    if (rec[k][0].tag > temp) {
                        SwapRec(k, 1, 0);
                    }
                }
            }

            break;

        default:
            break;
        }

        break;

    case 5:
        hold = 0;

        switch (rec[k][0].place) {
        case 1:
            if (rec[k][0].tag == temp)
                if ((rec[k][0].yr > Data->P[i].History[j].MissionYear) ||
                    (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                     rec[k][0].month >= Data->P[i].History[j].Month)) {
                    hold = 1;
                }

            if (rec[k][0].tag < temp) {
                hold = 1;
            }

            if (hold == 1) {
                WriteRecord(i, j, k, temp);
                SwapRec(k, 1, 0);
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 2:
            if (rec[k][1].tag == temp)
                if ((rec[k][1].yr > Data->P[i].History[j].MissionYear) ||
                    (rec[k][1].yr == Data->P[i].History[j].MissionYear &&
                     rec[k][1].month >= Data->P[i].History[j].Month)) {
                    hold = 1;
                }

            if (rec[k][1].tag < temp) {
                hold = 1;
            }

            if (hold == 1) {
                hold = 0;
                WriteRecord(i, j, k, temp);
                SwapRec(k, 2, 1);

                if (rec[k][0].tag == temp)
                    if ((rec[k][0].yr > Data->P[i].History[j].MissionYear) ||
                        (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                         rec[k][0].month >= Data->P[i].History[j].Month)) {
                        hold = 1;
                    }

                if (rec[k][0].tag < temp) {
                    hold = 1;
                }

                if (hold == 1) {
                    SwapRec(k, 1, 0);
                }
            } else {
                WriteRecord(i, j, k, temp);
            }

            break;

        case 3:
            if (rec[k][2].tag == temp)
                if ((rec[k][2].yr > Data->P[i].History[j].MissionYear) ||
                    (rec[k][2].yr == Data->P[i].History[j].MissionYear &&
                     rec[k][2].month >= Data->P[i].History[j].Month)) {
                    hold = 1;
                }

            if (rec[k][2].tag < temp) {
                hold = 1;
            }

            if (hold == 1) {
                hold = 0;
                --rec[k][0].place;
                WriteRecord(i, j, k, temp);

                if (rec[k][1].tag == temp)
                    if ((rec[k][1].yr > Data->P[i].History[j].MissionYear) ||
                        (rec[k][1].yr == Data->P[i].History[j].MissionYear &&
                         rec[k][1].month >= Data->P[i].History[j].Month)) {
                        hold = 1;
                    }

                if (rec[k][1].tag < temp) {
                    hold = 1;
                }

                if (hold == 1) {
                    hold = 0;
                    SwapRec(k, 2, 1);

                    if (rec[k][0].tag == temp)
                        if ((rec[k][0].yr > Data->P[i].History[j].MissionYear) ||
                            (rec[k][0].yr == Data->P[i].History[j].MissionYear &&
                             rec[k][0].month >= Data->P[i].History[j].Month)) {
                            hold = 1;
                        }

                    if (rec[k][0].tag < temp) {
                        hold = 1;
                    }

                    if (hold == 1) {
                        SwapRec(k, 1, 0);
                    }
                }
            }

            break;

        default:
            break;
        }  //end case #5 switch

        break;

    default:
        break;
    }  // end switch (Rec_Change)

    return;
}


/**
 * Read packed Record_Entry data from a file into an instance.
 *
 * If the entry is not read successfully, reading is stopped.
 * The entry may be partially read, and the file may be checked
 * for errors.
 *
 * \param fin  an open file handle.
 * \param dst  where to store the byte data.
 * \return  1 on success, 0 otherwise.
 */
size_t ImportRecordEntry(FILE *fin, Record_Entry &dst)
{
    bool success = true &&
                   fread(&dst.country, sizeof(dst.country), 1, fin) &&
                   fread(&dst.month, sizeof(dst.month), 1, fin) &&
                   fread(&dst.yr, sizeof(dst.yr), 1, fin) &&
                   fread(&dst.program, sizeof(dst.program), 1, fin) &&
                   fread(&dst.tag, sizeof(dst.tag), 1, fin) &&
                   fread(&dst.type, sizeof(dst.type), 1, fin) &&
                   fread(&dst.place, sizeof(dst.place), 1, fin) &&
                   fread(&dst.name[0], sizeof(dst.name), 1, fin) &&
                   fread(&dst.astro[0], sizeof(dst.astro), 1, fin);

    Swap16bit(dst.tag);
    return (success ? 1 : 0);
}


/**
 * Write a Record_Entry instance as a packed byte stream to a file.
 *
 * Outputs using little-endian ordering.
 *
 * \param fout  an open file handle.
 * \param src
 * \return  1 on success, 0 otherwise.
 */
size_t ExportRecordEntry(FILE *fout, const Record_Entry &src)
{
    uint16_t tempTag = src.tag;
    Swap16bit(tempTag);

    bool success = true &&
                   fwrite(&src.country, sizeof(src.country), 1, fout) &&
                   fwrite(&src.month, sizeof(src.month), 1, fout) &&
                   fwrite(&src.yr, sizeof(src.yr), 1, fout) &&
                   fwrite(&src.program, sizeof(src.program), 1, fout) &&
                   fwrite(&tempTag, sizeof(tempTag), 1, fout) &&
                   fwrite(&src.type, sizeof(src.type), 1, fout) &&
                   fwrite(&src.place, sizeof(src.place), 1, fout) &&
                   fwrite(&src.name[0], sizeof(src.name), 1, fout) &&
                   fwrite(&src.astro[0], sizeof(src.astro), 1, fout);

    return (success ? 1 : 0);
}
