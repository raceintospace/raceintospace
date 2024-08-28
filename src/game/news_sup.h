#ifndef NEWS_SUP_H
#define NEWS_SUP_H

int DamMod(int plr, int prog, int dam, int cost);
int SafetyMod(int plr, int prog, int type, int per);
int SaveMod(char plr, char prog);
int StealMod(int plr, int prog, int type);
int RDMod(int plr, int prog, int type, int val);

#endif // NEWS_SUP_H
