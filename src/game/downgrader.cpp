/* TODO: Copyright
 */

#include <cassert>

#include <json/json.h>

#include "downgrader.h" // Moved down because it includes Buzz_inc.h

#include "Buzz_inc.h"
#include "ioexception.h"
#include "logging.h"
#include "mission_util.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);

/**
 * Creates a Downgrader instance from a given base mission and
 * a MissionCode-indexed list of alternate mission codes.
 *
 * \param mission  The original mission to be downgraded.
 * \param downgrades  A char array of size[62][6].
 */
Downgrader::Downgrader(const struct MissionType &mission,
                       char downgrades[62][6])
    : mBasis(mission), mCurrent(mission), mDuration(mission.Duration)
{
    mCodes.push_back(mission.MissionCode);

    for (int mis = 0; mis < 6; mis++) {
        if (downgrades[mission.MissionCode][mis] != Mission_None) {
            mCodes.push_back(downgrades[mission.MissionCode][mis]);
        } else {
            break;
        }
    }

    mCodes.push_back(0);
    mIndex = mCodes.begin();
}


/**
 * Creates a Downgrader instance from a given base mission and
 * downgrade options.
 *
 * TODO: If unable to parse, should an exception be thrown instead?
 *
 * \param mission  The seed mission.
 * \param input  A JSON-formatted input stream.
 */
// Downgrader::Downgrader(const struct MissionType &mission,
//                        std::istream &input)
//     : mBasis(mission), mCurrent(mission), mDuration(mission.Duration)
// {
//     mCodes.push_back(mission.MissionCode);
//
//     Json::Value doc;
//     Json::Reader reader;
//     bool success = reader.parse(input, doc);
//
//     if (! success) {
//         CRITICAL1("Unable to parse JSON input stream");
//         mCodes.push_back(0);
//         mIndex = mCodes.begin();
//         return;
//     }
//
//     bool found = false;
//
//     for (int i = 0; i < doc.size(); i++) {
//         Json::Value &missionObject = doc[i];
//         assert(missionObject.isObject());
//
//         int missionCode = missionObject.get("mission", -1).asInt();
//         assert(missionCode >= 0 && missionCode <= 61);
//
//         if (missionCode == mission.MissionCode) {
//             found = true;
//             Json::Value &codeGroup = missionObject["downgrades"];
//             assert(codeGroup.isArray());
//
//             for (int j = 0; j < codeGroup.size(); j++) {
//                 mCodes.push_back(codeGroup[j].asInt());
//             }
//
//             break;
//         }
//     }
//
//     if (! found) {
//         ERROR2("Could not find downgrade options for mission %d",
//                mission.MissionCode);
//     }
//
//     // Ensure Mission_None is always available.
//     if (mCodes.back() != Mission_None) {
//         mCodes.push_back(Mission_None);
//     }
//
//     mIndex = mCodes.begin();
// }


Downgrader::~Downgrader()
{
}


/* Generates an instance of the current mission.
 *
 * \return  The last value returned by next(), or the original mission
 *          if next() has not been called.
 */
struct MissionType Downgrader::current() const {
    return mCurrent;
}


/* Cycles to the next downgrade option for the basis MissionType.
 *
 * The first option this method returns is the original mission.
 * After this method exhausts all known downgrades for the mission,
 * it restarts with the original mission.
 *
 * When this method returns a unmanned alternative for a manned mission,
 * it strips the crew information from the return value. Because it only
 * generates alternate missions, it does not update the crew's status.
 *
 * TODO: Should check to ensure a mission is not being downgraded to a
 * mission type it cannot become.
 *  - A Joint mission cannot downgrade to a single launch, and vice versa.
 *  - A non-duration mission cannot downgrade to a duration mission.
 *  - An unmanned mission cannot become a manned mission.
 *
 * \return  a new instance of the next downgrade option in the cycle.
 */
struct MissionType Downgrader::next()
{
    // Advance the cycle to the next option.
    struct mStr style;

    try {
        style = GetMissionPlan(*mIndex);
    } catch (IOException &err) {
        CCRITICAL2(filesys, err.what());
        mCurrent = mBasis;
        return mCurrent;
    }

    // If not on a Duration mission, advance to the next mission code.
    if (style.Index != mBasis.MissionCode || ! style.Dur ||
        --mDuration < style.Days) {

        if (++mIndex == mCodes.end()) {
            mIndex = mCodes.begin();
        }

        try {
            style = GetMissionPlan(*mIndex);
        } catch (IOException &err) {
            CCRITICAL2(filesys, err.what());
            mCurrent = mBasis;
            return mCurrent;
        }

        mDuration = (style.Dur) ? mBasis.Duration : style.Days;
        mCurrent = mBasis;
        mCurrent.MissionCode = style.Index;
        mCurrent.Duration = mDuration;

        // If the mission type is changing from Manned to Unmanned,
        // delist the crew.
        if (mCurrent.Men > 0 &&
            (style.mCrew == 0 || style.mCrew == 1 || style.mCrew == 5)) {
            mCurrent.Men = 0;
            mCurrent.PCrew = 0;
            mCurrent.BCrew = 0;
            mCurrent.Crew = 0;
        }
    } else {
        mCurrent.Duration = mDuration;
    }

    return mCurrent;
}
