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
/** \file future.c This is responsible for Future Mission planning screen.
 *
 */

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"
#include "display/legacy_surface.h"

#include "future.h"
#include <assert.h>
#include "Buzz_inc.h"
#include "draw.h"
#include "admin.h"
#include "crew.h"
#include "futbub.h"
#include "game_main.h"
#include "mc2.h"
#include "prest.h"
#include "mc.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "filesystem.h"

//Used to read steps from missStep.dat
FILE *MSteps;
char missStep[1024];
static inline char B_Mis(char x)
{
    return missStep[x] - 0x30;
}
/*missStep.dat is plain text, with:
Mission Number (2 first bytes of each line)
A Coded letter, each drawing a different line (1105-1127 for all possible letters)
Numbers following each letter, which are the parameters of the function
Each line must finish with a Z, so the game stops reading
Any other char is ignored, but it's easier to read for a human that way */


LOG_DEFAULT_CATEGORY(future)

/* The lock, status, and F1-F5 variables are all linked, representing
 * the condition of mission parameter settings. They represent:
 *   0: Mission Duration
 *   1: Docking status
 *   2: EVA status
 *   3: Lunar Module status
 *   4: Joint Mission status
 * F5 is an exception to the normal ordering, being tied to the mission
 * duration.
 * status[] tells the state of the mission parameter button.
 * lock[] tells the state of the lockout buttons.
 * F1-F4 are
 *   0: if the lockout button is not set
 *   1: if locked and the mission parameter is required
 *   2: if locked and the mission parameter is required to be absent.
 * F5 is
 *  -1: if locked and unmanned
 *   0: if the lockout button is not set
 * 1-6: if locked and the duration is set at 1-6.
 */
char MisType;
char status[5], F1, F2, F3, F4, F5, Pad;
bool lock[5]; // Record if the mission parameter settings are locked.
bool JointFlag, MarsFlag, JupiterFlag, SaturnFlag;
display::LegacySurface *vh;

struct StepInfo {
    int16_t x_cor;
    int16_t y_cor;
} StepBub[MAXBUB];

struct {
    char A;   /**< DOCKING */
    char B;   /**< EVA */
    char C;   /**< LEM */
    char D;   /**< JOINT */
    char E;     /**< MANNED/UNMANNED/Duration 0==unmanned 1-6==duration */
    char X;     /**< the type of mission for assign crew and hardware */
    char Z;   /**< A duration mission only */
} V[62];

extern int SEG;


void Load_FUT_BUT(void);
bool MarsInRange(unsigned int year, unsigned int season);
bool JupiterInRange(unsigned int year, unsigned int season);
bool SaturnInRange(unsigned int year, unsigned int season);
bool JointMissionOK(char plr, char pad);
void DrawFuture(char plr, int mis, char pad);
void ClearDisplay(void);
int GetMinus(char plr);
void SetParameters(void);
void DrawLocks(void);
void Toggle(int wh, int i);
void TogBox(int x, int y, int st);
void PianoKey(int X);
void draw_Pie(int s);
void PlaceRX(int s);
void ClearRX(int s);
int UpSearchRout(int num, char plr);
int DownSearchRout(int num, char plr);
void DurPri(int x) ;


void Load_FUT_BUT(void)
{
    FILE *fin;
    unsigned i;
    fin = sOpen("NFUTBUT.BUT", "rb", 0);
    i = fread(display::graphics.legacyScreen()->pixels(), 1, MAX_X * MAX_Y, fin);
    fclose(fin);
    RLED_img(display::graphics.legacyScreen()->pixels(), vh->pixels(), i, vh->width(), vh->height());
    return;
}

/* Is Mars at the right point in its orbit where a rocket launched at
 * the given time will be able to intercept it?
 */
bool MarsInRange(unsigned int year, unsigned int season)
{
    return ((year == 60 && season == 0) || (year == 62 && season == 0) ||
            (year == 64 && season == 0) || (year == 66 && season == 0) ||
            (year == 69 && season == 1) || (year == 71 && season == 1) ||
            (year == 73 && season == 1));
}

/* Is Jupiter at the right point in its orbit where a rocket launched
 * at the given time will be able to intercept it?
 */
bool JupiterInRange(unsigned int year, unsigned int season)
{
    return (year == 60 || year == 64 || year == 68 || year == 72 ||
            year == 73 || year == 77);
}

/* Is Saturn at the right point in its orbit where a rocket launched
 * at the given time will be able to intercept it?
 */
bool SaturnInRange(unsigned int year, unsigned int season)
{
    return (year == 61 || year == 66 || year == 72);
}

/* Is there room among the launch pads to schedule a joint mission,
 * with the given pad being the first part?
 *
 * \param plr  The player scheduling the launch
 * \param pad  0, 1, or 2 for the primary, secondary, or tertiary pad.
 * \return     True if a joint mission may be scheduled on the pad.
 */
bool JointMissionOK(char plr, char pad)
{
    return (pad >= 0 && pad < MAX_LAUNCHPADS - 2) &&
           (Data->P[plr].LaunchFacility[pad + 1] == 1) &&
           ((Data->P[plr].Future[pad + 1].MissionCode == Mission_None) ||
            (Data->P[plr].Future[pad + 1].part == 1));
}

/* Draws the entire Future Missions display, including the mission-
 * specific information. Used to initialize the mission selector
 * interface.
 *
 * This relies on the global buffer vh, which must have been created
 * prior. The future missions button art is loaded into vh by this
 * function.
 *
 * \param plr  The player scheduling the mission's design scheme.
 * \param mis  The mission type.
 * \param pad  0, 1, or 2 depending on which pad is being used.
 */
void DrawFuture(char plr, int mis, char pad)
{
    FadeOut(2, 10, 0, 0);
    Load_FUT_BUT();

    boost::shared_ptr<display::PalettizedSurface> planets(Filesystem::readImage("images/fmin.img.0.png"));
    planets->exportPalette();

    display::graphics.screen()->clear();

    gr_sync();

    if (MarsFlag == true) {
        display::graphics.screen()->draw(planets, 1, 1, 12, 11, 198, 153);
    }

    if (JupiterFlag == true) {
        display::graphics.screen()->draw(planets, 14, 1, 51, 54, 214, 130);
    }

    if (SaturnFlag == true) {
        display::graphics.screen()->draw(planets, 66, 1, 49, 53, 266, 135);
    }

    fill_rectangle(1, 1, 318, 21, 3);
    fill_rectangle(317, 22, 318, 198, 3);
    fill_rectangle(1, 197, 316, 198, 3);
    fill_rectangle(1, 22, 2, 196, 3);
    OutBox(0, 0, 319, 199);
    InBox(3, 3, 30, 19);
    InBox(3, 22, 316, 196);
    IOBox(242, 3, 315, 19);
    ShBox(5, 24, 183, 47);
    ShBox(5, 24, 201, 47); //name box
    ShBox(5, 74, 41, 82); // RESET
    ShBox(5, 49, 53, 72); //dur/man
    ShBox(43, 74, 53, 82);   // Duration lock
    ShBox(80, 74, 90, 82);   // Docking lock
    ShBox(117, 74, 127, 82); // EVA lock
    ShBox(154, 74, 164, 82); // LM lock
    ShBox(191, 74, 201, 82); // Joint mission lock
    ShBox(5, 84, 16, 130); //arrows up
    ShBox(5, 132, 16, 146); //middle box
    ShBox(5, 148, 16, 194); //    down
    ShBox(203, 24, 238, 31); // new right boxes

    // Mission penalty numerical display
    fill_rectangle(206, 36, 235, 44, 7);
    ShBox(203, 33, 238, 47);
    InBox(205, 35, 236, 45);

    // Mission scroll arrows
    draw_up_arrow(8, 95);
    draw_down_arrow(8, 157);

    // Display mission steps toggle
    vh->copyTo(display::graphics.legacyScreen(), 140, 5, 5, 132, 15, 146);

    // Draw the mission specification toggle buttons
    Toggle(5, 1);
    draw_Pie(0);
    OutBox(5, 49, 53, 72);
    Toggle(1, 1);
    TogBox(55, 49, 0);
    Toggle(2, 1);
    TogBox(92, 49, 0);
    Toggle(3, 1);
    TogBox(129, 49, 0);

    for (int i = 1; i < 4; i++) {
        if (status[i] != 0) {
            Toggle(i, 1);
        }
    };

    if (JointFlag == false) {
        F4 = 2;
        lock[4] = true;
        Toggle(4, 1);
        InBox(191, 74, 201, 82);
        PlaceRX(5);
        TogBox(166, 49, 1);
    } else {
        F4 = 0;
        lock[4] = false;
        status[4] = 0;
        Toggle(4, 1);
        OutBox(191, 74, 201, 82);
        ClearRX(5);
        TogBox(166, 49, 0);
    };

    gr_sync();

    Missions(plr, 8, 37, mis, 1);

    GetMinus(plr);

    display::graphics.setForegroundColor(5);

    /* lines of text are 1:8,30  2:8,37   3:8,44    */
    switch (pad) { // These used to say Pad 1, 2, 3  -Leon
    case 0:
        draw_string(8, 30, "PAD A:");
        break;

    case 1:
        draw_string(8, 30, "PAD B:");
        break;

    case 2:
        draw_string(8, 30, "PAD C:");
        break;
    }

    display::graphics.setForegroundColor(1);

    draw_string(9, 80, "RESET");
    draw_string(258, 13, "CONTINUE");

    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        draw_string(200, 9, "SPRING");
    } else {
        draw_string(205, 9, "FALL");
    }

    draw_string(206, 16, "19");
    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(1);
    draw_small_flag(plr, 4, 4);
    draw_heading(40, 5, "FUTURE MISSIONS", 0, -1);
    FadeIn(2, 10, 0, 0);

    return;
}

void ClearDisplay(void)
{
    boost::shared_ptr<display::PalettizedSurface> background(Filesystem::readImage("images/fmin.img.0.png"));

    display::graphics.screen()->draw(background, 202, 48, 40, 35, 202, 48);
    display::graphics.screen()->draw(background, 17, 83, 225, 103, 17, 83);
    display::graphics.screen()->draw(background, 242, 23, 74, 173, 242, 23);
    display::graphics.setForegroundColor(1);
    return;
}

int GetMinus(char plr)
{
    char i;
    int u;

    i = PrestMin(plr);
    fill_rectangle(206, 36, 235, 44, 7);

    if (i < 3) {
        u = 1;    //ok
    } else if (i < 9) {
        u = 10;    //caution
    } else {
        u = 19;    //danger
    }

    vh->copyTo(display::graphics.legacyScreen(), 203, u, 203, 24, 238, 31);
    display::graphics.setForegroundColor(11);

    if (i > 0) {
        draw_string(210, 42, "-");
    } else {
        grMoveTo(210, 42);
    }

    draw_number(0, 0, i);
    display::graphics.setForegroundColor(1);
    return 0;
}

void SetParameters(void)
{
    int i;
    FILE *fin;
    fin = sOpen("MISSION.DAT", "rb", 0);

    for (i = 0; i < 62; i++) {
        fread(&Mis, sizeof Mis, 1, fin);
        V[i].A = Mis.Doc;
        V[i].B = Mis.EVA;
        V[i].C = Mis.LM;
        V[i].D = Mis.Jt;
        V[i].E = Mis.Days;
        V[i].X = Mis.mCrew;
        V[i].Z = Mis.Dur;
    }

    fclose(fin);
    return;
}

void DrawLocks(void)
{
    int i;

    for (i = 0; i < 5; i++)
        if (lock[i] == true) {
            PlaceRX(i + 1);
        } else {
            ClearRX(i + 1);
        }

    return;
}


/* Draws the illustration on a mission parameter button. Each button
 * has two illustrations, one for the selected state and another for
 * the unselected state.
 *
 * The illustrations are stored in a buffer via the global pointer vh,
 * which reads the information in Load_FUT_BUT().
 *
 * \param wh the button
 * \param i in or out
 */
void Toggle(int wh, int i)
{
    TRACE3("->Toggle(wh %d, i %d)", wh, i);

    switch (wh) {
    case 1:
        if (i == 1) {
            vh->copyTo(display::graphics.legacyScreen(), 1, 21, 55, 49, 89, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 1, 56, 55, 49, 89, 81);
        }

        break;

    case 2:
        if (i == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 38, 21, 92, 49, 127, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 38, 56, 92, 49, 127, 81);
        }

        break;

    case 3:
        if (i == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 75, 21, 129, 49, 163, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 75, 56, 129, 49, 163, 81);
        }

        break;

    case 4:
        if (i == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 112, 21, 166, 49, 200, 81);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 112, 56, 166, 49, 200, 81);
        }

        break;

    case 5:
        if (i == 1)  {
            vh->copyTo(display::graphics.legacyScreen(), 153, 1, 5, 49, 52, 71);
        } else {
            vh->copyTo(display::graphics.legacyScreen(), 153, 26, 5, 49, 52, 71);
        }

        break;

    default:
        break;
    }

    TRACE1("<-Toggle()");
    return;
}

/* Draws a notched box outline for a mission parameter button.
 *
 * This is a custom form of the standard InBox/OutBox functions, using
 * the same color choices.
 *
 * \param x   The x coordinate of the upper-left corner.
 * \param y   The y coordinate of the upper-left corner.
 * \param st  1 if the button is depressed, 0 otherwise
 */
void TogBox(int x, int y, int st)
{
    TRACE4("->TogBox(x %d, y %d, st %d)", x, y, st);
    char sta[2][2] = {{2, 4}, {4, 2}};

    display::graphics.setForegroundColor(sta[st][0]);
    grMoveTo(0 + x, y + 32);
    grLineTo(0 + x, y + 0);
    grLineTo(34 + x, y + 0);
    display::graphics.setForegroundColor(sta[st][1]);
    grMoveTo(x + 0, y + 33);
    grLineTo(23 + x, y + 33);
    grLineTo(23 + x, y + 23);
    grLineTo(x + 35, y + 23);
    grLineTo(x + 35, y + 0);

    TRACE1("<-TogBox()");
    return;
}

/*
 * TODO: This seems to pass wrong PlaceRX values. However, it appears
 * the mistake is being covered up by the subsequent call to DrawLocks.
 */
void PianoKey(int X)
{
    TRACE2("->PianoKey(X %d)", X);
    int t;

    if (F1 == 0) {
        if (V[X].A == 1) {
            Toggle(1, 1);
            status[1] = 1;
        } else {
            Toggle(1, 0);
            PlaceRX(1);
            status[1] = 0;
        }
    }

    if (F2 == 0) {
        if (V[X].B == 1) {
            Toggle(2, 1);
            status[2] = 1;
        } else {
            Toggle(2, 0);
            PlaceRX(2);
            status[2] = 0;
        }
    }

    if (F3 == 0) {
        if (V[X].C == 1) {
            Toggle(3, 1);
            status[3] = 1;
        } else {
            Toggle(3, 0);
            PlaceRX(3);
            status[3] = 0;
        }
    }

    if (F4 == 0) {
        if (V[X].D == 1) {
            Toggle(4, 0);
            status[4] = 1;
        } else {
            Toggle(4, 1);
            status[4] = 0;
        }
    }

    if (F5 == -1 || (F5 == 0 && V[X].E == 0)) {
        Toggle(5, 0);
        status[0] = 0;
    } else {
        Toggle(5, 1);
        t = (F5 == 0) ? V[X].E : F5;
        assert(0 <= t);
        draw_Pie(t);
        status[0] = t;
    }

    DrawLocks();
    TRACE1("<-PianoKey()");
    return;
}

/* Draw a piechart with 0-6 pieces, filled in clockwise starting at the
 * top.
 *
 * \param s  How many slices are filled in on the piechart.
 */
void draw_Pie(int s)
{
    int off;

    if (s == 0) {
        off = 1;
    } else {
        off = s * 20;
    }

    vh->copyTo(display::graphics.legacyScreen(), off, 1, 7, 51, 25, 69);
    return;
}

/* Draws a restriction (lock) button in its active (restricted) state.
 * The restriction button indicates whether the linked mission parameter
 * setting may be modified.
 *
 * The button index is related to the global lock variable, but offset.
 *   1: Mission Duration
 *   2: Docking status
 *   3: EVA status
 *   4: Lunar Module status
 *   5: Joint Mission status
 *
 * \param s  The button index.
 */
void PlaceRX(int s)
{
    switch (s) {
    case 1:
        fill_rectangle(44, 75, 52, 81, 8);
        break;

    case 2:
        fill_rectangle(81, 75, 89, 81, 8);
        break;

    case 3:
        fill_rectangle(118, 75, 126, 81, 8);
        break;

    case 4:
        fill_rectangle(155, 75, 163, 81, 8);
        break;

    case 5:
        fill_rectangle(192, 75, 200, 81, 8);
        break;

    default:
        break;
    }

    return;
}

/* Draws a restriction (lock) button in its inactive (unrestricted)
 * state. The restriction button indicates whether the linked mission
 * parameter setting may be modified.
 *
 * The buttons are related to the global lock variable, but
 * offset.
 *   1: Mission Duration
 *   2: Docking status
 *   3: EVA status
 *   4: Lunar Module status
 *   5: Joint Mission status
 *
 * \param s  The button index.
 */
void ClearRX(int s)
{
    switch (s) {
    case 1:
        fill_rectangle(44, 75, 52, 81, 3);
        break;

    case 2:
        fill_rectangle(81, 75, 89, 81, 3);
        break;

    case 3:
        fill_rectangle(118, 75, 126, 81, 3);
        break;

    case 4:
        fill_rectangle(155, 75, 163, 81, 3);
        break;

    case 5:
        fill_rectangle(192, 75, 200, 81, 3);
        break;

    default:
        break;
    }

    return;
}

int UpSearchRout(int num, char plr)
{
    int found = 0, orig, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 1, c7 = 1, c8 = 1;
    orig = num;

    if (num >= 56 + plr) {
        num = 0;
    } else {
        num++;
    }

    while (found == 0) {
        c1 = 0;
        c2 = 0;
        c3 = 0;
        c4 = 0;
        c5 = 0;
        c6 = 1;
        c7 = 1;
        c8 = 1;

        if (F1 == V[num].A) {
            c1 = 1;    /* condition one is true */
        }

        if (F1 == 0 && V[num].A == 1) {
            c1 = 1;
        }

        if (F1 == 2 && V[num].A == 0) {
            c1 = 1;
        }

        if (F2 == V[num].B) {
            c2 = 1;    /* condition two is true */
        }

        if (F2 == 0 && V[num].B == 1) {
            c2 = 1;
        }

        if (F2 == 2 && V[num].B == 0) {
            c2 = 1;
        }

        if (F3 == V[num].C) {
            c3 = 1;    /* condition three is true */
        }

        if (F3 == 0 && V[num].C == 1) {
            c3 = 1;
        }

        if (F3 == 2 && V[num].C == 0) {
            c3 = 1;
        }

        if (F4 == V[num].D) {
            c4 = 1;    /* condition four is true */
        }

        if (F4 == 0 && V[num].D == 1) {
            c4 = 1;
        }

        if (F4 == 2 && V[num].D == 0) {
            c4 = 1;
        }

        if (num == 0) {
            c5 = 1;
        } else {
            if (F5 == -1 && V[num].Z == 0 && V[num].E == 0) {
                c5 = 1;
            }

            if (F5 == 0) {
                c5 = 1;
            }

            if (F5 > 1 && V[num].Z == 1) {
                c5 = 1;
            }

            if (F5 == V[num].E) {
                c5 = 1;
            }
        };

        if ((num == 32 || num == 36) && F5 == 2) {
            c5 = 0;
        }

        // planet check
        if (num == 10 && MarsFlag == false) {
            c6 = 0;
        }

        if (num == 12 && JupiterFlag == false) {
            c7 = 0;
        }

        if (num == 13 && SaturnFlag == false) {
            c8 = 0;
        }

        if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8)  {
            found = 1;
        }

        if (num == orig) {
            return 0;
        }

        if (found == 0) {
            if (num == 56 + plr) {
                num = 0;
            } else {
                ++num;
            }
        }

    }; /* end while */

    return num;
}

int DownSearchRout(int num, char plr)
{
    int found = 0, orig, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 1, c7 = 1, c8 = 1;
    orig = num;

    if (num <= 0) {
        num = 56 + plr;
    } else {
        --num;
    }

    while (found == 0) {
        c1 = 0;
        c2 = 0;
        c3 = 0;
        c4 = 0;
        c5 = 0;
        c6 = 1;
        c7 = 1;
        c8 = 1;

        if (F1 == V[num].A) {
            c1 = 1;
        }

        if (F1 == 0 && V[num].A == 1) {
            c1 = 1;    /* condition one is true */
        }

        if (F1 == 2 && V[num].A == 0) {
            c1 = 1;
        }

        if (F2 == V[num].B) {
            c2 = 1;    /* condition two is true */
        }

        if (F2 == 0 && V[num].B == 1) {
            c2 = 1;    /* condition one is true */
        }

        if (F2 == 2 && V[num].B == 0) {
            c2 = 1;
        }

        if (F3 == V[num].C) {
            c3 = 1;    /* condition three is true */
        }

        if (F3 == 0 && V[num].C == 1) {
            c3 = 1;    /* condition one is true */
        }

        if (F3 == 2 && V[num].C == 0) {
            c3 = 1;
        }

        if (F4 == V[num].D) {
            c4 = 1;    /* condition four is true */
        }

        if (F4 == 0 && V[num].D == 1) {
            c4 = 1;    /* condition one is true */
        }

        if (F4 == 2 && V[num].D == 0) {
            c4 = 1;
        }

        if (num == 0) {
            c5 = 1;
        } else {
            if (F5 == -1 && V[num].Z == 0 && V[num].E == 0) {
                c5 = 1;    // locked on zero duration
            }

            if (F5 == 0) {
                c5 = 1;    // nothing set
            }

            if (F5 > 1 && V[num].Z == 1) {
                c5 = 1;    // set duration with duration mission
            }

            if (F5 == V[num].E) {
                c5 = 1;    // the duration is equal to what is preset
            }
        };

        if ((num == 32 || num == 36) && F5 == 2) {
            c5 = 0;
        }

        // planet check
        if (num == 10 && MarsFlag == false) {
            c6 = 0;
        }

        if (num == 12 && JupiterFlag == false) {
            c7 = 0;
        }

        if (num == 13 && SaturnFlag == false) {
            c8 = 0;
        }

        if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8)  {
            found = 1;
        }

        if (num == orig) {
            return 0;
        }

        if (found == 0) {
            if (num == 0) {
                num = 56 + plr;
            } else {
                --num;
            }
        }

    }; /* end while */

    return num;
}

/* The main control loop for the Future Missions feature.
 */
void Future(char plr)
{
    /** \todo the whole Future()-function is 500 >lines and unreadable */
    TRACE1("->Future(plr)");
    int MisNum = 0, DuraType = 0, MaxDur = 6;
    int setting = -1, prev_setting = -1;

    display::LegacySurface local(166, 9);
    display::LegacySurface local2(177, 197);
    vh = new display::LegacySurface(240, 90);

    unsigned int year = Data->Year;
    unsigned int season = Data->Season;
    TRACE3("--- Setting year=Year (%d), season=Season (%d)", year, season);

    SetParameters();
    MarsFlag = MarsInRange(year, season);
    JupiterFlag = JupiterInRange(year, season);
    SaturnFlag = SaturnInRange(year, season);

    while ((MisNum = FutureCheck(plr, 0)) != 5) {
        F1 = F2 = F3 = F4 = F5 = 0;

        for (int i = 0; i < 5; i++) {
            lock[i] = false;
            status[i] = 0;
        }

        keyHelpText = "k011";
        helpText = "i011";
        Pad = MisNum;
        DuraType = 0;
        MisType = 0;
        ClrFut(plr, MisNum);

        JointFlag = JointMissionOK(plr, MisNum); // initialize joint flag

        DrawFuture(plr, MisType, MisNum);

        while (1) {
            key = 0;
            GetMouse();

            prev_setting = setting;
            setting = -1;

            // SEG determines the number of control points used in creating
            // the B-splines for drawing the mission flight path.
            // The more control points, the smoother the path should
            // appear.
            if (key == '-' && SEG > 1) {
                SEG--;
            } else if (key == '+' && SEG < 500) {
                SEG++;
            } else if (key >= 65 && key < Bub_Count + 65) {
                setting = key - 65;
            }

            // If the mouse is over one of the Mission Step bubbles,
            // display the step information.
            for (int i = 0; i < Bub_Count; i++) {
                if (x >= StepBub[i].x_cor && x <= StepBub[i].x_cor + 7 &&
                    y >= StepBub[i].y_cor && y <= StepBub[i].y_cor + 7) {
                    setting = i;
                    break;
                }
            }

            if (setting >= 0) {
                if (prev_setting < 0) {
                    local.copyFrom(display::graphics.legacyScreen(), 18, 186, 183, 194);
                }

                if (prev_setting != setting) {
                    ShBox(18, 186, 183, 194);
                    display::graphics.setForegroundColor(1);
                    MisStep(21, 192, Mev[setting].loc);
                }
            } else if (setting < 0 && prev_setting >= 0) {
                local.copyTo(display::graphics.legacyScreen(), 18, 186);
            }

            if (Mis.Dur <= V[MisType].E &&
                ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) ||
                 key == K_ENTER)) {
                InBox(244, 5, 313, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(300);
                }

                key = 0;
                OutBox(244, 5, 313, 17);

                // Copy the screen contents to a buffer. If the mission
                // requires a capsule to be assigned, a pop-up will be
                // created listing the options. Once the pop-up is
                // dismissed the screen may be redrawn from the buffer.
                local2.copyFrom(display::graphics.legacyScreen(), 74, 3, 250, 199);
                int NewType = V[MisType].X;
                Data->P[plr].Future[MisNum].Duration = DuraType;

                int Ok = HardCrewAssign(plr, MisNum, MisType, NewType);

                local2.copyTo(display::graphics.legacyScreen(), 74, 3);

                if (Ok == 1) {
                    Data->P[plr].Future[MisNum].Duration = DuraType;
                    break;        // return to launchpad loop
                } else {
                    ClrFut(plr, MisNum);
                    continue;
                }
            } else if ((x >= 43 && y >= 74 && x <= 53 && y <= 82 && mousebuttons > 0) ||
                       key == '!') { // Duration restriction lock
                lock[0] = (! lock[0]);

                if (lock[0] == true) {
                    InBox(43, 74, 53, 82);
                    PlaceRX(1);
                    F5 = (status[0] == 0) ? -1 : status[0];
                } else {
                    OutBox(43, 74, 53, 82);
                    ClearRX(1);
                    F5 = status[0] = 0;
                }

                WaitForMouseUp();

            } else if (lock[0] != true &&
                       ((x >= 5 && y >= 49 && x <= 53 && y <= 72 && mousebuttons > 0) ||
                        key == '1')) { // Duration toggle
                InBox(5, 49, 53, 72);

                if (DuraType == MaxDur) {
                    DuraType = 0;
                } else {
                    DuraType++;
                }

                Data->P[plr].Future[MisNum].Duration = DuraType;

                if (DuraType == 0) {
                    Toggle(5, 0);
                } else if (DuraType == 1) {
                    Toggle(5, 1);
                }

                if (DuraType != 0) {
                    draw_Pie(DuraType);
                }

                status[0] = DuraType;

                WaitForMouseUp();

                display::graphics.setForegroundColor(34);
                OutBox(5, 49, 53, 72);
            } else if ((x >= 5 && y >= 74 && x <= 41 && y <= 82 && mousebuttons > 0) ||
                       (key == K_ESCAPE)) { // Reset mission selection
                InBox(5, 74, 41, 82);

                WaitForMouseUp();

                MisType = 0;

                if (DuraType != 0) {
                    Toggle(5, 0);
                }

                DuraType = F1 = F2 = F3 = F4 = F5 = 0;

                for (int i = 1; i < 4; i++) {
                    if (status[i] != 0) {
                        Toggle(i, 1);
                    }
                }

                if (JointFlag == false) {
                    F4 = 2;
                    lock[4] = true;
                    Toggle(4, 1);
                    InBox(191, 74, 201, 82);
                    PlaceRX(5);
                    TogBox(166, 49, 1);
                } else {
                    F4 = 0;
                    lock[4] = false;
                    status[4] = 0;
                    Toggle(4, 1);
                    OutBox(191, 74, 201, 82);
                    ClearRX(5);
                    TogBox(166, 49, 0);
                }

                for (int i = 0; i < 4; i++) {
                    lock[i] = false;
                    status[i] = 0;
                }

                OutBox(5, 49, 53, 72);
                OutBox(43, 74, 53, 82);
                TogBox(55, 49, 0);
                OutBox(80, 74, 90, 82);
                TogBox(92, 49, 0);
                OutBox(117, 74, 127, 82);
                TogBox(129, 49, 0);
                OutBox(154, 74, 164, 82);

                ClrFut(plr, MisNum);
                Missions(plr, 8, 37, MisType, 1);
                GetMinus(plr);
                OutBox(5, 74, 41, 82);

            } else if ((x >= 80 && y >= 74 && x <= 90 && y <= 82 && mousebuttons > 0) ||
                       key == '@') { // Docking restriction lock
                lock[1] = (! lock[1]);

                if (lock[1] == true) {
                    InBox(80, 74, 90, 82);
                    PlaceRX(2);
                } else {
                    OutBox(80, 74, 90, 82);
                    ClearRX(2);
                }

                if ((status[1] == 0) && (lock[1] == true)) {
                    F1 = 2;
                } else if ((status[1] == 1) && (lock[1] == true)) {
                    F1 = 1;
                } else {
                    F1 = 0;
                }

                WaitForMouseUp();

            } else if (lock[1] == false &&
                       (((x >= 55 && y >= 49 && x <= 90 && y <= 82) && mousebuttons > 0) ||
                        key == '2')) { // Docking toggle
                TogBox(55, 49, 1);

                if (status[1] == 0) {
                    Toggle(1, 1);
                } else {
                    Toggle(1, 0);
                }

                status[1] = abs(status[1] - 1);
                WaitForMouseUp();
                TogBox(55, 49, 0);

            } else if ((x >= 117 && y >= 74 && x <= 127 && y <= 82 && mousebuttons > 0) ||
                       key == '#') { // EVA Restriction button
                lock[2] = (! lock[2]);

                if (lock[2] == true) {
                    InBox(117, 74, 127, 82);
                    PlaceRX(3);
                } else {
                    OutBox(117, 74, 127, 82);
                    ClearRX(3);
                }

                if ((status[2] == 0) && (lock[2] == true)) {
                    F2 = 2;
                } else if ((status[2] == 1) && (lock[2] == true)) {
                    F2 = 1;
                } else {
                    F2 = 0;
                }

                WaitForMouseUp();

            } else if (lock[2] == false &&
                       ((x >= 92 && y >= 49 && x <= 127 && y <= 82 && mousebuttons > 0) ||
                        key == '3')) { // EVA toggle
                TogBox(92, 49, 1);

                if (status[2] == 0) {
                    Toggle(2, 1);
                } else {
                    Toggle(2, 0);
                }

                status[2] = abs(status[2] - 1);
                WaitForMouseUp();
                TogBox(92, 49, 0);

            } else if ((x >= 154 && y >= 74 && x <= 164 && y <= 82 && mousebuttons > 0) ||
                       key == '$') { // Lunar Module Restriction button
                lock[3] = (! lock[3]);

                if (lock[3] == true) {
                    InBox(154, 74, 164, 82);
                    PlaceRX(4);
                } else {
                    OutBox(154, 74, 164, 82);
                    ClearRX(4);
                }

                if ((status[3] == 0) && (lock[3] == true)) {
                    F3 = 2;
                } else if ((status[3] == 1) && (lock[3] == true)) {
                    F3 = 1;
                } else {
                    F3 = 0;
                }

                WaitForMouseUp();

            } else if (lock[3] == false &&
                       ((x >= 129 && y >= 49 && x <= 164 && y <= 82 && mousebuttons > 0) ||
                        key == '4')) { // LEM toggle
                TogBox(129, 49, 1);

                if (status[3] == 0) {
                    Toggle(3, 1);
                } else {
                    Toggle(3, 0);
                }

                status[3] = abs(status[3] - 1);
                WaitForMouseUp();
                TogBox(129, 49, 0);

            } else if (JointFlag == true &&
                       ((x > 191 && y >= 74 && x <= 201 && y <= 82 && mousebuttons > 0) ||
                        key == '%')) { // Joint Mission Restriction button
                lock[4] = (! lock[4]);

                if (lock[4] == true) {
                    InBox(191, 74, 201, 82);
                    PlaceRX(5);
                } else {
                    OutBox(191, 74, 201, 82);
                    ClearRX(5);
                }

                if ((status[4] == 0) && (lock[4] == true)) {
                    F4 = 2;
                } else if ((status[4] == 1) && (lock[4] == true)) {
                    F4 = 1;
                } else {
                    F4 = 0;
                }

                WaitForMouseUp();

            } else if (lock[4] == false && JointFlag == true &&
                       ((x >= 166 && y >= 49 && x <= 201 && y <= 82 && mousebuttons > 0) ||
                        key == '5')) { // Joint Mission
                TogBox(166, 49, 1);
                status[4] = abs(status[4] - 1);

                if (status[4] == 0) {
                    Toggle(4, 1);
                } else {
                    Toggle(4, 0);
                }

                WaitForMouseUp();
                TogBox(166, 49, 0);

            } else if ((x >= 5 && y >= 84 && x <= 16 && y <= 130 && mousebuttons > 0) ||
                       (key == UP_ARROW)) {
                // Scroll up among Mission Types
                InBox(5, 84, 16, 130);

                for (int i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {
                        MisType = UpSearchRout(MisType, plr);
                        Data->P[plr].Future[MisNum].MissionCode = MisType;
                        i = 51;
                    }
                }

                // Keep scrolling while mouse/key is held down.
                while (mousebuttons == 1 || key == UP_ARROW) {
                    MisType = UpSearchRout(MisType, plr);
                    Data->P[plr].Future[MisNum].MissionCode = MisType;
                    Missions(plr, 8, 37, MisType, 3);
                    delay(100);
                    key = 0;
                    GetMouse();
                }

                Missions(plr, 8, 37, MisType, 3);
                DuraType = status[0];
                OutBox(5, 84, 16, 130);
            } else if ((x >= 5 && y >= 132 && x < 16 && y <= 146 && mousebuttons > 0) ||
                       (key == K_SPACE)) {
                // Turn on Mission Steps display
                InBox(5, 132, 16, 146);
                WaitForMouseUp();
                delay(50);
                MisType = Data->P[plr].Future[MisNum].MissionCode;
                assert(0 <= MisType);

                if (MisType != 0) {
                    Missions(plr, 8, 37, MisType, 1);
                } else {
                    Missions(plr, 8, 37, MisType, 3);
                }

                OutBox(5, 132, 16, 146);
            } else if ((x >= 5 && y >= 148 && x <= 16 && y <= 194 && mousebuttons > 0) ||
                       (key == DN_ARROW)) {
                // Scroll down among Mission Types
                InBox(5, 148, 16, 194);

                for (int i = 0; i < 50; i++) {
                    key = 0;
                    GetMouse();
                    delay(10);

                    if (mousebuttons == 0) {
                        MisType = DownSearchRout(MisType, plr);
                        Data->P[plr].Future[MisNum].MissionCode = MisType;
                        i = 51;
                    }

                }

                // Keep scrolling while mouse/key is held down.
                while (mousebuttons == 1 || key == DN_ARROW) {
                    MisType = DownSearchRout(MisType, plr);
                    Data->P[plr].Future[MisNum].MissionCode = MisType;
                    Missions(plr, 8, 37, MisType, 3);
                    delay(100);
                    key = 0;
                    GetMouse();
                }

                Missions(plr, 8, 37, MisType, 3);
                DuraType = status[0];
                OutBox(5, 148, 16, 194);
            }
        }                              // Input while loop
    }                              // Launch pad selection loop

    delete vh;
    vh = NULL;
    TRACE1("<-Future()");
}

/** draws the bubble on the screen,
 * starts with upper left coor
 *
 * \param x x-coord of the upper left corner of the bubble
 * \param y y-coord of the upper left corner of the bubble
 */
void Bd(int x, int y)
{
    int x1, y1, x2, y2;
    x1 = x - 2;
    y1 = y;
    x2 = x - 1;
    y2 = y - 1;
    fill_rectangle(x1, y1, x1 + 8, y1 + 4, 21);
    fill_rectangle(x2, y2, x2 + 6, y2 + 6, 21);
    display::graphics.setForegroundColor(1);
    grMoveTo(x, y + 4);
    /** \note references Bub_Count to determine the number of the character to draw in the bubble */
    draw_character(65 + Bub_Count);
    StepBub[Bub_Count].x_cor = x1;
    StepBub[Bub_Count].y_cor = y1;
    ++Bub_Count;
    return;
}

/** Print the duration of a mission
 *
 * \param x duration code
 *
 * \todo Link this at whatever place the duration is actually defined
 */
void DurPri(int x)
{
    display::graphics.setForegroundColor(5);

    switch (x) {
    case -1:
        draw_string(112, 30, "NO DURATION");
        break;

    case 0:
        draw_string(112, 30, "NO DURATION");
        break;

    case 1:
        draw_string(112, 30, "1 - 2 DAYS");
        break;

    case 2:
        draw_string(112, 30, "3 - 5 DAYS");
        break;

    case 3:
        draw_string(112, 30, "6 - 7 DAYS");
        break;

    case 4:
        draw_string(112, 30, "8 - 12 DAYS");
        break;

    case 5:
        draw_string(112, 30, "13 - 16 DAYS");
        break;

    case 6:
        draw_string(112, 30, "17 - 20 DAYS");
        break;
    }

    return;
}

void MissionName(int val, int xx, int yy, int len)
{
    TRACE5("->MissionName(val %d, xx %d, yy %d, len %d)", val, xx, yy, len);
    int i, j = 0;

    GetMisType(val);

    grMoveTo(xx, yy);

    for (i = 0; i < 50; i++) {
        if (j > len && Mis.Name[i] == ' ') {
            yy += 7;
            j = 0;
            grMoveTo(xx, yy);
        } else {
            draw_character(Mis.Name[i]);
        }

        j++;

        if (Mis.Name[i] == '\0') {
            break;
        }
    };

    TRACE1("<-MissionName");

    return;
}

/** Missions() will draw the future missions among other things
 *
 * \param plr Player
 * \param X screen coord for mission name string
 * \param Y screen coord for mission name string
 * \param val mission number
 * \param bub if set to 0 or 3 the function will not draw stuff
 */
void Missions(char plr, int X, int Y, int val, char bub)
{
    TRACE5("->Missions(plr, X %d, Y %d, val %d, bub %c)", X, Y, val, bub);

    memset(Mev, 0x00, sizeof Mev);

    if (bub == 1 || bub == 3) {
        PianoKey(val);
        Bub_Count = 0; // set the initial bub_count
        ClearDisplay();
        fill_rectangle(6, 31, 182, 46, 3);
        fill_rectangle(80, 25, 175, 30, 3);
        display::graphics.setForegroundColor(5);
        draw_string(55, 30, "TYPE: ");
        draw_number(0, 0, val);
        display::graphics.setForegroundColor(5);

        if (V[val].E > 0) {
            if (F5 > V[val].E && V[val].Z == 1) {
                DurPri(F5);
            } else {
                DurPri(V[val].E);
            }
        } else {
            DurPri(F5);
        }
    } else {
        display::graphics.setForegroundColor(1);
    }

    MissionName(val, X, Y, 24);

    if (bub == 3) {
        GetMinus(plr);
    }

    if (bub == 0 || bub == 3) {
        return;
    }


    MSteps = sOpen("missSteps.dat", "r", FT_DATA);

    if (fgets(missStep, 1024, MSteps) == NULL) {
        memset(missStep, 0, sizeof missStep);
    }

    while (!feof(MSteps) && ((missStep[0] - 0x30) * 10 + (missStep[1] - 0x30)) != val) {
        if (fgets(missStep, 1024, MSteps) == NULL) {
            break;
        }
    }

    fclose(MSteps);

    int n;

    for (n = 2; missStep[n] != 'Z'; n++)
        switch (missStep[n]) {
        case 'A':
            Draw_IJ(B_Mis(++n));
            break;

        case 'B':
            Draw_IJV(B_Mis(++n));
            break;

        case 'C':
            OrbOut(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'D':
            LefEarth(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'E':
            OrbIn(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'F':
            OrbMid(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'G':
            LefOrb(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'H':
            Draw_LowS(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4), B_Mis(n + 5), B_Mis(n + 6));
            n += 6;
            break;

        case 'I':
            Fly_By();
            break;

        case 'J':
            VenMarMerc(B_Mis(++n));
            break;

        case 'K':
            Draw_PQR();
            break;

        case 'L':
            Draw_PST();
            break;

        case 'M':
            Draw_GH(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'N':
            Q_Patch();
            break;

        case 'O':
            RghtMoon(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'P':
            DrawLunPas(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'Q':
            DrawLefMoon(B_Mis(n + 1), B_Mis(n + 2));
            n += 2;
            break;

        case 'R':
            DrawSTUV(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4));
            n += 4;
            break;

        case 'S':
            Draw_HighS(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3));
            n += 3;
            break;

        case 'T':
            DrawMoon(B_Mis(n + 1), B_Mis(n + 2), B_Mis(n + 3), B_Mis(n + 4), B_Mis(n + 5), B_Mis(n + 6), B_Mis(n + 7));
            n += 7;
            break;

        case 'U':
            LefGap(B_Mis(++n));
            break;

        case 'V':
            S_Patch(B_Mis(++n));
            break;

        case 'W':
            DrawZ();
            break;

        default :
            break;
        }

    gr_sync();
    MissionCodes(plr, MisType, Pad);
    TRACE1("<-Missions()");
}  // end function missions

/* vim: set noet ts=4 sw=4 tw=77: */
