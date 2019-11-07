#ifndef STATE_UTILS_H
#define STATE_UTILS_H


enum CrewType {
    CREW_PRIMARY,
    CREW_BACKUP,
    CREW_ALL
};


void ClearFutureCrew(char plr, int pad, CrewType crew);
void ClearMissionCrew(char plr, int pad, CrewType crew);


#endif // STATE_UTILS_H
