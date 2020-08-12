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
#include <Buzz_inc.h>
#include <externs.h>
#include <mis.h>
#include <av.h>

  Equipment *MH[2][8];   // Pointer to the hardware
  struct MisAst MA[2][4];  //[2][4]
  struct MisEval Mev[60];  // was *Mev;
  struct mStr Mis;
  struct MXM *AList;
  REPLAY Rep;
  extern char BIG;

  char pCnt,tMen;     // Counter for pAry
  int pAry[15]; // Array for Presige Firsts compelted

  char MANNED[2],CAP[2],LM[2],DOC[2],EVA[2],STEP,FINAL,JOINT,pal2[768],PastBANG,mcc;
  char fEarly; /**< kind of a boolean indicating early missions */
  char hero,DMFake;
  extern char STEPnum;
  extern char pNeg[NUM_PLAYERS][MAX_MISSIONS];
  extern char AI[2];
  ui16 MisStat;
  /* STEP tracks mission step numbers             */
  /* FINAL is the ultimate result of safety check */
  /* JOINT signals the joint mission code         */

void VerifyData(void)
{
 int i,j;
 for (j=0;j<NUM_PLAYERS;j++)
   for (i=0;i<7;i++) {
      strncpy(Data->P[j].Probe[i].ID,Data->P[j].Probe[i].IDX,2);
      strncpy(Data->P[j].Rocket[i].ID,Data->P[j].Rocket[i].IDX,2);
      strncpy(Data->P[j].Misc[i].ID,Data->P[j].Misc[i].IDX,2);
      strncpy(Data->P[j].Manned[i].ID,Data->P[j].Manned[i].IDX,2);
      }
   return;
}

void DrawControl(char plr)
{
  FILE *fin;
  i32 len;
  fin=sOpen("CONTROL.IMG","rb",0);
  fread(pal,768,1,fin);
  fread(&len,4,1,fin);
	Swap32bit(len);
  if (plr==1) {
    fseek(fin,len,SEEK_CUR);
    fread(pal,768,1,fin);
    fread(&len,4,1,fin);
		Swap32bit(len);
  }
  fread(vhptr.vptr,len,1,fin);fclose(fin);
  PCX_D((char *)vhptr.vptr,(char *)screen,(unsigned) len);
  av_need_update_xy(0, 0, MAX_X, MAX_Y);

}

void SetW(char ch)
{
  int i;
  i=0;
  while (Mev[i].Name[0]!='W') i++;

  Mev[i].Name[2]=ch;
}

int Launch(char plr,char mis)
{
   int i,j,t,k,mcode,avg,temp=0;
   char total;
   STEP=FINAL=JOINT=PastBANG=0;
   memset(pAry,0x00,sizeof pAry); pCnt=0;  // reset values
   MisStat=tMen=0x00;  // clear mission status flags
   
   remove_savedat("REPLAY.TMP");  // make sure replay buffer isn't there

   if (Data->P[plr].Mission[mis].part==1) return 0;

   memset(buffer,0x00,BUFFER_SIZE);  // Clear Buffer
   memset(MH,0x00,sizeof MH);
   memset(Mev,0x00,sizeof Mev);

   if (Data->P[plr].Mission[mis].MissionCode==2) Data->P[plr].Mission[mis].Duration=1;

   MANNED[0]=Data->P[plr].Mission[mis].Men;
   MANNED[1]=Data->P[plr].Mission[mis].Joint ? Data->P[plr].Mission[mis+1].Men : 0;

   JOINT = Data->P[plr].Mission[mis].Joint;

   temp=CheckCrewOK(plr,mis);
   if (temp==1) //found mission no crews
   {
    ClrMiss(plr,mis-Data->P[plr].Mission[mis].part); 
   }
  
   if (!AI[plr] && Data->P[plr].Mission[mis].MissionCode!=0) MisAnn(plr,mis);

   if (Data->P[plr].Mission[mis].MissionCode==0) return -20;
   MissionSetup(plr,mis);

   // ****************************************
   // Do all Manned Associated Stuff

   memset(MA,0x00,sizeof MA);

   for (i=0;i<(1+JOINT);i++) {
      // Decide who to use for each pad
	      Data->P[plr].Mission[mis+i].Crew = ( Data->P[plr].Mission[mis+i].PCrew > 0 ) ?
		     Data->P[plr].Mission[mis+i].PCrew :  Data->P[plr].Mission[mis+i].BCrew ;
	   for (j=0;j<MANNED[i];j++) {
        t=Data->P[plr].Mission[mis+i].Prog;
        k=Data->P[plr].Mission[mis+i].Crew-1;
		   total=Data->P[plr].Crew[t][k][j]-1;
        MA[i][j].A=&Data->P[plr].Pool[total];
        MA[i][j].loc=i;
         } 
      }

   // Set Mission Status Flag
   if (MA[0][0].A!=NULL && MA[1][0].A==NULL) MisStat=S_MAN;
   else if (MA[0][0].A==NULL && MA[1][0].A!=NULL) MisStat=JT1_UNMAN|JT2_MAN;
   else if (MA[0][0].A!=NULL && MA[1][0].A!=NULL) MisStat=JT1_MAN|JT2_MAN;

   // 0 here is for unused
    
   CAP[0]=LM[0]=EVA[0]=DOC[0]=CAP[1]=LM[1]=EVA[1]=DOC[1]=-1;

   for (i=0;i<(1+JOINT);i++) {
      switch(MANNED[i]) {  
	      case 0: CAP[i]=LM[i]=EVA[i]=DOC[i]=-1; break;
	      case 1: CAP[i]=LM[i]=EVA[i]=0;DOC[i]=-1; break;
	      case 2: CAP[i]=DOC[i]=0;LM[i]=EVA[i]=1; break;  // Last in LM
	      case 3:
                // LM is always on first mission part!!! :: makes things easier
                if (MH[0][2] && MH[0][2]->ID[1]==0x35) {CAP[i]=0;LM[i]=1;DOC[i]=EVA[i]=2;}
		           else {CAP[i]=0;LM[i]=EVA[i]=1;DOC[i]=2;};

              break;
	      case 4: CAP[i]=0; LM[i]=1; EVA[i]=2; DOC[i]=3; break;
	      default: break;
         };
  };
  // END MEN SETUP ****************************

  // Do actual Missions

  mcc=mcode=Data->P[plr].Mission[mis].MissionCode;

  // Fixup for Mercury Duration C stuff
  if (Data->P[plr].Mission[mis].Hard[Mission_Capsule]==0) 
		Data->P[plr].Mission[mis].Duration = minn(2,Data->P[plr].Mission[mis].Duration);

  MissionCodes(plr,mcode,mis);

     /////////////////////////////////////////////////
     // Fix for BARIS CD-ROM Planetary Steps (Step W)
     // E=moon ; M= mars ;S = saturn; V=venus; J= jupiter  R= Mercury
     // Must be at .Name[2]
     //
     // Search for Step 'W' on planetary steps
     //

  if (mcode==7) SetW('E');
  else if (mcode==9) SetW('V');
  else if (mcode==10) SetW('M');
  else if (mcode==11) SetW('R');
  else if (mcode==12) SetW('J');
  else if (mcode==13) SetW('S');

//  if (mcode>15 && NOCOPRO && !AI[plr]) MisPrt();

  // Exit missions early
  /** \todo The "early" missions should be defined in a file */
  /* 1 = Orbital sattelite
   * 7 = lunar flyby
   * 8 = lunar probe landing
   * 9 = venus flyby
   * 11 = mercury flyby
   */
  fEarly=(!Mis.Days && !(mcode==1 || mcode==7 || mcode==8 || mcode==9 || mcode==11));

  STEPnum=STEP;
  if (NOFAIL==1) MisPrt();

  MisDur(plr,Data->P[plr].Mission[mis].Duration);
  if (MANNED[0]>0 || MANNED[1]>0 || mcode==1 || mcode==7 || mcode==8)
    MisSkip(plr,Find_MaxGoal());

  MisRush(plr, Data->P[plr].Mission[mis].Rushing);
  STEPnum=0;

  if (!AI[plr] && BIG==0) {
      DrawControl(plr);
      FadeIn(2,pal,10,0,0);
      }
   else 
		if (BIG==1) gxClearDisplay(0, 0);

#define fpf(a,b) fprintf(a,#b ": %d\n",b)

	memset(&Rep, 0x00, sizeof Rep);	   // Clear Replay Data Struct

	/* whatever this mcode means... */
	if (!AI[plr] && mcode >= 53 )
	{
		avg = temp = 0;

		for (i = 0; Mev[i].loc != 0x7f; ++i)
		{
			/* Bugfix -> We need to skip cases when Mev[i].E is NULL */
			/* Same solution as used in mis_m.c (MisCheck):207 */
			if (!Mev[i].E)
				continue;

			avg += Mev[i].E->MisSaf + Mev[i].asf;
			temp += 1;
		};

		if (temp)
			avg /= temp;
		else
			avg = 0;

		if (avg >= 3 && avg <= 105)
			SafetyRecords(plr, avg);
	}

//   if (!AI[plr]) {PreLoadMusic(M_ELEPHANT); PlayMusic(0);}

   if ((Data->Def.Lev1 == 0 && Data->Def.Lev2 ==2) || (Data->Def.Lev2 == 0 && Data->Def.Lev1==2))
     xMODE |= PUSSY; //set easy flag

   if (AI[plr]) xMODE &= ~xMODE_PUSSY;  // map out computer from really easy level


   MisCheck(plr,mis);  // Mission Resolution

   xMODE &= ~xMODE_PUSSY;

//   if (!AI[plr]) KillMusic();

   if (Mis.Days==0) total=U_AllotPrest(plr,mis);   // Unmanned Prestige
   else total=AllotPrest(plr,mis);                    // Manned Prestige

   total=total-(pNeg[plr][mis]*3);

   Data->P[plr].Prestige+=total;
   MissionSetDown(plr,mis);
   MissionPast(plr,mis,total);
   // Update the Astro's

	for (i = 0; i < 1 + JOINT; i++)
	{
		/* XXX: was MANNED[i]+1, but why? */
		for (j = 0; j < MANNED[i]; j++)
		{
			if (MA[i][j].A)
			{
				if (FINAL >= 100)
					MA[i][j].A->Mis = 1;	// Successful
				else if (Data->P[plr].Other & 4)
					MA[i][j].A->Mis = 2;	// Failure
			}
		}
	}
   BIG=0;
   return total;
}


void MissionPast(char plr,char pad,int prest)
{
  int loc,i,j,loop,mc;
  FILE *fout,*fin;
  unsigned int num;
  long size;
  char dys[7]={0,2,5,7,12,16,20};

  loc=Data->P[plr].PastMis;
  mc=Data->P[plr].Mission[pad].MissionCode;
  memset(&Data->P[plr].History[loc],-1,sizeof (struct PastInfo));
  strcpy(&Data->P[plr].History[loc].MissionName[0][0],Data->P[plr].Mission[pad].Name);
  Data->P[plr].History[loc].Patch[0]=Data->P[plr].Mission[pad].Patch;
  if(Data->P[plr].Mission[pad].Joint==1) {
      strcpy(&Data->P[plr].History[loc].MissionName[1][0],Data->P[plr].Mission[pad+1].Name);
      Data->P[plr].History[loc].Patch[1]=Data->P[plr].Mission[pad+1].Patch;
  }

  // Flag for if mission is done
  Data->P[plr].History[loc].Event=Data->P[plr].History[loc].Saf=0; 
  Data->P[plr].History[loc].Event=(mc==10)?2:((mc==12)?7:((mc==13)?7:0));
  if ((mc==10 || mc==12 || mc==13) && prest!=0) Data->P[plr].History[loc].Event=0;
  if (MH[0][3])
	  Data->P[plr].History[loc].Saf=MH[0][3]->MisSaf;
  if (!(mc==10 || mc==12 || mc==13)) Data->P[plr].History[loc].Event=Data->P[plr].History[loc].Saf=0; 

  Data->P[plr].History[loc].MissionCode=Data->P[plr].Mission[pad].MissionCode;
  Data->P[plr].History[loc].MissionYear=Data->Year;
  Data->P[plr].History[loc].Month=Data->P[plr].Mission[pad].Month;
  Data->P[plr].History[loc].Prestige=maxx(prest,-10);
  Data->P[plr].History[loc].Duration=Data->P[plr].Mission[pad].Duration;
  for (loop=0;loop<(Data->P[plr].Mission[pad].Joint+1);loop++) {
      i=Data->P[plr].Mission[pad+loop].Prog;
      j=Data->P[plr].Mission[pad+loop].Crew-1;
      if (Data->P[plr].Mission[pad+loop].Men>0) {
          Data->P[plr].History[loc].Man[loop][0]=Data->P[plr].Crew[i][j][0]-1;
          Data->P[plr].History[loc].Man[loop][1]=Data->P[plr].Crew[i][j][1]-1;
          Data->P[plr].History[loc].Man[loop][2]=Data->P[plr].Crew[i][j][2]-1;
          Data->P[plr].History[loc].Man[loop][3]=Data->P[plr].Crew[i][j][3]-1;
      } else {
          Data->P[plr].History[loc].Man[loop][0]=Data->P[plr].History[loc].Man[loop][1]=
              Data->P[plr].History[loc].Man[loop][2]=Data->P[plr].History[loc].Man[loop][3]=-1;
      };
      if(Data->P[plr].Mission[pad+loop].Men>0) {
          for (i=0;i<4;i++) {
              j=Data->P[plr].Crew[Data->P[plr].Mission[pad+loop].Prog][Data->P[plr].Mission[pad+loop].Crew-1][i]-1;
              if(j>=0) Data->P[plr].Pool[j].MissionNum[Data->P[plr].Pool[j].Missions]=loc;
              if(j>=0) {
                 Data->P[plr].Pool[j].Missions++;
                 Data->P[plr].Pool[j].Prestige+=prest;
                 Data->P[plr].Pool[j].Days+=dys[Data->P[plr].Mission[pad+loop].Duration];
                 if (hero&0x01) Data->P[plr].Pool[j].Hero=1;
                 else if (hero&0x02 && j==EVA[loop]) Data->P[plr].Pool[j].Hero=1;
              }   
          }
      }
  };

  for (i=Mission_Capsule; i <= Mission_PrimaryBooster; i++) {
      Data->P[plr].History[loc].Hard[0][i]=Data->P[plr].Mission[pad].Hard[i];
      if (Data->P[plr].Mission[pad].Joint==1) {
          Data->P[plr].History[loc].Hard[1][i]=Data->P[plr].Mission[pad+1].Hard[i];
      }
  }
  Data->P[plr].History[loc].result=FINAL;
  Data->P[plr].History[loc].spResult=MaxFail();

  if (Data->P[plr].History[loc].spResult>=4000 && MANNED[0]>0 && MANNED[1]>0) {
    // first -all killed
     if (MaxFailPad(0)>=4000 && MaxFailPad(1)<4000) {
        Data->P[plr].History[loc].spResult=4197;
     }
    // second -all killed
     else if (MaxFailPad(0)<4000 && MaxFailPad(1)>=4000) {
        Data->P[plr].History[loc].spResult=4198;
     }

    // first and second all killed
     else if (MaxFailPad(0)>=4000 && MaxFailPad(1)>=4000) {
        Data->P[plr].History[loc].spResult=4199;
     }

     if (MANNED[0]>0 && MANNED[1]>0 && tMen==(MANNED[0]+MANNED[1]))
       Data->P[plr].History[loc].spResult=4199;

  }
  
  Rep.Qty=0;
  fout=sOpen("REPLAY.TMP","rt",1);
  while (!feof(fout)) {
     if (fscanf(fout,"%u\n",&num) != 1)
	     break;
     Rep.Off[Rep.Qty++]=num;
  };
  fclose(fout);
  remove_savedat("REPLAY.TMP");
  if (Rep.Qty==1 && Data->P[plr].History[loc].spResult<3000) Data->P[plr].History[loc].spResult=1999;

  fin=sOpen("REPLAY.DAT","r+b",1);
  size=(plr*100)+Data->P[plr].PastMis;
  fseek(fin,size * (sizeof Rep),SEEK_SET);
  fwrite(&Rep,sizeof Rep,1,fin);
  fclose(fin);

  Data->P[plr].PastMis++;
  return;
}


int MaxFailPad(char which)
{
   int i=0,t=0;
   while (i != 0x7f) {
     if (Mev[i].pad==which) {
       if (Mev[i].StepInfo==0) Mev[i].StepInfo=1003;
       t=maxx(Mev[i].StepInfo,t);
     }
     i=Mev[i].trace;
   }
   return t;
}

/* vim: set noet ts=4 sw=4 tw=77: */
