#ifndef AST0_H
#define AST0_H

void BarSkill(char plr, int lc, int nw, int *ary);
void DispLeft(char plr, int lc, int cnt, int nw, int *ary);
void LMBld(char plr);
int MoodColor(uint8_t mood);
void Moon(char plr);
void SatBld(char plr);

extern char MCol[110];
extern char sel[30];
extern char MaxSel;

#endif // AST0_H
