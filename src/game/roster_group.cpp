#include "roster_group.h"

#include <cassert>

#include <boost/foreach.hpp>
#include <json/json.h>

RosterGroup::RosterGroup(int player, int group_number, const Json::Value &json_object)
    : m_player(player), m_group_number(group_number)
{
    // the group object should look like:
    // {
    //    recruiting_delay: 4,
    //    number_to_choose: 7,
    //    entries: [ /* entry */, /* entry */, /* entry */ ]
    // }

    assert(json_object.isObject());

    m_recruiting_delay = json_object.get("recruiting_delay", 99).asInt();
    m_number_to_choose = json_object.get("number_to_choose", 10).asInt();

    const Json::Value &json_entries = json_object["entries"];
    assert(json_entries.isArray());

    // walk over all the entries in this array
    for (int i = 0; i < json_entries.size(); i++) {
        const Json::Value &object = json_entries[i];

        // parse this JSON object
        RosterEntry roster_entry(*this, object);

        // add it to the list of entries
        m_entries.push_back(roster_entry);

        // add to the list of male entries if appropriate
        if (!roster_entry.isFemale()) {
            m_male_entries.push_back(roster_entry);
        }
    }
}

RosterGroup::~RosterGroup()
{
}

const roster_entries_t &RosterGroup::getRosterEntries(bool include_females) const
{
    if (include_females) {
        return m_entries;
    } else {
        return m_male_entries;
    }
}

int RosterGroup::getPlayer() const
{
    return m_player;
}

int RosterGroup::getGroupNumber() const
{
    return m_group_number;
}

int RosterGroup::getRecruitingDelay() const
{
    return m_recruiting_delay;
}

int RosterGroup::getNumberToChoose() const
{
    return m_number_to_choose;
}

void RosterGroup::randomize()
{
    BOOST_FOREACH(RosterEntry & roster_entry, m_entries) {
        roster_entry.randomize();
    }
}
