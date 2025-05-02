#ifndef MIS_M_H
#define MIS_M_H

#include "data.h"

void MisCheck(char plr, char mpad);
int StepSafety(const struct MisEval &step);

extern char death;

#endif // MIS_M_H
