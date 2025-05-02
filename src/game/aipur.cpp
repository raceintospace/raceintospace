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
/** \file aipur.c AI Purchasing Routines
 */

// This file handles AI purchase and research of hardware, and 'naut recruitment.  It also shows the STATS part of the endgame.

#include "aipur.h"

#include "display/graphics.h"
#include "display/palettized_surface.h"

#include "Buzz_inc.h"
#include "data.h"
#include "draw.h"
#include "game_main.h"
#include "gr.h"
#include "hardef.h"
#include "museum.h"
#include "options.h"   //Naut Randomize && Naut Compatibility, Nikakd, 10/8/10
#include "pace.h"
#include "port.h"
#include "rdplex.h"
#include "records.h"
#include "sdlhelper.h"
#include "serialize.h"

std::vector<struct ManPool> Men;
std::vector<std::vector<int8_t>> portbuttons;//(7, std::vector<int8_t>(570));
  //std::vector<std::vector<int8_t>> portbuttons = {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};
uint8_t AIsel[25];


void DrawStatistics(char Win);
void SelectBest(char plr, int pos);
char Skill(char plr, char type);
void CheckAdv(char plr);


/**
 * Draws the menu for pulling up assorted summary statistics about the
 * finished match.
 *
 * \param Win  the country of the winner (0 for USA, 1 for USSR).
 */
void DrawStatistics(char Win)
{
    char AImg[7] = {8, 9, 10, 11, 13, 14, 0};
    int starty, qty, i;
    helpText = "i145";
    keyHelpText = "k045";
    FadeOut(2, 10, 0, 0);
    PortPal(0);

    display::graphics.screen()->clear();
    ShBox(35, 33, 288, 159);
    InBox(40, 69, 111, 109);
    InBox(116, 69, 283, 109);
    InBox(40, 114, 111, 154);
    InBox(116, 114, 283, 154);
    IOBox(191, 40, 280, 62);
    draw_flag(41, 70, Win);
    draw_flag(41, 115, other(Win));
    draw_heading(47, 44, "STATISTICS", 1, -1);
    draw_heading(216, 45, "EXIT", 1, -1);
    display::graphics.setForegroundColor(6);
    draw_string(122, 76, "WINNING PLAYER: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, &Data->P[Win].Name[0]);

    //if (AI[Win]) {
    //    draw_string(0, 0, " (AI)");
    //}

    display::graphics.setForegroundColor(6);
    draw_string(122, 125, "LOSING PLAYER: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, &Data->P[other(Win)].Name[0]);

    if (AI[other(Win)]) {
        draw_string(0, 0, " (AI)");
    }

    // There's only one AI player, so there's only one place where the
    // AI strategy is listed.
    if (AI[Win] || AI[other(Win)]) {
        display::graphics.setForegroundColor(6);
        draw_string(122, 84, "AI STRATEGY: ");
        display::graphics.setForegroundColor(8);

        int ai_player = AI[Win] ? Win : other(Win);
        int strat = Data->P[ai_player].AIStrategy[AI_STRATEGY];

        // This displays the computer strategy: 1, 2, or 2
        // Per Michael, AI_STRATEGY represents
        //   "More the path rather than the craft"
        draw_number(0, 0, strat);
    }

    qty = 6;
    starty = 118;
    display::LegacySurface local(30, 19);
    
    DESERIALIZE_JSON_FILE(&portbuttons, locate_file("portbut.json", FT_DATA));
    
    OutBox(152, 41, 183, 61); // directors ranking

    for (i = 0; i < qty; i++) {
        if (i <= 4 && AI[Win] == 0) {
            OutBox(starty + (i * 33), 87, 31 + starty + (i * 33), 107);
        }

        if (i <= 4 && AI[other(Win)] == 0) {
            OutBox(starty + (i * 33), 132, 31 + starty + (i * 33), 152);
        }

        memcpy(local.pixels(), portbuttons[i].data(), 570);
        if (memcmp(local.pixels(), portbuttons[i].data(), 570) != 0) {
            throw std::runtime_error("Error: portbuttons data not copied in local surface.");
        }

        if (i == 0) {
            local.copyTo(display::graphics.legacyScreen(), 153, 42);
        } else {
            if (AI[Win] == 0) {
                local.copyTo(display::graphics.legacyScreen(), starty + ((i - 1) * 33) + 1, 88);
            }

            if (AI[other(Win)] == 0) {
                local.copyTo(display::graphics.legacyScreen(), starty + ((i - 1) * 33) + 1, 133);
            }
        }
    }

    FadeIn(2, 10, 0, 0);

    return;
}

void Stat(char Win)
{
    int j, i, starty;
    DrawStatistics(Win);
    WaitForMouseUp();
    i = 0;
    key = 0;
    starty = 118;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 193 && y >= 42 && x <= 278 && y <= 60 && mousebuttons > 0) || key == K_ENTER) {
            InBox(193, 42, 278, 60);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(193, 42, 278, 60);
        } else if ((x >= 152 && y >= 41 && x <= 183 && y <= 61 && mousebuttons > 0) || key == 'D') {
            InBox(152, 41, 183, 61);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            RankMe(Win);
            DrawStatistics(Win);
            key = 0;
            i = 0;
            OutBox(152, 41, 183, 61);
        };

        for (j = 0; j < 5; j++) {
            if (AI[Win] == 0) {
                if ((x >= starty + (j * 33) && y >= 87 && x <= 31 + starty + (j * 33) && y <= 107 && mousebuttons > 0) || ((key >= '1' && key <= '4') || key == '0')) {
                    if (key > 0) {
                        switch (key) {
                        case '0':
                            j = 0;
                            break;

                        case '1':
                            j = 1;
                            break;

                        case '2':
                            j = 2;
                            break;

                        case '3':
                            j = 3;
                            break;

                        case '4':
                            j = 4;
                            break;

                        default:
                            break;
                        }
                    }

                    InBox(starty + (j * 33), 87, 31 + starty + (j * 33), 107);
                    WaitForMouseUp();
                    key = 0;
                    keyHelpText = "k999";

                    switch (j) {
                    case 0:
                        helpText = "i130";
                        keyHelpText = "k031";

                        if (Option == -1 || (Option == Win)) {
                            ShowSpHist(Win);
                        }

                        //ShowSpHist(Win);
                        break;

                    case 1:
                        helpText = "i131";
                        keyHelpText = "k321";
                        Records(Win);
                        break;

                    case 2:
                        helpText = "i132";
                        keyHelpText = "k033";
                        ShowPrest(Win);
                        break;

                    case 3:
                        helpText = "i034";
                        ShowHard(Win);
                        break;

                    case 4:
                        helpText = (Win == 0) ? "i133" : "i134";
                        keyHelpText = (Win == 0) ? "k035" : "k441";

                        if (Data->P[Win].AstroCount > 0)
                            if (Option == -1 || Option == Win) {
                                ShowAstrosHist(Win);
                            }

                    default:
                        break;
                    }

                    helpText = "i000";
                    keyHelpText = "k000";
                    DrawStatistics(Win);
                    key = 0;
                    i = 0;
                    OutBox(starty + (j * 33), 87, 31 + starty + (j * 33), 107);
                }
            }  // matches AI[Win]==0
        }

        for (j = 0; j < 5; j++) {
            if (AI[other(Win)] == 0) {
                if ((x >= starty + (j * 33) && y >= 132 && x <= 31 + starty + (j * 33) && y <= 152 && mousebuttons > 0) || (key >= '5' && key <= '9')) {
                    if (key > 0) {
                        switch (key) {
                        case '5':
                            j = 0;
                            break;

                        case '6':
                            j = 1;
                            break;

                        case '7':
                            j = 2;
                            break;

                        case '8':
                            j = 3;
                            break;

                        case '9':
                            j = 4;
                            break;

                        default:
                            break;
                        }
                    }

                    InBox(starty + (j * 33), 132, 31 + starty + (j * 33), 152);
                    WaitForMouseUp();
                    key = 0;

                    switch (j) {
                    case 0:
                        helpText = "i130";
                        keyHelpText = "k031";

                        if (Option == -1 || Option == other(Win)) {
                            ShowSpHist(other(Win));
                        }

                        //ShowSpHist(other(Win));
                        break;

                    case 1:
                        helpText = "i131";
                        keyHelpText = "k321";
                        Records(other(Win));
                        break;

                    case 2:
                        helpText = "i132";
                        keyHelpText = "k033";
                        ShowPrest(other(Win));
                        break;

                    case 3:
                        helpText = "i034";
                        keyHelpText = "k999";
                        ShowHard(other(Win));
                        break;

                    case 4:
                        helpText = (Win == 0) ? "i133" : "i134";
                        keyHelpText = (Win == 0) ? "k035" : "k441";

                        if (Data->P[other(Win)].AstroCount > 0)
                            if (Option == -1 || Option == other(Win)) {
                                ShowAstrosHist(other(Win));
                            }

                        break;

                    default:
                        break;
                    }

                    helpText = "i000";
                    keyHelpText = "k000";
                    DrawStatistics(Win);
                    key = 0;
                    i = 0;
                    OutBox(starty + (j * 33), 132, 31 + starty + (j * 33), 152);
                }
            }
        }
    }

    return;
}

/** AI Wants to purchase Astronauts
 */
void AIAstroPur(char plr)
{
    int cost;
    int astrosInPool = 0;
    struct BuzzData *pData = &Data->P[plr];

    if (pData->AstroLevel == 0) {
        cost = 20;
    } else {
        cost = 15;
    }

    // Player has no cash, no astronauts
    if (cost > pData->Cash) {
        return;
    }

    switch (pData->AstroLevel) {
    case 0:
        astrosInPool = ASTRO_POOL_LVL1;
        break;

    case 1:
        astrosInPool = ASTRO_POOL_LVL2;
        break;

    case 2:
        astrosInPool = ASTRO_POOL_LVL3;
        break;

    case 3:
        astrosInPool = ASTRO_POOL_LVL4;
        break;

    case 4:
        astrosInPool = ASTRO_POOL_LVL5;
        break;
    }

    // Select best astronauts out of number of positions to fill
    SelectBest(plr, astrosInPool);
}


//Naut Randomize, Nikakd, 10/8/10
void AIRandomizeNauts()
{
    int i;

    for (i = 0; i < Men.size(); i++) {
        Men[i].Cap = brandom(5);
        Men[i].LM  = brandom(5);
        Men[i].EVA = brandom(5);
        Men[i].Docking = brandom(5);
        Men[i].Endurance = brandom(5);
    }
}


/**
 * Select the best astronauts from the next class for recruitment.
 *
 * How the AI handles female astronauts/cosmonauts is a configurable
 * feature, depending on options.feat_female_nauts. Options are
 *   0: Classic mode (event-driven)
 *   1: Always for players, Game decides AI behavior
 * Greater specificity may be available, but is not guaranteed. Under
 * the current setup
 *   2: AI requires news event
 *   3: AI acts as if under news event
 *   4: AI is gender-blind [default]
 * Leon has suggested default behavior be based on AI astronaut
 * difficulty.
 *
 * Also noteworthy is that AI nauts don't suffer skill loss upon
 * selection. This is to skip the difficulty of managing them in
 * Basic Training - from which they are automatically withdrawn -
 * and therefore cannot benefit from.
 */
void SelectBest(char plr, int pos)
{
    int count = 0, now, MaxMen = 0, Index, AIMaxSel = 0, i, j;
    FILE *fin;
    char tot, done;
    struct BuzzData *pData = &Data->P[plr];

    // pData->FemaleAstronautsAllowed is the news event flag that
    // allows & requires female astronauts.
    // The configurable option should not affect its value.
    bool femaleAstronautsAllowed =
        pData->FemaleAstronautsAllowed == 1 ||
        (options.feat_female_nauts > 0 && options.feat_female_nauts != 2);
    bool femaleAstronautsRequired =
        pData->FemaleAstronautsAllowed == 1 ||
        options.feat_female_nauts == 3;

    for (i = 0; i < 25; i++) {
        AIsel[i] = 0;
    }

    memset(buffer, 0x00, 5000);
    DESERIALIZE_JSON_FILE(&Men, locate_file("roster.json", FT_SAVE));

    if (options.feat_random_nauts == 1) {
        AIRandomizeNauts();    //Naut Randomize, Nikakd, 10/8/10
    }

    switch (pData->AstroLevel) {
    case 0:
        MaxMen = femaleAstronautsAllowed ? 13 : 10;
        AIMaxSel = ASTRO_POOL_LVL1;
        Index = 0;
        break;

    case 1:
        MaxMen = femaleAstronautsAllowed ? 20 : 17;
        AIMaxSel = ASTRO_POOL_LVL2;
        Index = 14;
        break;

    case 2:
        MaxMen = femaleAstronautsAllowed ? 22 : 19;
        AIMaxSel = ASTRO_POOL_LVL3;
        Index = 35;
        break;

    case 3:
        MaxMen = 27;
        AIMaxSel = ASTRO_POOL_LVL4;
        Index = 58;
        break;

    case 4:
        MaxMen = 19;
        AIMaxSel = ASTRO_POOL_LVL5;
        Index = 86;
        break;

    default:
        // TODO: Log an error...
        MaxMen = 0;
        AIMaxSel = 0;
        Index = 0;
        break;
    }

    Index += plr * Men.size() / 2;

    now = Index;
    count = 0;

    // TODO: This is a crude way of ordering all the candidates.
    // It could be replaced with a superior method.
    for (i = 16; i > 0; i--) {
        done = 0;

        while (count <= AIMaxSel && done == 0) {
            for (j = now; j < now + MaxMen + 1; j++) {
                tot = Men[j].Cap + Men[j].LM + Men[j].EVA + Men[j].Docking;

                if (i == tot) {
                    AIsel[count++] = j;
                } else if (femaleAstronautsRequired && Men[j].Sex == 1) {
                    AIsel[count++] = j;
                }
            }

            done = 1;
        }
    };

    for (i = 0; i < AIMaxSel; i++) {
        strcpy(&pData->Pool[i + pData->AstroCount].Name[0], &Men[AIsel[i]].Name[0]);
        pData->Pool[i + pData->AstroCount].Sex = Men[AIsel[i]].Sex;
        pData->Pool[i + pData->AstroCount].Cap = Men[AIsel[i]].Cap;
        pData->Pool[i + pData->AstroCount].LM = Men[AIsel[i]].LM;
        pData->Pool[i + pData->AstroCount].EVA = Men[AIsel[i]].EVA;
        pData->Pool[i + pData->AstroCount].Docking = Men[AIsel[i]].Docking;
        pData->Pool[i + pData->AstroCount].Endurance = Men[AIsel[i]].Endurance;
        pData->Pool[i + pData->AstroCount].Status = AST_ST_ACTIVE;
        pData->Pool[i + pData->AstroCount].oldAssign = -1;
        pData->Pool[i + pData->AstroCount].TrainingLevel = 1;
        pData->Pool[i + pData->AstroCount].Group = pData->AstroLevel;
        pData->Pool[i + pData->AstroCount].CR = brandom(2) + 1;
        pData->Pool[i + pData->AstroCount].CL = brandom(2) + 1;
        pData->Pool[i + pData->AstroCount].Task = 0;
        pData->Pool[i + pData->AstroCount].Crew = 0;
        pData->Pool[i + pData->AstroCount].Unassigned = 0;
        pData->Pool[i + pData->AstroCount].Pool = 0;
        pData->Pool[i + pData->AstroCount].Compat = brandom(options.feat_compat_nauts) + 1;  //Naut Compatibility, Nikakd, 10/8/10
        pData->Pool[i + pData->AstroCount].Mood = 100;
        pData->Pool[i + pData->AstroCount].Face = brandom(77);

        if (pData->Pool[i + pData->AstroCount].Sex == 1) {
            pData->Pool[i + pData->AstroCount].Face = 77 + brandom(8);
        }
    }

    pData->AstroLevel++;
    pData->AstroCount = pData->AstroCount + pos;

    switch (pData->AstroLevel) {
    case 1:
        pData->AstroDelay = 6;
        break;

    case 2:
    case 3:
        pData->AstroDelay = 4;
        break;

    case 4:
        pData->AstroDelay = 8;
        break;

    case 5:
        pData->AstroDelay = 99;
        break;
    }

    // remove from the bottom up out of training
    for (i = 0; i < pData->AstroCount; i++) {
        if (pData->Pool[i].Status == AST_ST_TRAIN_BASIC_1) {
            pData->Pool[i].Status = AST_ST_ACTIVE;
        }
    }

    return;
}

void DumpAstro(char plr, int inx)
{
    struct BuzzData *pData = &Data->P[plr];

    for (int i = 0; i < pData->AstroCount; i++) {
        if (pData->Pool[i].Assign == inx && pData->Pool[i].Prime < 1) {
            pData->Pool[i].Assign = 0;  // back to limbo
            pData->Pool[i].Unassigned = 0;
        }
    }

    for (int i = 0; i < ASTRONAUT_CREW_MAX; i++) {
        for (int j = 0; j < ASTRONAUT_FLT_CREW_MAX; j++) {
            if (pData->Crew[inx][i][j] > 0
                && pData->Pool[pData->Crew[inx][i][j] - 1].Prime < 1) {
                pData->Crew[inx][i][j] = 0;
                pData->CrewCount[inx][i] = 0;
            }
        }
    }

    return;
}

char Skill(char plr, char type)
{
    char m, hgh = 0, tst, ind = 0;
    struct BuzzData *pData = &Data->P[plr];

    for (m = 0; m < pData->AstroCount; m++) {
        if (pData->Pool[m].Status == AST_ST_ACTIVE && pData->Pool[m].Assign == 0 && pData->Pool[m].Prime < 1) {
            tst = 0;

            switch (type) {
            case 1:
                tst = pData->Pool[m].Cap + pData->Pool[m].EVA;
                break;

            case 2:
                tst = pData->Pool[m].LM + pData->Pool[m].EVA;
                break;

            case 3:
                tst = pData->Pool[m].Docking;
                break;

            case 4:
                tst = pData->Pool[m].Cap;
                break;

            case 5:
                tst = pData->Pool[m].EVA;
                break;

            default:
                break;
            }

            if (tst >= hgh) {
                ind = m;
                hgh = tst;
            }
        }
    }

    return ind;
}

void TransAstro(char plr, int inx)
// indexed 1 through 5
{
    int i, j, w, count = 0, max, found, flt1, flt2, bug = 1;
    struct BuzzData *pData = &Data->P[plr];

    while (bug == 1) {
        count = 0;

        if (inx == 5) {
            max = 4;
        } else if (inx == 4) {
            max = 3;
        } else {
            max = inx;
        }

        if (pData->AstroCount == 0) {
            return;
        }

        for (i = 0; i < pData->AstroCount; i++) {
            pData->Pool[i].Mood = 100;

            if (pData->Pool[i].Status == AST_ST_RETIRED || pData->Pool[i].Status == AST_ST_DEAD) {
                pData->Pool[i].Status = AST_ST_ACTIVE;
                pData->Pool[i].Assign = 0;
                pData->Pool[i].Unassigned = 0;
                pData->Pool[i].Prime = 0;
                pData->Pool[i].oldAssign = -1;
                pData->Pool[i].Crew = 0;
            }

            pData->Pool[i].RetirementReason = 0;
            pData->Pool[i].Hero = 0;  //clear hero flag

            if (pData->Pool[i].Status == AST_ST_ACTIVE && pData->Pool[i].Assign == 0 && pData->Pool[i].Prime < 1) {
                ++count;
            }
        }

        if (count < max * 2) {
            CheckAdv(plr);
            return;
        }

        flt1 = -1;
        flt2 = -1;

        for (i = 0; i < ASTRONAUT_CREW_MAX; i++) {
            if (pData->CrewCount[inx][i] == 0) {
                if (flt1 == -1) {
                    flt1 = i;
                } else if (flt2 == -1) {
                    flt2 = i;
                }
            }
        }

        if (flt1 == -1 || flt2 == -1) {
            CheckAdv(plr);
            return;
        }

        for (i = 0; i < 2; i++) {
            for (j = 1; j <= max; j++) {
                w = 0;
                found = 0;

                while (w < pData->AstroCount && found == 0) {
                    if (pData->Pool[w].Status == AST_ST_ACTIVE && pData->Pool[w].Assign == 0 && pData->Pool[w].Prime < 1) {
                        // based on [j] an program and position pick best skill
                        switch (inx) {
                        case 1:
                            w = Skill(plr, 1);
                            break;

                        case 2:
                            if (j == 1) {
                                w = Skill(plr, 1);
                            } else {
                                w = Skill(plr, 2);
                            }

                            break;

                        case 3:
                            if (j == 1) {
                                w = Skill(plr, 1);
                            } else if (j == 2) {
                                w = Skill(plr, 2);
                            } else {
                                w = Skill(plr, 3);
                            }

                            break;

                        case 4:
                            if (j == 1) {
                                w = Skill(plr, 4);
                            } else if (j == 2) {
                                w = Skill(plr, 2);
                            } else {
                                w = Skill(plr, 3);
                            }

                            break;

                        case 5:
                            if (j == 1) {
                                w = Skill(plr, 4);
                            } else if (j == 2) {
                                w = Skill(plr, 4);
                            } else {
                                w = Skill(plr, 5);
                            }

                            break;

                        default:
                            break;
                        }

                        if (i == 0) {
                            pData->Pool[w].Assign = inx;
                            pData->Pool[w].Unassigned = 1;
                            pData->Crew[inx][flt1][j - 1] = w + 1;
                            found = 1;
                        } else {
                            pData->Pool[w].Assign = inx;
                            pData->Pool[w].Unassigned = 1;
                            pData->Crew[inx][flt2][j - 1] = w + 1;
                            found = 1;
                        }
                    }  // end if

                    ++w;
                }  // end while
            }

            pData->CrewCount[inx][flt1] = max;
            pData->CrewCount[inx][flt2] = max;
        }
    }  // end while

    return;
}

void CheckAdv(char plr)
{
    int count = 0;
    struct BuzzData *pData = &Data->P[plr];

    for (int i = 0; i < pData->AstroCount; i++) {
        if (pData->Pool[i].Status == AST_ST_ACTIVE && pData->Pool[i].Assign == 0) {
            ++count;
        }
    }

    if (count <= 3) {
        for (int i = 0; i < pData->AstroCount; i++) {
            if (pData->Pool[i].Status == AST_ST_ACTIVE && pData->Pool[i].Assign == 0) {
                pData->Pool[i].Focus = brandom(4) + 1;

                if (pData->Pool[i].Focus > 0) {
                    pData->Cash -= 3;
                    pData->Pool[i].Assign = 0;
                    pData->Pool[i].Status = AST_ST_TRAIN_ADV_1;
                }
            }
        }
    }

    return;
}

/**
 AI Research & Development improvements

 @param plr The player this equipment purchase is for.
 @param hardware_index The class of hardware to R&D
 @param index The unit index is the type hardware in the specified class

 */
void RDafford(char plr, int equipment_class, int index)
{
    int16_t cost = 0, roll = 0, ok = 0;
    struct BuzzData *pData = &Data->P[plr];

    if (pData->Buy[equipment_class][index] > 0) {
        return;
    }

    if (equipment_class == PROBE_HARDWARE) {
        cost = pData->Probe[index].RDCost;
        roll = pData->Probe[index].MaxRD - pData->Probe[index].Safety;
    }

    if (equipment_class == ROCKET_HARDWARE) {
        cost = pData->Rocket[index].RDCost;
        roll = pData->Rocket[index].MaxRD - pData->Rocket[index].Safety;
    }

    if (equipment_class == MANNED_HARDWARE) {
        cost += pData->Manned[index].RDCost;
        roll = pData->Manned[index].MaxRD - pData->Manned[index].Safety;
    }

    if (equipment_class == MISC_HARDWARE) {
        cost = pData->Misc[index].RDCost;
        roll = pData->Misc[index].MaxRD - pData->Misc[index].Safety;
    }

    roll = MAX(((roll * 10) / 35), 5);
    ok = 0;

    while (ok == 0 && roll != 0) {
        if ((cost * roll <= pData->Cash) && QueryUnit(equipment_class, index, plr) == 1
            && MaxChk(equipment_class, index, plr)) {
            pData->Buy[equipment_class][index] = RDUnit(equipment_class, index, roll, plr);
            pData->Cash = (pData->Cash - (cost * roll));
            ok = 1; // jump out of the loop
        } else {
            roll -= 1;
        }
    }
}

/**
 Purchase Orbital Probe & One-Stage Rocket

 @param plr The player this equipment purchase is for.
 */
void AIPur(char plr)
{
    struct BuzzData *pData = &Data->P[plr];

    if (pData->AIStat == 0) {
        pData->AIStat = 1;
    }

    if (pData->AIStrategy[AI_ORBITAL_SATELLITE] == 0) {
        pData->AIStrategy[AI_ORBITAL_SATELLITE] = 1;
    }

    // If we don't have a probe, buy one before the rocket, then attempt to research each
    if (pData->Probe[PROBE_HW_ORBITAL].Num <= pData->Rocket[ROCKET_HW_ONE_STAGE].Num) {
        GenPur(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
        RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
    } else {
        GenPur(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
        RDafford(plr, ROCKET_HARDWARE, ROCKET_HW_ONE_STAGE);
        GenPur(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
        RDafford(plr, PROBE_HARDWARE, PROBE_HW_ORBITAL);
    }
}

/**
 Generic Purchase routine.  Given the hardware and unit indicies equipment
 is purchased for (or given to) the AI player.

 @param plr The player this equipment purchase is for.
 @param hardware_index
 @param unit_index

 @return 0 No hardware was purchased
 @return 1 Hardware was purchased

 */
int GenPur(char plr, int hardware_index, int unit_index)
{
    bool newProgramStarted = false;
    bool itemPurchased = false;
    int n1, n2, n3, n4, n5, n6, n7;  // scratch variables for base safety value init

    struct BuzzData *pData = &Data->P[plr];

    // Force the Docking Module Instead before Kickers
    if (hardware_index == MISC_HARDWARE &&
        unit_index <= MISC_HW_KICKER_B &&
        pData->Misc[MISC_HW_DOCKING_MODULE].Num == PROGRAM_NOT_STARTED) {
        unit_index = MISC_HW_DOCKING_MODULE;
    };

    switch (hardware_index) {
    case PROBE_HARDWARE:  // Probe
        if (pData->Probe[unit_index].Num < 2) {
            // Probe Programs
            if (pData->Probe[unit_index].Num == PROGRAM_NOT_STARTED) {
                if (pData->Probe[unit_index].InitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Probe[unit_index].InitCost;
                    pData->Probe[unit_index].Num = 1;
                    itemPurchased = true;
                    newProgramStarted = true;
                }
            } else {
                // Do we have enough to purchase one?
                if (pData->Probe[unit_index].UnitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Probe[unit_index].UnitCost;
                    pData->Probe[unit_index].Num = pData->Probe[unit_index].Num + 1;
                    itemPurchased = true;
                }

#ifndef DISABLE_AI_CHEAT
                else {
                    // Just give it to them anyway
                    itemPurchased = true;
                    ++pData->Probe[unit_index].Num;
                }

#endif
            }
        };  // end case PROBE_HARDWARE

        break;

    case ROCKET_HARDWARE:  // Rockets
        if (pData->Rocket[unit_index].Num < ROCKET_HW_THREE_STAGE) {
            // Rocket Programs Purchasing
            if (pData->Rocket[unit_index].Num == PROGRAM_NOT_STARTED) {
                // Do we have enough to purchase one?
                if (pData->Rocket[unit_index].InitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Rocket[unit_index].InitCost;

                    if (pData->Rocket[unit_index].Num == PROGRAM_NOT_STARTED) {
                        pData->Rocket[unit_index].Num = 1;
                    } else {
                        ++pData->Rocket[unit_index].Num;
                    }

                    itemPurchased = true;
                    newProgramStarted = true;
                }
            } else {
                if (pData->Rocket[unit_index].Num == 1 && (pData->Rocket[unit_index].Safety < pData->Rocket[unit_index].MaxRD - 15)) {
                    // AI Player does R&D on this purchased item
                    RDafford(plr, ROCKET_HARDWARE, unit_index);
                    pData->Buy[ROCKET_HARDWARE][unit_index] = 0;  // Reset the record of this?
                    itemPurchased = true;
                } else if (pData->Rocket[unit_index].Num >= 0) {
                    // Do we have enough to purchase one?
                    if (pData->Rocket[unit_index].UnitCost < pData->Cash) {
                        pData->Cash = pData->Cash - pData->Rocket[unit_index].UnitCost;
                        ++pData->Rocket[unit_index].Num;
                        itemPurchased = true;
                    }

#ifndef DISABLE_AI_CHEAT
                    else {
                        // Just give them one anyway
                        itemPurchased = true;
                        ++pData->Rocket[unit_index].Num;
                    }

#endif
                }
            }
        };  // end case ROCKET_HARDWARE

        break;

    case MANNED_HARDWARE: // Manned Capsules
        if (unit_index == MANNED_HW_MINISHUTTLE && pData->Manned[MANNED_HW_MINISHUTTLE].Num == 1) {
            // Only need one Minishuttle
            return 1;
        } else if (pData->Manned[unit_index].Num < MANNED_HW_THREE_MAN_CAPSULE) {
            // Manned Programs
            if (pData->Manned[unit_index].Num == PROGRAM_NOT_STARTED) {
                // Do we have enough to purchase one?
                if (pData->Manned[unit_index].InitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Manned[unit_index].InitCost;

                    if (pData->Manned[unit_index].Num == PROGRAM_NOT_STARTED) {
                        pData->Manned[unit_index].Num = 1;
                    } else {
                        ++pData->Manned[unit_index].Num;
                    }

                    itemPurchased = true;
                    newProgramStarted = true;
                }
            } else {
                if (pData->Manned[unit_index].Num == 1 && (pData->Manned[unit_index].Safety < pData->Manned[unit_index].MaxRD - 15)) {
                    // AI Player does R&D on this purchased item
                    RDafford(plr, MANNED_HARDWARE, unit_index);
                    pData->Buy[MANNED_HARDWARE][unit_index] = 0;  // reset the record of this?
                    itemPurchased = true;
                } else if (pData->Manned[unit_index].Num >= 0) {
                    // Do we have enough to purchase one?
                    if (pData->Manned[unit_index].UnitCost < pData->Cash) {
                        pData->Cash = pData->Cash - pData->Manned[unit_index].UnitCost;
                        ++pData->Manned[unit_index].Num;
                        itemPurchased = true;
                    }

#ifndef DISABLE_AI_CHEAT
                    else {
                        // Just give it to them anyway
                        itemPurchased = true;
                        ++pData->Manned[unit_index].Num;
                    }

#endif
                }
            }
        };  // end case MANNED_HARDWARE

        break;

    case MISC_HARDWARE:  // Misc hardware (Kicker, EVA, Docking Module)
        if (pData->Misc[unit_index].Num < 2) {  // Misc Programs
            if (unit_index == MISC_HW_EVA_SUITS && pData->Misc[unit_index].Num == 1) {
                // Simply return when starting the EVA program, no cost for this
                return 1;
            }

            if (pData->Misc[unit_index].Num == PROGRAM_NOT_STARTED) {
                // Do we have enough to purchase one?
                if (pData->Misc[unit_index].InitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Misc[unit_index].InitCost;

                    if (pData->Misc[unit_index].Num == PROGRAM_NOT_STARTED) {
                        pData->Misc[unit_index].Num = 1;
                    } else {
                        ++pData->Misc[unit_index].Num;
                    }

                    itemPurchased = true;
                    newProgramStarted = true;
                }
            } else if (pData->Misc[unit_index].Num >= 0) {
                // Do we have enough to purchase one?
                if (pData->Misc[unit_index].UnitCost < pData->Cash) {
                    pData->Cash = pData->Cash - pData->Misc[unit_index].UnitCost;
                    ++pData->Misc[unit_index].Num;
                    itemPurchased = true;
                }

#ifndef DISABLE_AI_CHEAT
                else {
                    // Just give them one anyway
                    itemPurchased = true;
                    ++pData->Misc[unit_index].Num;
                }

#endif
            }
        };  // end case MISC_HARDWARE

        break;

    default:
        break;
    }  // end switch

    // Initialize starting and cost bonuses
    if (hardware_index == PROBE_HARDWARE && newProgramStarted) {
        Equipment *e = &pData->Probe[unit_index];  // Hardware we're modifying

        switch (unit_index) {
        case PROBE_HW_ORBITAL:
            if (pData->Probe[PROBE_HW_INTERPLANETARY].Safety >= 75) {
                e->Safety = 50;
            }

            if (pData->Probe[PROBE_HW_LUNAR].Safety >= 75) {
                e->Safety = 60;
            }

            break;

        case PROBE_HW_INTERPLANETARY:
            if (pData->Probe[PROBE_HW_ORBITAL].Safety >= 75) {
                e->Safety = 45;
            }

            if (pData->Probe[PROBE_HW_LUNAR].Safety >= 75) {
                e->Safety = 50;
            }

            break;

        case PROBE_HW_LUNAR:
            if (pData->Probe[PROBE_HW_ORBITAL].Safety >= 75) {
                e->Safety = 45;
            }

            if (pData->Probe[PROBE_HW_INTERPLANETARY].Safety >= 75) {
                e->Safety = 50;
            }

            break;
        }

        e->Base = e->Safety;  // Set the base safety level
    };

    if (hardware_index == ROCKET_HARDWARE && newProgramStarted) {
        Equipment *e = &pData->Rocket[unit_index];  // Hardware we're modifying

        // Safety levels of existing programs
        n1 = pData->Rocket[ROCKET_HW_ONE_STAGE].Safety;
        n2 = pData->Rocket[ROCKET_HW_TWO_STAGE].Safety;
        n3 = pData->Rocket[ROCKET_HW_THREE_STAGE].Safety;
        n4 = pData->Rocket[ROCKET_HW_MEGA_STAGE].Safety;
        n5 = pData->Rocket[ROCKET_HW_BOOSTERS].Safety;

        switch (unit_index) {
        case ROCKET_HW_ONE_STAGE:
            if (n2 >= 75 || n3 >= 75 || n4 >= 75 || n5 >= 75) {
                e->Safety = 35;
            }

            break;

        case ROCKET_HW_TWO_STAGE:
            if (n1 >= 75 || n5 >= 75) {
                e->Safety = 25;
            }

            if (n3 >= 75 || n4 >= 75) {
                e->Safety = 40;
            }

            if ((n1 >= 75 || n5 >= 75) && (n3 >= 75 || n4 >= 75)) {
                e->Safety = 65;
            }

            break;

        case ROCKET_HW_THREE_STAGE:
            if (n1 >= 75 || n5 >= 75) {
                e->Safety = 15;
            }

            if (n2 >= 75 || n4 >= 75) {
                e->Safety = 35;
            }

            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75)) {
                e->Safety = 60;
            }

            break;

        case ROCKET_HW_MEGA_STAGE:
            if (n1 >= 75 || n5 >= 75) {
                e->Safety = 10;
            }

            if (n2 >= 75 || n3 >= 75) {
                e->Safety = 25;
            }

            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n3 >= 75)) {
                e->Safety = 35;
            }

            break;

        case ROCKET_HW_BOOSTERS:
            if (n1 >= 75 || n2 >= 75 || n3 >= 75 || n4 >= 75) {
                e->Safety = 30;
            }

            break;
        }

        e->Base = e->Safety;  // Set the base safety level
    };

    if (hardware_index == MANNED_HARDWARE && newProgramStarted) {
        Equipment *e = &pData->Manned[unit_index];  // Hardware we're modifying

        // Safety levels of existing programs copied to working vars
        n1 = pData->Manned[MANNED_HW_ONE_MAN_CAPSULE].Safety;
        n2 = pData->Manned[MANNED_HW_TWO_MAN_CAPSULE].Safety;
        n3 = pData->Manned[MANNED_HW_THREE_MAN_CAPSULE].Safety;
        n4 = pData->Manned[MANNED_HW_MINISHUTTLE].Safety;
        n5 = pData->Manned[MANNED_HW_FOUR_MAN_CAPSULE].Safety;
        n6 = pData->Manned[MANNED_HW_TWO_MAN_MODULE].Safety;
        n7 = pData->Manned[MANNED_HW_ONE_MAN_MODULE].Safety;

        switch (unit_index) {
        case MANNED_HW_ONE_MAN_CAPSULE:
            if (n2 >= 75 || n3 >= 75 || n5 >= 75) {
                e->Safety = 40;
            }

            break;

        case MANNED_HW_TWO_MAN_CAPSULE:
            if (n1 >= 75) {
                e->Safety = 20;
            }

            if (n3 >= 75 || n5 >= 75) {
                e->Safety = 40;
            }

            break;

        case MANNED_HW_THREE_MAN_CAPSULE:
            if (n1 >= 75 || n5 >= 75) {
                e->Safety = 20;
            }

            if (n2 >= 75 || n4 >= 75) {
                e->Safety = 30;
            }

            if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75)) {
                e->Safety = 40;
            }

            break;

        case MANNED_HW_MINISHUTTLE:
            break;

        case MANNED_HW_FOUR_MAN_CAPSULE:
            if (n1 >= 75) {
                e->Safety = 10;
            }

            if (n2 >= 75) {
                e->Safety = 15;
            }

            if (n3 >= 75) {
                e->Safety = 25;
            }

            if ((n1 >= 75 || n2 >= 75 || n3 >= 75) && (n6 >= 75 || n7 >= 75)) {
                e->Safety = 35;
            }

            break;

        case MANNED_HW_TWO_MAN_MODULE:
            if (n7 >= 75) {
                e->Safety = 30;
            }

            if (n5 >= 75) {
                e->Safety = 40;
            }

            break;

        case MANNED_HW_ONE_MAN_MODULE:
            if (n6 >= 75) {
                e->Safety = 30;
            }

            if (n5 >= 75) {
                e->Safety = 40;
            }

            break;
        }

        e->Base = e->Safety;  // Set the base safety level

    }

    if (hardware_index == MISC_HARDWARE && newProgramStarted) {
        Equipment *e = &pData->Misc[unit_index];  // Hardware we're modifying

        switch (unit_index) {
        case MISC_HW_KICKER_A:
            if (pData->Rocket[ROCKET_HW_TWO_STAGE].Safety >= 75) {
                e->Safety = 40;
            }

            break;

        case MISC_HW_KICKER_B:
            if (pData->Rocket[ROCKET_HW_ONE_STAGE].Safety >= 75) {
                e->Safety = 35;
            }

            break;

        case MISC_HW_KICKER_C:
            if (pData->Rocket[ROCKET_HW_ONE_STAGE].Safety >= 75 ||
                pData->Rocket[ROCKET_HW_TWO_STAGE].Safety >= 75) {
                e->Safety = 25;
            }

            break;
        }

        e->Base = e->Safety;  // Set the base safety level

    }

    return (itemPurchased == true);
}


/* EOF */
