#ifndef NEWS_SUP_H
#define NEWS_SUP_H

int DamMod(int plr, int prog, int dam, int cost);
int NMod(int plr, int prog, int type, int per);
int SaveMods(char plr, char prog);
int Steal(int plr, int prog, int type);
int RDMods(int plr, int prog, int type, int val);
void NewNauts(int plr);

#endif // NEWS_SUP_H
