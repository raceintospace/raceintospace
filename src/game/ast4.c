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
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty
//

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>

#define Guy(a,b,c,d) (Data->P[a].Crew[b][c][d]-1)

#ifdef DEAD_CODE
int ALSpec(int att)
{
  int col=0;
  if (att>=65) col=16;
  if (att<65 && att>=40) col=11;
  if (att<40 && att>=20) col=8;
  if (att<20) col=0;
  if (att==0) col=3;
  return col;
}
#endif

int missions;     // Variable for how many missions each 'naut has flown
static char program;  /* Variable to store prog data for "Draws Astronaut attributes" section: 1=Mercury/Vostok...5=Jupiter/Kvartet */
int retdel;  /* Variable to store whether a given 'naut has announced retirement */
int sex;  /* Variable to store a given 'naut sex */

void AstLevel(char plr,char prog,char crew,char ast)
{
   GXHEADER local;
   int i,k,man,over=0,temp,val;
   char Compat[5],cnt;
   i=man=Guy(plr,prog,crew,ast);

      cnt=0;for (k=0;k<5;k++) Compat[k]=0;

      switch (Data->P[plr].Pool[i].Compat) {
	     case 1:
	      if (Data->P[plr].Pool[i].CL==2) Compat[cnt++]=9;
	      Compat[cnt++]=10;
	      Compat[cnt++]=1;
	      Compat[cnt++]=2;
	      if (Data->P[plr].Pool[i].CR==2) Compat[cnt++]=3;
	      break;

	     case 2:
	      if (Data->P[plr].Pool[i].CL==2) Compat[cnt++]=10;
	      Compat[cnt++]=1;
	      Compat[cnt++]=2;
	      Compat[cnt++]=3;
	      if (Data->P[plr].Pool[i].CR==2) Compat[cnt++]=4;
	      break;

	     case 3: case 4: case 5: case 6: case 7: case 8:
	      if (Data->P[plr].Pool[i].CL==2) Compat[cnt++]=Data->P[plr].Pool[i].Compat-2;
	      Compat[cnt++]=Data->P[plr].Pool[i].Compat-1;
	      Compat[cnt++]=Data->P[plr].Pool[i].Compat;
	      Compat[cnt++]=Data->P[plr].Pool[i].Compat+1;
	      if (Data->P[plr].Pool[i].CR==2) Compat[cnt++]=Data->P[plr].Pool[i].Compat+2;
	      break;

	     case 9:
	      if (Data->P[plr].Pool[i].CL==2) Compat[cnt++]=7;
	      Compat[cnt++]=8;
	      Compat[cnt++]=9;
	      Compat[cnt++]=10;
	      if (Data->P[plr].Pool[i].CR==2) Compat[cnt++]=1;
	      break;

	     case 10:
	      if (Data->P[plr].Pool[i].CL==2) Compat[cnt++]=8;
	      Compat[cnt++]=9;
	      Compat[cnt++]=10;
	      Compat[cnt++]=1;
	      if (Data->P[plr].Pool[i].CR==2) Compat[cnt++]=2;
	      break;
	    };

   GV(&local,143,74);
   
   gxGetImage(&local,94,38,236,111,0);
   ShBox(94,38,236,95);
   InBox(98,41,232,61);RectFill(99,42,231,60,7+plr*3);

   grSetColor(12);PrintAt(115,48,"COMPATIBILITY");

   over=0;val=0;
   for (i=0;i<Data->P[plr].Gcnt[prog][crew];i++) {
      if (man!=Guy(plr,prog,crew,i)) {
	     temp=0;
	     for (k=0;k<cnt;k++) 
	       if (Compat[k]==Data->P[plr].Pool[Guy(plr,prog,crew,i)].Compat) temp++;

        if ((plr==1 && Data->Def.Ast2==0) || (plr==0 && Data->Def.Ast1==0)) {
	       InBox(111,66+9*val,119,72+9*val);
	       if (temp==0) {RectFill(112,67+9*val,118,71+9*val,9);over++;}
	       else RectFill(112,67+9*val,118,71+9*val,16);
	       grSetColor(1);
	       PrintAt(122,71+9*val,Data->P[plr].Pool[Guy(plr,prog,crew,i)].Name);
        }
        else if (temp==0) over++;
	     val++;
	   }
   }
   switch(prog) {
      case 1: i=16; break;
      case 2: i=(over==1)? 9 : 16; break;
      case 3: i=(over==1)? 11 : ((over==2) ?9:16) ; break;
      case 4: i=(over==1)? 11 : ((over==2) ?9:16) ; break;
      case 5: i=(over==1)? 11 : ((over>=2) ?9:16) ; break;
      default: i=16;break;
      }

   // Level 2 Only
   if ((plr==1 && Data->Def.Ast2==1) || (plr==0 && Data->Def.Ast1==1)) {
	   InBox(111,66,119,72);
	   RectFill(112,67,118,71,i);
	   grSetColor(1);
	   PrintAt(122,71,"CREW RELATIONSHIP");
   }

   if ((plr==1 && Data->Def.Ast2==2) || (plr==0 && Data->Def.Ast1==2)) {
	   grSetColor(1);
	   PrintAt(111,71,"NO INFO AT THIS LEVEL");
   }



   //RectFill(113,53,119,57,i);  // shouldn't be mood
   grSetColor(11); PrintAt(115,57,Data->P[plr].Pool[man].Name);
   // don't do this for level three
   PrintAt(0,0,"  M: ");DispNum(0,0,Data->P[plr].Pool[man].Mood);

	 key=0;
	 if (mousebuttons) WaitForMouseUp();
	 else while (key==0) GetMouse();

   gxPutImage(&local,gxSET,94,38,0);
   DV(&local);
   return;
}


void PlaceEquip(char plr,char prog)
{
  int i;
  FILE *fin;
  GXHEADER local,local2;
	SimpleHdr table;

  GV(&local,80,50);GV(&local2,80,50);
  fin=sOpen("APROG.BUT","rb",0);
  fseek(fin,(plr*7+prog)*sizeof_SimpleHdr,SEEK_SET);
  fread_SimpleHdr(&table,1,fin);
  fseek(fin,14*sizeof_SimpleHdr,SEEK_SET);
  fread(&pal[0],768,1,fin);

  fseek(fin,table.offset,SEEK_SET);
  fread(buffer,table.size,1,fin);
  fclose(fin);
  RLED_img(buffer,local.vptr,table.size,local.w,local.h);
  gxGetImage(&local2,61,28,140,77,0);
  for (i=0;i<4000;i++) {
   if (local.vptr[i]!=0) local2.vptr[i]=local.vptr[i];
   }
  gxPutImage(&local,gxSET,61,28,0);
  DV(&local),DV(&local2);
  return;
}

void DrawProgs(char plr,char prog)
{
  int i,j,Name[30];
  strcpy((char *)Name,Data->P[plr].Manned[prog-1].Name);
  strcat((char *)Name," PROGRAM");
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  grSetColor(1);
  ShBox(0,0,319,22);ShBox(0,24,319,81);ShBox(0,83,319,123);
  ShBox(0,125,158,199);ShBox(161,125,319,199);
  RectFill(25,129,153,195,0);RectFill(5,129,19,195,0);
  ShBox(6,130,18,161);ShBox(6,163,18,194);
  IOBox(243,86,316,102);IOBox(243,104,316,120);
  IOBox(243,3,316,19);
  InBox(4,128,20,196);InBox(24,128,154,196);
  InBox(60,27,141,78);InBox(3,3,30,19);
  UPArrow(9,133);DNArrow(9,166);
  PlaceEquip(plr,prog-1);
  for (j=0;j<2;j++)
    for (i=0;i<4;i++) ShBox(164+77*j,139+i*15,238+77*j,151+i*15);
  ShBox(4,86,12,92);
  if (prog>=2) ShBox(4,95,12,101);
  if (prog>=3) ShBox(4,104,12,110);
  if (prog==5) ShBox(4,113,12,119);
  FlagSm(plr,4,4);
  grSetColor(1);
  PrintAt(250,96,"ASSIGN CREW");
  PrintAt(252,114,"BREAK CREW");
  PrintAt(258,13,"CONTINUE");
  grSetColor(5);
  PrintAt(183,133,"FLIGHT CREW SELECTION");
  grSetColor(7);
  PrintAt(152,35,&Data->P[plr].Manned[prog-1].Name[0]);
  grSetColor(9);
  if (prog==1) PrintAt(152,43,"ONE");
  if (prog==2) PrintAt(152,43,"TWO");
  if (prog==3 || prog==4) PrintAt(152,43,"THREE");
  if (prog==5) PrintAt(152,43,"FOUR");
  PrintAt(0,0,"-PERSON CAPACITY");
  grSetColor(7);
  PrintAt(152,51,"SAFETY FACTOR: ");
  grSetColor(11);
  DispNum(0,0,Data->P[plr].Manned[prog-1].Safety);PrintAt(0,0," %");
  grSetColor(7);
  PrintAt(152,59,"UNIT WEIGHT: ");
  grSetColor(11);
  DispNum(0,0,Data->P[plr].Manned[prog-1].UnitWeight);
  grSetColor(7);
  PrintAt(152,67,"MAX DURATION: ");
  grSetColor(11);
  DispNum(0,0,Data->P[plr].Manned[prog-1].Duration);PrintAt(0,0," DAYS (LVL ");
   if (prog==1) PrintAt(0,0,"B)");
   if (prog==2) PrintAt(0,0,"E)");
   if (prog==3 || prog==5) PrintAt(0,0,"F)");
   if (prog==4) PrintAt(0,0,"D)");
  grSetColor(7);
  PrintAt(152,75,"AVOID FAILURE: ");
  grSetColor(11);
  if (Data->P[plr].Manned[prog-1].SaveCard>0) PrintAt(0,0,"YES");
   else PrintAt(0,0,"NO");
  grSetColor(1);
  DispBig(40,5,(char *)Name,0,-1);
  return;
}

/** need to check programs to see if there are adequate astronauts there
 * check any assigned Cgroups yes then return
 * check how many astronauts assigned to current prog
 * 
 * 
 */
int CheckProgram(char plr,char prog)
{
 int check,i;
 check=0;
 for (i=0;i<Data->P[plr].AstroCount;i++)
    if (Data->P[plr].Pool[i].Crew!=0)
      ++check;
 if (check>0) return(1);
 check=0;
 for (i=0;i<Data->P[plr].AstroCount;i++)
    if (Data->P[plr].Pool[i].Assign==prog)
      ++check;
 if (prog>=1 && prog<=3 && check>=prog) return(2);
   else if ((prog==4 || prog==5) && check>=prog-1) return(2);
     else // return to limbo
       for (i=0;i<Data->P[plr].AstroCount;i++)
    if (Data->P[plr].Pool[i].Assign==prog)
      Data->P[plr].Pool[i].Assign=0;
 return(0);
}

void FixPrograms(char plr)
{
 int i;
 for (i=0;i<7;i++)
  {
   if (Data->P[plr].Manned[i].DCost>0 && Data->P[plr].Manned[i].DCost<=Data->P[plr].Cash)
    DamProb(plr,2,i);
  };
 for (i=0;i<5;i++)
  {
   if (Data->P[plr].Rocket[i].DCost>0 && Data->P[plr].Rocket[i].DCost<=Data->P[plr].Cash)
    DamProb(plr,1,i);
  };
 for (i=0;i<4;i++)
  {
   if (Data->P[plr].Misc[i].DCost>0 && Data->P[plr].Misc[i].DCost<=Data->P[plr].Cash)
    DamProb(plr,3,i);
  };
 for (i=0;i<3;i++)
  {
   if (Data->P[plr].Probe[i].DCost>0 && Data->P[plr].Probe[i].DCost<=Data->P[plr].Cash)
    DamProb(plr,0,i);
  };
 return;
}

void DamProb(char plr,char prog,int chk)
{
 int D_Cost,Saf_Loss, ESafety;
 char Digit[4],Name[30];

 Saf_Loss = D_Cost = ESafety = 0; /* XXX check uninitialized */

 FadeOut(2,pal,10,0,0);
 
 gxClearDisplay(0,0);
 switch(prog)
  {
   case 0:D_Cost=Data->P[plr].Probe[chk].DCost;
          Saf_Loss=Data->P[plr].Probe[chk].Damage;
	  ESafety =Data->P[plr].Probe[chk].Safety;
          strcpy((char *)Name,Data->P[plr].Probe[chk].Name);
          break;
   case 1:D_Cost=Data->P[plr].Rocket[chk].DCost;
          Saf_Loss=Data->P[plr].Rocket[chk].Damage;
	  ESafety =Data->P[plr].Rocket[chk].Safety;
          strcpy((char *)Name,Data->P[plr].Rocket[chk].Name);
          break;
   case 2:D_Cost=Data->P[plr].Manned[chk].DCost;
          Saf_Loss=Data->P[plr].Manned[chk].Damage;
	  ESafety =Data->P[plr].Manned[chk].Safety;
          strcpy((char *)Name,Data->P[plr].Manned[chk].Name);
          break;
   case 3:D_Cost=Data->P[plr].Misc[chk].DCost;
          Saf_Loss=Data->P[plr].Misc[chk].Damage;
	  ESafety =Data->P[plr].Misc[chk].Safety;
          strcpy((char *)Name,Data->P[plr].Misc[chk].Name);
          break;
   default:break;
  }
 ShBox(35,81,288,159);InBox(40,86,111,126);InBox(116,86,283,126);
 IOBox(116,130,189,155);IOBox(201,130,274,155);Flag(41,87,plr);
 DispBig(135,136,"YES",1,0);DispBig(225,136,"NO",1,0);
 DispBig(44,135,"REPAIR",1,-1);
 grSetColor(6);
 PrintAt(121,95,"DIRECTOR: ");
 grSetColor(8);
 if (plr==0) PrintAt(0,0,&Data->P[Data->plr[0]].Name[0]);
   else PrintAt(0,0,&Data->P[Data->plr[1]].Name[0]);

 grSetColor(6);PrintAt(121,104,"DAMAGE: ");
 grSetColor(11);
 strcat((char *)Name," PROGRAM");
 PrintAt(0,0,&Name[0]);
 grSetColor(6);
 PrintAt(121,113,"DAMAGE COST: ");
 grSetColor(1);
 sprintf(&Digit[0],"%d",D_Cost);PrintAt(0,0,&Digit[0]);
 PrintAt(0,0," M.B.  (OF ");
 DispMB (0,0,Data->P[plr].Cash); PrintAt(0,0,")");
 grSetColor(6);
 PrintAt(121,122,"SAFETY LOSS: ");
 grSetColor(1);
 sprintf(&Digit[0],"%d",Saf_Loss);PrintAt(0,0,&Digit[0]);
 PrintAt(0,0,"%  (FROM ");
sprintf(&Digit[0],"%d",ESafety);PrintAt(0,0,&Digit[0]);
 PrintAt(0,0,"%)");
 FadeIn(2,pal,10,0,0);
 
 WaitForMouseUp();
 while(1)
  {
   key=0;GetMouse();
   if (mousebuttons>0 || key>0)
     {
	   if ((x>=118 && y>=132 && x<=187 && y<=153 && mousebuttons>0) || key=='Y')
       {
	     InBox(118,132,187,153);
	     WaitForMouseUp();
        if (key>0) delay(150);
	     Data->P[plr].Cash-=D_Cost;
        switch(prog)
         {
          case 0:Data->P[plr].Probe[chk].DCost=0;
             Data->P[plr].Probe[chk].Damage=0;
             break;
          case 1:Data->P[plr].Rocket[chk].DCost=0;
             Data->P[plr].Rocket[chk].Damage=0;
             break;
          case 2:Data->P[plr].Manned[chk].DCost=0;
             Data->P[plr].Manned[chk].Damage=0;
             break;
          case 3:Data->P[plr].Misc[chk].DCost=0;
             Data->P[plr].Misc[chk].Damage=0;
             break;
          default:break;
         }
	     return;
	    }
      else
	   if ((x>=203 && y>=132 && x<=272 && y<=153 && mousebuttons>0) || key=='N')
       {
	     InBox(203,132,272,153);
	     WaitForMouseUp();
         if (key>0) delay(150);
	     return;
	     }
	   }
   }
}


#ifdef DEAD_CODE
void CrewProb(char plr,char prog)
{
 if (prog)EMPTY_BODY;
 
 ShBox(75,43,244,173);IOBox(81,152,238,167); InBox(81,70,238,106);
 RectFill(82,71,237,105,7+3*plr);
 grSetColor(11);
 PrintAt(136,162,"CONTINUE");
 DispBig(122,50,"PROBLEM",0,-1);
 PrintAt(90,80,"THERE ARE CURRENTLY NOT");
 PrintAt(90,88,"ENOUGH ");
 if (plr==1) PrintAt(0,0,"COSM");
   else PrintAt(0,0,"ASTR"); PrintAt(0,0,"ONAUTS IN THE");
 PrintAt(90,96,"PROGRAM TO ASSIGN CREWS.");
 
 WaitForMouseUp();
 while(1)
  {
   key=0;GetMouse();
   if (mousebuttons>0 || key>0)
     {
	 if ((x>=83 && y>=154 && x<=236 && y<=165 && mousebuttons>0) || key==K_ENTER || key==K_ESCAPE)
      { 
	    InBox(83,154,236,165);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    return;  // Abort - Redo Mission
	   }
     }
  }
}
#endif

void DrawPosition(char prog,int pos)
{
 
 grSetColor(5);
 switch(pos)
  {
   case 1: if (prog==1) PrintAt(17,91,"CAPSULE PILOT - EVA SPECIALIST");
      else if (prog==2) PrintAt(17,91,"CAPSULE PILOT - DOCKING SPECIALIST");
  	  else  if (prog>=3)
	      PrintAt(17,91,"COMMAND PILOT");
	   break;
   case 2: if (prog>1 && prog<5)
	     PrintAt(17,100,"LM PILOT - EVA SPECIALIST");
	   else if (prog==5) PrintAt(17,100,"LUNAR PILOT");
	   break;
   case 3: if (prog>2 && prog<5)
	     PrintAt(17,109,"DOCKING SPECIALIST");
	    else if (prog==5) PrintAt(17,109,"EVA SPECIALIST");
	   break;
   case 4: if (prog==5) PrintAt(17,118,"EVA SPECIALIST");
	   break;
   default:break;
  }
 grSetColor(1);
 
 return;
}


void
Programs(char plr, char prog)
{
	int i, max, chk, tst;
	int now2 = 0, count = 0, grp = 0, BarA = 0;
	int M[100], Gcnt[8];
	char ksel = 0;

	strcpy(IDT, "i036");
	strcpy(IKEY, "k036");

	for (i = 0; i < 100; i++)
		M[i] = -1;

	if (prog > 4)
		max = 4;
	else if (prog == 4)
		max = 3;
	else
		max = prog;

	music_start(M_PRGMTRG);
	DrawProgs(plr, prog);
	Flts(0, 0);

	for (i = 0; i < 8; i++)
	{
		Gcnt[i] = Data->P[plr].Gcnt[prog][i];
		if (Gcnt[i] == 0)
			FltsTxt(i, 8);
		if (Gcnt[i] < max && Gcnt[i] != 0)
			FltsTxt(i, 9);
		if (Gcnt[i] == max)
			FltsTxt(i, 1);
	}

	for (i = 0; i < Data->P[plr].AstroCount; i++)
		if (Data->P[plr].Pool[i].Assign == prog
			&& Data->P[plr].Pool[i].Crew == 0)
		{
			M[count++] = i;
		}

	ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
	DispLeft(plr, BarA, count, now2, &M[0]); 
	NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
		Data->P[plr].Crew[prog][grp][1],
		Data->P[plr].Crew[prog][grp][2], Data->P[plr].Crew[prog][grp][3]);
	FadeIn(2, pal, 10, 0, 0);

	chk = CheckProgram(plr, prog);
	if (chk == 0)
	{
		if (plr == 0)
			Help("i113");
		else
			Help("i114");
		music_stop();
		return;
	}

	WaitForMouseUp();
	while (1)
	{
		key = 0;
		GetMouse();
		for (i = 0; i < 8; i++)
		{						   // Right Select Box
			if (x >= 27 && y >= (131 + i * 8) && x <= 151
				&& y <= (137 + i * 8) && mousebuttons > 0
				&& (now2 - BarA + i) <= (count - 1))
			{					   // Left
				now2 -= BarA;
				now2 += i;
				BarA = i;
				RectFill(26, 129, 153, 195, 0);
				DispLeft(plr, BarA, count, now2, &M[0]);
				ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
				BarSkill(plr, BarA, now2, &M[0]);
				WaitForMouseUp();
				ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
				DispLeft(plr, BarA, count, now2, &M[0]);
			}
		}
		if (mousebuttons > 0 || key > 0)	/* Gameplay */
		{
			if (((x >= 6 && y >= 130 && x <= 18 && y <= 161
						&& mousebuttons > 0) || key == UP_ARROW)
				&& count > 0)
			{					   /* Lft Up */
				InBox(6, 130, 18, 161);
				for (i = 0; i < 50; i++)
				{
					key = 0;
					GetMouse();
					delay(10);
					if (mousebuttons == 0)
					{

						if (BarA == 0)
							if (now2 > 0)
							{
								now2--;
								RectFill(26, 129, 153, 195, 0);
								ShBox(26, 130 + BarA * 8, 152,
									138 + BarA * 8);
								DispLeft(plr, BarA, count, now2, &M[0]);
							};
						if (BarA > 0)
						{
							RectFill(26, 129, 153, 195, 0);
							BarA--;
							now2--;
							ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
							DispLeft(plr, BarA, count, now2, &M[0]);
						};

						i = 51;
					}
				}
				while (mousebuttons == 1 || key == UP_ARROW)
				{
					delay(100);

					if (BarA == 0)
						if (now2 > 0)
						{
							now2--;
							RectFill(26, 129, 153, 195, 0);
							ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
							DispLeft(plr, BarA, count, now2, &M[0]);
						};
					if (BarA > 0)
					{
						RectFill(26, 129, 153, 195, 0);
						BarA--;
						now2--;
						ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
						DispLeft(plr, BarA, count, now2, &M[0]);
					};

					key = 0;
					GetMouse();
				}
				OutBox(6, 130, 18, 161);
				delay(10);
			}
			else if (((x >= 6 && y >= 163 && x <= 18 && y <= 194
						&& mousebuttons > 0) || key == DN_ARROW)
				&& count > 0)
			{					   /* Lft Dwn */
				InBox(6, 163, 18, 194);
				for (i = 0; i < 50; i++)
				{
					key = 0;
					GetMouse();
					delay(10);
					if (mousebuttons == 0)
					{

						if (BarA == 7)
							if (now2 < count - 1)
							{
								now2++;
								RectFill(26, 129, 153, 195, 0);
								ShBox(26, 130 + BarA * 8, 152,
									138 + BarA * 8);
								DispLeft(plr, BarA, count, now2, &M[0]);
							};
						if (BarA < 7)
							if (now2 < count - 1)
							{
								RectFill(26, 129, 153, 195, 0);
								BarA++;
								now2++;
								ShBox(26, 130 + BarA * 8, 152,
									138 + BarA * 8);
								DispLeft(plr, BarA, count, now2, &M[0]);
							};

						i = 51;
					}
				}
				while (mousebuttons == 1 || key == DN_ARROW)
				{
					delay(100);

					if (BarA == 7)
						if (now2 < count - 1)
						{
							now2++;
							RectFill(26, 129, 153, 195, 0);
							ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
							DispLeft(plr, BarA, count, now2, &M[0]);
						};
					if (BarA < 7)
						if (now2 < count - 1)
						{
							RectFill(26, 129, 153, 195, 0);
							BarA++;
							now2++;
							ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
							DispLeft(plr, BarA, count, now2, &M[0]);
						};

					key = 0;
					GetMouse();
				}
				//WaitForMouseUp();
				OutBox(6, 163, 18, 194);
			}
			else if (key == K_HOME)
			{
				RectFill(26, 129, 153, 195, 0);
				BarA = 0;
				now2 = 0;
				ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
				DispLeft(plr, BarA, count, now2, &M[0]);
			}
			else if (key == K_END)
			{
				RectFill(26, 129, 153, 195, 0);
				BarA = minn(count - 1, 7);
				now2 = count - 1;
				ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
				DispLeft(plr, BarA, count, now2, &M[0]);
			}
			else if (key == 'S')
			{					   // Show Skill
				ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
				BarSkill(plr, BarA, now2, &M[0]);
			}
			else if (((x >= 4 && y >= 86 && x <= 12 && y <= 92
						&& mousebuttons > 0) || key == '1')
				&& Gcnt[grp] >= 1)
			{					   /* Display Man 1 */
				InBox(4, 86, 12, 92);
				AstLevel(plr, prog, grp, 0);
				OutBox(4, 86, 12, 92);
			}
			else if (((x >= 4 && y >= 95 && x <= 12 && y <= 101
						&& mousebuttons > 0) || key == '2') && prog >= 2
				&& Gcnt[grp] >= 2)
			{					   /* Display Man 2 */
				InBox(4, 95, 12, 101);
				AstLevel(plr, prog, grp, 1);
				OutBox(4, 95, 12, 101);
			}
			else if (((x >= 4 && y >= 104 && x <= 12 && y <= 110
						&& mousebuttons > 0) || key == '3') && prog >= 3
				&& Gcnt[grp] >= 3)
			{					   /* Display Man 3 */
				InBox(4, 104, 12, 110);
				AstLevel(plr, prog, grp, 2);
				OutBox(4, 104, 12, 110);
			}
			else if (((x >= 4 && y >= 113 && x <= 12 && y <= 119
						&& mousebuttons > 0) || key == '4') && prog >= 5
				&& Gcnt[grp] >= 4)
			{					   /* Display Man 4 */
				InBox(4, 113, 12, 119);
				AstLevel(plr, prog, grp, 3);
				OutBox(4, 113, 12, 119);
			}
			else if (key == 'C')
			{
				ClearIt();
				ksel = grp;		   //save old flt crew
				if (grp == 7)
					grp = 0;
				else
					++grp;
				Flts(ksel, grp);
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				if (key > 0)
					delay(150);
			}
			else if (x >= 164 && y >= 139 && x <= 238 && y <= 151
				&& mousebuttons > 0)
			{					   /* Flt Crew I */
				ClearIt();
				Flts(grp, 0);
				grp = 0;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 164 && y >= 154 && x <= 238 && y <= 166
				&& mousebuttons > 0)
			{					   /* Flt Crew II */
				ClearIt();
				Flts(grp, 1);
				grp = 1;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 164 && y >= 169 && x <= 238 && y <= 181
				&& mousebuttons > 0)
			{					   /* Flt Crew III */
				ClearIt();
				Flts(grp, 2);
				grp = 2;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 164 && y >= 184 && x <= 238 && y <= 196
				&& mousebuttons > 0)
			{					   /* Flt Crew IV */
				ClearIt();
				Flts(grp, 3);
				grp = 3;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 241 && y >= 139 && x <= 315 && y <= 151
				&& mousebuttons > 0)
			{					   /* Flt Crew V */
				ClearIt();
				Flts(grp, 4);
				grp = 4;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 241 && y >= 154 && x <= 315 && y <= 166
				&& mousebuttons > 0)
			{					   /* Flt Crew VI */
				ClearIt();
				Flts(grp, 5);
				grp = 5;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 241 && y >= 169 && x <= 315 && y <= 181
				&& mousebuttons > 0)
			{					   /* Flt Crew VII */
				ClearIt();
				Flts(grp, 6);
				grp = 6;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (x >= 241 && y >= 184 && x <= 315 && y <= 196
				&& mousebuttons > 0)
			{					   /* Flt Crew VIII */
				ClearIt();
				Flts(grp, 7);
				grp = 7;
				NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
					Data->P[plr].Crew[prog][grp][1],
					Data->P[plr].Crew[prog][grp][2],
					Data->P[plr].Crew[prog][grp][3]);
				WaitForMouseUp();
			}
			else if (((x >= 245 && y >= 88 && x <= 314 && y <= 100
						&& mousebuttons > 0) || key == 'A')
				&& Gcnt[grp] < max)
			{					   /* Assign 'Naut */
				if (Data->P[plr].Crew[prog][grp][Gcnt[grp]] == 0
					&& count > 0)
				{
					InBox(245, 88, 314, 100);
					Data->P[plr].Crew[prog][grp][Gcnt[grp]] = M[now2] + 1;

					AstNames(Gcnt[grp], &Data->P[plr].Pool[M[now2]].Name[0],
						Data->P[plr].Pool[M[now2]].Mood);
					Data->P[plr].Pool[M[now2]].Crew = grp + 1;
					Data->P[plr].Pool[M[now2]].Task = Gcnt[grp];
					Data->P[plr].Pool[M[now2]].Una = 1;
					for (i = now2; i < count; i++)
						M[i] = M[i + 1];
					M[i] = -1;
					count--;
					if (now2 == count)
					{
						if (now2 > 0)
							now2--;
						if (BarA > 0)
							BarA--;
					};
					RectFill(26, 129, 153, 195, 0);
					ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
					DispLeft(plr, BarA, count, now2, &M[0]);

					Gcnt[grp]++;
					Data->P[plr].Gcnt[prog][grp] = Gcnt[grp];
					if (Gcnt[grp] == max)
						FltsTxt(grp, 1);
					else
						FltsTxt(grp, 9);
					NewAstList(plr, prog, Data->P[plr].Crew[prog][grp][0],
						Data->P[plr].Crew[prog][grp][1],
						Data->P[plr].Crew[prog][grp][2],
						Data->P[plr].Crew[prog][grp][3]);
					WaitForMouseUp();
					if (key > 0)
						delay(150);
					OutBox(245, 88, 314, 100);
				};				   /* End outer if */
			}
			else if ((x >= 245 && y >= 106 && x <= 314 && y <= 118
					&& mousebuttons > 0) || key == 'B')
			{					   /* Break Group */
				tst = Data->P[plr].Crew[prog][grp][0] - 1;
				if (Data->P[plr].Pool[tst].Prime > 0)
				{
					OutBox(245, 106, 314, 118);
					gxDisplayVirtual(75, 43, 244, 173, 0, &vhptr, 75, 43);
					ShBox(75, 43, 244, 173);
					IOBox(81, 152, 238, 167);
					InBox(81, 70, 238, 113);
					RectFill(82, 71, 237, 112, 7 + 3 * plr);
					grSetColor(1);
					DispBig(118, 50, "PROBLEM", 0, -1);
					PrintAt(136, 162, "CONTINUE");
					grSetColor(11);
					PrintAt(88, 80, "FLIGHT CREW ");
					DispNum(0, 0, grp + 1);
					PrintAt(0, 0, " IS ALREADY");
					PrintAt(88, 88, "ASSIGNED TO THE ");
					if (Data->P[plr].Pool[tst].Prime == 4
						|| Data->P[plr].Pool[tst].Prime == 3)
						PrintAt(0, 0, "PRIMARY");
					else
						PrintAt(0, 0, "BACKUP");
					PrintAt(88, 96, "CREW OF A CURRENT MISSION:");
					PrintAt(88, 104, "CANNOT BREAK THIS CREW.");

					WaitForMouseUp();
					i = 1;
					while (i == 1)
					{
						key = 0;
						GetMouse();
						if (mousebuttons > 0 || key > 0)
						{
							if ((x >= 83 && y >= 154 && x <= 236 && y <= 165
									&& mousebuttons != 0) || key == K_ENTER)
							{
								InBox(83, 154, 236, 165);
								WaitForMouseUp();
								OutBox(83, 154, 236, 165);
								gxVirtualDisplay(&vhptr, 75, 43, 75, 43, 244,
									173, 0);
								i = 2;
							}
						}
					}
				}
				else if (Data->P[plr].Crew[prog][grp][0] != 0)
				{
					InBox(245, 106, 314, 118);
					while (Gcnt[grp] > 0)
					{
						M[count] =
							Data->P[plr].Crew[prog][grp][Gcnt[grp] - 1] - 1;
						Data->P[plr].Crew[prog][grp][Gcnt[grp] - 1] = 0;
						Data->P[plr].Pool[M[count]].Crew = 0;
						Data->P[plr].Pool[M[count]].Moved = 0;
						Data->P[plr].Pool[M[now2]].Una = 0;
						Gcnt[grp]--;
						count++;
					};
					ClearIt();
					RectFill(26, 129, 153, 195, 0);
					FltsTxt(grp, 8);
					ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
					DispLeft(plr, BarA, count, now2, &M[0]);
					for (i = 1; i < 5; i++)
						DrawPosition(prog, i);

					WaitForMouseUp();
					OutBox(245, 106, 314, 118);
				};
			}
			else if ((x >= 245 && y >= 5 && x <= 314 && y <= 17
					&& mousebuttons > 0) || key == K_ENTER)
			{					   /* Exit */
				InBox(245, 5, 314, 17);
				WaitForMouseUp();
				if (key > 0)
					delay(150);
				OutBox(245, 5, 314, 17);
				delay(10);
				for (i = 0; i < 8; i++)
				{
					if (Gcnt[i] < max)
						while (Gcnt[i] > 0)
						{
							M[count] =
								Data->P[plr].Crew[prog][i][Gcnt[i] - 1] - 1;
							Data->P[plr].Crew[prog][i][Gcnt[i] - 1] = 0;
							Data->P[plr].Pool[M[count]].Crew = 0;
							Data->P[plr].Gcnt[prog][i] = 0;
							Gcnt[i]--;
							count++;
						};
					Data->P[plr].Gcnt[prog][i] = Gcnt[i];
				};
				for (i = 0; i < count; i++)
				{
					Data->P[plr].Pool[M[i]].Assign = 0;
				};
				music_stop();
				return;			   /* Done */
			};
		}
	};
}

/** clears the screen
 */
void ClearIt(void)
{
 
 RectFill(16,87,75,91,3);RectFill(5,87,11,91,3);
 RectFill(16,96,75,100,3);RectFill(5,96,11,100,3);
 RectFill(16,105,75,109,3);RectFill(5,105,11,109,3);
 RectFill(16,114,75,118,3);RectFill(5,114,11,118,3);
 RectFill(16,87,238,121,3);
 
 return;
}

void NewAstList(char plr,char prog,int M1,int M2,int M3,int M4)
{
  
  program=prog;  /* Sets capsule/shuttle program for "Draws Astronaut attributes" section */
                 /* 1=Mercury/Vostok, 2=Gemini/Voskhod, 3=Apollo/Soyuz, 4=XMS-2/Lapot, 5=Jupiter/Kvartet */
				 /* This will be used to highlight the skills for each crew member's role -Leon */

  RectFill(13,86,231,122,3);  /* Clear Astro Area */
  grSetColor(1);
  if (M1>0)
    {
	 retdel=Data->P[plr].Pool[M1-1].RDelay;  // Sets whether 'naut has announced retirement 
	 sex=Data->P[plr].Pool[M1-1].Sex;  // Sets whether 'naut is male or female
	 missions=Data->P[plr].Pool[M1-1].Missions;
     AstNames(0,&Data->P[plr].Pool[M1-1].Name[0],Data->P[plr].Pool[M1-1].Mood);
     AstStats(plr,0,M1-1);
    }
  else DrawPosition(prog,1);
  if (M2>0) {
	retdel=Data->P[plr].Pool[M2-1].RDelay;  // Sets whether 'naut has announced retirement
	sex=Data->P[plr].Pool[M2-1].Sex;  // Sets whether 'naut is male or female
	 missions=Data->P[plr].Pool[M2-1].Missions;
    AstNames(1,&Data->P[plr].Pool[M2-1].Name[0],Data->P[plr].Pool[M2-1].Mood);
    AstStats(plr,1,M2-1);
  }
  else DrawPosition(prog,2);
  if (M3>0) {
	retdel=Data->P[plr].Pool[M3-1].RDelay;  // Sets whether 'naut has announced retirement
	sex=Data->P[plr].Pool[M3-1].Sex;  // Sets whether 'naut is male or female
	 missions=Data->P[plr].Pool[M3-1].Missions;
    AstNames(2,&Data->P[plr].Pool[M3-1].Name[0],Data->P[plr].Pool[M3-1].Mood);
    AstStats(plr,2,M3-1);
  }
  else DrawPosition(prog,3);
  if (M4>0) {
	retdel=Data->P[plr].Pool[M4-1].RDelay;  // Sets whether 'naut has announced retirement
	sex=Data->P[plr].Pool[M4-1].Sex;  // Sets whether 'naut is male or female
	 missions=Data->P[plr].Pool[M4-1].Missions;
    AstNames(3,&Data->P[plr].Pool[M4-1].Name[0],Data->P[plr].Pool[M4-1].Mood);
    AstStats(plr,3,M4-1); 
  }
  else DrawPosition(prog,4);
  
  return;
}

/** Draws Astronaut attributes
 * 
 */
void AstStats(char plr,char man,char num)
{
  int y;
  grSetColor(1);
  y=91+man*9;
  if (man==0) {grSetColor(11);}  /* Highlight CA for command pilot */
   PrintAt(119,y,"CA:");DispNum(0,0,Data->P[plr].Pool[num].Cap);
  grSetColor(1);
  if (man==1 && program>1) {grSetColor(11);}  /* Highlight LM for LM pilot */
   PrintAt(143,y,"LM:");DispNum(0,0,Data->P[plr].Pool[num].LM);
  grSetColor(1);
  if (program==1 || ((program==2 || program==3 || program==4) && man==1) || (program==5 && man>1)) {grSetColor(11);}  /* Highlight EV for EVA specialist */
   PrintAt(167,y,"EV:");DispNum(0,0,Data->P[plr].Pool[num].EVA);
  grSetColor(1);
  if ((program==2 && man==0) || ((program==3 || program==4) && man==2)) {grSetColor(11);}  /* Highlight DO for docking specialist */
   PrintAt(192,y,"DO:");DispNum(0,0,Data->P[plr].Pool[num].Docking);
  grSetColor(1);  /* Never highlight EN skill */
   PrintAt(217,y,"EN:");DispNum(0,0,Data->P[plr].Pool[num].Endurance);
  return;
}

void AstNames(int man,char *name,char att)
{
  char col = 0;
  switch(man) {
    case 0:RectFill(16,87,75,91,3);
	   RectFill(5,87,11,91,3);
	   break;
    case 1:RectFill(16,96,75,100,3);
	   RectFill(5,96,11,100,3);
	   break;
    case 2:RectFill(16,105,75,109,3);
	   RectFill(5,105,11,109,3);
	   break;
    case 3:RectFill(16,114,75,118,3);
	   RectFill(5,114,11,118,3);
	   break;
    default: break;
  };
  grSetColor(1);
  if (sex==1) grSetColor(18);  // Show name in blue if 'naut is female
  if (retdel>0) grSetColor(0);  // Show name in black if 'naut has announced retirement
  if (sex==1 && retdel>0) grSetColor(7);  // Show name in purple if 'naut is female AND has announced retirement
  switch(man) {
    case 0: PrintAt(17,91,&name[0]);
			if (missions>0) { PrintAt(0,0," (");DispNum(0,0,missions);PrintAt(0,0,")"); }
			break;
    case 1: PrintAt(17,100,&name[0]);
			if (missions>0) { PrintAt(0,0," (");DispNum(0,0,missions);PrintAt(0,0,")"); }
			break;
    case 2: PrintAt(17,109,&name[0]);
			if (missions>0) { PrintAt(0,0," (");DispNum(0,0,missions);PrintAt(0,0,")"); }
			break;
    case 3: PrintAt(17,118,&name[0]);
			if (missions>0) { PrintAt(0,0," (");DispNum(0,0,missions);PrintAt(0,0,")"); }
			break;
    default: break;
  };
  if (att>=65) col=16;
  if (att<65 && att>=40) col=11;
  if (att<40 && att>=20) col=8;
  if (att<20) col=0;
  if (att==0) col=3;
//  if (retdel>0) col=7;
  switch(man) {
    case 0: RectFill(5,87,11,91,col);break;
    case 1: RectFill(5,96,11,100,col);break;
    case 2: RectFill(5,105,11,109,col);break;
    case 3: RectFill(5,114,11,118,col);break;
    default: break;
  };
  return;
}

void Flts(char old,char nw)
{
 
  switch (old) {
    case 0: OutBox(164,139,238,151);break;
    case 1: OutBox(164,154,238,166);break;
    case 2: OutBox(164,169,238,181);break;
    case 3: OutBox(164,184,238,196);break;
    case 4: OutBox(241,139,315,151);break;
    case 5: OutBox(241,154,315,166);break;
    case 6: OutBox(241,169,315,181);break;
    case 7: OutBox(241,184,315,196);break;
  };
  switch (nw) {
    case 0: InBox(164,139,238,151);break;
    case 1: InBox(164,154,238,166);break;
    case 2: InBox(164,169,238,181);break;
    case 3: InBox(164,184,238,196);break;
    case 4: InBox(241,139,315,151);break;
    case 5: InBox(241,154,315,166);break;
    case 6: InBox(241,169,315,181);break;
    case 7: InBox(241,184,315,196);break;
  };
  
  return;
} /* End of Flts */

void FltsTxt(char nw,char col)
{
  grSetColor(col);
  
  switch(nw) {
    case 0:PrintAt(169,147,"FLT. CREW I");break;
    case 1:PrintAt(169,162,"FLT. CREW II");break;
    case 2:PrintAt(169,177,"FLT. CREW III");break;
    case 3:PrintAt(169,192,"FLT. CREW IV");break;
    case 4:PrintAt(246,147,"FLT. CREW V");break;
    case 5:PrintAt(246,162,"FLT. CREW VI");break;
    case 6:PrintAt(246,177,"FLT. CREW VII");break;
    case 7:PrintAt(246,192,"FLT. CREW VIII");break;
  };
  
  return;
} /* End of FltsTxt */

/* vi: set noet ts=4 sw=4 tw=78: */
