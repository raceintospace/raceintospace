#ifndef VEHICLE_H
#define VEHICLE_H

#include <list>
#include <string>

#include "data.h"


/**
 * This class functions as an interface between the player's data
 * structures and the VAB.
 */
class Vehicle
{
public:
    Vehicle(int player, int rocketIndex, bool hasBoosters = false);
    void assignTo(int pad);
    int available() const;
    bool damaged() const;
    bool delayed() const;
    int index() const
    {
        return mIndex;
    };
    std::string name() const;
    std::list<Equipment *> needed();
    void release();
    int safety() const;
    bool started() const;
    int thrust() const;
    bool translunar() const;
private:
    int mIndex;
    int mPlayer;
    Equipment *mPrimary;
    Equipment *mSecondary;

    void reserve();
};


#endif // VEHICLE_H
