#ifndef PREST_H
#define PREST_H

#include "data.h"

int AchievementPenalty(char plr, const mStr& mission);
int DurationPenalty(char plr, const mStr& mission);
int MilestonePenalty(char plr, const mStr& mission);
int NewMissionPenalty(char plr, const mStr& mission);

int PrestNeg(char plr, int i);
char Set_Goal(char plr, char which, char control);
int Find_MaxGoal();
int U_AllotPrest(char plr, char mis);
int AllotPrest(char plr, char mis);
int MaxFail();
int Update_Prestige_Data(char plr, char mis, int code);

#endif // PREST_H
