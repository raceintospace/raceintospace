#ifndef RIS_SPOT_H
#define RIS_SPOT_H


#define BABYSND 1
#define SPOT_LOAD 0
#define SPOT_STEP 1
#define SPOT_DONE 2
#define SPOT_KILL 3


void SpotActivity(char loc, char mode);
void SpotClose();
void SpotInit();
void SpotPause();
void SpotRefresh();
void SpotResume();


#endif // RIS_SPOT_H
