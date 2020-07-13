#ifndef PREST_H
#define PREST_H

int AchievementPenalty(char plr, const struct mStr &mission);
int DurationPenalty(char plr, const struct mStr &mission);
int MilestonePenalty(char plr, const struct mStr &mission);
int NewMissionPenalty(char plr, const struct mStr &mission);

int PrestNeg(char plr, int i);
char Set_Goal(char plr, char which, char control);
int Find_MaxGoal(void);
int U_AllotPrest(char plr, char mis);
int AllotPrest(char plr, char mis);
int MaxFail(void);

#endif // PREST_H
