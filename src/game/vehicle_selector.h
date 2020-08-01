#ifndef VEHICLE_SELECTOR_H
#define VEHICLE_SELECTOR_H

#include "vehicle.h"


/**
 * This class cycles through the available launch vehicle options,
 * composed of a rocket and optional boosters.
 */
class VehicleSelector
{
public:
    VehicleSelector(int plr) :  mIndex(0), mPlayer(plr) {};
    Vehicle start();
    Vehicle next();
private:
    int mIndex;
    const int mPlayer;
};


Vehicle VehicleSelector::start()
{
    mIndex = 0;
    return next();
}


Vehicle VehicleSelector::next()
{
    bool boosters = mIndex >= 4;
    int rkIndex = mIndex % 4;

    // By having mIndex track the next vehicle to be used, rather than the
    // current one, there is no need to initially call start()
    if (++mIndex >= 7) {
        mIndex = 0;
    }

    return Vehicle(mPlayer, rkIndex, boosters);
}


#endif // VEHICLE_SELECTOR_H
