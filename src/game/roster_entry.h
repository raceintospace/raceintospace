#ifndef ROSTER_ENTRY_H
#define ROSTER_ENTRY_H

#include <json/json.h>
#include <string>

#include "data.h"

class RosterGroup;

class RosterEntry
{
public:
    RosterEntry(const RosterGroup &group, const Json::Value &json_object);
    ~RosterEntry() = default;

    std::string getName() const;
    bool isFemale() const;
    int getCapsule() const;
    int getLunar() const;
    int getEVA() const;
    int getDocking() const;
    int getEndurance() const;

    // the AI uses this to determine who's best
    inline int getRecruitingPriority() const
    {
        return getCapsule() + getLunar() + getEVA() + getDocking();
    };

    // randomize the properties of this roster entry
    void randomize();

    // recruit this roster entry into an astronaut
    Astros *recruit(BuzzData &player) const;

private:
    std::string m_name;
    int m_group_number;
    bool m_female;
    int m_capsule, m_lunar, m_eva, m_docking, m_endurance;
};

#endif // ROSTER_ENTRY_H
