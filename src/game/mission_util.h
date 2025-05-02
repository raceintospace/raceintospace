#ifndef MISSION_UTIL_H
#define MISSION_UTIL_H

#include <vector>

#include "data.h"


bool Equals(const struct MissionType &m1, const struct MissionType &m2);
const char *GetDurationParens(int duration);
std::vector<struct mStr> GetMissionData();
struct mStr GetMissionPlan(int code);
void DrawMissionName(int val, int posX, int posY, int len);
bool IsDocking(int mission);
bool IsDuration(int mission);
bool IsEVA(int mission);
bool IsJoint(int mission);
bool IsLEORegion(int mission);
bool IsLunarLanding(int mission);
bool IsLM(int mission);
bool IsManned(int mission);
bool MissionTimingOk(int mission, unsigned int year, unsigned int season);


#endif // MISSION_UTIL_H
