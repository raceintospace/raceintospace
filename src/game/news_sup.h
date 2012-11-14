#ifndef NEWS_SUP_H
#define NEWS_SUP_H

int DamMod(int p, int prog, int dam, int cost);
int NMod(int p, int prog, int type, int per);
int SaveMods(char p, char prog);
int Steal(int p, int prog, int type);
int RDMods(int p, int prog, int type, int val);

#endif // NEWS_SUP_H
