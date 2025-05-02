#ifndef MIS_C_H
#define MIS_C_H

#include "data.h"

void FirstManOnMoon(char plr, char isAI, char misNum,
                    const struct MisEval &step);
void PlaySequence(char plr, int step, const char *Seq, char mode);
char FailureMode(char plr, int prelim, char *text);
void Tick(char plr);

extern char daysAMonth[12];
extern char STEPnum;
extern struct AnimType AHead;
extern struct BlockHead BHead;

#endif // MIS_C_H
