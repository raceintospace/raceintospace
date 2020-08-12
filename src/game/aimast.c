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
// Programmed by Michael K McCarty and Morgan Roarty
//
/** \file aimast.c AI Master Routines
 */

#include <Buzz_inc.h>
#include <externs.h>

extern char Month[12][11];
extern char AI[2];
char Level_Check;
extern char Nums[30][7];
enum Opponent_Status Cur_Status;

// Track[0] - orbital satellite
// Track[1] - end stage location holder
// Track[2] - holds lunar module
// Track[3] - strategy 0=none 1=one 2=two
// Track[4] - beginning game status
// Track[5] - larger rocket (3-stage)
// AILunar  - way to the moon
// AIPrim   - primary program
// AISec    - secondary program

void AIMaster(char plr)
{
 int val,i,P_total=0,O_total=0;
 char prg[2];
 if (plr==0) Level_Check=(Data->Def.Lev1==0) ? 0 : 1;
  else  if (plr==1) Level_Check=(Data->Def.Lev2==0) ? 0 : 1;
//select strategy
 P_total=random(100);
 if (Data->P[plr].Track[3]==0)
  {
   if (P_total<33) Data->P[plr].Track[3]=1;
    else if (P_total<66) Data->P[plr].Track[3]=2;
     else Data->P[plr].Track[3]=3;
  }
 Cur_Status=Equal;
// *** check status ***
  for (i=0;i<Data->P[plr].PastMis;i++)
     P_total+=Data->P[plr].History[i].Prestige;
  for (i=0;i<Data->P[other(plr)].PastMis;i++)
     O_total+=Data->P[other(plr)].History[i].Prestige;
  if (P_total>O_total) Cur_Status=Ahead;
   else if (P_total==O_total) Cur_Status=Equal;
    else if (P_total<O_total) Cur_Status=Behind;
  if (Data->Year==59 && Data->Season==1) Data->P[plr].Track[4]=0;
  if (Data->P[plr].AstroDelay<=0) AIAstroPur(plr);
  if (Data->P[plr].AIStat==1)
     if (Data->Prestige[0].Place==plr || Data->Prestige[0].mPlace==plr) Data->P[plr].AIStat=2;
  if (Data->P[plr].AIStat==1 && PreChe(plr,1)==0 && Cur_Status==Behind) Data->P[plr].AIStat=2;
  if (Data->P[plr].AIStat==2 && Data->Year>=61 && Data->Season==0) Data->P[plr].AIStat=3;
  if (Data->P[plr].AIStat<=2)
  {
     if (GenPur(plr,0,0)) RDafford(plr,0,0);
     else RDafford(plr,0,0);
     if (GenPur(plr,1,0)) RDafford(plr,1,0);
     else RDafford(plr,1,0);
     Data->P[plr].Buy[0][0]=0;Data->P[plr].Buy[1][0]=0;
     Data->P[plr].Buy[2][0]=0;
     if (Data->Year<=59)
     {
        Data->P[plr].Probe[0].Num++;
        Data->P[plr].Rocket[0].Num++;
        AIFuture(plr,1,0,0);
     }
  KeepRD(plr,5);
  Data->P[plr].Buy[0][0]=0;Data->P[plr].Buy[1][0]=0;
  Data->P[plr].Buy[2][0]=0;
  RDafford(plr,2,0);RDafford(plr,1,0);
  if (Data->P[plr].AIStat<2) AIPur(plr);
  prg[0]=1;
  if (NoFail(plr)==0)
   {
    if ((Data->Year==59 && Data->Season==1) || Data->Year>=60)
     {
      switch(Data->P[plr].Track[4])
       {
        case 0:if (Data->P[plr].Misc[3].Safety>Data->P[plr].Misc[3].MaxRD-20)
                AIFuture(plr,6,0,(char *)&prg); else AIFuture(plr,2,0,(char *)&prg);
               Data->P[plr].Manned[0].Safety+=10;Data->P[plr].Misc[3].Safety+=10;
               ++Data->P[plr].Track[4];
               break;
        case 1:if (PreChe(plr,27)==0 && PreChe(other(plr),27)==0)  // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                AIFuture(plr,2,0,(char *)&prg);
                 else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==0)  // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                  AIFuture(plr,4,0,(char *)&prg);
                   else if (PreChe(plr,27)==0 && PreChe(other(plr),27)==1) // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                  AIFuture(plr,4,0,(char *)&prg);
                 else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==1) // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                AIFuture(plr,2,0,(char *)&prg);
               ++Data->P[plr].Track[4];
               break;
        case 2:if (PreChe(plr,27)==0 && PreChe(other(plr),27)==0)  // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                AIFuture(plr,2,0,(char *)&prg);
                 else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==0)  // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                  AIFuture(plr,4,0,(char *)&prg);
                   else if (PreChe(plr,27)==0 && PreChe(other(plr),27)==1) // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                  AIFuture(plr,4,0,(char *)&prg);
                 else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==1) // && Data->P[plr].Manned[1].Safety>Data->P[plr].Manned[1].MaxRD-25)
                AIFuture(plr,6,0,(char *)&prg); //2
               else Data->P[plr].AIStat=3;
               ++Data->P[plr].Track[4];
               break;
        default:break;
      }
     if (Data->P[plr].Manned[0].Num>=2 && Data->P[plr].Rocket[0].Num>=2)
      {
       if (Data->P[plr].Future[0].MissionCode==2) AIFuture(plr,4,1,(char *)&prg);
        else if (Data->P[plr].Future[0].MissionCode==4) AIFuture(plr,2,1,(char *)&prg);
         else if (Data->P[plr].Future[0].MissionCode==6) AIFuture(plr,4,1,(char *)&prg); 
      };
   };
  };
  AILaunch(plr);
 }
 if (GenPur(plr,3,3)) RDafford(plr,3,3);
  else RDafford(plr,3,3);
 Data->P[plr].Buy[3][3]=0;RDafford(plr,3,3);
 if (Data->P[plr].AIStat>=2)
  {
   CheckVoting(plr); // gets AIPrim,AISec,AILunar
   if (Data->P[plr].AIPrim==0) ProgramVoting(plr);
  }
 // primary/secondary programs
 if (Data->P[plr].AIPrim==8) {Data->P[plr].AIPrim=6;Data->P[plr].AISec=8;}
 if (Data->P[plr].AIPrim==6) Data->P[plr].AISec=8;
 if (Data->P[plr].AIPrim==9) {Data->P[plr].AIPrim=6;Data->P[plr].AISec=9;}

 if (Data->P[plr].AILunar==4)
   {
	Data->P[plr].AIPrim=6;
   Data->P[plr].AISec= (Data->P[plr].Manned[1].Safety>=Data->P[plr].Manned[4].Safety) ? 6 : 10;
   }

 // boosters
 if (Data->P[plr].AIStat>=2)
  {
   if (GenPur(plr,1,4)) RDafford(plr,1,4);
    else RDafford(plr,1,4);
  }

  if (CheckSafety(plr,Data->P[plr].AIPrim) >= CheckSafety(plr,Data->P[plr].AISec))
     KeepRD(plr,Data->P[plr].AIPrim); else KeepRD(plr,Data->P[plr].AISec);

// larger rocket klugge
  if (Data->P[plr].Track[5]==1)
  {
  if (Level_Check!=0) Data->P[plr].Cash+=25; // temporary
  if (Data->P[plr].AILunar<4)
   {
    if (GenPur(plr,1,2)) RDafford(plr,1,2);
     else RDafford(plr,1,2);
    Data->P[plr].Buy[1][2]=0;RDafford(plr,1,2);
   }
  else
   {
    if (Level_Check!=0) Data->P[plr].Cash+=25;
    if (GenPur(plr,1,3)) RDafford(plr,1,3);
     else RDafford(plr,1,3);
    Data->P[plr].Buy[1][3]=0;RDafford(plr,1,3);
   }
 }

if (Data->Year>=62)
 if (Data->P[plr].Track[2]<5) PickModule(plr);

   if (Data->P[plr].AILunar>0 && Data->P[plr].AILunar<4)
    {
	  if (Data->P[plr].Track[2]>0)
	   {
	    if (GenPur(plr,2,Data->P[plr].Track[2])) 
	     RDafford(plr,2,Data->P[plr].Track[2]);
          else RDafford(plr,2,Data->P[plr].Track[2]);
       Data->P[plr].Buy[2][Data->P[plr].Track[2]]=0;
		 RDafford(plr,2,Data->P[plr].Track[2]);
	   }
   }

 for (i=0;i<3;i++)
  if (Data->P[plr].LaunchFacility[i]>1)
   if (Data->P[plr].LaunchFacility[i]<=Data->P[plr].Cash)
	 {
	  Data->P[plr].Cash-=Data->P[plr].LaunchFacility[i];
	  Data->P[plr].LaunchFacility[i]=1;
   }   
 Data->P[plr].LaunchFacility[0]=1;
 Data->P[plr].LaunchFacility[1]=1;
 Data->P[plr].LaunchFacility[2]=1;
 if (Data->P[plr].AIStat==3) {
 switch(Data->P[plr].AILunar)
  {
	case 1:MoonProgram(plr,1);break;
	case 2:MoonProgram(plr,2);break;
	case 3:MoonProgram(plr,3);break;
	case 4:MoonProgram(plr,4);
          if (GenPur(plr,1,3)) RDafford(plr,1,3);
			  RDafford(plr,1,3);
			 break;
	  default:break;
  }
 }
 if (Data->P[plr].AIStat>=2 && Data->Year>=61 && Data->P[plr].Track[1]>=2)
 {
  if ((Data->P[plr].Probe[0].Safety>=Data->P[plr].Probe[0].MaxRD-20) || Data->P[plr].Probe[1].Num>=0)
   {
    Data->P[plr].Cash+=Data->P[plr].Probe[1].InitCost+30;
    if (Data->P[plr].Probe[1].Num<=0) {
	  if (GenPur(plr,0,1)) RDafford(plr,0,1);
	   else RDafford(plr,0,1);
    }
    Data->P[plr].Buy[0][1]=0;RDafford(plr,0,1);
   }
  if ((Data->P[plr].Probe[1].Safety>=Data->P[plr].Probe[1].MaxRD-20) || Data->P[plr].Probe[2].Num>=0)
   {
    Data->P[plr].Cash+=Data->P[plr].Probe[2].InitCost+30;
    if (Data->P[plr].Probe[2].Num<=0) {
	  if (GenPur(plr,0,2)) RDafford(plr,0,2);
	    else RDafford(plr,0,2);
    }
    Data->P[plr].Buy[0][2]=0;RDafford(plr,0,2);
   }
   if (GenPur(plr,1,1)) RDafford(plr,1,1);
	 else RDafford(plr,1,1);
  }
 if (PreChe(plr,27) || PreChe(plr,18)) Data->P[plr].AIStat=3;

// **** end stages ***
 if (Data->P[plr].AIStat==3)
 {
  if (Data->P[plr].AILunar<4)
   {
    if (CheckSafety(plr,Data->P[plr].AIPrim)>CheckSafety(plr,Data->P[plr].AISec))
	  val=Data->P[plr].AIPrim; else val=Data->P[plr].AISec;
    if (val<7) val=val-4;
     else val=val-5;
    if (Data->P[plr].Manned[val-1].Safety>=Data->P[plr].Manned[val-1].MaxRD-15)
     NewAI(plr,val);
   }
  else if (Data->P[plr].AILunar==4)
   {
    if (Data->P[plr].Manned[4].Safety>=Data->P[plr].Manned[4].MaxRD-10)
      Data->P[plr].AISec=10; else Data->P[plr].AISec=6;

    if (CheckSafety(plr,Data->P[plr].AIPrim)>CheckSafety(plr,Data->P[plr].AISec))
	  val=Data->P[plr].AIPrim; else val=Data->P[plr].AISec;
    if (val<7) val=val-4;
     else val=val-5;
    if (Data->P[plr].Manned[val-1].Safety>=Data->P[plr].Manned[val-1].MaxRD-15)
     NewAI(plr,val);
      else {
          Data->P[plr].Probe[0].Num+=2;
          Data->P[plr].Rocket[0].Num+=2;
          AIFuture(plr,1,0,0);
          AIFuture(plr,1,1,0);
          AIFuture(plr,1,2,0);
         }
   }

  if (CheckSafety(plr,Data->P[plr].AIPrim)>CheckSafety(plr,Data->P[plr].AISec))
   {
    val=Data->P[plr].AIPrim;
    if (val<7) val=val-4;
     else val=val-5;
    if (CheckSafety(plr,Data->P[plr].AIPrim)>Data->P[plr].Manned[val-1].MaxRD-10)
     if (Data->P[plr].Manned[val-1].Num>=1) KeepRD(plr,Data->P[plr].AISec);
   }
  else
   {
    val=Data->P[plr].AISec;
    if (val<7) val=val-4;
     else val=val-5;
    if (CheckSafety(plr,Data->P[plr].AISec)>Data->P[plr].Manned[val-1].MaxRD-10)
     if (Data->P[plr].Manned[val-1].Num>=1) KeepRD(plr,Data->P[plr].AIPrim);
   }
 } 
 RDPrograms(plr);
 return;
}


char NoFail(char plr)
{
 char RT_value=0,i;
 for (i=0;i<Data->P[plr].PastMis;i++)
   if ((Data->P[plr].History[i].MissionCode==4 || Data->P[plr].History[i].MissionCode==2) && Data->P[plr].History[i].spResult>=3000)
     ++RT_value;
 if (RT_value>=2) {
    RT_value=1;Data->P[plr].AIStat=3;
   }
 return(RT_value);
}


void RDPrograms(char plr)
{
 int i;
 for (i=0;i<7;i++)
  {
   if (Data->P[plr].Manned[i].Num>=0)
	 RDafford(plr,2,i);
   Data->P[plr].Manned[i].Damage=0;
   Data->P[plr].Manned[i].DCost=0;
  };
 for (i=0;i<5;i++)
  {
   if (Data->P[plr].Rocket[i].Num>=0)
    RDafford(plr,1,i);
   Data->P[plr].Rocket[i].Damage=0;
   Data->P[plr].Rocket[i].DCost=0;
  };
 for (i=0;i<4;i++)
  {
   if (Data->P[plr].Misc[i].Num>=0)
	 RDafford(plr,3,i);
   Data->P[plr].Misc[i].Damage=0;
   Data->P[plr].Misc[i].DCost=0;
  };
 for (i=0;i<3;i++)
  {
   if (Data->P[plr].Probe[i].Num>=0)
	 RDafford(plr,0,i);
   Data->P[plr].Probe[i].Damage=0;
   Data->P[plr].Probe[i].DCost=0;
  };
 return;
}

void CheckVoting(char plr)
{
 int st;
 if (Data->P[plr].AILunar==0) MoonVoting(plr);
   else
	if (Data->P[plr].AILunar==4)
	  {
	   st= GenPur(plr,2,4);
	   if (st==1)
		 {
		  RDafford(plr,2,4);
		  Data->P[plr].AILunar=4; // lunar landing set
		 }
	  }
 return;
}

void KeepRD(char plr,int m)
{
 //reassing player level
 if (plr==0) Level_Check=(Data->Def.Lev1==0) ? 0 : 1;
  else  if (plr==1) Level_Check=(Data->Def.Lev2==0) ? 0 : 1;
 if (m < 4) return; // hasn't voted yet
  switch(m)
	 {
	  case 5:if (Data->P[plr].Rocket[0].Num<=Data->P[plr].Manned[0].Num)
			   {
				if (GenPur(plr,1,0)) RDafford(plr,1,0);
				if (GenPur(plr,2,0)) RDafford(plr,2,0);
				RDafford(plr,2,0);
				RDafford(plr,1,0);
			   }
			  else
			   {
				if (GenPur(plr,2,0)) RDafford(plr,2,0);
				if (GenPur(plr,1,0)) RDafford(plr,1,0);
				RDafford(plr,1,0);
				RDafford(plr,2,0);

			   }
			 break;
	  case 6:if (Data->P[plr].Rocket[1].Num<=Data->P[plr].Manned[1].Num)
			   {
				 if (GenPur(plr,1,1)) RDafford(plr,1,1);
               else RDafford(plr,1,1);
				 if (GenPur(plr,2,1)) RDafford(plr,2,1);
               else RDafford(plr,2,1);
             if (Level_Check!=0) {
             Data->P[plr].Buy[1][1]=0;Data->P[plr].Buy[2][1]=0;
				 RDafford(plr,2,1);RDafford(plr,1,1);}
			   }
			 else
			   {
				 if (GenPur(plr,2,1)) RDafford(plr,2,1);
              else RDafford(plr,2,1);
				 if (GenPur(plr,1,1)) RDafford(plr,1,1);
              else RDafford(plr,1,1);
             if (Level_Check!=0) {
             Data->P[plr].Buy[1][1]=0;Data->P[plr].Buy[2][1]=0;
				 RDafford(plr,1,1);RDafford(plr,2,1);}
			   }
			 break;
	  case 8:if (Data->P[plr].Rocket[2].Num<=Data->P[plr].Manned[2].Num)
			   {
				 if (GenPur(plr,1,2)) RDafford(plr,1,2);
              else RDafford(plr,1,2);
				 if (GenPur(plr,2,2)) RDafford(plr,2,2);
              else RDafford(plr,2,2);
             if (Level_Check!=0) {
             Data->P[plr].Buy[1][2]=0;Data->P[plr].Buy[2][2]=0;
				 RDafford(plr,2,2);RDafford(plr,1,2);}
			   }
			 else
			   {
				if (GenPur(plr,2,2)) RDafford(plr,2,2);
             else RDafford(plr,2,2);
				if (GenPur(plr,1,2)) RDafford(plr,1,2);
             else RDafford(plr,1,2);
            if (Level_Check!=0) {
            Data->P[plr].Buy[1][2]=0;Data->P[plr].Buy[2][2]=0;
				RDafford(plr,1,2);RDafford(plr,2,2);}
			   }
		  break;
	  case 9:if (Data->P[plr].Rocket[3].Num<=Data->P[plr].Manned[3].Num)
			   {
				if (GenPur(plr,1,3)) RDafford(plr,1,3);
             else RDafford(plr,1,3);
				if (GenPur(plr,2,3)) RDafford(plr,2,3);
             else RDafford(plr,2,3);
            if (Level_Check!=0) {
            Data->P[plr].Buy[1][3]=0;Data->P[plr].Buy[2][3]=0;
				RDafford(plr,2,3);RDafford(plr,1,3);}
			   }
			  else
			   {
				if (GenPur(plr,2,3)) RDafford(plr,2,3);
             else RDafford(plr,2,3);
				if (GenPur(plr,1,3)) RDafford(plr,1,3);
             else RDafford(plr,1,3);
            if (Level_Check!=0) {
            Data->P[plr].Buy[1][3]=0;Data->P[plr].Buy[2][3]=0;
				RDafford(plr,1,3);RDafford(plr,2,3);}
			   }
         break;
      case 10:if (Data->P[plr].Rocket[3].Num<=Data->P[plr].Manned[4].Num)
			   {
				if (GenPur(plr,1,3)) RDafford(plr,1,3);
             else RDafford(plr,1,3);
				if (GenPur(plr,2,4)) RDafford(plr,2,4);
             else RDafford(plr,2,4);
            Data->P[plr].Buy[1][3]=0;Data->P[plr].Buy[2][4]=0;
				RDafford(plr,2,4);RDafford(plr,1,3);
			   }
			  else
			   {
				if (GenPur(plr,2,4)) RDafford(plr,2,4);
             else RDafford(plr,2,4);
				if (GenPur(plr,1,3)) RDafford(plr,1,3);
             else RDafford(plr,1,3);
            Data->P[plr].Buy[1][3]=0;Data->P[plr].Buy[2][4]=0;
				RDafford(plr,1,3);RDafford(plr,2,4);
			   }
			  break;
	  default:break;
	 }
 return;
}

void PickModule(char plr)
{
 Data->P[plr].Track[2]=6; 
 if (GenPur(plr,2,6)) RDafford(plr,2,6);
   RDafford(plr,2,6);
 return;
}

void MoonVoting(char plr)
{
 int high=-1,val;
val=random(100)+1;
if (val<70) high=0;
 else if (val<78) high=1;
  else if (val<92) high=2;
   else high=3;
 if (high!=-1)
   {
	switch (high)
	 {
	  case 0: Data->P[plr].AILunar=1;
		  PickModule(plr);
		  MoonProgram(plr,1);
		  break;
	  case 1: Data->P[plr].AILunar=2;
		  PickModule(plr);
		  MoonProgram(plr,2);
		  break;
	  case 2: Data->P[plr].AILunar=3;
		  PickModule(plr);
		  MoonProgram(plr,3);
		  break;
	  case 3: Data->P[plr].AILunar=4;
		  MoonProgram(plr,4);
		  break;
	  default:break;
	 }
   }
 return;
}

#ifdef DEAD_CODE
int CheckLimit(char plr,char m)
{
 switch(m)
	 {
	  case 5: return(Data->P[plr].Manned[0].Num);
	  case 6: return(Data->P[plr].Manned[1].Num);
	  case 8: return(Data->P[plr].Manned[2].Num);
	  case 9: return(Data->P[plr].Manned[3].Num);
     case 10: return(Data->P[plr].Manned[4].Num);
	  default:break;
	 }
 return 0;
}
#endif

int CheckSafety(char plr,char m)
{
 switch(m)
   {
	case 5: return(Data->P[plr].Manned[0].Safety);
	case 6: return(Data->P[plr].Manned[1].Safety);
	case 8: return(Data->P[plr].Manned[2].Safety);
	case 9: return(Data->P[plr].Manned[3].Safety);
   case 10: return(Data->P[plr].Manned[4].Safety);
	default:break;
   }
 return 0;
}

void MoonProgram(char plr,char m)
{
 switch(m)
  {
   case 1:if (GenPur(plr,3,1)) RDafford(plr,3,1);
			   RDafford(plr,3,1);
          Data->P[plr].Buy[3][1]=0;RDafford(plr,3,1);
		    break;
   case 2:if (GenPur(plr,3,1)) RDafford(plr,3,1);
			   RDafford(plr,3,1);
          Data->P[plr].Buy[3][1]=0;RDafford(plr,3,1);
		    break;
   case 3:if (GenPur(plr,3,0)) RDafford(plr,3,0);
			   RDafford(plr,3,0);
            Data->P[plr].Buy[3][0]=0;RDafford(plr,3,0);
		    break;
   case 4:if (GenPur(plr,2,4)) RDafford(plr,2,4);
			   RDafford(plr,2,4);
		    if (GenPur(plr,3,1)) RDafford(plr,3,1);
			   RDafford(plr,3,1);
         Data->P[plr].Buy[3][1]=0;RDafford(plr,3,1);
		   break;
   default:break;
  }
 return;
}

void ProgramVoting(char plr)
{
 int i=0;
 i=random(100);
 if (i<65) Data->P[plr].AIPrim=8;
  else Data->P[plr].AIPrim=9;
 return;
}

/* EOF */



