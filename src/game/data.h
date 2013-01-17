#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>

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
/**
\page eventcards Event Card Structure
 *
 * \section datafile Sample Work for Data File Saving and Loading
 *
 * \subsection missioncode MissionCode
 *         x = Mission Type.
 *
 * \todo A load of code uses these number directly.
 * We should fix this and attach propper attributes to
 * the missions or keep lists of missions having these attributes.
 *
 * \subsection turnonly TurnOnly
 * \li 1 = No new astronauts
 * \li 2 = No manned missions this year
 * \li 3 = Hardware 50% off this season
 * \li 4 = Hardware 50% off next season
 * \li 5 = 50% chance of explosion on pad
 * \li 6 = Only one launch this turn
 *
 * \section futureplans FuturePlans
 * \li  1 = Multi-Manned Mission
 * \li  2 = Satellite
 * \li  3 = Manned Launch
 * \li  4 = Joint Mission
 *
 * \section astronautstatus Astronaut.Status
 * \li   0 = Active
 * \li   1 = Dead
 * \li   2 = Retired
 * \li   3 = Injured
 * \li   4 = Training Season I
 * \li   5 = Training Season II
 * \li   6 = Training Season III
 * \li   7 = Advanced Training I
 * \li   8 = Advanced Training II
 * \li   9 = Advanced Training III
 * \li  10 = Advanced Training IV
 *
 * \section astronauttraininglevel Astronaut.TrainingLevel:
 * \li   0 = None
 * \li   1 = Basic Training I
 * \li   2 = Basic Training II
 * \li   3 = Basic Training III
 * \li   4 = Advanced Training I
 * \li   5 = Advanced Training II
 * \li   6 = Advanced Training III
 * \li   7 = Advanced Training IV
 *
 * \section astronautspecial Astros::Special
 *                 News specials
 * \li   1 = Retirement announcement
 * \li   2 = Actual retirement
 * \li   3 = ??? unused ???
 * \li   4 = ??? unused ???
 * \li   5 = Injury resolution
 * \li   6 = ??? Breaking Group/Replacing Astronaut ???
 * \li   7 = Finished Training
 * \li   8 = Finished Advanced
 * \li   9 = Injury during training
 * \li  10 = Retirement after training
*/

// BARIS program definitions

#define NUM_PLAYERS             2 /**< number of players */
#define MAX_LAUNCHPADS          3 /**< number of launchpads available */
#define MAX_MISSIONS            3 /**< number of concurrent missions available */

#define MAXIMUM_NEWS_EVENTS         100
#define MAXIMUM_PRESTIGE_NUM                28

/** \name Astronaut related
 *@{
 */
#define ASTRONAUT_POOLS             5
#define ASTRONAUT_CREW_MAX          8
#define ASTRONAUT_FLT_CREW_MAX      4
/*@}*/

#pragma pack(push, 1)

struct Prest_Upd {
    char Poss[10];
    char PossVal[10];
    char HInd;
    char Qty;
    char Mnth;
};

/**
 * Player default settings
 */
struct Defl {
    char Plr1, Plr2;    /**< 0=USA,1=USSR,2=AI+USA,3=AI+USSR,6=Remote+USA,7=Remote+USSR */
    char Lev1, Lev2;  /**< 0=Easy, 1=Medium, 2=Hard */
    char Ast1, Ast2;  /**< 0=Easy, 1=Medium, 2=Hard */
    char Input;       /**< 0=Mouse, 1=Keyboard, 2=Joystick */
    char Anim;        /**< 0=Full, 1=Partial, 2=Results Only */
    char Music;       /**< 0=Full, 1=Partial, 2=None */
    char Sound;       /**< 0=On, 1=Off */
};

/** The Prestige for any achievement is stored in this.
*/
struct PrestType {
    char Add[4];        /**< 4&5 used for qty completed */
    char Goal[NUM_PLAYERS];       /**< Goal steps used for prest calculations */
    int16_t Points[NUM_PLAYERS];  /**< Total points earned */
    char Index;         /**< the index for the first into .History[] */
    char Place;         /**< Who got the prestige first. */
    char mPlace;        /**< If you've done it at all */
    char Month;         /**< Date of the First */
    char Year;          /**< Date of the First */
};

enum LaunchPads {
    PAD_NONE = -1,
    PAD_A = 0,
    PAD_B = 1,
    PAD_C = 2,
};

enum EquipmentIndex {
    PROBE_HARDWARE  = 0,   /**< unmanned satelite */
    ROCKET_HARDWARE = 1,   /**< propulsion system */
    MANNED_HARDWARE = 2,   /**< vehicle to transport humans */
    MISC_HARDWARE   = 3,   /**< piece of hardware to support other stuff */
};

enum EquipProbeIndex {
    PROBE_HW_ORBITAL,        // 0
    PROBE_HW_INTERPLANETARY, // 1
    PROBE_HW_LUNAR           // 2
};

enum EquipRocketIndex {
    ROCKET_HW_ONE_STAGE,   // 0
    ROCKET_HW_TWO_STAGE,   // 1
    ROCKET_HW_THREE_STAGE, // 2
    ROCKET_HW_MEGA_STAGE,  // 3
    ROCKET_HW_BOOSTERS,    // 4
};

enum EquipMannedIndex {
    MANNED_HW_ONE_MAN_CAPSULE,   // 0
    MANNED_HW_TWO_MAN_CAPSULE,   // 1
    MANNED_HW_THREE_MAN_CAPSULE, // 2
    MANNED_HW_MINISHUTTLE,       // 3
    MANNED_HW_FOUR_MAN_CAPSULE,  // 4
    MANNED_HW_TWO_MAN_MODULE,    // 5
    MANNED_HW_ONE_MAN_MODULE     // 6
};

enum EquipMiscIndex {
    MISC_HW_KICKER_A,       // 0
    MISC_HW_KICKER_B,       // 1
    MISC_HW_KICKER_C,       // 2
    MISC_HW_EVA_SUITS,      // 3
    MISC_HW_DOCKING_MODULE, // 4
    MISC_HW_PHOTO_RECON     // 5
};


typedef struct _Equipment {
    char Name[20];      /**< Name of Hardware */
    char ID[2];         /**< EquipID "C0 C1 C2 C3 : Acts as Index */
    int16_t Safety;     /**< current safety factor */
    int16_t MisSaf;     /**< Safety During Mission */
    int16_t MSF;        /**< used only to hold safety for docking kludge / Base Max R&D for others (from 1.0.0)*/
    char Base;          /**< initial safety factor */
    int16_t InitCost;   /**< Startup Cost of Unit */
    char UnitCost;      /**< Individual Cost */
    int16_t UnitWeight; /**< Weight of the Item */
    int16_t MaxPay;     /**< Maximum payload */
    char RDCost;        /**< Cost of R&D per roll */
    char Code;          /**< Equipment Code for qty scheduled */
    char Num;           /**< quantity in inventory */
    char Spok;          /**< qty being used on missions */
    char Seas;          /**< Seasons Program is Active */
    char Used;          /**< total number used in space */
    char unused_IDX[2];        /**< unsed - EquipID "C0 C1 C2 C3 : Acts as Index */
    int16_t Steps;      /**< Program Steps Used */
    int16_t Failures;   /**< number of program failures */
    char MaxRD;         /**< maximum R & D */
    char MaxSafety;     /**< maximum safety factor */
    char SMods;         /**< safety factor mods for next launch */
    char SaveCard;      /**< counter next failure in this prog */
    char Delay;         /**< delay in purchase - in seasons */
    char Duration;      /**< Days it can last in space */
    char Damage;        /**< Damage percent for next launch */
    char DCost;         /**< Cost to repair damage */
    char MisSucc;       /**< Mission Successes */
    char MisFail;       /**< Mission Failures */
} Equipment;

struct MissionType {
    char Name[25];      /**< name of mission */
    char MissionCode;   /**< internal code of mission type */
    char Patch;         /**< Patch Used on this Mission */
    char part;          /**< primary/secondary portion */
    char Hard[6];       /**< Prime,Kick,LEM,PayLoad,Rocket */
    char Joint;         /**< Joint Mission flag */
    char Rushing;       /**< rushing time 0-3 */
    char Month;         /**< Month of Launch */
    char Duration;      /**< Duration time */
    char Men;           /**< Qty of men used in mission */
    char Prog;          /**< Hardware program used */
    char PCrew;         /**< Primary Crew */
    char BCrew;         /**< Backup Crew */
    char Crew;          /**< Actual Mission Crew */
};

/**
 * \brief Number of astronauts to recruit in each level
*/
enum AstronautPoolClass {
    ASTRO_POOL_LVL1 = 7,
    ASTRO_POOL_LVL2 = 9,
    ASTRO_POOL_LVL3 = 14,
    ASTRO_POOL_LVL4 = 16,
    ASTRO_POOL_LVL5 = 14
};

/* WARNING: these numbers are used throughout the code, do not change! */
enum AstronautStatus {
    AST_ST_ACTIVE = 0,
    AST_ST_DEAD = 1,
    AST_ST_RETIRED = 2,
    AST_ST_INJURED = 3,
    AST_ST_TRAIN_BASIC_1 = 4,
    AST_ST_TRAIN_BASIC_2 = 5,
    AST_ST_TRAIN_BASIC_3 = 6,
    AST_ST_TRAIN_ADV_1 = 7,
    AST_ST_TRAIN_ADV_2 = 8,
    AST_ST_TRAIN_ADV_3 = 9,
    AST_ST_TRAIN_ADV_4 = 10,
};

enum AstronautAdvancedFocus {
    AST_FOCUS_CAPSULE = 1,
    AST_FOCUS_LEM = 2,
    AST_FOCUS_EVA = 3,
    AST_FOCUS_DOCKING = 4,
    AST_FOCUS_ENDURANCE = 5,
};

enum AstronautMissionStatus {
    ASTRO_MISSION_CLEAR = 0,
    ASTRO_MISSION_SUCCESS = 1,
    ASTRO_MISSION_FAILURE = 2,
    ASTRO_MISSION_SCRUBBED = 3,
};


enum AI_Tracking {
    AI_ORBITAL_SATELLITE = 0,
    AI_END_STAGE_LOCATION = 1,
    AI_LUNAR_MODULE = 2,
    AI_STRATEGY = 3,  // 0=none, 1=one, 2=two
    AI_BEGINNING_STRATEGY = 4,
    AI_LARGER_ROCKET_STRATEGY = 5,
};


/**
\brief This is a structure representing astronauts/cosmonauts.
*/
struct Astros {
    char Name[14];     /**< Astronaut Name */
    char Face;         /**< Code for Astronaut Face */
    char Sex;          /**< Male or Female */
    char Compat;       /**< Compatability Code */
    char CR;           /**< Range of Compatability Right */
    char CL;           /**< Range of Compatability Left */
    char Moved;        /**< if 0 Astro Moved around this turn */
    char currentMissionStatus;          /**< Successful Mission this Turn */
    char Happy;        /**< \brief Happy within group */
    /**< \li 1 = Successful Mission */
    /**< \li 2 = Personal First */
    char Prime;        /**< \brief indicates the type of crew s/he's in */
    /**< \li 2 = Primary Crew Member */
    /**< \li 1 = Backup Crew Member */
    /**< \li 0 = Not selected */
    char Active;       /**< Seasons Active */
    char AISpecial;    /**< AI Special Astro Note */
    char Service;      /**< Service of Origination */
    char Status;       /**< Current status of Astronaut */
    char Focus;        /**< Focus of Advanced Training 0-4 */
    char InjuryDelay;       /**< Injury Delay - Ok if 0 */
    char RetirementDelay;       /**< Retire Delay - Retire if 0 */
    char RetirementReason;      /**< Retire Reason flag */
    char Special;      /**< Special for announcements */
    char Assign;       /**< Location of Astro 0=limbo */
    char oldAssign;    /**< program of last turn */
    char Unassigned;   /**< Unassigned signed flag */
    char Crew;         /**< Crew Assiciated in prog Assigm */
    char Task;         /**< What his job is in Flt Crew */
    int16_t Prestige;  /**< Prestige Earned */
    char Days;         /**< Days in Space */
    char Medals;       /**< Medals Earned */
    char Save;         /**< Astro Saving Roll Percent */
    char Missions;     /**< Number of Missions Flown */
    char MissionNum[10];       /**< Mission numbers participated */
    char Cap;          /**< Capsule Pilot Skills */
    char LM;           /**< LEM Pilot Skills */
    char EVA;          /**< EVA Skills */
    char Docking;      /**< Docking Skills */
    char Endurance;    /**< Constitution Skills */
    char TrainingLevel;/**< Level of Schooling */
    char Group;        /**< Group Entered with */
    char Mood;         /**< Mood of Astronaut */
    char Pool;         /**< Temp Storage for Astros */
    char Hero;         /**< Hero Status */
};

/**
 * \brief For Records of Missions
*/
struct PastInfo {
    char MissionName[2][25];       /**< Name of Mission */
    char Patch[2];                 /**< Patch Type */
    char MissionCode;              /**< Type of Mission */
    char MissionYear;              /**< Year Mission Took Place */
    char Month;                    /**< Month of Launch */
    char Time;                     /**< Early / Late Flag */
    char Man[2][4];                /**< Astros # involved in Mis */
    char Hard[2][5];               /**< Set of hardware */
    uint16_t result;               /**< Success / Failure */
    uint16_t spResult;             /**< Actual deciding Result */
    char Event;                    /**< Single event code */
    char Saf;                      /**< Safety of the Equipment for delays */
    char Part;                     /**< Participant in event code */
    int16_t Prestige;              /**< Prestige Earned on this Mission */
    char Duration;                 /**< Length of Mission (units:ABCDEF) */
};

struct BuzzData {                   // master data list for Buzz Aldrin's
    char Header[4];                  // Sync information
    char Name[20];                   // Player Name
    char unused_Level; // unused
    int16_t Cash;                      // Current Cash on Hand
    int16_t Budget;                    // Next Season's Budget
    int16_t Prestige;                    // Prestige Earned
    int16_t PrestHist[5][2];             // Prestige History array -- [][historical|random]
    int16_t PresRev[5];                  // Pres. Review History
    int16_t tempPrestige[2];                 // Holds this and prev prestige
    int16_t BudgetHistory[40];         // Record of Last 40 Budgets
    int16_t BudgetHistoryF[40];        // Record of Last 40 Budgets
    int16_t Spend[5][4];                 // Record of last 5 seasons spending
    //  of each equip type
    char RD_Mods_For_Turn;                     // R & D Mods for turn only
    char RD_Mods_For_Year;                     // R & D Mods for full year
    char TurnOnly;                   // Events for this turn only
    char Plans;                      // Code for future plans
    // 1 = Manned Launch, -10 MB
    char FuturePlans;                // Special future mission
    char DurationLevel;                      // Current Duration Level
    char LMpts;                      // Accumulation of any LM Test points
    Equipment Probe[7];       // 0 = Orbital
    // 1 = Inter Planetary
    // 2 = Lunar Probe
    Equipment Rocket[7];      // 0 = One Stage
    // 1 = Two Stage
    // 2 = Three Stage
    // 3 = Mega Stage
    // 4 = Strap On Boosters
    Equipment Manned[7];      // 0 = One Man Capsule
    // 1 = Two Man Capsule
    // 2 = Three Man Capsule
    // 3 = Minishuttle
    // 4 = Four Man Cap/Mod
    // 5 = Two Man Module
    // 6 = One Man Module
    Equipment Misc[7];        // 0 = Kicker Level A
    // 1 = Kicker Level B
    // 2 = Kicker Level C
    // 3 = EVA Suits
    // 4 = Docking Modules
    // 5 = Photo Recon
    char unused_ZCost; // unused
    char unused_ZFlag; // unused
    char DockingModuleInOrbit;                       // Docking Module in Orbit, in seasons
    char LaunchFacility[MAX_LAUNCHPADS];    /**< -1=no facility; 1=purchased, >1 repair cost) */
    char AstroCount;                 // Current # in Program
    char AstroLevel;                 // Level of selection
    char AstroDelay;                 // Wait until next selection
    struct Astros Pool[65];          // Pool of SpaceMen
    struct {
        char Vle;
        char Asp;
    } unused_WList[5]; // unused
    struct Prest_Upd Udp[3];
    char unused_WTop; // unused
    char unused_VList[5]; // unused
    char unused_VTop; // unused
    char IntelHardwareTable[5][7];                // safety factor for intelligence
    char CrewCount[ASTRONAUT_POOLS + 1][ASTRONAUT_CREW_MAX];               // Count of num in groups
    char Crew[ASTRONAUT_POOLS + 1][ASTRONAUT_CREW_MAX][ASTRONAUT_FLT_CREW_MAX + 1];          // Flt Crews
    // [5] - Program #
    // [8] - Eight Crews Max
    // [4] - Four Max per Crew
    char FemaleAstronautsAllowed;                     // FemaleAstronautsAlloweds allowed in program
    struct MissionType Mission[MAX_MISSIONS];   // Current Mission Info
    char Block;                      // Blocked Mission Number
    struct MissionType Future[MAX_MISSIONS];    // Future Mission Info
    struct PastInfo History[100];    // Past Mission History
    int16_t PastMissionCount;                   // Number of Past Missions
    char MissionCatastrophicFailureOnTurn;                      // Catastrophic Fail on Turn
    // 1 = Cat Failure
    // 2 = Program Death
    // 3 = Program First
    uint8_t Buy[4][7];  // Record of R&D for Turn - encoded dices & rolls
    char eCount;
    struct {
        char code;
        int16_t num;
        char prog;
        char cur;
        char index;
        char SafetyFactor;
        char cdex;
    } PastIntel[30];
    char unused_AIpath[23]; // unused
    char AIStrategy[10];                  // AI Strategy Values
    char AILunar,                    // Flag to tell way to moon
         AIPrim,                     // Currect Primary Prog
         AISec,                      // Currect Secondary Prog
         unused_AIEva, // unused
         unused_AIDur, // unused
         unused_AIDck, // unused
         AIStat,                     // the status of program
         unused_AITech, // unused
         unused_AILem; // unused
    char unused_X[20]; // unused
    char unused_AIMissions[62];                      // How many missions of type done
    char BadCardEventFlag[15];                // Flag for Bad Events
    char Port[40];                   // Levels for SPort display
};

struct Players {
    char BUZZ[4];                    /**< Save Version Marker */
    uint32_t Checksum;               /**< Checksum of Data */
    char plr[NUM_PLAYERS];           /**< Order of Turns */
    struct Defl Def;                 /**< Defaults */
    char Year;                       /**< Game Turn */
    char Season;                     /**< Season of Year */
    struct PrestType Prestige[MAXIMUM_PRESTIGE_NUM];   /**< Definitions of Prest Vals */
    struct BuzzData P[NUM_PLAYERS];  /**< Player Game Data */
    char unused_EMark[4];                   /**< unused - Event Marker */
    char Events[MAXIMUM_NEWS_EVENTS]; /**< History of Event Cards */
    char Count;                      /**< Number of Events Picked */
    char PD[NUM_PLAYERS][MAXIMUM_PRESTIGE_NUM];
    char Mile[NUM_PLAYERS][10];      /**< MileStone Calcs */
};



/** MissionHard Descriptions
 * \verbatim
   0=Capsule
   1=Kicker
   2=LM
   3=Probe/DM
   4=Primary Booster
   5=EVA
   6=Photo Recon
   7=Secondary Booster
 * \endverbatim
*/
enum MissionHardwareType {
    Mission_Capsule = 0,            // 0
    Mission_Kicker,                     // 1
    Mission_LM,                             // 2
    Mission_Probe_DM,                   // 3
    Mission_PrimaryBooster,     // 4
    Mission_EVA,                            // 5
    Mission_PhotoRecon,             // 6
    Mission_SecondaryBooster    // 7 - unused
};

struct MisAst {  // This struct will be -1's if empty
    struct Astros *A;
    char loc;
};

struct MisEval {
    char step;              /**< actual step id number */
    char loc;               /**< Mission Step Name Index */
    uint16_t StepInfo;      /**< ID of step success  1=succ   !1=fail */
    Equipment *E;           /**< Pointer into equipment */
    char Prest;             /**< Prestige Step #  (-1 for none) */
    char PComp;             /**< PComp will be set to amt of prest to be awarded. */
    char pad;               /**< pad location  (Index into First Part of MH[x][] */
    char Class;             /**< VAB Order index into Class types MH[][x] */
    char fgoto;             /**< goto on failure */
    char sgoto;             /**< goto on success */
    char dgoto;             /**< where to branch on deaths */
    char trace;             /**< allow tracing of mission (for prest) */
    char dice;              /**< Die roll to check safety against */
    int16_t rnum;           /**< Random number holder for failure type */
    char ast;               /**< Astro specialist num */
    char asf;               /**< Astro specialist safety addition */
    char FName[5];          /**< Name of Failure File (without .DAT) */
    char Name[10];          /**< Name of Anim Code */
};

struct XFails {
    int32_t per;               // Failure Key number
    int16_t code, val, xtra;   // Failure Codes
    int16_t fail;              // failure value ID
    char text[200];// Failure Text
};

struct AnimType {
    char ID[8];
    char OVL[4];
    char SD[2][4];         // Sound ID : Max 2
    int16_t w, h;
    uint8_t sPlay[2];// Frame to play the Sound
    uint8_t fNum;    // Number of frames
    uint8_t fLoop;   // Times to loop
    uint8_t cOff;    // Color offsets
    uint8_t cNum;    // Number of Colors
    // char cPal[cNum];    // Placemarker for RGB Colors
};

struct BlockHead {
    uint8_t cType;   // Type of Compression
    int32_t fSize;            // Size of data frame
    // char aFrame[fSize];
};

struct mStr {
    char Index;       /**< Mission Index Number */
    char Name[50];    /**< Name of Mission */
    char Abbr[24];    /**< Name of Mission Abbreviated */
    char Code[62];    /**< Mission Coding String */
    char Alt[36];     /**< Alternate Sequence */
    char AltD[36];    /**< Alternate Sequence for Deaths (Jt Missions Only) */
    char Days,        /**< Duration Level */
         Dur,              /**< Duration Mission */
         Doc,              /**< Docking Mission */
         EVA,              /**< EVA Mission */
         LM,               /**< LM Mission */
         Jt,               /**< Joint Mission */
         Lun,              /**< Lunar Mission */
         mEq,              /**< Minimum Equipment */
         mCrew;            /**< Useful for Morgans Code */
    uint8_t mVab[2];  /**< Hardware Requirements */
    char PCat[5],     /**< Prestige Category List */
         LMAd;             /**< LM Addition Points */
};

// Vehicle assembly working structure
// qty = quantity of this item
// ac = number spoken for with launches
// sf = safety factor
// dex = index into the player Equipment structs (Manned[], MISC[], Probe[], etc)
// img = the image to use for this, -1 for none
// wt = unit weight, used to deterine launch vehicle
// dmg = Equipment->Damage flag
struct VInfo {
    char name[15], qty, ac, sf, dex, img;
    int16_t wt;
    char dmg;
};
struct ManPool {
    char Name[14], Sex, Cap, LM, EVA, Docking, Endurance;
};
struct order {
    int16_t plr, loc, budget, date;
};

// Space port overlays
enum SpacePortOverlays {
    PORT_Monument = 0,        // 00 Washington's Monument
    PORT_Pentagon,            // 01 The Pentagon
    PORT_Capitol,             // 02 Capitol Building
    PORT_Cemetery,            // 03 Arlington Cemetery
    PORT_VAB,                 // 04 Vehicle Assembly
    PORT_Museum,              // 05 Museum of Space History
    PORT_Admin,               // 06 Administration
    PORT_AstroComplex,        // 07 Astronaut Complex
    PORT_MedicalCtr,          // 08 Medical Center
    PORT_BasicTraining,       // 09 Basic Training
    PORT_Helipad,             // 10 Helipad: Adv LM
    PORT_Pool,                // 11 Pool: Adv EVA
    PORT_Planetarium,         // 12 Planetarium: Adv Docking
    PORT_Centrifuge,          // 13 Centrifuge: Adv Endurance
    PORT_Airfield,            // 14 Airfield: Adv Capsule
    PORT_Satellite,           // 15 Satellite Programs
    PORT_LM,                  // 16 LM Program
    PORT_Jupiter,             // 17 Jupiter Program
    PORT_XMS,                 // 18 XMS Program
    PORT_Apollo,              // 19 Apollo Program
    PORT_Gemini,              // 20 Gemini Program
    PORT_Mercury,             // 21 Mercury Program
    PORT_Research,            // 22 Research & Development
    PORT_LaunchPad_A,         // 23 Launch Facility A
    PORT_LaunchPad_B,         // 24 Launch Facility B
    PORT_LaunchPad_C,         // 25 Launch Facility C
    PORT_MissionControl,      // 26 Mission Control
    PORT_ViewingStand,        // 27 Viewing Stand
    PORT_FlagPole,            // 28 Flag Pole: End Turn
    PORT_Gate,                // 29 Security Gate: Quit
    PORT_Moon,                // 30 Moon
    PORT_SovMonumentAlt,      // 31 Soviet Monument #2
    PORT_Zond,                // 32 Zond - unused
    PORT_Tracking,            // 33 Tracking Station
    PORT_SVHQ,                // 34 Strategic Visions H.Q.
};

enum PrestigeValues {
    Prestige_OrbitalSatellite = 0,
    Prestige_LunarFlyby = 1,
    Prestige_MercuryFlyby = 2,
    Prestige_VenusFlyby = 3,
    Prestige_MarsFlyby = 4,
    Prestige_JupiterFlyby = 5,
    Prestige_SaturnFlyby = 6,
    Prestige_LunarProbeLanding = 7,
    Prestige_Duration_F = 8,
    Prestige_Duration_E = 9,
    Prestige_Duration_D = 10,
    Prestige_Duration_C = 11,
    Prestige_Duration_B = 12,
    Prestige_Duration_A = -1,  // this is intentionally out of order
    Prestige_Duration_Calc = 14,
    Prestige_OnePerson = 13,
    Prestige_TwoPerson = 14,
    Prestige_ThreePerson = 15,
    Prestige_Minishuttle = 16,
    Prestige_FourPerson = 17,
    Prestige_MannedOrbital = 18,
    Prestige_MannedLunarPass = 19,
    Prestige_MannedLunarOrbit = 20,
    Prestige_MannedRescueAttempt = 21,
    Prestige_MannedLunarLanding = 22,
    Prestige_OrbitingLab = 23,
    Prestige_MannedDocking = 24,
    Prestige_WomanInSpace = 25,
    Prestige_Spacewalk = 26,
    Prestige_MannedSpaceMission = 27,
    Prestige_LunarMoonwalk = 40,
};

enum Milestones {
    Milestone_OrbitalSatellite = 0,
    Milestone_ManInSpace = 1,
    Milestone_EarthOrbit = 2,
    Milestone_LunarFlyby = 3,
    Milestone_LunarPlanetary = 4,
    Milestone_LunarPass = 5,
    Milestone_LunarOrbit = 6,
    Milestone_LunarLanding = 7,
};

// These mission enumerations are dependant on the loaded mission data file
// Only added what the AI player is actually using
enum MissionValues {
    Mission_None = 0,
    Mission_Orbital_Satellite = 1,
    Mission_SubOrbital = 2,
    Mission_U_SubOrbital = 3,
    Mission_Earth_Orbital = 4,
    Mission_Unmanned_Earth_Orbital = 5,
    Mission_Earth_Orbital_EVA = 6,
    Mission_LunarFlyby = 7,
    Mission_Lunar_Probe = 8,
    Mission_VenusFlyby = 9,
    Mission_MarsFlyby = 10,
    Mission_MercuryFlyby = 11,
    Mission_JupiterFlyby = 12,
    Mission_SaturnFlyby = 13,
    Mission_Orbital_Docking = 14,
    Mission_U_Orbital_D = 15,
    Mission_JT_ORBITAL_D = 16,

    //"ORBITAL-O",            //17
    Mission_Jt_Unmanned_Orbital_Docking = 18,
    //"JT ORBITALS-D",        //19

    Mission_Manned_Orbital_Docking_EVA = 20,
    //"JT ORBITAL-DE",        //21
    //"JT ORBITALS-DE",       //22
    //"JT ORBITALS-D2E",      //23
    //"ORBITAL-OE",           //24

    Mission_Orbital_Duration = 25,
    Mission_Orbital_EVA_Duration = 26,
    Mission_Orbital_Docking_Duration = 27,
    Mission_Orbital_DockingInOrbit_Duration = 28,
    //"ORBITAL-OEX",          //29

    //"JT ORBITALS-DEX",      //30
    //"JT ORBITAL-DX",        //31
    Mission_Jt_OrbitingLab = 32,
    //"ORBITAL-DEX",          //33
    //"JT ORBITAL-DEX",       //34

    //"JT ORBITALS-DEX",      //35

    Mission_Jt_OrbitingLab_EVA = 36,
    //"JT ORBITALS-DX",       //37
    //"EARTH ORBITAL-L",      //38
    //"JT EARTH ORBITAL-L",   //39

    //"EARTH ORBITAL-LEX",    //40
    //"JT EARTH ORBITAL-LEX", //41
    //"U. LUNAR PASS",        //42
    Mission_LunarPass = 43,
    //"JT LUNAR PASS:EOR",    //44

    //"U. LUNAR ORBITAL",     //45
    Mission_LunarOrbital = 46,
    //"JT LUNAR ORBITAL:EOR", //47
    Mission_Lunar_Orbital = 48,
    //"JT LUNAR ORB:LOR-L",   //49

    //"LUNAR ORBITAL-LE",     //50
    Mission_LunarOrbital_LM_Test = 50,
    //"JT LUN ORB:LOR EVA-L", //51
    //"JT LUN ORB:EOR/LOR-L", //52
    Mission_HistoricalLanding = 53,
    Mission_DirectAscent_LL = 54,

    Mission_Jt_LunarLanding_EOR = 55,
    Mission_Jt_LunarLanding_LOR = 56,

    Mission_Soyuz_LL = 57,
    //"RES IN EARTH ORBIT",   //58
    //"RES IN LUNAR ORBIT",   //59

    //"RES HISTORICAL L.L.",  //60
    //"RES DRCT ASCENT L.L.", //61
};

// Typical
typedef struct _patch {
    int16_t w;
    int16_t h;
    uint16_t size;
    uint32_t offset;
} PatchHdr;

// Typical, or maybe not?
typedef struct {
    int8_t w;
    int8_t h;
    uint16_t size;
    uint32_t offset;
} PatchHdrSmall;


#define MAX_REPLAY_ITEMS        200L

// Mission Replay Data Structure
typedef struct ReplayItem {
    uint8_t Qty;         // Number of Animated Sequences
    uint16_t Off[35];     // Offsets to Each animated Part
} REPLAY;

enum Opponent_Status {Ahead, Equal, Behind};

typedef struct {
    uint32_t offset;  // Offset to the memory location of the displayed news string
    uint16_t size;      // Size of the old news string
} OLDNEWS;

// Save Game related typedefs
//#define RaceIntoSpace_Signature   'RiSP'
#define RaceIntoSpace_Signature 0x52695350
#define RaceIntoSpace_Old_Sig 0x49443a00  //'ID:\0"

typedef enum {
    SAVEGAME_Normal = 0,
    SAVEGAME_PlayByMail,
    SAVEGAME_Modem
} SaveGameType;

typedef struct {
    uint32_t ID;        // Going to use this to determine endianness of the save file
    char Name[23], PName[2][20], Country[2], Season, Year;
    uint16_t dataSize;      // Size of Players struct
    uint16_t compSize;      // Compressed size of data
} SaveFileHdr;


typedef struct {
    char Name[15], Title[23];
    uint16_t time, date;
} SFInfo;



/**
 * Data structure to hold transient save game data
 */
typedef struct {
    // REPLAY.DAT related variables
    uint32_t replaySize;
    REPLAY tempReplay[MAX_REPLAY_ITEMS];
    // EVENT.TMP related variables
    //   Format: array of 84 OLDNEWS structures followed by all the text of the displayed
    //   event strings.  This text is listed in card order and offset via the OLDNEWS structure
    uint32_t eventSize;
    OLDNEWS *tempEvents;   // helper pointer, always set to eventBuffer.  EVENT.TMP
    char *eventBuffer;   // raw buffer for event data
    // ENDTURN.TMP related variables
    uint32_t endTurnSaveSize;
    char *endTurnBuffer;
} INTERIMDATA;

#pragma pack(pop)

// Double-check that the structs we write to disk have the proper size
// Errors in these lines mean that the struct is the wrong shape
#include <boost/static_assert.hpp>
BOOST_STATIC_ASSERT(sizeof(PrestType) == 15);
BOOST_STATIC_ASSERT(sizeof(Equipment) == 58);
BOOST_STATIC_ASSERT(sizeof(MissionType) == 43);
BOOST_STATIC_ASSERT(sizeof(Astros) == 63);
BOOST_STATIC_ASSERT(sizeof(PastInfo) == 84);
BOOST_STATIC_ASSERT(sizeof(BuzzData) == 15520);
BOOST_STATIC_ASSERT(sizeof(Players) == 31663);

#endif // __DATA_H__

/* vim: set noet ts=4 sw=4 tw=77: */
