#ifndef NEWS_SUP_H
#define NEWS_SUP_H

int DamMod(int plr, int prog, int dam, int cost);
int SafetyMod(int plr, int prog, int type, int per);
int SaveMod(int plr, int prog);
int StealMod(int plr, int prog, int type);
int RDMod(int plr, int prog, int type, int val);
void NewNauts(int plr);

#endif // NEWS_SUP_H
