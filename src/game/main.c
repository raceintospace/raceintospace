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

//****************************************************************
//*Interplay's BUZZ ALDRIN's RACE into SPACE                     *
//*                                                              *
//*Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA   *
//*Copyright 1991 by Strategic Visions, Inc.                     *
//*Designed by Fritz Bronner                                     *
//*Programmed by Michael K McCarty                               *
//*                                                              *
//****************************************************************

#include <Buzz_inc.h>
#include <options.h>
#include <utils.h>
#include <logging.h>
#include <externs.h>
#include <ctype.h>

#ifdef CONFIG_MACOSX
// SDL.h needs to be included here to replace the original main() with
// what it needs for the Mac
#include <SDL.h>
#endif

  int cdROM,hDISK;

  char Name[20];
  struct Players *Data;
  int x,y,mousebuttons,key,oldx,oldy;
  unsigned char LOAD,QUIT,HARD1,UNIT1,BUTLOAD,FADE,AL_CALL,XMAS;
  char plr[NUM_PLAYERS],IDT[5],IKEY[5],df,IDLE[2];
  char *buffer;
  GXHEADER vhptr,vhptr2;
  char * oldpal,pNeg[NUM_PLAYERS][MAX_MISSIONS];
  long xMODE;
  HTIMER server;
  char Option=-1,MAIL=-1;
  int SEG=15,FadeVal,fOFF=-1;
  extern struct mStr Mis;
  extern struct Prest_Upd MP[3];
  struct cdtable *cdt;
  long PalOff;
  ui16 LetHand;
  char BIG, 			   /**< 1 for fullscreen mission playback, 0 otherwise */
	manOnMoon=0, dayOnMoon=20;

char *S_Name[] = {
   "LAUNCH",
   "ORBITAL INS. BURN",
   "HARDWARE POWER-ON",
   "REENTRY",
   "RECOVERY/LANDING",
   "EARTH ORBITAL ACTIVITIES",
   "EARTH DE-ORBIT BURN",
   "ORBITAL EVA",
   "DOCKING",
   "TRANS-LUNAR INJECTION",
   "TRANS-EARTH INJECTION",
   "LUNAR ORBITAL INS. BURN",
   "LUNAR ORBITAL ACTIVITIES",
   "EARTH MID-COURSE ACT.",
   "LUNAR MID-COURSE ACT.",
   "LUNAR EVA",
   "LEM ACTIVITIES",
   "LUNAR DE-ORBIT BURN",
   "LUNAR LANDING",
   "LUNAR LIFTOFF",
   "PHOTO RECONNAISSANCE",
   "PLANET ORBIT BURN",
   "PLANETARY ACTIVITIES",
   "EARTH RENDEZVOUS",
   "LUNAR RENDEZVOUS",
   "TRANS-PLANETARY INJ.",
   "LEM THRUST TEST",
   "DURATION",
   "JOINT DURATION",
   "REENTRY CORRIDOR",
   "MID-COURSE CORR. BURN",
   "MID-COURSE CORR. BURN",
   "EARTH ORBITAL INS. BURN"
   };

#define BSOUND 1
#define HOST 0
#define SLAVE 1
#define MODEM_ERROR 4

extern struct order Order[6];
extern struct ManPool *Men;
char AI[2]={0,0};

#ifdef DEAD_CODE
static char BUZZ_DIR[32];

void Plop(char plr,char mode);
#endif

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

int main(int argc, char *argv[])
{
  int i;
  FILE *fin;
  const char * see_readme = "look for further instructions in the README file";

  char AName[6][22]={"NEW GAME","OLD GAME","MODEM","PLAY BY MAIL","CREDITS","EXIT TO DOS"};
  char ex;

  argc = setup_options(argc, argv);
  /* hacking... */
  log_setThreshold(&_LOGV(LOG_ROOT_CAT), maxx(0, LP_NOTICE-(int)options.want_debug));

  fin = open_gamedat("USA_PORT.DAT");
  if (fin == NULL)
  {
      CRITICAL1("can't find game data files");
      NOTICE1("set environment variable BARIS_DATA or edit config file");
      NOTICE2("%s", see_readme);
      exit(EXIT_FAILURE);
  }
  fclose(fin);

  if (create_save_dir() != 0)
  {
      CRITICAL3("can't create save directory `%s': %s",
              options.dir_savegame, strerror(errno));
      NOTICE1("set environment variable BARIS_SAVE to a writable directory");
      NOTICE2("%s", see_readme);
      exit(EXIT_FAILURE);
  }

  av_setup();

  strcpy(IDT,"i000\0");  strcpy(IKEY,"k000\0");

  LOAD=QUIT=0;

  xMODE=0;

  xMODE|=xMODE_NOCOPRO;

  Data = xmalloc(sizeof (struct Players) + 1);
  buffer = xmalloc(BUFFER_SIZE);

  DEBUG3("main buffer %p (%d)", buffer, BUFFER_SIZE);

  memset(buffer,0x00,BUFFER_SIZE);

  OpenEmUp();                   // OPEN SCREEN AND SETUP GOODIES
  if (options.want_intro)
      Introd();

  ex=0;
  while(ex==0) {

    MakeRecords();

    fin=sOpen("RAST.DAT","rb",0);
    i=fread(buffer,1,BUFFER_SIZE,fin);
    fclose(fin);

    DEBUG2("reading Players: size = %d", (int)sizeof (struct Players));
    RLED(buffer,(char *)Data,i);
    SwapGameDat();	// Take care of endian read
    if (Data->Checksum!=(sizeof (struct Players))) {
        /* XXX: too drastic */
      CRITICAL1("wrong version of data file");
      exit(EXIT_FAILURE);
    }

    gxClearDisplay(0,0);
    PortPal(0);
    key=0;strcpy(IDT,"i000\0");strcpy(IKEY,"i000\0");
    df=1;

tommy:
		music_start(M_LIFTOFF);

    i=99;
    while (i == 99)
     {
      i=MChoice(6,(char *)AName);
      if (i==98) goto tommy;
      IDLE[0]=IDLE[1]=0;
      if (i==99) {Introd();PortPal(0);}
     }

    switch(i) {
      case 1:  // New Game
	      LOAD=QUIT=0,BUTLOAD=0;
	      HARD1=UNIT1=1;
         MAIL=-1;Option=-1;
         strcpy(IDT,"i013");
	      Prefs(0);                     // GET INITIAL PREFS FROM PLAYER
	      plr[0]=Data->Def.Plr1;         // SET GLOBAL PLAYER VALUES
	      plr[1]=Data->Def.Plr2;
	      Data->plr[0]=Data->Def.Plr1;    // SET STRUCTURE PLAYER VALUES
	      Data->plr[1]=Data->Def.Plr2;
	      if (plr[0]==2 || plr[0]==3) AI[0]=1;
          else AI[0]=0;
	      if (plr[1]==2 || plr[1]==3) AI[1]=1;
          else AI[1]=0;
	      InitData();                   // PICK EVENT CARDS N STUFF
	      MainLoop();                   // PLAY GAME
            gxClearDisplay(0,0);
	      break;
      case 2: // Play Old Game
	       LOAD=QUIT=BUTLOAD=0;
	       HARD1=UNIT1=1;
          MAIL=-1;Option=-1;
	       FileAccess(1);

	       if (LOAD==1)
          {
           if (Option==-1 && MAIL==-1) MainLoop(); //Regular game
            else { //Modem game
		    WARNING1("can't do modem games");
		    break;
	    }
          }
         else if (!QUIT) FadeOut(2,pal,10,0,0);
         QUIT=0;
        gxClearDisplay(0,0);
	      break;
      case 3:
         break;
      case 4:
	      break;
      case 5:
         df=0;
         Credits();
         df=1;
	      break;
      case 6:
         //KillMusic();
	      ex=1;
         FadeOut(2,pal,10,0,0);
         break;
    }
  }

  CloseEmUp(0,0);  // Normal Exit
  exit(EXIT_SUCCESS);
}

/** utility fn for AI to see if it should scrub the mission
 *
 * Rules are:
 * - always do unmanned missions
 * - Safety must be within 15 of R&D-Safety
 *
 * \param plr Playerdata
 * \param launchIdx ID of the launch
 */
int CheckIfMissionGo(char plr,char launchIdx)
{
  char idx,mcode;
  struct MissionType *pMission;
  Equipment *E = NULL;	/* Pointer to Equipment we're looking at */

	// Grab the Mission Code from the current Launch Index
  mcode=Data->P[plr].Mission[launchIdx].MissionCode;
  pMission=&Data->P[plr].Mission[launchIdx];

	// Always a go for Unmanned missions
  /** \todo introduce mission attribute "manned vs. unmanned" */
  if (mcode==1 || mcode==3 || mcode==5 || (mcode>=7 && mcode<=13) || mcode==15)
     return 1;

	// Spin through mission hardware checking safety
	for (idx = Mission_Capsule; idx <= Mission_PrimaryBooster; idx++) {
		switch(idx) {
			case Mission_Capsule:
				E=&Data->P[plr].Manned[pMission->Hard[idx]];
				E->MisSaf=E->Safety;
				break;
			case Mission_Kicker:
				E=&Data->P[plr].Misc[pMission->Hard[idx]];
				E->MisSaf=E->Safety;
				break;
			case Mission_LM:
				E=&Data->P[plr].Manned[pMission->Hard[idx]];
				E->MisSaf=E->Safety;
				break;
			case Mission_PrimaryBooster:
				E=&Data->P[plr].Manned[pMission->Hard[idx%4]];	// YYY No idea why this is using this value

				// YYY  Safety check for this is never reached
				if (idx>Mission_PrimaryBooster)		// implies
					E->MisSaf = (int) RocketBoosterSafety(E->Safety, Data->P[plr].Manned[pMission->Hard[Mission_PrimaryBooster]].Safety);
				break;
		}

		if (E && idx != Mission_Probe_DM && pMission->Hard[idx]>=0)
		{
			// If mission Safety is not within 15 points of the MaxRD then NO Go
			if (E->MisSaf < (E->MaxRD-15))
				return 0;
		}
	}
  return 1;
}

void InitData(void)
{
  int i,j;

  SetEvents();                  // RESET EVENT CARDS
  Data->Count=0;                 // SET EVENT COUNTER TO ZERO
  for(j=0;j<NUM_PLAYERS;j++)
    for(i=0;i<5;i++)
      Data->P[j].PresRev[i]=8;
  return;
}

void MainLoop(void)
{
int i,j,t1,t2,t3,prest,sign,kik;

  if (LOAD!=1) {
    Data->P[0].Cash=Data->P[0].Budget;   // INCREMENT BY BUDGET
    Data->P[1].Cash=Data->P[1].Budget;
  }

restart:                              // ON A LOAD PROG JUMPS TO HERE

  LOAD=0;                             // CLEAR LOAD FLAG

  while (Data->Year < 78)              // WHILE THE YEAR IS NOT 1977
  {
		EndOfTurnSave((char *) Data, sizeof ( struct Players));

   Data->P[0].RDMods=0;           // CLEAR ALL TURN RD MODS
   Data->P[1].RDMods=0;

   Data->P[0].BudgetHistory[Data->Year-53]=Data->P[0].Budget; // RECORD BUDGET
   Data->P[1].BudgetHistory[Data->Year-53]=Data->P[1].Budget;

   Data->P[0].BudgetHistoryF[Data->Year-53]=     // MAKE ESTIMATE OF BUDGETS
      (Data->P[0].Budget*(random(40)+80))/100;
   Data->P[1].BudgetHistoryF[Data->Year-53]=
      (Data->P[1].Budget*(random(40)+80))/100;

  for(t1=0;t1<NUM_PLAYERS;t1++)                   // Move Expenditures down one
  {
    for(t2=4;t2>=0;t2--)
     for(t3=0;t3<4;t3++)
	Data->P[t1].Spend[t2][t3]=Data->P[t1].Spend[t2-1][t3];
    for(t3=0;t3<4;t3++)
     Data->P[t1].Spend[0][t3]=0;
  };
   if(Data->Season==0) CalcPresRev();

   for (i=0;i<NUM_PLAYERS;i++) {
     xMODE&= ~xMODE_CLOUDS;  // reset clouds for spaceport
     if (Data->Season==1) {
       IntelPhase(plr[i]-2*AI[i],0);
      }
   // computer vs. human

   if ((IDLE[0]>12 || IDLE[1]>12) || ((AI[i] && plr[other(i)]<NUM_PLAYERS && ((Data->Def.Lev1!=0 && other(i)==0) || (Data->Def.Lev2!=0 && other(i)==1)))))
    {
     if (IDLE[0]>12 || IDLE[1]>12 || Data->P[abs(i-1)].PresRev[0]>=16)
      {
       strncpy(IDT,"i136",4);
       Data->P[abs(i-1)].PresRev[0]=0x7F;
       strncpy(IDT,"i000",4);
       if (IDLE[0]>12 || IDLE[1]>12) SpecialEnd();
       else {
         Review(abs(i-1));
         FakeWin(plr[i]-2);
       }
       FadeOut(2,pal,10,0,0);
       QUIT=1;
       return;
      }
    };
     if (!AI[i]) {
       NextTurn(plr[i]);
       VerifySF(plr[i]);
       News(plr[i]);                  // EVENT FOR PLAYER
       if ((Data->P[plr[i] % NUM_PLAYERS].Mission[0].MissionCode>6 ||
           Data->P[plr[i] % NUM_PLAYERS].Mission[1].MissionCode>6 ||
           Data->P[plr[i] % NUM_PLAYERS].Mission[2].MissionCode>6) &&
           (NOCOPRO && !PUSSY ))
          xMODE &= ~xMODE_NOCOPRO;
       VerifyCrews(plr[i]);
       VerifySF(plr[i]);
       strncpy(IDT,"i000",4);strncpy(IKEY,"k000",4);
       FixPrograms(plr[i]);

       //soften sound
//       SetVoiceVolume(80);            // 80% seems good MWR
       Master(plr[i]);                // PLAY TURN
       //restore sound
//       SetVoiceVolume(115);
        gxClearDisplay(0,0);
       IDLE[plr[i]]++;
       if (LOAD==1) goto restart;     // TEST FOR LOAD
     } else {
      AI_Begin(plr[i]-2);  // Turns off Mouse for AI
      GetMouse();
      VerifySF(plr[i]-2);
	    AIEvent(plr[i]-2);VerifySF(plr[i]-2);
	    AIMaster(plr[i]-2);
      AI_Done(); // Fade Out AI Thinking Screen and Restores Mouse
	 };
     Data->Count++;
     if (QUIT) return;
   };

   DockingKludge();  // fixup for both sides

   // Do Missions Here
   kik=OrderMissions();

for (i = 0; i < kik; i++)
	if (Data->P[Order[i].plr].Mission[Order[i].loc].MissionCode != 0)
	{
		if (AI[Order[i].plr] == 1)
		{
			if (!CheckIfMissionGo(Order[i].plr, Order[i].loc))
				ClrMiss(Order[i].plr, Order[i].loc);
		}
		if (Data->P[Order[i].plr].Mission[Order[i].loc].MissionCode != 0)
		{
			if (!AI[Order[i].plr])
				IDLE[0] = IDLE[1] = 0;
			if (!(Data->P[Order[i].plr].Mission[Order[i].loc].part == 1
					|| Data->P[Order[i].plr].Mission[Order[i].loc].Hard[4] ==
					0))
			{
				prest = Launch(Order[i].plr, Order[i].loc);
				// check for prestige firsts
				if (AI[Order[i].plr] == 1 && Data->Prestige[22].Place == -1)	// supposed to be 1
					for (j = 0; j < 28; j++)
					{
						if (j != 4 && j != 5 && j != 6)
							if (Data->Prestige[j].Place == Order[i].plr
								&& Data->PD[Order[i].plr][j] != 1)
							{
								PlayFirst(Order[i].plr, j);
							}
					}
				if (Data->Prestige[22].Place != -1)
				{
					UpdateRecords(1);
					NewEnd(Data->Prestige[22].Place, Order[i].loc);
					FadeOut(2, pal, 10, 0, 0);
					return;
				}
				if (!AI[Order[i].plr] && prest != -20)	// -20 means scrubbed
					MisRev(Order[i].plr, prest);
			}
		}

	};							   //for(i=0...
   Update();  /* Moves Future launches to Missions + More */

   // Update M array to speed AI code
   for (j=0;j<NUM_PLAYERS;j++) {
      for (i=0;i<Data->P[j].PastMis;i++) {
	 if (Data->P[j].History[i].result>=100)
	    Data->P[j].M[Data->P[j].History[i].MissionCode]=1;
	 }
      }

   if (Data->Year==77 && Data->Season==1 && Data->Prestige[22].Place==-1)
     {  // nobody wins .....
      SpecialEnd();
      FadeOut(2,pal,10,0,0);
      return;
     }

   Data->P[0].Budget+=Data->P[0].Prestige;
   Data->P[1].Budget+=Data->P[1].Prestige;
   if (Data->P[0].Budget>180) Data->P[0].Budget=180;
   if (Data->P[1].Budget>180) Data->P[1].Budget=180;
  // move prestige history down one;
  for(t3=0;t3<2;t3++) // t3 is the index to the real and random hists
  {
    for(t1=0;t1<NUM_PLAYERS;t1++) // t1 is the player index
    {
      for(t2=4;t2>0;t2--) // t2 is the time index
	Data->P[t1].PrestHist[t2][t3]=Data->P[t1].PrestHist[t2-1][t3];
      sign = (Data->P[t1].Prestige < 0) ? -1 : 1;

      Data->P[t1].PrestHist[0][t3] = (t3 == 0) ? Data->P[t1].Prestige
	  : (Data->P[t1].Prestige*4)/5+sign*random(Data->P[t1].Prestige*2/5)+1;
      if(t3==0) Data->P[t1].tempPrestige[Data->Season] += Data->P[t1].Prestige;
    }
  };
   Data->P[0].Prestige=Data->P[1].Prestige=0;

	if (Data->Season == 1)
	{
		for (i = 0; i < NUM_PLAYERS; i++)
		{
			int p = plr[i] % NUM_PLAYERS;
			int pi = plr[i] - 2*AI[i];

			assert(0 <= pi && pi <= NUM_PLAYERS);
			Data->P[p].Cash += Data->P[pi].Budget;
			if (Data->P[p].Cash > 999)
				Data->P[pi].Cash = 900;
		}
		Data->Season = 0;
		Data->Year++;
	}
	else
		Data->Season++;
  };
  FadeOut(2,pal,10,0,0);
  Museum(0);
  Museum(1);
  return;
}

void DockingKludge(void)
{
  int j;
  for (j=0;j<NUM_PLAYERS;j++) {
    Data->P[j].Misc[4].MSF=
       maxx( maxx(Data->P[j].Probe[0].Safety,Data->P[j].Probe[1].Safety),
	    Data->P[j].Probe[2].Safety);
  }
  return;
}

/** Reset Crews on a particular Mission
 */
void FreePadMen(char plr,struct MissionType *XMis)
{
   int i,c;
   if (XMis->PCrew>0) {    // Remove Primary Crew
      for (i=0;i<Data->P[plr].Gcnt[XMis->Prog][XMis->PCrew-1];i++) {
         c=Data->P[plr].Crew[XMis->Prog][XMis->PCrew-1][i]-1;
         Data->P[plr].Pool[c].Prime=0;
         }
      }
   if (XMis->BCrew>0) {    // Remove Backup Crew
      for (i=0;i<Data->P[plr].Gcnt[XMis->Prog][XMis->BCrew-1];i++) {
         c=Data->P[plr].Crew[XMis->Prog][XMis->BCrew-1][i]-1;
         Data->P[plr].Pool[c].Prime=0;
         }
      }
   return;
}


/** Destroy Pad and Reset any Crews affected
 *
 * \param cost amount of MB to pay for repairs
 * \param pad number of the launch facility destroyed
 * \param plr Player data
 * \param mode 0 if future mission, 1 is current mission
 */
void DestroyPad(char plr,char pad,int cost,char mode)
{
   struct MissionType *BMis=NULL,*AMis=NULL;

   Data->P[plr].LaunchFacility[pad]=cost;  // Destroys pad

   AMis=(mode==0) ? &Data->P[plr].Future[pad] : &Data->P[plr].Mission[pad];
   if (AMis->Joint==1) {
      if (AMis->part==0) BMis=&AMis[1];
      if (AMis->part==1) BMis=&AMis[-1];
      }

   if (AMis!=NULL) {
      if (AMis->Men!=0) FreePadMen(plr,AMis);
      memset(AMis,0x00,sizeof (struct MissionType));
      }
   if (BMis!=NULL) {
      if (BMis->Men!=0) FreePadMen(plr,BMis);
      memset(BMis,0x00,sizeof (struct MissionType));
      }

   return;
}

void GV(GXHEADER *obj,int w,int h)
{
  gxCreateVirtual(gxCMM,obj,gxVGA_13,w,h);
}

void DV(GXHEADER *obj)
{
  gxDestroyVirtual(obj);
  return;
}

void
GetMouse (void)
{
	av_block ();
	GetMouse_fast ();
}


/* get mouse of keyboard input, non-blocking */
void GetMouse_fast(void)
{
  mousebuttons=0;
  oldx=x;oldy=y;

  gr_maybe_sync ();
  av_step ();

  if (XMAS!=0) {
	  if (grGetMouseButtons()) {
		  mousebuttons=1;
		  grGetMousePressedPos(&x,&y);
	  } else {
		  mousebuttons=0;
		  grGetMouseCurPos (&x, &y);
	  }
  }
  while (bioskey(1)) {
    key=bioskey(0);
    if((key&0x00ff)>0) key=toupper(key & 0xff);
    mousebuttons=0;
    //grSetMousePos(319,199);
    }

   if (mousebuttons>0) key=0;

  if (key>>8==15) CloseEmUp(0,0);
  else if (XMAS && AL_CALL==0 && (key>>8==0x3B)) {
    if (mousebuttons!=1) Help(IDT);
    }
  else if (AL_CALL==0 && ((key>>8)==0x3C)) Help(IKEY);
  else if (AL_CALL==0 && ((key>>8)==0x3D)) Help("i123");
}

void WaitForMouseUp(void)
{
	// Wait for mouse and key to be up
	while (mousebuttons)
		GetMouse();
}

void WaitForKeyOrMouseDown(void)
{
	// Wait for mouse and key to be up
	while (mousebuttons==0 && key==0)
		GetMouse();
}


void PauseMouse(void)
{
	/* wait until mouse button is released */
	while(1)  {
		GetMouse(); /* blocks briefly */
		if (mousebuttons==0) break;
	}

	GetMouse ();
}

/** Print string at specific position
 *
 * The function will print a string at a certain position.
 * If both the x and y coordinate are zero (0) the position is "current position".
 * The area will be marked as "needs update" to the graphics handler.
 *
 * \note Will not print anything if length of string exceeds 100 characters
 *
 * \param x x-coordinate of the start of the string
 * \param y y-coordinate of the start of the string
 * \param s pointer to char-array (string) to print
 *
 */
void PrintAt(int x,int y,char *s)
{
  short i;
  if (x!=0 && y!=0) grMoveTo(x,y);
  if (strlen(s) > 100) return;
  for(i=0;i<(int)strlen(s);i++)
    DispChr(s[i]);
  av_need_update_xy(x, y-7, x+i*7, y);
  return;
}

void PrintAtKey(int x,int y,char *s,char val)
{
  PrintAt(x, y, s);
  grMoveTo(x,y);grSetColor(9);
  DispChr(s[val]);
  return;
}

#ifdef DEAD_CODE
void DrawLED(int x,int y,char st)
{
  int i,j;
  unsigned char Dots[2][5][5] ={
	  {{0,9,9,10,0},{9,8,9,9,10},{9,9,9,9,10},{10,9,9,9,10},{0,10,10,10,0}}, // Red
	  {{0,15,15,16,0},{15,14,15,15,16},{15,15,15,15,16},{16,15,15,15,16},{0,16,16,16,0}}}; // Grn

  for (i=0;i<5;i++) for (j=0;j<5;j++) if (Dots[st][i][j]!=0)
    grPutPixel(x+i,y+j,Dots[st][i][j]);
  return;
}
#endif

void DispBig(int x,int y,char *txt,char mode,char te)
{
  int i,k,l,px;
  struct LET {char width,img[15][21];} letter;
  int c;
  int x0 = x;

  y--;
  if(mode)EMPTY_BODY;

  for (i=0;i<(int)strlen(txt);i++) {
    if (txt[i]==0x20) {x+=6;i++;};
    c=toupper(txt[i] & 0xff);
    if (c>=0x30 && c<=0x39) px=c-32;
    else px=c-33;
    if (c=='-') px++;

    memcpy(&letter,letter_dat + (sizeof letter*px),sizeof letter);  // copy letter over
    for (k=0;k<15;k++)
      for (l=0;l<letter.width;l++) {
  	    if (letter.img[k][l]!=0x03) {
	     if ((letter.img[k][l]==0x01 || letter.img[k][l]==0x02) && i==te)
	      grPutPixel(x+l,y+k,letter.img[k][l]+7);
	    else grPutPixel(x+l,y+k,letter.img[k][l]);
	    }
      }
    x+=letter.width-1;
  };
  av_need_update_xy(x0, y, x, y+15);
}

void DispNum(int xx,int yy,int num)
{
  short n0,n1,n2,n3,t;
  if (xx!=0 && yy!=0) grMoveTo(xx,yy);
  t=num;
  num=abs(t);
  if (t<0) PrintAt(0,0,"-");
  n0=num/1000;
  n1=num/100-n0*10;
  n2=num/10-n0*100-n1*10;
  n3=num-n0*1000-n1*100-n2*10;
  if (n0!=0) {
    DispChr(n0+0x30);
    DispChr(n1+0x30);
    DispChr(n2+0x30);
    DispChr(n3+0x30);
  };
  if (n0==0 && n1!=0) {
    DispChr(n1+0x30);
    DispChr(n2+0x30);
    DispChr(n3+0x30);
  };
  if (n0==0 && n1==0 && n2!=0) {
    DispChr(n2+0x30);
    DispChr(n3+0x30);
  };
  if (n0==0 && n1==0 && n2==0)
    DispChr(n3+0x30);
  return;
}


void DispMB(int x,int y,int val)
{
  DispNum(x,y,val);
  PrintAt(0,0," MB");
  return;
}

#ifdef DEAD_CODE
// Place a glimmer on a box
void Gl(int x1,int x2,int y,char t)
{
  int i,nx;
  char Glim[13]={31,30,29,28,27,1,1,1,27,28,29,30,31};
  if ((x2-x1)<30) return;
  nx=(x2-x1)/2;
  if (t==0) nx=nx-(nx/3);
  else nx=nx+(nx/3);
  for (i=0;i<13;i++) grPutPixel(x1+nx+i,y,Glim[i]);
  return;
}
#endif

void ShBox(int x1,int y1,int x2,int y2)
{
  RectFill(x1,y1,x2,y2,3);
  OutBox(x1,y1,x2,y2);
  return;
}

void UPArrow(int x1,int y1)
{
  grSetColor(4);grDrawLine(x1,y1,x1,25+y1);grDrawLine(3+x1,11+y1,5+x1,11+y1);
  grSetColor(2);grDrawLine(1+x1,y1,6+x1,11+y1);grDrawLine(3+x1,12+y1,3+x1,25+y1);
  return;
}

void RTArrow(int x1,int y1)
{
 grSetColor(4);grDrawLine(x1,y1,x1+31,y1);grDrawLine(x1+20,y1+3,x1+20,y1+5);
 grSetColor(2);grDrawLine(x1,y1+3,x1+19,y1+3);grDrawLine(x1+31,y1+1,x1+20,y1+6);
 return;
}

void LTArrow(int x1,int y1)
{
 grSetColor(4);grDrawLine(x1,y1,x1+31,y1);grDrawLine(x1+11,y1+3,x1+11,y1+5);
 grSetColor(2);grDrawLine(x1,y1+1,x1+11,y1+6);grDrawLine(x1+12,y1+3,x1+31,y1+3);
 return;
}

void DNArrow(int x1,int y1)
{
  grSetColor(4);grDrawLine(x1,y1,x1,25+y1);grDrawLine(3+x1,14+y1,5+x1,14+y1);
  grSetColor(2);grDrawLine(3+x1,y1,3+x1,13+y1);grDrawLine(6+x1,14+y1,1+x1,25+y1);
  return;
}

void InBox(int x1,int y1,int x2,int y2)
{
  grSetColor(2);  grMoveTo(x1,y2);  grLineTo(x2,y2);  grLineTo(x2,y1);
  grSetColor(4);  grLineTo(x1,y1);  grLineTo(x1,y2);
  av_need_update_xy(x1, y1, x2, y2);
}

void OutBox(int x1,int y1,int x2,int y2)
{
  grSetColor(4);  grMoveTo(x1,y2); grLineTo(x2,y2);  grLineTo(x2,y1);
  grSetColor(2);  grMoveTo(x2-1,y1); grLineTo(x1,y1);  grLineTo(x1,y2-1);
  av_need_update_xy(x1, y1, x2, y2);
}

void IOBox(int x1, int y1, int x2, int y2)
{
  InBox(x1,y1,x2,y2);
  grSetColor(0);Box(x1+1,y1+1,x2-1,y2-1);
  OutBox(x1+2,y1+2,x2-2,y2-2);
}

/** draw a rectangle
 *
 * \param x1
 * \param y1
 * \param x2
 * \param y2
 * \param col Color code of the rectangle
 *
 */
void RectFill(int x1,int y1,int x2,int y2,char col)
{
  grSetBkColor(col);
  grClearArea(x1,y1,x2,y2);
  return;
}

void Box(int x1,int y1,int x2,int y2)
{
  grDrawRect(x1,y1,x2,y2,grOUTLINE);
  return;
}

void GradRect(int x1,int y1,int x2,int y2,char plr)
{
  //register int i,j,val;
  //val=3*plr+6;

  RectFill(x1,y1,x2,y2,7+3*plr);
  //for (j=x1;j<=x2;j+=4)
  //  for (i=y1;i<=y2;i+=4)
  //    screen[j+320*i]=val;
   return;
}

void FlagSm(char plr,int xm,int ym)
{
  int i;
  if (plr==0) {
    RectFill(xm,ym,xm+25,ym+14,1);
    grSetColor(8);
    for (i=0;i<15;i++) {
      grMoveTo(xm,ym+i);
      grLineTo(xm+25,ym+i);
      i++;
    };
    RectFill(xm,ym,xm+12,ym+7,5);
    xm++;
    for (i=0;i<11;i++) {
      grPutPixel(xm+i,ym+1,2);
      grPutPixel(xm+i,ym+3,2);
      grPutPixel(xm+i,ym+5,2);
       i++;
    };
    for (i=1;i<10;i++) {
      grPutPixel(xm+i,ym+2,2);
      grPutPixel(xm+i,ym+4,2);
      grPutPixel(xm+i,ym+6,2);
      i++;
    };
  } else {
    RectFill(xm,ym,xm+25,ym+14,8);
    grSetColor(11); grMoveTo(xm+2,ym+6); grLineTo(xm+4,ym+6); grLineTo(xm+4,ym+5);
    grLineTo(xm+5,ym+5); grLineTo(xm+5,ym+3);
    grPutPixel(xm+3,ym+4,11);grPutPixel(xm+4,ym+2,11);grPutPixel(xm+5,ym+1,11);
    grPutPixel(xm+6,ym+2,11);
  }
  return;
}

void Flag(int x,int y,char plr)
{
  int i,j;
  if (plr==0) {
    for (i=0;i<7;i++) RectFill(x,y+i*6,x+69,y+2+i*6,8);
    for (i=0;i<6;i++) RectFill(x,y+3+i*6,x+69,y+5+i*6,1);
    RectFill(x,y,x+33,y+20,6);
    RectFill(x,y,x+32,y+20,5);
    for (j=0;j<5;j++) for (i=0;i<8;i++) PP(x+2+i*4,y+2+4*j,2);
    for (j=0;j<4;j++) for (i=0;i<7;i++) PP(x+4+i*4,y+4+4*j,2);
  } else {
    RectFill(x,y,x+69,38+y,8);
    PP(10+x,2+y,11);
    PP(8+x,3+y,11); PP(9+x,3+y,11);PP(11+x,3+y,11); PP(12+x,3+y,11);
    PP(9+x,5+y,11); PP(11+x,5+y,11);
    PP(12+x,6+y,11);
    PP(10+x,7+y,11);PP(13+x,7+y,11);
    PP(9+x,8+y,11);PP(10+x,8+y,11);PP(11+x,8+y,11);PP(14+x,8+y,11);
    PP(8+x,9+y,11);PP(9+x,9+y,11);PP(10+x,9+y,11);PP(14+x,9+y,11);PP(15+x,9+y,11);
    PP(7+x,10+y,11);PP(8+x,10+y,11);PP(9+x,10+y,11);PP(10+x,10+y,11);PP(14+x,10+y,11);PP(15+x,10+y,11);
    PP(11+x,11+y,11);PP(14+x,11+y,11);
    PP(12+x,12+y,11);PP(13+x,12+y,11);PP(14+x,12+y,11);
    PP(7+x,13+y,11);PP(8+x,13+y,11);PP(12+x,13+y,11);PP(13+x,13+y,11);
    PP(6+x,14+y,11);PP(9+x,14+y,11);PP(10+x,14+y,11);PP(11+x,14+y,11);
    PP(12+x,14+y,11);PP(13+x,14+y,11);PP(14+x,15+y,11);
    PP(9+x,4+y,12);PP(10+x,4+y,12);PP(11+x,4+y,12);
    PP(14+x,7+y,12); PP(8+x,11+y,12); PP(10+x,11+y,12); PP(15+x,11+y,12);
    PP(11+x,12+y,12); PP(14+x,13+y,12); PP(7+x,14+y,12); PP(5+x,15+y,12);
    PP(15+x,15+y,12);
  };
  return;
}

/** Prints a character at current position of graphics handler.
 *
 * \note The function converts all characters to upper case before printing.
 *
 * \param chr Character to be printed
 */
void DispChr(char chr)
{
  switch(toupper(chr))
  {
    case 'A': LR(0,-3);LR(1,-1);LR(2,0);LR(1,1);LR(0,3);LR(-1,-1);
	      LR(-2,0);MR(5,1);break;
    case 'B': LR(0,-4);LR(3,0);LR(1,1);LR(-1,1);LR(1,1);
	      LR(-1,1);LR(-2,0);MR(0,-2);LR(1,0);MR(4,2);
	      break;
    case 'C': MR(4,-4);LR(-3,0);LR(-1,1);LR(0,2);LR(1,1);LR(3,0);MR(2,0);
	      break;
    case 'D': LR(0,-4);LR(3,0);LR(1,1);LR(0,2);LR(-1,1);LR(-2,0);MR(5,0);
	      break;
    case 'E': LR(0,-4);LR(4,0);MR(0,4);LR(-3,0);MR(0,-2);LR(1,0);MR(4,2);
	      break;
    case 'F': LR(0,-4);LR(4,0);MR(-2,2);LR(-1,0);MR(5,2);break;
    case 'G': MR(4,-4);LR(-3,0);LR(-1,1);LR(0,2);LR(1,1);LR(3,0);
	      LR(0,-2);LR(-1,0);MR(3,2);break;
    case 'H': LR(0,-4);MR(1,2);LR(2,0);MR(1,-2);LR(0,4);MR(2,0);
	      break;
    case 'I': LR(2,0);LR(-1,-1);LR(0,-2);LR(-1,-1);LR(2,0);MR(2,4);break;
    case 'J': MR(0,-1);LR(1,1);LR(2,0);LR(1,-1);LR(0,-3);MR(2,4);break;
    case 'K': LR(0,-4);MR(4,0);LR(-2,2);LR(-1,0);MR(1,0);LR(2,2);MR(2,0);
	      break;
    case 'L': MR(0,-4);LR(0,4);LR(4,0);
	      MR(2,0);break;
    case 'M': LR(0,-4);LR(2,2);LR(2,-2);LR(0,4);MR(2,0);break;
    case 'N': LR(0,-4);LR(4,4);LR(0,-4);MR(2,4);break;
    case 'O': case '0': MR(0,-1);LR(0,-2);LR(1,-1);LR(2,0);LR(1,1);
	      LR(0,2);LR(-1,1);LR(-2,0);MR(5,0);break;
    case 'P': LR(0,-4);LR(3,0);LR(1,1);LR(-1,1);LR(-2,0);MR(5,2);break;
    case 'Q': MR(0,-1);LR(0,-2);LR(1,-1);LR(2,0);LR(1,1);
	      LR(0,2);LR(-1,1);LR(-2,0);MR(2,-1);LR(1,1);MR(2,0);break;
    case 'R': LR(0,-4);LR(3,0);LR(1,1);LR(-1,1);LR(-2,0);MR(3,1);LR(0,1);
	      MR(2,0);break;
    case 'S': LR(3,0);LR(1,-1);LR(-1,-1);LR(-2,0);LR(-1,-1);
	      LR(1,-1);LR(3,0);MR(2,4);break;
    case 'T': MR(2,0);LR(0,-4);LR(-2,0);LR(4,0);MR(2,4);break;
    case 'U': MR(0,-4);LR(0,3);LR(1,1);LR(2,0);LR(1,-1);LR(0,-3);MR(2,4);break;
    case 'V': MR(0,-4);LR(0,2);LR(2,2);LR(2,-2);LR(0,-2);MR(2,4);break;
    case 'W': MR(0,-4);LR(0,4);LR(2,-2);LR(2,2);LR(0,-4);MR(2,4);break;
    case 'X': MR(0,-4);LR(4,4);MR(0,-4);LR(-4,4);MR(6,0);break;
    case 'Y': MR(2,0);LR(0,-1);LR(-2,-2);LR(0,-1);MR(4,0);LR(0,1);
	      LR(-2,2);MR(4,1);break;
    case 'Z': MR(0,-4);LR(4,0);LR(-4,4);LR(4,0);MR(2,0);break;
    case '1': LR(2,0);LR(-1,-1);LR(0,-3);LR(-1,1);MR(4,3);break;
    case '2': MR(0,-4);LR(3,0);LR(1,1);LR(-1,1);LR(-1,0);LR(-2,2);
	      LR(4,0);MR(2,0);break;
    case '3': LR(3,0);LR(1,-1);LR(-1,-1);LR(-1,0);LR(1,0);LR(1,-1);
	      LR(-1,-1);LR(-3,0);MR(6,4);break;
    case '4': MR(4,-1);LR(-4,0);LR(0,-1);LR(2,-2);LR(1,0);LR(0,4);MR(3,0);
	      break;
    case '5': LR(3,0);LR(1,-1);LR(-1,-1);LR(-2,0);LR(-1,-1);LR(0,-1);
	      LR(4,0);MR(2,4);break;
    case '6': MR(1,-2);LR(2,0);LR(1,1);LR(-1,1);LR(-2,0);LR(-1,-1);
	      LR(0,-2);LR(1,-1);LR(2,0);MR(3,4);break;
    case '7': MR(0,-4);LR(4,0);LR(0,1);LR(-2,2);LR(0,1);MR(4,0);break;
    case '8': MR(1,0);LR(2,0);LR(1,-1);LR(-1,-1);LR(-2,0);LR(-1,-1);
	      LR(1,-1);LR(2,0);LR(1,1);MR(-4,2);LR(0,0);MR(6,1);break;
    case '9': MR(1,0);LR(2,0);LR(1,-1);LR(0,-2);LR(-1,-1);LR(-2,0);
	      LR(-1,1);LR(1,1);LR(2,0);MR(3,2);break;
    case '-': MR(0,-2),LR(3,0);MR(2,2);break;
    case '.': LR(0,0);MR(2,0);break;
    case ',': MR(0,1);LR(1,-1);MR(2,0);break;
    case ':': MR(0,-1);LR(0,0);MR(0,-2);LR(0,0);MR(2,3);break;
    case '&': MR(0,-1);LR(1,1);LR(1,0);LR(1,-1);LR(1,1);LR(-2,-2);
	      LR(-1,0);LR(0,-1);LR(1,-1);LR(1,1);MR(3,3);break;
    case ' ': MR(3,0); break;
    case '!': LR(0,0);MR(0,-2);LR(0,-2);MR(2,4);break;
    case '@':
    case '#': MR(1,0);LR(0,-4);MR(-1,1);LR(4,0);MR(-1,-1);LR(0,4);
	      MR(1,-1);LR(-4,0);MR(6,1);break;
    case '%': LR(4,-4);MR(-3,0);LR(-1,1);LR(0,-1);LR(4,4);LR(-1,0);LR(1,-1);
	      MR(2,1);break;
    case '(': MR(1,0);LR(-1,-1);LR(0,-2);LR(1,-1);MR(2,4);break;
    case ')': LR(1,-1);LR(0,-2);LR(-1,-1);MR(3,4);break;
    case '/': LR(4,-4);MR(2,4);break;
    case '<': MR(4,-4);LR(-2,2);LR(2,2);MR(2,0);break;
    case '>': MR(0,-4);LR(2,2);LR(-2,2);MR(4,0);break;
    case 0x27: MR(0,-4);LR(0,1);MR(2,3);break;
    case '*': MR(1,0);LR(0,-4);MR(-1,1);LR(4,0);MR(-1,-1);LR(0,4);
	      MR(1,-1);LR(-4,0);MR(6,1);break;
    case '^': MR(0,-3);LR(1,-1);LR(1,0);LR(1,1);MR(0,3);break;
    case '?': MR(0,-3);LR(1,-1);LR(2,0);LR(1,1);LR(-1,1);
         MR(0,2);LR(0,0);MR(3,0);break;
    case 0x14: LR(0,-4);MR(2,4);
	       break;
    default:  break;
  }
}

void VerifySF(char plr)
{
  int i;
  Equipment *px;

  for (i=0;i<28;i++) {
    px=(Equipment *) &Data->P[plr].Probe[i];
    if (px->Safety>px->MaxSafety) px->Safety=px->MaxSafety;
    if (px->Safety<px->Base) px->Safety=px->Base;
  }
}

void VerifyCrews(char plr)
{
  int i,t,k;

  for (i=0;i<3;i++) {
     if (Data->P[plr].Mission[i].MissionCode==18 && Data->P[plr].Mission[i].part==0) {
        Data->P[plr].Mission[i].Joint=1;
        Data->P[plr].Mission[i+1].Joint=1;
        Data->P[plr].Mission[i].part=0;
        Data->P[plr].Mission[i+1].part=1;
     }
     if (Data->P[plr].Mission[i].PCrew>0) {  // primary verify
        t=Data->P[plr].Mission[i].Prog;
        k=Data->P[plr].Mission[i].PCrew-1;
        if (Data->P[plr].Gcnt[t][k]==0) Data->P[plr].Mission[i].PCrew=0;

        t=Data->P[plr].Mission[i].Prog;  // backup verify
        k=Data->P[plr].Mission[i].BCrew-1;
        if (Data->P[plr].Gcnt[t][k]==0) Data->P[plr].Mission[i].BCrew=0;

     }
  }
}

/** Opens "MISSION.DAT" to find mission data
 *
 * \param mcode Code of the mission - works as index for the file
 *
 * \todo This is highly coupled to game logic
 * and size of internal structures.
 */
void GetMisType(char mcode)
{
  FILE *fin;
	/** \todo Missing assert for opening file? */
  fin=sOpen("MISSION.DAT","rb",0);
  fseek(fin,mcode*(sizeof Mis),SEEK_SET); // Find Mission Type
  fread(&Mis,sizeof Mis,1,fin);            // Get new Mission Info
  fclose(fin);
}


int MisRandom(void)
{
  int i,nval;
  do {
    nval=107;
    for (i=0;i<250;i++) nval+=(random(7)-3);
  } while (nval<50 || nval>150);

  return nval-50;
}

#ifdef DEAD_CODE
#   ifdef CONFIG_THEORA_VIDEO
#      include <av.h>
#      include <mmfile.h>
#   endif

void
Plop(char plr, char mode)
{
	char sName[20];

#   ifndef CONFIG_THEORA_VIDEO
	int wlen;
	FILE *fin;
	char *ext = "frm";
#   else
	char *ext = "ogg";
	mm_file vidfile;
	char fname[1000];
#   endif

	if (mode == 0)
		sprintf(sName, "%s%d.%s", (plr == 0) ? "us" : "sviet",
			random(5) + 1, ext);
	else if (mode == 1)
		sprintf(sName, "passt%02d.%s", random(11) + 1, ext);
	else
		sprintf(sName, "static.%s", ext);

#   ifndef CONFIG_THEORA_VIDEO
	fin = sOpen(sName, "rb", 0);
	if (!fin)
		return;
	wlen = 8;
	fread(&vhptr.vptr[40000], 1, wlen * 2048, fin);
	vhptr.vptr[40000] = vhptr.vptr[40001] = vhptr.vptr[40002] =
		vhptr.vptr[40005];
	vhptr.vptr[55999] = vhptr.vptr[55995];
	vhptr.vptr[55998] = vhptr.vptr[55994];
	memcpy(&pal[384], &vhptr.vptr[56000], 384);
	/* SetPal(pal); FIXME */
	if (BIG == 0)
		SMove(&vhptr.vptr[40000], 80, 3 + plr * 10);
	else
		LMove(&vhptr.vptr[40000]);
	fclose(fin);
	return;
#   else
	sprintf(fname, "%s/%s", movies_dir, sName);
	if (mm_open(&vidfile, fname) <= 0)
		return;
	if (mm_video_info(&vidfile, NULL, NULL, NULL) <= 0)
		goto end;
	if (mm_decode_video(&vidfile, video_overlay) <= 0)
		goto end;
	if (BIG == 0)
	{
		video_rect.w = 160;
		video_rect.h = 100;
		video_rect.x = 80;
		video_rect.y = 3 + plr * 10;
	}
	else
	{
		video_rect.x = MAX_X / 4;
		video_rect.y = MAX_Y / 4;
		video_rect.w = MAX_X / 2;
		video_rect.h = MAX_Y / 2;
	}
  end:
	mm_close(&vidfile);
#   endif
}

#endif
// EOF
