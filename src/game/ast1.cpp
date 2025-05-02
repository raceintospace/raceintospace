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
/** \file ast1.c Astronaut Recruitment
 */

// This file handles 'naut recruitment.

/* Variables in this file
   BarA   The line where the bar is showing on the Left (0-7)
   BarB   The line where the bar is showing on the Right (0-7)
   MaxMen The total number of candidates available for this recruitment
   MaxSel The maximum number of candidates you can select in this recruitment
   min    The first person being read from the 'naut roster
   max    The last person being read from the 'naut roster
   count  The number of candidates selected so far
   now    The selected candidate (?) on the Right
   now2   The selected candidate (?) on the Left
   ksel   Which side is currently selected (0=Right, 1=Left) */

#include "ast1.h"

#include <cassert>

#include "display/graphics.h"

#include "aipur.h"
#include "ast0.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "game_main.h"
#include "gr.h"
#include "options.h"   //Naut Randomize && Naut Compatibility, Nikakd, 10/8/10
#include "pace.h"
#include "place.h"
#include "sdlhelper.h"


enum DialogueResponse {
    DLG_RESPONSE_YES = 1,
    DLG_RESPONSE_NO = -1,
    DLG_RESPONSE_NONE = 0
};

namespace
{
enum ProfileDisplay {
    SHOW_CAPSULE = 0x01,
    SHOW_LM = 0x02,
    SHOW_EVA = 0x04,
    SHOW_DOCKING = 0x08,
    SHOW_ENDURANCE = 0x10
};

  constexpr std::array<char* , 7>AstService = {"CIVILIAN", "AIR FORCE", \
                                               "NAVY", "MARINE CORPS",  \
                                               "S. ROCKET FORCE",       \
                                               "AIR DEF. FORCE", "FOREIGN"};
}

int AstSelectPrompt(char plr, int cost);
void DispEight(int now, int loc);
void DispEight2(int nw, int lc, int cnt);
void DrawAstCheck(char plr);
void DrawAstSel(char plr);
void DrawRecruitProfile(int x, int y, const struct ManPool *recruit,
                        int display);
int ProfileMask(int player);
void RandomizeNauts();
void Recruit(char plr, uint8_t pool, uint8_t candidate);


/**
 * Ask the player if they wish to pay the cost to recruit new
 * astronauts/cosmonauts.
 */
int AstSelectPrompt(char plr, int cost)
{
    DrawAstCheck(plr);
    WaitForMouseUp();
    int choice = DLG_RESPONSE_NONE;

    while (choice == DLG_RESPONSE_NONE) {
        key = 0;
        GetMouse();

        if ((x >= 100 && y >= 135 && x <= 148 && y <= 147 && mousebuttons > 0) || key == 'Y') {

            InBox(100, 135, 148, 147);
            WaitForMouseUp();

            if (Data->P[plr].Cash >= cost) {
                choice = DLG_RESPONSE_YES;
            } else {
                choice = DLG_RESPONSE_NO;
            }

            if (Data->P[plr].AstroDelay > 0) {
                choice = DLG_RESPONSE_NO;
            }

            if (key > 0) {
                delay(150);
            }

            OutBox(100, 135, 148, 147);
        }

        if ((x >= 168 && y >= 135 && x <= 216 && y <= 147 && mousebuttons > 0)
            || key == 'N' || key == K_ESCAPE || key == K_ENTER) {

            InBox(168, 135, 216, 147);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            choice = DLG_RESPONSE_NO;
            OutBox(168, 135, 216, 147);
        }
    }

    return choice;
}


/**
 * Display the list of potential astronaut/cosmonaut recruits.
 *
 * \param now  index of the selected candidate among all candidates in the pool.
 * \param loc  index of the selection bar relative to top entry (0-7).
 */
void DispEight(int now, int loc)
{
    int i, start;
    start = now - loc;

    fill_rectangle(186, 129, 314, 195, 0);  // Blank display area
    ShBox(187, 130 + loc * 8, 313, 138 + loc * 8);  // Selection bar

    for (i = start; i < start + 8; i++) {
        if (MCol[i] == 1) {
            display::graphics.setForegroundColor(8);
        } else {
            display::graphics.setForegroundColor(6 + (Men[i].Sex + 1) * 6);
        }

        draw_string(189, 136 + (i - start) * 8, &Men[i].Name[0]);
    }

    return;
} /* End of Disp8 */


/**
 * Display the list of picked recruits.
 *
 * \param now  index of the selected candidate among all selected candidates.
 * \param loc  index of the selection bar relative to top entry (0-7).
 * \param cnt  how many 'nauts have been picked.
 */
void DispEight2(int nw, int lc, int cnt)
{
    int i, start, num;
    start = nw - lc;
    num = (cnt < 8) ? cnt : 8;

    fill_rectangle(26, 129, 153, 195, 0);
    ShBox(26, 130 + lc * 8, 152, 138 + lc * 8);

    display::graphics.setForegroundColor(11);

    for (i = start; i < start + num; i++) {
        if (sel[i] != -1) {
            display::graphics.setForegroundColor(6 + (Men[sel[i]].Sex + 1) * 6);
            draw_string(28, 136 + (i - start) * 8, &Men[ sel[i] ].Name[0]);
        }
    }

    return;
}


void DrawAstCheck(char plr)
{
    int i, pos, ad = 0;

    pos = 0;  /* XXX check uninitialized */

    if (Data->P[plr].AstroDelay > 0) {
        ad = 1;
    }

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(80, 44, 237, 155);
    InBox(87, 49, 230, 103);
    fill_rectangle(88, 50, 229, 102, 7 + plr * 3);
    IOBox(98, 133, 150, 149);
    IOBox(166, 133, 218, 149);
    display::graphics.setForegroundColor(5);

    if (plr == 0) {
        draw_string(99, 60, "ASTRONAUT");
    } else {
        draw_string(99, 60, "COSMONAUT");
    }

    draw_string(0, 0, " RECRUITMENT");
    display::graphics.setForegroundColor(11);
    draw_string(100, 73, "GROUP ");

    switch (Data->P[plr].AstroLevel) {
    case 0:
        draw_string(0, 0, "I");
        pos = ASTRO_POOL_LVL1;
        break;

    case 1:
        draw_string(0, 0, "II");
        pos = ASTRO_POOL_LVL2;
        break;

    case 2:
        draw_string(0, 0, "III");
        pos = ASTRO_POOL_LVL3;
        break;

    case 3:
        draw_string(0, 0, "IV");
        pos = ASTRO_POOL_LVL4;
        break;

    case 4:
        draw_string(0, 0, "V");
        pos = ASTRO_POOL_LVL5;
        break;
    }

    if (Data->Season == 0) {
        draw_string(160, 73, "SPRING 19");
    } else {
        draw_string(170, 73, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    if (Data->P[plr].AstroLevel == 0) {
        i = 20;
    } else {
        i = 15;
    }

    if (ad == 0) {
        if (pos < 10) {
            draw_number(110, 86, pos);
        } else {
            draw_number(108, 86, pos);
        }

        draw_string(0, 0, " POSITIONS TO FILL");
        draw_string(116, 97, "COST: ");
        display::graphics.setForegroundColor(1);
        draw_number(0, 0, i);
        draw_string(0, 0, " MB ");
        display::graphics.setForegroundColor(11);
        draw_string(0, 0, "(OF ");
        draw_number(0, 0, Data->P[plr].Cash);
        draw_string(0, 0, ")");
    } else {
        if (Data->P[plr].AstroDelay != 1) {
            draw_number(114, 86, Data->P[plr].AstroDelay);
        } else {
            draw_number(118, 86, Data->P[plr].AstroDelay);
        }

        draw_string(0, 0, " SEASON");

        if (Data->P[plr].AstroDelay != 1) {
            draw_string(0, 0, "S");
        }

        draw_string(0, 0, " TO WAIT");
        draw_string(104, 97, "FOR THE NEW RECRUITS");
    }

    display::graphics.setForegroundColor(1);

    if (ad == 1) {
        draw_string(102, 113, "  YOU CANNOT RECRUIT");

        if (plr == 0) {
            draw_string(102, 122, "ASTRONAUTS THIS TURN");
        } else {
            draw_string(102, 122, "COSMONAUTS THIS TURN");
        }

        display::graphics.setForegroundColor(8);
        draw_string(114, 143, "EXIT");
        draw_string(182, 143, "EXIT");
    }

    if (ad == 0) {
        if (Data->P[plr].Cash < i) {
            draw_string(111, 113, "YOU CANNOT AFFORD");

            if (plr == 0) {
                draw_string(103, 122, "ASTRONAUTS THIS TURN");
            } else {
                draw_string(103, 122, "COSMONAUTS THIS TURN");
            }

            display::graphics.setForegroundColor(8);
            draw_string(113, 143, "EXIT");
            draw_string(181, 143, "EXIT");
        } else {
            draw_string(101, 113, "DO YOU WISH TO RECRUIT");

            if (plr == 0) {
                draw_string(100, 122, "ASTRONAUTS THIS TURN?");
            } else {
                draw_string(100, 122, "COSMONAUTS THIS TURN?");
            }

            display::graphics.setForegroundColor(6);
            draw_string(116, 143, "Y");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, "ES");
            display::graphics.setForegroundColor(6);
            draw_string(187, 143, "N");
            display::graphics.setForegroundColor(1);
            draw_string(0, 0, "O");
        }
    }

    draw_small_flag(plr, 4, 4);

    FadeIn(2, 10, 0, 0);

    return;
}


/** Draw selection screen for Astronaut recruitment
 *
 */
void DrawAstSel(char plr)
{

    helpText = "i012";
    keyHelpText = "k012";
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 158, 199);
    ShBox(161, 24, 319, 199);
    fill_rectangle(5, 129, 19, 195, 0);
    fill_rectangle(166, 129, 180, 195, 0);
    fill_rectangle(25, 129, 153, 195, 0);
    fill_rectangle(186, 129, 314, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    ShBox(167, 130, 179, 161);
    ShBox(167, 163, 179, 194);
    IOBox(172, 3, 240, 19);
    IOBox(244, 3, 316, 19);
    IOBox(5, 109, 153, 125);
    IOBox(166, 109, 315, 125);
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    InBox(185, 128, 315, 196);
    InBox(165, 128, 181, 196);
    InBox(3, 3, 30, 19);
    draw_up_arrow(9, 133);
    draw_down_arrow(9, 166);  //Left
    draw_up_arrow(170, 133);
    draw_down_arrow(170, 166);  //Right
    display::graphics.setForegroundColor(1);
    draw_string(189, 13, "CANCEL");
    draw_string(258, 13, "CONTINUE");
    draw_heading(40, 5, "RECRUITMENT", 0, -1);
    draw_small_flag(plr, 4, 4);

    InBox(3, 27, 155, 44);
    fill_rectangle(4, 28, 154, 43, 7);
    InBox(164, 27, 316, 44);
    fill_rectangle(165, 28, 315, 43, 7);

    display::graphics.setForegroundColor(1);

    if (plr == 0) {
        draw_string(25, 34, "ASTRONAUT SELECTION");
    } else {
        draw_string(25, 34, "COSMONAUT SELECTION");
    }

    draw_string(192, 34, "POOL OF APPLICANTS");

    display::graphics.setForegroundColor(11);
    draw_string(12, 41, "GROUP ");

    switch (Data->P[plr].AstroLevel) {
    case 0:
        draw_string(0, 0, "I");
        break;

    case 1:
        draw_string(0, 0, "II");
        break;

    case 2:
        draw_string(0, 0, "III");
        break;

    case 3:
        draw_string(0, 0, "IV");
        break;

    case 4:
        draw_string(0, 0, "V");
        break;
    }

    draw_string(185, 41, "REMAINING POSITIONS: ");

    if (Data->Season == 0) {
        draw_string(88, 41, "SPRING 19");
    } else {
        draw_string(98, 41, "FALL 19");
    }

    draw_number(0, 0, Data->Year);
    display::graphics.setForegroundColor(9);
    draw_string(12, 52, "NAME:");
    draw_string(173, 52, "NAME:");
    draw_string(12, 61, "SERVICE:");
    draw_string(173, 61, "SERVICE:");
    draw_string(12, 70, "SKILLS:");
    draw_string(173, 70, "SKILLS:");
    display::graphics.setForegroundColor(11);
    draw_string(54, 70, "CAPSULE PILOT:");
    draw_string(215, 70, "CAPSULE PILOT:");
    display::graphics.setForegroundColor(11);
    draw_string(54, 78, "L.M. PILOT: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 78, "L.M. PILOT: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 86, "E.V.A.: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 86, "E.V.A.: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 94, "DOCKING: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(215, 94, "DOCKING: ");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "--");
    display::graphics.setForegroundColor(11);
    draw_string(54, 102, "ENDURANCE:");
    draw_string(215, 102, "ENDURANCE:");
    display::graphics.setForegroundColor(6);
    draw_string(33, 119, "D");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ISMISS APPLICANT");
    display::graphics.setForegroundColor(6);
    draw_string(194, 119, "R");
    display::graphics.setForegroundColor(1);
    draw_string(0, 0, "ECRUIT APPLICANT");

    return;
}


/**
 * Display the personal information, including stats, of a candidate.
 *
 * Parts of candidate's information may not be available, notably
 * their stats. The display argument takes a set of ProfileDisplay
 * flags that determine which stats should be revealed, and which
 * should be hidden.
 *
 * \param x       the x coord of the top-left corner of the profile space.
 * \param y       the y coord of the top-left corner of the profile space.
 * \param recruit  the astronaut/cosmonaut or NULL for an empty profile.
 * \param display  flags indicating which recruit skills to reveal.
 */
void DrawRecruitProfile(int x, int y, const struct ManPool *recruit,
                        int display)
{
    // Regular text has a height of 5 pixels.
    fill_rectangle(x + 33, y + 1, x + 133, y + 5, 3);  // Draws over Name
    fill_rectangle(x + 48, y + 10, x + 133, y + 14, 3);  // Service
    fill_rectangle(x + 120, y + 19, x + 133, y + 23, 3);  // Capsule
    fill_rectangle(x + 94, y + 27, x + 107, y + 31, 3);  // LM
    fill_rectangle(x + 71, y + 35, x + 84, y + 39, 3);  // EVA
    fill_rectangle(x + 87, y + 43, x + 100, y + 47, 3);  // Docking
    fill_rectangle(x + 101, y + 51, x + 114, y + 55, 3);  // Endurance

    display::graphics.setForegroundColor(1);

    if (recruit) {
        draw_string(x + 33, y + 5, recruit->Sex ? "MS. " : "MR. ");
        draw_string(0, 0, &(recruit->Name[0]));
        // Service
        draw_string(x + 48, y + 14, AstService.at(recruit->Service));
    }

    if (recruit && display & SHOW_CAPSULE) {
        draw_number(x + 121, y + 23, recruit->Cap);
    } else {
        draw_string(x + 121, y + 23, "--");  // Capsule rating
    }

    if (recruit && display & SHOW_LM) {
        draw_number(x + 95, y + 31, recruit->LM);
    } else {
        draw_string(x + 94, y + 31, "--");  // LM rating
    }

    if (recruit && display & SHOW_EVA) {
        draw_number(x + 72, y + 39, recruit->EVA);
    } else {
        draw_string(x + 71, y + 39, "--");  // EVA rating
    }

    if (recruit && display & SHOW_DOCKING) {
        draw_number(x + 88, y + 47, recruit->Docking);
    } else {
        draw_string(x + 87, y + 47, "--");  // Docking rating
    }

    if (recruit && display & SHOW_ENDURANCE) {
        draw_number(x + 102, y + 55, recruit->Endurance);
    } else {
        draw_string(x + 102, y + 55, "--");  // Endurance rating
    }
}


/**
 * Determine which candidate stats should be revealed.
 *
 * \param player  the player index.
 */
int ProfileMask(int player)
{
    assert(player == 0 || player == 1);
    uint8_t level = (player == 0) ? Data->Def.Ast1 : Data->Def.Ast2;

    int stats = SHOW_CAPSULE | SHOW_ENDURANCE;

    if (options.feat_show_recruit_stats && level < 2) {
        stats |= SHOW_DOCKING;
    }

    if (options.feat_show_recruit_stats && level < 1) {
        stats |= SHOW_LM | SHOW_EVA;
    }

    return stats;
}


// Naut Randomize, Nikakd, 10/8/10
// Note: These stats are far more generous than the historical ones.
void RandomizeNauts()
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
 * Copy astronaut data from the roster pool into the player data.
 *
 * When an astronaut is recruited from the pool of candidates, there
 * is a chance of losing a point of Capsule or Endurance, and they will
 * lose 3 points randomly split among LM, EVA, and Docking (which can
 * take the astronaut below 0).
 *
 * \param plr   0 for the USA, 1 for the USSR.
 * \param pool  The index of the astronaut in the recruited candidate pool.
 * \param candidate  The index of the candidate among all available 'nauts.
 */
void Recruit(const char plr, const uint8_t pool, const uint8_t candidate)
{
    struct Astros &recruit = Data->P[plr].Pool[pool];

    strcpy(&recruit.Name[0], &Men[candidate].Name[0]);
    recruit.Sex = Men[candidate].Sex;
    recruit.Race = Men[candidate].Race;
    recruit.Service = Men[candidate].Service;
    recruit.Cap = Men[candidate].Cap;
    recruit.LM = Men[candidate].LM;
    recruit.EVA = Men[candidate].EVA;
    recruit.Docking = Men[candidate].Docking;
    recruit.Endurance = Men[candidate].Endurance;
    recruit.Status = AST_ST_TRAIN_BASIC_1;
    recruit.TrainingLevel = 1;
    recruit.Group = Data->P[plr].AstroLevel;
    recruit.CR = brandom(2) + 1;
    recruit.CL = brandom(2) + 1;
    recruit.Task = 0;
    recruit.Crew = 0;
    recruit.Unassigned = 0;
    recruit.Pool = 0;
    recruit.Compat = brandom(options.feat_compat_nauts) + 1;  //Naut Compatibility, Nikakd, 10/8/10
    recruit.Mood = 85 + 5 * brandom(4);
    recruit.Race = Men[candidate].Race;
    if (recruit.Race == 2) {
        recruit.Face = 92 + brandom(4);
    } else if (recruit.Race == 1) {
        recruit.Face = 87 + brandom(4);
    } else {
        recruit.Face = recruit.Sex ? (77 + brandom(8)) : brandom(77);
    }

    if (brandom(10) > 5) {
        if (brandom(2) == 0) {
            recruit.Endurance--;
        } else {
            recruit.Cap--;
        }
    }

    for (uint8_t j = 0; j < 3; j++) {
        uint8_t skill = brandom(3);

        switch (skill) {
        case 0:
            recruit.LM--;
            break;

        case 1:
            recruit.EVA--;
            break;

        case 2:
            recruit.Docking--;
            break;

        default:
            break;
        }
    }
}


void AstSel(char plr)
{
    int i, j, k, BarA, BarB, MaxMen, Index, now, now2, max, change, min, count,
         ksel = 0;
    FILE *fin;

    bool femaleAstronautsAllowed =
        (options.feat_female_nauts ||
         Data->P[plr].FemaleAstronautsAllowed == 1);
    bool femaleAstronautsRequired = Data->P[plr].FemaleAstronautsAllowed;

    MaxMen = Index = now = now2 = max = min = count = 0;

    music_start(M_DRUMSM);

    int cost = (Data->P[plr].AstroLevel == 0) ? 20 : 15;

    if (AstSelectPrompt(plr, cost) != DLG_RESPONSE_YES) {
        music_stop();    /* too poor for astronauts or NO */
        return;
    }

    BarA = 0;
    BarB = 0;
    DrawAstSel(plr);

    memset(sel, -1, sizeof(sel));
    memset(MCol, 0x00, sizeof(MCol));

    DESERIALIZE_JSON_FILE(&Men, locate_file("roster.json", FT_SAVE));

    if (options.feat_random_nauts == 1) {
        RandomizeNauts();    //Naut Randomize, Nikakd, 10/8/10
    }

    switch (Data->P[plr].AstroLevel) {
    case 0:
        MaxMen = femaleAstronautsAllowed ? 13 : 10;
        MaxSel = ASTRO_POOL_LVL1;
        Index = 0;
        break;

    case 1:
        MaxMen = femaleAstronautsAllowed ? 20 : 17;
        MaxSel = ASTRO_POOL_LVL2;
        Index = 14;
        break;

    case 2:
        MaxMen = femaleAstronautsAllowed ? 22 : 19;
        MaxSel = ASTRO_POOL_LVL3;
        Index = 35;
        break;

    case 3:
        MaxMen = femaleAstronautsAllowed ? 30 : 27;
        MaxSel = ASTRO_POOL_LVL4;
        Index = 58;
        femaleAstronautsRequired = false;
        break;

    case 4:
        MaxMen = femaleAstronautsAllowed ? 22 : 19;
        MaxSel = ASTRO_POOL_LVL5;
        Index = 89;
        femaleAstronautsRequired = false;
        break;
    }

    display::graphics.setForegroundColor(11);
    draw_number(292, 41, MaxSel);

    Data->P[plr].Cash -= cost;
    Data->P[plr].Spend[0][2] += cost;

    // A lot of the right-side astronaut selection logic depends on
    // having 8+ constant selections at all times.
    assert(MaxMen >= 8);

    int showStats = ProfileMask(plr);

    Index += plr * Men.size() / 2;

    now = Index;
    max = Index + MaxMen;
    min = Index;
    now2 = 0;
    count = 0;  /* counter for # selected */

    DispEight(now, BarB);
    DrawRecruitProfile(173, 47, &Men[now], showStats);
    DispEight2(now2, BarA, count);
    DrawRecruitProfile(12, 47, NULL, showStats);

    FadeIn(2, 10, 0, 0);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        for (i = 0; i < 8; i++) {
            // Right Select Box
            if (((x >= 188 && y >= (131 + i * 8) && x <= 312 && y <= (137 + i * 8) && mousebuttons > 0)
                 || (key == RT_ARROW && ksel == 1)) && (now - BarB + i) <= max) {

                if (ksel == 1) {
                    ksel = 0;
                }

                now -= BarB;
                now += i;
                BarB = i;
                DispEight(now, BarB);
                DrawRecruitProfile(173, 47, &Men[now], showStats);
                WaitForMouseUp();
            }

            if (((x >= 27 && y >= (131 + i * 8) && x <= 151 && y <= (137 + i * 8) && mousebuttons > 0)
                 || (key == LT_ARROW && ksel == 0)) && (now2 - BarA + i) <= (count - 1)) {
                // Left Select Box

                if (ksel == 0) {
                    ksel = 1;
                }

                now2 -= BarA;
                now2 += i;
                BarA = i;
                DispEight2(now2, BarA, count);
                DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);
                WaitForMouseUp();
            }
        }

        // else
        if ((key == K_HOME && ksel == 1) && count > 0) {
            /* Left Home */
            InBox(6, 130, 18, 161);

            BarA = 0;
            now2 = 0;
            DispEight2(now2, BarA, count);
            DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);

            delay(10);
            OutBox(6, 130, 18, 161);

        } else if ((key == K_PGUP && ksel == 1) && count > 0) {
            /* Left Page Up */
            InBox(6, 130, 18, 161);

            if (now2 > 0) {
                now2 = (now2 >= 8) ? now2 - 8 : 0;

                if (now2 < 8) {
                    BarA = now2;
                }

                DispEight2(now2, BarA, count);
                DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);
            }

            delay(10);
            OutBox(6, 130, 18, 161);

        } else if (((x >= 6 && y >= 130 && x <= 18 && y <= 161 && mousebuttons > 0) || (key == UP_ARROW && ksel == 1)) && count > 0) {
            /* Left Up */
            InBox(6, 130, 18, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 0) {
                        if (now2 > 0) {
                            now2--;
                            DispEight2(now2, BarA, count);
                            DrawRecruitProfile(
                                12, 47, &Men[sel[now2]], showStats);
                        }
                    }

                    if (BarA > 0) {
                        BarA--;
                        now2--;
                        DispEight2(now2, BarA, count);
                        DrawRecruitProfile(
                            12, 47, &Men[sel[now2]], showStats);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == UP_ARROW) {
                delay(100);

                if (BarA == 0) {
                    if (now2 > 0) {
                        now2--;
                        DispEight2(now2, BarA, count);
                        DrawRecruitProfile(
                            12, 47, &Men[sel[now2]], showStats);
                    }
                }

                if (BarA > 0) {
                    BarA--;
                    now2--;
                    DispEight2(now2, BarA, count);
                    DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);
                }

                key = 0;

                GetMouse();
            }

            //WaitForMouseUp();
            OutBox(6, 130, 18, 161);
            delay(10);
        } else if (((x >= 6 && y >= 163 && x <= 18 && y <= 194 && mousebuttons > 0) || (key == DN_ARROW && ksel == 1)) && count > 0) {
            /* Left Down */
            InBox(6, 163, 18, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarA == 7) {
                        if (now2 < count - 1) {
                            now2++;
                            DispEight2(now2, BarA, count);
                            DrawRecruitProfile(
                                12, 47, &Men[sel[now2]], showStats);
                        }
                    }

                    if (BarA < 7) {
                        if (now2 < count - 1) {
                            BarA++;
                            now2++;
                            DispEight2(now2, BarA, count);
                            DrawRecruitProfile(
                                12, 47, &Men[sel[now2]], showStats);
                        }
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == DN_ARROW) {
                delay(100);

                if (BarA == 7) {
                    if (now2 < count - 1) {
                        now2++;
                        DispEight2(now2, BarA, count);
                        DrawRecruitProfile(
                            12, 47, &Men[sel[now2]], showStats);
                    }
                }

                if (BarA < 7) {
                    if (now2 < count - 1) {
                        BarA++;
                        now2++;
                        DispEight2(now2, BarA, count);
                        DrawRecruitProfile(
                            12, 47, &Men[sel[now2]], showStats);
                    }
                }

                key = 0;

                GetMouse();
            }

            WaitForMouseUp();
            OutBox(6, 163, 18, 194);
            delay(10);

        } else if (key == K_PGDN && ksel == 1 && count > 0) {
            /* Left Page Down */
            InBox(6, 163, 18, 194);

            if (now2 < count - 1) {
                now2 = MIN(now2 + 8, count - 1);

                if (now2 > (count - 1) - 8) {
                    BarA = MIN(count - 1, 7 - ((count - 1) - now2));
                }

                DispEight2(now2, BarA, count);
                DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);
            }

            delay(10);
            OutBox(6, 163, 18, 194);

        } else if ((key == K_END && ksel == 1) && count > 0) {
            /* Left End */
            InBox(6, 163, 18, 194);

            BarA = MIN(count - 1, 7);
            now2 = count - 1;
            DispEight2(now2, BarA, count);
            DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);

            delay(10);
            OutBox(6, 163, 18, 194);

        } else if (key == K_HOME && ksel == 0) {
            /* Right Home */
            InBox(167, 130, 179, 161);

            BarB = 0;
            now = min;
            DispEight(now, BarB);
            DrawRecruitProfile(173, 47, &Men[now], showStats);

            delay(10);
            OutBox(167, 130, 179, 161);

        } else if (key == K_PGUP && ksel == 0) {
            /* Right Page Up */
            InBox(167, 130, 179, 161);

            if (now > min) {
                now = (now >= min + 8) ? now - 8 : min;

                if (now < min + 8) {
                    BarB = now - min;
                }

                DispEight(now, BarB);
                DrawRecruitProfile(173, 47, &Men[now], showStats);
            }

            delay(10);
            OutBox(167, 130, 179, 161);

        } else if ((x >= 167 && y >= 130 && x <= 179 && y <= 161 && mousebuttons > 0) || (key == UP_ARROW && ksel == 0)) {
            /* Right Up */
            InBox(167, 130, 179, 161);

            for (i = 0; i < 50; i++) {
                key = 0;
                ksel = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarB == 0) {
                        if (now > min) {
                            now--;
                            DispEight(now, BarB);
                            DrawRecruitProfile(
                                173, 47, &Men[now], showStats);
                        }
                    }

                    if (BarB > 0) {
                        BarB--;
                        now--;
                        DispEight(now, BarB);
                        DrawRecruitProfile(173, 47, &Men[now], showStats);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == UP_ARROW) {
                delay(100);

                if (BarB == 0) {
                    if (now > min) {
                        now--;
                        DispEight(now, BarB);
                        DrawRecruitProfile(173, 47, &Men[now], showStats);
                    }
                }

                if (BarB > 0) {
                    BarB--;
                    now--;
                    DispEight(now, BarB);
                    DrawRecruitProfile(173, 47, &Men[now], showStats);
                }

                key = 0;
                GetMouse();
            }

            // WaitForMouseUp();
            OutBox(167, 130, 179, 161);
            delay(10);

        } else if ((x >= 167 && y >= 163 && x <= 179 && y <= 194 && mousebuttons > 0) || (key == DN_ARROW && ksel == 0)) {
            /* Right Down */
            InBox(167, 163, 179, 194);

            for (i = 0; i < 50; i++) {
                key = 0;
                GetMouse();
                delay(10);

                if (mousebuttons == 0) {

                    if (BarB == 7) {
                        if (now <= max) {
                            if (now < max) {
                                now++;
                            }

                            DispEight(now, BarB);
                            DrawRecruitProfile(
                                173, 47, &Men[now], showStats);
                        }
                    }

                    if (BarB < 7) {
                        BarB++;
                        now++;
                        DispEight(now, BarB);
                        DrawRecruitProfile(173, 47, &Men[now], showStats);
                    }

                    i = 51;
                }
            }

            while (mousebuttons == 1 || key == DN_ARROW) {
                delay(100);

                if (BarB == 7) {
                    if (now <= max) {
                        if (now < max) {
                            now++;
                        }

                        DispEight(now, BarB);
                        DrawRecruitProfile(173, 47, &Men[now], showStats);
                    }
                }

                if (BarB < 7) {
                    BarB++;
                    now++;
                    DispEight(now, BarB);
                    DrawRecruitProfile(173, 47, &Men[now], showStats);
                }

                key = 0;

                GetMouse();
            }

            OutBox(167, 163, 179, 194);
            delay(10);

        } else if (key == K_PGDN && ksel == 0) {
            /* Right Page Down */
            InBox(167, 163, 179, 194);

            if (now < max) {
                now = (now < max - 8) ? now + 8 : max;

                if (now > max - 8) {
                    BarB = 7 - (max - now);
                }

                DispEight(now, BarB);
                DrawRecruitProfile(173, 47, &Men[now], showStats);
            }

            delay(10);
            OutBox(167, 163, 179, 194);

        } else if (key == K_END && ksel == 0) {
            /* Right End */
            InBox(167, 163, 179, 194);

            BarB = 7;
            now = max;
            DispEight(now, BarB);
            DrawRecruitProfile(173, 47, &Men[now], showStats);

            delay(10);
            OutBox(167, 163, 179, 194);

        } else if ((x >= 7 && y >= 111 && x <= 151 && y <= 123 && count > 0 && mousebuttons > 0) || (key == 'D' && count > 0)) {
            /* Dismiss */
            InBox(7, 111, 151, 123);
            ksel = 1;
            count--;
            MCol[sel[now2]] = 0;

            for (i = now2; i < count + 1; i++) {
                sel[i] = sel[i + 1];
            }

            sel[i] = -1;  /* remove astronaut from left */

            if (now2 == count) {
                if (now2 > 0) {
                    now2--;
                }

                if (BarA > 0) {
                    BarA--;
                }
            }

            DispEight2(now2, BarA, count);
            DrawRecruitProfile(
                12, 47, (count > 0) ? &Men[sel[now2]] : NULL, showStats);
            DispEight(now, BarB);
            DrawRecruitProfile(173, 47, &Men[now], showStats);

            WaitForMouseUp();

            if (key > 0) {
                delay(110);
            }

            fill_rectangle(292, 36, 301, 41, 7);
            display::graphics.setForegroundColor(11);
            draw_number(292, 41, MaxSel - count);

            OutBox(7, 111, 151, 123);

        } else if ((x >= 164 && y >= 111 && x <= 313 && y <= 123 && MCol[now] == 0 && mousebuttons > 0) || (key == 'R' && MCol[now] == 0)) {
            /* Recruit */
            InBox(168, 111, 313, 123);

            if (count < MaxSel) {
                sel[count] = now;  /* move astronaut into left */
                MCol[now] = 1;
                count++;

                if (BarB == 7) {
                    if (now < max) {
                        now++;
                        DispEight(now, BarB);
                        DrawRecruitProfile(173, 47, &Men[now], showStats);
                    }
                }

                if (BarB < 7) {
                    BarB++;
                    now++;
                    DispEight(now, BarB);
                    DrawRecruitProfile(173, 47, &Men[now], showStats);
                }

                DispEight2(now2, BarA, count);
                DrawRecruitProfile(12, 47, &Men[sel[now2]], showStats);

                fill_rectangle(292, 36, 310, 41, 7);
                display::graphics.setForegroundColor(11);
                draw_number(292, 41, MaxSel - count);
            }

            WaitForMouseUp();

            if (key > 0) {
                delay(110);
            }

            OutBox(168, 111, 313, 123);
        }

        if ((x >= 246 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {  /* Exit - not 'til done */
            bool femaleAstronautsSelected = false;

            if (femaleAstronautsRequired) {
                for (i = 0; i < count; i++) {
                    if (Men[sel[i]].Sex == 1) {
                        femaleAstronautsSelected = true;
                        break;
                    }
                }

                if (! femaleAstronautsSelected) {
                    InBox(246, 5, 314, 17);
                    Help("i100");
                    OutBox(246, 5, 314, 17);
                }
            }

            if (count != MaxSel) {
                Help("i045");
            }

            if ((! femaleAstronautsRequired || femaleAstronautsSelected) &&
                count == MaxSel) {
                InBox(246, 5, 314, 17);
                WaitForMouseUp();

                if (key > 0) {
                    delay(150);
                }

                for (i = 0; i < count; i++) {
                    Recruit(plr, i + Data->P[plr].AstroCount, sel[i]);
                }

                Data->P[plr].AstroLevel++;
                Data->P[plr].AstroCount += count;

                switch (Data->P[plr].AstroLevel) {
                case 1:
                    Data->P[plr].AstroDelay = 6;
                    break;

                case 2:
                case 3:
                    Data->P[plr].AstroDelay = 4;
                    break;

                case 4:
                    Data->P[plr].AstroDelay = 8;
                    break;

                case 5:
                    Data->P[plr].AstroDelay = 99;
                    break;
                }

                OutBox(246, 5, 314, 17);

                music_stop();

                return;  /* Done */
            }
        }

        if ((x >= 174 && y >= 5 && x <= 238 && y <= 17 && mousebuttons > 0) || key == K_ESCAPE) {  /* Cancel - and give the player a refund */
            InBox(174, 5, 238, 17);
            WaitForMouseUp();
            Data->P[plr].Cash += cost;
            OutBox(174, 5, 238, 17);
            return;  /* Cancel out */
        }
    }
}
