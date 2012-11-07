#ifndef MIS_C_H
#define MIS_C_H

int RocketBoosterSafety(int safetyRocket, int safetyBooster);
void FirstManOnMoon(char plr, char isAI, char misNum);
void PlaySequence(char plr, int step, const char *Seq, char mode);
char FailureMode(char plr, int prelim, char *text);

#endif // MIS_C_H