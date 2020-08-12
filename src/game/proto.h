/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* XXX clean this up */
#include <mmfile.h>
#include <data.h>
// ***************************
//   USEFUL AND RARE DEFINES
// ***************************

#define MR grMoveRel
#define LR grLineRel
#define PP grPutPixel
#define LT grLineTo
#define MT grMoveTo
#define SC grSetColor
#define BUFFER_SIZE 20*1024   /**< Allocated Buffer in Beginning */
#define MINSAFETY 50          /**< Minimum Safety */
#define READYFACTOR 48        /**< Ready Safetyfactor */
#define MAXBUB 30             /**< Maximum Bubbles */
#define pline(a,b,c,d)        {grMoveTo(a,b) ; grLineTo(c,d);}
#define other(a)	      abs( (a)-1 )
#define maxx(a,b)	      (((a) > (b)) ? (a) : (b))
#define minn(a,b)	      (((a) < (b)) ? (a) : (b))
#define MisStep(a,b,c) PrintAt((a),(b),S_Name[(c)])
//#define VBlank() while(!(inp(0x3da)&0x08))  // the all important VBlank
#define VBlank() do{}while(0)
#define PROGRAM_NOT_STARTED	-1	/* Hardware program not started when Num is set to this */

#define isMile(a,b)   Data->Mile[a][b]

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif

#define xMODE_HIST 0x0001
#define xMODE_CHEAT 0x0002
#define xMODE_NOFAIL 0x0020
#define xMODE_TOM 0x0040
#define xMODE_DOANIM 0x0080
#define xMODE_NOCOPRO 0x00800
#define xMODE_PUSSY 0x0400
#define xMODE_CLOUDS 0x1000
#define xMODE_SPOT_ANIM 0x2000

#define HIST    (xMODE&xMODE_HIST)
#define CHEAT   (xMODE&xMODE_CHEAT)
#define NOFAIL  (xMODE&xMODE_NOFAIL)
#define TOM     (xMODE&xMODE_TOM)
#define DOANIM  (xMODE&xMODE_DOANIM)
#define NOCOPRO (xMODE&xMODE_NOCOPRO)
#define PUSSY   (xMODE&xMODE_PUSSY)

// SPOT ANIMS / CLOUDS RESERVE 0xf000  high nibble

#define UP_ARROW  	0x4800
#define DN_ARROW	   0x5000
#define RT_ARROW	   0x4D00
#define LT_ARROW	   0x4B00
#define K_HOME		   0x4700
#define K_END		   0x4F00
#define K_ENTER		0x000D
#define K_QUIT		   0x000F
#define K_ESCAPE	   0x001B
#define K_SPACE		0x0020

#define ME_FB        2
#define VE_FB        3
#define MA_FB        4
#define JU_FB        5
#define SA_FB        6

#define MILE_OSAT    0
#define MILE_MAN     1
#define MILE_EORBIT  2
#define MILE_LFLY    3
#define MILE_LPL     4
#define MILE_LPASS   5
#define MILE_LORBIT  6
#define MILE_LLAND   7

#define MANSPACE     27
#define EORBIT       18
#define LPASS        19
#define LORBIT       20
#define LRESCUE      21
#define LLAND        22
#define ORBSAT       0
#define LUNFLY       1
#define PROBELAND    7

#define DUR_F 8
#define DUR_E 9
#define DUR_D 10
#define DUR_C 11
#define DUR_B 12
#define DUR_A -1    // No such experience

#define CAP1 13
#define CAP2 14
#define CAP3 15
#define CAPMS 16
#define CAP4 17

#define OLAB 23
#define DOCK 24
#define WOMAN 25
#define EWALK 26
#define LWALK 40

#define MAX_X	320
#define MAX_Y	200

// ***************************
//   PROTOTYPES FOR ROUTINES
// ***************************

// ADMIN.C
void Admin(char plr);
int GenerateTables(SaveGameType saveType);
void FileAccess(char mode);
char GetBlockName(char *Nam);
void DrawFiles(char now,char loc,char tFiles);
void BadFileType(void);
void FileText(char *name);
int FutureCheck(char plr,char type);
char RequestX(char *s,char md);
#ifdef DEAD_CODE
void SaveMail(void);
#endif
void EndOfTurnSave(char *inData, int dataLen); 	// Create ENDTURN.TMP

// AIMAST.C
void AIMaster(char plr);
char NoFail(char plr);
void RDPrograms(char plr);
void CheckVoting(char plr);
void KeepRD(char plr,int m);
void PickModule(char plr);
void MoonVoting(char plr);
#ifdef DEAD_CODE
int CheckLimit(char plr,char m);
#endif
int CheckSafety(char plr,char m);
void MoonProgram(char plr,char m);
void ProgramVoting(char plr);


// AIMIS.C
void AIVabCheck(char plr,char mis,char prog);
char Best(void);
int ICost(char plr,char h,char i);
void CalcSaf(char plr,char vs);
char Panic_Level(char plr,int *m_1,int *m_2);
void NewAI(char plr,char frog);
void AIFuture(char plr,char mis,char pad,char *prog);
#ifdef DEAD_CODE
char Search(char plr,char m);
void GetHard(char plr);
char BoostCheck(char plr);
void Scheduler(char plr);
#endif
void AILaunch(char plr);

// AIPUR.C
void DrawStatistics(char Win);
void Stat(char Win);
void AIAstroPur(char plr);
void SelectBest(char plr,int pos);
void DumpAstro(char plr,int inx);
char Skill(char plr,char type);
void TransAstro(char plr,int inx);
void CheckAdv(char plr);
#ifdef DEAD_CODE
void RemoveUnhappy(char plr);
#endif
void RDafford(char plr,int Class,int index);
#ifdef DEAD_CODE
int CheckMax(char plr,int m);
char SF(char plr,char m);
char SN(char plr,char m);
#endif
void AIPur(char plr);
int GenPur(char plr,int hardware_index,int unit_index);

// AST0.C
void Moon(char plr);
void DispLeft(char plr,int lc,int cnt,int nw,int *ary);
void BarSkill(char plr,int lc,int nw,int *ary);
void SatDraw(char plr);
void LMDraw(char plr);
void SatText(char plr);
void PlanText(char plr,char plan);
void LMPict(char poff);
void LMBld(char plr);
void SatBld(char plr);


// AST1.C
void DispEight(char now,char loc);
void DispEight2(int nw,int lc,int cnt);
void DrawAstCheck(char plr);
void DrawAstSel(char plr);
void AstSel(char plr);


// AST2.C
void DrawLimbo(char plr);
void Limbo(char plr);
void Clear(void);
void LimboText(char plr,int astro);


// AST3.C
void DrawTrain(char plr,char lvl);
void TrainText(char plr,int astro,int cnt);
void Train(char plr,int level);
void Hospital(char plr,int sel);


// AST4.C
#ifdef DEAD_CODE
int ALSpec(int att);
#endif
void AstLevel(char plr,char prog,char crew,char ast);
void PlaceEquip(char plr,char prog);
void DrawProgs(char plr,char prog);
int CheckProgram(char plr,char prog);
void FixPrograms(char plr);
void DamProb(char plr,char prog,int chk);
#ifdef DEAD_CODE
void CrewProb(char plr,char prog);
#endif
void DrawPosition(char prog,int pos);
void Programs(char plr,char prog);
void ClearIt(void);
void NewAstList(char plr,char prog,int M1,int M2,int M3,int M4);
void AstStats(char plr,char man,char num);
void AstNames(int man,char *name,char att);
void Flts(char old,char nw);
void FltsTxt(char nw,char col);


// BUDGET.C
void DrawBudget(char player,char *pStatus);
void BudPict(char poff);
void DrawPastExp(char player,char *pStatus);
void Budget(char player);
void DrawViewing(char plr);
void DrawVText(char got);
int RetFile(char plr,int card);
void Viewing(char plr);
void DrawPreviousMissions (char plr);



// CREW.C
int HardCrewAssign(char plr,char Pad,int MisType,char NewType);
void ClrFut(char plr,char pad);
int AsnCrew(char plr,char pad,char part);
void FutFltsTxt(char nw,char col);
void FutSt(char plr,int pr,int p,int b);
void FutSt2(int num,int type);
void FutAstList(char plr,char men,int M1,int M2,int M3,int M4);
void DrawHard(char mode,char pad,char mis,char plr);
int HardRequest(char plr,char mode,char mis,char pad);
int SecondHard(char plr,char mode,char mis,char pad);


// ENDGAME.C
char Burst(char win);
void EndGame(char win,char pad);
void Load_LenFlag(char win);
void Draw_NewEnd(char win);
void NewEnd(char win,char loc);
void FakeWin(char win);
void FakeHistory(char plr,char Fyear);
void HistFile(char *buf,unsigned char bud);
void PrintHist(char *buf);
void PrintOne(char *buf,char tken);
void AltHistory(char plr);
void SpecialEnd(void);
void EndPict(int x,int y,char poff,unsigned char coff);
void LoserPict(char poff,unsigned char coff);
void PlayFirst(char plr,char first);


// FUTBUT.C
void drawBspline(int segments,char color,...);
void Draw_IJ(char w);
void Draw_GH(char a,char b);
void Draw_IJV(char w);
void OrbIn(char a,char b,char c);
void OrbMid(char a,char b,char c,char d);
void Q_Patch(void);
void OrbOut(char a,char b,char c);
void LefEarth(char a,char b);
void LefOrb(char a,char b,char c,char d);
void Fly_By(void);
void VenMarMerc(char x);
void Draw_PQR(void);
void Draw_PST(void);
void Draw_LowS(char a,char b,char c,char x,char y,char z);
void Draw_HighS(char x,char y,char z);
void RghtMoon(char x,char y);
void DrawLunPas(char x,char y,char z,char w);
void DrawLefMoon(char x,char y);
void DrawMoon(char x,char y,char z,char w,char j,char k,char l);
void DrawZ(void);
void DrawSTUV(char x,char y,char z,char w);
void LefGap(char x);
void S_Patch(char x);

// FUTURE.C
void Load_FUT_BUT(void);
void DrawFuture(char plr,int mis,char pad);
void ClearDisplay(void);
int GetMinus(char plr);
void SetParameters(void);
void DrawLocks(void);
void Toggle(int wh,int i);
void TogBox(int x,int y,int st);
void PianoKey(int X);
void draw_Pie(int s);
void PlaceRX(int s);
void ClearRX(int s);
int UpSearchRout(int num,char plr);
int DownSearchRout(int num,char plr);
void Future(char plr);
void Bd(int x,int y);
void DurPri(int x) ;
void MissionName(int val,int xx,int yy,int len);
void Missions(char plr,int X,int Y,int val,char bub);
#ifdef DEAD_CODE
char FutBad(void);
#endif

// HARDEF.C
void DrawHardef(char plr);
void ShowHard(char plr);
void HDispIt(int x1,int y1,int x2, int y2,int s,int t);
void PInfo(char plr,char loc);
void HInfo(char plr,char loc,char w);
void RankMe(char plr);
void DrawRank(char plr);
int CalcScore(char plr,char lvA,char lvB);

// INTEL.C
void Intel(char plr);
void MisIntel(char plr,char acc);
void XSpec(char plr,char mis,char year);
void Special(char p,int ind);
void BackIntel(char p,char year);
void HarIntel(char p,char acc);
void TopSecret(char plr,char poff);
void SaveIntel(char p,char prg,char ind);
#ifdef DEAD_CODE
void Bad(char plr,char pt);
#endif
void ImpHard(char plr,char hd,char dx);
void UpDateTable(char plr);
void IntelPhase(char plr,char pt);
void DrawBre(char plr);
void Bre(char plr);
void Load_CIA_BUT(void);
void DrawIStat(char plr);
void ReButs(char old,char nw);
void IStat(char plr);
void DispIt(int x1,int y1,int x2, int y2,int s,int t);
void IInfo(char plr,char loc,char w);

// INTRO.C
void Credits(void);
void Introd(void);
void NextTurn(char plr);


// MAIN.C
void Rout_Debug(int line, char *file);
void RestoreDir(void);
int main(int argc, char *argv[]);
int CheckIfMissionGo(char plr,char launchIdx);
void oclose(int fil);
void InitData(void);
void MMainLoop(void);
void Progress(char mode);
void MainLoop(void);
void DockingKludge(void);
void OpenEmUp(void);
void CloseEmUp(unsigned char error,unsigned int value);
void FreePadMen(char plr,struct MissionType *XMis);
void DestroyPad(char plr,char pad,int cost,char mode);
void GV(GXHEADER *obj,int w,int h);
void DV(GXHEADER *obj);
void GetMouse(void);
void WaitForMouseUp(void);
void WaitForKeyOrMouseDown(void) ;
void PauseMouse(void);
void PrintAt(int x,int y,char *s);
void PrintAtKey(int x,int y,char *s,char val);
#ifdef DEAD_CODE
void DrawLED(int x,int y,char st);
#endif
void DispBig(int x,int y,char *txt,char mode,char te);
void DispNum(int xx,int yy,int num);
void DispMB(int x,int y,int val);
#ifdef DEAD_CODE
void Gl(int x1,int x2,int y,char t);
#endif
void ShBox(int x1,int y1,int x2,int y2);
void UPArrow(int x1,int y1);
void RTArrow(int x1,int y1);
void LTArrow(int x1,int y1);
void DNArrow(int x1,int y1);
void InBox(int x1,int y1,int x2,int y2);
void OutBox(int x1,int y1,int x2,int y2);
void IOBox(int x1, int y1, int x2, int y2);
void RectFill(int x1,int y1,int x2,int y2,char col);
void Box(int x1,int y1,int x2,int y2);
void GradRect(int x1,int y1,int x2,int y2,char plr);
void FlagSm(char plr,int xm,int ym);
void Flag(int x,int y,char plr);
void DispChr(char chr);
void VerifySF(char plr)  ;
void VerifyCrews(char plr);
void GetMisType(char mcode);
int MisRandom(void);


// MC.C
void VerifyData(void);
void DrawControl(char plr);
void SetW(char ch);
int Launch(char plr,char mis);
void MissionPast(char plr,char pad,int prest);
int MaxFailPad(char which);


// MC2.C
void MissionCodes(char plr,char val,char pad);
void MissionParse(char plr,char *MCode,char *LCode,char pad);
char WhichPart(char plr,int which);
void MissionSteps(char plr,int mcode,int Mgoto,int step,int pad);
void MisPrt(void);
void MissionSetup(char plr,char mis);
void MissionSetDown(char plr,char mis);
void MisDur(char plr,char dur);
void MisSkip(char plr,char ms);
void MisRush(char plr, char rush_level);

// MIS_C.C
void RLEE (void *dest, void *src, unsigned int src_size);
void PlaySequence(char plr,int step,char *Seq,char mode);
void DoPack(char plr,FILE *ffin,char mode,char *cde,char *fName);
void InRFBox(int a, int b, int c, int d, int col);
void GuyDisp(int xa,int ya,struct Astros *Guy);
char FailureMode(char plr,int prelim,char *text);
FILE *OpenAnim(char *fname);
int CloseAnim(FILE * fin);
int StepAnim(int x,int y,FILE *fin);
char DrawMoonSelection (char nauts,char plr);
void FirstManOnMoon (char plr, char isAI, char misNum);
int RocketBoosterSafety(int safetyRocket, int safetyBooster);

// MIS_M.C
void MevDebug(FILE *fout,char i);
void GetFailStat(struct XFails *Now,char *FName,int rnum);
void MisCheck(char plr,char mpad);
int MCGraph(char plr,int lc,int safety,int val,char prob);
void F_KillCrew(char mode,struct Astros *Victim);
void F_IRCrew(char mode,struct Astros *Guy);
int FailEval(char plr,int type,char *text,int val,int xtra);
void DebugSetFailure(char *Text);


// MUSEUM.C
void Museum(char plr);
void ShowPrest(char plr);
void Move2(char plr, char *pos, char *pos2, char val );
void BackOne(char plr, char *pos,char *pos2);
void ForOne(char plr, char *pos,char *pos2);
void DPrest(char plr,char *pos,char *pos2);
void ShowSpHist(char plr);
void Mission_Data_Buttons(char plr, int *where);
void Draw_Mis_Stats(char plr, char index, int *where, char mode);
void FastOne(char plr,int *where);
void FullRewind(char plr, int *where);
void RewindOne(char plr, int *where);
void FullFast(char plr, int *where);
void DrawMisHist(char plr,int *where);
void ShowAstrosHist(char plr);
void DisplAst(char plr, char *where, char *where2);
void ShowAstroUp(char plr,char *where,char *where2);
void ShowAstroDown(char plr, char *where,char *where2);
void ShowAstroBack(char plr, char *where,char *where2);
void ShowAstroFor(char plr, char *where,char *where2);
void DispLoc(char plr,char *where);
void DisplAstData(char plr, char *where,char *where2);
void DownAstroData(char plr, char *where,char *where2);
void UpAstroData(char plr,char *where,char *where2);
int astcomp(const void *no1, const void *no2);

// NEWMIS.C
char OrderMissions(void);
void MisOrd(char num);
void MisAnn(char plr,char pad);
void AI_Begin(char plr);
void AI_Done(void);

// NEWS.C
#ifdef DEAD_CODE
void NFrame(int x1, int y1, int x2, int y2);
#endif
void GoNews(char plr);
void OpenNews(char plr,char *buf,int bud);
void DispNews(char plr,char *src,char *dest);
FILE* PreLoadAnim(char plr,char mode);
void CloseNewsAnim(void);
#ifdef DEAD_CODE
void DrawNews(char plr);
#endif
void DrawNText(char plr,char got);
void News(char plr);
void DeAlloc(BYTE Page);
void AIEvent(char plr);
char ResolveEvent(char plr);
#ifdef DEAD_CODE
void Breakgrp(char plr);
#endif
int PlayNewsAnim(mm_file *);
mm_file *LoadNewsAnim(int plr, int bw, int type, int Mode, mm_file * fp);
void ShowEvt(char plr,char crd);


// NEWS_SUP.C
int Steal(int p,int prog,int type);
int NMod(int p,int prog,int type,int per);
int DamMod(int p,int prog,int dam,int cost);
int RDMods(int p,int prog,int type,int val);
int SaveMods(char p,char prog);

// NEWS_SUQ.C
char CheckCrewOK(char plr,char pad);
char REvent(char plr);
void Replace_Snaut(char plr);

// PACE.C
int PCX_D (void *src,void *dest,unsigned src_size);
long RLEC (char *src, char *dest, unsigned int src_size);
void FadeIn(char wh,char *palx,int steps,int val,char mode);
void FadeOut(char wh,char *palx,int steps,int val,char mode);
int RLED (void *src, void *dest, unsigned int src_size);

// PLACE.C
void BCDraw(int y);
int MChoice(char qty,char *Name);
int BChoice(char plr,char qty,char *Name,char *Imx);
void PatchMe(char plr,int x,int y,char prog,char poff,unsigned char coff);
void AstFaces(char plr,int x,int y,char face);
void SmHardMe(char plr,int x,int y,char prog,char planet,unsigned char coff);
void BigHardMe(char plr,int x,int y,char hw,char unit,char sh,unsigned char coff);
void DispHelp(char top, char bot, char *txt);
int Help(char *FName);
void Draw_Mis_Stats(char plr, char index, int *where,char mode);
void writePrestigeFirst(char index);

// PORT.C
void Seek_sOff(int where);
void Seek_pOff(int where);
void SpotCrap(char loc,char mode);
void WaveFlagSetup(void);
void WaveFlagDel(void);
void PadBub(int x,int y,int col);
void PortPlace(FILE *fin,long table);
void PortPal(char plr);
void DrawSpaceport(char plr);
void PortText(int x,int y,char *txt,char col);
void UpdatePortOverlays(void);
void Master(char plr);
void GetMse(char plr,char fon);
void DoCycle(void);
void PortOutLine(unsigned int Count,uint16_t *buf,char mode);
void PortRestore(unsigned int Count);
int MapKey(char plr,int key,int old) ;
void Port(char plr);
char PortSel(char plr,char loc);
char Request(char plr,char *s,char md);
char MisReq(char plr);

// PROTOCOL.C
void Read_CRC(void);
void SaveSide(char side);
void UpdPrestige();
void RecvSide(char side);
//static int timed_get(int n);
//static int build_block(int l, FILE *file);
//static void abort_transfer(void);
int xmit_file(int (*error_handler)(int c, long p, char *s), char *files[]);
//static int getblock(int block, int crc);
int recv_file(int(*error_handler)(int c, long p, char *s), char *path);

// PREFS.C
void DrawPrefs(int where,char a1,char a2);
void HModel(char mode,char tx);
void Levels(char plr,char which,char x);
void BinT(int x,int y,char st);
void PLevels(char side,char wh);
void CLevels(char side,char wh);
void Prefs(int where);
void EditAst(void);
void ChangeStat(char mum,char Cur,char Cnt);
void DrawStats(char mum,char chce);
void First(char now,char loc);
void Second(int nw,int lc);

// PREST.C
void Set_Dock(char plr, char total);
void Set_LM(char plr, char total);
int Check_Photo(void);
int Check_Dock(int limit);
char PrestMin(char plr);
int PrestCheck(char plr);
char Was_Goal(char total,char which);
char Set_Goal(char plr,char which,char control);
char Did_Fail(void);
int MaxFail(void);
char PosGoal(char *PVal);
char NegGoal(char *PVal);
char SupGoal(char *PVal);
int PrestNeg(char plr,int i);
int AllotPrest(char plr,char mis);
char PosGoal_Check(char *PVal);
int Find_MaxGoal(void);
int U_AllotPrest(char plr,char mis);

// RADAR.C
void PadDraw(char plr,char pad);
void ClrMiss(char plr,char pad);
void PadPict(char poff);
void ShowPad(char plr,char pad);

// RANDOMIZE.C
void RandomizeEq(void);
void SetEquipName (char plr);
char GetNewName (char name[20]);

// RDPLEX.C
void SRPrintAt(int x, int y, char *text,char fgd,char bck);
void Load_RD_BUT(char plr);
void Del_RD_BUT(void);
void DrawRD(char plr);
void BButs(char old,char nw);
void RDButTxt(int v1,int val,char plr, char SpDModule);  //DM Screen, Nikakd, 10/8/10
char RD(char plr);
void ManSel(int activeButtonIndex);
char QueryUnit(char hardware_index,char unit_index,char plr);
char MaxChk(char hardware_index,char unit_index,char plr);
uint8_t RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls, char playerIndex);
void ShowUnit(char hw,char un,char plr);
void OnHand(char qty);
void DrawHPurc(char plr);
char HPurc(char plr);
void BuyUnit(char hw2,char un2,char plr);

// RECORDS.C
void SafetyRecords(char plr,int temp);
void MakeRecords(void);
void Records(char plr);
void Move2rec(char *pos, char *pos2, char val );
void ClearRecord(char *pos2);
void Back1rec(char *pos,char *pos2);
void For1rec(char *pos,char *pos2);
void Drec(char *pos,char *pos2,char mde);
void WriteRecord(int i,int j,int k,int temp);
void SwapRec(int Rc,int pl1,int pl2);
char CheckSucess(int i,int j);
void UpdateRecords(char Ty);

// REPLAY.C
void RLEF(void *dest, void *src, unsigned int src_size);
void DispBaby(int x, int y, int loc,char neww);
void AbzFrame(char plr,int num,int dx,int dy,int width,int height,char *Type,char mode);
void Replay(char plr,int num,int dx,int dy,int width,int height,char *Type);

// REVIEW.C
void DrawReview(char plr);
void Review(char plr);
void MisRev(char plr,int pres);
void PresPict(char poff);
void CalcPresRev(void);
void DrawRevText(char plr, int val);

// RUSH.C
void DrawRush(char plr);
void Rush(char plr);
void ResetRush(int mode,int val);
void SetRush(int mode,int val);

// SEL.C
void DFrame(int x1, int y1, int x2, int y2);
void SFrame(int x1, int y1, int x2, int y2);
void SFWin(int x1, int y1, int x2, int y2,char *txt);

// SOUND.C
void GetVoice( char val);
void UpdateVoice(void);
void NGetVoice(char plr,char val);
void NUpdateVoice(void);
char AnimSoundCheck(void);
void U_GetVoice( char val);
void U_UpdateVoice(void);
void KillVoice(void);
void FadeVoiceIn( int maxvolume, unsigned rate);
void FadeVoiceOut( unsigned rate);
void SetVoiceVolume(int percent);
void PlayVoice(void);
void StopVoice(void);
void *load_global_timbre( FILE *GTL, unsigned bank, unsigned patch);
void GetMusic(FILE *fin,long size)  ;
void KillMusic(void);
void FadeMusicIn(int maxvolume, unsigned rate);
void FadeMusicOut(unsigned rate);
void PlayMusic(char mode);
void UpdateMusic(void);
void StopMusic(void);
int board_ID(drvr_desc *desc);
void PreLoadMusic(char val);
void Buzz_SoundDestroy(void);
unsigned int ABXframe(char *scratch,char *dest,int frameno,FILE *fhand);
int GetBuzzEnv(drvr_desc *desc,char *envs);
void PlayAudio(char *name,char mode);
void UpdateAudio(void);
void StopAudio(char mode);
void MuteChannel(int channel, int mute);
int IsChannelMute(int channel);


// START.C
void SetEvents(void);
void AstroTurn(void);
void Update(void);
void UpdAll(char side);
void TestFMis(int j,int i);
void UpdateHardTurn(char plr);

// VAB.C
int ChkDelVab(char plr,char f);
int ChkVabRkt(char plr,int rk,int *q);
void GradRect2(int x1,int y1,int x2,int y2,char plr);
void DispVAB(char plr,char pad);
int FillVab(char plr,char f,char mode);
int  BuyVabRkt(char plr,int rk,int *q,char mode);
void ShowVA(char f);
void ShowRkt(char *Name,int sf,int qty,char mode, char isDmg);
void DispVA(char plr,char f);
void DispRck(char plr,char wh);
void DispWts(int two,int one);
void VAB(char plr);
void BuildVAB(char plr,char mis,char ty,char pa,char pr);
void LMAdd(char plr,char prog,char kic,char part);
void VVals(char plr,char tx,Equipment *EQ,char v4,char v5);


// SERIAL_F.C
void SaveFirst(char);
void CheckFirst(char);
void UpdateFirst(void);
void SendFirst(void);
void RecvFirst(void);
int e_handler(int c, long p, char *s);
void SaveFirst(char mode);
void SendFirst(void);
void UpdateFirst(void);
void CheckFirst(char Hmode);
void RecvFirst(void);
int port_exist(int port);
void open_port(int port, int inlen);
void close_port(void);
void purge_in(void);
void set_baud(long baud);
long get_baud(void);
int get_bits(void);
int get_parity(void);
int get_stopbits(void);
void set_data_format(int bits, int parity, int stopbit);
void set_port(long baud, int bits, int parity, int stopbit);
int in_ready(void);
int carrier(void);
void set_dtr(int n);

// CDROM.C
int CDAccess(int drive,int track,char mode);
int AquireDrive(void);

// TRACK.C
void Message(int msg, int tr);
void AAset(void);
void DialAt(int x,int y,char *s);
void get_dial(char *old);
void Read_Config(void);
int e_handler(int c, long p, char *s);
void Dnload(void);
void Upload(void);
void ReadMsg(void);
void WriteMsg(void);
void Write_Config(void);
void DrawCenter();
void MesCenter(void);
char MPrefs(char mode);
void Toggle_Cfg(int opt,int old);
void TrackPict(char mode);

// MODEM.CPP
char dial(void);
char CheckCarrier(void);
void Modem_Config(void);
char *get_modem_string(char *s, char *d);
char DoModem(int sel);
void exit_program(void);
void hangup(void);
void modem_control_string(char *s);

void save_game (char *filename);

// EOF
