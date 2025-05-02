/**
 * Copyright (C) 2005 Michael K. McCarty & Fritz Bronner,
 * Copyright (C) 2020 Hendrik Weimer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty

// This file handles play by e-mail.

#include "pbm.h"

#include "admin.h"
#include "data.h"
#include "game_main.h"
#include "pace.h"
#include "review.h"

/* Show the prestige results of all the missions perfomed in the
 * previous turn.
 */
void ShowPrestigeResults(char plr)
{
    int i, turn, mturn, old_mail;

    turn = 2 * (Data->Year - 57) + Data->Season;

    for (i = 0; i < Data->P[plr].PastMissionCount; i++) {
        // Spring: Jan to Jun
        // Fall: Jul to Dec

        mturn = (2 * (Data->P[plr].History[i].MissionYear - 57));

        if (Data->P[plr].History[i].Month > 5) {
            mturn++;
        }

        if (turn == mturn + 1) {
            old_mail = MAIL;
            MAIL = -1; // Don't want to show prestige as "PENDING"
            MisRev(plr, Data->P[plr].History[i].Prestige, i);
            MAIL = old_mail;
        }
    }
}

/* Saves the current game. Only fades out if the game is not a PBEM
 * game.
 */
void MailSwitchPlayer(void)
{
    if (MAIL != -1) {
        FileAccess(2);
    }

    FadeOut(2, 10, 0, 0);
}

/* Immediately hand over to the other player during the endgame. MAIL
   must be either 1 or 2 to show endgame. */
void MailSwitchEndgame(void)
{
    switch (MAIL) {
    case 0:
        MAIL = 1;
        break;

    case 1:
        MAIL = 2;
        break;

    case 2:
        MAIL = 1;
        break;

    case 3:
        MAIL = 2;
        break;
    }

    MailSwitchPlayer();
}
