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
/** \file endgame.c End Game Routines
 */

#include <Buzz_inc.h>
#include <externs.h>

#define NUM_LIGHTS 100
#define FLY_TIME 20
#define GRAVITY 0.6
#define FRICTION 0.3
#define PI 3.1415926
#define MAXINITSPEED 270
#define MAXWAIT 1
#define PutPixel(x,y,col) grPutPixel(x,y,col)

const int draw_projectiles = 0;

extern char Month[12][11];
extern char AI[2];
extern char Nums[30][7];
extern char Option,MAIL,manOnMoon,dayOnMoon;
extern char daysAMonth[12];
char month,firstOnMoon, capName[30]; 
char PF[29][40]={
    "ORBITAL SATELLITE","LUNAR FLYBY","MERCURY FLYBY","VENUS FLYBY",
    "MARS FLYBY","JUPITER FLYBY","SATURN FLYBY","LUNAR PROBE LANDING",
    "DURATION LEVEL F","DURATION LEVEL E","DURATION LEVEL D",
    "DURATION LEVEL C","DURATION LEVEL B","ONE-PERSON CRAFT",
    "TWO-PERSON CRAFT","THREE-PERSON CRAFT","MINISHUTTLE","FOUR-PERSON CRAFT",
    "MANNED ORBITAL","MANNED LUNAR PASS","MANNED LUNAR ORBIT",
    "MANNED RESCUE ATTEMPT","MANNED LUNAR LANDING","ORBITING LAB",
    "MANNED DOCKING","WOMAN IN SPACE","SPACE WALK","MANNED SPACE MISSION"};


char
Burst(char win)
{
	float Spsn[2];
	char R_value = 0;
	struct PROJECTILE
	{
		char clr;
		float vel[2];
		float psn[2];
		i16 per;
	} Bomb[NUM_LIGHTS];
	int lp1, lp2, Region, xx, yy;
	float Ang, Spd, InitSpd;
	char clr = 1;

	key = 0;
	strncpy(IDT, "i144", 4);
	strncpy(IKEY, "k044", 4);
	gxGetImage(&vhptr, 0, 0, 319, 199, 0);
	while (1)
	{
		Region = random(100);
		if (Region < 60)
		{
			Spsn[0] = 132 + random(187);
			Spsn[1] = 5 + random(39);
		}
		else
		{
			Spsn[0] = 178 + random(66);
			Spsn[1] = 11 + random(33);
		}
		InitSpd = random(MAXINITSPEED);
		for (lp1 = 0; lp1 < NUM_LIGHTS; lp1++)
		{
			Ang = random(2 * PI);
			Spd = random(InitSpd);
			Bomb[lp1].psn[0] = Spsn[0];
			Bomb[lp1].psn[1] = Spsn[1];
			Bomb[lp1].vel[0] = Spd * cos(Ang);
			Bomb[lp1].vel[1] = Spd * sin(Ang);
			Bomb[lp1].clr = clr;
			Bomb[lp1].per = random(FLY_TIME);
		}
		for (lp1 = 0; lp1 < FLY_TIME; lp1++)
		{
			for (lp2 = 0; lp2 < NUM_LIGHTS; lp2++)
			{
				xx = Bomb[lp2].psn[0];
				yy = Bomb[lp2].psn[1];
				/* This is overkill for pixels, but let's see... */
				if (xx >= 0 && xx < 320 && yy >= 0 && yy <= 172)
				{
					PutPixel(xx, yy, vhptr.vptr[xx + 320 * yy]);
					if (draw_projectiles)
						av_need_update_xy(xx, yy, xx, yy);
				}
				key = 0;
				/* We can't wait 30 ms on default timer */
				if (draw_projectiles)
					GetMouse_fast();
				else
					GetMouse();
				if (key > 0 || mousebuttons > 0)
				{
					if ((x >= 14 && y >= 182 && x <= 65 && y <= 190
							&& mousebuttons > 0) || key == 'H')
						R_value = 1;
					if ((x >= 74 && y >= 182 && x <= 125 && y <= 190
							&& mousebuttons > 0) || key == 'S')
						R_value = 2;
					if ((x >= 134 && y >= 182 && x <= 185 && y <= 190
							&& mousebuttons > 0) || key == 'P')
						R_value = 3;
					if ((x >= 194 && y >= 182 && x <= 245 && y <= 190
							&& mousebuttons > 0) || key == 'M')
						R_value = 4;
					if ((x >= 254 && y >= 182 && x <= 305 && y <= 190
							&& mousebuttons > 0) || key == K_ENTER)
						R_value = 5;
					if (R_value > 0)
					{

						gxPutImage(&vhptr, gxSET, 0, 0, 0);
						strncpy(IDT, "i144", 4);
						strncpy(IKEY, "k044", 4);

						return (R_value);
					}
				}
				Bomb[lp2].vel[1] = Bomb[lp2].vel[1] + GRAVITY;
				Bomb[lp2].vel[0] = Bomb[lp2].vel[0] * FRICTION;
				Bomb[lp2].vel[1] = Bomb[lp2].vel[1] * FRICTION;

				Bomb[lp2].psn[0] =
					(float) Bomb[lp2].psn[0] + Bomb[lp2].vel[0];
				Bomb[lp2].psn[1] =
					(float) Bomb[lp2].psn[1] + Bomb[lp2].vel[1];
				xx = Bomb[lp2].psn[0];
				yy = Bomb[lp2].psn[1];
				if (win == 0)
				{
					if (clr == 1)
						clr = 6;
					else if (clr == 6)
						clr = 9;
					else if (clr == 9)
						clr = 1;
				}
				else
				{
					if (clr == 1)
						clr = 9;
					else if (clr == 9)
						clr = 11;
					else if (clr == 11)
						clr = 9;
				}
				if (lp1 < Bomb[lp2].per && (xx >= 0 && xx < 320 && yy >= 0
						&& yy <= 172))
				{
					PutPixel(xx, yy, clr);
					if (draw_projectiles)
						av_need_update_xy(xx, yy, xx, yy);
				}
			}
			/* XXX: need to optimize SDL_Scale2x for this to work */
			if (draw_projectiles)
				av_sync();
		}
		for (lp2 = 0; lp2 < NUM_LIGHTS; lp2++)
		{
			xx = Bomb[lp2].psn[0];
			yy = Bomb[lp2].psn[1];
			if (xx >= 0 && xx < 320 && yy >= 0 && yy <= 172)
			{
				PutPixel(xx, yy, vhptr.vptr[xx + 320 * yy]);
				if (draw_projectiles)
					av_need_update_xy(xx, yy, xx, yy);
			}
		}
	}							   // end while

}

void EndGame(char win,char pad)
{
 int i=0,r,gork;
 char miss,prog,man1,man2,man3,man4,bud;

 
 FadeOut(2,pal,10,0,0);
 strncpy(IDT,"i000",4);strncpy(IKEY,"k000",4);
 gxClearDisplay(0,0);
 ShBox(0,0,319,22);InBox(3,3,30,19);
 IOBox(242,3,315,19);
 ShBox(0,24,319,199);RectFill(5,28,314,195,0);
 RectFill(5,105,239,110,3);ShBox(101,102,218,113);
 grSetColor(6);PrintAt(112,110,"ALTERNATE HISTORY");
 if (win==0) DispBig(34,5,"US WINS",1,-1);
   else DispBig(34,5,"USSR WINS",1,-1);
 FlagSm(win,4,4);
 grSetColor(1);PrintAt(258,13,"CONTINUE");

 if (Option==-1 && MAIL==-1) miss=Data->P[win].Mission[pad].MissionCode;
  else {
   miss=Data->P[win].History[Data->Prestige[22].Indec].MissionCode;
  }

 grSetColor(6);PrintAt(10,50,"MISSION TYPE: ");grSetColor(8);
  if (miss==55  || miss==56 || miss==57) i=1; else i=0;

 MissionName(miss,80,50,24);

 if (Option==-1 && MAIL==-1)
  {
	strcpy(capName , Data->P[win].Mission[pad].Name);
	month   = Data->P[win].Mission[pad].Month;
  }
 else {
	month   = Data->Prestige[22].Month;
   if (MAIL!=-1 || Option==win) strcpy(capName , Data->P[win].History[Data->Prestige[22].Indec].MissionName[0]);
   else {
     prog=Data->P[win].History[Data->Prestige[22].Indec].Hard[i][0]+1;
     strcpy(capName , &Data->P[win].Manned[prog-1].Name[0]);
     strcat(capName , " ");
     strcat(capName , Nums[Data->P[win].Manned[prog-1].Used]);
   }
 }

	grSetColor(6);PrintAt(10,40,"MISSION: ");grSetColor(8);PrintAt(0,0,capName);
	grSetColor(6);PrintAt(0,0,"  -  ");grSetColor(8);DispNum(0,0,dayOnMoon);PrintAt(0,0," ");
	PrintAt(0,0,Month[month]);PrintAt(0,0,"19");DispNum(0,0,Data->Year);

// correct mission pictures
 bud=0; // initialize bud
 gork=0;

 if (Option==-1 && MAIL==-1) gork=Data->P[win].PastMis-1;
  else gork=Data->Prestige[22].Indec;

 if (win==1 && Data->P[win].History[gork].Hard[i][0]>=3) bud=5;
   else if (win==0 && Data->P[win].History[gork].Hard[i][0]==4) bud=2;
     else bud=((Data->P[win].History[gork].Hard[i][2]-5)+(win*3));
 if (bud<0 || bud>5) bud=0+win;
 InBox(241,67,313,112);EndPict(242,68,bud,128);
 PatchMe(win,270,34,Data->P[win].History[gork].Hard[i][0],Data->P[win].History[gork].Patch[win],32);
 man1=Data->P[win].History[gork].Man[i][0]; 
 man2=Data->P[win].History[gork].Man[i][1]; 
 man3=Data->P[win].History[gork].Man[i][2]; 
 man4=Data->P[win].History[gork].Man[i][3]; 
// no astronaut klugge
 r=Data->P[win].AstroCount;
 if (man1<=-1) man1=random(r);
 if (man2<=-1) man2=random(r);
 if (man3<=-1) man3=random(r);
 if (man4<=-1) man4=random(r);
 if (!(Option==-1 || Option==win))
  {
   Data->P[win].History[gork].Man[i][0]=man1; 
   Data->P[win].History[gork].Man[i][1]=man2; 
   Data->P[win].History[gork].Man[i][2]=man3; 
   Data->P[win].History[gork].Man[i][3]=man4; 
  }
 prog=Data->P[win].History[gork].Hard[i][0]+1;
 for (i=1;i<5;i++)
   {
	grSetColor(6);
	switch(i)
	  {
	   case 1:if (prog==1) PrintAt(10,70,"CAPSULE PILOT - EVA: ");
             else if (prog==2) PrintAt(10,70,"CAPSULE PILOT - DOCKING: ");
 			      else if (prog>=3) PrintAt(10,70,"COMMAND PILOT: ");
			    grSetColor(8);
			    if (man1!=-1)
			    PrintAt(0,0,&Data->P[win].Pool[man1].Name[0]);
			    break;
	   case 2:if (prog>1 && prog<5)
				  PrintAt(10,79,"LM PILOT - EVA: ");
				   else if (prog==5) PrintAt(10,79,"LUNAR PILOT: ");
			    grSetColor(8);
			    if (man2!=-1)
			     PrintAt(0,0,&Data->P[win].Pool[man2].Name[0]);
			    break;
	   case 3:if (prog>2 && prog<5)
				  PrintAt(10,88,"DOCKING SPECIALIST: ");
				 else if (prog==5) PrintAt(10,88,"EVA SPECIALIST: ");
			    grSetColor(8);
			    if (man3!=-1 && prog>2)
			     PrintAt(0,0,&Data->P[win].Pool[man3].Name[0]);
			   break;
	   case 4:if (prog==5)
              {
               PrintAt(10,97,"EVA SPECIALIST: ");
			      grSetColor(8);
			      if (man4!=-1)
				   PrintAt(0,0,&Data->P[win].Pool[man4].Name[0]);
              }
			   break;
	   default:break;
	  }
	 }
 //Print the first in the moon
 firstOnMoon = (manOnMoon==1? man1 : manOnMoon==2? man2 : manOnMoon==3? man3 :manOnMoon==4? man4: man2);
 grSetColor(11);PrintAt(10,60,"FIRST ON THE MOON: ");grSetColor(14);PrintAt(0,0,&Data->P[win].Pool[firstOnMoon].Name[0]);
 
 grSetColor(6);
 AltHistory(win);
 FadeIn(2,pal,10,0,0);
 
 WaitForMouseUp();
 i=0;key=0;
 while (i==0)
  {
	key=0;GetMouse();
	if ((x>=244 && y>=5 && x<=313 && y<=17 && mousebuttons>0) || key==K_ENTER)
	  {
	   InBox(244,5,313,17);
	   WaitForMouseUp();
      if (key>0) delay(150);
	   i=1;key=0;
	   OutBox(244,5,313,17);
	  };
  }
 return;
}

void Load_LenFlag(char win)
{
	PatchHdr P;
  GXHEADER local,local2;
  unsigned int coff;
  int j,Off_X,Off_Y;
  char poff;
  FILE *in;
 if (win==1) {in=sOpen("LENIN.BUT","rb",0);Off_X=224;Off_Y=26;}
  else {in=sOpen("FLAGGER.BUT","rb",0);Off_X=195;Off_Y=0;}
 poff=0;coff=128;
 fread(&pal[coff*3],384,1,in);
 fseek(in,(poff)*(sizeof P),SEEK_CUR);
 fread(&P,sizeof P,1,in);
	SwapPatchHdr(&P);
	if (win!=1)
		P.w++; /* BUGFIX as everywhere */
 fseek(in,P.offset,SEEK_SET);
 GV(&local,P.w,P.h); GV(&local2,P.w,P.h);
 gxClearVirtual(&local2,0);
 gxGetImage(&local2,Off_X,Off_Y,Off_X+P.w-1,Off_Y+P.h-1,0);
 fread(local.vptr,P.size,1,in);
 fclose(in);
 for (j=0;j<P.size;j++)
	/* now fix the strip */
	if (win == 1 || ((j+1) % P.w != 0))
		local2.vptr[j]=local.vptr[j]+coff;

 gxPutImage(&local2,gxSET,Off_X,Off_Y,0);
 DV(&local); DV(&local2);
 return;
}

void Draw_NewEnd(char win)
{
 long size;
 FILE *in;
 
 music_start(M_VICTORY);

 FadeOut(2,pal,10,0,0);
 gxClearDisplay(0,0);
 in=sOpen("WINNER.BUT","rb",0);
 fread(pal,384,1,in);
 size=fread(vhptr.vptr,1,vhptr.h*vhptr.w,in);
 fclose(in);
 PCX_D(vhptr.vptr,screen,size);
 ShBox(0,173,319,199);InBox(5,178,314,194);
 IOBox(12,180,67,192);IOBox(72,180,127,192);
 IOBox(132,180,187,192);IOBox(192,180,247,192);IOBox(252,180,307,192);
 grSetColor(1);
 PrintAt(21,188,"HISTORY");PrintAt(85,188,"STATS");PrintAt(142,188,"PARADE");
 PrintAt(198,188,"MOON EVA");PrintAt(269,188,"EXIT");
 FadeIn(0,pal,10,128,0);
 Load_LenFlag(win);
 FadeIn(1,pal,40,128,1);
}

void NewEnd(char win,char loc)
{
 GXHEADER local;
 int i,Re_Draw=0;
 char R_V=0;

 music_start(M_VICTORY);
 EndGame(win,loc);
 Draw_NewEnd(win);
 R_V=Burst(win);
 WaitForMouseUp();
 i=0;key=0;
 GV(&local,162,92);gxClearVirtual(&local,0);

 while (i==0)
 {
	 key=0;GetMouse();
   strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
	 
	 music_start(M_VICTORY);

   if (((key=='P' || key=='M' || key=='H' || key=='S') || mousebuttons>0) || R_V==0)
    if (Re_Draw==1)
     {
      if ((x>=14 && y>=182 && x<=65 && y<=190 && mousebuttons>0) || key=='H') R_V=1;
      if ((x>=74 && y>=182 && x<=125 && y<=190 && mousebuttons>0) || key=='S') R_V=2;
      if ((x>=134 && y>=182 && x<=185 && y<=190 && mousebuttons>0) || key=='P') R_V=3;
      if ((x>=194 && y>=182 && x<=245 && y<=190 && mousebuttons>0) || key=='M') R_V=4;
      if ((x>=254 && y>=182 && x<=305 && y<=190 && mousebuttons>0) || key==K_ENTER) R_V=5;
      
      gxPutImage(&local,gxSET,149,9,0);
      memset(&pal[384],0,384);gxSetDisplayPalette(pal);
      gxClearVirtual(&local,0);
      Load_LenFlag(win);
      FadeIn(1,pal,40,128,1);
      
      if (R_V==0 || R_V==-1) R_V=Burst(win);
      Re_Draw=0;
      strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
     }
   if (((x>=14 && y>=182 && x<=65 && y<=190 && mousebuttons>0) || key=='H') || R_V==1)
    {
		 // History box
     InBox(14,182,65,190);
     WaitForMouseUp();
     if (key>0 || R_V>0) delay(150);
     i=0;key=0;
     OutBox(14,182,65,190);
     EndGame(win,loc);Draw_NewEnd(win);
     strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
     R_V=0;
     R_V=Burst(win);
    }
   if (((x>=74 && y>=182 && x<=125 && y<=190 && mousebuttons>0) || key=='S') || R_V==2)
    {
		 // Stats box
		 music_stop();
     InBox(74,182,125,190);
     WaitForMouseUp();
     if (key>0 || R_V>0) delay(150);
     i=0;key=0;
     OutBox(74,182,125,190);
     music_start(M_THEME);
     Stat(win);Draw_NewEnd(win);
     strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
     R_V=0;
     R_V=Burst(win);
    }
	if (((x>=134 && y>=182 && x<=185 && y<=190 && mousebuttons>0) || key=='P') || R_V==3)
	{
		 // Parade
		 music_stop();
     InBox(134,182,185,190);
     WaitForMouseUp();
     if (key>0 || R_V>0) delay(150);
     if (R_V==3) R_V=-1;
     i=0;key=0;
     Re_Draw=1;OutBox(134,182,185,190);
     FadeOut(1,pal,40,128,1);RectFill(195,0,319,172,0);
     gxGetImage(&local,149,9,309,100,0);
     ShBox(149,9,309,100);InBox(153,13,305,96);
     music_start(M_PRGMTRG);
     Replay(win,0,154,14,149,82,(win==0)?"UPAR":"SPAR");
		 music_stop();
     strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
    }
   if (((x>=194 && y>=182 && x<=245 && y<=190 && mousebuttons>0) || key=='M') || R_V==4)
    {
		 // Moon EVA
		 music_stop();
     InBox(194,182,245,190);
     WaitForMouseUp();
     if (key>0 || R_V>0) delay(150);
     if (R_V==4) R_V=-1;
     i=0;key=0;
     OutBox(194,182,245,190);Re_Draw=1;
     FadeOut(1,pal,40,128,1);RectFill(195,0,319,172,0);
     gxGetImage(&local,149,9,309,100,0);
     ShBox(149,9,309,100);InBox(153,13,305,96);
     music_start(M_MISSPLAN);
     Replay(win,0,154,14,149,82,(win==0) ? "PUM3C6":"PSM3C6");
		 music_stop();
     strncpy(IDT,"i144",4);strncpy(IKEY,"k044",4);
    }
   if (((x>=254 && y>=182 && x<=305 && y<=190 && mousebuttons>0) || key==K_ENTER) || R_V==5)
    {
     music_stop();
     InBox(254,182,305,190);
     WaitForMouseUp();
     if (key>0) delay(150);
     i=1;key=0;
     OutBox(254,182,305,190);
    };
  }
 DV(&local);
 return;
}

void FakeWin(char win)
{
 int i,r;
 char miss,prog,man1,man2,man3,man4,bud,yr,monthWin;
 monthWin=random(12);
 
 FadeOut(2,pal,10,0,0);
 gxClearDisplay(0,0);
 ShBox(0,0,319,22);InBox(3,3,30,19);
 IOBox(242,3,315,19);
 ShBox(0,24,319,199);RectFill(5,28,314,195,0);
 RectFill(5,105,239,110,3);ShBox(101,102,218,113);
 grSetColor(6);PrintAt(112,110,"ALTERNATE HISTORY");
 if (win==0) DispBig(36,5,"US WINS",1,-1);
   else DispBig(36,5,"USSR WINS",1,-1);
 FlagSm(win,4,4);
 grSetColor(1);PrintAt(258,13,"CONTINUE");
 r=random(100);
 if (r<45) miss=53;
  else if (r<50) miss=54;
   else if (r<85) miss=55;
    else miss=56;

 grSetColor(6);PrintAt(10,50,"MISSION TYPE: ");grSetColor(8);
 if (miss==55  || miss==56) i=1; else i=0;
 MissionName(miss,80,50,24);
 grSetColor(6);

 if (Data->Year<=65) r=65+random(5);
  else if (Data->Year<=70) r=70+random(3);
    else if (Data->Year<=77) r=Data->Year;

 yr=r;
 r=random(100);
 if (miss==54) prog=5;
  else if (r<20) prog=2;
    else if (r<60) prog=3;
     else prog=4;
 grSetColor(6);PrintAt(10,40,"MISSION: ");grSetColor(8);PrintAt(0,0,&Data->P[win].Manned[prog-1].Name[0]);
 PrintAt(0,0," ");PrintAt(0,0,&Nums[random(15)+1][0]);grSetColor(6);
 PrintAt(0,0,"  -  ");
 grSetColor(8);;DispNum(0,0, random(daysAMonth[monthWin])+1);PrintAt(0,0," ");
 PrintAt(0,0,Month[monthWin]);PrintAt(0,0,"19");DispNum(0,0,yr);
 bud=0; // initialize bud
 r=random(100);
 if (win==1 && prog==5) bud=5;
   else if (win==0 && prog==5) bud=2;
     else bud= (r<50) ? 0+(win*3) : 1+(win*3);
 if (bud<0 || bud>5) bud=0+win;
 InBox(241,67,313,112);EndPict(242,68,bud,128);
 PatchMe(win,270,34,prog-1,random(9),32);
 r=Data->P[win].AstroCount;
 man1=random(r);man2=random(r);
 man3=random(r);man4=random(r);
 while(1)
  {
   if ((man1!=man2) && (man1!=man3) && (man2!=man4) &&
     (man2!=man3) && (man3!=man4) && (man1!=man4)) break;
   while (man1==man2) man2=random(r);
   while (man1==man3) man3=random(r);
   while (man2==man4) man2=random(r);
   while (man2==man3) man3=random(r);
   while (man3==man4) man4=random(r);
   while (man1==man4) man4=random(r);
  }
 for (i=1;i<5;i++)
   {
	grSetColor(6);
	switch(i)
	  {
	   case 1: if (prog>=1 && prog<=3)
				 PrintAt(10,70,"CAPSULE PILOT - EVA: ");
			   if (prog>3)
				 PrintAt(10,70,"COMMAND PILOT: ");
			   grSetColor(8);
			   if (man1!=-1)
			   PrintAt(0,0,&Data->P[win].Pool[man1].Name[0]);
			   break;
	   case 2: if (prog>1 && prog<5)
				 PrintAt(10,79,"LM PILOT - EVA: ");
				   else if (prog==5) PrintAt(10,79,"LUNAR PILOT: ");
			   grSetColor(8);
			   if (man2!=-1 && (prog>1 && prog<5))
			   PrintAt(0,0,&Data->P[win].Pool[man2].Name[0]);
			   break;
	   case 3: if (prog>2 && prog<5)
				 PrintAt(10,88,"DOCKING SPECIALIST: ");
				else if (prog==5) PrintAt(10,88,"EVA SPECIALIST: ");
			   grSetColor(8);
			   if (man3!=-1 && prog>2)
			   PrintAt(0,0,&Data->P[win].Pool[man3].Name[0]);
			   break;
	   case 4: if (prog==5)
               {
                PrintAt(10,97,"EVA SPECIALIST: ");
			       grSetColor(8);
			       if (man4!=-1 && prog==5)
				    PrintAt(0,0,&Data->P[win].Pool[man4].Name[0]);
               }
			   break;
	   default:break;
	  }
	 }
 manOnMoon=man2; if (prog==3 || prog==4) manOnMoon=man1;
 grSetColor(11);PrintAt(10,60,"FIRST ON THE MOON: ");grSetColor(14);PrintAt(0,0,&Data->P[win].Pool[manOnMoon].Name[0]);
 grSetColor(6);
 FakeHistory(win,yr);
 music_start(M_INTERLUD);
 FadeIn(2,pal,10,0,0);
 
 WaitForMouseUp();
 i=0;key=0;
  while (i==0)
  {
	key=0;GetMouse();
	if ((x>=244 && y>=5 && x<=313 && y<=17 && mousebuttons>0) || key==K_ENTER)
	  {
	   InBox(244,5,313,17);
	   WaitForMouseUp();
      if (key>0) delay(150);
	   i=1;key=0;
	   OutBox(244,5,313,17);
	  };
  }
 music_stop();
 return;
}

void FakeHistory(char plr,char Fyear)  // holds the winning player
{
 char bud;
 memset(buffer, 0, BUFFER_SIZE);
 if (Fyear<=65) bud=0+plr;
   else if (Fyear<=67) bud=2+plr;
	 else if (Fyear<=69) bud=4+plr;
	   else if (Fyear<=71) bud=6+plr;
		 else if (Fyear>=72) bud=8+plr;
		   else bud=10+plr;
 HistFile(buffer+1000,bud);
 PrintHist(buffer+1000);
 return;
}

void HistFile(char *buf,unsigned char bud)
{
  FILE *fin; long i;
  i=bud*600;
  fin=sOpen("ENDGAME.DAT","rb",0);
  fseek(fin,i,SEEK_SET);
  fread(buf,600,1,fin);
  fclose(fin);
}

void PrintHist(char *buf)
{
 int i,k;
 grSetColor(8);
 k=121;grMoveTo(10,k);
  for (i=0;i<(int)strlen(buf);i++)
	{
	 if (buf[i]=='*') {k+=7;grMoveTo(10,k);}
	   else DispChr(buf[i]);
	}
}

void PrintOne(char *buf,char tken)
{
 int i,k;
 grSetColor(7);
 k=0;
 if (tken==0)  k=127; else k=170;
 grMoveTo(10,k);
  for (i=0;i<(int)strlen(buf);i++)
	{
	 if (buf[i]=='*') {k+=7;grMoveTo(10,k);}
	   else DispChr(buf[i]);
	}
}

void AltHistory(char plr)  // holds the winning player
{
 char bud;
 memset(buffer, 0, BUFFER_SIZE);
 if (Data->Year<=65) bud=0+plr;
   else if (Data->Year<=67) bud=2+plr;
	 else if (Data->Year<=69) bud=4+plr;
	   else if (Data->Year<=71) bud=6+plr;
		 else if (Data->Year>=72) bud=8+plr;
		   else bud=10+plr;
 HistFile(buffer+1000,bud);
 PrintHist(buffer+1000);
 return;
}

void SpecialEnd(void)
{
 char i;
 music_start(M_BADNEWS);
 
 gxClearDisplay(0,0);
 ShBox(0,0,319,24);DispBig(5,5,"FAILED OBJECTIVE",1,-1);
 ShBox(0,26,319,199);RectFill(1,27,318,198,7);InBox(5,31,314,194);
 RectFill(6,32,313,193,3);
 IOBox(242,3,315,19);grSetColor(1);PrintAt(258,13,"CONTINUE");
 ShBox(6,109,313,119);ShBox(6,151,313,161);
 grSetColor(9);PrintAt(130,116,"UNITED STATES");PrintAt(134,158,"SOVIET UNION");
 RectFill(6,32,313,108,0);
 InBox(178,3,205,19);FlagSm(0,179,4);
 InBox(210,3,237,19);FlagSm(1,211,4);
 LoserPict(0,128); // load loser picture 
 memset(buffer,0x00,BUFFER_SIZE);    
 HistFile(buffer+1000,10);
 PrintOne(buffer+1000,0);
 memset(buffer,0x00,BUFFER_SIZE);
 HistFile(buffer+1000,11);
 PrintOne(buffer+1000,1);
 FadeIn(2,pal,10,0,0);
 
 WaitForMouseUp();
 i=0;key=0;
 while (i==0)
  {
   key=0;GetMouse();
	 if ((x>=244 && y>=5 && x<=313 && y<=17 && mousebuttons>0) || key==K_ENTER)
	  {
	   InBox(244,5,313,17);
	   WaitForMouseUp();
      if (key>0) delay(150);
	   i=1;key=0;
	  };
  }
 music_stop();
 return;
}

void
EndPict(int x, int y, char poff, unsigned char coff)
{
	PatchHdrSmall P;
	GXHEADER local, local2;
	unsigned int j;
	FILE *in;

	in = sOpen("ENDGAME.BUT", "rb", 0);
	fread(&pal[coff * 3], 384, 1, in);
	fseek(in, (poff) * (sizeof P), SEEK_CUR);
	fread(&P, sizeof P, 1, in);
	SwapPatchHdrSmall(&P);
	/*
	 * off by one error in data file - again
	 * P.w += 1 solves the problem, but then
	 * we get a strip of garbage on the right hand side
	 */
	P.w++;
	fseek(in, P.offset, SEEK_SET);
	GV(&local, P.w, P.h);
	GV(&local2, P.w, P.h);
	gxGetImage(&local2, x, y, x + P.w - 1, y + P.h - 1, 0);
	fread(local.vptr, P.size, 1, in);
	fclose(in);
	for (j = 0; j < P.size; j++)
		/* fix the strip */
		if (local.vptr[j] != 0 && ((j+1) % P.w != 0))
			local2.vptr[j] = local.vptr[j] + coff;
	gxPutImage(&local2, gxSET, x, y, 0);
	DV(&local);
	DV(&local2);
	return;
}

void
LoserPict(char poff, unsigned char coff)
{
	/* This hasn't got an off-by-one...*/
	PatchHdr P;
	GXHEADER local, local2;
	unsigned int j;
	FILE *in;

	in = sOpen("LOSER.BUT", "rb", 0);
	fread(&pal[coff * 3], 384, 1, in);
	fseek(in, (poff) * (sizeof P), SEEK_CUR);
	fread(&P, sizeof P, 1, in);
	SwapPatchHdr(&P);
	fseek(in, P.offset, SEEK_SET);
	GV(&local, P.w, P.h);
	GV(&local2, P.w, P.h);
	gxGetImage(&local2, 6, 32, 6 + P.w - 1, 32 + P.h - 1, 0);
	fread(local.vptr, P.size, 1, in);
	fclose(in);
	for (j = 0; j < P.size; j++)
		if (local.vptr[j] != 0)
			local2.vptr[j] = local.vptr[j] + coff;
	gxPutImage(&local2, gxSET, 6, 32, 0);
	DV(&local);
	DV(&local2);
	return;
}


void PlayFirst(char plr,char first)
{
 char i,w=0,index;
 int Check=0;

 FadeOut(2,pal,10,0,0);
 gxClearDisplay(0,0);
 music_start(M_LIFTOFF);
 ShBox(80,18,240,39);DispBig(92,22,"PRESTIGE FIRST",0,-1);
 ShBox(80,41,240,132);InBox(84,45,236,128);RectFill(85,46,235,127,0);
 ShBox(80,134,240,189); //77 first parameter
 grSetColor(1);
 PrintAt(84,141,"GOAL STEP COMPLETE: ");grSetColor(6);

 //Modem Opponent => assure prestige first that mission
 Check=Data->Prestige[first].Indec;
 index=plr;

 if (index==0) PrintAt(0,0,"U.S.A.");
  else PrintAt(0,0,"SOVIET");

 for (i=first;i<28;i++)
  {
   grSetColor(9);
   if (Data->Prestige[i].Place==index && Data->PD[index][i]==0)
    {
     if (Option==-1 && MAIL==-1)
      {
       PrintAt(84,148+w*8,&PF[i][0]);
       ++w;
       Data->PD[index][i]=1;
      }
     else
      {
       //Found prestige first same mission
       if (Data->Prestige[i].Indec==Check)
        {
         PrintAt(84,148+w*8,&PF[i][0]);
         ++w;
         Data->PD[index][i]=1;
        }
      }
    } 
  }
 grSetColor(7);
 FadeIn(2,pal,10,0,0);
 if (Option==-1 && MAIL==-1) Replay(plr,Data->P[plr].PastMis-1,85,46,151,82,"OOOO");
  else Replay(index,Data->Prestige[first].Indec,85,46,151,82,"OOOO");
 
 PauseMouse();
 FadeOut(2,pal,10,0,0);
 gxClearDisplay(0,0);
 music_stop();
 return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
