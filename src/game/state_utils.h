#ifndef STATE_UTILS_H
#define STATE_UTILS_H


enum CrewType {
    CREW_PRIMARY,
    CREW_BACKUP,
    CREW_ALL
};


void ClearFutureCrew(char plr, int pad, CrewType crew);
void ClearMissionCrew(char plr, int pad, CrewType crew);
void DelayMission(char plr, int pad);
void FreeLaunchHardware(char plr, int pad);
void ScrubMission(char plr, int pad);


#endif // STATE_UTILS_H
