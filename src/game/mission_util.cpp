// This file seems to handle identifying the type of a given mission

#include "mission_util.h"

#include <cstring>
#include <cstdio>
#include <vector>

#include "Buzz_inc.h"
#include "draw.h"
#include "gr.h"
#include "ioexception.h"
#include "logging.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);

namespace
{
    bool MarsInRange(unsigned int year, unsigned int season);
    bool JupiterInRange(unsigned int year, unsigned int season);
    bool SaturnInRange(unsigned int year, unsigned int season);
};

std::vector<struct mStr> missionData;

//----------------------------------------------------------------------
// Header function definitions
//----------------------------------------------------------------------


/* Compares two MissionType instances to see if their fields are
 * identical.
 *
 * NOTE: It would probably be nicer to overload the equivalence operator
 * for struct MissionType, but that might alter the size of the struct,
 * and that's currently verboten. -- rnyoakum
 *
 * \param m1  The first instance.
 * \param m2  The second instance.
 * \returns  true if equivalent.
 */
bool Equals(const struct MissionType &m1, const struct MissionType &m2)
{
    return m1.MissionCode == m2.MissionCode &&
           (strcmp(&m1.Name[0], &m2.Name[0]) == 0) &&
           // (memcmp(&m1.Name[0], &m2.Name[0], sizeof(m1.Name)) == 0) &&
           m1.Patch == m2.Patch && m1.part == m2.part &&
           (memcmp(&m1.Hard[0], &m2.Hard[0], sizeof(m1.Hard)) == 0) &&
           m1.Joint == m2.Joint && m1.Rushing == m2.Rushing &&
           m1.Month == m2.Month && m1.Duration == m2.Duration &&
           m1.Men == m2.Men && m1.Prog == m2.Prog &&
           m1.PCrew == m2.PCrew && m1.BCrew == m2.BCrew &&
           m1.Crew == m2.Crew;
}


/* Checks if the mission type corresponds to a docking mission.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if the mission has a docking step, false otherwise.
 * \throws IOException  if unable to load the mission template.
 */
bool IsDocking(const int mission)
{
    return ((GetMissionPlan(mission).Doc >= 1) ? true : false);
}


/* Checks via mission code if the mission type corresponds to a
 * duration mission.
 *
 * This implementation depends upon strict mission numbering, so any
 * changes to the mission data file could result in errors.
 * A more durable implementation would load the corresponding mission
 * from the data file and check its duration field. However, this
 * approach is fast and doesn't rely on file I/O.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if a duration-adjustable mission, false otherwise.
 */
bool IsDuration(int mission)
{
    return ((mission > 24 && mission < 32) || mission == 33 ||
            mission == 34 || mission == 35 || mission == 37 ||
            mission == 40 || mission == 41);
}


/**
 * Checks if the mission contains a spacewalk.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if an EVA step is required.
 */
bool IsEVA(int mission)
{
    return GetMissionPlan(mission).EVA >= 1;
}


/**
 * Checks if the mission has two separate launches.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if a joint launch.
 */
bool IsJoint(int mission)
{
    return GetMissionPlan(mission).Jt >= 1;
}


/**
 * Checks if the mission takes place within the Low Earth Orbit (LEO)
 * region of space.
 *
 * The Low Earth Orbit region includes "space" through Low Earth Orbit
 * and thus covers suborbital flights.
 *
 * The Atlas/R-7 rocket is incapable of reaching lunar orbit or beyond.
 * This implementation depends upon strict mission numbering, so any
 * changes to the mission data file could result in errors.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if within the LEO region, false if beyond.
 */
bool IsLEORegion(int mission)
{
    return !((mission >= Mission_LunarFlyby &&
              mission <= Mission_SaturnFlyby) ||
             (mission >= 42 && mission <= 57));
}


/**
 * Checks if the mission uses a Lunar Module.
 *
 * All missions using a LEM are manned.
 *
 * \param mission  The type per mStr.Index of MissionType.MissionCode.
 * \return  true if the crew operates the lunar module.
 */
bool IsLM(int mission)
{
    return GetMissionPlan(mission).LM >= 1;
}


/**
 * Checks if the mission is a lunar landing.
 *
 * This implementation depends upon strict mission numbering, so any
 * changes to the mission data file could result in errors.
 * A more durable implementation would load the corresponding mission
 * from the data file and check the Prestige Categories field (mStr.PCcat)
 * for the presence of Prestige_MannedLunarLanding.
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if a lunar landing, false otherwise.
 */
bool IsLunarLanding(int mission)
{
    return (mission >= Mission_HistoricalLanding &&
            mission <= Mission_Soyuz_LL);
}


/**
 * Checks if the mission is manned.
 *
 * This method uses Morgan's code to check the mission type to see
 * if it is manned. In Morgan's code, mission types are
 *   0: An unmanned mission
 *   1: A single pad unmanned mission with capsule/minishuttle
 *   2: A single pad manned mission
 *   3: A joint mission with a single manned launch
 *   4: A joint mission with two manned launches
 *   5: A joint mission with an unmanned capsule/minishuttle
 *
 * Morgan's code is a little esoteric, so other methods of checking
 * include:
 *  - Strict mission numbering check (no file IO, but vulnerable
 *    to change), or
 *  - checking the Prestige Categories field (mStr.PCcat) for the
 *    presence of Prestige_MannedSpaceMission, or
 *  - the duration listing (mStr.Days) for a non-zero value
 *    (Prevents use of mStr.Days for storing Flyby duration).
 *
 * \param mission  The type per mStr.Index or MissionType.MissionCode.
 * \return  true if manned, false otherwise.
 * \throws IOException  if unable to load the mission template.
 */
bool IsManned(int mission)
{
    char mCrew = GetMissionPlan(mission).mCrew;
    return (mCrew == 2 || mCrew == 3 || mCrew == 4);
}


/* Return a letter representation of the mission duration, surrounded
 * by parenthesis, for appending to a mission name.
 *
 * Only string literals are used so there is no need to manage memory.
 *
 * \param duration  the mission length in increments (0-6).
 * \return  a string literal of the form " (X)".
 */
const char *GetDurationParens(int duration)
{
    switch (duration) {
    case 0:
    case 1:
        return "";

    case 2:
        return " (B)";

    case 3:
        return " (C)";

    case 4:
        return " (D)";

    case 5:
        return " (E)";

    case 6:
        return " (F)";

    default:
        ERROR2("Invalid arg to GetDurationParens(duration = %d)",
               duration);
        return "";
    }

    return "";
}


/**
 * Cache a subset of mission data in a local array.
 *
 * Populates the global vector missionData with stored mission data.
 */
std::vector<struct mStr> GetMissionData()
{       
    // Deserialize Mission Data
    std::ifstream file(locate_file("mission.json", FT_DATA));
    if (!file) {
      throw IOException("Error. Could not open mission.json");
    }

    cereal::JSONInputArchive ar(file);
    ar(CEREAL_NVP(missionData));
    DEBUG1("missionData successfully uploaded.");
    return missionData;
}


/* Gets the mission template for the specified mission code.
 *
 * Opens "mission.json" and loads the mission data from the file.
 *
 *
 * \param code  A unique index for the mission.
 * \return  the mStr with the given mStr.Index value.
 * \throws IOException  if unable to read MISSION.DAT.
 */
struct mStr GetMissionPlan(const int code)
{
    if (missionData.empty()) {
        GetMissionData();
    }
    
    mStr mission = missionData[code];
    TRACE2("mission plan `%d' loaded.", code);
    
    return mission;
}


/* Prints the name of the selected mission.
 *
 * This writes the name of the mission associated with the given mission code
 *
 * \param mission  The mission data.
 * \param posX   The x coordinates for the name block's upper-left corner.
 * \param posY   The y coordinates for the name block's upper-left corner.
 * \param len  The number of characters at which to start a new line.
 */
void DrawMissionName(int val, int posX, int posY, int len)
{
    TRACE5("->DrawMissionName(val %d, posX %d, posxY %d, len %d)",
           val, posX, posY, len);
    int i, j = 0;

    grMoveTo(posX, posY);
    
    mStr mission = GetMissionPlan(val);    
    
    for (i = 0; i < 50; i++) {
        if (j > len && mission.Name[i] == ' ') {
            posY += 7;
            j = 0;
            grMoveTo(posX, posY);
        } else {
            draw_character(mission.Name[i]);
        }

        j++;

        if (mission.Name[i] == '\0') {
            break;
        }
    }

    TRACE1("<-DrawMissionName");

    return;
}

/**
 * Check if the mission is viable given the launch period.
 *
 * Not all missions can be performed during every year and season.
 * While the Moon is always nearby, the orbits of other planets can
 * take them far enough away from the Earth that an intercept course
 * isn't possible, at least not on a reasonable schedule.
 *
 * \param mission  the mission code.
 * \param year  the year the mission would be scheduled as a Future mission.
 * \param season  the season during which the mission is planned as a FM.
 * \return  true if the mission may be scheduled.
 */
bool MissionTimingOk(int mission, unsigned int year, unsigned int season)
{
    if (mission == Mission_MarsFlyby) {
        return MarsInRange(year, season);
    } else if (mission == Mission_JupiterFlyby) {
        return JupiterInRange(year, season);
    } else if (mission == Mission_SaturnFlyby) {
        return SaturnInRange(year, season);
    }

    return true;
}


//----------------------------------------------------------------------
// Local definitions
//----------------------------------------------------------------------

namespace  // Start of local namespace
{

/* Is Mars at the right point in its orbit where a rocket launched at
 * the given time will be able to intercept it?
 */
bool MarsInRange(unsigned int year, unsigned int season)
{
    return ((year == 60 && season == 0) || (year == 62 && season == 0) ||
            (year == 64 && season == 0) || (year == 66 && season == 0) ||
            (year == 69 && season == 1) || (year == 71 && season == 1) ||
            (year == 73 && season == 1));
}


/* Is Jupiter at the right point in its orbit where a rocket launched
 * at the given time will be able to intercept it?
 */
bool JupiterInRange(unsigned int year, unsigned int season)
{
    return (year == 60 || year == 64 || year == 68 || year == 72 ||
            year == 73 || year == 77);
}


/* Is Saturn at the right point in its orbit where a rocket launched
 * at the given time will be able to intercept it?
 */
bool SaturnInRange(unsigned int year, unsigned int season)
{
    return (year == 61 || year == 66 || year == 72);
}

};  // End of local namespace
