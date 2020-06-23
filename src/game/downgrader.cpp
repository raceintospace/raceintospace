/* Copyright (C) 2020 Ryan Yoakum
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

// This file handles mission downgrades.

#include "downgrader.h"

#include <cassert>
#include <fstream>
#include <json/json.h>

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

    mDowngrades[mission].push_back(code);
}


/**
 */
const std::vector<int> Downgrader::Options::downgrades(int mission) const
{
    return (mDowngrades.size() > mission) ?  mDowngrades[mission]
           : std::vector<int>();
}


/**
 * Creates a downgrader instance from a given base mission and
 * a set of alternate missions for that base mission.
 *
 * \param mission  The original mission to be downgraded.
 * \param downgrades  A set of MissionType.MissionCode(s).
 */
Downgrader::Downgrader(const struct MissionType &mission,
                       const std::vector<int> &downgrades)
    : mBasis(mission), mCurrent(mission), mDuration(mission.Duration)
{
    // if (downgrades.find(mission.MissionCode) != downgrades.end()) {
    //     mCodes.push_back(mission.MissionCode);
    // }
    mCodes.push_back(mission.MissionCode);

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
    const std::vector<int> dgs = downgrades.downgrades(mission.MissionCode);

    // if (dgs.find(mission.MissionCode) == dgs.end()) {
    //     mCodes.push_back(mission.MissionCode);
    // }
    mCodes.push_back(mission.MissionCode);

    mCodes.insert(mCodes.end(), dgs.begin(), dgs.end());

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
 * When this method returns an unmanned alternative for a manned mission,
 * it strips the crew information from the return value. Because it only
 * generates alternate missions, it does not update the crew's status.
 *
 * TODO: Should check to ensure a mission is not being downgraded to a
 * mission type it cannot become.
 *  - A Joint mission cannot downgrade to a single launch, and vice versa.
 *  - A non-duration mission cannot downgrade to a duration mission.
 *  - An unmanned mission cannot become a manned mission.
 *  - Probes cannot downgrade, or be the result of downgrading.
      (Exception: I've allowed the Lunar Probe Landing to be downgraded to
       a Lunar Flyby, on the logic that any probe capable of landing should
       be able to do a flyby.  The idea is that if a player has flybys
       stubbornly failing and finds themselves with the Lunar Probe Landing
       in the VAB, it should be an option to fly the landing probe as a
       flyby to avoid a milestone penalty - rather than having to scrub it
       and delay your flyby by a turn. -Leon)
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

//----------------------------------------------------------------------
// End of Downgrader class methods
//----------------------------------------------------------------------


/* Read the mission downgrade options from a file.
 *
 * The Json format is:
 * {
 *   "missions": [
 *     { "mission": <Code>, "downgrades": [<Codes>] },
 *     ...
 *   ]
 * }
 *
 * \param filename  A Json-formatted data file.
 * \return  A collection of MissionType.MissionCode-indexed downgrade
 *          options.
 * \throws IOException  If filename is not a readable Json file.
 */
Downgrader::Options LoadJsonDowngrades(std::string filename)
{
    char *path = locate_file(filename.c_str(), FT_DATA);

    if (path == NULL) {
        free(path);
        throw IOException(std::string("Unable to open path to ") +
                          filename);
    }

    std::ifstream input(path);
    Json::Value doc;
    Json::Reader reader;
    bool success = reader.parse(input, doc);

    if (! success) {
        free(path);
        throw IOException("Unable to parse JSON input stream");
    }

    assert(doc.isObject());
    Json::Value &missionList = doc["missions"];
    assert(missionList.isArray());

    Downgrader::Options options;

    for (int i = 0; i < missionList.size(); i++) {
        Json::Value &missionEntry = missionList[i];
        assert(missionEntry.isObject());

        int missionCode = missionEntry.get("mission", -1).asInt();
        assert(missionCode >= 0);
        // assert(missionCode >= 0 && missionCode <= 61);

        Json::Value &codeGroup = missionEntry["downgrades"];
        assert(codeGroup.isArray());

        for (int j = 0; j < codeGroup.size(); j++) {
            options.add(missionCode, codeGroup[j].asInt());
        }
    }

    input.close();
    free(path);
    return options;
}
