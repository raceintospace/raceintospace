#ifndef HARDWARE_H
#define HARDWARE_H

#include "data.h"

int CrewSize(const Equipment &program);
Equipment &HardwareProgram(int player, int type, int program);
MissionHardwareType HardwareType(const Equipment &program);
int RocketBoosterSafety(int safetyRocket, int safetyBooster);


#endif // HARDWARE_H
