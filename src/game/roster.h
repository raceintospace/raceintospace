#ifndef ASTRONAUT_POOL_H
#define ASTRONAUT_POOL_H

#include <json/json.h>
#include <string>

#include "data.h"

class RosterEntry
{
public:
    RosterEntry(const Json::Value& json_object);
    ~RosterEntry();
    
    inline std::string getName() { return m_name; };
    inline int getPlayer() { return m_player; };
    inline int getGroup() { return m_group; };
    inline bool isFemale() { return m_female; };
    inline int getCapsule() { return m_capsule; };
    inline int getLunar() { return m_lunar; };
    inline int getEVA() { return m_eva; };
    inline int getDocking() { return m_docking; };
    inline int getEndurance() { return m_endurance; };
    
    // the AI uses this to determine who's best
    inline int getRecruitingPriority() { return getCapsule() + getLunar() + getEVA() + getDocking(); };
    
    // randomize the properties of this roster entry
    void randomize();
    
    // recruit this roster entry into an astronaut
    Astros* recruit(BuzzData& player);
    
private:
    std::string m_name;
    int m_player;
    int m_group;
    bool m_female;
    int m_capsule, m_lunar, m_eva, m_docking, m_endurance;
};

#endif // ASTRONAUT_POOL_H