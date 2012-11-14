#ifndef PORT_H
#define PORT_H

#include <stdint.h>

void PortPal(char plr);
void GetMse(char plr, char fon);
void DrawSpaceport(char plr);
void Master(char plr);

extern int16_t Vab_Spot;

#endif // PORT_H
