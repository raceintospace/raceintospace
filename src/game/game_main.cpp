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

//****************************************************************
//*Interplay's BUZZ ALDRIN's RACE into SPACE                     *
//*                                                              *
//*Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA   *
//*Copyright 1991 by Strategic Visions, Inc.                     *
//*Designed by Fritz Bronner                                     *
//*Programmed by Michael K McCarty                               *
//*                                                              *
//****************************************************************

#include <cassert>
#include <cctype>
#include <cerrno>
#include <math.h>

#include <sstream>
#include <stdexcept>

#include "game_main.h"

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "admin.h"
#include "aimast.h"
#include "ast4.h"
#include "Buzz_inc.h"
#include "crash.h"
#include "crew.h"
#include "endgame.h"
#include "filesystem.h"
#include "gr.h"
#include "hardware.h"
#include "intel.h"
#include "intro.h"
#include "mc.h"
#include "mission_util.h"
#include "museum.h"
#include "newmis.h"
#include "news.h"
#include "options.h"
#include "pace.h"
#include "pbm.h"
#include "place.h"
#include "port.h"
#include "prefs.h"
#include "records.h"
#include "review.h"
#include "sdlhelper.h"
#include "settings.h"
#include "start.h"
#include "state_utils.h"
#include "utils.h"

#ifdef CONFIG_MACOSX
// SDL.h needs to be included here to replace the original main() with
// what it needs for the Mac
#include <SDL.h>
#endif

char Name[20];
struct Players *Data;
int x;
int y;
int mousebuttons;
int key;
int oldx;
int oldy;
unsigned char LOAD;
unsigned char QUIT;
unsigned char AL_CALL;
char plr[NUM_PLAYERS];
std::string helpText;
std::string keyHelpText;
char *buffer;
char pNeg[NUM_PLAYERS][MAX_MISSIONS];
int32_t xMODE;
char Option = -1;
// true for fullscreen mission playback, false otherwise
bool fullscreenMissionPlayback;
char manOnMoon = 0;
char dayOnMoon = 20;
char AI[2] = {0, 0};
// Used to hold mid-turn save game related information
INTERIMDATA interimData;
struct AssetData *Assets;

const char *S_Name[] = {
    "LAUNCH",
    "ORBITAL INS. BURN",
    "HARDWARE POWER-ON",
    "REENTRY",
    "RECOVERY/LANDING",
    "EARTH ORBITAL ACTIVITIES",
    "EARTH DE-ORBIT BURN",
    "ORBITAL EVA",
    "DOCKING",
    "TRANS-LUNAR INJECTION",
    "TRANS-EARTH INJECTION",
    "LUNAR ORBITAL INS. BURN",
    "LUNAR ORBITAL ACTIVITIES",
    "EARTH MID-COURSE ACT.",
    "LUNAR MID-COURSE ACT.",
    "LUNAR EVA",
    "LEM ACTIVITIES",
    "LUNAR DE-ORBIT BURN",
    "LUNAR LANDING",
    "LUNAR LIFTOFF",
    "PHOTO RECONNAISSANCE",
    "PLANET ORBIT BURN",
    "PLANETARY ACTIVITIES",
    "EARTH RENDEZVOUS",
    "LUNAR RENDEZVOUS",
    "TRANS-PLANETARY INJ.",
    "LEM THRUST TEST",
    "DURATION",
    "JOINT DURATION",
    "REENTRY CORRIDOR",
    "MID-COURSE CORR. BURN",
    "MID-COURSE CORR. BURN",
    "EARTH ORBITAL INS. BURN"
};

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)


int game_main_impl(int argc, char *argv[]);
int CheckIfMissionGo(char plr, char launchIdx);
void ConfigureAudio();
void InitData(void);
void MainLoop(void);
void DockingKludge(void);
void OpenEmUp(void);
void CloseEmUp(unsigned char error, unsigned int value);
void VerifyCrews(char plr);



int game_main_impl(int argc, char *argv[])
{
    const char *see_readme = "look for further instructions in the README file";

    char ex, choice;

    // initialize the filesystem
    Filesystem::init(argv[0]);

    if (!display::image::libpng_versions_match()) {
        std::stringstream message;
        message
                << "This build was compiled against libpng " << display::image::libpng_headers_version()
                << ", but is running with libpng " << display::image::libpng_runtime_version() << ".";
        crash("libpng mismatch", message.str());
    }

    setup_options(argc, argv);
    Filesystem::addPath(options.dir_gamedata);
    Filesystem::addPath(options.dir_savegame);
    /* hacking... */
    log_setThreshold(&_LOGV(LOG_ROOT_CAT), MAX(0, LP_NOTICE - (int)options.want_debug));
    
    std::ifstream file(locate_file("usa_port.json", FT_DATA));
    if (!file) {
        CRITICAL1("can't find game data files");
        NOTICE1("set environment variable BARIS_DATA or edit config file");
        NOTICE2("%s", see_readme);
        
        crash("Data missing", "Unable to locate game data files.");
    } else {
        INFO1("game data files found");
    }
    
    if (create_save_dir() != 0) {
        CRITICAL3("can't create save directory `%s': %s",
                  options.dir_savegame, strerror(errno));
        NOTICE1("set environment variable BARIS_SAVE to a writable directory");
        NOTICE2("%s", see_readme);

        crash("Save directory", "Couldn't create save directory");
    }

    av_setup();

    helpText = "i000";
    keyHelpText = "k000";

    LOAD = QUIT = 0;

    xMODE = 0;

    Data = new struct Players;
    buffer = (char *)xmalloc(BUFFER_SIZE);

    DEBUG3("main buffer %p (%d)", buffer, BUFFER_SIZE);

    memset(buffer, 0x00, BUFFER_SIZE);

    Assets = new struct AssetData;

    DESERIALIZE_JSON_FILE(&Assets->sSeq, locate_file("seq.json", FT_DATA));
    DESERIALIZE_JSON_FILE(&Assets->fSeq, locate_file("fseq.json", FT_DATA));
    DESERIALIZE_JSON_FILE(&Assets->fails, locate_file("fails.json", FT_DATA));
    DESERIALIZE_JSON_FILE(&Assets->help, locate_file("help.json", FT_DATA));

    OpenEmUp();                   // OPEN SCREEN AND SETUP GOODIES

    ConfigureAudio();

    if (options.want_intro) {
        Introd();
    }

    ex = 0;

    while (ex == 0) {
        bool launchGame = false;
        MakeRecords();

        DESERIALIZE_JSON_FILE(Data, locate_file("urast.json", FT_DATA));

        if (Data->Checksum != (sizeof(struct Players))) {
            /* XXX: too drastic */
            CRITICAL1("wrong version of data file");
            exit(EXIT_FAILURE);
        }

        display::graphics.screen()->clear();
        PortPal(0);
        key = 0;
        helpText = "i000";
        keyHelpText = "i000";

        music_start(M_LIFTOFF);

        choice = MainMenuChoice();

        switch (choice) {
        case MAIN_NEW_GAME:  // New Game
#ifdef ALLOW_PBEM
        case MAIN_PBEM_GAME:  // New Mail Game
#endif // ALLOW_PBEM
            LOAD = QUIT = 0;
#ifdef ALLOW_PBEM
            MAIL = (choice == MAIN_PBEM_GAME) ? 0 : -1;
#else
            MAIL = -1;
#endif
            Option = -1;

            if (MAIL == -1) {
                // GET INITIAL PREFS FROM PLAYER
                launchGame = (NewGamePreferences() != PREFS_ABORTED);
            }

#ifdef ALLOW_PBEM
            else { // MAIL GAME
                launchGame = (NewPBEMGamePreferences() != PREFS_ABORTED);
            }

#endif // ALLOW_PBEM

            if (launchGame) {
                plr[0] = Data->Def.Plr1;       // SET GLOBAL PLAYER VALUES
                plr[1] = Data->Def.Plr2;
                Data->plr[0] = Data->Def.Plr1;  // SET STRUCTURE PLAYER VALUES
                Data->plr[1] = Data->Def.Plr2;

                if (MAIL == -1) {
                    if (plr[0] == 2 || plr[0] == 3) {
                        AI[0] = 1;
                    } else {
                        AI[0] = 0;
                    }

                    if (plr[1] == 2 || plr[1] == 3) {
                        AI[1] = 1;
                    } else {
                        AI[1] = 0;
                    }
                } else {
                    AI[0] = AI[1] = 0;
                }

                InitData();                   // PICK EVENT CARDS N STUFF
                MainLoop();                   // PLAY GAME
                display::graphics.screen()->clear();
            }

            break;

        case MAIN_OLD_GAME: // Play Old Game
            LOAD = QUIT = 0;
            MAIL = -1;
            Option = -1;
            FileAccess(1);

            if (LOAD == 1) {
                if (Option == -1) {
                    MainLoop();    //Regular game
                } else { //Modem game
                    WARNING1("can't do modem games");
                    break;
                }
            } else if (!QUIT) {
                FadeOut(2, 10, 0, 0);
            }

            QUIT = 0;
            display::graphics.screen()->clear();
            break;

        case MAIN_CREDITS:
            Credits();
            break;

        case MAIN_EXIT:
            //KillMusic();
            ex = 1;
            FadeOut(2, 10, 0, 0);
            break;
        }
    }

    display::graphics.destroy();
    CloseEmUp(0, 0); // Normal Exit
    exit(EXIT_SUCCESS);
}

int game_main(int argc, char *argv[])
{
    // Do all the work in game_main_impl(), but trap exceptions here, since we're called from C
    try {
        return game_main_impl(argc, argv);
    } catch (const std::exception &e) {
        fprintf(stderr, "unhandled exception: %s\n", e.what());
        abort();
    } catch (const std::string &e) {
        fprintf(stderr, "unhandled exception: %s\n", e.c_str());
        abort();
    } catch (...) {
        fprintf(stderr, "unhandled exception of unknown type, terminating\n");
        abort();
    }
}

/** utility fn for AI to see if it should scrub the mission
 *
 * Rules are:
 * - always do unmanned missions
 * - Safety must be within 15 of R&D-Safety
 *
 * \param plr Playerdata
 * \param launchIdx ID of the launch
 * \return  1 if mission is safe, 0 if not.
 */
int CheckIfMissionGo(char plr, char launchIdx)
{
    char idx, mcode;
    struct MissionType *pMission;
    Equipment *E = NULL;  /* Pointer to Equipment we're looking at */

    // Grab the Mission Code from the current Launch Index
    mcode = Data->P[plr].Mission[launchIdx].MissionCode;
    pMission = &Data->P[plr].Mission[launchIdx];

    // Always a go for Unmanned missions
    if (! IsManned(mcode)) {
        return 1;
    }

    // Spin through mission hardware checking safety
    // TODO: The EVA suit safety should be checked, but since EVA
    // suits are _always_ included in missions that will require
    // reworking the EVA suit inclusion.
    for (idx = Mission_Capsule; idx <= Mission_PrimaryBooster; idx++) {
        // Mission components in struct MissionType.Hard are indexed
        // starting at 0 per EquipProbeIndex, EquipMannedIndex, etc.
        // EXCEPT for rockets (more below) so a missing component is
        // flagged with a negative value.
        if (pMission->Hard[idx] < 0) {
            continue;
        }

        switch (idx) {
        case Mission_Capsule:
        case Mission_LM:
            E = &Data->P[plr].Manned[pMission->Hard[idx]];
            E->MisSaf = E->Safety;
            break;

        case Mission_Kicker:
            E = &Data->P[plr].Misc[pMission->Hard[idx]];
            E->MisSaf = E->Safety;
            break;

        case Mission_PrimaryBooster:
            // Vab sets Hard[Mission_PrimaryBooster] to the rocket's
            // index + 1 so the game can check if mission hardware
            // has been assigned by testing
            //     Hard[Mission_PrimaryBooster] > 0
            // MissionSetup() will correct for that in mc2.cpp.
        {
            int rocketIndex = pMission->Hard[idx] - 1;
            E = &Data->P[plr].Rocket[rocketIndex % ROCKET_HW_BOOSTERS];

            if (rocketIndex >= ROCKET_HW_BOOSTERS) {
                const Equipment *boosters =
                    &Data->P[plr].Rocket[ROCKET_HW_BOOSTERS];
                E->MisSaf =
                    RocketBoosterSafety(E->Safety, boosters->Safety);
            } else {
                E->MisSaf = E->Safety;
            }
        }

        break;

        case Mission_Probe_DM:
            E = NULL;  // TODO: Might want to consider checking DM.
            break;

        default:  // Catch anything weird
            assert(false);
            WARNING2("Unexpected MissionHardwareType=%d found in "
                     "CheckIfMissionGo()", idx);
            E = NULL;
            break;
        }

        if (E && idx != Mission_Probe_DM && pMission->Hard[idx] >= 0) {
            // If mission Safety is not within 15 points of the MaxRD
            // then NO Go
            if (E->MisSaf < (E->MaxRD - 15)) {
                return 0;
            }
        }
    }

    return 1;
}

void InitData(void)
{
    InitializeEvents();                  // RESET EVENT CARDS
    Data->Count = 0;               // SET EVENT COUNTER TO ZERO

    for (int j = 0; j < NUM_PLAYERS; j++) {
        for (int i = 0; i < 5; i++) {
            Data->P[j].PresRev[i] = 8;
        }
    }

    return;
}

void MainLoop(void)
{
    int t1, t2, t3, prest, sign, turn, old_mission_count;
    bool newTurn;
    std::array<char, 2> IDLE = {0, 0};
    struct PrestType oldPrestige[MAXIMUM_PRESTIGE_NUM];

    if (LOAD != 1) {
        Data->P[0].Cash = Data->P[0].Budget; // INCREMENT BY BUDGET
        Data->P[1].Cash = Data->P[1].Budget;
    }

restart:                              // ON A LOAD PROG JUMPS TO HERE

    // Standard saves are created using a stored game state from before
    // the start of the turn. When loaded, they proceed into a new turn.
    // Autosaves are created using the game state when the end-of-turn
    // flag is selected, and restore into that same turn.
    // Start-of-turn activities, such as generating Intel briefings,
    // applying news events, and the like should not be performed.
    // This duplicates information, and that can lead to array overflow
    // errors.
    //
    // There's more than one way to determine this, but the easiest way
    // is to check how many news events have been recorded, since they
    // happen every turn. If a news event has been recorded, assume all
    // other start-of-turn upkeep occured.
    //
    // Turn 1 is Spring 1957. The event count begins at 0, and is
    // incremented as the news event text is being generated.
    turn = 2 * (Data->Year - 57) + Data->Season + 1; // Start on turn 1
    newTurn = (turn > Data->P[MAIL_INVERTED == 1 ? 1 : 0].eCount); // eCount starts at 0.

    LOAD = 0;                           // CLEAR LOAD FLAG

    while (Data->Year < 78) {            // WHILE THE YEAR IS NOT 1977

        if (newTurn) {
            // CLEAR ALL TURN RD MODS
            Data->P[0].RD_Mods_For_Turn = 0;
            Data->P[1].RD_Mods_For_Turn = 0;

            // RECORD BUDGET
            Data->P[0].BudgetHistory[Data->Year - 53] = Data->P[0].Budget;
            Data->P[1].BudgetHistory[Data->Year - 53] = Data->P[1].Budget;

            // MAKE ESTIMATE OF BUDGETS
            Data->P[0].BudgetHistoryF[Data->Year - 53] =
                (Data->P[0].Budget * (brandom(40) + 80)) / 100;
            Data->P[1].BudgetHistoryF[Data->Year - 53] =
                (Data->P[1].Budget * (brandom(40) + 80)) / 100;

            // MOVE EXPENDITURES DOWN
            for (int i = 0; i < NUM_PLAYERS; i++) {
                for ( int j = 3; j >= 0; j--) {
                    for (int k = 0; k < 4; k++) {
                        Data->P[i].Spend[j][k] =Data->P[i].Spend[j+1][k];
                    }
                }
                // RESET FIRST ROW
                for (int k = 0; k < 4; k++) {
                    Data->P[i].Spend[0][k] = 0;
                }
            }

            // Presidential Review of Space Program
            if (Data->Season == 0) {
                CalcPresRev();
            }

            // Intel only gets updated in the fall, because there are
            // only 30 Intel Briefing slots in the save record per
            // player.
            // Also, check to make sure this isn't an autosave repeating
            // the start-of-turn actions.
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (Data->Season == 1 && newTurn) {
                    IntelPhase(plr[i] - 2 * AI[i], 0);
                }
            }

        }

        for (int i = 0; i < NUM_PLAYERS; i++) {

            if (!i && (MAIL_PLAYER == 1)) { // Soviet turn coming up
                continue;
            }

            if (i && (MAIL_PLAYER == 0)) { // U.S. turn coming up
                continue;
            }

            xMODE &= ~xMODE_CLOUDS; // reset clouds for spaceport

            // computer vs. human

            if ((IDLE[0] > 12 || IDLE[1] > 12) || ((AI[i] && plr[other(i)] < NUM_PLAYERS && ((Data->Def.Lev1 != 0 && other(i) == 0) || (Data->Def.Lev2 != 0 && other(i) == 1))))) {
                if (IDLE[0] > 12 || IDLE[1] > 12 || Data->P[abs(i - 1)].PresRev[0] >= 16) {
                    helpText = "i136";
                    Data->P[abs(i - 1)].PresRev[0] = 0x7F;
                    helpText = "i000";

                    if (IDLE[0] > 12 || IDLE[1] > 12) {
                        SpecialEnd();
                    } else {
                        Review(abs(i - 1));
                        FakeWin(plr[i] - 2);
                    }

                    FadeOut(2, 10, 0, 0);
                    QUIT = 1;
                    return;
                }
            }

            if (!AI[i]) {
                NextTurn(plr[i]);
                VerifySafety(plr[i]);

                // Show prestige resulution from the previous turn
                if (MAIL == 1 || MAIL == 2) {

                    if (Data->Prestige[Prestige_MannedLunarLanding].Place != -1) {
                        UpdateRecords(1);
                        NewEnd(Data->Prestige[Prestige_MannedLunarLanding].Place, 0);

                        FadeOut(2, 10, 0, 0);
                        return;
                    }

                    if (Data->Year == 77 && Data->Season == 1 && Data->Prestige[Prestige_MannedLunarLanding].Place == -1 && MAIL == 1) { // Nobody won
                        SpecialEnd();
                        FadeOut(2, 10, 0, 0);
                        return;
                    }

                    PlayAllFirsts(MAIL_OPPONENT);
                    ShowPrestigeResults(MAIL_PLAYER);
                }

                News(plr[i]);                  // EVENT FOR PLAYER

                VerifyCrews(plr[i]);
                VerifySafety(plr[i]);
                helpText = "i000";
                keyHelpText = "k000";
                FixPrograms(plr[i]);

                //soften sound
//       SetVoiceVolume(80);            // 80% seems good MWR
                Master(plr[i]);                // PLAY TURN
                //restore sound
//       SetVoiceVolume(115);
                display::graphics.screen()->clear();

                if (LOAD == 1) {
                    goto restart;    // TEST FOR LOAD
                }

                IDLE.at(plr[i])++;
            } else {
                AI_Begin(plr[i] - 2); // Turns off Mouse for AI
                GetMouse();
                VerifySafety(plr[i] - 2);
                AIEvent(plr[i] - 2);
                VerifySafety(plr[i] - 2);
                AIMaster(plr[i] - 2);
                AI_Done(); // Fade Out AI Thinking Screen and Restores Mouse
            }

            // Only increase the global event counter if this is really a new
            // turn and not one already played in a save game
            if (Data->Count == 2 * (2 * (Data->Year - 57) + Data->Season)
                + (MAIL_INVERTED == 1 ? 1 ^ i : i)) {
                Data->Count++;
            }

            if (QUIT) {
                return;
            }
        }

        // Generate copy of the prestige data to be sent later to the
        // other side. We need to keep filling the prestige data to
        // prevent unwarranted milestone/duration penalties, but the
        // actual prestige resolution is done after the opponent's
        // turn.
        if (MAIL == 0 || MAIL == 3) {
            memcpy(oldPrestige, Data->Prestige, MAXIMUM_PRESTIGE_NUM * sizeof(struct PrestType));
            // Save the current mission counter
            old_mission_count = Data->P[MAIL_PLAYER].PastMissionCount;
        }

        DockingKludge();  // fixup for both sides

        // Do Missions Here
        int kik = OrderMissions();

        for (int i = 0; i < kik; i++) {
            if (Data->P[Order[i].plr].Mission[Order[i].loc].MissionCode) {
                if (AI[Order[i].plr] == 1) {
                    if (!CheckIfMissionGo(Order[i].plr, Order[i].loc)) {
                        ScrubMission(Order[i].plr, Order[i].loc);
                    }
                }

                if (Data->P[Order[i].plr].Mission[Order[i].loc].MissionCode) {
                    if (!AI[Order[i].plr]) {
                        IDLE[0] = IDLE[1] = 0;
                    }

                    if (Data->P[Order[i].plr].Mission[Order[i].loc].part != 1
                        && Data->P[Order[i].plr].Mission[Order[i].loc].Hard[4]) {
                        prest = Launch(Order[i].plr, Order[i].loc);

                        // check for prestige firsts
                        if (AI[Order[i].plr] == 1 || (MAIL == 1 && Order[i].plr == 0) || (MAIL == 2 && Order[i].plr == 1) && Data->Prestige[Prestige_MannedLunarLanding].Place == -1) {   // supposed to be 1
                            PlayAllFirsts(Order[i].plr);
                        }

                        if (Data->Prestige[Prestige_MannedLunarLanding].Place != -1) {
                            if (MAIL != 0 && MAIL != 3) {
                                UpdateRecords(1);
                                NewEnd(Data->Prestige[Prestige_MannedLunarLanding].Place, Order[i].loc);

                                // Immediately hand over to the other player
                                MailSwitchEndgame();

                                return;
                            }

                            // TODO: Don't perform missions after the L.L.
                            else {
                                break;
                            }


                        }

                        if (!(AI[Order[i].plr] || (MAIL == 1 && Order[i].plr == 0) || (MAIL == 2 && Order[i].plr == 1)) && prest != -20) { // -20 means scrubbed
                            MisRev(Order[i].plr, prest, Data->P[Order[i].plr].PastMissionCount - 1);
                        }
                    }
                }

            }                             //for(i=0...
        }

        if (MAIL == 0 || MAIL == 3) { // Hand over to the other side
            // Restore mission counter
            Data->P[MAIL_PLAYER].PastMissionCount = old_mission_count;
            // Restore prestige data to the status before the missions
            memcpy(Data->Prestige, oldPrestige, MAXIMUM_PRESTIGE_NUM * sizeof(struct PrestType));
            MAIL = MAIL_NEXT;

            MailSwitchPlayer();
            return;
        }

        Update();  /* Moves Future launches to Missions + More */

        if (Data->Year == 77 && Data->Season == 1 && Data->Prestige[Prestige_MannedLunarLanding].Place == -1) {
            // nobody wins .....
            SpecialEnd();
            MailSwitchEndgame();
            return;
        }

        Data->P[0].Budget += Data->P[0].Prestige;
        Data->P[1].Budget += Data->P[1].Prestige;

        if (Data->P[0].Budget > 180) {
            Data->P[0].Budget = 180;
        }

        if (Data->P[1].Budget > 180) {
            Data->P[1].Budget = 180;
        }

        // move prestige history down one;
        for (t3 = 0; t3 < 2; t3++) { // t3 is the index to the real and random hists
            for (t1 = 0; t1 < NUM_PLAYERS; t1++) { // t1 is the player index
                for (t2 = 4; t2 > 0; t2--) { // t2 is the time index
                    Data->P[t1].PrestHist[t2][t3] = Data->P[t1].PrestHist[t2 - 1][t3];
                }

                sign = (Data->P[t1].Prestige < 0) ? -1 : 1;

                Data->P[t1].PrestHist[0][t3] = (t3 == 0) ? Data->P[t1].Prestige
                                               : (Data->P[t1].Prestige * 4) / 5 + sign * brandom(Data->P[t1].Prestige * 2 / 5) + 1;

                if (t3 == 0) {
                    Data->P[t1].tempPrestige[Data->Season] += Data->P[t1].Prestige;
                }
            }
        }

        Data->P[0].Prestige = Data->P[1].Prestige = 0;

        if (Data->Season == 1) {
            for (int i = 0; i < NUM_PLAYERS; i++) {
                int p = plr[i] % NUM_PLAYERS;
                int pi = plr[i] - 2 * AI[i];

                assert(0 <= pi && pi <= NUM_PLAYERS);
                Data->P[p].Cash += Data->P[pi].Budget;

                if (Data->P[p].Cash > 999) {
                    Data->P[pi].Cash = 900;
                }
            }

            Data->Season = 0;
            Data->Year++;
        } else {
            Data->Season++;
        }


        // End of turn MAIL update
        if (MAIL == 1) {
            MAIL = 3;
        } else if (MAIL == 2) {
            MAIL = 0;
        }

        newTurn = true;

    }

    FadeOut(2, 10, 0, 0);

    Museum(0);

    Museum(1);

    return;
}


/**
 * Initialize audio channels.
 *
 * Loads the audio settings from the configuration file and sets the
 * audio channels per user preferences.
 */
void ConfigureAudio()
{
    AudioConfig audio = LoadAudioSettings();

    music_set_mute(audio.master.muted || audio.music.muted);
    MuteChannel(AV_SOUND_CHANNEL,
                audio.master.muted || audio.soundFX.muted);
}


void DockingKludge(void)
{
    for (int j = 0; j < NUM_PLAYERS; j++) {
        Data->P[j].Misc[MISC_HW_DOCKING_MODULE].MSF =
            MAX(MAX(Data->P[j].Probe[PROBE_HW_ORBITAL].Safety, Data->P[j].Probe[PROBE_HW_INTERPLANETARY].Safety),
                Data->P[j].Probe[PROBE_HW_LUNAR].Safety);
    }

    return;
}


/** Destroy Pad and Reset any Crews affected
 *
 * \param cost amount of MB to pay for repairs
 * \param pad number of the launch facility destroyed
 * \param plr Player data
 * \param mode 0 if future mission, 1 is current mission
 */
void DestroyPad(char plr, char pad, int cost, char mode)
{
    Data->P[plr].LaunchFacility[pad] = cost; // Destroys pad

    if (mode == 0) {
        ClrFut(plr, pad);
    } else {
        ScrubMission(plr, pad);
    }

    return;
}

void
GetMouse(void)
{
    av_block();
    GetMouse_fast();
}


/* get mouse of keyboard input, non-blocking */
void GetMouse_fast(void)
{
    mousebuttons = 0;
    oldx = x;
    oldy = y;

    gr_sync();
    av_step();

    if (grGetMouseButtons()) {
        mousebuttons = 1;
        grGetMousePressedPos(&x, &y);
    } else {
        mousebuttons = 0;
        grGetMouseCurPos(&x, &y);
    }

    while (is_new_key_available()) {
        key = get_pressed_key();

        if ((key & 0x00ff) > 0) {
            key = toupper(key & 0xff);
        }

        mousebuttons = 0;
        //grSetMousePos(319,199);
    }

    if (mousebuttons > 0) {
        key = 0;
    }

    if (key >> 8 == 15) {
        CloseEmUp(0, 0);
    } else if (AL_CALL == 0 && (key >> 8 == 0x3B)) {
        if (mousebuttons != 1) {
            Help(helpText.c_str());
        }
    } else if (AL_CALL == 0 && ((key >> 8) == 0x3C)) {
        Help(keyHelpText.c_str());
    } else if (AL_CALL == 0 && ((key >> 8) == 0x3D)) {
        Help("i123");
    }
}

void WaitForMouseUp(void)
{
    // Wait for mouse and key to be up
    while (mousebuttons) {
        GetMouse();
    }
}

void WaitForKeyOrMouseDown(void)
{
    // Wait for mouse and key to be up
    while (mousebuttons == 0 && key == 0) {
        GetMouse();
    }
}


void PauseMouse(void)
{
    /* wait until mouse button is released */
    while (1)  {
        GetMouse(); /* blocks briefly */

        if (mousebuttons == 0) {
            break;
        }
    }

    GetMouse();
}

void VerifySafety(char plr)
{
    // Pointers to each Equipment struct
    Equipment* arrays[4] = {
        Data->P[plr].Probe,
        Data->P[plr].Rocket,
        Data->P[plr].Manned,
        Data->P[plr].Misc
    };
    // Corrected sizes for each Equipment (Probe, Rocket, Manned, Misc)
    int sizes[4] = { 3, 5, 7, 6 };

    // Check for safety for each Equipment
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < sizes[i]; j++) {
            Equipment *px = &arrays[i][j];

            if (px->Safety > px->MaxSafety) {
                px->Safety = px->MaxSafety;
            }

            if (px->Safety < px->Base) {
                px->Safety = px->Base;
            }
        }
    }
}

void VerifyCrews(char plr)
{
    for (int i = 0; i < 3; i++) {
        if (Data->P[plr].Mission[i].MissionCode == Mission_Jt_Unmanned_Orbital_Docking && Data->P[plr].Mission[i].part == 0) {
            Data->P[plr].Mission[i].Joint = 1;
            Data->P[plr].Mission[i + 1].Joint = 1;
            Data->P[plr].Mission[i].part = 0;
            Data->P[plr].Mission[i + 1].part = 1;
        }

        if (Data->P[plr].Mission[i].PCrew > 0) { // primary verify
            int t = Data->P[plr].Mission[i].Prog;
            int k = Data->P[plr].Mission[i].PCrew - 1;

            if (Data->P[plr].CrewCount[t][k] == 0) {
                Data->P[plr].Mission[i].PCrew = 0;
            }

            t = Data->P[plr].Mission[i].Prog; // backup verify
            k = Data->P[plr].Mission[i].BCrew - 1;

            if (Data->P[plr].CrewCount[t][k] == 0) {
                Data->P[plr].Mission[i].BCrew = 0;
            }

        }
    }
}


/* Mission random number generator used on level 1. For random numbers
 * above brandom_threshold, there is a second roll of a Gaussian
 * random number generator that is slightly biased against high
 * numbers.
 */
int MisRandom(void)
{
    const double mu = 57;
    const double sigma = sqrt(1000);
    const int brandom_threshold = 66;
    double u1, u2, r_gaussian;

    int r_uniform = brandom(100) + 1;

    if (r_uniform < brandom_threshold) {
        return r_uniform;
    }

    do {
        // Generate two uniformly distributed random numbers.
        u1 = rand() / (double) RAND_MAX;
        u2 = rand() / (double) RAND_MAX;

        // Box-Muller transform to obtain a Gaussian random variable
        // with mean mu and standard deviation sigma. A value of 0.5
        // is added to ensure correct rounding.

        r_gaussian = sigma * sqrt(-2 * log(u1)) * cos(2 * M_PI * u2) + mu + 0.5;

    } while ((r_gaussian >= 101) || (r_gaussian < brandom_threshold));

    return (int) r_gaussian;
}


