#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include "gxdatatypes.h"

void InBox(int x1, int y1, int x2, int y2);
void OutBox(int x1, int y1, int x2, int y2);
void WaitForMouseUp(void);
void DispNum(int xx, int yy, int num);
void DispMB(int x, int y, int val);
void PrintAt(int x, int y, const char *s);
void RectFill(int x1, int y1, int x2, int y2, char col);
void GetMisType(char mcode);
void DV(GXHEADER *obj);
void GetMouse(void);
void GV(GXHEADER *obj, int w, int h);
void FlagSm(char plr, int xm, int ym);
void DispBig(int x, int y, const char *txt, char mode, char te);
void IOBox(int x1, int y1, int x2, int y2);
void ShBox(int x1, int y1, int x2, int y2);
void DispChr(char chr);
void GradRect(int x1, int y1, int x2, int y2, char plr);
void UPArrow(int x1, int y1);
void DNArrow(int x1, int y1);
void RTArrow(int x1, int y1);
void LTArrow(int x1, int y1);
void Box(int x1, int y1, int x2, int y2);
void WaitForKeyOrMouseDown(void) ;
void Flag(int x, int y, char plr);
void VerifySF(char plr)  ;
int MisRandom(void);
void DestroyPad(char plr, char pad, int cost, char mode);
void PauseMouse(void);
void PrintAtKey(int x, int y, const char *s, char val);
void GetMouse_fast(void);

extern char Option;
extern char MAIL;
extern int fOFF;
extern char AI[2];
extern char manOnMoon;
extern char dayOnMoon;
extern int SEG;
extern GXHEADER vhptr;
extern GXHEADER vhptr2;
extern char BIG;
extern char pNeg[NUM_PLAYERS][MAX_MISSIONS];
extern unsigned char AL_CALL;
extern char helptextIndex[5];
extern char keyhelpIndex[5];
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