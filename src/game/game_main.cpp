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

#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "filesystem.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "options.h"
#include "utils.h"
#include "admin.h"
#include "aimast.h"
#include "ast4.h"
#include "endgame.h"
#include "intel.h"
#include "intro.h"
#include "mc.h"
#include "mis_c.h"
#include "mission_util.h"
#include "museum.h"
#include "newmis.h"
#include "news.h"
#include "place.h"
#include "port.h"
#include "prefs.h"
#include "records.h"
#include "review.h"
#include "start.h"
#include "state_utils.h"
#include "pace.h"
#include "sdlhelper.h"
#include "gr.h"
#include "crash.h"
#include "endianness.h"
#include "crew.h"

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
unsigned char HARD1;
unsigned char UNIT1;
unsigned char FADE;
unsigned char AL_CALL;
char plr[NUM_PLAYERS];
std::string helpText;
std::string keyHelpText;
char IDLE[2];
char *buffer;
char pNeg[NUM_PLAYERS][MAX_MISSIONS];
int32_t xMODE;
char MAIL = -1;
char Option = -1;
int fOFF = -1;
// true for fullscreen mission playback, false otherwise
bool fullscreenMissionPlayback;
char manOnMoon = 0;
char dayOnMoon = 20;
char AI[2] = {0, 0};
// Used to hold mid-turn save game related information
INTERIMDATA interimData;

char *S_Name[] = {
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

#include <sstream>
#include <stdexcept>

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)


void Rout_Debug(int line, char *file);
void RestoreDir(void);
int CheckIfMissionGo(char plr, char launchIdx);
void oclose(int fil);
void InitData(void);
void MMainLoop(void);
void Progress(char mode);
void MainLoop(void);
void DockingKludge(void);
void OpenEmUp(void);
void CloseEmUp(unsigned char error, unsigned int value);
void VerifyCrews(char plr);


int game_main_impl(int argc, char *argv[])
{
    FILE *fin;
    const char *see_readme = "look for further instructions in the README file";

    char ex;

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

    fin = open_gamedat("USA_PORT.DAT");

    if (fin == NULL) {
        CRITICAL1("can't find game data files");
        NOTICE1("set environment variable BARIS_DATA or edit config file");
        NOTICE2("%s", see_readme);

        crash("Data missing", "Unable to locate game data files.");
    }

    fclose(fin);

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

    xMODE |= xMODE_NOCOPRO;

    Data = (Players *)xmalloc(sizeof(struct Players) + 1);
    buffer = (char *)xmalloc(BUFFER_SIZE);

    DEBUG3("main buffer %p (%d)", buffer, BUFFER_SIZE);

    memset(buffer, 0x00, BUFFER_SIZE);

    OpenEmUp();                   // OPEN SCREEN AND SETUP GOODIES

    if (options.want_intro) {
        Introd();
    }

    ex = 0;

    while (ex == 0) {

        MakeRecords();

#define UNCOMPRESSED_RAST 1

#ifndef UNCOMPRESSED_RAST
        fin = sOpen("RAST.DAT", "rb", 0);
        i = fread(buffer, 1, BUFFER_SIZE, fin);
        fclose(fin);

        DEBUG2("reading Players: size = %d", (int)sizeof(struct Players));
        RLED(buffer, (char *)Data, i);
#else
        fin = sOpen("URAST.DAT", "rb", 0);
        fread(Data, 1, (sizeof(struct Players)), fin);
        fclose(fin);
#endif
        SwapGameDat();  // Take care of endian read

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

        switch (MainMenuChoice()) {
        case 0:  // New Game
            LOAD = QUIT = 0;
            HARD1 = UNIT1 = 0;
            MAIL = -1;
            Option = -1;
            helpText = "i013";
            Prefs(0);                     // GET INITIAL PREFS FROM PLAYER
            plr[0] = Data->Def.Plr1;       // SET GLOBAL PLAYER VALUES
            plr[1] = Data->Def.Plr2;
            Data->plr[0] = Data->Def.Plr1;  // SET STRUCTURE PLAYER VALUES
            Data->plr[1] = Data->Def.Plr2;

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

            InitData();                   // PICK EVENT CARDS N STUFF
            MainLoop();                   // PLAY GAME
            display::graphics.screen()->clear();
            break;

        case 1: // Play Old Game
            LOAD = QUIT = 0;
            HARD1 = UNIT1 = 0;
            MAIL = -1;
            Option = -1;
            FileAccess(1);

            if (LOAD == 1) {
                if (Option == -1 && MAIL == -1) {
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

        case 2:
            Credits();
            break;

        case 3:
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
    int i, j;

    InitializeEvents();                  // RESET EVENT CARDS
    Data->Count = 0;               // SET EVENT COUNTER TO ZERO

    for (j = 0; j < NUM_PLAYERS; j++) {
        for (i = 0; i < 5; i++) {
            Data->P[j].PresRev[i] = 8;
        }
    }

    return;
}

void MainLoop(void)
{
    int i, j, t1, t2, t3, prest, sign, turn, kik;
    bool newTurn;

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
    newTurn = (turn > Data->P[0].eCount);    // eCount starts at 0.

    LOAD = 0;                           // CLEAR LOAD FLAG

    while (Data->Year < 78) {            // WHILE THE YEAR IS NOT 1977
        EndOfTurnSave((char *) Data, sizeof(struct Players));

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

            // Move Expenditures down one
            for (t1 = 0; t1 < NUM_PLAYERS; t1++) {
                for (t2 = 4; t2 >= 0; t2--) {
                    for (t3 = 0; t3 < 4; t3++) {
                        Data->P[t1].Spend[t2][t3] =
                            Data->P[t1].Spend[t2 - 1][t3];
                    }
                }

                for (t3 = 0; t3 < 4; t3++) {
                    Data->P[t1].Spend[0][t3] = 0;
                }
            }

            // Presidential Review of Space Program
            if (Data->Season == 0) {
                CalcPresRev();
            }
        }

        for (i = 0; i < NUM_PLAYERS; i++) {
            xMODE &= ~xMODE_CLOUDS; // reset clouds for spaceport

            // Intel only gets updated in the fall, because there are
            // only 30 Intel Briefing slots in the save record per
            // player.
            // Also, check to make sure this isn't an autosave repeating
            // the start-of-turn actions.
            if (Data->Season == 1 && newTurn) {
                IntelPhase(plr[i] - 2 * AI[i], 0);
            }

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
                VerifySF(plr[i]);
                News(plr[i]);                  // EVENT FOR PLAYER

                if ((Data->P[plr[i] % NUM_PLAYERS].Mission[0].MissionCode > 6 ||
                     Data->P[plr[i] % NUM_PLAYERS].Mission[1].MissionCode > 6 ||
                     Data->P[plr[i] % NUM_PLAYERS].Mission[2].MissionCode > 6) &&
                    (NOCOPRO && !EASYMODE)) {
                    xMODE &= ~xMODE_NOCOPRO;
                }

                VerifyCrews(plr[i]);
                VerifySF(plr[i]);
                helpText = "i000";
                keyHelpText = "k000";
                FixPrograms(plr[i]);

                //soften sound
//       SetVoiceVolume(80);            // 80% seems good MWR
                Master(plr[i]);                // PLAY TURN
                //restore sound
//       SetVoiceVolume(115);
                display::graphics.screen()->clear();
                IDLE[plr[i]]++;

                if (LOAD == 1) {
                    goto restart;    // TEST FOR LOAD
                }
            } else {
                AI_Begin(plr[i] - 2); // Turns off Mouse for AI
                GetMouse();
                VerifySF(plr[i] - 2);
                AIEvent(plr[i] - 2);
                VerifySF(plr[i] - 2);
                AIMaster(plr[i] - 2);
                AI_Done(); // Fade Out AI Thinking Screen and Restores Mouse
            }

            Data->Count++;

            if (QUIT) {
                return;
            }
        }

        DockingKludge();  // fixup for both sides

        // Do Missions Here
        kik = OrderMissions();

        for (i = 0; i < kik; i++) {
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

                    if (!(Data->P[Order[i].plr].Mission[Order[i].loc].part == 1
                          || Data->P[Order[i].plr].Mission[Order[i].loc].Hard[4] ==
                          0)) {
                        prest = Launch(Order[i].plr, Order[i].loc);

                        // check for prestige firsts
                        if (AI[Order[i].plr] == 1 && Data->Prestige[Prestige_MannedLunarLanding].Place == -1)    // supposed to be 1
                            for (j = 0; j < 28; j++) {
                                if (j != 4 && j != 5 && j != 6)
                                    if (Data->Prestige[j].Place == Order[i].plr
                                        && Data->PD[Order[i].plr][j] != 1) {
                                        PlayFirst(Order[i].plr, j);
                                    }
                            }

                        if (Data->Prestige[Prestige_MannedLunarLanding].Place != -1) {
                            UpdateRecords(1);
                            NewEnd(Data->Prestige[Prestige_MannedLunarLanding].Place, Order[i].loc);
                            FadeOut(2, 10, 0, 0);
                            return;
                        }

                        if (!AI[Order[i].plr] && prest != -20) { // -20 means scrubbed
                            MisRev(Order[i].plr, prest);
                        }
                    }
                }

            }                             //for(i=0...
        }

        Update();  /* Moves Future launches to Missions + More */

        if (Data->Year == 77 && Data->Season == 1 && Data->Prestige[Prestige_MannedLunarLanding].Place == -1) {
            // nobody wins .....
            SpecialEnd();
            FadeOut(2, 10, 0, 0);
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
            for (i = 0; i < NUM_PLAYERS; i++) {
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

        newTurn = true;
    }

    FadeOut(2, 10, 0, 0);

    Museum(0);

    Museum(1);

    return;
}

void DockingKludge(void)
{
    int j;

    for (j = 0; j < NUM_PLAYERS; j++) {
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
    struct MissionType *BMis = NULL, *AMis = NULL;

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

    while (bioskey(1)) {
        key = bioskey(0);

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

void VerifySF(char plr)
{
    int i;
    Equipment *px;

    // TODO: This is _very_ implementation-specific, relying on the
    // Probe, Rocket, Manned, and Misc arrays all being defined as
    // Equipment arrays of length 7 being defined consecutively in
    // struct BuzzData. This should be rewritten.
    for (i = 0; i < 28; i++) {
        px = &Data->P[plr].Probe[i];

        if (px->Safety > px->MaxSafety) {
            px->Safety = px->MaxSafety;
        }

        if (px->Safety < px->Base) {
            px->Safety = px->Base;
        }
    }
}

void VerifyCrews(char plr)
{
    int i, t, k;

    for (i = 0; i < 3; i++) {
        if (Data->P[plr].Mission[i].MissionCode == Mission_Jt_Unmanned_Orbital_Docking && Data->P[plr].Mission[i].part == 0) {
            Data->P[plr].Mission[i].Joint = 1;
            Data->P[plr].Mission[i + 1].Joint = 1;
            Data->P[plr].Mission[i].part = 0;
            Data->P[plr].Mission[i + 1].part = 1;
        }

        if (Data->P[plr].Mission[i].PCrew > 0) { // primary verify
            t = Data->P[plr].Mission[i].Prog;
            k = Data->P[plr].Mission[i].PCrew - 1;

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

/* Reads mission data for the specified mission into the global
 * variable Mis.
 *
 * Opens "MISSION.DAT" to find mission data. Global variable Mis
 * defined in mc.cpp.
 *
 * \param mcode Code of the mission - works as index for the file
 *
 * \todo This is highly coupled to game logic
 * and size of internal structures.
 */
void GetMisType(char mcode)
{
    FILE *fin;
    /** \todo Missing assert for opening file? */
    fin = sOpen("MISSION.DAT", "rb", 0);
    fseek(fin, mcode * (sizeof Mis), SEEK_SET); // Find Mission Type
    fread(&Mis, sizeof Mis, 1, fin);         // Get new Mission Info
    fclose(fin);
}


int MisRandom(void)
{
    int i, nval;

    do {
        nval = 107;

        for (i = 0; i < 250; i++) {
            nval += (brandom(7) - 3);
        }
    } while (nval < 50 || nval > 150);

    return nval - 50;
}
