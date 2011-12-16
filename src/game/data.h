#ifndef __DATA_H__
#define __DATA_H__

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

#define NUM_PLAYERS				2 /**< number of players */
#define MAX_LAUNCHPADS			3 /**< number of launchpads available */
#define MAX_MISSIONS			3 /**< number of concurrent missions available */

/** \name Astronaut related
 *@{
 */
#define ASTRONAUT_POOLS				5
#define ASTRONAUT_CREW_MAX			8
#define ASTRONAUT_FLT_CREW_MAX		4

#define ASTRONAUT_MOOD_THRESHOLD	40
/*@}*/

#pragma pack (1)

struct Prest_Upd {
 char Poss[10];
 char PossVal[10];
 char HInd;
 char Qty;
 char Mnth;
};

struct Defl {char Plr1,Plr2,Lev1,Lev2,Ast1,Ast2,Input,Anim,Music,Sound;};

/** The Prestige for any achievement is stored in this.
*/
struct PrestType {
	char Add[4];		/**< 4&5 used for qty completed */
	char Goal[2];		/**< Goal steps used for prest calculations */
	int16_t Points[2];	/**< Total points earned */
	char Indec;			/**< the index for the first into .History[] */
						/**< \bug Indec is probably a typo. Should be Index. */
	char Place;			/**< Who got the prestige first. */
	char mPlace;		/**< If you've done it at all */
	char Month;			/**< Date of the First */
	char Year;			/**< Date of the First */
};

enum EquipmentIndex {
	EQUIP_PROBE = 0,	/**< unmanned satelite */
	EQUIP_ROCKET = 1,	/**< propulsion system */
	EQUIP_MANNED = 2,	/**< vehicle to transport humans */
	EQUIP_TECH = 3,		/**< piece of hardware to support other stuff */
};

typedef struct _Equipment {
	char Name[20];		/**< Name of Hardware */
	char ID[2];			/**< EquipID "C0 C1 C2 C3 : Acts as Index */
	int16_t Safety;		/**< current safety factor */
	int16_t MisSaf;		/**< Safety During Mission */
	int16_t MSF;		/**< used only to hold safety for docking kludge / Base Max R&D for others (from 1.0.0)*/
	char Base;			/**< initial safety factor */
	int16_t InitCost;	/**< Startup Cost of Unit */
	char UnitCost;		/**< Individual Cost */
	int16_t UnitWeight;	/**< Weight of the Item */
	int16_t MaxPay;		/**< Maximum payload */
	char RDCost;		/**< Cost of R&D per roll */
	char Code;			/**< Equipment Code for qty scheduled */
	char Num;			/**< quantity in inventory */
	char Spok;			/**< qty being used on missions */
	char Seas;			/**< Seasons Program is Active */
	char Used;			/**< total number used in space */
	char IDX[2];		/**< EquipID "C0 C1 C2 C3 : Acts as Index */
	int16_t Steps;		/**< Program Steps Used */
	int16_t Failures;	/**< number of program failures */
	char MaxRD;			/**< maximum R & D */
	char MaxSafety;		/**< maximum safety factor */
	char SMods;			/**< safety factor mods for next launch */
	char SaveCard;		/**< counter next failure in this prog */
	char Delay;			/**< delay in purchase - in seasons */
	char Duration;		/**< Days it can last in space */
	char Damage;		/**< Damage percent for next launch */
	char DCost;			/**< Cost to repair damage */
	char MisSucc;		/**< Mission Successes */
	char MisFail;		/**< Mission Failures */
} Equipment;

struct MissionType {
	char Name[25];		/**< name of mission */
	char MissionCode;	/**< internal code of mission type */
	char Patch;			/**< Patch Used on this Mission */
	char part;			/**< primary/secondary portion */
	char Hard[6];		/**< Prime,Kick,LEM,PayLoad,Rocket */
	char Joint;			/**< Joint Mission flag */
	char Rushing;		/**< rushing time 0-3 */
	char Month;			/**< Month of Launch */
	char Duration;		/**< Duration time */
	char Men;			/**< Qty of men used in mission */
	char Prog;			/**< Hardware program used */
	char PCrew;			/**< Primary Crew */
	char BCrew;			/**< Backup Crew */
	char Crew;			/**< Actual Mission Crew */
};

/**
 * \brief Number of astronauts to recruit in each level
*/
enum AstronautPoolClass
{
	ASTRO_POOL_LVL1	= 7,
	ASTRO_POOL_LVL2 = 9,
	ASTRO_POOL_LVL3 = 14,
	ASTRO_POOL_LVL4 = 16,
	ASTRO_POOL_LVL5 = 14
};

/* WARNING: these numbers are used throughout the code, do not change! */
enum AstronautStatus
{
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

/**
\brief This is a structure representing astronauts/cosmonauts.
*/
struct Astros
{
	char Name[14];	   /**< Astronaut Name */
	char Face;		   /**< Code for Astronaut Face */
	char Sex;		   /**< Male or Female */
	char Compat;	   /**< Compatability Code */
	char CR;		   /**< Range of Compatability Right */
	char CL;		   /**< Range of Compatability Left */
	char Moved;		   /**< if 0 Astro Moved around this turn */
	char Mis;		   /**< Successful Mission this Turn */
	char Happy;		   /**< \brief Happy within group */
					   /**< \li 1 = Successful Mission */
					   /**< \li 2 = Personal First */
	char Prime;		   /**< \brief indicates the type of crew s/he's in */
					   /**< \li 2 = Primary Crew Member */
					   /**< \li 1 = Backup Crew Member */
					   /**< \li 0 = Not selected */
	char Active;	   /**< Seasons Active */
	char AISpecial;	   /**< AI Special Astro Note */
	char Service;	   /**< Service of Origination */
	char Status;	   /**< Current status of Astronaut */
	char Focus;		   /**< Focus of Advanced Training 0-4 */
	char IDelay;	   /**< Injury Delay - Ok if 0 */
	char RDelay;	   /**< Retire Delay - Retire if 0 */
	char RetReas;	   /**< Retire Reason flag */
	char Special;	   /**< Special for announcements */
	char Assign;	   /**< Location of Astro 0=limbo */
	char oldAssign;	   /**< program of last turn */
	char Una;		   /**< Unassigned flag */
	char Crew;		   /**< Crew Assiciated in prog Assigm */
	char Task;		   /**< What his job is in Flt Crew */
	int16_t Prestige;  /**< Prestige Earned */
	char Days;		   /**< Days in Space */
	char Medals;	   /**< Medals Earned */
	char Save;		   /**< Astro Saving Roll Percent */
	char Missions;	   /**< Number of Missions Flown */
	char MissionNum[10];	   /**< Mission numbers participated */
	char Cap;		   /**< Capsule Pilot Skills */
	char LM;		   /**< LEM Pilot Skills */
	char EVA;		   /**< EVA Skills */
	char Docking;	   /**< Docking Skills */
	char Endurance;	   /**< Constitution Skills */
	char TrainingLevel;/**< Level of Schooling */
	char Group;		   /**< Group Entered with */
	char Mood;		   /**< Mood of Astronaut */
	char Pool;		   /**< Temp Storage for Astros */
	char Hero;		   /**< Hero Status */
};

/**
 * \brief For Records of Missions
*/
struct PastInfo
{
	char MissionName[2][25];	   /**< Name of Mission */
	char Patch[2];				   /**< Patch Type */
	char MissionCode;			   /**< Type of Mission */
	char MissionYear;			   /**< Year Mission Took Place */
	char Month;					   /**< Month of Launch */
	char Time;					   /**< Early / Late Flag */
	char Man[2][4];				   /**< Astros # involved in Mis */
	char Hard[2][5];			   /**< Set of hardware */
	uint16_t result;			   /**< Success / Failure */
	uint16_t spResult;			   /**< Actual deciding Result */
	char Event;					   /**< Single event code */
	char Saf;					   /**< Safety of the Equipment for delays */
	char Part;					   /**< Participant in event code */
	int16_t Prestige;			   /**< Prestige Earned on this Mission */
	char Duration;				   /**< Length of Mission (units:ABCDEF) */
};

struct BuzzData {                   // master data list for Buzz Aldrin's
	char Header[4];                  // Sync information
	char Name[20];                   // Player Name
	char Level;                      // Level of Play
	int16_t Cash;                      // Current Cash on Hand
	int16_t Budget;                    // Next Season's Budget
	int16_t Prestige;                    // Prestige Earned
	int16_t PrestHist[5][2];             // Prestige History array
	int16_t PresRev[5];                  // Pres. Review History
	int16_t tempPrestige[2];		         // Holds this and prev prestige
	int16_t BudgetHistory[40];         // Record of Last 40 Budgets
	int16_t BudgetHistoryF[40];        // Record of Last 40 Budgets
	int16_t Spend[5][4];                 // Record of last 5 seasons spending
					                     //  of each equip type
	char RDMods;                     // R & D Mods for turn only
	char RDYear;                     // R & D Mods for full year
	char TurnOnly;                   // Events for this turn only
	char Plans;                      // Code for future plans
					                     // 1 = Manned Launch, -10 MB
	char FuturePlans;                // Special future mission
	char DurLevel;			            // Current Duration Level
	char LMpts;                      // Accumulation of any LM Test points
#if 0
	Equipment Hardware[4][7];
#else
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
#endif
	char ZCost;                      // Cost for Zond upgrade 10-20
	char ZFlag;                      // Flag for Zond program
	char DMod;                       // Docking Module in Orbit
  /** \todo what is LaunchFacility == 1? */
    char LaunchFacility[MAX_LAUNCHPADS];    /**< Cost to repair (-1=no facility; 0 = all okay) */
	char AstroCount;                 // Current # in Program
	char AstroLevel;                 // Level of selection
	char AstroDelay;                 // Wait until next selection
	struct Astros Pool[65];          // Pool of SpaceMen
	struct WantList {
	  char Vle;
	  char Asp;
	} WList[5];
   struct Prest_Upd Udp[3];
	char WTop;
	char VList[5];
	char VTop;
	char Table[5][7];                // safety factor for intelligence
	char Gcnt[ASTRONAUT_POOLS+1][ASTRONAUT_CREW_MAX];                 // Count of num in groups
	char Crew[ASTRONAUT_POOLS+1][ASTRONAUT_CREW_MAX][ASTRONAUT_FLT_CREW_MAX+1];              // Flt Crews
					 // [5] - Program #
					 // [8] - Eight Crews Max
					 // [4] - Four Max per Crew
	char Female;                     // Females allowed in program
	struct MissionType Mission[MAX_MISSIONS];   // Current Mission Info
	char Block;                      // Blocked Mission Number
	struct MissionType Future[MAX_MISSIONS];    // Future Mission Info
	struct PastInfo History[100];    // Past Mission History
	int16_t PastMis;                   // Number of Past Missions
	char Other;                      // Catastrophic Fail on Turn
					 // 1 = Cat Failure
					 // 2 = Program Death
					 // 3 = Program First
	uint8_t Buy[4][7];	// Record of R&D for Turn - encoded dices & rolls
  char eCount;
	struct Intelligence {
	  char code;
	  int16_t num;
	  char prog;
	  char cur;
	  char index;
	  char sf;
	  char cdex;
	 } PastIntel[30];
	char AIpath[23];                 // Track AI choices from evts
	char Track[10];                  // Misc Values
	char AILunar,                    // Flag to tell way to moon
	     AIPrim,                     // Currect Primary Prog
	     AISec,                      // Currect Secondary Prog
	     AIEva,
	     AIDur,
	     AIDck,
	     AIStat,                     // the status of program
	     AITech,                     // Level of AI technology
	     AILem;
	char X[20];                      // AI where the hell am I status
	char M[62];                      // How many missions of type done
	char BadCard[15];                // Flag for Bad Events
   char Port[40];                   // Levels for SPort display
};

struct Players {
    char BUZZ[4];                    /**< Save Version Marker */
    uint32_t Checksum;               /**< Checksum of Data */
    char plr[NUM_PLAYERS];           /**< Order of Turns */
    struct Defl Def;                 /**< Defaults */
    char Year;                       /**< Game Turn */
    char Season;                     /**< Season of Year */
    struct PrestType Prestige[28];   /**< Definitions of Prest Vals */
    struct BuzzData P[NUM_PLAYERS];  /**< Player Game Data */
    char EMark[4];                   /**< Event Marker */
    char Events[100];                /**< History of Event Cards */
    char Count;                      /**< Number of Events Picked */
    char PD[NUM_PLAYERS][28];
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
enum MissionHardwareType
{
	Mission_Capsule = 0,			// 0
	Mission_Kicker,						// 1
	Mission_LM,								// 2
	Mission_Probe_DM,					// 3
	Mission_PrimaryBooster,		// 4
	Mission_EVA,							// 5
	Mission_PhotoRecon,				// 6
	Mission_SecondaryBooster	// 7
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
  int16_t code,val,xtra;     // Failure Codes
  int16_t fail;              // failure value ID
  char text[200];// Failure Text
};

struct AnimType {
  char ID[8];
  char OVL[4];
  char SD[2][4];         // Sound ID : Max 2
  int16_t w,h;
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
	char Index;		  /**< Mission Index Number */
	char Name[50];	  /**< Name of Mission */
	char Abbr[24];	  /**< Name of Mission Abbreviated */
	char Code[62];	  /**< Mission Coding String */
	char Alt[36];	  /**< Alternate Sequence */
	char AltD[36];	  /**< Alternate Sequence for Deaths (Jt Missions Only) */
	char Days,		  /**< Duration Level */
	Dur,			  /**< Duration Mission */
	Doc,			  /**< Docking Mission */
	EVA,			  /**< EVA Mission */
	LM,				  /**< LM Mission */
	Jt,				  /**< Joint Mission */
	Lun,			  /**< Lunar Mission */
	mEq,			  /**< Minimum Equipment */
	mCrew;			  /**< Useful for Morgans Code */
	uint8_t mVab[2];  /**< Hardware Requirements */
	char PCat[5],	  /**< Prestige Category List */
	LMAd;			  /**< LM Addition Points */
};

struct MXM {
   char Code[8];
   int16_t  Qty;
   int32_t Offset[6];
};

struct VInfo {char name[15],qty,ac,sf,dex,img;int16_t wt;char dmg;};
struct ManPool {char Name[14],Sex,Cap,LM,EVA,Docking,Endurance;};
struct order {int16_t plr,loc,budget,date;};

struct mfab {
  char Text[2][75];
  char qty;
  int16_t idx[4];
};



// Space port overlays
enum SpacePortOverlays
{
  PORT_Monument = 0,   		// 00 Washington's Monument
  PORT_Pentagon,			// 01 The Pentagon
  PORT_Capitol,				// 02 Capitol Building
  PORT_Cemetery,			// 03 Arlington Cemetery
  PORT_VAB,					// 04 Vehicle Assembly
  PORT_Museum, 				// 05 Museum of Space History
  PORT_Admin,				// 06 Administration
  PORT_AstroComplex,		// 07 Astronaut Complex
  PORT_MedicalCtr,			// 08 Medical Center
  PORT_BasicTraining,		// 09 Basic Training
  PORT_Helipad,				// 10 Helipad: Adv LM
  PORT_Pool,				// 11 Pool: Adv EVA
  PORT_Planetarium,			// 12 Planetarium: Adv Docking
  PORT_Centrifuge,			// 13 Centrifuge: Adv Endurance
  PORT_Airfield,			// 14 Airfield: Adv Capsule
  PORT_Satellite,			// 15 Satellite Programs
  PORT_LM,					// 16 LM Program
  PORT_Jupiter,				// 17 Jupiter Program
  PORT_XMS,					// 18 XMS Program
  PORT_Apollo,				// 19 Apollo Program
  PORT_Gemini,				// 20 Gemini Program
  PORT_Mercury,				// 21 Mercury Program
  PORT_Research,			// 22 Research & Development
  PORT_LaunchPad_A,			// 23 Launch Facility A
  PORT_LaunchPad_B,			// 24 Launch Facility B
  PORT_LaunchPad_C,			// 25 Launch Facility C
  PORT_MissionControl,		// 26 Mission Control
  PORT_ViewingStand,		// 27 Viewing Stand
  PORT_FlagPole,			// 28 Flag Pole: End Turn
  PORT_Gate,				// 29 Security Gate: Quit
  PORT_Moon,				// 30 Moon
  PORT_SovMonumentAlt,		// 31 Soviet Monument #2
  PORT_Zond,				// 32 Zond - unused
  PORT_Tracking,			// 33 Tracking Station
  PORT_SVHQ,				// 34 Strategic Visions H.Q.
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


#define MAX_REPLAY_ITEMS		200L
#define MAX_REPLAY_ITEM_OFFSETS	35

// Mission Replay Data Structure
typedef struct ReplayItem {
   uint8_t Qty;         // Number of Animated Sequences
   uint16_t Off[35];     // Offsets to Each animated Part
} REPLAY;

enum Opponent_Status {Ahead,Equal,Behind};

typedef struct oldNews {
  uint32_t offset;
  uint16_t size;
} ONEWS;

// Save Game related typedefs
//#define RaceIntoSpace_Signature	'RiSP'
#define RaceIntoSpace_Signature 0x52695350
#define RaceIntoSpace_Old_Sig 0x49443a00  //'ID:\0"
#define SAVEFILE_UNK_MAGIC	0x1A

typedef enum {
	SAVEGAME_Normal = 0,
	SAVEGAME_PlayByMail,
	SAVEGAME_Modem
} SaveGameType;

typedef struct {
	uint32_t ID;		// Going to use this to determine endianness of the save file
	char Name[23],PName[2][20],Country[2],Season,Year;
	uint16_t dataSize;		// Size of Players struct
	uint16_t compSize;		// Compressed size of data
} SaveFileHdr;


typedef struct {
    char Name[15],Title[23];
    uint16_t time,date;
} SFInfo;


#endif // __DATA_H__

/* vim: set noet ts=4 sw=4 tw=77: */
