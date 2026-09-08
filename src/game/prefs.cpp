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

// This file handles original (main) game Preferences

#include "prefs.h"

#include <cctype>
#include <string>

#include "display/graphics.h"
#include "display/surface.h"

#include "admin.h"
#include "ast_mod.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gamedata.h"
#include "gr.h"
#include "ioexception.h"
#include "options.h"
#include "pace.h"
#include "randomize.h"
#include "sdlhelper.h"
#include "settings.h"


enum PreferencesMode {
    PREFS_NEWGAME,
    PREFS_NEWPBEM,
    PREFS_INGAME,
};

struct DisplayContext {
    boost::shared_ptr<display::PalettizedSurface> prefs_image;
};

void DrawPrefs(PreferencesMode where, bool is_AI_1, bool is_AI_2, AudioConfig audio,
               DisplayContext& dctx);
void EditDirectorName(int plr);
std::string GetTextInput(int x, int y, int maxLength);
void DrawModel(char model);
void DrawLevel(char side, char button, char level, DisplayContext& dctx);
void DrawMapOutline(int x, int y);
void DrawMap(char side, char country, DisplayContext& dctx);
void DrawPlayerIcon(char side, bool is_AI, DisplayContext& dctx);
int Preferences(int player, PreferencesMode where);
void SavePreferences(const AudioConfig& audio);


void DrawPrefs(PreferencesMode where, bool is_AI_1, bool is_AI_2, AudioConfig audio,
               DisplayContext& dctx)
{
    FadeOut(2, 10, 0, 0);

    display::graphics.screen()->clear();
    dctx.prefs_image->exportPalette();
    ShBox(0, 0, 319, 22);
    ShBox(0, 24, 89, 199);
    ShBox(91, 24, 228, 107);

    // Audio buttons should display as disabled if audio is disabled.
    if (audio.master.muted) {
        InBox(98, 28, 137, 63);
        InBox(98, 68, 137, 103);
    } else {
        IOBox(98, 28, 137, 63);
        IOBox(98, 68, 137, 103);
    }

    IOBox(144, 28, 221, 63);
    /* This draws disabled button around camera */
    /* IOBox(144,68,221,103); */
    InBox(144, 68, 221, 103);

    ShBox(91, 109, 228, 199);
    InBox(95, 113, 224, 195);
    fill_rectangle(96, 114, 223, 194, 0);
    ShBox(230, 24, 319, 199);

    if (where != PREFS_INGAME) {
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
    } else {
        music_start(M_DRUMSM);
        InBox(8, 107, 81, 138);
        InBox(8, 160, 81, 191);
        InBox(238, 107, 311, 138);
        InBox(238, 160, 311, 191);
        InBox(8, 77, 18, 85);
        InBox(238, 77, 248, 85);
    }

    DrawMapOutline(8, 54);
    DrawMapOutline(238, 54);
    fill_rectangle(237, 35, 312, 41, 0);
    fill_rectangle(7, 35, 82, 41, 0);

    if (where == PREFS_NEWPBEM) {
        draw_heading(3, 5, "PLAY BY MAIL SELECTIONS", 0, -1);
    } else {
        draw_heading(6, 5, "PREFERENCES SELECTIONS", 0, -1);
    }

    IOBox(243, 3, 316, 19);
    InBox(236, 34, 313, 42);
    InBox(6, 34, 83, 42);
    DrawMap(0, Data->Def.Plr1, dctx);
    DrawPlayerIcon(0, is_AI_1, dctx);
    DrawMap(1, Data->Def.Plr2, dctx);
    DrawPlayerIcon(1, is_AI_2, dctx);
    DrawLevel(0, 0, Data->Def.Ast1, dctx);
    DrawLevel(0, 1, Data->Def.Lev1, dctx);
    DrawLevel(1, 0, Data->Def.Ast2, dctx);
    DrawLevel(1, 1, Data->Def.Lev2, dctx);

    if (where != PREFS_INGAME) {
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
    draw_string(8, 40, Data->P[ Data->Def.Plr1 ].Name);
    draw_string(238, 40, Data->P[ Data->Def.Plr2 ].Name);

    display::graphics.legacyScreen()->draw(
        dctx.prefs_image, (audio.music.muted ? 153 : 187), 0, 33, 29, 101, 31);
    display::graphics.legacyScreen()->draw(
        dctx.prefs_image, (audio.soundFX.muted ? 221 : 255), 0, 33, 29, 101, 71);

    display::graphics.legacyScreen()->draw(dctx.prefs_image, 216, 30, 71, 29, 147, 31);
    display::graphics.legacyScreen()->draw(dctx.prefs_image, 72 * (Data->Def.Anim), 90, 71, 29, 147, 71);
    DrawModel(Data->Def.Input);

    FadeIn(2, 10, 0, 0);
}


void EditDirectorName(int plr)
{
    int x = (plr == 0) ? 7 : 237;
    int maxLength = MIN(12, sizeof(Data->P[plr].Name) - 1);
    fill_rectangle(x, 35, x + 75, 41, 0);
    std::string name = GetTextInput(x + 1, 40, maxLength);

    if (name.length() > 0 && name.length() <= maxLength) {
        memset(Data->P[plr].Name, 0, sizeof(Data->P[plr].Name));
        strncpy(Data->P[plr].Name, name.c_str(), maxLength + 1);
    }

    display::graphics.setForegroundColor(1);
    draw_string(x + 1, 40, Data->P[plr].Name);
    av_sync();
}


/**
 * Creates a text input field GUI element and returns user input.
 *
 * \param x  the x-coordinate for drawing the inputted text string.
 * \param y  the y-coordinate for drawing the inputted text string.
 * \param maxLength  the maximum number of characters the user may enter.
 */
std::string GetTextInput(int x, int y, int maxLength)
{
    std::string input;
    input.reserve(maxLength + 1);
    int key = 0;
    int width = maxLength * 6 + 1;

    fill_rectangle(x, y - 4, x + width, y, 0);
    display::graphics.setForegroundColor(1);
    grMoveTo(x, y);
    draw_character(0x14); // what is this?
    av_sync();

    do {
        key = getch();

        if (key != (key & 0xff)) {
            key = 0x00;
        } else if (key >= 'a' && key <= 'z') {
            key = toupper(key);
        }

        if (key == 0x08) {  // Backspace
            if (input.length() > 0) {
                input.pop_back();
                fill_rectangle(x, y - 4, x + width, y, 0);
                draw_string(x, y, input.c_str());
                draw_character(0x14);
                key = 0;
            }
        } else if (isupper(key) || isdigit(key) || key == ' ') {
            if (input.length() < maxLength) {
                input.push_back(key);
                fill_rectangle(x, y - 4, x + width, y, 0);
                draw_string(x, y, input.c_str());
                draw_character(0x14);
                key = 0;
            }
        }

        av_sync();
    } while (!(key == K_ENTER || key == K_ESCAPE));

    fill_rectangle(x, y - 4, x + width, y, 0);

    // Trim trailing whitespace
    while (input.length() > 0 && input.back() == ' ') {
        input.pop_back();
    }

    return (key == K_ENTER && input.length() > 0) ? input : "";
}


/* Draw the Hardware Model / Roster settings button
 *
 * Hardware Model & Roster settings rotate through
 *     0: Basic Model / Historical Roster
 *     1: Basic Model / Custom Roster
 *     2: Historical Model / Historical Roster
 *     3: Historical Model / Custom Roster
 *     4: Random Model / Historical Roster       (Optional)
 *     5: Random Model / Custom Roster           (Optional)
 *
 * The Basic Model uses identical values for comparable US/USSR
 * hardware (i.e. Mercury/Vostok), ensuring a balanced game environment.
 * The Historical Model uses different values for comparable US/USSR
 * programs, representing the historical variations between the two
 * countries' programs.
 * The Random Model... TODO: does what?
 * Random Model is only available if options.feat_random_eq is enabled.
 * See the manual for more information.
 *
 * Modifies the main screen palette.
 *
 * \param mode  The current model/roster setup (0-5).
 */
void DrawModel(char model)
{
    char filename[128];
    int image = (model == 0 || model == 1 || model == 4) ? 1 : 0;
    snprintf(filename, sizeof(filename), "images/prfx.but.%d.png", image);

    boost::shared_ptr<display::PalettizedSurface> prefsImage(
        Filesystem::readImage(filename));

    // The loaded image versions have their own palettes, which are
    // not included in the Preferences screen palette. They occupy
    // an unused 96-color space - [112, 112 + 96) - to not interfere
    // with the Preferences palette, but their palette must be added.
    prefsImage->exportPalette(112, 112 + 95);

    fill_rectangle(96, 114, 223, 194, 0);
    display::graphics.screen()->draw(prefsImage, 97, 115);

    display::graphics.setForegroundColor(11);

    if (model <= 1) {
        draw_string(100, 122, "BASIC MODEL");
    } else if (model <= 3) {
        draw_string(100, 122, "HISTORICAL MODEL");
    } else {
        draw_string(100, 122, "RANDOM MODEL");
    }

    display::graphics.setForegroundColor(9);

    if (model % 2 == 0) {
        draw_string(100, 128, "HISTORICAL ROSTER");
    } else {
        draw_string(100, 128, "CUSTOM ROSTER");
    }
}


void DrawLevel(char side, char button, char level, DisplayContext& dctx)
{
    unsigned int srcX = level * 72;
    unsigned int srcY = (button == 0) ? 30 : 60;
    unsigned int x = (side == 0) ? 9 : 239;
    unsigned int y = (button == 0) ? 161 : 108;

    display::graphics.legacyScreen()->draw(dctx.prefs_image, srcX, srcY, 71, 29, x, y);
}

void DrawMapOutline(int x, int y)
{
    display::graphics.setForegroundColor(4);
    grMoveTo(0 + x, y + 20);
    grLineTo(0 + x, y + 0);
    grLineTo(72 + x, y + 0);
    grMoveTo(12 + x, y + 21);
    grLineTo(12 + x, y + 30);
    display::graphics.setForegroundColor(2);
    grMoveTo(0 + x, y + 21);
    grLineTo(11 + x, y + 21);
    grMoveTo(12 + x, y + 31);
    grLineTo(73 + x, y + 31);
    grLineTo(73 + x, y + 0);
}

void DrawMap(char side, char country, DisplayContext& dctx)
{
    unsigned int srcX = (country == 0) ? 0 : 72;
    unsigned int x = (side == 0) ? 9 : 239;

    // Map is drawn in 2 rectangles to preserve the player icon
    display::graphics.legacyScreen()->draw(dctx.prefs_image, srcX,      0, 12, 19, x     , 55);
    display::graphics.legacyScreen()->draw(dctx.prefs_image, srcX + 11, 0, 60, 29, x + 12, 55);

}

void DrawPlayerIcon(char side, bool is_AI, DisplayContext& dctx)
{
    unsigned int x = (side == 0) ? 9 : 239;
    unsigned int srcY = is_AI ? 7 : 0;

    display::graphics.legacyScreen()->draw(dctx.prefs_image, 144, srcY, 9, 7, x, 78);
}


/**
 * Opens the settings menu for changing game settings.
 *
 * During gameplay, the ability to change certain settings is disabled.
 *
 * Pref Levels:
 * Player Select:
 *     0 = USA
 *     1 = USSR
 *     2 = USA AI
 *     3 = USSR AI
 *     4 = FOREIGN (maybe)
 *     5 = FOREIGN AI (maybe)
 *     6 = USA (modem play)
 *     7 = USSR (modem play)
 *
 * Game Level:
 *     0 = Easy
 *     1 = Medium
 *     2 = Hard
 * Astro Level:
 *     0 = Easy
 *     1 = Medium
 *     2 = Hard
 * Input:
 *     0 = Mouse
 *     1 = Keyboard
 *     2 = Joystick (maybe)
 * Anim Level:
 *     0 = Full
 *     1 = Partial
 *     2 = Results Only
 * Music Level:
 *     0 = Full
 *     1 = Partial
 *     2 = None
 * Sound FX:
 *     0 = On
 *     1 = Off
 *
 * See documentation for HModel() for more about the Hardware Model /
 * Roster settings.
 *
 * \param where  current game state used for determining options
 *               (0: Pregame setup
 *                1: In-game settings menu
 *                3: New PBEM game)
 * \return PREFS_ABORTED if cancelling out of menu, PREFS_SET otherwise.
 */
int Preferences(int player, PreferencesMode where)
{
    int selected_player = 0;
    bool is_AI_1 = false;
    bool is_AI_2 = false;
    DisplayContext dctx;
    AudioConfig audio = LoadAudioSettings();
    char numHModels = options.feat_random_eq > 0 ? 6 : 4;

    helpText = "i013";
    keyHelpText = "K013";

    if (where != PREFS_NEWPBEM) {
        // If starting a new game, set default configuration
        if (where == PREFS_NEWGAME) {
            Data->Def.Plr1 = 0;
            Data->Def.Plr2 = 1;
            is_AI_1 = false;
            is_AI_2 = true;
            Data->Def.Lev1 = Data->Def.Ast1 = Data->Def.Ast2 = 0;
            Data->Def.Lev2 = 2;   // start computer level 3
            Data->Def.Input = 2;  // Historical Model / Historical Roster
        }

        if (Data->Def.Plr1 > 1) {
            Data->Def.Plr1 -= 2;
            is_AI_1 = true;
        }

        if (Data->Def.Plr2 > 1) {
            Data->Def.Plr2 -= 2;
            is_AI_2 = true;
        }
    } else {
        Data->Def.Lev1 = Data->Def.Lev2 = Data->Def.Ast1 = Data->Def.Ast2 = 0;
        Data->Def.Input = 2;  // Historical Model / Historical Roster
    }

    boost::shared_ptr<display::PalettizedSurface> prefs_image(Filesystem::readImage("images/preferences.png"));
    dctx.prefs_image = prefs_image;

    DrawPrefs(where, is_AI_1, is_AI_2, audio, dctx);
    WaitForMouseUp();

    while (1) {
        key = 0;
        GetMouse();
        if (mousebuttons == 0 && key == 0) continue;
        
        /* Gameplay */
        if (((x >= 245 && y >= 5 && x <= 314 && y <= 17) || key == K_ENTER) && !(is_AI_1 && is_AI_2)) {
            InBox(245, 5, 314, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            OutBox(245, 5, 314, 17);

            if (!(Data->Def.Input == 2 || Data->Def.Input == 3)) {
                if (options.feat_eq_new_name && !is_AI_1) {
                    SetEquipName(0);
                }

                if (options.feat_eq_new_name && !is_AI_2) {
                    SetEquipName(1);
                }
            }  // Change Name, if basic mode and for human players

            if (Data->Def.Plr1 == Data->Def.Plr2) continue;
            
            if (Data->Def.Plr1 == 1) {
                char Name[20]{};
                strcpy(Name, Data->P[0].Name);
                strcpy(Data->P[0].Name, Data->P[1].Name);
                strcpy(Data->P[1].Name, Name);
                
                std::swap(Data->Def.Lev1, Data->Def.Lev2);
                std::swap(Data->Def.Ast1, Data->Def.Ast2);
            }

            if (is_AI_1) {
                Data->Def.Plr1 += 2;
            }
            if (is_AI_2) {
                Data->Def.Plr2 += 2;
            }

            if (where != PREFS_INGAME) {
                FadeOut(2, 10, 0, 0);
            }

            key = 0;

            if (where != PREFS_INGAME && (Data->Def.Input == 2 || Data->Def.Input == 3)) {
                std::ifstream os{locate_file("hist.json", FT_DATA)};
                cereal::JSONInputArchive ar{os};

                // Don't make a loop over the players as this
                // will break the preprocessor macro.

                ARCHIVE_VECTOR(Data->P[0].Probe, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[0].Rocket, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[0].Manned, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[0].Misc, struct Equipment, 7);

                ARCHIVE_VECTOR(Data->P[1].Probe, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[1].Rocket, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[1].Manned, struct Equipment, 7);
                ARCHIVE_VECTOR(Data->P[1].Misc, struct Equipment, 7);
            }

            // Random Equipment
            if (where != PREFS_INGAME && (Data->Def.Input == 4 || Data->Def.Input == 5)) {
                RandomizeEq();
            }

            for (int i = 0; i < NUM_PLAYERS; i++) {
                for (int k = 0; k < 7; k++) {
                    Data->P[i].Probe[k].MSF = Data->P[i].Probe[k].MaxRD;
                    Data->P[i].Rocket[k].MSF = Data->P[i].Rocket[k].MaxRD;
                    Data->P[i].Manned[k].MSF = Data->P[i].Manned[k].MaxRD;
                    Data->P[i].Misc[k].MSF = Data->P[i].Misc[k].MaxRD;
                }
            }

            CacheCrewFile();
            SavePreferences(audio);
            music_stop();
            return PREFS_SET;

        } else if (key == K_ESCAPE) {
            SavePreferences(audio);
            music_stop();
            FadeOut(2, 10, 0, 0);
            return PREFS_ABORTED;

        } else if (key == 'P' && where != PREFS_INGAME) {
            selected_player = other(selected_player);

            fill_rectangle(59, 26, 68, 31, 3);
            fill_rectangle(290, 26, 298, 31, 3);

            int color1 = 34;
            int color2 = 9;
            if (selected_player == 0) std::swap(color1, color2);
            
            display::graphics.setForegroundColor(color1);
            draw_string(23, 30, "PLAYER 1");
            display::graphics.setForegroundColor(color2);
            draw_string(253, 30, "PLAYER 2");
            
        } else if ((x >= 146 && y >= 30 && x <= 219 && y <= 61 && mousebuttons > 0)
                   || key == 'E') {
            // Edit astronauts has been ripped out
            InBox(146, 30, 219, 61);
            delay(500);
            AstronautModification();
            // TODO: Make sure *everything* is redrawn with the
            // correct values!
            DrawPrefs(where, is_AI_1, is_AI_2, audio, dctx);

        } else if (((x >= 96 && y >= 114 && x <= 223 && y <= 194 && mousebuttons > 0) || key == K_SPACE) 
                   && where != PREFS_INGAME) {  // Hist
            WaitForMouseUp();
            Data->Def.Input = (Data->Def.Input + 1) % numHModels;
            DrawModel(Data->Def.Input);

        } else if ((x >= 146 && y >= 70 && x <= 219 && y <= 101 && mousebuttons > 0) || key == 'A') {
            /* disable this option right now */
        } else if ((x >= 100 && y >= 30 && x <= 135 && y <= 61 && mousebuttons > 0) || key == 'M') {
            if (audio.master.muted) continue;
            
            InBox(100, 30, 135, 61);
            WaitForMouseUp();
            audio.music.muted = !audio.music.muted;
            music_set_mute(audio.music.muted);
            display::graphics.legacyScreen()->draw(
                dctx.prefs_image, (audio.music.muted ? 153 : 187), 0, 33, 29, 101, 31);
            OutBox(100, 30, 135, 61);

            /* Music Level */
        } else if ((x >= 100 && y >= 70 && x <= 135 && y <= 101 && mousebuttons > 0) || key == 'S') {
            if (audio.master.muted) continue;
            
            InBox(100, 70, 135, 101);
            WaitForMouseUp();
            audio.soundFX.muted = !audio.soundFX.muted;
            MuteChannel(AV_SOUND_CHANNEL, audio.soundFX.muted);
            display::graphics.legacyScreen()->draw(
                dctx.prefs_image, (audio.soundFX.muted ? 221 : 255), 0, 33, 29, 101, 71);
            OutBox(100, 70, 135, 101);

            /* Sound Level */
        } else if (where == PREFS_NEWGAME && ((x >= 8 && y >= 77 && x <= 18 && y <= 85 && mousebuttons > 0)
                 || (selected_player == 0 && key == 'H'))) {
            InBox(8, 77, 18, 85);
            WaitForMouseUp();
            is_AI_1 = !is_AI_1;

            DrawPlayerIcon(0, is_AI_1, dctx);
            OutBox(8, 77, 18, 85);

            /* P1: Human/Computer */
            //change human to dif 1 and comp to 3
            Data->Def.Lev1 = is_AI_1 ? 2 : 0;
            DrawLevel(0, 1, Data->Def.Lev1, dctx);

        } else if (where != PREFS_INGAME && ((x >= 8 && y >= 107 && x <= 81 && y <= 138 && mousebuttons > 0)
                   || (selected_player == 0 && key == 'G'))) {
            InBox(8, 107, 81, 138);
            WaitForMouseUp();
            OutBox(8, 107, 81, 138);
            Data->Def.Lev1 = (Data->Def.Lev1 + 1) % 3;
            DrawLevel(0, 1, Data->Def.Lev1, dctx);

            /* P1: Game Level */
        } else if (where != PREFS_INGAME && ((x >= 8 && y >= 160 && x <= 81 && y <= 191 && mousebuttons > 0)
                   || (selected_player == 0 && key == 'L'))) {
            InBox(8, 160, 81, 191);
            WaitForMouseUp();
            OutBox(8, 160, 81, 191);
            Data->Def.Ast1 = (Data->Def.Ast1 + 1) % 3;
            DrawLevel(0, 0, Data->Def.Ast1, dctx);

            /* P1: Astro Level */
        } else if (where == PREFS_NEWGAME && ((x >= 238 && y >= 77 && x <= 248 && y <= 85 && mousebuttons > 0)
                   || (selected_player == 1 && key == 'H'))) {
            InBox(238, 77, 248, 85);
            WaitForMouseUp();
            is_AI_2 = !is_AI_2;

            DrawPlayerIcon(1, is_AI_2, dctx);
            OutBox(238, 77, 248, 85);

            /* P2:Human/Computer */
            //change human to dif 1 and comp to 3
            Data->Def.Lev2 = is_AI_2 ? 2 : 0;
            DrawLevel(1, 1, Data->Def.Lev2, dctx);

        } else if (where != PREFS_INGAME && ((x >= 238 && y >= 107 && x <= 311 && y <= 138 && mousebuttons > 0)
                   || (selected_player == 1 && key == 'G'))) {
            InBox(238, 107, 311, 138);
            WaitForMouseUp();
            OutBox(238, 107, 311, 138);
            Data->Def.Lev2 = (Data->Def.Lev2 + 1) % 3;
            DrawLevel(1, 1, Data->Def.Lev2, dctx);

            /* P2: Game Level */
        } else if (where != PREFS_INGAME && ((x >= 238 && y >= 160 && x <= 311 && y <= 191 && mousebuttons > 0)
                   || (selected_player == 1 && key == 'L'))) {
            InBox(238, 160, 311, 191);
            WaitForMouseUp();
            OutBox(238, 160, 311, 191);
            Data->Def.Ast2 = (Data->Def.Ast2 + 1) % 3;
            DrawLevel(1, 0, Data->Def.Ast2, dctx);

            /* P2: Astro Level */
        } else if ((x >= 6 && y >= 34 && x <= 83 && y <= 42 && mousebuttons > 0)
                   || (selected_player == 0 && key == 'N')) {
            /* P1: Director Name */
            if (where != PREFS_INGAME || player == 0 || !IsHumanPlayer(0)) {
                EditDirectorName(0);
            }
        } else if ((x >= 236 && y >= 34 && x <= 313 && y <= 42 && mousebuttons > 0)
                   || (selected_player == 1 && key == 'N')) {
            /* P2: Director Name */
            if (where != PREFS_INGAME || player == 1 || !IsHumanPlayer(1)) {
                EditDirectorName(1);
            }
        }
    }
}

void SavePreferences(const AudioConfig& audio)
{
    try {
        SaveAudioSettings(audio);
    } catch (const IOException& err) {
        CAT_ERROR(filesys, err.what());
    } catch (const cereal::Exception& err) {
        CAT_ERROR(filesys, err.what());
    }
}


void IngamePreferences(int player)
{
    Preferences(player, PREFS_INGAME);
}


int NewGamePreferences()
{
    return Preferences(0, PREFS_NEWGAME);
}


int NewPBEMGamePreferences()
{
    return Preferences(0, PREFS_NEWPBEM);
}
