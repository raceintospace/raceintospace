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

// This file handles the Astronaut Complex / Cosmonaut Center.

#include "ast2.h"

#include <vector>

#include "display/graphics.h"

#include "ast0.h"
#include "ast3.h"
#include "ast4.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "game_main.h"
#include "gr.h"
#include "pace.h"
#include "place.h"
#include "sdlhelper.h"

namespace
{
enum LimboOperation {
    OP_TRANSFER = 0,
    OP_VISIT,
    OP_SENDALL
};
};


void DrawLimbo(char plr);
void Clear(void);
void LimboText(char plr, int astroIndex);


void DrawLimbo(char plr)
{
    FadeOut(2, 10, 0, 0);

    helpText = "i039";
    keyHelpText = "k039";
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(243, 3, 316, 19);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    if (plr == 0) {
        draw_heading(36, 5, "ASTRONAUT COMPLEX", 0, -1);
    } else {
        draw_heading(40, 5, "COSMONAUT CENTER", 0, -1);
    }

    ShBox(0, 24, 158, 199);
    InBox(9, 51, 90, 102);
    fill_rectangle(25, 129, 153, 195, 0);
    fill_rectangle(5, 129, 19, 195, 0);
    ShBox(6, 130, 18, 161);
    ShBox(6, 163, 18, 194);
    draw_up_arrow(8, 133);
    draw_down_arrow(8, 166);
    ShBox(161, 24, 319, 72);
    InBox(165, 28, 315, 42);
    fill_rectangle(166, 29, 314, 41, 7);
    ShBox(161, 74, 319, 199);
    InBox(165, 77, 315, 89);
    fill_rectangle(166, 78, 314, 88, 10);
    fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
    InBox(165, 46, 315, 67);
    fill_rectangle(166, 47, 314, 66, 0);
    ShBox(167, 48, 225, 65);  // Transfer Astronaut button
    ShBox(227, 48, 256, 65);  // Send All button
    ShBox(258, 48, 313, 65);  // Facility Transfer button
    InBox(4, 128, 20, 196);
    InBox(24, 128, 154, 196);
    // new boxes
    OutBox(0, 24, 158, 47);
    OutBox(0, 105, 158, 128);
    InBox(9, 28, 148, 42);
    InBox(9, 108, 148, 123);

    display::graphics.setForegroundColor(11);
    draw_string(172, 55, "T");
    display::graphics.setForegroundColor(20);
    draw_string(0, 0, "RANSFER");

    if (plr == 0) {
        draw_string(170, 62, "ASTRONAUT");
    } else {
        draw_string(170, 62, "COSMONAUT");
    }

    display::graphics.setForegroundColor(11);
    draw_string(230, 55, "S");
    display::graphics.setForegroundColor(20);
    draw_string(0, 0, "END");
    draw_string(233, 62, "ALL");

    display::graphics.setForegroundColor(11);
    draw_string(264, 55, "F");
    display::graphics.setForegroundColor(20);
    draw_string(0, 0, "ACILITY");
    draw_string(262, 62, "TRANSFER");
    
    
    for (int i = 0; i < 5; i++) {
        char str[21];

        IOBox(165, 93 + 21 * i, 238, 111 + 21 * i);
        IOBox(242, 93 + 21 * i, 315, 111 + 21 * i);

        if (Data->P[plr].Manned[i].Num < 0) {
            InBox(167, 95 + 21 * i, 236, 109 + 21 * i);
            display::graphics.setForegroundColor(17);
        } else {
            display::graphics.setForegroundColor(11);
        }

        assert(sizeof(Data->P[plr].Manned[i].Name) <= 20);
        snprintf(str, sizeof(str), "%s", Data->P[plr].Manned[i].Name);
        draw_string(201, 101 + 21 * i, str, ALIGN_CENTER);
        draw_string(181, 107 + 21 * i, "PROGRAM");

        display::graphics.setForegroundColor(11);
        draw_string(257, 107 + 21 * i, "TRAINING");
        // This remmed out to manually place the Adv. Training names
        // so they could be centered -Leon
        //grMoveTo(254,101+21*i);

        switch (i + AST_FOCUS_CAPSULE) {
        case AST_FOCUS_CAPSULE:
            draw_string(258, 101 + 21 * i, "CAPSULE");
            break;

        case AST_FOCUS_LEM:
            draw_string(271, 101 + 21 * i, "L.M.");
            break;

        case AST_FOCUS_EVA:
            draw_string(268, 101 + 21 * i, "E.V.A.");
            break;

        case AST_FOCUS_DOCKING:
            draw_string(259, 101 + 21 * i, "DOCKING");
            break;

        case AST_FOCUS_ENDURANCE:
            draw_string(252, 101 + 21 * i, "ENDURANCE");
            break;

        default:
            break;
        }
    }

    display::graphics.setForegroundColor(11);
    draw_string(187, 37, "OPERATION SELECTION");

    display::graphics.setForegroundColor(20);
    //draw_string(256,13,"CONTINUE");
    draw_string(15, 37, "NAME:");
    draw_string(17, 118, "GROUP:");
    draw_string(88, 118, "TENURE:");
    display::graphics.setForegroundColor(11);
    draw_string(102, 60, "MOOD:");
    draw_string(102, 68, "CAP:");
    draw_string(102, 76, "L.M.:");
    draw_string(102, 84, "EVA:");
    draw_string(102, 92, "DOCK:");
    draw_string(102, 100, "END:");
    draw_small_flag(plr, 4, 4);

    return;
}


void Limbo(char plr)
{
    int BarA, now2, tag ;
    std::vector<int> AstroList;

    DrawLimbo(plr);
    music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

    now2 = BarA = 0;

    for (int i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status == AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign == 0) {
            AstroList.push_back(i);
        }
    }
 
    DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

    FadeIn(2, 10, 0, 0);

    if (AstroList.size()) {
        InBox(167, 48, 225, 65);  // Default Transfer Astronaut button
        tag = OP_TRANSFER;
        fill_rectangle(166, 78, 314, 88, 10);
        display::graphics.setForegroundColor(11);
        draw_string(185, 85, "TRANSFER TO LOCATION");
        LimboText(plr, AstroList.at(now2));
    } else {
        InBox(258, 48, 313, 65);  // Default Facility transfer button 
        tag = OP_VISIT;
        fill_rectangle(166, 78, 314, 88, 10);
        display::graphics.setForegroundColor(11);
        draw_string(203, 85, "VISIT LOCATION");
        Help((plr == 0) ? "i176" : "i177");
    }

    while (1) {
        key = 0;
        WaitForMouseUp();
        key = 0;
        GetMouse();

        //Mouse ManSelect from being Clicked on
        if (AstroList.size()) {
	    for (int i = 0; i < 8; i++) {
	        if (x >= 27 && y >= (131 + i * 8) && x <= 151 
	        && y <= (137 + i * 8) && mousebuttons > 0 
	        && (now2 - BarA + i) <= (AstroList.size() - 1)) {
		        now2 -= BarA;
		        now2 += i;
		        BarA = i;
		        DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
		        LimboText(plr, AstroList.at(now2));
		        WaitForMouseUp();
	        }
	    }
	  }
        
        if (key == K_HOME) {
            InBox(6, 130, 18, 161);
            delay(10);

            now2 = 0;
            BarA = 0;
            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
            LimboText(plr, AstroList.at(now2));

            OutBox(6, 130, 18, 161);
            delay(10);
        }

        // Left Arrow Up
	if (AstroList.size()) {
	    if (mousebuttons > 0 && x >= 6 && y >= 130 && x <= 18 
	    && y <= 161 || key == UP_ARROW) {
	        InBox(6, 130, 18, 161);

	        for (int i = 0; i < 50; i++) {
		        key = 0;
		        GetMouse();
		        delay(10);

		        if (mousebuttons == 0) {

			        if (BarA == 0 && now2 > 0) {
				        now2--;
				        DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
				        LimboText(plr, AstroList.at(now2));
			        }

			        if (BarA > 0) {
				        BarA--;
				        now2--;
				        DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
				        LimboText(plr, AstroList.at(now2));
			        }
			        i = 51;
		        }
	        }

	        while (mousebuttons == 1 || key == UP_ARROW) {
		        delay(100);

		        if (BarA == 0 && now2 > 0) {
			        now2--;
			        DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
			        LimboText(plr, AstroList.at(now2));
		        }

		        if (BarA > 0) {
			        BarA--;
			        now2--;
			        DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
			        LimboText(plr, AstroList.at(now2));
		        }

		        key = 0;

		        GetMouse();
	        }

	        //WaitForMouseUp();key=0;
	        OutBox(6, 130, 18, 161);
	        delay(10);
		}
	}
		
        if (key == K_PGUP) {
            InBox(6, 130, 18, 161);
            delay(10);

            now2 = MAX(now2 - 7, 0);
            BarA = 0;
            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
            LimboText(plr, AstroList.at(now2));

            OutBox(6, 130, 18, 161);
            delay(10);
        }

        if (key == K_PGDN) {
            InBox(6, 163, 18, 194);
            delay(10);

            now2 = MIN(now2 + 7, AstroList.size() - 1);
            BarA = MIN(AstroList.size() - 1, 7);
            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
            LimboText(plr, AstroList.at(now2));

            OutBox(6, 163, 18, 194);
            delay(10);
        }

        if (key == K_END) {
            InBox(6, 163, 18, 194);
            delay(10);

            now2 = AstroList.size() - 1;
            BarA = MIN(AstroList.size() - 1, 7);
            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
            LimboText(plr, AstroList.at(now2));

            OutBox(6, 163, 18, 194);
            delay(10);
        }

        // Left Arrow Down
	if (AstroList.size()) {
	    if ((mousebuttons > 0 && x >= 6 && y >= 163 && x <= 18 && y <= 194) 
	    || key == DN_ARROW) {
	        InBox(6, 163, 18, 194);
	        for (int i = 0; i < 50; i++) {
		        key = 0;
		        GetMouse();
		        delay(10);

		        if (mousebuttons == 0) {
		            if (BarA == 7 && (now2 < AstroList.size() - 1)) {
			            now2++;
			            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
			            LimboText(plr, AstroList.at(now2));
		            }

		            if (BarA < 7 && now2 < AstroList.size() - 1) {
			            BarA++;
			            now2++;
			            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
			            LimboText(plr, AstroList.at(now2));
		            }

		            i = 51;
		        }
	        }

	        while (mousebuttons == 1 || key == DN_ARROW) {
	            delay(100);

	            if (BarA == 7 && (now2 < AstroList.size() - 1)) {
		            now2++;
		            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
		            LimboText(plr, AstroList.at(now2));
	            }

	            if (BarA < 7 && now2 < AstroList.size() - 1) {
		            BarA++;
		            now2++;
		            DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());
		            LimboText(plr, AstroList.at(now2));
	            }

	            key = 0;
	            GetMouse();
	        }

	        OutBox(6, 163, 18, 194);
	        delay(10);
	    }
	}

        // Continue
        if ((mousebuttons > 0 && x >= 245 && y >= 5 && x <= 314 && y <= 17) 
        || key == K_ENTER || key == K_ESCAPE) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);
            music_stop();
            return;
        }

        // Select Transfer Button
        if (AstroList.size()) {
            if ((mousebuttons > 0 && x >= 167 && y >= 48 && x <= 225 && y <= 65) || key == 'T') {
                InBox(167, 48, 225, 65);
                OutBox(227, 48, 256, 65);
                OutBox(258, 48, 313, 65);

                if (tag == OP_SENDALL) {
                    for (int i = 0; i < 5; i++) {
                        OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                    }
                }

                tag = OP_TRANSFER;
                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(185, 85, "TRANSFER TO LOCATION");
            }
        }
        
        // Select Visit To Button
        if ((mousebuttons > 0 && x >= 256 && y >= 48 && x <= 313 && y <= 65) || key == 'F') {
            InBox(258, 48, 313, 65);
            OutBox(227, 48, 256, 65);
            OutBox(167, 48, 225, 65);

            if (tag == OP_SENDALL) {
                for (int i = 0; i < 5; i++) {
                    OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                }
            }

            tag = OP_VISIT;
            fill_rectangle(166, 78, 314, 88, 10);
            display::graphics.setForegroundColor(11);
            draw_string(203, 85, "VISIT LOCATION");
        }

        // Select Send All Button
        if (AstroList.size()){
            if ((mousebuttons > 0 && x >= 227 && y >= 48 && x <= 256 && y <= 65) || key == 'S') {
                InBox(227, 48, 256, 65);
                OutBox(167, 48, 225, 65);
                OutBox(258, 48, 313, 65);

                for (int i = 0; i < 5; i++) {
                    InBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                }

                tag = OP_SENDALL;
                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(187, 85, "SEND ALL TO LOCATION");
            }
        }



        // Selection Loops
        for (int i = 0; i < 5; i++) {
            // Program Transfer
            if ((tag == OP_TRANSFER && Data->P[plr].Manned[i].Num >= 0) &&
            ((mousebuttons > 0 && x >= 167 && y >= (95 + 21 * i) && x <= 236 
            && y <= (109 + 21 * i)) || key == 0x0030 + i)) {
                InBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (AstroList.size() > 0) {
                    Data->P[plr].Pool[AstroList.at(now2)].Assign = i + 1;
                    Data->P[plr].Pool[AstroList.at(now2)].Unassigned = 0;
                    Data->P[plr].Pool[AstroList.at(now2)].Moved = 0;

                    AstroList.erase(AstroList.begin() + now2);
                }

                if (now2 == AstroList.size()) {
                    if (now2 > 0) {
                        now2--;
                    }

                    if (BarA > 0) {
                        BarA--;
                    }
                }

                DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

                if (AstroList.size() > 0) {
                    LimboText(plr, AstroList.at(now2));
                } else {
                    fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
                    Clear();
                }
            }

            // Program Visit
            if ((tag == OP_VISIT && Data->P[plr].Manned[i].Num >= 0) &&
                ((mousebuttons > 0 && x >= 167 && y >= (95 + 21 * i) && x <= 236 && y <= (109 + 21 * i))
                 || key == 0x0030 + i)) {
                InBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(167, 95 + 21 * i, 236, 109 + 21 * i);
                music_stop();
                Programs(plr, i + 1);
                DrawLimbo(plr);
                music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

                InBox(258, 48, 313, 65);
                OutBox(227, 48, 256, 65);
                OutBox(167, 48, 225, 65);
                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(203, 85, "VISIT LOCATION");
                now2 = BarA = 0;

                DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

                if (AstroList.size() > 0) {
                    LimboText(plr, AstroList.at(now2));
                }

                FadeIn(2, 10, 0, 0);
            }

            // Training Transfer
            if ((tag == OP_TRANSFER && AstroList.size() > 0) &&
            ((mousebuttons > 0 && x >= 244 && y >= (95 + 21 * i) && x <= 313 && y <= (109 + 21 * i)) 
            || key == 0x0035 + i)) {
                InBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                int skilLev = 0;

                // Figure out relevant skill level before sending to Adv Training -Leon
                if (i == 0) {
                    skilLev = Data->P[plr].Pool[AstroList.at(now2)].Cap;
                }

                if (i == 1) {
                    skilLev = Data->P[plr].Pool[AstroList.at(now2)].LM;
                }

                if (i == 2) {
                    skilLev = Data->P[plr].Pool[AstroList.at(now2)].EVA;
                }

                if (i == 3) {
                    skilLev = Data->P[plr].Pool[AstroList.at(now2)].Docking;
                }

                if (i == 4) {
                    skilLev = Data->P[plr].Pool[AstroList.at(now2)].Endurance;
                }

                display::graphics.setForegroundColor(2);

                if (Data->P[plr].Pool[AstroList.at(now2)].TrainingLevel > 6) {
                    Help("i120");
                } else if (skilLev > 3) {
                    // If they have a 4 in that skill, don't send to Adv Training  -Leon
                    OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                } else if (Data->P[plr].Cash < 3) {
                    Help("i121");
                } else {
                    if (skilLev > 2) {
                        // If they have a 3 in that skill, send them
                        // directly to Adv III and charge just 2MB  -Leon
                        Data->P[plr].Pool[AstroList.at(now2)].Status = AST_ST_TRAIN_ADV_3;
                        Data->P[plr].Cash -= 2;
                    } else {
                        Data->P[plr].Pool[AstroList.at(now2)].Status = AST_ST_TRAIN_ADV_1;
                        Data->P[plr].Cash -= 3;
                    }

                    Data->P[plr].Pool[AstroList.at(now2)].Focus = i + 1;
                    Data->P[plr].Pool[AstroList.at(now2)].Assign = 0;
                    Data->P[plr].Pool[AstroList.at(now2)].Unassigned = 0;
                    Data->P[plr].Pool[AstroList.at(now2)].Moved = 0;

                    AstroList.erase(AstroList.begin() + now2);

                    if (now2 == AstroList.size()) {
                        if (now2 > 0) {
                            now2--;
                        }

                        if (BarA > 0) {
                            BarA--;
                        }
                    }

                    DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

                    if (AstroList.size() > 0) {
                        LimboText(plr, AstroList.at(now2));
                    } else {
                        // Clear portrait display area
                        fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
                        Clear();
                    }
                }  // else
            }  // if adv training

            // Training Visit
            if (tag == OP_VISIT &&
            ((mousebuttons > 0 && x >= 244 && y >= (95 + 21 * i) && x 
            <= 313 && y <= (109 + 21 * i)) || key == 0x0035 + i)) {
                InBox(244, 95 + 21 * i, 313, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;
                OutBox(244, 95 + 21 * i, 313, 109 + 21 * i);
                music_stop();
                Train(plr, i + 1);
                DrawLimbo(plr);
                music_start((plr == 0) ? M_ASTTRNG : M_ASSEMBLY);

                InBox(258, 48, 313, 65);
                OutBox(230, 48, 256, 65);
                OutBox(167, 48, 225, 65);
                fill_rectangle(166, 78, 314, 88, 10);
                display::graphics.setForegroundColor(11);
                draw_string(203, 85, "VISIT LOCATION");
                now2 = BarA = 0;

                DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

                if (AstroList.size() > 0) {
                    LimboText(plr, AstroList.at(now2));
                }

                FadeIn(2, 10, 0, 0);
            }

            // Transfer all eligible astronauts/cosmonauts to a program
            if ((tag == OP_SENDALL && Data->P[plr].Manned[i].Num >= 0) &&
                ((mousebuttons > 0 && x >= 167 && y >= (95 + 21 * i) && x <= 236 && y <= (109 + 21 * i))
                 || key == 0x0030 + i)) {
                InBox(167, 95 + 21 * i, 236, 109 + 21 * i);

                if (key > 0) {
                    delay(140);
                }

                WaitForMouseUp();
                key = 0;

                for (auto astroIndex : AstroList) {
                    Astros &astro = Data->P[plr].Pool[astroIndex];

                    if (astro.Mood >= 20 && astro.RetirementDelay == 0) {
                        astro.Assign = i + 1;
                        astro.Unassigned = 0;
                        astro.Moved = 0;
                    }
                }

                AstroList.clear();

                for (int i = 0; i < Data->P[plr].AstroCount; i++) {
                    if (Data->P[plr].Pool[i].Status == AST_ST_ACTIVE &&
                        Data->P[plr].Pool[i].Assign == 0) {
                        AstroList.push_back(i);
                    }
                }

                // Trying to keep the selection bar in the same place
                // is tricky, so just reset it for now.
                now2 = BarA = 0;
                DispLeft(plr, BarA, AstroList.size(), now2, AstroList.data());

                if (AstroList.size() > 0) {
                    LimboText(plr, AstroList.at(now2));
                } else {
                    fill_rectangle(10, 52, 89, 101, 7 + plr * 3);
                    Clear();
                }

                OutBox(167, 95 + 21 * i, 236, 109 + 21 * i);
            }
        } /* end Selection Loops*/

    }  /* end while */
}  /* end Limbo */

/**
 * Clear the Astronaut profile display fast.
 */
void Clear(void)
{
    fill_rectangle(44, 31, 145, 40, 3);
    fill_rectangle(49, 112, 65, 119, 3);
    fill_rectangle(123, 62, 145, 77, 3);
    fill_rectangle(125, 79, 145, 85, 3);
    fill_rectangle(128, 86, 145, 93, 3);
    fill_rectangle(123, 95, 145, 101, 3);
    fill_rectangle(127, 113, 141, 120, 3);
    fill_rectangle(130, 54, 155, 61, 3);
    return;
}


void LimboText(char plr, int astroIndex)
{
    struct Astros &astro = Data->P[plr].Pool[astroIndex];
    int color = MoodColor(astro.Mood);

    Clear();

    // Print 'naut name in green/yellow/red/black depending on mood -Leon
    display::graphics.setForegroundColor(color);

    // Print name in black if 'naut has announced retirement (override mood) - Leon
    if (astro.RetirementDelay > 0) {
        display::graphics.setForegroundColor(0);
    }

    draw_string(46, 37, astro.Name);
    display::graphics.setForegroundColor(11);

    switch (astro.Group) {
    case 0:
        draw_string(53, 118, "I");
        break;

    case 1:
        draw_string(53, 118, "II");
        break;

    case 2:
        draw_string(53, 118, "III");
        break;

    case 3:
        draw_string(53, 118, "IV");
        break;

    case 4:
        draw_string(53, 118, "V");
        break;
    }

    // Print 'naut mood in green/yellow/red/black depending on mood -Leon
    display::graphics.setForegroundColor(color);
    draw_number(132, 60, astro.Mood);
    display::graphics.setForegroundColor(11);
    draw_number(125, 68, astro.Cap);
    draw_number(123, 76, astro.LM);
    draw_number(125, 84, astro.EVA);
    draw_number(131, 92, astro.Docking);
    draw_number(125, 100, astro.Endurance);
    draw_number(130, 118, astro.Active);
    AstFaces(plr, 10, 52, astro.Face);
    return;
}


// EOF
