#ifndef ROSTER_H
#define ROSTER_H

#include <list>
#include <istream>
#include <string>

#include "roster_group.h"

using roster_groups_t = std::list<RosterGroup>;

class Roster
{
public:
    Roster(std::istream& input_stream);
    ~Roster() = default;

    RosterGroup& getGroup(int player, int group_number);

    static Roster load(const std::string& filename = "roster.json");

private:
    roster_groups_t m_groups;
};

#endif // ROSTER_H
