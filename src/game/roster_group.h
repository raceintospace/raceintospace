#ifndef ROSTER_GROUP_H
#define ROSTER_GROUP_H

#include <json/json.h>

#include <string>
#include <list>

#include "roster_entry.h"

typedef std::list<RosterEntry> roster_entries_t;

class RosterGroup
{
public:
    RosterGroup(int player, int group_number, const Json::Value &json_array);
    ~RosterGroup() = default;

    const roster_entries_t &getRosterEntries(bool include_females) const;

    int getPlayer() const;

    // which group is this? (1-based)
    int getGroupNumber() const;

    // how many turns do we wait after this group before recruiting again?
    // FIXME: I feel like this should be inverted (how many turns until this group is ready)
    int getRecruitingDelay() const;

    // how many astronauts should we recruit from this group?
    int getNumberToChoose() const;

    // randomize the properties of all the astronauts in this group
    void randomize();

private:
    int m_player;
    int m_group_number;
    int m_recruiting_delay;
    int m_number_to_choose;
    roster_entries_t m_entries;
    roster_entries_t m_male_entries;
};

#endif // ROSTER_GROUP_H
