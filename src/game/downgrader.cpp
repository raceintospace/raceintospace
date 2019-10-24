/* TODO: Copyright
 */

#include <cassert>

#include "downgrader.h" // Moved down because it includes Buzz_inc.h

#include "Buzz_inc.h"
#include "ioexception.h"
#include "logging.h"
#include "mission_util.h"


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);


Downgrader::Options::Options()
{
}


Downgrader::Options::~Options()
{
}


int Downgrader::Options::add(const int mission, const int code)
{
    if (mission >= mDowngrades.size()) {
        mDowngrades.resize(mission + 1);
    }

    mDowngrades[mission].insert(code);
}


/**
 */
const std::set<int> Downgrader::Options::downgrades(int mission) const
{
    return (mDowngrades.size() > mission) ?  mDowngrades[mission]
        : std::set<int>();
}


/**
 * Creates a downgrader instance from a given base mission and
 * a set of alternate missions for that base mission.
 *
 * \param mission  The original mission to be downgraded.
 * \param downgrades  A set of MissionType.MissionCode(s).
 */
Downgrader::Downgrader(const struct MissionType &mission,
                       const std::set<int> &downgrades)
    : mBasis(mission), mCurrent(mission), mDuration(mission.Duration)
{
    if (downgrades.find(mission.MissionCode) != downgrades.end()) {
        mCodes.push_back(mission.MissionCode);
    }

    mCodes.insert(mCodes.end(), downgrades.begin(), downgrades.end());

    // Ensure Mission_None is always available.
    if (mCodes.back() != Mission_None) {
        mCodes.push_back(Mission_None);
    }

    mIndex = mCodes.begin();
}


/**
 * Creates a Downgrader instance from a given base mission and
 * a collection of alternate mission codes.
 *
 * \param mission  The original mission to be downgraded.
 * \param downgrades  A collection of mission downgrade choices.
 */
Downgrader::Downgrader(const struct MissionType &mission,
                       const Options downgrades)
    : mBasis(mission), mCurrent(mission), mDuration(mission.Duration)
{
    const std::set<int> dgs = downgrades.downgrades(mission.MissionCode);

    if (dgs.find(mission.MissionCode) == dgs.end()) {
        mCodes.push_back(mission.MissionCode);
    }

    mCodes.insert(mCodes.end(), dgs.rbegin(), dgs.rend());

    // Ensure Mission_None is always available.
    if (mCodes.back() != Mission_None) {
        mCodes.push_back(Mission_None);
    }

    mIndex = mCodes.begin();
}


Downgrader::~Downgrader()
{
}


/* Generates an instance of the currently selected alternate mission.
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
