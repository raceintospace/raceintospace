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
/** \file future.c This is responsible for Future Mission planning screen.
 * 
 */
#include <Buzz_inc.h>
#include <externs.h>
#include <assert.h>
#include <logging.h>

  //Used to read steps from missStep.dat
  FILE* MSteps;
  char missStep[1024];
  static inline char B_Mis(char x) {return missStep[x]-0x30;}
  /*missStep.dat is plain text, with:
Mission Number (2 first bytes of each line)
A Coded letter, each drawing a different line (1105-1127 for all possible letters)
Numbers following each letter, which are the parameters of the function
Each line must finish with a Z, so the game stops reading
Any other char is ignored, but it's easier to read for a human that way */


LOG_DEFAULT_CATEGORY(future)

  char status[5],lck[5],F1,F2,F3,F4,FMen,F5,Pad;
  char JointFlag,MarFlag,JupFlag,SatFlag,MisType;
  GXHEADER vh;

  struct StepInfo {
    i16 x_cor;
    i16 y_cor;
  } StepBub[MAXBUB];

  struct Parameter {
    char A;   /**< DOCKING */
    char B;   /**< EVA */
    char C;   /**< LEM */
    char D;   /**< JOINT */
    char E;     /**< MANNED/UNMANNED/Duration 0==unmanned 1-6==duration */
    char X;     /**< the type of mission for assign crew and hardware */
    char Z;   /**< A duration mission only */
  } V[62];

extern int Bub_Count;
extern struct mStr Mis;
extern struct MisEval Mev[60];
extern int SEG;

void Load_FUT_BUT(void)
{
  FILE *fin;
  unsigned i;
  fin=sOpen("NFUTBUT.BUT","rb",0);
  i=fread(screen,1,MAX_X*MAX_Y,fin);
  fclose(fin);
  RLED_img((char *)screen,(char *)vh.vptr,(unsigned)i,vh.w,vh.h);
  return;
}

void DrawFuture(char plr,int mis,char pad)
{
  int i,j;
  FILE *fin;
  unsigned sz;
  strcpy(IKEY,"k011");strcpy(IDT,"i011");
  
  JointFlag=0; // initialize joint flag
  F1=F2=F3=F4=FMen=F5=0;
  for (i=0;i<5;i++) lck[i]=status[i]=0;
  FadeOut(2,pal,10,0,0);
  Load_FUT_BUT();
  fin=sOpen("FMIN.IMG","rb",0);
  fread(&pal[0],768,1,fin);
  sz=fread(screen,1,MAX_X*MAX_Y,fin);
  fclose(fin);
  RLED_img((char *)screen,(char *)vhptr.vptr,sz,vhptr.w,vhptr.h);
  gxClearDisplay(0,0);

  gr_sync ();

    if (pad==2) JointFlag=0; // third pad automatic no joint mission
    else
      if (Data->P[plr].LaunchFacility[pad+1] == 1)
      {
       if (Data->P[plr].Future[pad+1].MissionCode==0) JointFlag=1; // if no mission then set joint flag
	  else if (Data->P[plr].Future[pad+1].part==1) // check if the part of that second mission is set
		 {
		  JointFlag=1;
		  Data->P[plr].Future[pad+1].MissionCode=0; // clear mission
		  Data->P[plr].Future[pad+1].part=0;
		 };
      };

  if (pad==1 || pad==0) {
    if (Data->P[plr].LaunchFacility[pad+1]==1) JointFlag=1;
  }

  i=Data->Year;j=Data->Season;
  TRACE3("--- Setting i=Year (%d), j=Season (%d)", i, j);
  if ((i==60 && j==0) || (i==62 && j==0) || (i==64 && j==0) ||
      (i==66 && j==0) || (i==69 && j==1) || (i==71 && j==1) ||
      (i==73 && j==1)) {

     gxVirtualVirtual(&vhptr,1,2,12,11,&vhptr,198,153,gxSET); /* Mars */
     MarFlag=1; } else MarFlag=0;
  if ((i==60 || i==64 || i==68 || i==72 || i==73 || i==77)) {
    gxVirtualVirtual(&vhptr,14,2,64,54,&vhptr,214,130,gxSET); /* Jup */
    JupFlag=1; } else JupFlag=0;
  if (i==61 || i==66 || i==72) {
    gxVirtualVirtual(&vhptr,66,2,114,53,&vhptr,266,135,gxSET); /* Sat */
    SatFlag=1; } else SatFlag=0;

  RectFill(1,1,318,21,3);RectFill(317,22,318,198,3);RectFill(1,197,316,198,3);
  RectFill(1,22,2,196,3);OutBox(0,0,319,199);InBox(3,3,30,19);
  InBox(3,22,316,196);
  IOBox(242,3,315,19);
  ShBox(5,24,183,47);
  ShBox(5,24,201,47); //name box
  ShBox(5,74,41,82); // RESET
  ShBox(5,49,53,72); //dur/man
  ShBox(43,74,53,82); // lock
  ShBox(80,74,90,82);
  ShBox(117,74,127,82);
  ShBox(154,74,164,82);
  ShBox(191,74,201,82);
  ShBox(5,84,16,130);   //arrows up
  ShBox(5,132,16,146); //middle box
  ShBox(5,148,16,194);   //    down
  ShBox(203,24,238,31);  // new right boxes
  RectFill(206,36,235,44,7);
  ShBox(203,33,238,47);
  InBox(205,35,236,45);
  UPArrow(8,95);DNArrow(8,157);
  
  gxVirtualDisplay(&vh,140,5,5,132,15,146,0);
  Toggle(5,1);draw_Pie(0);OutBox(5,49,53,72);
  Toggle(1,1);TogBox(55,49,0);
  Toggle(2,1);TogBox(92,49,0);
  Toggle(3,1);TogBox(129,49,0);

  FMen=F1=F2=F3=F4=F5=0;
  for (i=1;i<4;i++){
    if (status[i]!=0) {
      Toggle(i,1);
    }
  };
	 if (JointFlag==0) {
      F4=2;lck[4]=1;
      Toggle(4,1);
      InBox(191,74,201,82);
      PlaceRX(5);
      TogBox(166,49,1);
    }
	 else {
      F4=0; lck[4]=0;
      status[4]=0;
      Toggle(4,1);
      OutBox(191,74,201,82);
      ClearRX(5);
      TogBox(166,49,0);
    };

  gr_sync ();
  Missions(plr,8,37,mis,1);
  GetMinus(plr);
  grSetColor(5);
  /* lines of text are 1:8,30  2:8,37   3:8,44    */
  switch(pad) { // These used to say Pad 1, 2, 3  -Leon
    case 0: PrintAt(8,30,"PAD A:");break;
    case 1: PrintAt(8,30,"PAD B:");break;
    case 2: PrintAt(8,30,"PAD C:");break;
    default:break;
  };
  grSetColor(1);
  PrintAt(9,80,"RESET");
  PrintAt(256,13,"CONTINUE");
  grSetColor(11);
   if (Data->Season==0) PrintAt(200,9,"SPRING");
   else PrintAt(205,9,"FALL");
   PrintAt(206,16,"19");
   DispNum(0,0,Data->Year);
  grSetColor(1);
  FlagSm(plr,4,4);
  DispBig(40,5,"FUTURE MISSIONS",0,-1);
  FadeIn(2,pal,10,0,0);
  
  return;
}

void ClearDisplay(void)
{
 gxVirtualDisplay(&vhptr,202,48,202,48,241,82,0);
 gxVirtualDisplay(&vhptr,17,83,17,83,241,195,0);
 gxVirtualDisplay(&vhptr,242,23,242,23,315,195,0);
 grSetColor(1);
 return;
}

int GetMinus(char plr)
{
 char i;int u;

 i=PrestMin(plr);
  RectFill(206,36,235,44,7);
 if (i<3) u=1; //ok
   else if (i<9) u=10; //caution
	else u=19; //danger
  gxVirtualDisplay(&vh,203,u,203,24,238,31,0);
 grSetColor(11);
 if (i>0) PrintAt(210,42,"-");
  else grMoveTo(210,42);
 DispNum(0,0,i);
 grSetColor(1);
 return 0;
}

void SetParameters(void)
{
  int i;
  FILE *fin;
  fin=sOpen("MISSION.DAT","rb",0);
  for (i=0;i<62;i++) {
    fread(&Mis,sizeof Mis,1,fin);
    V[i].A=Mis.Doc;  V[i].B=Mis.EVA;
    V[i].C=Mis.LM;   V[i].D=Mis.Jt;
    V[i].E=Mis.Days; V[i].X=Mis.mCrew;
    V[i].Z=Mis.Dur;
  }
  fclose(fin);
  return;
}

void DrawLocks(void)
{
 int i;
 for (i=0;i<5;i++)
   if (lck[i]==1) PlaceRX(i+1);
   else ClearRX(i+1);
 return;
}


/** set the toggles???
 * 
 * \param wh the button 
 * \param i in or out
 */
void Toggle(int wh,int i) 
{
 TRACE3("->Toggle(wh %d, i %d)", wh, i);
 switch(wh)
   {
    case 1:if (i==1) gxVirtualDisplay(&vh,1,21,55,49,89,81,0);else
	  gxVirtualDisplay(&vh,1,56,55,49,89,81,0); break;

    case 2:if(i==1)  gxVirtualDisplay(&vh,38,21,92,49,127,81,0);else
	  gxVirtualDisplay(&vh,38,56,92,49,127,81,0); break;

    case 3:if(i==1)  gxVirtualDisplay(&vh,75,21,129,49,163,81,0);else
	  gxVirtualDisplay(&vh,75,56,129,49,163,81,0); break;

    case 4:if(i==1)  gxVirtualDisplay(&vh,112,21,166,49,200,81,0);else
	  gxVirtualDisplay(&vh,112,56,166,49,200,81,0); break;

    case 5:if (i==1)  gxVirtualDisplay(&vh,153,1,5,49,52,71,0);
	     else gxVirtualDisplay(&vh,153,26,5,49,52,71,0); break;

  default:break;
 }
 TRACE1("<-Toggle()");
 return;
}

void TogBox(int x,int y,int st)
 {
  TRACE4("->TogBox(x %d, y %d, st %d)", x, y, st);
  char sta[2][2]={{2,4},{4,2}};
  
  grSetColor(sta[st][0]);
  grMoveTo(0+x,y+32);grLineTo(0+x,y+0);grLineTo(34+x,y+0);
  grSetColor(sta[st][1]);
  grMoveTo(x+0,y+33);grLineTo(23+x,y+33);grLineTo(23+x,y+23);
  grLineTo(x+35,y+23);grLineTo(x+35,y+0);
  
  TRACE1("<-TogBox()");
  return;
}

void PianoKey(int X)
{
 TRACE2("->PianoKey(X %d)", X);
 int t;
 if (F1==0) {
   if (V[X].A==1) {Toggle(1,1);status[1]=1;}
     else {Toggle(1,0);PlaceRX(1);status[1]=0;}}
 if (F2==0) {
   if (V[X].B==1) {Toggle(2,1);status[2]=1;}
     else {Toggle(2,0);PlaceRX(2);status[2]=0;}}
 if (F3==0) {
   if (V[X].C==1) {Toggle(3,1);status[3]=1;}
     else {Toggle(3,0);PlaceRX(3);status[3]=0;}}
 if (F4==0) {
   if (V[X].D==1) {Toggle(4,0);status[4]=1;}
	 else {Toggle(4,1);status[4]=0; }}

 if (F5==-1 || (F5==0 && V[X].E==0))
  {
   Toggle(5,0);
   status[0]=0;
  }
 else
  {
   Toggle(5,1);
   t=(F5==0) ? V[X].E : F5;
   assert(0 <= t);
   draw_Pie(t);
   status[0]=t;
  }
 
 DrawLocks();
 TRACE1("<-PianoKey()");
 return;
}

/** draw a piechart
 * 
 * The piechart is indicating the number of astronauts on this mission.
 * 
 * \param s something of an offset...
 */
void draw_Pie(int s)
{
 int off;
 if (s==0) off=1;
   else off=s*20;
 gxVirtualDisplay(&vh,off,1,7,51,25,69,0);
 return;
}

void PlaceRX(int s)
{
 switch(s)
  {
   case 1: RectFill(44,75,52,81,8);break;
   case 2: RectFill(81,75,89,81,8);break;
   case 3: RectFill(118,75,126,81,8);break;
   case 4: RectFill(155,75,163,81,8);break;
   case 5: RectFill(192,75,200,81,8);break;
   default:break;
  }
 return;
}

void ClearRX(int s)
{
 switch(s)
 {
   case 1: RectFill(44,75,52,81,3);break;
   case 2: RectFill(81,75,89,81,3);break;
   case 3: RectFill(118,75,126,81,3);break;
   case 4: RectFill(155,75,163,81,3);break;
   case 5: RectFill(192,75,200,81,3);break;
   default:break;
  }
 return;
}

int UpSearchRout(int num,char plr)
{
 int found=0,orig,c1=0,c2=0,c3=0,c4=0,c5=0,c6=1,c7=1,c8=1;
 orig=num;
 if (num >= 56+plr) num=0;
  else num++;
  while (found==0)
   {
	c1=0;c2=0;c3=0;c4=0;c5=0;c6=1;c7=1;c8=1;
	if (F1==V[num].A) c1=1; /* condition one is true */
	if (F1==0 && V[num].A==1) c1=1;
	if (F1==2 && V[num].A==0) c1=1;
	if (F2==V[num].B) c2=1; /* condition two is true */
	if (F2==0 && V[num].B==1) c2=1;
	if (F2==2 && V[num].B==0) c2=1;
	if (F3==V[num].C) c3=1; /* condition three is true */
	if (F3==0 && V[num].C==1) c3=1;
	if (F3==2 && V[num].C==0) c3=1;
	if (F4==V[num].D) c4=1; /* condition four is true */
	if (F4==0 && V[num].D==1) c4=1;
	if (F4==2 && V[num].D==0) c4=1;
	if (num==0) c5=1;
	  else {
		if (F5==-1 && V[num].Z==0 && V[num].E==0) c5=1;
		if (F5==0) c5=1;
		if (F5>1 && V[num].Z==1) c5=1;
		if (F5==V[num].E) c5=1;
	};

  if ((num==32 || num==36) && F5==2) c5=0;

	// planet check
	if (num==10 && MarFlag==0) c6=0;
	if (num==12 && JupFlag==0) c7=0;
	if (num==13 && SatFlag==0) c8=0;

	if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8)  found=1;
	if (num==orig) return(0);

	if (found==0) {
			  if (num==56+plr) num=0;
			  else ++num;
	}

   }; /* end while */
  return(num);
}

int DownSearchRout(int num,char plr)
{
 int found=0,orig,c1=0,c2=0,c3=0,c4=0,c5=0,c6=1,c7=1,c8=1;
 orig=num;
 if (num<=0) num=56+plr;
   else --num;
 while (found==0)
   {
	c1=0;c2=0;c3=0;c4=0;c5=0;c6=1;c7=1;c8=1;
	if (F1==V[num].A) c1=1;
	if (F1==0 && V[num].A==1) c1=1; /* condition one is true */
	if (F1==2 && V[num].A==0) c1=1;
	if (F2==V[num].B) c2=1; /* condition two is true */
	if (F2==0 && V[num].B==1) c2=1; /* condition one is true */
	if (F2==2 && V[num].B==0) c2=1;
	if (F3==V[num].C) c3=1; /* condition three is true */
	if (F3==0 && V[num].C==1) c3=1; /* condition one is true */
	if (F3==2 && V[num].C==0) c3=1;
	if (F4==V[num].D) c4=1; /* condition four is true */
	if (F4==0 && V[num].D==1) c4=1; /* condition one is true */
	if (F4==2 && V[num].D==0) c4=1;
	if (num==0) c5=1;
	  else {
		if (F5==-1 && V[num].Z==0 && V[num].E==0) c5=1;  // locked on zero duration
		if (F5==0) c5=1;                  // nothing set
		if (F5>1 && V[num].Z==1) c5=1;    // set duration with duration mission
		if (F5==V[num].E) c5=1;           // the duration is equal to what is preset
	};

  if ((num==32 || num==36) && F5==2) c5=0;

	 // planet check
	if (num==10 && MarFlag==0) c6=0;
	if (num==12 && JupFlag==0) c7=0;
	if (num==13 && SatFlag==0) c8=0;
	if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8)  found=1;
	if (num==orig) return(0);

	if (found==0) {
			  if (num==0) num=56+plr;
			  else --num;
	}

   }; /* end while */
  return(num);
}

void
Future(char plr)
{
    /** \todo the whole Future()-function is 500 >lines and unreadable */
    TRACE1("->Future(plr)");
	int MisNum = 0, DuraType = 0, MaxDur = 6, i, ii;
	int setting = -1, prev_setting = -1;
	int Ok, NewType;
	GXHEADER local, local2;

	GV(&local, 166, 9);
	GV(&local2, 177, 197);
    GV(&vh,240,90);                  /* global variable */
  begfut:
	MisNum = FutureCheck(plr, 0);
	if (MisNum == 5)
	{
		DV(&local);
		DV(&local2);
        DV(&vh);
		return;
	}

	F1 = F2 = F3 = F4 = FMen = F5 = 0;
	// memset(buffer, 0x00, 20000);
	for (i = 0; i < 5; i++)
		lck[i] = status[i] = 0;
	SetParameters();
	strcpy(IDT, "i011");
	Pad = MisNum;
	DuraType = FMen = MisType = 0;
	ClrFut(plr, MisNum);
	DrawFuture(plr, MisType, MisNum);
begfut_noredraw:
  
//  for (i=0;i<5;i++) ClearRX(i+1);
	while (1)
	{
		GetMouse();
		if (mousebuttons == 0)
			break;
	}
	while (1)
	{
		GetMouse();

		prev_setting = setting;
		setting = -1;

		if (key == '-' && SEG > 1)
			SEG--;

		if (key == '+' && SEG < 500)
			SEG++;

		if (key >= 65 && key < Bub_Count + 65)
			setting = key - 65;

		for (ii = 0; ii < Bub_Count; ii++)
		{
			if (x >= StepBub[ii].x_cor && x <= StepBub[ii].x_cor + 7
				&& y >= StepBub[ii].y_cor && y <= StepBub[ii].y_cor + 7)
				setting = ii;
		}

		if (setting >= 0)
		{
			if (prev_setting < 0)
				gxGetImage(&local, 18, 186, 183, 194, 0);

			if (prev_setting != setting)
			{
				ShBox(18, 186, 183, 194);
				grSetColor(1);
				MisStep(21, 192, Mev[setting].loc);
			}
		}
		else if (setting < 0 && prev_setting >= 0)
		{
			gxPutImage(&local, gxSET, 18, 186, 0);
		}

		if (Mis.Dur <= V[MisType].E && ((x >= 244 && y >= 5 && x <= 313
					&& y <= 17 && mousebuttons > 0) || key == K_ENTER))
		{
			InBox(244, 5, 313, 17);
			WaitForMouseUp();
			if (key > 0)
				delay(300);
			key = 0;
			OutBox(244, 5, 313, 17);
			gxGetImage(&local2, 74, 3, 250, 199, 0);
			NewType = V[MisType].X;
			Data->P[plr].Future[MisNum].Duration = DuraType;

			Ok = HardCrewAssign(plr, Pad, MisType, NewType);

			gxPutImage(&local2, gxSET, 74, 3, 0);
			// DV(&local2);
			if (Ok == 1)
			{
				Data->P[plr].Future[MisNum].Duration = DuraType;
				goto begfut;	   // return to loop
			}
			else
			{
				ClrFut(plr, MisNum);
				// DuraType = FMen = MisType = 0;
				key = 0;
                goto begfut_noredraw;
				// DrawFuture(plr, MisType, MisNum);
			}
			key = 0;
		};
		// continue

		if ((((x >= 5 && y >= 49 && x <= 53 && y <= 72) || (x >= 43
						&& y >= 74 && x <= 53 && y <= 82))
				&& mousebuttons > 0) || (key == '!' || key == '1'))
		{
			if ((x >= 43 && y >= 74 && x <= 53 && y <= 82) || key == '!')
			{

				lck[0] = abs(lck[0] - 1);
				if (lck[0] == 1)
					InBox(43, 74, 53, 82);
				else
					OutBox(43, 74, 53, 82);
				if (lck[0] == 1)
					F5 = (status[0] == 0) ? -1 : status[0];
				if (lck[0] == 1)
					PlaceRX(1);
				else
					ClearRX(1);
				if (lck[0] == 0)
				{
					F5 = 0;
					status[0] = 0;
				}

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
			}
			else if (lck[0] != 1)
			{
				InBox(5, 49, 53, 72);

				if (DuraType == MaxDur)
					DuraType = 0;
				else
					DuraType++;
				Data->P[plr].Future[MisNum].Duration = DuraType;

				if (DuraType == 0)
					Toggle(5, 0);
				else if (DuraType == 1)
					Toggle(5, 1);
				if (DuraType != 0)
					draw_Pie(DuraType);

				status[0] = DuraType;

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
				grSetColor(34);
				OutBox(5, 49, 53, 72);
			};
			key = 0;
			/* Duration */
		};
		if ((x >= 5 && y >= 74 && x <= 41 && y <= 82 && mousebuttons > 0)
			|| (key == K_ESCAPE))
		{
			InBox(5, 74, 41, 82);
			while (1)
			{
				GetMouse();
				if (mousebuttons == 0)
					break;
			}
			MisType = 0;
			if (DuraType != 0)
				Toggle(5, 0);
			FMen = DuraType = F1 = F2 = F3 = F4 = F5 = 0;
			for (i = 1; i < 4; i++)
				if (status[i] != 0)
					Toggle(i, 1);
			if (JointFlag == 0)
			{
				F4 = 2;
				lck[4] = 1;
				Toggle(4, 1);
				InBox(191, 74, 201, 82);
				PlaceRX(5);
				TogBox(166, 49, 1);
			}
			else
			{
				F4 = 0;
				lck[4] = 0;
				status[4] = 0;
				Toggle(4, 1);
				OutBox(191, 74, 201, 82);
				ClearRX(5);
				TogBox(166, 49, 0);
			};
			for (i = 0; i < 4; i++)
			{
				lck[i] = status[i] = 0;
			}
			OutBox(5, 49, 53, 72);
			OutBox(43, 74, 53, 82);
			TogBox(55, 49, 0);
			OutBox(80, 74, 90, 82);
			TogBox(92, 49, 0);
			OutBox(117, 74, 127, 82);
			TogBox(129, 49, 0);
			OutBox(154, 74, 164, 82);

			ClrFut(plr, MisNum);
			Data->P[plr].Future[MisNum].Duration = 0;
			Missions(plr, 8, 37, MisType, 1);
			GetMinus(plr);
			OutBox(5, 74, 41, 82);
			key = 0;
			/* Reset */
		};
		if ((x >= 55 && y >= 49 && x <= 90 && y <= 82 && mousebuttons > 0)
			|| (key == '2' || key == '@'))
		{
			if ((x >= 80 && y >= 74 && x <= 90 && y <= 82) || (key == '@'))
			{

				if (lck[1] == 0)
					InBox(80, 74, 90, 82);
				else
					OutBox(80, 74, 90, 82);
				lck[1] = abs(lck[1] - 1);
				if (lck[1] == 1)
					PlaceRX(2);
				else
					ClearRX(2);
				if ((status[1] == 0) && (lck[1] == 1))
					F1 = 2;
				else if ((status[1] == 1) && (lck[1] == 1))
					F1 = 1;
				else
					F1 = 0;

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
			}
			else if (lck[1] != 1)
			{

				TogBox(55, 49, 1);
				if (status[1] == 0)
					Toggle(1, 1);
				else
					Toggle(1, 0);
				status[1] = abs(status[1] - 1);

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
				TogBox(55, 49, 0);
			};					   /* Docking */
			key = 0;
		};

		if ((x >= 92 && y >= 49 && x <= 127 && y <= 82 && mousebuttons > 0)
			|| (key == '3' || key == '#'))
		{
			if ((x >= 117 && y >= 74 && x <= 127 && y <= 82) || (key == '#'))
			{

				if (lck[2] == 0)
					InBox(117, 74, 127, 82);
				else
					OutBox(117, 74, 127, 82);
				lck[2] = abs(lck[2] - 1);
				if (lck[2] == 1)
					PlaceRX(3);
				else
					ClearRX(3);
				if ((status[2] == 0) && (lck[2] == 1))
					F2 = 2;
				else if ((status[2] == 1) && (lck[2] == 1))
					F2 = 1;
				else
					F2 = 0;

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
			}
			else if (lck[2] != 1)
			{

				TogBox(92, 49, 1);
				if (status[2] == 0)
					Toggle(2, 1);
				else
				{
					Toggle(2, 0);
				};
				status[2] = abs(status[2] - 1);

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
				TogBox(92, 49, 0);
			};					   /* EVA */
			key = 0;
		};

		if ((x >= 129 && y >= 49 && x <= 164 && y <= 82 && mousebuttons > 0)
			|| (key == '4' || key == '$'))
		{
			if ((x >= 154 && y >= 74 && x <= 164 && y <= 82) || (key == '$'))
			{

				if (lck[3] == 0)
					InBox(154, 74, 164, 82);
				else
					OutBox(154, 74, 164, 82);
				lck[3] = abs(lck[3] - 1);	// F3=lck[3];
				if (lck[3] == 1)
					PlaceRX(4);
				else
					ClearRX(4);
				if ((status[3] == 0) && (lck[3] == 1))
					F3 = 2;
				else if ((status[3] == 1) && (lck[3] == 1))
					F3 = 1;
				else
					F3 = 0;

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
			}
			else if (lck[3] != 1)
			{

				TogBox(129, 49, 1);
				if (status[3] == 0)
					Toggle(3, 1);
				else
				{
					Toggle(3, 0);
				};
				status[3] = abs(status[3] - 1);

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
				TogBox(129, 49, 0);
			};					   /* LEM */
			key = 0;
		};
		if (((x >= 166 && y >= 49 && x <= 201 && y <= 82 && mousebuttons > 0)
				|| (key == '5' || key == '%')) && (JointFlag == 1))
		{
			if ((x > 191 && y >= 74 && x <= 201 && y <= 82) || (key == '%'))
			{

				if (lck[4] == 0)
					InBox(191, 74, 201, 82);
				else
					OutBox(191, 74, 201, 82);
				lck[4] = abs(lck[4] - 1);
				if (lck[4] == 1)
					PlaceRX(5);
				else
					ClearRX(5);

				if ((status[4] == 0) && (lck[4] == 1))
					F4 = 2;
				else if ((status[4] == 1) && (lck[4] == 1))
					F4 = 1;
				else
					F4 = 0;

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
			}
			else if (lck[4] != 1)
			{

				TogBox(166, 49, 1);

				status[4] = abs(status[4] - 1);
				if (status[4] == 0)
				{
					Toggle(4, 1);
				}
				else
				{
					Toggle(4, 0);
				}

				while (1)
				{
					GetMouse();
					if (mousebuttons == 0)
						break;
				}
				TogBox(166, 49, 0);
			};					   /* Joint Launch */
			key = 0;
		};
		if ((x >= 5 && y >= 84 && x <= 16 && y <= 130 && mousebuttons > 0)
			|| (key == UP_ARROW))
		{
			InBox(5, 84, 16, 130);
			for (i = 0; i < 50; i++)
			{
				key = 0;
				GetMouse();
				delay(10);
				if (mousebuttons == 0)
				{
					MisType = UpSearchRout(MisType, plr);
					Data->P[plr].Future[MisNum].MissionCode = MisType;
					i = 51;
				}
			}
			while (mousebuttons == 1 || key == UP_ARROW)
			{
				MisType = UpSearchRout(MisType, plr);
				Data->P[plr].Future[MisNum].MissionCode = MisType;
				Missions(plr, 8, 37, MisType, 3);
				DuraType = status[0];
				delay(100);
				key = 0;
				GetMouse();
			}
            Missions(plr, 8, 37, MisType, 3);
			DuraType = status[0];
			OutBox(5, 84, 16, 130);
			key = 0;
			/* Mission Type plus */
		};
		if ((x >= 5 && y >= 132 && x < 16 && y <= 146 && mousebuttons > 0)
			|| (key == K_SPACE))
		{
			InBox(5, 132, 16, 146);
			WaitForMouseUp();
			delay(50);
  		    MisType = Data->P[plr].Future[MisNum].MissionCode;
  		    assert(0 <= MisType);
			if (MisType != 0){
                Missions(plr, 8, 37, MisType, 1);
			}
			else{
                Missions(plr, 8, 37, MisType, 3);
			}
			OutBox(5, 132, 16, 146);
			key = 0;
		}
		if ((x >= 5 && y >= 148 && x <= 16 && y <= 194 && mousebuttons > 0)
			|| (key == DN_ARROW))
		{
			InBox(5, 148, 16, 194);
			for (i = 0; i < 50; i++)
			{
				key = 0;
				GetMouse();
				delay(10);
				if (mousebuttons == 0)
				{
					MisType = DownSearchRout(MisType, plr);
					Data->P[plr].Future[MisNum].MissionCode = MisType;
					i = 51;
				}
				key = 0;
			}
			while (mousebuttons == 1 || key == DN_ARROW)
			{
				MisType = DownSearchRout(MisType, plr);
				Data->P[plr].Future[MisNum].MissionCode = MisType;
                Missions(plr, 8, 37, MisType, 3);
				DuraType = status[0];
				delay(100);
				key = 0;
				GetMouse();
			}
            Missions(plr, 8, 37, MisType, 3);
			DuraType = status[0];
			OutBox(5, 148, 16, 194);
			key = 0;
			/* Mission Type minus */

		};
	}							   // while
  TRACE1("<-Future()");
}

/** draws the bubble on the screen,
 * starts with upper left coor
 * 
 * \param x x-coord of the upper left corner of the bubble
 * \param y y-coord of the upper left corner of the bubble
 */
void Bd(int x,int y)
{
 int x1,y1,x2,y2;
 x1=x-2; y1=y; x2=x-1; y2=y-1;
 RectFill(x1,y1,x1+8,y1+4,21);
 RectFill(x2,y2,x2+6,y2+6,21);
 grSetColor(1);
 grMoveTo(x,y+4);
 /** \note references Bub_Count to determine the number of the character to draw in the bubble */
 DispChr(65+Bub_Count);
 StepBub[Bub_Count].x_cor=x1;
 StepBub[Bub_Count].y_cor=y1;
 ++Bub_Count;
 return;
}

/** Print the duration of a mission
 * 
 * \param x duration code
 * 
 * \todo Link this at whatever place the duration is actually defined
 */
void DurPri(int x) 
{
 grSetColor(5);
 switch(x)
  {
   case -1:PrintAt(112,30,"NO DURATION");break;
   case 0:PrintAt(112,30,"NO DURATION");break;
   case 1:PrintAt(112,30,"1 - 2 DAYS");break;
   case 2:PrintAt(112,30,"3 - 5 DAYS");break;
   case 3:PrintAt(112,30,"6 - 7 DAYS");break;
   case 4:PrintAt(112,30,"8 - 12 DAYS");break;
   case 5:PrintAt(112,30,"13 - 16 DAYS");break;
   case 6:PrintAt(112,30,"17 - 20 DAYS");break;
   default:break;
  };
 return;
}

void MissionName(int val,int xx,int yy,int len)
{
  TRACE5("->MissionName(val %d, xx %d, yy %d, len %d)", val, xx, yy, len);
  int i,j=0;

  GetMisType(val);

  grMoveTo(xx,yy);
  for (i=0;i<50;i++) {
    if (j>len && Mis.Name[i]==' ') {yy+=7;j=0;grMoveTo(xx,yy);}
    else DispChr(Mis.Name[i]);
    j++;if (Mis.Name[i]=='\0') break;
  };
  TRACE1("<-MissionName");
  return;
}

/** Missions() will draw the future missions among other things
 * 
 * \param plr Player
 * \param X screen coord for mission name string
 * \param Y screen coord for mission name string
 * \param val mission number
 * \param bub if set to 0 or 3 the function will not draw stuff
 */
void Missions(char plr,int X,int Y,int val,char bub)
{
  TRACE5("->Missions(plr, X %d, Y %d, val %d, bub %c)", X, Y, val, bub);
  
  if (bub==1 || bub==3) {
    PianoKey(val);
    Bub_Count=0;   // set the initial bub_count
    ClearDisplay();
    RectFill(6,31,182,46,3);
    RectFill(80,25,175,30,3);grSetColor(5);
    PrintAt(55,30,"TYPE: ");DispNum(0,0,val);
    grSetColor(5);
    if (V[val].E>0) {
      if (F5 > V[val].E && Mis.Dur==1) DurPri(F5);
	     else DurPri(V[val].E);}
	      else DurPri(F5);
  } else grSetColor(1);
  MissionName(val,X,Y,24);
  if (bub==3) GetMinus(plr);
  if (bub==0 || bub==3) {return;}
  
    
	MSteps=sOpen("missSteps.dat","r",FT_DATA);
	if (fgets(missStep, 1024, MSteps) == NULL)
		memset (missStep, 0, sizeof missStep);

	while (!feof(MSteps)&&((missStep[0]-0x30)*10+(missStep[1]-0x30))!=val) {
		if (fgets(missStep, 1024, MSteps) == NULL)
			break;
	}
	fclose(MSteps);

  int n;
	for (n=2;missStep[n]!='Z';n++)
		switch (missStep[n]) {
			case 'A': Draw_IJ	(B_Mis(++n));	break;
			case 'B': Draw_IJV	(B_Mis(++n));	break;
			case 'C': OrbOut	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3));	n+=3;	break;
			case 'D': LefEarth	(B_Mis(n+1),B_Mis(n+2));	n+=2; 	break;
			case 'E': OrbIn		(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3));	n+=3; 	break;
			case 'F': OrbMid	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4));	n+=4;	break;
			case 'G': LefOrb	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4));	n+=4;	break;
			case 'H': Draw_LowS	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4),B_Mis(n+5),B_Mis(n+6));	n+=6;	break;
			case 'I': Fly_By	();		break;
			case 'J': VenMarMerc	(B_Mis(++n));	break;
			case 'K': Draw_PQR	();		break;
			case 'L': Draw_PST	();		break;
			case 'M': Draw_GH	(B_Mis(n+1),B_Mis(n+2));	n+=2;	break;
			case 'N': Q_Patch	();		break;
			case 'O': RghtMoon	(B_Mis(n+1),B_Mis(n+2));	n+=2; 	break;
			case 'P': DrawLunPas	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4));	n+=4;	break;
			case 'Q': DrawLefMoon	(B_Mis(n+1),B_Mis(n+2)); 	n+=2;	break;
			case 'R': DrawSTUV	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4));	n+=4;	break;
			case 'S': Draw_HighS	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3));	n+=3;	break;
			case 'T': DrawMoon	(B_Mis(n+1),B_Mis(n+2),B_Mis(n+3),B_Mis(n+4),B_Mis(n+5),B_Mis(n+6),B_Mis(n+7));	n+=7;	break;
			case 'U': LefGap	(B_Mis(++n));	break;
			case 'V': S_Patch	(B_Mis(++n));	break;
			case 'W': DrawZ		();		break;
			default : break;
		}
  gr_sync ();
  MissionCodes(plr,MisType,Pad);
  TRACE1("<-Missions()");
}  // end function missions

#ifdef DEAD_CODE
/** Draws stuff about choosing a program and having < 2 groups available
 * 
 * \deprecated This function appears to be deprecated. 
 */
char FutBad(void)
{
  char i;
  
  grSetColor(0);
  ShBox(84,41,232,128);
  InBox(91,47,225,103);
  IOBox(91,107,225,123);
  grSetColor(1);
  PrintAt(150,117,"EXIT");
  grSetColor(11);
  PrintAt(96,60,"YOU HAVE SELECTED A");
  PrintAt(96,70,"PROGRAM WITH LESS THAN");
  PrintAt(96,80,"TWO GROUPS AVAILABLE.");
  
  WaitForMouseUp();
  i=0;
  while(i==0) {
  GetMouse();
   if (mousebuttons!=0) {
     if (x>=93 && y>=109 && x<=223 && y<=121) {
       InBox(93,109,223,123);i=3;
       delay(50);
     };
   };
  }; /* End while */
  return (i);
}
#endif

/* vim: set noet ts=4 sw=4 tw=77: */
