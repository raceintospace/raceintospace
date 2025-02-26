#ifndef LEGACY_H
#define LEGACY_H

#include "data.h"
#include "serialize.h"

void LegacyLoad(SaveFileHdr header, FILE *fin, size_t fileLength);

#pragma pack(push, 1)

struct LegacyEquipment {
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
    int8_t MisSucc;       /**< Mission Successes */
    int8_t MisFail;       /**< Mission Failures */

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


struct LegacyAstros {
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

struct LegacyBuzzData {                   // master data list for Buzz Aldrin's
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
    LegacyEquipment Probe[7];       // 0 = Orbital
    // 1 = Inter Planetary
    // 2 = Lunar Probe
    LegacyEquipment Rocket[7];      // 0 = One Stage
    // 1 = Two Stage
    // 2 = Three Stage
    // 3 = Mega Stage
    // 4 = Strap On Boosters
    LegacyEquipment Manned[7];      // 0 = One Man Capsule
    // 1 = Two Man Capsule
    // 2 = Three Man Capsule
    // 3 = Minishuttle
    // 4 = Four Man Cap/Mod
    // 5 = Two Man Module
    // 6 = One Man Module
    LegacyEquipment Misc[7];        // 0 = Kicker Level A
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
    struct LegacyAstros Pool[MAX_POOL];      // Pool of SpaceMen
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

struct LegacyPlayers {
    char BUZZ[4];                    /**< Save Version Marker */
    uint32_t Checksum;               /**< Checksum of Data */
    int8_t plr[NUM_PLAYERS];        /**< Order of Turns */
    struct Defl Def;                 /**< Defaults */
    int8_t Year;                    /**< Game Turn */
    int8_t Season;                  /**< Season of Year */
    struct PrestType Prestige[MAXIMUM_PRESTIGE_NUM];   /**< Definitions of Prest Vals */
    struct LegacyBuzzData P[NUM_PLAYERS];  /**< Player Game Data */
    int8_t unused_EMark[4]           ; /**< unused - Event Marker */
    int8_t Events[MAXIMUM_NEWS_EVENTS]; /**< History of Event Cards */
    int8_t Count;                      /**< Number of Events Picked */
    int8_t PD[NUM_PLAYERS][MAXIMUM_PRESTIGE_NUM]; /**< Prestige First Displayed: First Bit: Seen in MisRev, Second Bit: Seen by Opponent */
    int8_t Mile[NUM_PLAYERS][10];      /**< MileStone Calcs */
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

        ar(CEREAL_NVP(Mev));
        ar(CEREAL_NVP(Step));

        // SECURITY: Data sanitization
        ASSERT(Count >= 0 && Count < MAX_NEWS_ITEMS);
        ASSERT(Year >= 57 && Year <= 78);
        ASSERT(Season == 0 || Season == 1);
    }
};

#pragma pack(pop)

#endif //LEGACY_H
