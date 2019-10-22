#ifndef MISSION_UTIL_H
#define MISSION_UTIL_H


bool Equals(const struct MissionType &m1, const struct MissionType &m2);
const char *GetDurationParens(int duration);
struct mStr GetMissionPlan(int code);
bool IsDuration(int mission);


#endif // MISSION_UTIL_H
