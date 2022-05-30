#ifndef MC2_H
#define MC2_H

#include "data.h"

void MissionCodes(char plr, char val, char pad);
void MissionSetup(char plr, char mis);
void MissionSetDown(char plr, char mis);
void MisSkip(char plr, const struct mStr &mission);
void MisRush(char plr, char rush_level);

#endif // MC2_H
