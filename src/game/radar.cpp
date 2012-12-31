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

#include "radar.h"
#include "gamedata.h"
#include "draw.h"
#include "Buzz_inc.h"
#include "future.h"
#include "game_main.h"
#include "news_suq.h"
#include "place.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"

void PadDraw(char plr, char pad);
void PadPict(char poff);


void PadDraw(char plr, char pad)
{
    int i, j, k, l;
    int missions;     // Variable for how many missions each 'naut has flown

    FadeOut(2, display::graphics.palette(), 10, 0, 0);
    display::graphics.screen()->clear(0);
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 319, 198);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    InBox(167, 27, 316, 176);
    fill_rectangle(168, 28, 315, 175, 0);
    IOBox(167, 179, 316, 195);
    ShBox(4, 28, 162, 43);
    InBox(6, 30, 160, 41);
    ShBox(4, 46, 162, 61);
    InBox(6, 48, 160, 59);
    ShBox(4, 68, 162, 97);
    InBox(6, 70, 160, 95);
    ShBox(56, 64, 110, 74);
    InBox(57, 65, 109, 73);
    ShBox(4, 180, 162, 195);
    InBox(6, 182, 160, 193); //sched. duration
    InBox(6, 99, 160, 178);
    display::graphics.setForegroundColor(9);
    draw_string(18, 190, "SCHEDULED DURATION: ");
    display::graphics.setForegroundColor(7);
    int MisCod;
    MisCod = Data->P[plr].Mission[pad].MissionCode;

    if ((MisCod > 24 && MisCod < 37) || MisCod == 40 || MisCod == 41 || MisCod == 43 || MisCod == 44 || MisCod > 45)
        // Show Duration level for manned missions with Duration steps (this keeps the Mission[pad].Duration
        // variable from continuing to show Duration level if mission is scrubbed or downgraded) - Leon
    {
        switch (Data->P[plr].Mission[pad].Duration) {
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
            draw_string(0, 0, "NONE");
            break;
        }
    } else {
        if (Data->P[plr].Mission[pad].PCrew - 1 >= 0 || Data->P[plr].Mission[pad].BCrew - 1 >= 0) {
            draw_string(0, 0, "A");
        } else {
            draw_string(0, 0, "NONE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(65, 71, "MISSION");
    draw_small_flag(plr, 4, 4);

    if (Data->P[plr].LaunchFacility[pad] == 1 && Data->P[plr].Mission[pad].MissionCode) {
        PadPict(2 + plr);
    } else if (Data->P[plr].LaunchFacility[pad] == 1 && Data->P[plr].Mission[pad].MissionCode == Mission_None) {
        PadPict(4 + plr);
    } else if (Data->P[plr].LaunchFacility[pad] > 1) {
        PadPict(0 + plr);
    }

    display::graphics.setForegroundColor(1);
    draw_string(15, 37, "STATUS: ");
    display::graphics.setForegroundColor(9);

    if (Data->P[plr].LaunchFacility[pad] == 1) {
        draw_string(0, 0, "OPERATIONAL");
    } else {
        draw_string(0, 0, "DESTROYED");
    }

    display::graphics.setForegroundColor(1);

    if (Data->P[plr].LaunchFacility[pad] > 1) {
        draw_string(15, 56, "REPAIR COST: ");
        display::graphics.setForegroundColor(9);
        draw_number(0, 0, Data->P[plr].LaunchFacility[pad]);
        draw_string(0, 0, "MB");

        if (Data->P[plr].Cash < Data->P[plr].LaunchFacility[pad]) {
            InBox(169, 181, 314, 193);
        }
    } else {
        display::graphics.setForegroundColor(9);

        if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
            draw_string(15, 56, "NO LAUNCH SCHEDULED");
            InBox(169, 181, 314, 193);
        } else {
            draw_string(15, 56, "PRE-MISSION CHECK");
        }
    }

    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    if (Data->P[plr].LaunchFacility[pad] == 1) {
        display::graphics.setForegroundColor(9);
        draw_string(210, 189, "S");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "CRUB MISSION");
    } else {
        display::graphics.setForegroundColor(9);
        draw_string(205, 189, "F");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "IX LAUNCH PAD");
    }

    draw_heading(37, 5, "LAUNCH FACILITY", 0, -1);

    switch (pad) {
    case 0:
        draw_heading(201, 5, "A", 0, -1);
        break;

    case 1:
        draw_heading(201, 5, "B", 0, -1);
        break;

    case 2:
        draw_heading(201, 5, "C", 0, -1);
        break;
    }

    display::graphics.setForegroundColor(6);
    MissionName(Data->P[plr].Mission[pad].MissionCode, 11, 81, 20);
    display::graphics.setForegroundColor(1);

    // joint mission part
    if (Data->P[plr].Mission[pad].Joint == 1) {
        ShBox(38, 91, 131, 101);
        InBox(39, 92, 130, 100);
        display::graphics.setForegroundColor(1);

        if (Data->P[plr].Mission[pad].part == 0) {
            draw_string(53, 98, "PRIMARY PART");
        } else {
            draw_string(44, 98, "SECONDARY PART");
        }
    }

    // Hardware to Use

    i = Data->P[plr].Mission[pad].Prog;
    j = Data->P[plr].Mission[pad].PCrew - 1;
    l = Data->P[plr].Mission[pad].BCrew - 1;

    // Crews
    display::graphics.setForegroundColor(7);
    draw_string(13, 107, "PRIMARY CREW  ");

    if (j >= 0) {
        display::graphics.setForegroundColor(11); // Now display the crew number, for player's easy reference - Leon

        if (j == 0) {
            draw_string(0, 0, "(CREW I)");
        }

        if (j == 1) {
            draw_string(0, 0, "(CREW II)");
        }

        if (j == 2) {
            draw_string(0, 0, "(CREW III)");
        }

        if (j == 3) {
            draw_string(0, 0, "(CREW IV)");
        }

        if (j == 4) {
            draw_string(0, 0, "(CREW V)");
        }

        if (j == 5) {
            draw_string(0, 0, "(CREW VI)");
        }

        if (j == 6) {
            draw_string(0, 0, "(CREW VII)");
        }

        if (j == 7) {
            draw_string(0, 0, "(CREW VIII)");
        }

        for (k = 0; k < Data->P[plr].CrewCount[i][j]; k++) {
            // Draw a morale box for each crew member - Leon
            display::graphics.setForegroundColor(1);
            fill_rectangle(13, 110 + 7 * k, 20, 110 + 7 * k, 2); // Top
            fill_rectangle(13, 110 + 7 * k, 13, 116 + 7 * k, 2); // Left
            fill_rectangle(13, 116 + 7 * k, 20, 116 + 7 * k, 4); // Bottom
            fill_rectangle(21, 110 + 7 * k, 21, 116 + 7 * k, 4); // Right

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood >= 65) {
                fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, 16);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood < 65 && Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood >= 40) {
                fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, 11);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood < 40 && Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood >= 20) {
                fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, 8);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood < 20) {
                fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, 0);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Mood == 0) {
                fill_rectangle(14, 111 + 7 * k, 20, 115 + 7 * k, 3);
            }

            display::graphics.setForegroundColor(1);

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Sex == 1) {
                display::graphics.setForegroundColor(5);    // Show female 'nauts in blue
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(0);    // Show anyone who's announced retirement in black
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Sex == 1 && Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(7);
            }

            // Show name in purple if 'naut is female AND has announced retirement
            draw_string(25, 115 + 7 * k, &Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Name[0]);
            missions = Data->P[plr].Pool[Data->P[plr].Crew[i][j][k] - 1].Missions;

            if (missions > 0) {
                draw_string(0, 0, " (");
                draw_number(0, 0, missions);
                draw_string(0, 0, ")");
            }
        }

        if (l == -1) {
            draw_string(25, 174, "UNAVAILABLE");
        }
    }

    display::graphics.setForegroundColor(7);
    draw_string(13, 145, "BACKUP CREW  ");

    if (l >= 0) {
        display::graphics.setForegroundColor(11); // Now display the crew number, for player's easy reference - Leon

        if (l == 0) {
            draw_string(0, 0, "(CREW I)");
        }

        if (l == 1) {
            draw_string(0, 0, "(CREW II)");
        }

        if (l == 2) {
            draw_string(0, 0, "(CREW III)");
        }

        if (l == 3) {
            draw_string(0, 0, "(CREW IV)");
        }

        if (l == 4) {
            draw_string(0, 0, "(CREW V)");
        }

        if (l == 5) {
            draw_string(0, 0, "(CREW VI)");
        }

        if (l == 6) {
            draw_string(0, 0, "(CREW VII)");
        }

        if (l == 7) {
            draw_string(0, 0, "(CREW VIII)");
        }

        for (k = 0; k < Data->P[plr].CrewCount[i][l]; k++) {
            // Draw a morale box for each crew member - Leon
            display::graphics.setForegroundColor(1);
            fill_rectangle(13, 148 + 7 * k, 20, 148 + 7 * k, 2); // Top
            fill_rectangle(13, 148 + 7 * k, 13, 154 + 7 * k, 2); // Left
            fill_rectangle(13, 154 + 7 * k, 20, 154 + 7 * k, 4); // Bottom
            fill_rectangle(21, 148 + 7 * k, 21, 154 + 7 * k, 4); // Right

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood >= 65) {
                fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, 16);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood < 65 && Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood >= 40) {
                fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, 11);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood < 40 && Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood >= 20) {
                fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, 8);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood < 20) {
                fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, 0);
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Mood == 0) {
                fill_rectangle(14, 149 + 7 * k, 20, 153 + 7 * k, 3);
            }

            display::graphics.setForegroundColor(1);

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Sex == 1) {
                display::graphics.setForegroundColor(5);    // Show female 'nauts in blue
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(0);    // But show anyone who's announced retirement in black
            }

            if (Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Sex == 1 && Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].RetirementDelay > 0) {
                display::graphics.setForegroundColor(7);
            }

            // Show name in purple if 'naut is female AND has announced retirement
            draw_string(25, 153 + 7 * k, &Data->P[plr].Pool[Data->P[plr].Crew[i][l][k] - 1].Name[0]);
        }

        if (j == -1) {
            draw_string(25, 136, "UNAVAILABLE");
        }
    }

    if (Data->P[plr].Mission[pad].Prog > 0)
        PatchMe(plr, 126, 40, Data->P[plr].Mission[pad].Prog - 1,
                Data->P[plr].Mission[pad].Patch, 32);

    FadeIn(2, display::graphics.palette(), 10, 0, 0);

    return;
}


/** pad is +3 for compatibility with News so that
 * it will go ahead and scrub the mission automatically
 *
 */
void ClrMiss(char plr, char pad)
{
    char prime, back, men, i, prg, temp = 0, padd;
    padd = pad % 3;

//pad joint first part/second part/single
    if (Data->P[plr].Mission[padd].Joint == 0) {
        if (!AI[plr] && pad < 3) {
            temp = Help("i111");
        }
    } else
        switch (padd) {
        case 0:
            if (!AI[plr] && pad < 3) {
                temp = Help("i110");
            }

            break;

        case 1:
            if (Data->P[plr].Mission[1].Joint == 1) {
                if (Data->P[plr].Mission[0].Joint == 1) {
                    if (!AI[plr] && pad < 3) {
                        temp = Help("i112");
                    }

                    padd = 0;
                } else {
                    if (!AI[plr] && pad < 3) {
                        temp = Help("i110");
                    }
                }
            }

            break;

        case 2:
            padd = 1;

            if (!AI[plr] && pad < 3) {
                temp = Help("i112");
            }

            break;

        default:
            temp = 0;
            break;
        }

    if (AI[plr]) {
        temp = 1;
    }

    if (temp == -1) {
        return;
    }

    prg = Data->P[plr].Mission[padd].Prog;
    Data->P[plr].Mission[padd].Hard[Mission_PrimaryBooster] = 0;

    if (Data->P[plr].Mission[padd].PCrew != 0) {
        prime = Data->P[plr].Mission[padd].PCrew - 1;
    } else {
        prime = -1;
    }

    if (Data->P[plr].Mission[padd].BCrew != 0) {
        back = Data->P[plr].Mission[padd].BCrew - 1;
    } else {
        back = -1;
    }

    men = Data->P[plr].Mission[padd].Men;

    if (prime != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 0;
        }

    if (back != -1)
        for (i = 0; i < men; i++) {
            Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 0;
        }

    if (Data->P[plr].Mission[padd].Joint == 1) {
        prg = Data->P[plr].Mission[padd + 1].Prog;

        if (Data->P[plr].Mission[padd + 1].PCrew != 0) {
            prime = Data->P[plr].Mission[padd + 1].PCrew - 1;
        } else {
            prime = -1;
        }

        if (Data->P[plr].Mission[padd + 1].BCrew != 0) {
            back = Data->P[plr].Mission[padd + 1].BCrew - 1;
        } else {
            back = -1;
        }

        men = Data->P[plr].Mission[padd + 1].Men;

        if (prime != -1)
            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][prime][i] - 1].Prime = 0;
            }

        if (back != -1)
            for (i = 0; i < men; i++) {
                Data->P[plr].Pool[Data->P[plr].Crew[prg][back][i] - 1].Prime = 0;
            }

        Data->P[plr].Mission[padd + 1].part = 0;
        Data->P[plr].Mission[padd + 1].Prog = 0;
        Data->P[plr].Mission[padd + 1].PCrew = 0;
        Data->P[plr].Mission[padd + 1].BCrew = 0;
        Data->P[plr].Mission[padd + 1].Joint = 0;
        Data->P[plr].Mission[padd + 1].Men = 0;
        Data->P[plr].Mission[padd + 1].MissionCode = Mission_None;
    }

    Data->P[plr].Mission[padd].Prog = 0;
    Data->P[plr].Mission[padd].PCrew = 0;
    Data->P[plr].Mission[padd].BCrew = 0;
    Data->P[plr].Mission[padd].Men = 0;
    Data->P[plr].Mission[padd].Joint = 0;
    Data->P[plr].Mission[padd].MissionCode = Mission_None;

    if (Data->P[plr].Mission[padd].Joint == 1 && Data->P[plr].Mission[padd].part == 0) {
        memset(&Data->P[plr].Mission[padd + 1], 0x00, sizeof(struct MissionType));
    }

    if (Data->P[plr].Mission[padd + 1].Joint == 1 && Data->P[plr].Mission[padd + 1].part == 1) {
        memset(&Data->P[plr].Mission[padd + 1], 0x00, sizeof(struct MissionType));
    }

    return;
}


void PadPict(char poff)
{
    SimpleHdr table;
    FILE *in;
    in = sOpen("LFACIL.BUT", "rb", 0);
    fread_SimpleHdr(&table, 1, in);
    fseek(in, 6 * sizeof_SimpleHdr, SEEK_SET);
    fread(display::graphics.palette(), 768, 1, in);
    fseek(in, table.offset, SEEK_SET);
    fread(buffer, table.size, 1, in);
    display::Surface local(148, 148);
    display::Surface local2(148, 148);
    RLED_img(buffer, local.pixels(), table.size, local.width(), local.height());
    fseek(in, (poff)*sizeof_SimpleHdr, SEEK_SET);
    fread_SimpleHdr(&table, 1, in);
    fseek(in, table.offset, SEEK_SET);
    fread(buffer, table.size, 1, in);
    RLED_img(buffer, local2.pixels(), table.size, local2.width(), local2.height());

    local2.copyTo(display::graphics.screen(), 168, 28);

    fclose(in);
}


void ShowPad(char plr, char pad)
{
    char temp;
    music_start((plr == 1) ? M_USMIL : M_USSRMIL);
    PadDraw(plr, pad);
    temp = CheckCrewOK(plr, pad);

    if (temp == 1) { //found mission no crews

        ClrMiss(plr, pad + 3);
        return;
    }

    helpText = "i028";
    keyHelpText = "k028";
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();

        if ((Data->P[plr].LaunchFacility[pad] == 1 && x >= 169 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0 && Data->P[plr].Mission[pad].MissionCode)
            || (Data->P[plr].LaunchFacility[pad] == 1 && Data->P[plr].Mission[pad].MissionCode && key == 'S')) {
            // Scrub Mission
            InBox(169, 181, 314, 193);
            key = 0;
            WaitForMouseUp();
            ClrMiss(plr, pad);
            OutBox(169, 181, 314, 193);
            key = 0;

            if (Data->P[plr].Mission[pad].MissionCode == Mission_None) {
                return;
            }
        } else if ((Data->P[plr].LaunchFacility[pad] <= Data->P[plr].Cash && Data->P[plr].LaunchFacility[pad] > 1 && x >= 169 && y >= 181 && x <= 314 && y <= 193 && mousebuttons > 0)
                   || (key == 'F' && Data->P[plr].LaunchFacility[pad] > 1 && Data->P[plr].LaunchFacility[pad] <= Data->P[plr].Cash)) {
            // Scrub Mission
            InBox(169, 181, 314, 193);
            key = 0;
            WaitForMouseUp();

            if (Data->P[plr].Cash >= Data->P[plr].LaunchFacility[pad]) {
                temp = Help("i115");

                if (temp == 1) {
                    Data->P[plr].Cash -= Data->P[plr].LaunchFacility[pad];
                    Data->P[plr].LaunchFacility[pad] = 1;
                }
            } else {
                Help("i116");
            }

            OutBox(169, 181, 314, 193);
            key = 0;

            if (temp == 1) {
                return;
            }
        } else if ((x >= 245 && y >= 5 && x <= 314 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(245, 5, 314, 17);
            key = 0;
            WaitForMouseUp();
            OutBox(245, 5, 314, 17);

            if (key > 0) {
                delay(150);
            }

            key = 0;
            return;  /* Done */
        };
    }
}

// EOF
