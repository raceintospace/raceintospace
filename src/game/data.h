#ifndef RIS_DATA_H
#define RIS_DATA_H

#include <stdint.h>
#include <array>
#include <map>

#include "serialize.h"
#include "fs.h"

//using namespace std;

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
 * \li   3 = Death during mission
 * \li   4 = Injury during mission
 * \li   5 = Injury resolution
 * \li   6 = Removed from FLT crew
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
#define MAXIMUM_PRESTIGE_NUM        28
#define MAX_STEPS                   60
#define MAX_MISSION_COUNT           100
#define MAX_REPLAY_ITEMS            200
#define MAX_NEWS_ITEMS              84

#define MAX_PORT_LEVEL              40
#define MAX_PORT_REGION             4

#define NUM_PROGRAMS                5 /**< 1,2,3,4 seater + shuttle */
#define MAX_PCAT                    5

/** \name Astronaut related
 *@{
 */
#define ASTRONAUT_POOLS             5
#define ASTRONAUT_CREW_MAX          8
#define ASTRONAUT_FLT_CREW_MAX      4
#define MAX_POOL                    65
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
    int8_t Plr1, Plr2;  /**< 0=USA,1=USSR,2=AI+USA,3=AI+USSR,6=Remote+USA,7=Remote+USSR */
    int8_t Lev1, Lev2;  /**< 0=Easy, 1=Medium, 2=Hard */
    int8_t Ast1, Ast2;  /**< 0=Easy, 1=Medium, 2=Hard */
    int8_t Input;       /**< 0=Mouse, 1=Keyboard, 2=Joystick */
    int8_t Anim;        /**< 0=Full, 1=Partial, 2=Results Only */
    int8_t Music;       /**< 0=Full, 1=Partial, 2=None */
    int8_t Sound;       /**< 0=On, 1=Off */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(Plr1));
        ar(CEREAL_NVP(Plr2));
        ar(CEREAL_NVP(Lev1));
        ar(CEREAL_NVP(Lev2));
        ar(CEREAL_NVP(Ast1));
        ar(CEREAL_NVP(Ast2));
        ar(CEREAL_NVP(Input));
        ar(CEREAL_NVP(Anim));
        ar(CEREAL_NVP(Music));
        ar(CEREAL_NVP(Sound));

        // SECURITY: Data sanitization
        ASSERT(Plr1 >= 0 && Plr1 <= 8);
        ASSERT(Plr2 >= 0 && Plr2 <= 9);
    }
};

/** The Prestige for any achievement is stored in this.
*/
struct PrestType {
    int8_t Add[4];        /**< 4&5 used for qty completed */
    int8_t Goal[NUM_PLAYERS];       /**< Goal steps used for prest calculations */
    int16_t Points[NUM_PLAYERS];  /**< Total points earned */
    int8_t Index;         /**< the index for the first into .History[] */
    int8_t Place;         /**< Who got the prestige first. */
    int8_t mPlace;        /**< If you've done it at all */
    int8_t Month;         /**< Date of the First */
    int8_t Year;          /**< Date of the First */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        /*        std::vector<int8_t> vAdd(Add, Add+4);
        ar(cereal::make_nvp("Add", vAdd));
        memcpy(Add, vAdd.data(), 4);

        std::vector<int8_t> vGoal(Goal, Goal+2);
        ar(cereal::make_nvp("Goal", vGoal));
        memcpy(Goal, vGoal.data(), 2);*/

        ARCHIVE_VECTOR(Add, int8_t, 4);
        ARCHIVE_VECTOR(Goal, int8_t, 2);
        ARCHIVE_VECTOR(Points, int16_t, 2);

        ar(CEREAL_NVP(Index));
        ar(CEREAL_NVP(Place));
        ar(CEREAL_NVP(mPlace));
        ar(CEREAL_NVP(Month));
        ar(CEREAL_NVP(Year));

        // SECURITY: Data sanitization
        ASSERT(Month >= 0 && Month < 12);
    }

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


struct Equipment {
    char Name[20];      /**< Name of Hardware */
    char ID[2];         /**< EquipID "C0 C1 C2 C3 : Acts as Index */
    int16_t Safety;     /**< current safety factor */
    int16_t MisSaf;     /**< Safety During Mission */
    int16_t MSF;        /**< used only to hold safety for docking kludge / Base Max R&D for others (from 1.0.0)*/
    int8_t Base;          /**< initial safety factor */
    int16_t InitCost;   /**< Startup Cost of Unit */
    int8_t UnitCost;      /**< Individual Cost */
    int16_t UnitWeight; /**< Weight of the Item */
    int16_t MaxPay;     /**< Maximum payload */
    int8_t RDCost;        /**< Cost of R&D per roll */
    int8_t Code;          /**< Equipment Code for qty scheduled */
    int8_t Num;           /**< quantity in inventory */
    int8_t Spok;          /**< qty being used on missions */
    int8_t Seas;          /**< Seasons Program is Active */
    int8_t Used;          /**< total number used in space */
    char unused_IDX[2];        /**< unsed - EquipID "C0 C1 C2 C3 : Acts as Index */
    int16_t Steps;      /**< Program Steps Used */
    int16_t Failures;   /**< number of program failures */
    int8_t MaxRD;         /**< maximum R & D */
    int8_t MaxSafety;     /**< maximum safety factor */
    int8_t SMods;         /**< safety factor mods for next launch */
    int8_t SaveCard;      /**< counter next failure in this prog */
    int8_t Delay;         /**< delay in purchase - in seasons */
    int8_t Duration;      /**< Days it can last in space */
    int8_t Damage;        /**< Damage percent for next launch */
    int8_t DCost;         /**< Cost to repair damage */
    int8_t MisSucc[2];       /**< Mission Successes */
    int8_t MisFail[2];       /**< Mission Failures */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        std::string sName = Name;
        ar(cereal::make_nvp("Name", sName));
        strntcpy(Name, sName.c_str(), sizeof(Name));

        ar(CEREAL_NVP(ID));
        ar(CEREAL_NVP(Safety));
        ar(CEREAL_NVP(MisSaf));
        ar(CEREAL_NVP(MSF));
        ar(CEREAL_NVP(Base));
        ar(CEREAL_NVP(InitCost));
        ar(CEREAL_NVP(UnitCost));
        ar(CEREAL_NVP(UnitWeight));
        ar(CEREAL_NVP(MaxPay));
        ar(CEREAL_NVP(RDCost));
        ar(CEREAL_NVP(Code));
        ar(CEREAL_NVP(Num));
        ar(CEREAL_NVP(Spok));
        ar(CEREAL_NVP(Seas));
        ar(CEREAL_NVP(Used));
        ar(CEREAL_NVP(unused_IDX));
        ar(CEREAL_NVP(Steps));
        ar(CEREAL_NVP(Failures));
        ar(CEREAL_NVP(MaxRD));
        ar(CEREAL_NVP(MaxSafety));
        ar(CEREAL_NVP(SMods));
        ar(CEREAL_NVP(SaveCard));
        ar(CEREAL_NVP(Delay));
        ar(CEREAL_NVP(Duration));
        ar(CEREAL_NVP(Damage));
        ar(CEREAL_NVP(DCost));
    }

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

struct MissionType {
    char Name[25];      /**< name of mission */
    int8_t MissionCode;   /**< internal code of mission type */
    int8_t Patch;         /**< Patch Used on this Mission */
    int8_t part;          /**< primary/secondary portion */
    int8_t Hard[6];       /**< Prime,Kick,LEM,PayLoad,Rocket */
    int8_t Joint;         /**< Joint Mission flag */
    int8_t Rushing;       /**< rushing time 0-3 */
    int8_t Month;         /**< Month of Launch */
    int8_t Duration;      /**< Duration time */
    int8_t Men;           /**< Qty of men used in mission */
    int8_t Prog;          /**< Hardware program used */
    int8_t PCrew;         /**< Primary Crew */
    int8_t BCrew;         /**< Backup Crew */
    int8_t Crew;          /**< Actual Mission Crew */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ARCHIVE_STRING(Name);

        ar(CEREAL_NVP(MissionCode));
        ar(CEREAL_NVP(Patch));
        ar(CEREAL_NVP(part));
        ar(CEREAL_NVP(Hard));
        ar(CEREAL_NVP(Joint));
        ar(CEREAL_NVP(Rushing));
        ar(CEREAL_NVP(Month));
        ar(CEREAL_NVP(Duration));
        ar(CEREAL_NVP(Men));
        ar(CEREAL_NVP(Prog));
        ar(CEREAL_NVP(PCrew));
        ar(CEREAL_NVP(BCrew));
        ar(CEREAL_NVP(Crew));

        // SECURITY: Data sanitization
        ASSERT(Month >= 0 && Month < 12);
        ASSERT(Joint == 0 || Joint == 1);
        ASSERT(part == 0 || part == 1);
        ASSERT(Prog >= 0 && Prog < NUM_PROGRAMS + 1);
        ASSERT(PCrew >= 0 && PCrew <= ASTRONAUT_CREW_MAX);
        ASSERT(BCrew >= 0 && BCrew <= ASTRONAUT_CREW_MAX);
        ASSERT(Crew >= 0 && Crew <= ASTRONAUT_CREW_MAX);
        ASSERT(Men >= 0 && Men <= ASTRONAUT_FLT_CREW_MAX);

        for (int i = 0; i < Mission_PrimaryBooster; i++) {
            ASSERT(Hard[i] >= -1 && Hard[i] < 7);
        }
        // Saturn V / N1 with boosters has a hardware index of 7
        ASSERT(Hard[Mission_PrimaryBooster] >= -1
               && Hard[Mission_PrimaryBooster] < 8);
        ASSERT(Hard[Mission_EVA] == -1
               || Hard[Mission_EVA] == 0
               || Hard[Mission_EVA] == MISC_HW_EVA_SUITS);
    }
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
    int8_t Face;         /**< Code for Astronaut Face */
    int8_t Sex;          /**< Male or Female */
    int8_t Compat;       /**< Compatibility Code */
    int8_t CR;           /**< Range of Compatibility Right */
    int8_t CL;           /**< Range of Compatibility Left */
    int8_t Moved;        /**< if 0 Astro Moved around this turn */
    int8_t currentMissionStatus;          /**< Successful Mission this Turn */
    int8_t Happy;        /**< \brief Happy within group */
    /**< \li 1 = Successful Mission */
    /**< \li 2 = Personal First */
    int8_t Prime;        /**< \brief indicates the type of crew s/he's in */
    /**< \li 2 = Primary Crew Member */
    /**< \li 1 = Backup Crew Member */
    /**< \li 0 = Not selected */
    int8_t Active;       /**< Seasons Active */
    int8_t AISpecial;    /**< AI Special Astro Note */
    int8_t Service;      /**< Service of Origination */
    int8_t Status;       /**< Current status of Astronaut */
    int8_t Focus;        /**< Focus of Advanced Training 0-4 */
    int8_t InjuryDelay;       /**< Injury Delay - Ok if 0 */
    int8_t RetirementDelay;       /**< Retire Delay - Retire if 0 */
    int8_t RetirementReason;      /**< Retire Reason flag */
    int8_t Special;      /**< Special for announcements */
    int8_t Assign;       /**< Location of Astro 0=limbo */
    int8_t oldAssign;    /**< program of last turn */
    int8_t Unassigned;   /**< Unassigned signed flag */
    int8_t Crew;         /**< Crew Associated in prog Assigm */
    int8_t Task;         /**< What his job is in Flt Crew */
    int16_t Prestige;  /**< Prestige Earned */
    int8_t Days;         /**< Days in Space */
    int8_t Medals;       /**< Medals Earned */
    int8_t Save;         /**< Astro Saving Roll Percent */
    int8_t Missions;     /**< Number of Missions Flown */
    int8_t MissionNum[10];       /**< Mission numbers participated */
    int8_t Cap;          /**< Capsule Pilot Skills */
    int8_t LM;           /**< LEM Pilot Skills */
    int8_t EVA;          /**< EVA Skills */
    int8_t Docking;      /**< Docking Skills */
    int8_t Endurance;    /**< Constitution Skills */
    int8_t TrainingLevel;/**< Level of Schooling */
    int8_t Group;        /**< Group Entered with */
    int8_t Mood;         /**< Mood of Astronaut */
    int8_t Pool;         /**< Temp Storage for Astros */
    int8_t Hero;         /**< Hero Status */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ARCHIVE_STRING(Name);

        ar(CEREAL_NVP(Face));
        ar(CEREAL_NVP(Sex));
        ar(CEREAL_NVP(Compat));
        ar(CEREAL_NVP(CR));
        ar(CEREAL_NVP(CL));
        ar(CEREAL_NVP(Moved));
        ar(CEREAL_NVP(currentMissionStatus));
        ar(CEREAL_NVP(Happy));
        ar(CEREAL_NVP(Prime));
        ar(CEREAL_NVP(Active));
        ar(CEREAL_NVP(AISpecial));
        ar(CEREAL_NVP(Service));
        ar(CEREAL_NVP(Status));
        ar(CEREAL_NVP(Focus));
        ar(CEREAL_NVP(InjuryDelay));
        ar(CEREAL_NVP(RetirementDelay));
        ar(CEREAL_NVP(RetirementReason));
        ar(CEREAL_NVP(Special));
        ar(CEREAL_NVP(Assign));
        ar(CEREAL_NVP(oldAssign));
        ar(CEREAL_NVP(Unassigned));
        ar(CEREAL_NVP(Crew));
        ar(CEREAL_NVP(Task));
        ar(CEREAL_NVP(Prestige));
        ar(CEREAL_NVP(Days));
        ar(CEREAL_NVP(Medals));
        ar(CEREAL_NVP(Save));
        ar(CEREAL_NVP(Missions));
        ar(CEREAL_NVP(MissionNum));
        ar(CEREAL_NVP(Cap));
        ar(CEREAL_NVP(LM));
        ar(CEREAL_NVP(EVA));
        ar(CEREAL_NVP(Docking));
        ar(CEREAL_NVP(Endurance));
        ar(CEREAL_NVP(TrainingLevel));
        ar(CEREAL_NVP(Group));
        ar(CEREAL_NVP(Mood));
        ar(CEREAL_NVP(Pool));
        ar(CEREAL_NVP(Hero));

        // SECURITY: Data sanitization
        ASSERT(Assign >= 0 && Assign <= NUM_PROGRAMS + 1);
        ASSERT(Crew >= 0 && Crew <= ASTRONAUT_CREW_MAX);

    }
};

/**
 * \brief For Records of Missions
*/
struct PastInfo {
    char MissionName[2][25];       /**< Name of Mission */
    int8_t Patch[2];                 /**< Patch Type */
    int8_t MissionCode;              /**< Type of Mission */
    int8_t MissionYear;              /**< Year Mission Took Place */
    int8_t Month;                    /**< Month of Launch */
    int8_t Time;                     /**< Early / Late Flag */
    int8_t Man[2][4];                /**< Astros # involved in Mis */
    int8_t Hard[2][5];               /**< Set of hardware */
    uint16_t result;               /**< Success / Failure */
    uint16_t spResult;             /**< Actual deciding Result */
    int8_t Event;                    /**< Single event code */
    int8_t Saf;                      /**< Safety of the Equipment for delays */
    int8_t Part;                     /**< Participant in event code */
    int16_t Prestige;              /**< Prestige Earned on this Mission */
    int8_t Duration;                 /**< Length of Mission (units:ABCDEF) */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        std::string sMissionName[2];
        sMissionName[0] = MissionName[0];
        sMissionName[1] = MissionName[1];
        ar(cereal::make_nvp("MissionName", sMissionName));
        strntcpy(MissionName[0], sMissionName[0].c_str(), sizeof(MissionName[0]));
        strntcpy(MissionName[1], sMissionName[1].c_str(), sizeof(MissionName[1]));

        ar(CEREAL_NVP(Patch));
        ar(CEREAL_NVP(MissionCode));
        ar(CEREAL_NVP(MissionYear));
        ar(CEREAL_NVP(Month));
        ar(CEREAL_NVP(Time));
        ar(CEREAL_NVP(Man));
        ar(CEREAL_NVP(Hard));
        ar(CEREAL_NVP(result));
        ar(CEREAL_NVP(spResult));
        ar(CEREAL_NVP(Event));
        ar(CEREAL_NVP(Saf));
        ar(CEREAL_NVP(Part));
        ar(CEREAL_NVP(Prestige));
        ar(CEREAL_NVP(Duration));

        // SECURITY: Data sanitization
        ASSERT(Month >= 0 && Month < 12);
    }
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
    int8_t RD_Mods_For_Turn;                     // R & D Mods for turn only
    int8_t RD_Mods_For_Year;                     // R & D Mods for full year
    int8_t TurnOnly;                   // Events for this turn only
    int8_t Plans;                      // Code for future plans
    // 1 = Manned Launch, -10 MB
    int8_t FuturePlans;                // Special future mission
    int8_t DurationLevel;                      // Current Duration Level
    int8_t LMpts;                      // Accumulation of any LM Test points
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
    int8_t DockingModuleInOrbit;                       // Docking Module in Orbit, in seasons
    int8_t LaunchFacility[MAX_LAUNCHPADS];    /**< -1=no facility; 1=purchased, >1 repair cost) */
    int8_t AstroCount;                 // Current # in Program
    int8_t AstroLevel;                 // Level of selection
    int8_t AstroDelay;                 // Wait until next selection
    struct Astros Pool[MAX_POOL];      // Pool of SpaceMen
    struct {
        char Vle;
        char Asp;
    } unused_WList[5]; // unused
    struct Prest_Upd Udp[3];
    char unused_WTop; // unused
    char unused_VList[5]; // unused
    char unused_VTop; // unused
    int8_t IntelHardwareTable[5][7];                // safety factor for intelligence
    int8_t CrewCount[ASTRONAUT_POOLS + 1][ASTRONAUT_CREW_MAX];               // Count of num in groups
    int8_t Crew[ASTRONAUT_POOLS + 1][ASTRONAUT_CREW_MAX][ASTRONAUT_FLT_CREW_MAX + 1];          // Flt Crews
    // [5] - Program #
    // [8] - Eight Crews Max
    // [4] - Four Max per Crew
    int8_t FemaleAstronautsAllowed;                     // FemaleAstronautsAlloweds allowed in program
    struct MissionType Mission[MAX_MISSIONS];   // Current Mission Info
    int8_t Block;                      // Blocked Mission Number
    struct MissionType Future[MAX_MISSIONS];    // Future Mission Info
    struct PastInfo History[MAX_MISSION_COUNT];    // Past Mission History
    int16_t PastMissionCount;                   // Number of Past Missions
    int8_t MissionCatastrophicFailureOnTurn;                      // Catastrophic Fail on Turn
    // 1 = Cat Failure
    // 2 = Program Death
    // 3 = Program First
    uint8_t Buy[4][7];  // Record of R&D for Turn - encoded dices & rolls
    int8_t eCount;
    struct {
        int8_t code;
        int16_t num;
        int8_t prog;
        int8_t cur;
        int8_t index;
        int8_t SafetyFactor;
        int8_t cdex;

        template<class Archive>
        void serialize(Archive &ar, uint32_t const version)
        {
            ar(CEREAL_NVP(code));
            ar(CEREAL_NVP(num));
            ar(CEREAL_NVP(prog));
            ar(CEREAL_NVP(cur));
            ar(CEREAL_NVP(index));
            ar(CEREAL_NVP(SafetyFactor));
            ar(CEREAL_NVP(cdex));
        }
    } PastIntel[30];
    char unused_AIpath[23]; // unused
    int8_t AIStrategy[10];                  // AI Strategy Values
    int8_t AILunar,                    // Flag to tell way to moon
           AIPrim,                     // Currect Primary Prog
           AISec,                      // Currect Secondary Prog
           unused_AIEva, // unused
           unused_AIDur, // unused
           unused_AIDck, // unused
           AIStat,                     // the status of program
           unused_AITech, // unused
           unused_AILem; // unused
    int8_t unused_X[20]; // unused
    int8_t unused_AIMissions[62];                      // How many missions of type done
    int8_t BadCardEventFlag[15];                // Flag for Bad Events
    int8_t Port[MAX_PORT_LEVEL];                   // Levels for SPort display

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ARCHIVE_STRING(Name);

        ar(CEREAL_NVP(Cash));
        ar(CEREAL_NVP(Budget));
        ar(CEREAL_NVP(Prestige));
        ar(CEREAL_NVP(PrestHist));
        ar(CEREAL_NVP(PresRev));
        ar(CEREAL_NVP(tempPrestige));
        ar(CEREAL_NVP(BudgetHistory));
        ar(CEREAL_NVP(BudgetHistoryF));
        ar(CEREAL_NVP(Spend));
        ar(CEREAL_NVP(RD_Mods_For_Turn));
        ar(CEREAL_NVP(RD_Mods_For_Year));
        ar(CEREAL_NVP(TurnOnly));
        ar(CEREAL_NVP(Plans));
        ar(CEREAL_NVP(FuturePlans));
        ar(CEREAL_NVP(DurationLevel));
        ar(CEREAL_NVP(LMpts));
        ar(CEREAL_NVP(Probe));
        ar(CEREAL_NVP(Rocket));
        ar(CEREAL_NVP(Manned));
        ar(CEREAL_NVP(Misc));
        ar(CEREAL_NVP(DockingModuleInOrbit));
        ar(CEREAL_NVP(LaunchFacility));
        ar(CEREAL_NVP(AstroCount));
        ar(CEREAL_NVP(AstroLevel));
        ar(CEREAL_NVP(AstroDelay));
        ar(CEREAL_NVP(Pool));
        ar(CEREAL_NVP(IntelHardwareTable));
        ar(CEREAL_NVP(CrewCount));
        ar(CEREAL_NVP(Crew));
        ar(CEREAL_NVP(FemaleAstronautsAllowed));
        ar(CEREAL_NVP(Mission));
        ar(CEREAL_NVP(Future));
        ar(CEREAL_NVP(History));
        ar(CEREAL_NVP(PastMissionCount));
        ar(CEREAL_NVP(MissionCatastrophicFailureOnTurn));
        ar(CEREAL_NVP(Buy));
        ar(CEREAL_NVP(eCount));
        ar(CEREAL_NVP(PastIntel));
        ar(CEREAL_NVP(AIStrategy));
        ar(CEREAL_NVP(AILunar));
        ar(CEREAL_NVP(AIPrim));
        ar(CEREAL_NVP(AISec));
        ar(CEREAL_NVP(AIStat));
        ar(CEREAL_NVP(BadCardEventFlag));
        ar(CEREAL_NVP(Port));

        // SECURITY: Data sanitization
        ASSERT(PastMissionCount >= 0 && PastMissionCount < MAX_MISSION_COUNT);
        ASSERT(eCount >= 0 && eCount < MAX_NEWS_ITEMS / 2);
        ASSERT(AstroCount >= 0 && AstroCount <= MAX_POOL);

        for (int i = 0; i < MAX_PORT_LEVEL; i++) {
            ASSERT(Port[i] >= 0 && Port[i] < MAX_PORT_REGION);
        }

        for (int i = 0; i <= ASTRONAUT_POOLS; i++) {
            for (int j = 0; j < ASTRONAUT_CREW_MAX; j++) {
                for (int k = 0; k <= ASTRONAUT_FLT_CREW_MAX; k++) {
                    ASSERT(Crew[i][j][k] >= 0 && Crew[i][j][k] <= MAX_POOL);
                }

                ASSERT(CrewCount[i][j] >= 0 && CrewCount[i][j] <= ASTRONAUT_FLT_CREW_MAX);
            }
        }

    }

};

struct MisEval {
    int8_t step;              /**< actual step id number */
    int8_t loc;               /**< Mission Step Name Index */
    uint16_t StepInfo;      /**< ID of step success  1=succ   !1=fail */
    union {
        Equipment *Ep;      /**< Pointer into equipment, use GetEquipment() */
        uint64_t Ebits;     /**< keep 64b also on 32b platforms.  */
    };
    int8_t Prest;             /**< Prestige Step #  (-1 for none) */
    int8_t PComp;             /**< PComp will be set to amt of prest to be awarded. */
    int8_t pad;               /**< pad location  (Index into First Part of MH[x][] */
    int8_t Class;             /**< VAB Order index into Class types MH[][x] */
    int8_t fgoto;             /**< goto on failure */
    int8_t sgoto;             /**< goto on success */
    int8_t dgoto;             /**< where to branch on deaths */
    int8_t trace;             /**< allow tracing of mission (for prest) */
    int8_t dice;              /**< Die roll to check safety against */
    int16_t rnum;           /**< Random number holder for failure type */
    int8_t ast;               /**< Astro specialist num */
    int8_t asf;               /**< Astro specialist safety addition */
    char FName[5];          /**< Name of Failure File (without .DAT) */
    char Name[10];          /**< Name of Anim Code */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(step));
        ar(CEREAL_NVP(loc));
        ar(CEREAL_NVP(StepInfo));
        ar(CEREAL_NVP(Prest));
        ar(CEREAL_NVP(PComp));
        ar(CEREAL_NVP(pad));
        ar(CEREAL_NVP(Class));
        ar(CEREAL_NVP(fgoto));
        ar(CEREAL_NVP(sgoto));
        ar(CEREAL_NVP(dgoto));
        ar(CEREAL_NVP(trace));
        ar(CEREAL_NVP(dice));
        ar(CEREAL_NVP(rnum));
        ar(CEREAL_NVP(ast));
        ar(CEREAL_NVP(asf));

        ARCHIVE_STRING(FName);
        ARCHIVE_STRING(Name);

        // SECURITY: Data sanitization
        ASSERT(abs(Prest) < MAXIMUM_PRESTIGE_NUM || abs(Prest) == 100);
        ASSERT(trace < MAX_STEPS || trace == 0x7f);

    }

};


struct Players {
    char BUZZ[4];                    /**< Save Version Marker */
    uint32_t Checksum;               /**< Checksum of Data */
    int8_t plr[NUM_PLAYERS];        /**< Order of Turns */
    struct Defl Def;                 /**< Defaults */
    int8_t Year;                    /**< Game Turn */
    int8_t Season;                  /**< Season of Year */
    struct PrestType Prestige[MAXIMUM_PRESTIGE_NUM];   /**< Definitions of Prest Vals */
    struct BuzzData P[NUM_PLAYERS];  /**< Player Game Data */
    int8_t unused_EMark[3]           ; /**< unused - Event Marker */
    int8_t Events[MAXIMUM_NEWS_EVENTS]; /**< History of Event Cards */
    int8_t Count;                      /**< Number of Events Picked */
    int8_t PD[NUM_PLAYERS][MAXIMUM_PRESTIGE_NUM]; /**< Prestige First Displayed: First Bit: Seen in MisRev, Second Bit: Seen by Opponent */
    int8_t Mile[NUM_PLAYERS][10];      /**< MileStone Calcs */
    int8_t Mail;                       /* Current status of PBEM game */
    struct MisEval Mev[MAX_LAUNCHPADS][60]; /** < Mission eval for mail games */
    int8_t Step[MAX_LAUNCHPADS]; /** Number of mission steps for mail games */

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(BUZZ));
        ar(CEREAL_NVP(Checksum));
        ar(CEREAL_NVP(plr));
        ar(CEREAL_NVP(Def));
        ar(CEREAL_NVP(Year));
        ar(CEREAL_NVP(Season));
        ar(CEREAL_NVP(Prestige));
        ar(CEREAL_NVP(P));
        ar(CEREAL_NVP(unused_EMark));
        ar(CEREAL_NVP(Events));
        ar(CEREAL_NVP(Count));
        ar(CEREAL_NVP(PD));
        ar(CEREAL_NVP(Mile));
        if (version > 0) {
            ar(CEREAL_NVP(Mail));
        }
        ar(CEREAL_NVP(Mev));
        ar(CEREAL_NVP(Step));

        // SECURITY: Data sanitization
        ASSERT(Count >= 0 && Count < MAX_NEWS_ITEMS);
        ASSERT(Year >= 57 && Year <= 78);
        ASSERT(Season == 0 || Season == 1);
    }
};

CEREAL_CLASS_VERSION(struct Players, 1)

struct MisAst {  // This struct will be -1's if empty
    struct Astros *A;
    char loc;
};

struct XFails {
    std::string MissionStep;
    int32_t percentage;         // Failure Key number
    int16_t code, val, xtra;   // Failure Codes
    int16_t fail;              // failure value ID
    char text[200];// Failure Text

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(MissionStep));
        ar(CEREAL_NVP(percentage));
        ar(CEREAL_NVP(code));
        ar(CEREAL_NVP(val));
        ar(CEREAL_NVP(xtra));
        ar(CEREAL_NVP(fail));
        
        ARCHIVE_STRING(text);
    }
      
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
    char PCat[MAX_PCAT],   /**< Prestige Category List */
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
// MaxRD = Mar R&D
struct VInfo {
    char name[15], qty, ac, sf, dex, img;
    int16_t wt;
    char dmg;
    int MaxRD;
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
    //"LUNAR ORBITAL-L,"      //48
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
struct PatchHdr {
    int16_t w;
    int16_t h;
    uint16_t size;
    uint32_t offset;
};

// Typical, or maybe not?
struct PatchHdrSmall {
    int8_t w;
    int8_t h;
    uint16_t size;
    uint32_t offset;
};

// Mission Sequence data read from seq.json and fseq.json
struct MissionSequenceKey {
    std::string MissionStep;
    // MissionIdSequence format:
    // [0]: number for sequence variation (must be stored in decreasing order)
    // [1]: Maximum number of video files
    // [2]: female crew member?
    // [3-n]: Hardware-specific name (taken from Mev[].Name)
    // [n+1-n+2]: Only for failures: failure code from XFails.fail
    std::string MissionIdSequence;
    std::vector<std::string> video;
    std::vector<std::string> audio;

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        try {
            ar(CEREAL_NVP(MissionStep));
        } catch (...) { // No MissionStep in seq.json
        }
            
        ar(CEREAL_NVP(MissionIdSequence));
        ar(CEREAL_NVP(video));
        ar(CEREAL_NVP(audio));
    }
};



// Mission Replay Data Structure
typedef struct LegacyReplayItem {
    uint8_t Qty;         // Number of Animated Sequences
    uint16_t Off[35];     // Offsets to Each animated Part

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(Qty));
        ARCHIVE_VECTOR(Off, uint16_t, 35);

        // SECURITY: Data sanitization
        ASSERT(Qty <= 35);
    }

} LEGACY_REPLAY;

typedef struct ReplayItem {
    bool Failure;
    std::string seq;

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        if (version >= 2) {
            ar(CEREAL_NVP(Failure));
            ar(CEREAL_NVP(seq));
        }
    }
} REPLAY;

enum Opponent_Status {Ahead, Equal, Behind};

struct OLDNEWS {
    uint32_t offset;  // Offset to the memory location of the displayed news string
    uint16_t size;      // Size of the old news string

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(offset));
        ar(CEREAL_NVP(size));
    }

};

// Save Game related typedefs
//#define RaceIntoSpace_Signature   'RiSP'
#define RaceIntoSpace_Signature 0x52695350
#define RaceIntoSpace_Old_Sig 0x49443a00  //'ID:\0"


struct SaveFileHdr {
    uint32_t ID;        // Going to use this to determine endianness of the save file
    char Name[23], PName[2][20];
    int8_t Country[2], Season, Year;
    unsigned char dataSize[4]; // Uncompressed size in big endian format

    // Legacy format save games have instead of usize:
    // uint16_t dataSize;      // Size of Players struct
    // uint16_t compSize;      // Compressed size of data

};


/**
 * Data structure to hold transient save game data
 */
struct INTERIMDATA {
    // REPLAY.DAT related variables
    std::array<std::vector <REPLAY>, MAX_REPLAY_ITEMS> tempReplay;
    // EVENT.TMP related variables
    std::array<std::string, MAX_NEWS_ITEMS> tempEvents;
    std::string filename;

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        if (version == 1) {
            ARCHIVE_ARRAY(tempReplay, std::vector <REPLAY>);
            ARCHIVE_ARRAY(tempEvents, std::string);
        }
        else if (version == 0) {
            // Legacy replay format
            ARCHIVE_ARRAY(tempEvents, std::string);

            std::vector<LEGACY_REPLAY> legacyReplay;
            ar(cereal::make_nvp("tempReplay", legacyReplay));

            std::vector<std::string> seq;
            std::map<int, std::string> fseq;

            DESERIALIZE_JSON_FILE(&seq, locate_file("legacy-seq.json", FT_DATA));
            DESERIALIZE_JSON_FILE(&fseq, locate_file("legacy-fseq.json", FT_DATA));

            for (int i = 0; i < legacyReplay.size(); i++) {
                tempReplay.at(i).clear();
                assert(legacyReplay.at(i).Qty <= 35);
                for (int j = 0; j < legacyReplay.at(i).Qty; j++) {

                    int code = legacyReplay.at(i).Off[j];

                    if (code < 1000) {
                        tempReplay.at(i).push_back({false, seq.at(code)});
                    } else {
                        tempReplay.at(i).push_back({true, fseq.at(code)});
                    }
                        
                }
            }
            
        }
        else {
            throw(std::invalid_argument("Invalid INTERIMDATA version number"));
        }
    }
};

CEREAL_CLASS_VERSION(INTERIMDATA, 1);

struct AssetData {
    std::vector<struct MissionSequenceKey> sSeq; // Success sequences
    std::vector<struct MissionSequenceKey> fSeq; // Failure sequences
    std::vector <struct XFails> fails; // Failure texts and codes
};

#pragma pack(pop)

// Double-check that the structs we write to disk have the proper size
// Errors in these lines mean that the struct is the wrong shape
#include <boost/static_assert.hpp>
BOOST_STATIC_ASSERT(sizeof(PrestType) == 15);
BOOST_STATIC_ASSERT(sizeof(Equipment) == 60);
BOOST_STATIC_ASSERT(sizeof(MissionType) == 43);
BOOST_STATIC_ASSERT(sizeof(Astros) == 63);
BOOST_STATIC_ASSERT(sizeof(PastInfo) == 84);
BOOST_STATIC_ASSERT(sizeof(BuzzData) == 15576);
BOOST_STATIC_ASSERT(sizeof(MisEval) == 40);
BOOST_STATIC_ASSERT(sizeof(Players) == 38978);

#endif // RIS_DATA_H

/* vim: set noet ts=4 sw=4 tw=77: */
