#ifndef MISSION_UTIL_H
#define MISSION_UTIL_H


bool Equals(const struct MissionType &m1, const struct MissionType &m2);
const char *GetDurationParens(int duration);
struct mStr GetMissionPlan(int code);
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
