#ifndef MC_H
#define MC_H

#include "data.h"

int Launch(char plr, char mis);

extern struct mStr Mis;
extern Equipment *MH[2][8];
extern struct MisAst MA[2][4];
extern struct MisEval Mev[60];
extern struct MXM *AList;
extern REPLAY Rep;
extern char MANNED[2];
extern char CAP[2];
extern char LM[2];
extern char DOC[2];
extern char EVA[2];
extern char STEP;
extern char FINAL;
extern char JOINT;
extern char PastBANG;
extern char DMFake;
extern char fEarly;
extern char mcc;
extern char hero;

#endif // MC_H
