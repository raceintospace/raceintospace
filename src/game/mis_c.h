#ifndef MIS_C_H
#define MIS_C_H

#include "data.h"

void FirstManOnMoon(char plr, char isAI, char misNum,
                    const MisEval& step);
void PlaySequence(char plr, int step, const char* Seq, char mode);
char FailureMode(char plr, int prelim, const char* text);
void Tick(char plr);

extern char daysAMonth[12];
extern char STEPnum;
extern AnimType AHead;
extern BlockHead BHead;

#endif // MIS_C_H
