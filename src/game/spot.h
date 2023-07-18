#ifndef RIS_SPOT_H
#define RIS_SPOT_H


#define BABYSND 1


enum SpotAnimations {
    SPOT_NONE = -1,
    USA_LM_TEST = 0,
    USA_PLANE_FLY_BY = 1,
    USA_HELICOPTER = 2,
    USA_LM_CRASH = 3,
    USA_ROCKET_TO_PAD = 4,
    SOV_LM_TEST = 5,
    SOV_PLANE_FLY_BY = 6,
    SOV_HELICOPTER = 7,
    SOV_LM_CRASH = 8,
    // SOV_ROCKET_TO_THE_PAD = 9, (open/unused spot)
    SOV_GATE = 10,  // SOV Gate Open/Close
    USA_TRACKING = 11,
    SOV_STORM_CLOUDS = 12,
    SOV_SMALL_VAB_DOORS = 13,  // Not used & audio absent...
    USA_STORM_CLOUDS = 14,
    USA_ROCKET_TO_VAB = 15,  // USA_Door_and_Rocket_Delivery_Truck
    SOV_ROCKET_TO_PAD = 16,
    SOV_TRACKING = 17,
    SOV_NEW_PLANE = 18,
    USA_ROTATING_CRANE = 19  // VAB crane
};


void SpotAdvance();
void SpotClose();
void SpotInit();
void SpotKill();
void SpotLoad(int animationIndex);
void SpotPause();
void SpotRefresh();
void SpotResume();


#endif // RIS_SPOT_H
