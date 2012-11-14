#ifndef AIPUR_H
#define AIPUR_H

void AIAstroPur(char plr);
void AIPur(char plr);
void DumpAstro(char plr, int inx);
int GenPur(char plr, int hardware_index, int unit_index);
void RDafford(char plr, int Class, int index);
void Stat(char Win);
void TransAstro(char plr, int inx);

extern struct ManPool *Men;

#endif // AIPUR_H
