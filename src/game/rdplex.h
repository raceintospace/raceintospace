#ifndef RDPLEX_H
#define RDPLEX_H

#include "gxdatatypes.h"

char RD(char plr);
void Load_RD_BUT(char plr);
void Del_RD_BUT(void);
char QueryUnit(char hardware_index, char unit_index, char plr);
char MaxChk(char hardware_index, char unit_index, char plr);
uint8_t RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls, char playerIndex);
char HPurc(char plr);

extern display::LegacySurface *but;

#endif // RDPLEX_H
