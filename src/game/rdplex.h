#ifndef RDPLEX_H
#define RDPLEX_H

#include <cstdint>

char RD(char plr);
char QueryUnit(char hardware_index, char unit_index, char plr);
char MaxChk(char hardware_index, char unit_index, char plr);
uint8_t RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls, char playerIndex);
char HPurc(char plr);

#endif // RDPLEX_H
