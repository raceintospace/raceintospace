#include "roster_entry.h"

#include <cassert>
#include <cstring>

#include <json/json.h>

#include "options.h"
#include "pace.h"
#include "roster_group.h"

RosterEntry::RosterEntry(const RosterGroup &group, const Json::Value &json_object)
    : m_group_number(group.getGroupNumber())
{
    assert(json_object.isObject());

    m_name = json_object["name"].asString();
    m_female = json_object.get("female", false).asBool();
    m_capsule = json_object.get("capsule", 0).asInt();
    m_lunar = json_object.get("lunar", 0).asInt();
    m_eva = json_object.get("eva", 0).asInt();
    m_docking = json_object.get("docking", 0).asInt();
    m_endurance = json_object.get("endurance", 0).asInt();
}

std::string RosterEntry::getName() const
{
    return m_name;
}

bool RosterEntry::isFemale() const
{
    return m_female;
}

int RosterEntry::getCapsule() const
{
    return m_capsule;
}

int RosterEntry::getLunar() const
{
    return m_lunar;
}

int RosterEntry::getEVA() const
{
    return m_eva;
}

int RosterEntry::getDocking() const
{
    return m_docking;
}

int RosterEntry::getEndurance() const
{
    return m_endurance;
}

void RosterEntry::randomize()
{
    // FIXME: this is entirely too generous compared to the historical roster
    m_capsule = brandom(5);
    m_lunar = brandom(5);
    m_eva = brandom(5);
    m_docking = brandom(5);
    m_endurance = brandom(5);
}

Astros *RosterEntry::recruit(BuzzData &player) const
{
    // get a pointer to the next Astros, and increment the count
    Astros *astronaut = &player.Pool[(uint8_t)player.AstroCount++];

    // zero this
    memset(astronaut, 0, sizeof(Astros));
    strncpy(astronaut->Name, getName().c_str(), sizeof(astronaut->Name) - 1);
    astronaut->Name[sizeof(astronaut->Name) - 1] = '\0';    // ensure NUL-terminated
    astronaut->Group = m_group_number - 1; // XXX: is Astros->Group 0-based?
    astronaut->Sex = isFemale() ? 1 : 0;
    astronaut->Cap = getCapsule();
    astronaut->LM = getLunar();
    astronaut->EVA = getEVA();
    astronaut->Docking = getDocking();
    astronaut->Endurance = getEndurance();
    astronaut->Status = AST_ST_ACTIVE;
    astronaut->oldAssign = -1;
    astronaut->TrainingLevel = 1;
    astronaut->CR = brandom(2) + 1;
    astronaut->CL = brandom(2) + 1;
    astronaut->Compat = brandom(options.feat_compat_nauts) + 1;
    astronaut->Mood = 100;

    if (astronaut->Sex == 0) {
        astronaut->Face = brandom(77);
    } else {
        astronaut->Face = 77 + brandom(8);
    }

    return astronaut;
}
