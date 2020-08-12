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
// AI Master Routines

#include <Buzz_inc.h>
#include <externs.h>

struct cStr {i16 cost,sf,i;} Mew[5];
int whe[2],rck[2];
char pc[2],bc[2],Alt_A[2]={0,0},Alt_B[2]={0,0},Alt_C[2]={0,0};
extern struct VInfo VAS[7][4];
extern char VASqty;
extern struct mStr Mis;
extern enum Opponent_Status Cur_Status;
void Strategy_One(char plr,int *m_1,int *m_2,int *m_3);
void Strategy_Two(char plr,int *m_1,int *m_2,int *m_3);
void Strategy_Thr(char plr,int *m_1,int *m_2,int *m_3);

void AIVabCheck(char plr,char mis,char prog)
{
  VASqty=0;
  //prog=1; 0=UnM : 1=1Mn ...
  GetMisType(mis);
  whe[0]=whe[1]=-1;

  if (prog==5) {
	if(Mis.Jt==0 && Mis.LM==0 && Mis.Doc==0) {
	  BuildVAB(plr,mis,1,0,prog-1);
	  CalcSaf(plr,VASqty);
	  whe[0]=Best();
	  if (Mew[whe[0]].i<60) whe[0]=0;  // Weed out low safety's
	}
  }
  else if (prog>=Mis.mEq && (prog!=0)) // && (Mis.mVab[0]&0x80 || Mis.mVab[1]&0x80)) )
  {
	if (Mis.Jt==1) {                        // Joint mission
	  BuildVAB(plr,mis,1,0,prog-1);           // first launch
	  CalcSaf(plr,VASqty);
	  whe[0]=Best();
	  if (Mew[whe[0]].i<60) whe[0]=0;

	  BuildVAB(plr,mis,1,1,prog-1);
	  CalcSaf(plr,VASqty);
	  whe[1]=Best();
	  if (Mew[whe[1]].i<60) whe[1]=0;  // Weed out low safety's
	} else {
	 // RectFill(100,100,200,190,5);
	 // PrintAt(110,110,"MIS ");DispNum(0,0,mis);
	 // PrintAt(110,120,"PRG ");DispNum(0,0,prog);
	 // PauseMouse();

	  BuildVAB(plr,mis,1,0,prog-1);
//        PrintAt(110,130,"VAS ");DispNum(0,0,VASQTY);
	  CalcSaf(plr,VASqty);
	  whe[0]=Best();
	  if (Mew[whe[0]].i<60) whe[0]=0;
	}
  }
  else if (prog==0 && prog==Mis.mEq) {  // Unmanned Vechicle
	BuildVAB(plr,mis,1,0,prog);             //  plr,mcode,ty=1,part,prog
	CalcSaf(plr,VASqty);
	whe[0]=Best();
       // if (Mew[whe[0]].i<30) whe[0]=0;
   // ShowVA(whe[0]);
  }
  return;
}

char Best(void)
{
  int i,j,ct,ct1;
  char valid[5];
  for (i=0;i<5;i++) valid[i]=0;

  for (i=1;i<VASqty+1;i++) {
	ct=ct1=0;
	for (j=0;j<4;j++) {
	  if (strncmp("NONE",&VAS[i][j].name[0],4)!=0) ct1++;
	  if (VAS[i][j].qty>=0) ct+=VAS[i][j].sf;
	}
	if (ct1>0) valid[i]= ct/ct1;
  }
  ct1=0;
  for (i=1;i<VASqty+1;i++) ct1=(valid[i]>valid[ct1])?i:ct1;
  return ct1;
}


int ICost(char plr,char h,char i)
{
  int cost=0;
  switch(h) {
    case 0: case 2:     // Capsules & LM's
	   cost=cost+Data->P[plr].Manned[i].MaxRD-Data->P[plr].Manned[i].Safety;
	   cost=cost/3.5;
	   cost=cost*Data->P[plr].Manned[i].RDCost;
	 if (Data->P[plr].Manned[i].Num<0) cost+=Data->P[plr].Manned[i].InitCost;
	 if (Data->P[plr].Manned[i].Num==0) cost+=Data->P[plr].Manned[i].UnitCost;
	 break;
	case 1:   // Kickers
	   cost=cost+Data->P[plr].Misc[i].MaxRD-Data->P[plr].Misc[i].Safety;
	   cost=cost/3.5;
	   cost=cost*Data->P[plr].Misc[i].RDCost;
	 if (Data->P[plr].Misc[i].Num<0) cost+=Data->P[plr].Misc[i].InitCost;
	 if (Data->P[plr].Misc[i].Num==0) cost+=Data->P[plr].Misc[i].UnitCost;
	 break;
	case 3:
	 if (i<4) {
	   cost=cost+Data->P[plr].Probe[i].MaxRD-Data->P[plr].Probe[i].Safety;
	   cost=cost/3.5;
	   cost=cost*Data->P[plr].Probe[i].RDCost;
	   if (Data->P[plr].Probe[i].Num<0) cost+=Data->P[plr].Probe[i].InitCost;
	   if (Data->P[plr].Probe[i].Num==0) cost+=Data->P[plr].Probe[i].UnitCost;
	 } else {
	   if (Data->P[plr].Misc[4].Num<0) cost+=Data->P[plr].Misc[4].InitCost;
	   if (Data->P[plr].Misc[4].Num==0) cost+=Data->P[plr].Misc[4].UnitCost;
	 }
	 break;
	default: break;
  }
  return cost;
}


void CalcSaf(char plr,char vs)
{
  int i,j,k,sum=0,co=0,t=0;

  for (i=0;i<14;i++) Mew[i].cost=Mew[i].sf=0; // Clear thing

	// Do first part

   for (j=1;j<vs+1;j++) {
	  for (k=0;k<4;k++) {
	if (VAS[j][k].qty>=0) sum+=VAS[j][k].sf;
	if (strncmp("NONE",&VAS[j][k].name[0],4)!=0) t++;
	if (VAS[j][k].wt>0)  co+=ICost(plr,k,VAS[j][k].dex);
	  }
	  Mew[j].cost+=co; co=0;
	  Mew[j].sf+=sum;  sum=0;
	  if (t>0) Mew[j].i=Mew[j].sf/t; t=0;
   }
  return;
}

char Panic_Level(char plr,int *m_1,int *m_2)
{
 // PANIC level manned docking/EVA/duration
 if (Alt_B[plr]<=1 && Data->P[plr].Track[1]==4 && PreChe(plr,24)==0 && PreChe(plr,26)==0 &&
     Data->P[plr].Mission[0].MissionCode!=15 && Data->P[plr].Mission[1].MissionCode!=20)
  {
   *m_1=15;*m_2=20;++Alt_B[plr];return(1);
  }
 // PANIC lunar pass/probe landing/lunar fly-by
 if (Data->P[plr].Track[1]==5 && !PreChe(plr,1) && !PreChe(plr,7) && Cur_Status==Ahead && Alt_A[plr]<=2)
  {
   *m_1=7;if (Data->P[plr].DurLevel<=2) *m_2=25;else *m_2=8;++Alt_A[plr];return(1);
  }
   // PANIC level duration/pass/lunar orbital/LM_pts
 return(0);
}

void Strategy_One(char plr,int *m_1,int *m_2,int *m_3)
{
 //AI version 12/26/92
 switch(Data->P[plr].Track[1])
  {
   case 0:// mission 26 -> if manned docking and eva  -> DurLevel+1
          *m_1=15;*m_2=15;++Data->P[plr].Track[1];*m_3=7;break;
   case 1:*m_1=15;if (PreChe(plr,26)==0) *m_2=20;else *m_2=14;++Data->P[plr].Track[1];*m_3=7;break;
   case 2:if (PreChe(plr,24) && PreChe(plr,26))
           {*m_1=25;*m_2=27;++Data->P[plr].Track[1];}
            else {*m_1=15;*m_2=20;++Data->P[plr].Track[1];}
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;
          break;
   case 3:*m_1=27;if (Cur_Status==Behind) *m_2=25;
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;
          ++Data->P[plr].Track[1];break;
   case 4:switch(Data->P[plr].DurLevel)
           {
            case 0: case 1:*m_1=25;*m_2=26;break;
            case 2:*m_1=27;*m_2=(Data->P[plr].Probe[1].Safety>=Data->P[plr].Probe[1].MaxRD-10)? 7: 15;*m_3=7;break;
            case 3: case 4:case 5:*m_1=15;*m_2=7;*m_3=8;++Data->P[plr].Track[1];break;
            default:break;
           }
          if (Data->P[plr].Cash<=0) Data->P[plr].Cash=0;
           Data->P[plr].Cash+=Data->P[plr].Rocket[2].InitCost+25;
          if (GenPur(plr,1,2)) RDafford(plr,1,2);
           else RDafford(plr,1,2);
          if (Data->P[plr].Rocket[2].Num>=0) Data->P[plr].Track[5]=1;
          Data->P[plr].Buy[1][2]=0;RDafford(plr,1,2);
          break;
   case 5:*m_1=43;if (Cur_Status==Behind) *m_2=46;
          //lunar pass
          ++Data->P[plr].Track[1];
          break;
   case 6:if (Data->P[plr].Manned[6].Safety>Data->P[plr].Manned[6].MaxRD-10)
           *m_1=48; else *m_1=46; //lunar orbit
          ++Data->P[plr].Track[1];
          break;
   case 7:if (PreChe(plr,19)==0) {*m_1=43;Data->P[plr].Track[1]=6;}
           else if (PreChe(plr,20)==0 && Data->P[plr].Mission[0].MissionCode!=46) *m_1=46;
            else *m_1=48;
          ++Data->P[plr].Track[1];
          break;
   case 8:if (PreChe(plr,20)==0) {if (Cur_Status==Behind) *m_1=48;else *m_1=46;}
           else if (Data->P[plr].LMpts==0 && Data->P[plr].Mission[0].MissionCode!=48) *m_1=48;
            else *m_1=53;
          ++Data->P[plr].Track[1];
          break;
   case 9:if (Data->P[plr].Misc[4].Safety>=80)
           {
            switch(Data->P[plr].LMpts)
             {
              case 0:case 1:if (Data->P[plr].Mission[0].MissionCode==48) *m_1=53;
                      else {*m_1=48;}break;
              case 2: case 3:*m_1=53;break;
              default:*m_1=53;break;
             }
           }
           else {
               *m_1=15;
               if (Data->P[plr].Misc[4].Safety<60) *m_2=14; else *m_2=15;
               }
          break;
     default:break;
    }
 return;
}

void Strategy_Two(char plr,int *m_1,int *m_2,int *m_3)
{
 // AI version 12/28/92
 switch(Data->P[plr].Track[1])
  {
   case 0:*m_1=15;*m_2=15;++Data->P[plr].Track[1];*m_3=7;break;
   case 1:*m_1=15;*m_2=14;++Data->P[plr].Track[1];*m_3=7;break;
   case 2:if (PreChe(plr,24) && PreChe(plr,26))
           {*m_1=25;*m_2=27;++Data->P[plr].Track[1];}
            else {*m_1=15;*m_2=20;++Data->P[plr].Track[1];}
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;
          break;
   case 3:*m_1=25;*m_2=27;
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;else *m_3=7;
          ++Data->P[plr].Track[1];break;
   case 4:switch(Data->P[plr].DurLevel)
           {
            case 0: case 1:*m_1=25;*m_2=26;*m_3=7;break;
            case 2:*m_1=27;*m_2=(Data->P[plr].Probe[1].Safety>=Data->P[plr].Probe[1].MaxRD-10)? 7: 15;*m_3=7;break;
            case 3: case 4:case 5:*m_1=15;*m_2=7;*m_3=8;++Data->P[plr].Track[1];break;
            default:break;
           }
          if (Data->P[plr].Cash<=0) Data->P[plr].Cash=0;
           Data->P[plr].Cash+=Data->P[plr].Rocket[2].InitCost+25;
          if (GenPur(plr,1,2)) RDafford(plr,1,2);
           else RDafford(plr,1,2);
          if (Data->P[plr].Rocket[2].Num>=0) Data->P[plr].Track[5]=1;
          Data->P[plr].Buy[1][2]=0;RDafford(plr,1,2);
          break;
   case 5:*m_1=43;if (Cur_Status==Behind) *m_2=46;
          //lunar pass
          ++Data->P[plr].Track[1];
          break;
   case 6:if (Data->P[plr].Manned[6].Safety>Data->P[plr].Manned[6].MaxRD-10)
           *m_1=46; //lunar orbit
          ++Data->P[plr].Track[1];
          break;
   case 7:if (PreChe(plr,19)==0) {*m_1=43;Data->P[plr].Track[1]=6;}
           else if (PreChe(plr,20)==0 && Data->P[plr].Mission[0].MissionCode!=46) *m_1=46;
            else *m_1=48;
          ++Data->P[plr].Track[1];
          break;
   case 8:if (PreChe(plr,20)==0) {if (Cur_Status==Behind) *m_1=48;else *m_1=46;}
           else if (Data->P[plr].LMpts==0 && Data->P[plr].Mission[0].MissionCode!=48) *m_1=48;
            else *m_1=53;
          ++Data->P[plr].Track[1];
          break;
   case 9:if (Data->P[plr].Misc[4].Safety>=80)
           {
            switch(Data->P[plr].LMpts)
             {
              case 0:case 1:if (Data->P[plr].Mission[0].MissionCode==48) *m_1=53;
                      else {*m_1=48;}break;
              case 2: case 3:*m_1=53;break;
              default:*m_1=53;break;
             }
           }
           else {
               *m_1=15;
               if (Data->P[plr].Misc[4].Safety<60) *m_2=14; else *m_2=15;
               }
          break;
     default:break;
    }
 return;
}

void Strategy_Thr(char plr,int *m_1,int *m_2,int *m_3)
{
 //new version undated
 switch(Data->P[plr].Track[1])
  {
   case 0:// mission 26 -> if manned docking and eva  -> DurLevel+1
          *m_1=15;*m_2=15;++Data->P[plr].Track[1];*m_3=7;break;
   case 1:*m_1=15;if (PreChe(plr,26)==0) *m_2=20;else *m_2=14;++Data->P[plr].Track[1];*m_3=7;break;
   case 2:if (PreChe(plr,24) && PreChe(plr,26))
           {*m_1=25;*m_2=27;++Data->P[plr].Track[1];}
            else {*m_1=15;*m_2=20;++Data->P[plr].Track[1];}
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;
          break;
   case 3:*m_1=27;if (Cur_Status==Behind) *m_2=25;
          if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-10)
           *m_3=8;
          ++Data->P[plr].Track[1];break;
   case 4:switch(Data->P[plr].DurLevel)
           {
            case 0: case 1:*m_1=25;*m_2=26;break;
            case 2:*m_1=27;*m_2=(Data->P[plr].Probe[1].Safety>=Data->P[plr].Probe[1].MaxRD-10)? 7: 15;*m_3=7;break;
            case 3: case 4:case 5:*m_1=15;*m_2=7;*m_3=8;++Data->P[plr].Track[1];break;
            default:break;
           }
          if (Data->P[plr].Cash<=0) Data->P[plr].Cash=0;
           Data->P[plr].Cash+=Data->P[plr].Rocket[2].InitCost+25;
          if (GenPur(plr,1,2)) RDafford(plr,1,2);
           else RDafford(plr,1,2);
          if (Data->P[plr].Rocket[2].Num>=0) Data->P[plr].Track[5]=1;
          Data->P[plr].Buy[1][2]=0;RDafford(plr,1,2);
          break;
   case 5:*m_1=43;if (Cur_Status==Behind) *m_2=46;
          //lunar pass
          ++Data->P[plr].Track[1];
          break;
   case 6:if (Data->P[plr].Manned[6].Safety>Data->P[plr].Manned[6].MaxRD-10)
           *m_1=46; //lunar orbit
          ++Data->P[plr].Track[1];
          break;
   case 7:if (PreChe(plr,19)==0) {*m_1=43;Data->P[plr].Track[1]=6;}
           else if (PreChe(plr,20)==0 && Data->P[plr].Mission[0].MissionCode!=46) *m_1=46;
            else *m_1=48;
          ++Data->P[plr].Track[1];
          break;
   case 8:if (PreChe(plr,20)==0) {if (Cur_Status==Behind) *m_1=48;else *m_1=46;}
           else if (Data->P[plr].LMpts==0 && Data->P[plr].Mission[0].MissionCode!=48) *m_1=48;
            else *m_1=53;
          ++Data->P[plr].Track[1];
          break;
   case 9:if (Data->P[plr].Misc[4].Safety>=80)
           {
            switch(Data->P[plr].LMpts)
             {
              case 0:case 1:if (Data->P[plr].Mission[0].MissionCode==48) *m_1=53;
                      else {*m_1=48;}break;
              case 2: case 3:*m_1=53;break;
              default:*m_1=53;break;
             }
           }
           else {
               *m_1=15;
               if (Data->P[plr].Misc[4].Safety<60) *m_2=14; else *m_2=15;
               }
          break;
     default:break;
    }
 return;
}

void NewAI(char plr,char frog)
{
 char i,spc[2],prg[2],P_pad,B_pad,hsf,Panic_Check=0;
 int mis1,mis2,mis3,val;

 spc[0] = 0; /* XXX check uninitialized */

 prg[0]=frog;
 mis1=mis2=mis3=P_pad=B_pad=0;
 GenPur(plr,2,frog-1);
 if (Data->P[plr].AILunar<4)
  {
	mis1=0;mis2=0;mis3=0;hsf=0;
	for(i=0;i<3;i++)
	 if (Data->P[plr].Probe[hsf].Safety<=Data->P[plr].Probe[i].Safety) hsf=i;
   RDafford(plr,0,hsf);
	if (Data->P[plr].Probe[hsf].Safety<90)
	 {
	  if (GenPur(plr,0,hsf)) RDafford(plr,0,hsf);
		else RDafford(plr,0,hsf);
	 }
   Data->P[plr].Misc[4].Num=2;
   Panic_Check=Panic_Level(plr,&mis1,&mis2);
  if (!Panic_Check)
   {
    if (Data->P[plr].Track[3]==1) Strategy_One(plr,&mis1,&mis2,&mis3);
     else if (Data->P[plr].Track[3]==2) Strategy_Two(plr,&mis1,&mis2,&mis3);
      else Strategy_Thr(plr,&mis1,&mis2,&mis3);
    if (mis1==53)
    switch(Data->P[plr].AILunar)
      {
       case 1:mis1=53; //Apollo behind Gemini
              if (frog==2 && (Data->P[plr].AISec==8 || Data->P[plr].AISec==9))
               {
                val=Data->P[plr].AISec;
                if (val<7) val=val-4;
                 else val=val-5;
                if (Data->P[plr].Manned[val-1].Safety>=Data->P[plr].Manned[val-1].MaxRD-10)
                 {
                  mis2=53;
                  spc[0]=val;
                 }
               }
              break;
       case 2:mis1=55;mis2=0;break;
       case 3:mis1=56;mis2=0;break;
       default:break;
      }
   }
  }
 else
  {
   switch(Data->P[plr].Track[1])
    {
     case 0:mis1=25;mis2=20;++Data->P[plr].Track[1];break;
     case 1:mis1=25;mis2=25;++Data->P[plr].Track[1];break;
     case 2:if (Data->P[plr].Cash<=0) Data->P[plr].Cash=0;
            Data->P[plr].Cash+=Data->P[plr].Rocket[3].InitCost+25;
            if (GenPur(plr,1,3)) RDafford(plr,1,3);
              else RDafford(plr,1,3);
            if (Data->P[plr].Rocket[3].Num>=0) Data->P[plr].Track[5]=1;
            Data->P[plr].Buy[1][3]=0;RDafford(plr,1,3);
            mis1=25;++Data->P[plr].Track[1];break;
     case 3:switch(Data->P[plr].DurLevel)
             {
              case 0: case 1:mis1=25;mis2=25;break;
              case 2:mis1=25;mis2=7;++Data->P[plr].Track[1];break;
              case 3: case 4:case 5:mis1=7;mis2=7;++Data->P[plr].Track[1];break;
              default:break;
             }
            break;
     case 4:mis1=25;mis2=7;++Data->P[plr].Track[1];break;
     case 5:switch(Data->P[plr].DurLevel)
             {
              case 0:case 1:case 2:mis1=25;break;
              case 3:mis1=(PreChe(plr,18)==0) ? 26 : 25;break;
              case 4:case 5:if (PreChe(plr,1)==plr || PreChe(plr,7)==plr) mis1=43;
                             else {mis1=7;mis2=8;}
                     break;
              default:break;
             }
            ++Data->P[plr].Track[1];
            break;
     case 6:mis1=43;++Data->P[plr].Track[1];break;
     case 7:if (PreChe(plr,19)==0) mis1=43;
             else mis1=46;
            ++Data->P[plr].Track[1];
            break;
     case 8:mis1=46;++Data->P[plr].Track[1];break;
     case 9:if (PreChe(plr,20)==0) mis1=46;++Data->P[plr].Track[1];break;
     case 10:if (PreChe(plr,20)==0) mis1=46; else mis1=54;break;
     default:break;
    }
  };
 // unmanned/manned klugge
 if (mis1==14 && mis2==15) {mis2=15;mis1=14;};
 //lunar fly-by/probe landing klugge
 if (mis1==7 && mis2==7)
   if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-15) mis2=8;

 GetMisType(mis1);
 // deal with lunar modules
 if (Mis.LM==1) {
   if (Data->P[plr].Track[2]>0)
	 {
	  if (GenPur(plr,2,Data->P[plr].Track[2])) RDafford(plr,2,Data->P[plr].Track[2]);
		else RDafford(plr,2,Data->P[plr].Track[2]);	 }
   else
	{
	 Data->P[plr].Track[2]=6;
	 if (GenPur(plr,2,Data->P[plr].Track[2])) RDafford(plr,2,Data->P[plr].Track[2]);
		else RDafford(plr,2,Data->P[plr].Track[2]);
	}
 }
 if (Mis.Jt==1)
	{ // JOINT LAUNCH
	 P_pad=-1;
	 if (Data->P[plr].Future[0].MissionCode==0 && Data->P[plr].LaunchFacility[0]==1 &&
	  Data->P[plr].Future[1].MissionCode==0 && Data->P[plr].LaunchFacility[1]==1) P_pad=0;
	 if (mis1>0)
	  if (P_pad!=-1)
		AIFuture(plr,mis1,P_pad,(char *)&prg);
	}
	else
	 { // SINGLE LAUNCH
	  if (mis1==54) prg[0]=5;
	  if (mis1==7 || mis1==8) prg[0]=0;
	  B_pad=-1;P_pad=-1;
	  if (Data->P[plr].Future[0].MissionCode==0 && Data->P[plr].LaunchFacility[0]==1) P_pad=0;
	  if (Data->P[plr].Future[1].MissionCode==0 && Data->P[plr].LaunchFacility[1]==1)
	   {if (P_pad==0) B_pad=1; else P_pad=1;}
	  if (Data->P[plr].Future[2].MissionCode==0 && Data->P[plr].LaunchFacility[2]==1)
	   {
	    if (P_pad!=0 && B_pad!=1)
	      {
	       if (P_pad==1) B_pad=2;
	        else if (P_pad==0 && B_pad==-1) B_pad=2;
	       else P_pad=2;
		   }
		};
	  if (mis1>0)
      {
		 if (P_pad!=-1)
		  AIFuture(plr,mis1,P_pad,(char *)&prg);
      }
	  if (mis2>0)
		{
       if (mis2==7 || mis2==8) prg[0]=0;
         else prg[0]=frog;
       if (mis2==53) prg[0]=spc[0];
		 if (B_pad!=-1)
		  AIFuture(plr,mis2,B_pad,(char *)&prg);
		}
     if (mis3>0)
		{
       prg[0]=frog;
		 if (B_pad!=-1)
		  AIFuture(plr,mis3,2,(char *)&prg);
		}
     }
	  if (Data->P[plr].Future[2].MissionCode==0 && Data->P[plr].LaunchFacility[2]==1)
		{
       if ((mis1==0 && frog==2 && (Data->P[plr].Manned[2].Safety>=Data->P[plr].Manned[2].MaxRD-10)) ||
         (Data->P[plr].Manned[3].Safety>=Data->P[plr].Manned[3].MaxRD-10))
          {
           if (PreChe(plr,27)==0 && PreChe(other(plr),27)==0) mis3=2;
            else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==0) mis3=4;
           if (mis3==0) {
            if (PreChe(plr,27)==0 && PreChe(other(plr),27)==1) mis3=2;
             else if (PreChe(plr,18)==0 && PreChe(other(plr),18)==1) mis3=4;
	   }
           frog=prg[0]=Data->P[plr].AISec-6;
          }
         if (mis3==0)
          if (mis1!=7 && mis1!=8)
           {
            if (mis1==7) mis3=8;
             else if (mis1==8) mis3=7;
		      if (Data->P[plr].Probe[2].Safety>Data->P[plr].Probe[2].MaxRD-15)
			   if (PreChe(plr,7)==0 || Data->P[plr].Misc[5].Safety<85)
			    {
			     if (mis3==0) mis3=8;
			    }
            if ((Data->P[plr].Probe[1].Safety>Data->P[plr].Probe[1].MaxRD-15) && mis3==0)
             {
		        if (PreChe(plr,1)==0 && PreChe(other(plr),1)==0 && Data->P[plr].Mission[2].MissionCode!=7) mis3=7;
               else if (PreChe(plr,2)==0 && PreChe(other(plr),2)==0 && Data->P[plr].Mission[2].MissionCode!=11) mis3=11;
                else if (PreChe(plr,3)==0 && PreChe(other(plr),3)==0 && Data->P[plr].Mission[2].MissionCode!=9) mis3=9;
                 else if (PreChe(plr,4)==0 && PreChe(other(plr),4)==0 && Data->P[plr].Mission[2].MissionCode!=10) mis3=10;
                else if (PreChe(plr,5)==0 && PreChe(other(plr),5)==0 && Data->P[plr].Mission[2].MissionCode!=12) mis3=12;
               else if (PreChe(plr,6)==0 && PreChe(other(plr),6)==0 && Data->P[plr].Mission[2].MissionCode!=13) mis3=13;
              if (mis3==0)
               {
                if (PreChe(plr,1)==0 && PreChe(other(plr),1)==1 && Data->P[plr].Mission[2].MissionCode!=7) mis3=7;
                 else if (PreChe(plr,2)==0 && PreChe(other(plr),2)==1 && Data->P[plr].Mission[2].MissionCode!=11) mis3=11;
                  else if (PreChe(plr,3)==0 && PreChe(other(plr),3)==1 && Data->P[plr].Mission[2].MissionCode!=9) mis3=9;
                   else if (PreChe(plr,4)==0 && PreChe(other(plr),4)==1 && Data->P[plr].Mission[2].MissionCode!=10) mis3=10;
                  else if (PreChe(plr,5)==0 && PreChe(other(plr),5)==1 && Data->P[plr].Mission[2].MissionCode!=12) mis3=12;
                 else if (PreChe(plr,6)==0 && PreChe(other(plr),6)==1 && Data->P[plr].Mission[2].MissionCode!=13) mis3=13;
               }
            }
         }
		  if (mis3==0)
			{
			 if (GenPur(plr,0,0)) RDafford(plr,0,0);
			  else RDafford(plr,0,0);
			 if (GenPur(plr,1,0)) RDafford(plr,1,0);
			  else RDafford(plr,1,0);
			 if (Data->P[plr].Probe[0].Num>=1 && Data->P[plr].Rocket[0].Num>=1) mis3=1;
			}
		  if (mis3!=2 || mis3!=4) prg[0]=0;
		  if (mis3>0) AIFuture(plr,mis3,2,(char *)&prg);
	 }
 AILaunch(plr);
 return;
}

void AIFuture(char plr,char mis,char pad,char *prog)
{
  int i,j;
  char prime,back,max,men;
  char fake_prog[2];

  if (prog == NULL) {
	  memset (fake_prog, 0, sizeof fake_prog);
	  prog = fake_prog;
  }

  GetMisType(mis);

  for (i=0;i<(Mis.Jt+1);i++) {
	Data->P[plr].Future[pad+i].MissionCode=mis;
	Data->P[plr].Future[pad+i].part=i;
	// duration
	if (Data->P[plr].DurLevel<=5 && Data->P[plr].Future[pad+i].Duration==0) {
	  if (Mis.Dur==1) Data->P[plr].Future[pad+i].Duration=
	    maxx(Mis.Days,minn(Data->P[plr].DurLevel+1,6));
	  else Data->P[plr].Future[pad+i].Duration=Mis.Days;
	}
   if (Data->P[plr].Mission[0].Duration==Data->P[plr].Future[pad+i].Duration ||
       Data->P[plr].Mission[1].Duration==Data->P[plr].Future[pad+i].Duration)
       ++Data->P[plr].Future[pad+i].Duration;

   if (pad==1 && Data->P[plr].Future[0].Duration==Data->P[plr].Future[pad+i].Duration)
     ++Data->P[plr].Future[pad+i].Duration;

   if (Data->P[plr].Future[pad+i].Duration>=6)
     Data->P[plr].Future[pad+i].Duration=6;

   // one man capsule duration klugge
   if (Data->P[plr].Future[pad+i].Prog==1)
    {
    if (Data->P[plr].DurLevel==0) Data->P[plr].Future[pad+i].Duration=1;
      else Data->P[plr].Future[pad+i].Duration=2;
    }; // limit duration 'C' one man capsule
   // lunar mission klugge
   if (Mis.Lun==1 || Data->P[plr].Future[pad+i].MissionCode==55 ||
    Data->P[plr].Future[pad+i].MissionCode==56 || Data->P[plr].Future[pad+i].MissionCode==53) Data->P[plr].Future[pad+i].Duration=4;
   // unmanned duration klugge
   if (Mis.Days==0) Data->P[plr].Future[pad+i].Duration=0;
	Data->P[plr].Future[pad+i].Joint=Mis.Jt;
	Data->P[plr].Future[pad+i].Month=0;
   if (mis==1) prog[i]=0;
   Data->P[plr].Future[pad+i].Prog=prog[0];
	if (prog[i]>0 && Mis.Days>0)
	  {
	   for (j=1;j<6;j++) DumpAstro(plr,j);
	   TransAstro(plr,prog[i]); //indexed OK
	   if (Data->P[plr].Future[pad+i].PCrew!=0)
		 prime=Data->P[plr].Future[pad+i].PCrew-1; else prime=-1;
	   if (Data->P[plr].Future[pad+i].BCrew!=0)
		 back=Data->P[plr].Future[pad+i].BCrew-1; else back=-1;
	   max=prog[i];
	   if (prog[i]>3) max=prog[i]-1;
	   Data->P[plr].Future[pad+i].Men=max;
	   men= Data->P[plr].Future[pad+i].Men;
	   if (prime!=-1)
		 for (j=0;j<men;j++)
		   Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][prime][j]-1].Prime=0;
	   if (back!=-1)
		 for (j=0;j<men;j++)
		   Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][back][j]-1].Prime=0;
	   Data->P[plr].Future[pad+i].PCrew=0;Data->P[plr].Future[pad+i].BCrew=0;
		pc[i]=-1;bc[i]=-1;
	   for (j=0;j<8;j++)
		 if (pc[i]==-1 && Data->P[plr].Crew[prog[i]][j][0]!=0 && Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][j][0]-1].Prime==0)
		  pc[i]=j;
	   if (pc[i]==-1)
		 {
		 // astronaut/duration klugge
		  if (Mis.Days>0) Data->P[plr].Future[pad+i].Men=max;
		  // no astronauts available have to go unmanned
		  Data->P[plr].Future[pad+i].Men=0;
		  Data->P[plr].Future[pad+i].PCrew=0;
		  Data->P[plr].Future[pad+i].BCrew=0;
		  return;
		 }
	   Data->P[plr].Future[pad+i].PCrew=pc[i]+1;
	   bc[i]=-1;
	   for (j=0;j<8;j++)
		  if (bc[i]==-1 && j!=pc[i] && Data->P[plr].Crew[prog[i]][j][0]!=0 && Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][j][0]-1].Prime==0)
			bc[i]=j;
	   Data->P[plr].Future[pad+i].BCrew=bc[i]+1;
	   for (j=0;j<men;j++)
		  Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][pc[i]][j]-1].Prime=4;
	   for (j=0;j<men;j++)
		  Data->P[plr].Pool[Data->P[plr].Crew[prog[i]][bc[i]][j]-1].Prime=2;
	  }
	else
	  {
	   Data->P[plr].Future[pad+i].Men=0;
	   Data->P[plr].Future[pad+i].PCrew=0;
	   Data->P[plr].Future[pad+i].BCrew=0;
	  }
  }
 // joint mission 55 and 56 men klugge
 if (mis==55 || mis==56)
  {
   Data->P[plr].Future[pad+1].Men=Data->P[plr].Future[pad].Men;
   Data->P[plr].Future[pad+1].PCrew=Data->P[plr].Future[pad].PCrew;
   Data->P[plr].Future[pad+1].BCrew=Data->P[plr].Future[pad].BCrew;
   Data->P[plr].Future[pad+1].Prog=Data->P[plr].Future[pad].Prog;
   Data->P[plr].Future[pad].Men=0;Data->P[plr].Future[pad].PCrew=0;
   Data->P[plr].Future[pad].BCrew=0;Data->P[plr].Future[pad].Prog=0;
   Data->P[plr].Future[pad+1].Duration=Data->P[plr].Future[pad].Duration;
   Data->P[plr].Future[pad].Duration=0;
  }
 return;
}

#ifdef DEAD_CODE
char Search(char plr,char m)
{
  return (Data->P[plr].M[m]);
}
#endif

#ifdef DEAD_CODE
void GetHard(char plr)
{
 int bwgt[7],prwt[8],i;
 char rck;
 for (i=0;i<8;i++) {
   prwt[i]=(i<3) ? (Data->P[plr].Probe[i].UnitWeight) : Data->P[plr].Manned[i-3].UnitWeight;}
 for (i=0;i<7;i++) {
  bwgt[i]=(i>3) ?
	   (Data->P[plr].Rocket[i-4].MaxPay+Data->P[plr].Rocket[4].MaxPay)
	  : Data->P[plr].Rocket[i].MaxPay; }
 rck=-1;
 for (i=0;i<7;i++)
   if (rck==-1 && bwgt[i] > prwt[Data->P[plr].WList[0].Asp]) rck=i;
 if (rck<2) i=GenPur(plr,1,rck);
   else {i=GenPur(plr,1,4);i=GenPur(plr,1,rck-4);}
 return;
}
#endif

#ifdef DEAD_CODE
char BoostCheck(char plr)
{
 char check = 0; /* XXX check unintialized */
 if (Data->P[plr].Rocket[4].Num>=2 && Data->P[plr].Rocket[4].Safety>75) check=1;
   else
	{
	 if (Data->P[plr].Rocket[4].Num==2) RDafford(plr,1,4);
	   else check=GenPur(plr,1,4);
	 if (check==1) RDafford(plr,1,4);
	 check=0;
	}
 return(check);
}
#endif

#ifdef DEAD_CODE
void Scheduler(char plr)
{
 char mis,prim,z,i,tmoon[4][2];
 mis=Data->P[plr].WList[0].Vle;
 GetMisType(mis);
  if (Mis.Jt==1)
	{ // JOINT LAUNCH
	  prim=-1;
	  for (i=0;i<2;i++)
		if (Data->P[plr].Future[i].MissionCode==0 && Data->P[plr].LaunchFacility[i]==1 &&
	  Data->P[plr].Future[i+1].MissionCode==0 && Data->P[plr].LaunchFacility[i+1]==1) prim=i;
	  if (prim!=-1)
		{
		 tmoon[0][0]=Data->P[plr].WList[0].Asp;tmoon[0][1]=Data->P[plr].WList[0].Asp;tmoon[1][0]=Mis.Days;tmoon[1][1]=Mis.Days;
		 AIFuture(plr,Data->P[plr].WList[0].Vle,prim,(char *)&tmoon[0]);
		}
	}
	else
	  { // SINGLE LAUNCH
	   z=-1;prim=-1;
	   if (Data->P[plr].Future[0].MissionCode==0 && Data->P[plr].LaunchFacility[0]==1) prim=0;
	   if (Data->P[plr].Future[1].MissionCode==0 && Data->P[plr].LaunchFacility[1]==1)
		 {if (prim==0) z=1; else prim=1;}
	   if (Data->P[plr].Future[2].MissionCode==0 && Data->P[plr].LaunchFacility[2]==1)
		 {if (prim==1) z=2; else prim=2;}
	   if (prim!=-1)
		{
		 tmoon[0][0]=Data->P[plr].WList[0].Asp;tmoon[0][1]=-1;tmoon[1][0]=Mis.Days;tmoon[1][1]=-1;
		 AIFuture(plr,Data->P[plr].WList[0].Vle,prim,(char *)&tmoon[0]);
		}
	   if (z!=-1)
		 if (Data->P[plr].WList[0].Vle>=1)
		  {
		   mis=Data->P[plr].WList[0].Vle;
		   GetMisType(mis);
		   tmoon[0][0]=Data->P[plr].WList[0].Asp;tmoon[0][1]=-1;tmoon[1][0]=Mis.Days;tmoon[1][1]=-1;
		   AIFuture(plr,Data->P[plr].WList[0].Vle,z,(char *)&tmoon[0]);
		  }
	  }
 for (i=0;i<3;i++) if (Data->P[plr].Future[i].MissionCode==0) AIFuture(plr,1,i,0);
 return;
}
#endif

void AILaunch(char plr)
{
  int i,j,k=0,l=0,JR=0,wgt,bwgt[7];
  char boos[7],bdex[7];


  for (i=0;i<7;i++) {
	bdex[i]=i;
	boos[i]=(i>3) ?
		RocketBoosterSafety(Data->P[plr].Rocket[i-4].Safety, Data->P[plr].Rocket[4].Safety)
	  : Data->P[plr].Rocket[i].Safety;
	bwgt[i]=(i>3) ?
	   (Data->P[plr].Rocket[i-4].MaxPay+Data->P[plr].Rocket[4].MaxPay)
	  : Data->P[plr].Rocket[i].MaxPay;

	if (boos[i]<60) boos[i]=-1;  // Get Rid of any Unsafe rkt systems
   if (Data->P[plr].Rocket[4].Num<1) for (j=4;j<7;j++)boos[j]=-1;
   for (j=0;j<4;j++) if (Data->P[plr].Rocket[j].Num<1) boos[j]=-1;
  }

  for (i=0;i<3;i++)
   {
	 if (Data->P[plr].Mission[i].MissionCode==28 && Data->P[plr].DMod==0)
	  {
	   Data->P[plr].Mission[i].MissionCode=0;
	   return;
	  }
	 if (Data->P[plr].Mission[i].MissionCode>0 && Data->P[plr].Mission[i].part==0)
     {
	   whe[0]=whe[1]=-1;
	   if (Data->P[plr].Mission[i].Joint==1) AIVabCheck(plr,Data->P[plr].Mission[i].MissionCode,Data->P[plr].Mission[i+1].Prog);
       else AIVabCheck(plr,Data->P[plr].Mission[i].MissionCode,Data->P[plr].Mission[i].Prog);
	   if (whe[0]>0)
		 {
		  if (Data->P[plr].Mission[i].Prog==0) BuildVAB(plr,Data->P[plr].Mission[i].MissionCode,1,0,Data->P[plr].Mission[i].Prog);
		   else BuildVAB(plr,Data->P[plr].Mission[i].MissionCode,1,0,Data->P[plr].Mission[i].Prog-1);
		  for (j=Mission_Capsule; j <= Mission_Probe_DM; j++)
				Data->P[plr].Mission[i].Hard[j] = VAS[whe[0]][j].dex;

		  wgt=0;for (j=0;j<4;j++) wgt+=VAS[whe[0]][j].wt;
		  rck[0]=-1;
		  for (k=0;k<7;k++)
		  if (boos[k]!=-1 && bwgt[k]>=wgt) {
		   if (rck[0]==-1) rck[0]=bdex[k];
			 else if (boos[k]>= boos[rck[0]]) rck[0]=bdex[k];
		  }
		  if (rck[0]==-1) ClrMiss(plr,i-Data->P[plr].Mission[i].part);
		   else
			{
			 if (Data->P[plr].Mission[i].MissionCode==1) rck[0]=0;
          if (Data->P[plr].Mission[i].MissionCode>=7 &&
              Data->P[plr].Mission[i].MissionCode<=13) rck[0]=1;
          if (Data->P[plr].Mission[i].MissionCode==3) rck[0]=1;
          if (Data->P[plr].Mission[i].MissionCode==15) rck[0]=1;
			 Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] = rck[0]+1;
			}
		 }
	   else
		{   // Clear Mission
		 Data->P[plr].Mission[i].MissionCode=0;
		}
	  // joint mission part
	  if (whe[1]>0 && Data->P[plr].Mission[i+1].part==1)
		{
		 if (Data->P[plr].Mission[i].Prog==0) BuildVAB(plr,Data->P[plr].Mission[i].MissionCode,1,1,Data->P[plr].Mission[i].Prog);
		   else BuildVAB(plr,Data->P[plr].Mission[i].MissionCode,1,1,Data->P[plr].Mission[i].Prog-1);
		 for (j=Mission_Capsule ; j <= Mission_Probe_DM; j++)
		 	Data->P[plr].Mission[i+1].Hard[j] = VAS[whe[1]][j].dex;
		 wgt=0;for (j=0;j<4;j++) wgt+=VAS[whe[1]][j].wt;
		 rck[1]=-1;
		 for (k=0;k<7;k++)
		 if (boos[k]!=-1 && bwgt[k]>=wgt) {
		   if (rck[1]==-1) rck[1]=bdex[k];
			 else if (boos[k]>= boos[rck[1]]) rck[1]=bdex[k];
		 }
		 if (rck[1]==-1) rck[1]=Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster]-1;
		 Data->P[plr].Mission[i+1].Hard[Mission_PrimaryBooster]=rck[1]+1;
		}
	 }
 }

// JOINT MISSION KLUGGE MISSION 55 & 56
if (Data->P[plr].Mission[0].MissionCode==55)
 {
  Data->P[plr].Mission[1].Hard[Mission_Capsule]=Data->P[plr].Mission[1].Prog-1;
  Data->P[plr].Mission[0].Hard[Mission_LM]=6; // LM
  Data->P[plr].Mission[0].Hard[Mission_Probe_DM]=4; // DM
  Data->P[plr].Misc[1].Safety=maxx(Data->P[plr].Misc[1].Safety,Data->P[plr].Misc[1].MaxRD);
  Data->P[plr].Mission[1].Hard[Mission_Kicker]=1; // kicker second part
 };

if (Data->P[plr].Mission[0].MissionCode==56)
 {
  Data->P[plr].Mission[1].Hard[Mission_Capsule]=Data->P[plr].Mission[1].Prog-1;
  Data->P[plr].Mission[0].Hard[Mission_LM]=6; // LM
  Data->P[plr].Mission[0].Hard[Mission_Probe_DM]=4; // DM
  Data->P[plr].Misc[1].Safety=maxx(Data->P[plr].Misc[1].Safety,Data->P[plr].Misc[1].MaxRD);
  Data->P[plr].Mission[0].Hard[Mission_Kicker]=1;
	Data->P[plr].Mission[1].Hard[Mission_Kicker]=1;
 };

  // lunar module klugge
  for (i=0;i<3;i++)
   if (Data->P[plr].Mission[i].Hard[Mission_LM]>=5)
     {
      Data->P[plr].Mission[i].Hard[Mission_LM]= Data->P[plr].Manned[5].Safety>=Data->P[plr].Manned[6].Safety ? 5 : 6;
     }
  JR=0;k=0;
  for (l=0;l<3;l++) {
	if (Data->P[plr].Mission[l].Joint==1) JR=1;
	if (Data->P[plr].Mission[l].MissionCode>0 &&
	Data->P[plr].Mission[l].part==0) k++;
	Data->P[plr].Mission[l].Rushing=0;   // Clear Data
  }

  if (k==3) {   // Three non joint missions
	Data->P[plr].Mission[0].Month=2+Data->Season*6;
	Data->P[plr].Mission[1].Month=3+Data->Season*6;
	Data->P[plr].Mission[2].Month=4+Data->Season*6;
  };

  if (k==2 && JR==0) {     // Two non joint missions
	l=3;
	if (Data->P[plr].Mission[0].MissionCode>0) {
	  Data->P[plr].Mission[0].Month=l+Data->Season*6; l+=2; };
	if (Data->P[plr].Mission[1].MissionCode>0) {
	  Data->P[plr].Mission[1].Month=l+Data->Season*6; l+=2; };
	if (Data->P[plr].Mission[2].MissionCode>0)
	  Data->P[plr].Mission[2].Month=l+Data->Season*6;
  };

  if (k==1 && JR==0) {     // Single Mission Non joint
	if (Data->P[plr].Mission[0].MissionCode>0)
	  Data->P[plr].Mission[0].Month=4+Data->Season*6;
	if (Data->P[plr].Mission[1].MissionCode>0)
	  Data->P[plr].Mission[1].Month=4+Data->Season*6;
	if (Data->P[plr].Mission[2].MissionCode>0)
	  Data->P[plr].Mission[2].Month=4+Data->Season*6;
  };

  if (k==2 && JR==1) {  // Two launches, one Joint;
	if (Data->P[plr].Mission[1].part==1) {  // Joint first
	  Data->P[plr].Mission[0].Month=3+Data->Season*6;
	  Data->P[plr].Mission[1].Month=3+Data->Season*6;
      Data->P[plr].Mission[2].Month=5+Data->Season*6;
	};
    if (Data->P[plr].Mission[2].part==1) { // Joint second
      Data->P[plr].Mission[0].Month=3+Data->Season*6;
	  Data->P[plr].Mission[1].Month=5+Data->Season*6;
	  Data->P[plr].Mission[2].Month=5+Data->Season*6;
	};
  };

  if (k==1 && JR==1) { //  Single Joint Launch
    if (Data->P[plr].Mission[1].part==1) {   // found on pad 1+2
	  Data->P[plr].Mission[0].Month=4+Data->Season*6;
      Data->P[plr].Mission[1].Month=4+Data->Season*6;
    } else {   // found on pad 2+3
      Data->P[plr].Mission[1].Month=4+Data->Season*6;
      Data->P[plr].Mission[2].Month=4+Data->Season*6;
    };
  }
  return;
}

/* EOF */

