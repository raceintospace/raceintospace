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
/** \file prest.c Handles all the prestige related code.
 */

#include <Buzz_inc.h>
#include <externs.h>

extern struct mStr Mis;
extern struct MisEval Mev[60];
extern struct Prest_Upd MP[3];
extern struct MisAst MA[2][4];
extern char STEPnum,AI[2],MANNED[2];
//extern char tame[29][40];
extern char hero,death;
extern char Option,MAIL;

char tYr,tMo,tIDX,bIDX;
FILE *ft;

void Set_Dock(char plr, char total)
{
  int i;
  for (i=0;i<total;i++) {
   if (Mev[i].loc==8 && Mev[i].StepInfo==1)
     {Data->Prestige[24].Goal[plr]++;return;};
   }
  return;
}

void Set_LM(char plr, char total)
{
  int i;
  for (i=0;i<total;i++) {
   if (Mev[i].loc==26 && Mev[i].StepInfo==1) Data->P[plr].LMpts++;
   }
  return;
}

int Check_Photo(void)
{
  int i;
  for (i=0;i<STEPnum;i++) {
   if (Mev[i].loc==20 && Mev[i].StepInfo==1) return 1;
   if (Mev[i].loc==22 && i==4 && Mev[i].StepInfo==1) return 1;
   if (Mev[i].loc==20 && Mev[i].StepInfo>1) return 2;
   }
  return 0;
}

int Check_Lab(void)
{
  int i;
  for (i=0;i<STEPnum;i++) {
   if (Mev[i].loc==28 && Mev[i].StepInfo==1) return 1;
   }
  return 0;
}

int Check_Dock(int limit)
{
  int i;
  for (i=0;i<STEPnum;i++) {
   if (Mev[i].loc==8 && Mev[i].StepInfo == 0) return 0;
   if (Mev[i].loc==8 && Mev[i].StepInfo <= limit) return 2;
   if (Mev[i].loc==8 && Mev[i].StepInfo > limit) return 1;
   }
  return 0;
}

int Check_EVA(int limit)
{
  int i;
  for (i=0;i<STEPnum;i++) {
   if (Mev[i].loc==7 && Mev[i].StepInfo == 0) return 0;
   if (Mev[i].loc==7 && Mev[i].StepInfo <= limit) return 2;
   if (Mev[i].loc==7 && Mev[i].StepInfo > limit) return 1;
   }
  return 0;
}

/**
 * Map prestige category list (mStr::PCat) to milestones.
 */
int
PrestMap(int val)
{
	switch (val) {
		case ORBSAT:
			return MILE_OSAT;
		case MANSPACE:
			return MILE_MAN;
		case EORBIT:
			return MILE_EORBIT;
		case LUNFLY:
			return MILE_LFLY;
		case PROBELAND:
			return MILE_LPL;
		case LPASS:
			return MILE_LPASS;
		case LORBIT:
			return MILE_LORBIT;
		case LLAND:
			return MILE_LLAND;
		default:
			return -1;
	}
}

/**
 * Calculate mission penalty due to missed milestones and duration.
 *
 * \param plr current player
 * \return penalty
 *
 * \note Call only when Mis is valid
 */
char
PrestMin(char plr)
{
	int i, j, Neg = 0;

	Neg = 0;
	j = 0;
	if (Mis.Index == 0)
		return 0;
	for (i = 0; i < 5; i++)
		j = maxx(j, PrestMap(Mis.PCat[i]));

	/* walk all milestones lower than maximum required for mission */
	for (i = 0; i <= j; ++i)
	{
		/* if milestone not met, then add penalty */
		if (Data->Mile[plr][i] == 0)
			Neg += 3;
	}

	Neg = Neg + (plr ? Data->Def.Lev2 : Data->Def.Lev1) - 2;
	// Neg -= (2 - ((plr == 0) ? Data->Def.Lev1 : Data->Def.Lev2));
	Neg = maxx(Neg, 0);

	/* Index 2 = Manned suborbital
	 * Index 4 = Manned orbital
	 * Index 6 = Manned orbital EVA
	 */
	if (Mis.Index != 2
			&& Mis.Index != 4
			&& Mis.Index != 6
			&& (Mis.Days - Data->P[plr].DurLevel) > 1)   // Raised this from "> 0" to disable broken Duration penalty system -Leon
	{
		Neg += 5 * (Mis.Days - Data->P[plr].DurLevel);
	}


	return Neg;
}


/** Returns the amount of prestige added
 *
 * \note Assumes that the Mis Structure is Loaded
 */
int PrestCheck(char plr)
{
 int i,total=0; char prg,tm;

 prg=Mis.mEq;

  for (i=0;i<5;i++) {   // Sum all first/second Nation Bonuses
    tm=Mis.PCat[i];
    if (tm!=-1 && Data->Prestige[tm].Goal[plr]==0) { // First Mission Bonus
      if (Data->Prestige[tm].Goal[other(plr)]==0 && tm<27)
	total+=Data->Prestige[tm].Add[0];     // your first
      else total+=Data->Prestige[tm].Add[1];  // your second
    }
  }

  if (Mis.Doc==1 && Data->Prestige[24].Goal[plr]==0) {
    if (Data->Prestige[24].Goal[other(plr)]==0)
      total+=Data->Prestige[24].Add[0];      // your first
    else total+=Data->Prestige[24].Add[1];   // your second
  }

  if (Mis.EVA==1 && Data->Prestige[26].Goal[plr]==0) {
    if (Data->Prestige[26].Goal[other(plr)]==0)
      total+=Data->Prestige[26].Add[0];     // your first
    else total+=Data->Prestige[26].Add[1];  // your second
  }

  if (Mis.Days>1 && Data->P[plr].DurLevel<Mis.Days) {
    if (Mis.Days==6 && Data->Prestige[14-Mis.Days].Goal[plr]==0)
      total+=Data->Prestige[14-Mis.Days].Add[0];
    else if (Mis.Days==5 && Data->Prestige[14-Mis.Days].Goal[plr]==0)
      total+=Data->Prestige[14-Mis.Days].Add[0];
    else if (Mis.Days==4 && Data->Prestige[14-Mis.Days].Goal[plr]==0)
      total+=Data->Prestige[14-Mis.Days].Add[0];
    else if (Mis.Days==3 && Data->Prestige[14-Mis.Days].Goal[plr]==0)
      total+=Data->Prestige[14-Mis.Days].Add[0];
    else if (Mis.Days==2 && Data->Prestige[14-Mis.Days].Goal[plr]==0)
      total+=Data->Prestige[14-Mis.Days].Add[0];
  }

  // Hardware Checks
  if (Mis.Days>1 && Data->Prestige[12+prg].Goal[plr]==0) {
    if (Data->Prestige[12+prg].Goal[other(plr)]==0)
      total+=Data->Prestige[12+prg].Add[0];    // your first
    else total+=Data->Prestige[12+prg].Add[1]; // your second
  }

  if (total!=0) return total;

  // Other mission bonus

   // Sum all additional Mission Bonuses
  for (i=0;i<5;i++)
    if (Mis.PCat[i]!=-1)
      total+=Data->Prestige[Mis.PCat[i]].Add[2];

  if (Mis.Doc==1 && Data->Prestige[24].Goal[plr]==0)
    total+=Data->Prestige[24].Add[2];

  if (Mis.EVA==1 && Data->Prestige[26].Goal[plr]==0)
    total+=Data->Prestige[26].Add[2];

  return total;
}


#ifdef DEAD_CODE
/** ???
 *
 * \deprecated
 *
 * \note Requires MEV to be packed
 * Only checks goal steps ::
 * returns GOAL value
 * Does not check if entire mission is a success
 *
 */
char Was_Goal(char total,char which)
{
   char i,val,x=10;

   switch(which) {
      case MANSPACE:val=4; break; // RECOVERY
      case EORBIT:val=1;break;    // OIB
      case LPASS:val=14;break;    // LIRA
      case LORBIT:val=12;break;   // LOA
      case LLAND:val=18;break;    // LLAND
      case ORBSAT:val=2;break;    // POWERON
      case LUNFLY: case ME_FB: case VE_FB: case MA_FB: case JU_FB: case SA_FB:
         val=22;break;                   // P.ACT
      case PROBELAND: val=18; break;     // LLAND
      case OLAB:val=28;break;            // Jt Dur
      case DOCK:val=8;break;             // Docking
      case EWALK:val=7;break;     // E EVA
      case LWALK:val=15;break;    // L EVA
      case -1:
			default:
					return -1;
      }

   i=0;

   while ( i != 0x7f) {                               // not branched
      if (Mev[i].loc==val && Mev[i].StepInfo<3000 ) x=minn(1,x);
      if ((which==ORBSAT || which==LUNFLY || which==PROBELAND) && Mev[i].StepInfo>500) x=-1;
      if (Mev[i].StepInfo>=3000) x=-1;  // Manned will only be affected
      i=Mev[i].trace;
      };

   if (x>0 && x<10) return which;

  switch(which) {
    case EORBIT:return Was_Goal(total,MANSPACE);
    case LPASS:return Was_Goal(total,EORBIT);
    case LORBIT:return Was_Goal(total,LPASS);
    case LLAND:return Was_Goal(total,LORBIT);
    case EWALK:return Was_Goal(total,LWALK);
    default: return -1;  // no goal step achieved
  }
}
#endif /* DEAD_CODE */

char HeroCheck(int which)
{
   switch(which) {
      case MANSPACE: return 0x01;// RECOVERY
      case EORBIT: return 0x01;  // OIB
      case LPASS:return 0x01;    // LIRA
      case LLAND:return 0x01;    // LLAND
      case EWALK:return 0x02;    // E EVA
      case LWALK:return 0x02;    // L EVA
      default: return 0;
      }
}

/** Sets Goal Values and Sums into Data Structure
 * control should be always called with a value of 0
 * Successful Steps Only
 * Requires MEV to be packed
 */
char Set_Goal(char plr,char which,char control)
{
  char sum,pd,qt;

  sum=0;
  if (control!=3) {
      if (MaxFail()>1999) return -1;
  }
  else control=0;

    if (control==1 || which>=0) {  // Means successful to this part

      if (Data->Prestige[which].Place==-1) {
        switch(which) {  // flag milestones
         case ORBSAT: isMile(plr,MILE_OSAT)=1;break;
         case MANSPACE: isMile(plr,MILE_MAN)=1;break;
         case EORBIT: isMile(plr,MILE_EORBIT)=1;break;
         case LUNFLY: isMile(plr,MILE_LFLY)=1;break;
         case PROBELAND:  isMile(plr,MILE_LPL)=1;break;
         case LPASS: isMile(plr,MILE_LPASS)=1;break;
         case LORBIT: isMile(plr,MILE_LORBIT)=1;break;
         case LLAND: isMile(plr,MILE_LLAND)=1;break;
        }

	     if (control==0) {
	       Data->P[plr].Other|=4;  // for astros


        if (Option==1)
         {
          pd=Mev[0].pad;
          qt=MP[pd].Qty;
          MP[pd].HInd=Data->P[1].PastMis;
          MP[pd].Poss[qt]=which;
          MP[pd].PossVal[qt]=0;
          MP[pd].Mnth=tMo;
          ++MP[pd].Qty;
         }
        else if (MAIL==0) {
          pd=Mev[0].pad;
          qt=Data->P[0].Udp[pd].Qty;
          Data->P[0].Udp[pd].HInd=Data->P[0].PastMis;
          Data->P[0].Udp[pd].Poss[qt]=which;
          Data->P[0].Udp[pd].PossVal[qt]=0;
          Data->P[0].Udp[pd].Mnth=tMo;
          ++Data->P[0].Udp[pd].Qty;
         }
        else
         {
          Data->Prestige[which].Place=plr;
          Data->Prestige[which].Indec=Data->P[plr].PastMis;
          Data->Prestige[which].Year=tYr;
          Data->Prestige[which].Month=tMo;
	       Data->Prestige[which].Goal[plr]++;  // increment count
	       Data->Prestige[which].Points[plr]+=Data->Prestige[which].Add[0];
	       sum+=Data->Prestige[which].Add[0];
         }

          hero|=HeroCheck(which);
	     } else if (control==1) {
          switch(which) {
            case DUR_B:
            case DUR_C:
            case DUR_D:
            case DUR_E:
            case DUR_F:
             if (Option==1)
              {
               pd=Mev[0].pad;
               qt=MP[pd].Qty;
               MP[pd].HInd=Data->P[1].PastMis;
               MP[pd].Poss[qt]=which;
               MP[pd].PossVal[qt]=0;
               MP[pd].Mnth=tMo;
               ++MP[pd].Qty;
              }
             else if (MAIL==0) {
               pd=Mev[0].pad;
               qt=Data->P[0].Udp[pd].Qty;
               Data->P[0].Udp[pd].HInd=Data->P[0].PastMis;
               Data->P[0].Udp[pd].Poss[qt]=which;
               Data->P[0].Udp[pd].PossVal[qt]=0;
               Data->P[0].Udp[pd].Mnth=tMo;
               ++Data->P[0].Udp[pd].Qty;
              }
             else {
	           Data->Prestige[which].Place=plr;
              Data->Prestige[which].Indec=Data->P[plr].PastMis;
              Data->Prestige[which].Year=tYr;
              Data->Prestige[which].Month=tMo;
             }
            default: break;
          };
        }
     }
      else if (Data->Prestige[which].mPlace==-1 && Data->Prestige[which].Place!=plr){
	     Data->P[plr].Other|=4;  // for astros


        Data->Prestige[which].mPlace=plr;

        switch(which) {  // flag milestones
         case ORBSAT: isMile(plr,MILE_OSAT)=1;break;
         case MANSPACE: isMile(plr,MILE_MAN)=1;break;
         case EORBIT: isMile(plr,MILE_EORBIT)=1;break;
         case LUNFLY: isMile(plr,MILE_LFLY)=1;break;
         case PROBELAND: isMile(plr,MILE_LPL)=1;break;
         case LPASS: isMile(plr,MILE_LPASS)=1;break;
         case LORBIT: isMile(plr,MILE_LORBIT)=1;break;
         case LLAND: isMile(plr,MILE_LLAND)=1;break;
        }

	     if (control==0) {
        if (Option==1) {
          pd=Mev[0].pad;
          qt=MP[pd].Qty;
          MP[pd].HInd=Data->P[1].PastMis;
          MP[pd].Poss[qt]=which;
          MP[pd].PossVal[qt]=1;
          MP[pd].Mnth=tMo;
          ++MP[pd].Qty;
         }
         else {
	       Data->Prestige[which].Goal[plr]++;  // increment count
	       sum+=Data->Prestige[which].Add[1];
	       Data->Prestige[which].Points[plr]+=Data->Prestige[which].Add[1];
        }
         hero|=HeroCheck(which);
	       }
        }
      else if(sum<3) {  // Other
	     if (control==0) {
         if (Option==1) {
          pd=Mev[0].pad;
          qt=MP[pd].Qty;
          MP[pd].HInd=Data->P[1].PastMis;
          MP[pd].Poss[qt]=which;
          MP[pd].PossVal[qt]=2;
          MP[pd].Mnth=tMo;
          ++MP[pd].Qty;
         }
         else {
	       Data->Prestige[which].Goal[plr]++;  // increment count
	       sum+=Data->Prestige[which].Add[2];
	       Data->Prestige[which].Points[plr]+=Data->Prestige[which].Add[2];
        }
	       }
        }
     }

  //----------------------------------------
  //Specs: Lunar Landing klugge (Duration D)
  //----------------------------------------
  if (which==LLAND || Data->Prestige[22].Place==plr)
   Data->P[plr].History[Data->P[plr].PastMis].Duration=4;

  switch(which) {
    case ORBSAT: return(sum);
    case MANSPACE:return(sum);
    case EORBIT:return(sum+Set_Goal(plr,ORBSAT,1));
    case LUNFLY:return(sum+Set_Goal(plr,EORBIT,1));
    case PROBELAND:return(sum+Set_Goal(plr,LUNFLY,1));
    case LPASS:return(sum+Set_Goal(plr,PROBELAND,1));
    case LORBIT:return(sum+Set_Goal(plr,LPASS,1));
    case LLAND:return(sum+Set_Goal(plr,LORBIT,1));
    case DUR_A: return(sum);
    case DUR_B: return(sum);
    case DUR_C:return(sum+Set_Goal(plr,DUR_B,1));
    case DUR_D:return(sum+Set_Goal(plr,DUR_C,1));
    case DUR_E:return(sum+Set_Goal(plr,DUR_D,1));
    case DUR_F:return(sum+Set_Goal(plr,DUR_E,1));
    case CAP1: return(sum);
    case CAP2:return(sum+Set_Goal(plr,CAP1,1));
    case CAP3:return(sum+Set_Goal(plr,CAP2,1));
    case CAPMS:return(sum+Set_Goal(plr,CAP3,1));
    case CAP4:return(sum+Set_Goal(plr,CAPMS,1));
    case ME_FB: case VE_FB: case MA_FB: case JU_FB: case SA_FB:
    case OLAB: case EWALK: case DOCK: case WOMAN:return (sum);
    default: return 0;
  }
}

/** Only sets negative for highest failed goal step
 *
 * checks if entire mission was a failure
 */
char Did_Fail(void)
{
   char i,bra;
   unsigned int fail;

   fail=0;bra=0;i=0;
   while ( i != 0x7f) {
      if (Mev[i].StepInfo>=5000) fail+=1000;
      if (Mev[i].StepInfo>=4000 && Mev[i].StepInfo<=4999) fail+=1000;
      if (Mev[i].StepInfo>=3000 && Mev[i].StepInfo<=3999) fail+=100;
      if (Mev[i].StepInfo>=2000 && Mev[i].StepInfo<=2999) fail+=10;
      if (Mev[i].StepInfo>=1000 && Mev[i].StepInfo<=1999) fail+=1;
      if (Mev[i].trace!=(i+1)) bra++;
      i=Mev[i].trace;
      }

   if (fail<90 && bra==0) return 1;
   else return -1;
}

int MaxFail(void)
{
   int i=0,t=0,bra=0,count=0;

   while (i != 0x7f && count<54) {
      if (Mev[i].StepInfo==0) Mev[i].StepInfo=1003;
      t=maxx(Mev[i].StepInfo,t);
      if (Mev[i].trace!=(i+1)) bra++;
      i=Mev[i].trace;
      ++count;
    }
 if (count>=54) return 1; else return t;
}

#define PSTS(a)  (PVal[a]==1 || PVal[a]==2)
#define NSTS(a)  (PVal[a]==4)
#define SSTS(a)  (PVal[a]==3)
#define STSp(a)  (PVal[a]==1 || PVal[a]==2)
#define STSn(a)  (PVal[a]==4)
#define IsGoal(a) ( (18)==18 || (a)==19 || (a)==20 || (a)==22 || (a)==27) )


char PosGoal(char *PVal)
{
   if (PSTS(22)) return 22;
   else if (PSTS(20)) return 20;
   else if (PSTS(19)) return 19;
   else if (PSTS(18)) return 18;
   else if (PSTS(27)) return 27;
   else return -1;
}

char NegGoal(char *PVal)
{
   if (NSTS(22)) return 22;
   else if (NSTS(20)) return 20;
   else if (NSTS(19)) return 19;
   else if (NSTS(18)) return 18;
   else if (NSTS(27)) return 27;
   else return -1;
}

char SupGoal(char *PVal)
{
   if (SSTS(22)) return 22;
   else if (SSTS(20)) return 20;
   else if (SSTS(19)) return 19;
   else if (SSTS(18)) return 18;
   else if (SSTS(27)) return 27;
   else return -1;
}

int PrestNeg(char plr,int i)
{
  int negs=0;
  char pd,qt;

  if (Option==1)
   {
    pd=Mev[0].pad;
    qt=MP[pd].Qty;
    MP[pd].HInd=Data->P[1].PastMis;
    MP[pd].Poss[qt]=i;
    MP[pd].PossVal[qt]=3;
    ++MP[pd].Qty;
   }
  else {
    negs=Data->Prestige[i].Add[3];
    Data->Prestige[i].Goal[plr]++;
	  Data->Prestige[i].Points[plr]+=Data->Prestige[i].Add[3];
  }

  return negs;
}


int AllotPrest(char plr,char mis)
{
   int i,total,other,negs,mcode,mike,P_Goal,N_Goal,S_Goal,ival,cval;
   extern struct mStr Mis;
   char PVal[28];

   hero=0;
   tMo=Data->P[plr].Mission[mis].Month;
   tYr=Data->Year;
   tIDX=bIDX=0;
   memset(PVal,0x00,sizeof PVal);

   // SETUP INFO
   mcode=Data->P[plr].Mission[mis].MissionCode;

   GetMisType(mcode);

   other=MaxFail();
   total=negs=0;

   for (i=0;i<STEPnum;i++) {
      if (Mev[i].PComp==5 && Mev[i].StepInfo==0) {
         Mev[i].PComp=0;Mev[i].Prest=-100;
         }
      if ((MANNED[0]+MANNED[1])>0) {
         if (other>=3000) Mev[i].PComp=4;
         else if (Mev[i].Prest>=-28 && Mev[i].StepInfo>2999) Mev[i].PComp=4;
         }
      }

   // FEMALE 'NAUTS
     PVal[25]= (MA[0][0].A!=NULL && MA[0][0].A->Sex)
        || (MA[0][1].A!=NULL && MA[0][1].A->Sex)
        || (MA[0][2].A!=NULL && MA[0][2].A->Sex)
        || (MA[0][3].A!=NULL && MA[0][3].A->Sex)
        || (MA[1][0].A!=NULL && MA[1][0].A->Sex)
        || (MA[1][1].A!=NULL && MA[1][1].A->Sex)
        || (MA[1][2].A!=NULL && MA[1][2].A->Sex)
        || (MA[1][3].A!=NULL && MA[1][3].A->Sex);

   for (i=0;i<STEPnum;i++) {
      ival=abs(Mev[i].Prest);
      cval=Mev[i].PComp;

			// ival of 100 seems to mean "don't record this in PVal[]"
			// Regardless of intention, it's out of bounds, so don't access or overwrite it
			if (ival != 100) {
				if (Mev[i].StepInfo==0 && PVal[ival]==0)
					cval=4;

				if (PVal[ival]!=4)
					PVal[ival]=cval;
			}
	 }

   // EVA FIX FOR ALTERNATE STEPS LATER IN MISSION
   if (Mis.EVA==1 && (PVal[26]==0 || PVal[26]==5)) PVal[26]=4;
   else if (Mis.EVA==0 && PVal[26]==5) PVal[26]=0;

   // DOCKING FIX FOR ALTERNATE STEPS LATER IN SESSION
   if (Mis.Doc==1 && (PVal[24]==0 || PVal[24]==5)) PVal[27]=4;
   else if (Mis.EVA==0 && PVal[24]==5) PVal[24]=0;

   // CLEAR TOTAL VALUE
   total=0; negs=0;

   // PHOTO RECON
   if (PVal[19]>0 && PVal[19]<4) Data->P[plr].Misc[5].Safety+=5;  // manned stuff gets 5

   Data->P[plr].Misc[5].Safety=minn(Data->P[plr].Misc[5].Safety,99);

   if (death==1) {
     for (i=0;i<28;i++) if (PVal[i]>0 && PVal[i]<4) PVal[i]=4;
   }

   // GOAL FILTER: MANNED
   P_Goal=PosGoal(PVal);
   N_Goal=NegGoal(PVal);
   S_Goal=SupGoal(PVal);

   if (P_Goal==LLAND) {  // make sure EVA was done
      if (!(PVal[26]>=1 && PVal[26]<=3)) {
         P_Goal=LORBIT;
         PVal[LLAND]=0;
      }
   }

   if ((P_Goal==-1 && S_Goal==-1) && (PVal[25]>0)) PVal[25]=4;

     if (Check_Dock(500)==2) {    // Success
         Data->P[plr].Misc[4].Safety+=10;
         Data->P[plr].Misc[4].Safety=minn(Data->P[plr].Misc[4].Safety,Data->P[plr].Misc[4].MaxSafety);
       }
     else if (Check_Dock(500)==1) {
         Data->P[plr].Misc[4].Safety+=5;
         Data->P[plr].Misc[4].Safety=minn(Data->P[plr].Misc[4].Safety,Data->P[plr].Misc[4].MaxSafety);
       }

   if (STSp(27) || STSn(27)) PVal[27]=0;  // Clear All Firsts/Negative Goals
   if (STSp(18) || STSn(18)) PVal[18]=0;
   if (STSp(19) || STSn(19)) PVal[19]=0;
   if (STSp(20) || STSn(20)) PVal[20]=0;
   if (STSp(22) || STSn(22)) PVal[22]=0;

   // DURATION FIRSTS
   Data->P[plr].Mission[mis].Duration=maxx(Data->P[plr].Mission[mis].Duration,1);

   if (!Mis.Dur) {
     switch(P_Goal) {
       case MANSPACE:
         mike=7;
         Data->P[plr].Mission[mis].Duration=1;
         break;
       case EORBIT:
         mike= (Mis.Index<=6) ? (Data->P[plr].Mission[mis].Duration=1,7) : (Data->P[plr].Mission[mis].Duration=2,12);break;
       case LPASS:
         mike=11;
         Data->P[plr].Mission[mis].Duration=3;
         break;
       case LORBIT:
         mike=10;
         Data->P[plr].Mission[mis].Duration=4;
         break;
       case LLAND:
         mike=10;
         Data->P[plr].Mission[mis].Duration=4;
         break;
       default: mike=0; break;
     }
   } else {
      mike=14-Data->P[plr].Mission[mis].Duration;
   }

   if (mike>=8 && mike<=12)
      if (P_Goal>=18 || S_Goal>=18)
         PVal[mike]=1;
           //total+=(char) Set_Goal(plr,mike,0);

   // GOAL POSTIVE
   if (P_Goal!=-1) {
      total=Set_Goal(plr,P_Goal,0);
      if (P_Goal!=27) {total+=Set_Goal(plr,27,0);PVal[27]=0;}
      //if (!(Data->Prestige[27].Place==plr || Data->Prestige[27].mPlace==plr))
      //    total+=Set_Goal(plr,27,0);
      }

   // GOAL NEGATIVE
   if (N_Goal!=-1) {
     negs+=PrestNeg(plr,N_Goal);
     PVal[N_Goal]=0;
   }

   if (mcode==32 || mcode==36) PVal[23]=Check_Lab();

   // TOTAL ALL MISSION FIRSTS
   for (i=0;i<28;i++)
      if (PVal[i]==1 || (PVal[i]==2 && other<3000))
         total+=(char) Set_Goal(plr,i,0);
      //else if (PVal[i]==4) negs+=Set_Goal(plr,i,0);

   // CAPSULE FIRSTS   need to check for failure on capsule
   if ((P_Goal!=-1 || S_Goal!=-1) && other<3000 && MANNED[0]>0 && Data->P[plr].Mission[mis].Hard[Mission_Capsule]!=-1) {  // Hardware on first part
     total+=Set_Goal(plr,12+Data->P[plr].Mission[mis].Prog,0);
      }
   if ((P_Goal!=-1 || S_Goal!=-1) && other<3000 && MANNED[1]>0 && Data->P[plr].Mission[mis+1].Hard[Mission_Capsule]!=-1 &&
     Data->P[plr].Mission[mis+1].part==1) {
     total+=Set_Goal(plr,12+Data->P[plr].Mission[mis+1].Prog,0);
     }

#define DNE(a,b) (Data->Prestige[b].Place==(a) || Data->Prestige[b].mPlace==(a))

	if (DNE(plr, DUR_F))
		Data->P[plr].DurLevel = 6;
	else if (DNE(plr, DUR_E))
		Data->P[plr].DurLevel = 5;
	else if (DNE(plr, DUR_D))
		Data->P[plr].DurLevel = 4;
	else if (DNE(plr, DUR_C))
		Data->P[plr].DurLevel = 3;
	else if (DNE(plr, DUR_B))
		Data->P[plr].DurLevel = 2;
	else if (DNE(plr, MANSPACE))
		Data->P[plr].DurLevel = 1;

   // TOTAL ALL MISSION SUBSEQUENTS
   if (total==0) {
      // SET SUBSEQUENT Goal
      if (S_Goal!=-1 && other<3000) total=Set_Goal(plr,S_Goal,0);

      for (i=0;i<28;i++)
         if (PVal[i]==1 || (PVal[i]==2 && other<3000))
            total+=(char) Set_Goal(plr,i,0);
         else if (PVal[i]==3) Set_Goal(plr,i,0);
      }

   // LM POINTS
   Set_LM(plr,STEPnum);
   if (mcode>=48 && mcode<=52 && other<3000) Set_LM(plr,STEPnum);

   // ADD IN NEGATIVES AND RETURN MIN of -10
   total=((total+negs) < -10) ? -10 : total+negs;

   if (!death && total==-10) total = -7;

   return total;
}


char PosGoal_Check(char *PVal)
{
   if (PSTS(22)) return 22;
   else if (PSTS(20)) return 20;
   else if (PSTS(19)) return 19;
   else if (PSTS(7)) return 7;
   else if (PSTS(1)) return 1;
   else if (PSTS(18)) return 18;
   else if (PSTS(27)) return 27;
   else if (PSTS(0)) return 0;

   else return -1;
}


int Find_MaxGoal(void)
{
   int i,ival,cval;
   char PVal[28];
   memset(PVal,0x00,sizeof PVal);

   for (i=0;i<STEPnum;i++) {
      ival=abs(Mev[i].Prest);
      cval=Mev[i].PComp;
      if (ival!=100) {
         PVal[ival]=cval;
         }
      }

   return PosGoal_Check(PVal);
}

int U_AllotPrest(char plr,char mis)
{
   int i=0,total,other,negs,mcode,lun;
   extern struct mStr Mis;
   char PVal[28];

   memset(PVal,0x00,sizeof PVal);     // CLEAR TOTAL VALUE
   total=0, negs=0, lun=0;
   tMo=Data->P[plr].Mission[mis].Month;
   tYr=Data->Year;
   tIDX=bIDX=0;

   // SETUP INFO
   mcode=Data->P[plr].Mission[mis].MissionCode;
   GetMisType(mcode);

   lun=Check_Photo();

   other=MaxFail();

   if ((mcode>=7 && mcode<=13) || mcode==1) {  // Unmanned Probes
       switch (mcode) {
         case 1:  i=0;break; // O.S.
         case 7:  i=1;break; // L.F.B.
         case 8:  i=7;break; // L.P.L.
         case 9:  i=3;break;
         case 10: i=4;break;
         case 11: i=2;break;
         case 12: i=5;break;
         case 13: i=6;break;
         }


      if (other==1) {
        if (mcode==10 || mcode==12 || mcode==13) return 0;
        total=Set_Goal(plr,i,0);
      }
      else negs=PrestNeg(plr,i);

     if (mcode==7 || mcode==8) {
       if (lun==1) {  // UNMANNED PHOTO RECON
         Data->P[plr].Misc[5].Safety+=5;
         Data->P[plr].Misc[5].Safety=minn(Data->P[plr].Misc[5].Safety,99);
       } // if
      } // if

     if (mcode==8 && MaxFail()==1) {  // extra 10 for landing on Moon
       if (lun==1) {  // UNMANNED PHOTO RECON
         Data->P[plr].Misc[5].Safety+=10;
         Data->P[plr].Misc[5].Safety=minn(Data->P[plr].Misc[5].Safety,99);
       } // if
      } // if

   } // if

     if (Check_Dock(2)==2) {
         Data->P[plr].Misc[4].Safety+=10;
         Data->P[plr].Misc[4].Safety=minn(Data->P[plr].Misc[4].Safety,Data->P[plr].Misc[4].MaxSafety);
       }
     else if (Check_Dock(2)==1) {
         Data->P[plr].Misc[4].Safety+=5;
         Data->P[plr].Misc[4].Safety=minn(Data->P[plr].Misc[4].Safety,Data->P[plr].Misc[4].MaxSafety);
       }
  return total+negs;
}

/* vim: set noet ts=4 sw=4 tw=77: */
