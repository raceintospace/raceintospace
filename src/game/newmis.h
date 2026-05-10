#ifndef NEWMIS_H
#define NEWMIS_H

void MisAnn(char plr, char pad);
void AI_Begin(char plr);
void AI_Done();
char OrderMissions();

extern char Month[12][11];
extern struct order Order[7];

#endif // NEWMIS_H
