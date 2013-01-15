#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include <string>

namespace display
{
class LegacySurface;
};

void WaitForMouseUp(void);
void GetMisType(char mcode);
void GetMouse(void);
void WaitForKeyOrMouseDown(void) ;
void VerifySF(char plr)  ;
int MisRandom(void);
void DestroyPad(char plr, char pad, int cost, char mode);
void PauseMouse(void);
void GetMouse_fast(void);

extern char Option;
extern char MAIL;
extern int fOFF;
extern char AI[2];
extern char manOnMoon;
extern char dayOnMoon;
extern int SEG;
extern display::LegacySurface *vhptr;
extern display::LegacySurface *vhptr2;
extern bool fullscreenMissionPlayback;
extern char pNeg[NUM_PLAYERS][MAX_MISSIONS];
extern unsigned char AL_CALL;
extern std::string helpText;
extern std::string keyHelpText;
extern int oldx;
extern int oldy;
extern unsigned char HARD1;
extern unsigned char UNIT1;
extern unsigned char BUTLOAD;
extern unsigned char LOAD;
extern unsigned char QUIT;
extern unsigned char FADE;
extern char plr[NUM_PLAYERS];
extern struct Players *Data;
extern int x;
extern int y;
extern int mousebuttons;
extern int key;
extern char Name[20];
extern char *buffer;
extern int32_t xMODE;
extern char *S_Name[];

#endif // GAME_MAIN_H
