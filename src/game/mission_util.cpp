#include "mission_util.h"
#include "logging.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);


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
