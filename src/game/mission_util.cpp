// This file seems to handle identifying the type of a given mission

#include "mission_util.h"

#include <cstring>
#include <cstdio>

#include "Buzz_inc.h"
#include "ioexception.h"
#include "logging.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);


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


/* Gets the mission template for the specified mission code.
 *
 * Opens "MISSION.DAT" and loads the mission data from the file.
 *
 * TODO: This is dependent on the exact size of internal structures.
 *       MISSION.DAT relies on 226-byte mStr structs.
 *
 * \param code  A unique index for the mission.
 * \return  the mStr with the given mStr.Index value.
 * \throws IOException  if unable to read MISSION.DAT.
 */
struct mStr GetMissionPlan(const int code)
{
    mStr mission;
    FILE *fin = sOpen("MISSION.DAT", "rb", 0);

    if (! fin) {
        throw IOException("Error opening file MISSION.DAT");
    }

    // Find Mission Type
    if (fseek(fin, code * (sizeof(struct mStr)), SEEK_SET)) {
        char error[1000];
        sprintf(error, "Error %d while seeking position in file "
                "MISSION.DAT: Seeking position %lu"
                " (%d blocks of size %lu)",
                ferror(fin),
                static_cast<const size_t>(code) * (sizeof(struct mStr)),
                code, sizeof(struct mStr));
        fclose(fin);
        throw IOException(error);
        // throw IOException("Error seeking position in file MISSION.DAT");
    }

    if (fread(&mission, sizeof(struct mStr), 1, fin) != 1) {
        fclose(fin);
        throw IOException("Error reading from file MISSION.DAT");
    }

    fclose(fin);

    return mission;
}
