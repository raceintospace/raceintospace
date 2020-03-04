#ifndef INTEL_H
#define INTEL_H

#include <boost/shared_ptr.hpp>

namespace display
{
class LegacySurface;
}

void Intel(char plr);
void IntelPhase(char plr, char pt);
boost::shared_ptr<display::LegacySurface> LoadCIASprite();

#endif // INTEL_H
