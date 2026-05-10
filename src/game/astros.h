#ifndef ASTROS_H
#define ASTROS_H

#include "data.h"

int AvailableCrewsCount(int plr, int program);
void CheckFlightCrews(int player);
bool Compatible(const Astros& ast, const Astros& peer);

#endif // ASTROS_H
