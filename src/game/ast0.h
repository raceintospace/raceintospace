#ifndef AST0_H
#define AST0_H

#include "data.h"

void BarSkill(char plr, int lc, int nw, int *ary);
void DispLeft(char plr, int lc, int cnt, int nw, const int *ary);
void LMBld(char plr);
int MoodColor(uint8_t mood);
void Moon(char plr);
void SatBld(char plr);

extern uint8_t MCol[MAX_ROSTER];
extern uint8_t sel[MAX_SELECTION];
extern uint8_t MaxSel;

#endif // AST0_H
