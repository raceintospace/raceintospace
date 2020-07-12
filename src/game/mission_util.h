#ifndef MISSION_UTIL_H
#define MISSION_UTIL_H


bool Equals(const struct MissionType &m1, const struct MissionType &m2);
const char *GetDurationParens(int duration);
struct mStr GetMissionPlan(int code);
bool IsDocking(int mission);
bool IsDuration(int mission);
bool IsLunarLanding(int mission);
bool MissionTimingOk(int mission, unsigned int year, unsigned int season);


#endif // MISSION_UTIL_H
