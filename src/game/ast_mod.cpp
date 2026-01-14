
// This page handles editing the Custom Roster

#include "ast_mod.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "display/graphics.h"

#include "data.h"
#include "draw.h"
#include "fs.h"
#include "game_main.h"
#include "ioexception.h"
#include "logging.h"
#include "macros.h"
#include "pace.h"
#include "place.h"
#include "proto.h"


namespace
{
const int MAX_SKILL_RATING = 4;
const int MIN_SKILL_RATING = 0;
const int ROSTER_SIZE = 106;

enum SkillSelection {
    SKILL_CAP = 0,
    SKILL_LM,
    SKILL_EVA,
    SKILL_DOCK,
    SKILL_ENDR,
    SKILL_NONE
};

enum EditorMode {
    EDITOR_NONE = 0,
    EDITOR_NAME,
    EDITOR_SKILLS
};


bool Clicked(int x1, int y1, int x2, int y2);
void DisplayRecruit(int plr, const struct ManPool &recruit);
void DisplayRoster(int plr, int selected, int bar,
                   const std::vector<struct ManPool> &roster);
void DisplaySkillRating(int rating);
void DrawArrows(int x, int y);
void DrawEditorMode(EditorMode mode);
void DrawInterface();
void DrawNameEditor();
void DrawSkillEditor();
void DrawSkillSelect(SkillSelection button, bool selected);
void ExportRoster(const std::vector<struct ManPool> &usaRoster,
                  const std::vector<struct ManPool> &sovRoster);
std::string LaunchNameEditor(const struct ManPool &recruit);
std::vector<struct ManPool> LoadRoster(FILE *file, int plr);
void SetSkillLevel(struct ManPool &recruit, SkillSelection skill,
                   int rating);
int SkillLevel(const struct ManPool &recruit, SkillSelection skill);
};


/**
 * Main interface for Astronaut editing screen.
 *
 * Create cyborg astronauts, I guess...
 *
 * TODO: If actively entering text for a name, disable navigation?
 * TODO: This is fundamentally limited by not having reactive components
 * or existing as a class, or having lambda functions. There is a _lot_
 * of duplicate code that sets or toggles variables when a button is
 * clicked or the active recruit changes.
 */
void AstronautModification()
{
    int nation = -1;
    SkillSelection prev = SKILL_NONE;
    SkillSelection current = SKILL_NONE;
    EditorMode mode = EDITOR_NAME;

    int sovBar = 0;
    int usaBar = 0;
    int sovIndex = 0;
    int usaIndex = 0;
    int rating = 0;
    bool editFlag = false;

    helpText = "I040";
    keyHelpText = "K040";

    FadeOut(2, 10, 0, 0);
    DrawInterface();
    DrawEditorMode(mode);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();

    // Would prefer to instantiate rosters with call,
    // but can't do that within the try...catch scope.
    std::vector<struct ManPool> usaRoster;
    std::vector<struct ManPool> sovRoster;
    std::vector<struct ManPool> all;

    {
        try {

            if(!locate_file("user.json", FT_SAVE_CHECK).empty()) {
                bool useOriginal = Help("I105") > 0;

                if (useOriginal) {
                    DESERIALIZE_JSON_FILE(&all, locate_file("crew.json", FT_DATA));
                }
                else {
                    DESERIALIZE_JSON_FILE(&all, locate_file("user.json", FT_SAVE));
                }

            } else {
            // Should this be CNOTICE2?
                CINFO2(filesys,
                       "user.json not found. Loading crew.json rosters...");
                DESERIALIZE_JSON_FILE(&all, locate_file("crew.json", FT_DATA));
            }

            for (int i = 0; i < all.size() / 2; i++) {
                usaRoster.push_back(all.at(i));
                sovRoster.push_back(all.at(i + all.size() / 2));
            }

        } catch (const std::exception& err) {
            // TODO: Use a pop-up error display.
            CERROR3(filesys, "Unable to access rosters: %s", err.what());
            return;
        }
    }

    // Start by loading the USA Roster information.
    InBox(5, 28, 5 + 27, 28 + 16);
    nation = 0;
    mode = EDITOR_NAME;
    DisplayRoster(nation, 0, 0, usaRoster);
    DisplayRecruit(nation, usaRoster[0]);
    DrawSkillSelect(current, false);
    DrawNameEditor();

    while (true) {
        key = 0;
        GetMouse();

        if (key >= 'a' && key <= 'z') {
            key = toupper(key);
        }

        if (Clicked(246, 5, 314, 17) || key == K_ESCAPE ||
            (mode != EDITOR_SKILLS && key == K_ENTER)) {
            // IOBox(244, 3, 316, 19);
            InBox(246, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            if (editFlag) {
                bool proceed = true;

                if (!locate_file("user.json", FT_SAVE_CHECK).empty()) {
                    proceed = Help("I106") > 0;
                }

                if (proceed) {
                    try {
                        ExportRoster(usaRoster, sovRoster);
                    } catch (IOException &err) {
                        CERROR3(filesys, "Unable to save roster changes: %s",
                                err.what());
                    }
                }
            }

            OutBox(246, 5, 314, 17);
            break;
        } else if (Clicked(6, 29, 31, 43) || key == 'U') {
            if (nation != 0) {
                nation = 0;
                InBox(5, 28, 5 + 27, 28 + 16);
                OutBox(165, 28, 165 + 27, 28 + 16);
                fill_rectangle(186, 48, 313, 114, 0);
                DisplayRoster(nation, 0, 0, usaRoster);
                DisplayRecruit(nation, usaRoster[0]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }

            WaitForMouseUp();
        } else if (Clicked(166, 29, 191, 43) || key == 'S') {
            if (nation != 1) {
                nation = 1;
                OutBox(5, 28, 5 + 27, 28 + 16);
                InBox(165, 28, 165 + 27, 28 + 16);
                fill_rectangle(26, 48, 153, 114, 0);
                DisplayRoster(1, 0, 0, sovRoster);
                DisplayRecruit(nation, sovRoster[0]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }

            WaitForMouseUp();
        } else if (Clicked(26, 48, 153, 114)) {

            // Clicked on a USA roster entry
            if (nation == 0) {
                for (int i = 0; i < 8; i++) {
                    if (Clicked(27, 49 + i * 8, 152, 57 + i * 8)) {
                        usaIndex = usaIndex + (i - usaBar);
                        usaBar = i;
                        DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                        DisplayRecruit(nation, usaRoster[usaIndex]);
                        DrawSkillSelect(current, false);
                        current = SKILL_NONE;
                        prev = SKILL_NONE;
                        mode = EDITOR_NAME;
                        DrawNameEditor();

                        break;
                    }
                }
            }

            WaitForMouseUp();
        } else if (Clicked(186, 48, 313, 114)) {

            // Clicked on a Soviet roster entry
            if (nation == 1) {
                for (int i = 0; i < 8; i++) {
                    if (Clicked(187, 49 + i * 8, 312, 57 + i * 8)) {
                        sovIndex = sovIndex + (i - sovBar);
                        sovBar = i;
                        DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                        DisplayRecruit(nation, sovRoster[sovIndex]);
                        DrawSkillSelect(current, false);
                        current = SKILL_NONE;
                        prev = SKILL_NONE;
                        mode = EDITOR_NAME;
                        DrawNameEditor();

                        break;
                    }
                }
            }

            WaitForMouseUp();
        } else if (Clicked(6, 124, 42, 196) || key == 'C' ||
                   key == 'L' || key == 'E' || key == 'D' || key == 'N') {
            if (nation >= 0 && nation < NUM_PLAYERS) {
                int selection = SKILL_NONE;

                for (int i = SKILL_CAP; i < SKILL_NONE; i++) {
                    if (Clicked(6, 124 + i * 15, 42, 136 + i * 15)) {
                        selection = i;
                    }
                }

                if (key == 'C') {
                    selection = SKILL_CAP;
                } else if (key == 'L') {
                    selection = SKILL_LM;
                } else if (key == 'E') {
                    selection = SKILL_EVA;
                } else if (key == 'D') {
                    selection = SKILL_DOCK;
                } else if (key == 'N') {
                    selection = SKILL_ENDR;
                }

                if (selection != SKILL_NONE) {
                    current = SkillSelection(selection);
                    rating = SkillLevel(nation ? sovRoster[sovIndex] :
                                        usaRoster[usaIndex], current);

                    if (current == prev) {
                        DrawSkillSelect(current, false);
                        current = SKILL_NONE;
                        prev = SKILL_NONE;
                        mode = EDITOR_NAME;
                        DrawNameEditor();
                    } else {
                        if (mode == EDITOR_SKILLS) {
                            DrawSkillSelect(prev, false);
                        } else {
                            mode = EDITOR_SKILLS;
                            DrawSkillEditor();
                        }

                        DrawSkillSelect(current, true);
                        DisplaySkillRating(rating);
                        prev = current;
                    }

                    WaitForMouseUp();
                }
            }
        } else if (Clicked(6, 49, 18, 80) ||
                   (nation == 0 && key == UP_ARROW)) {

            if (nation == 0 && usaIndex > 0) {
                InBox(6, 49, 18, 80);

                do {
                    usaIndex--;

                    if (usaBar > 0) {
                        usaBar--;
                    }

                    DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                    DisplayRecruit(nation, usaRoster[usaIndex]);
                    DrawSkillSelect(current, false);
                    current = SKILL_NONE;
                    prev = SKILL_NONE;
                    mode = EDITOR_NAME;
                    DrawNameEditor();
                    delay(50);

                    if (mousebuttons > 0) {
                        GetMouse();
                    }
                } while (mousebuttons > 0 && usaIndex > 0);

                OutBox(6, 49, 18, 80);
            }

        } else if (Clicked(6, 82, 18, 113) ||
                   (nation == 0 && key == DN_ARROW)) {

            if (nation == 0 && usaIndex < usaRoster.size() - 1) {
                InBox(6, 82, 18, 113);

                do {
                    usaIndex++;

                    if (usaBar < 7) {
                        usaBar++;
                    }

                    DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                    DisplayRecruit(nation, usaRoster[usaIndex]);
                    DrawSkillSelect(current, false);
                    current = SKILL_NONE;
                    prev = SKILL_NONE;
                    mode = EDITOR_NAME;
                    DrawNameEditor();
                    delay(50);

                    if (mousebuttons > 0) {
                        GetMouse();
                    }
                } while (mousebuttons > 0 &&
                         usaIndex < usaRoster.size() - 1);

                OutBox(6, 82, 18, 113);
            }

        } else if (Clicked(166, 49, 178, 80) ||
                   (nation == 1 && key == UP_ARROW)) {

            if (nation == 1 && sovIndex > 0) {
                InBox(166, 49, 178, 80);

                do {
                    sovIndex--;

                    if (sovBar > 0) {
                        sovBar--;
                    }

                    DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                    DisplayRecruit(nation, sovRoster[sovIndex]);
                    DrawSkillSelect(current, false);
                    current = SKILL_NONE;
                    prev = SKILL_NONE;
                    mode = EDITOR_NAME;
                    DrawNameEditor();
                    delay(50);

                    if (mousebuttons > 0) {
                        GetMouse();
                    }
                } while (mousebuttons > 0 && sovIndex > 0);

                OutBox(166, 49, 178, 80);
            }

        } else if (Clicked(166, 82, 178, 113) ||
                   (nation == 1 && key == DN_ARROW)) {

            if (nation == 1 && sovIndex < sovRoster.size() - 1) {
                InBox(166, 82, 178, 113);

                do {
                    sovIndex++;

                    if (sovBar < 7) {
                        sovBar++;
                    }

                    DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                    DisplayRecruit(nation, sovRoster[sovIndex]);
                    DrawSkillSelect(current, false);
                    current = SKILL_NONE;
                    prev = SKILL_NONE;
                    mode = EDITOR_NAME;
                    DrawNameEditor();
                    delay(50);

                    if (mousebuttons > 0) {
                        GetMouse();
                    }
                } while (mousebuttons > 0 &&
                         sovIndex < sovRoster.size() - 1);

                OutBox(166, 82, 178, 113);
            }

        } else if (key == K_PGUP) {
            if (nation == 0) {
                usaIndex = MAX(usaIndex - 7, 0);
                usaBar = 0;
                DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                DisplayRecruit(nation, usaRoster[usaIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            } else if (nation == 1) {
                sovIndex = MAX(sovIndex - 7, 0);
                sovBar = 0;
                DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                DisplayRecruit(nation, sovRoster[sovIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }
        } else if (key == K_PGDN) {
            if (nation == 0) {
                usaIndex = MIN(usaIndex + 7, usaRoster.size() - 1);
                usaBar = 7;
                DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                DisplayRecruit(nation, usaRoster[usaIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            } else if (nation == 1) {
                sovIndex = MIN(sovIndex + 7, usaRoster.size() - 1);
                sovBar = 7;
                DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                DisplayRecruit(nation, sovRoster[sovIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }
        } else if (key == K_HOME) {
            if (nation == 0) {
                usaIndex = 0;
                usaBar = 0;
                DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                DisplayRecruit(nation, usaRoster[usaIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            } else if (nation == 1) {
                sovIndex = 0;
                sovBar = 0;
                DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                DisplayRecruit(nation, sovRoster[sovIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }
        } else if (key == K_END) {
            if (nation == 0) {
                usaIndex = usaRoster.size() - 1;
                usaBar = MIN(7, usaIndex);
                DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                DisplayRecruit(nation, usaRoster[usaIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            } else if (nation == 1) {
                sovIndex = sovRoster.size() - 1;
                sovBar = MIN(7, sovIndex);
                DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                DisplayRecruit(nation, sovRoster[sovIndex]);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();
            }
        } else if (mode == EDITOR_SKILLS) {
            if (Clicked(209, 168, 251, 176) || key == '-') {
                // IOBox(207, 166, 253, 178);
                InBox(209, 168, 251, 176);
                WaitForMouseUp();

                if (key) {
                    delay(150);
                }

                if (rating > MIN_SKILL_RATING) {
                    rating--;
                    DisplaySkillRating(rating);
                }

                OutBox(209, 168, 251, 176);
            } else if (Clicked(262, 168, 304, 176) || key == '+') {
                // IOBox(260, 166, 306, 178);
                InBox(262, 168, 304, 176);
                WaitForMouseUp();

                if (key) {
                    delay(150);
                }

                if (rating < MAX_SKILL_RATING) {
                    rating++;
                    DisplaySkillRating(rating);
                }

                OutBox(262, 168, 304, 176);
            } else if (Clicked(209, 184, 304, 192) || key == K_ENTER) {
                // IOBox(207, 182, 306, 194);
                InBox(209, 184, 304, 192);
                WaitForMouseUp();

                if (key) {
                    delay(150);
                }

                if (nation == 0) {
                    SetSkillLevel(usaRoster[usaIndex], current, rating);
                    editFlag = true;
                    DisplayRecruit(nation, usaRoster[usaIndex]);
                } else if (nation == 1) {
                    SetSkillLevel(sovRoster[sovIndex], current, rating);
                    editFlag = true;
                    DisplayRecruit(nation, sovRoster[sovIndex]);
                } else {
                    // TODO: Handle this better...
                    return;
                    assert(false);
                }

                OutBox(209, 184, 304, 192);
                DrawSkillSelect(current, false);
                current = SKILL_NONE;
                prev = SKILL_NONE;
                mode = EDITOR_NAME;
                DrawNameEditor();

            }
        } else if (mode == EDITOR_NAME &&
                   (Clicked(208, 156, 305, 167) || key == K_SPACE)) {
            WaitForMouseUp();

            if (key) {
                delay(100);
            }

            struct ManPool &recruit = nation ? sovRoster[sovIndex] :
                                          usaRoster[usaIndex];

            std::string name = LaunchNameEditor(recruit);

            if (strcmp(recruit.Name, name.c_str()) != 0) {
                strncpy(recruit.Name, name.c_str(),
                        sizeof(recruit.Name) - 1);
                editFlag = true;
                DisplayRecruit(nation, recruit);

                if (nation == 0) {
                    DisplayRoster(nation, usaIndex, usaBar, usaRoster);
                } else if (nation == 1) {
                    DisplayRoster(nation, sovIndex, sovBar, sovRoster);
                }
            }

            DrawNameEditor();
        }
    }

    return;
}


//------------------------------------------------------------------------


namespace
{

/**
 * Checks to see if the mouse clicked in the defined rectangle.
 *
 * Relies on the global values x, y, and mouse buttons.
 */
bool Clicked(int x1, int y1, int x2, int y2)
{
    if (!(mousebuttons > 0)) {
        return false;
    }

    // if (x1 > x2) {
    //     int temp = x2;
    //     x2 = x1;
    //     x1 = temp;
    // }

    // if (y1 > y2) {
    //     int temp = y2;
    //     y2 = y1;
    //     y1 = temp;
    // }

    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}


/**
 * Draw the Astronaut's stats in the skill profile box.
 *
 * This does not handle full 14-character names well.
 */
void DisplayRecruit(int plr, const struct ManPool &recruit)
{
    fill_rectangle(100, 134, 185, 140, 3);
    fill_rectangle(125, 150, 135, 187, 3);
    // Show USA recruits in blue, Soviets in red
    display::graphics.setForegroundColor(plr ? 9 : 6);
    draw_string(102, 139, recruit.Name);
    display::graphics.setForegroundColor(11);
    draw_number(recruit.Cap == 1 ? 128 : 127, 155, recruit.Cap);
    draw_number(recruit.LM == 1 ? 128 : 127, 163, recruit.LM);
    draw_number(recruit.EVA == 1 ? 128 : 127, 171, recruit.EVA);
    draw_number(recruit.Docking == 1 ? 128 : 127, 179, recruit.Docking);
    draw_number(recruit.Endurance == 1 ? 128 : 127, 187, recruit.Endurance);
}


void DisplayRoster(int plr, int selected, int bar,
                   const std::vector<struct ManPool> &roster)
{
    assert(bar >= 0 && bar < 8);
    assert(plr >= 0 && plr < NUM_PLAYERS);

    int start = selected - bar;
    fill_rectangle(26 + plr * 160, 48, 153 + plr * 160, 114, 0);
    ShBox(27 + plr * 160, 49 + bar * 8,
          152 + plr * 160, 57 + bar * 8);

    for (int i = start; i < start + 8 && i < roster.size(); i++) {
        display::graphics.setForegroundColor(1);
        draw_number(31 + plr * 160, 55 + (i - start) * 8, i + 1);

        display::graphics.setForegroundColor(roster[i].Sex ? 19 : 1);
        draw_string(50 + plr * 160, 55 + (i - start) * 8, roster[i].Name);
    }
}


void DisplaySkillRating(int rating)
{
    fill_rectangle(248, 151, 265, 159, 0);
    display::graphics.setForegroundColor(1);
    draw_number(257 - (rating == 1 ? 2 : 3), 157, rating);
}


void DrawArrows(int x, int y)
{
    fill_rectangle(x + 1, y + 1, x + 15, y + 67, 0);
    InBox(x, y, x + 16, y + 68);
    ShBox(x + 2, y + 2, x + 14, y + 33);
    draw_up_arrow(x + 5, y + 5);
    ShBox(x + 2, y + 35, x + 14, y + 66);
    draw_down_arrow(x + 5, y + 38);
}


void DrawEditorMode(EditorMode mode)
{
    switch (mode) {
    case EDITOR_NONE:
        // ShBox(0, 120, MAX_X - 1, MAX_Y - 1);
        break;

    case EDITOR_NAME:
        DrawNameEditor();
        break;

    case EDITOR_SKILLS:
        DrawSkillEditor();
        break;

    default:
        assert(false);
        break;
    }
}


void DrawInterface()
{
    display::graphics.screen()->clear();

    ShBox(0, 0, 319, 22);
    draw_heading(10, 5, "NAME AND SKILL EDITOR", 0, -1);

    // Draw Continue button
    IOBox(244, 3, 316, 19);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");

    // Draw USA Box
    ShBox(0, 24, 158, 118);
    OutBox(5, 28, 5 + 27, 28 + 16);
    draw_small_flag(0, 6, 29);  // 26x15
    InBox(40, 29, 150, 43);
    display::graphics.setForegroundColor(6);
    draw_string(95, 38, "ASTRONAUTS", ALIGN_CENTER);
    DrawArrows(4, 47);

    // Draw USA Astro box
    InBox(25, 47, 154, 115);
    fill_rectangle(25 + 1, 47 + 1, 154 - 1, 115 - 1, 0);


    // Draw Soviet Box
    ShBox(160, 24, 319, 118);
    OutBox(165, 28, 165 + 27, 28 + 16);
    draw_small_flag(1, 166, 29);
    InBox(200, 29, 310, 43);
    display::graphics.setForegroundColor(9);
    draw_string(255, 38, "COSMONAUTS", ALIGN_CENTER);
    DrawArrows(164, 47);

    // Draw Soviet Cosmo box
    InBox(185, 47, 314, 115);
    fill_rectangle(185 + 1, 47 + 1, 314 - 1, 115 - 1, 0);

    // Editing Box
    ShBox(0, 120, MAX_X - 1, MAX_Y - 1);

    // Skill Select Buttons
    OutBox(6, 124, 42, 136);
    display::graphics.setForegroundColor(9);
    draw_string(16, 132, "CAP");
    OutBox(6, 139, 42, 151);
    display::graphics.setForegroundColor(9);
    draw_string(17, 147, "L.M.");
    OutBox(6, 154, 42, 166);
    display::graphics.setForegroundColor(9);
    draw_string(16, 162, "EVA");
    OutBox(6, 169, 42, 181);
    display::graphics.setForegroundColor(9);
    draw_string(13, 177, "DOCK");
    OutBox(6, 184, 42, 196);
    display::graphics.setForegroundColor(9);
    draw_string(13, 192, "ENDR");

    // Astronaut/Cosmonaut skill display
    OutBox(59, 126, 195, 194);
    fill_rectangle(60, 127, 194, 193, 9);
    InBox(66, 130, 188, 190);
    fill_rectangle(67, 131, 187, 189, 3);
    display::graphics.setForegroundColor(1);
    draw_string(72, 139, "NAME:");
    draw_string(72, 147, "SKILLS:");
    draw_string(97, 155, "CAP");
    draw_string(97, 163, "L.M.");
    draw_string(97, 171, "EVA");
    draw_string(97, 179, "DOCK");
    draw_string(97, 187, "ENDR");

    // Name/Skill Change box
    OutBox(204, 127, 309, 145);
    fill_rectangle(205, 128, 308, 144, 6);
    InBox(207, 130, 306, 142);
    fill_rectangle(208, 131, 305, 141, 3);
    // DrawSkillEditor();
    // DrawNameEditor();
    // fill_rectangle(208, 131, 305, 141, 3);
    // display::graphics.setForegroundColor(6);
    // draw_string(256, 138, "NAME CHANGE", ALIGN_CENTER);

    // InBox(207, 155, 306, 168);
    // fill_rectangle(208, 156, 305, 167, 0);
}


void DrawNameEditor()
{
    fill_rectangle(208, 131, 305, 141, 3);
    display::graphics.setForegroundColor(6);
    draw_string(256, 138, "NAME CHANGE", ALIGN_CENTER);

    fill_rectangle(204, 150, 309, 195, 3);
    InBox(207, 155, 306, 168);
    fill_rectangle(208, 156, 305, 167, 0);

    display::graphics.setForegroundColor(1);
    draw_string(257, 180, "PRESS <SPACE> TO", ALIGN_CENTER);
    draw_string(257, 188, "EDIT RECRUIT NAME", ALIGN_CENTER);

    // display::graphics.setForegroundColor(9);
    // draw_string(257, 180, "SORRY, NAME EDITING", ALIGN_CENTER);
    // draw_string(257, 188, "IS NOT YET FINISHED", ALIGN_CENTER);
}


void DrawSkillEditor()
{
    fill_rectangle(208, 131, 305, 141, 3);
    display::graphics.setForegroundColor(9);
    draw_string(257, 138, "SKILL CHANGE", ALIGN_CENTER);

    fill_rectangle(204, 150, 309, 195, 3);
    InBox(207, 150, 242, 160);
    display::graphics.setForegroundColor(9);
    draw_string(213, 157, "MIN 0");
    InBox(271, 150, 306, 160);
    display::graphics.setForegroundColor(9);
    draw_string(276, 157, "MAX 4");

    InBox(247, 150, 266, 160);
    fill_rectangle(248, 151, 265, 159, 0);

    IOBox(207, 166, 253, 178);
    display::graphics.setForegroundColor(1);
    draw_string(230, 174, "MINUS", ALIGN_CENTER);
    IOBox(260, 166, 306, 178);
    display::graphics.setForegroundColor(1);
    draw_string(283, 174, "PLUS", ALIGN_CENTER);
    IOBox(207, 182, 306, 194);
    display::graphics.setForegroundColor(1);
    draw_string(256, 190, "DONE", ALIGN_CENTER);
}


void DrawSkillSelect(SkillSelection button, bool selected)
{
    static char name[5][6] = { "CAP", "L.M.", "EVA", "DOCK", "ENDR." };

    assert(button >= SKILL_CAP && button <= SKILL_NONE);

    if (button == SKILL_NONE) {
        return;
    }

    if (selected) {
        InBox(6, 124 + button * 15, 42, 136 + button * 15);
        display::graphics.setForegroundColor(11);
        draw_string(12, 132 + button * 15, name[button]);
        draw_string(97, 155 + button * 8, name[button]);
        fill_rectangle(248, 151, 265, 159, 0);
    } else {
        OutBox(6, 124 + button * 15, 42, 136 + button * 15);
        display::graphics.setForegroundColor(9);
        draw_string(12, 132 + button * 15, name[button]);
        display::graphics.setForegroundColor(1);
        draw_string(97, 155 + button * 8, name[button]);
    }
}


/**
 * Write the roster contents to user.json.
 *
 * TODO: Check the return values of the fwrite commands.
 */
void ExportRoster(const std::vector<struct ManPool> &usaRoster,
                  const std::vector<struct ManPool> &sovRoster)
{
    if (usaRoster.size() != ROSTER_SIZE) {
        // TODO: Throw an exception instead?
        CERROR4(baris, "USA roster is %d entries, expecting %d",
                usaRoster.size(), ROSTER_SIZE);
        return;
    }

    if (sovRoster.size() != ROSTER_SIZE) {
        // TODO: Throw an exception instead?
        CERROR4(baris, "Soviet roster is %d entries, expecting %d",
                sovRoster.size(), ROSTER_SIZE);
        return;
    }

    std::vector<struct ManPool> all = usaRoster;
    all.insert(all.end(), sovRoster.begin(), sovRoster.end());

    std::stringstream stream;
    {
        cereal::JSONOutputArchive::Options options = cereal::JSONOutputArchive::Options::NoIndent();
        cereal::JSONOutputArchive archive(stream, options);

        archive(all);
    }

    FILE *file = sOpen("user.json", "w", FT_SAVE);

    if (file == NULL) {
        throw IOException("Unable to open file user.json for writing");
    } else {
        CINFO2(filesys, "Exporting custom rosters to user.json...");
    }

    std::string str = stream.str();
    fwrite(str.data(), sizeof(char), str.size(), file);

    fclose(file);
}


/**
 * Creates a form for editing the current Astronaut/Cosmonaut's name.
 *
 * \return  the new name (original if cancelled).
 */
std::string LaunchNameEditor(const struct ManPool &recruit)
{
    const int maxLength = sizeof(recruit.Name) - 1;
    std::string originalName(
        recruit.Name, MIN(strlen(recruit.Name), sizeof(recruit.Name)));
    std::string name(originalName);
    name.reserve(maxLength + 1);

    display::graphics.setForegroundColor(1);
    fill_rectangle(204, 170, 309, 195, 3);
    draw_string(257, 179, "PRESS <ENTER> TO", ALIGN_CENTER);
    draw_string(257, 187, "SAVE RECRUIT NAME", ALIGN_CENTER);
    draw_string(257, 195, "<ESC> TO CANCEL", ALIGN_CENTER);

    fill_rectangle(208, 156, 305, 167, 0);
    display::graphics.setForegroundColor(1);
    draw_string(211, 164, name.c_str());
    display::graphics.setForegroundColor(9);
    draw_character(0x14);

    while (!(key == K_ENTER || key == K_ESCAPE)) {
        key = 0;
        GetMouse();

        if (key >= 'a' && key <= 'z') {
            key = toupper(key);
        }

        if (key & 0x00ff) {

            if ((name.length() < maxLength)
                && ((key == ' ') || ((key >= 'A' && key <= 'Z')) ||
                    (key >= '0' && key <= '9'))) {
                name.push_back(key);
                fill_rectangle(208, 156, 305, 167, 0);
                display::graphics.setForegroundColor(1);
                draw_string(211, 164, name.c_str());
                display::graphics.setForegroundColor(9);
                draw_character(0x14);
                key = 0;
            } else if (name.length() && key == 0x08) {
                name.erase(name.end() - 1);
                fill_rectangle(208, 156, 305, 167, 0);
                display::graphics.setForegroundColor(1);
                draw_string(211, 164, name.c_str());
                display::graphics.setForegroundColor(9);
                draw_character(0x14);
                key = 0;
            }
        }
    }

    fill_rectangle(208, 156, 305, 167, 0);
    return (key == K_ENTER && name.length()) ? name : originalName;
}


/**
 *
 *
 */
std::vector<struct ManPool> LoadRoster(FILE *file, int plr)
{
    assert(plr >= 0 && plr < NUM_PLAYERS);

    struct ManPool recruit;

    std::vector<struct ManPool> roster;

    fseek(file, plr * (sizeof(struct ManPool)) * ROSTER_SIZE, SEEK_SET);

    for (int i = 0; i < ROSTER_SIZE; i++) {
        if (!fread(&recruit, sizeof(recruit), 1, file)) {
            fclose(file);
            throw IOException("Error reading from roster file");
        }

        roster.push_back(recruit);
    }

    return roster;
}


void SetSkillLevel(struct ManPool &recruit, SkillSelection skill,
                   int rating)
{
    assert(skill >= SKILL_CAP && skill <= SKILL_ENDR);
    assert(rating >= MIN_SKILL_RATING && skill <= MAX_SKILL_RATING);

    switch (skill) {
    case SKILL_CAP:
        recruit.Cap = rating;
        break;

    case SKILL_LM:
        recruit.LM = rating;
        break;

    case SKILL_EVA:
        recruit.EVA = rating;
        break;

    case SKILL_DOCK:
        recruit.Docking = rating;
        break;

    case SKILL_ENDR:
        recruit.Endurance = rating;
        break;

    default:
        CWARNING3(baris, "Unrecognized value %d for parameter `skill`",
                  skill);
        break;
    }
}


int SkillLevel(const struct ManPool &recruit, SkillSelection skill)
{
    assert(skill >= SKILL_CAP && skill <= SKILL_ENDR);

    switch (skill) {
    case SKILL_CAP:
        return recruit.Cap;
        break;

    case SKILL_LM:
        return recruit.LM;
        break;

    case SKILL_EVA:
        return recruit.EVA;
        break;

    case SKILL_DOCK:
        return recruit.Docking;
        break;

    case SKILL_ENDR:
        return recruit.Endurance;
        break;

    default:
        CWARNING3(baris, "Unrecognized value %d for parameter `skill`",
                  skill);
        break;
    }

    return 0;
}

};  // End of namespace
