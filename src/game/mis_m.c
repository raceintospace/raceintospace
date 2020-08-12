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
#include <utils.h>
#include <options.h>
#include <logging.h>

#define MIS_SET 0             // FAILURES ON

LOG_DEFAULT_CATEGORY(mission)

extern struct MisAst MA[2][4];
extern Equipment *MH[2][8];
extern struct MisEval Mev[60];
extern char MANNED[2],pal2[768],STEPnum,FINAL,AI[2],CAP[2],LM[2],DOC[2],EVA[2],fEarly,mcc,JOINT;
extern char STEP; /**< Index of current mission step */
extern char DMFake;
extern int AUDIO;
char MFlag,death,durx,MPad,Unm,SCRUBS,noDock,InSpace;
char Dock_Skip; /**< used for mission branching */
extern struct mStr Mis;

extern ui16 MisStat;
extern char pCnt,tMen,BIG;            // Counter for pAry
extern i16 pAry[15];         /**< Array for Presige Firsts compelted */
extern REPLAY Rep;

void Tick(char);

void GetFailStat(struct XFails *Now,char *FName,int rnum)
{
  DEBUG2("->GetFailStat(struct XFails *Now,char *FName,int rnum %d)", rnum);
  int i;
  FILE *fin;
  long count;
  struct Fdt {
    char Code[6];
    long offset;
    i16 size;
  } Pul;

  fin=sOpen("FAILS.CDR","rb",0);
  count=44;
  fread(&count,sizeof count,1,fin);  // never written to file
	Swap32bit(count);
  fread(&Pul,sizeof Pul,1,fin);
	Swap32bit(Pul.offset);
	Swap16bit(Pul.size);
  i=0;
  while (xstrncasecmp(Pul.Code,FName,4)!=0 && i<count) {
     fread(&Pul,sizeof Pul,1,fin);
		 Swap32bit(Pul.offset);
		 Swap16bit(Pul.size);
     i++;
  }
  if (i==count) {fclose(fin);return;}

  fseek(fin,Pul.offset,SEEK_SET);

	if (rnum<0) { // Unmanned portion
	  do {
	     fread(Now,sizeof (struct XFails),1,fin);
				Swap32bit(Now->per);	// Only need to swap this one since we're checking only that

	  } while (Now->per!=rnum);
	}
  else {
	  do {
	     fread(Now,sizeof (struct XFails),1,fin);
				Swap32bit(Now->per);
				Swap16bit(Now->code);
				Swap16bit(Now->val);
				Swap16bit(Now->xtra);
				Swap16bit(Now->fail);
	  } while (Now->per<=rnum);
	};

  fclose(fin);
  DEBUG1("<-GetFailStat()");
}

void MisCheck(char plr,char mpad)
{
   int tomflag=0; // toms checking flag
   int val,safety,save,PROBLEM,i,lc,durxx;
   struct XFails Now;
   unsigned char gork=0;

   lc = 0; /* XXX check uninitialized */

   STEPnum=STEP;
   FINAL=STEP=MFlag=0; // Clear Everything
   Unm=MANNED[0]+MANNED[1];
   Dock_Skip=0;  // used for mission branching
   MPad=mpad;
   SCRUBS=noDock=InSpace=0;



  if (!AI[plr] && BIG==0) {
    //FadeOut(1,pal,100,128,1);
    if (plr==1) {
        RectFill(189,173,249,196,55);
        for (i=190;i<250;i+=2) {
          grPutPixel(i,178,61);
          grPutPixel(i,184,61);
          grPutPixel(i,190,61);
          }
        lc=191;
        }
    else if (plr==0) {
        RectFill(73,173,133,196,55);
        for (i=73;i<133;i+=2) {
          grPutPixel(i,178,61);
          grPutPixel(i,184,61);
          grPutPixel(i,190,61);
          }
        lc=76;
        }
  } // END if (!AI[plr])

  if (!AI[plr] && BIG==0) Tick(2);
  Mev[0].trace=0;
  death=0;

  durxx=durx=-1;
  if (Data->P[plr].Mission[mpad].Duration>0) {
    durxx=Data->P[plr].Mission[mpad].Duration-1;
    Data->P[plr].Mission[mpad].Duration=0;
  }
  if (JOINT==1) {
      durxx=maxx(durxx,Data->P[plr].Mission[mpad+1].Duration-1);
      Data->P[plr].Mission[mpad+1].Duration=0;
  }

  do {
     if (STEP>30 || STEP<0) delay(20);

     if (Dock_Skip==1) {
        if (Mev[Mev[STEP].trace].loc==8)
           Mev[STEP].trace++;  // skip over docking.
     }
     if (Mev[STEP].loc==16 && Mis.PCat[4]==22) FirstManOnMoon(plr,0,Mis.Index);

     // Duration Hack Part 1 of 3   (during the Duration stuff)
     if ((Mev[STEP].loc==27 || Mev[STEP].loc==28) && durx>0) {

        if (Mev[STEP].StepInfo!=1) {
           Data->P[plr].Mission[MPad+Mev[STEP].pad].Duration= 1; //Original code would also return 1
           durx=-1;  // end durations
        }
        else {
           Data->P[plr].Mission[MPad+Mev[STEP].pad].Duration++;
           durx--;
           if ((Data->Def.Lev1==0 && plr==0) || (Data->Def.Lev2==0 && plr==1))
              Mev[STEP].dice=MisRandom();
           else Mev[STEP].dice=random(100)+1;
           Mev[STEP].rnum=random(10000);  // reroll failure type
           Mev[STEP].trace=STEP;
        }
     }

    if (Mev[STEP].Name[0]=='A' || Mev[STEP].StepInfo==9 || Mev[STEP].StepInfo==19)
      STEP=Mev[STEP].trace;
    else {
     if (Mev[STEP].trace!=0) STEP=Mev[STEP].trace;
     else {
        Mev[STEP].trace=STEP+1;
        STEP++;
     }
    }

    // Duration Hack Part 2 of 3    (set up durx for duration use)
    if ((Mev[STEP].loc==27 || Mev[STEP].loc==28) && durx==-1) {
        durx=durxx-1;
        Data->P[plr].Mission[MPad+Mev[STEP].pad].Duration=2;
    }

    if (Mev[STEP].Name[0]=='A') {
        grSetColor(11);
        if (!AI[plr] && BIG==0) {
           if (plr==0) {x=5;y=112;RectFill(2,107,140,115,3);}
           else {x=82,y=8;RectFill(78,2,241,10,3);}
           PrintAt(x,y,"COUNTDOWN");
           if (plr==0) {
            RectFill(188,107,294,113,3);
            grSetColor(1);
            PrintAt(190,112,(Mev[STEP].pad==0) ? "PRIMARY LAUNCH":"SECOND LAUNCH");
           }
           else {
            RectFill(244,56,314,62,3);
            grSetColor(1);
            PrintAt(246,61,(Mev[STEP].pad==0) ? "PRIMARY PAD":"SECOND PAD");
           }

        }
        memset(Name,0x00,sizeof Name);
        strcpy(Name,Mev[STEP].Name);
        Name[0]='#';  // Launch Code
        PlaySequence(plr,STEP,Name,0); // Special Case #47236
    };

    // Necessary to keep code from crashing on bogus mission step
    while (Mev[STEP].E==NULL) {
      STEP++;
    }

    // Draw Mission Step Name
    if (!AI[plr] && BIG==0) {
      if (!(fEarly && STEP!=0)) {
        if (plr==0) {x=5;y=112;RectFill(2,107,140,115,3);}
        else {x=82,y=8;RectFill(78,2,241,10,3);}
        grSetColor(11); MisStep(x,y,Mev[STEP].loc);
        if (plr==0) {
          RectFill(188,107,294,113,3);
          grSetColor(1);
          PrintAt(190,112,(Mev[STEP].pad==0) ? "PRIMARY LAUNCH":"SECOND LAUNCH");
        }
        else {
          RectFill(244,56,314,62,3);
          grSetColor(1);
          PrintAt(246,61,(Mev[STEP].pad==0) ? "PRIMARY PAD":"SECOND PAD");
        }
      }
    }


    // SAFETY FACTOR STUFF

    safety=Mev[STEP].E->MisSaf;

    if ((Mev[STEP].Name[0]=='A') && MH[Mev[STEP].pad][7]!=NULL)
    {  // boosters involved
    	safety = RocketBoosterSafety(safety, MH[Mev[STEP].pad][7]->Safety);
    }

    // Duration Hack Part 3 of 3
    if (Mev[STEP].loc==28 || Mev[STEP].loc==27) {
      safety=Mev[STEP].E->MisSaf;  // needs to be for both
      if (InSpace==2) safety=(MH[0][0]->MisSaf+MH[1][0]->MisSaf)/2;   //joints
    }

    if (strncmp(Mev[STEP].E->Name,"DO",2)==0) {
      if (Mev[STEP].loc==1 || Mev[STEP].loc==2)
         safety=Mev[STEP].E->MSF;
      }

    val=Mev[STEP].dice;
    safety+=Mev[STEP].asf;

    if (safety>=100) safety=99;
    save=(Mev[STEP].E->SaveCard==1)? 1 : 0;

    PROBLEM = val > safety;

    if (!AI[plr] && options.want_cheats)
	    PROBLEM = 0;

    DEBUG6("step %c:%s safety %d rolled %d%s", Mev[STEP].Name[0], S_Name[Mev[STEP].loc],
            safety, val,
            PROBLEM ? " problem" : (options.want_cheats ? " cheating" : ""));

    if (!AI[plr] && BIG==0)
      if (!(fEarly && STEP!=0))
        lc=MCGraph(plr,lc,maxx(0,safety),maxx(0,val),PROBLEM);  // Graph Chart

    if (PROBLEM && save==1) {    // Failure Saved
      Mev[STEP].E->SaveCard--;    // Deduct SCard
      PROBLEM=0;   // Fix problem
      }


   // Debug Code Only
#if MIS_SET
   if (!AI[plr]) {               //&& TOM && NOFAIL
     memset(Text,0x00,sizeof Text);
     strcpy(Text,"FAIL:\0");
     strcpy(&Text[5],S_Name[Mev[STEP].loc]);

     PROBLEM=Request(0,Text,6);

   }
#endif

   // Fix wrong anim thing for the Jt Durations
      if (Mev[STEP].loc==28 || Mev[STEP].loc==27) {
         strcpy(Mev[STEP].Name,(plr==0)?"_BUSC0\0":"_BSVC0");
         Mev[STEP].Name[5]=Mev[STEP].E->ID[1];
      }

   if (PROBLEM==1) {    //Step Problem

#if MIS_SET
     if (!AI[plr] ) DebugSetFailure(Text);  // && TOM && NOFAIL
#endif

     // for the unmanned mission
	   if (MANNED[Mev[STEP].pad]==0 && MANNED[other(Mev[STEP].pad)]==0)
       Mev[STEP].rnum=(-1)*(random(5)+1);

     // Unmanned also
     if (MANNED[Mev[STEP].pad]==0 && noDock==0)
       Mev[STEP].rnum=(-1)*(random(5)+1);

      memset(&Now,0x00,sizeof Now);


     //***************TC Special little HMOON EVA FAILURE FIX

         tomflag = 0;
         if (Mev[STEP].Name[0]=='H' && Mev[STEP].Name[1]=='M')
      {

         gork=Data->P[plr].Mission[mpad].Prog;
         if (gork==2) strcpy(Mev[STEP].Name,(plr==0)?"HUM3C1":"HSM3C1");
         else if (gork==3) strcpy(Mev[STEP].Name,(plr==0)?"HUM3C2":"HSM3C2");
         else strcpy(Mev[STEP].Name,(plr==0)?"HUM3C3":"HSM3C3");
         tomflag = 1;
      };



     //:::::: Failure docking klugge

      if (Mev[STEP].Name[0]=='I')
      {
       gork=Data->P[plr].Mission[mpad].Prog;
       if (gork==2) strcpy(Mev[STEP].Name,(plr==0)?"IUM4C1":"ISM4C1");
        else if (gork==3) strcpy(Mev[STEP].Name,(plr==0)?"IUM4C2":"ISM4C2");
         else strcpy(Mev[STEP].Name,(plr==0)?"IUM4C3":"ISM4C3");
      };

// *********** TOM's FAIL HMOON KLUDGE

      // if HMOON FAILURE
      if (tomflag) {
        GetFailStat(&Now,Mev[STEP].FName,7595);
      } else {
        DEBUG3("Failing !tomflag - calling GetFailStat(&Now, Mev[STEP].FName %s, MEV[STEP].rnum %d))", Mev[STEP].FName, Mev[STEP].rnum);
        GetFailStat(&Now,Mev[STEP].FName,Mev[STEP].rnum);       // all others
      }


      VerifyData();
      memset(Name,0x00,sizeof Name);
      memcpy(Name,Mev[STEP].Name,strlen(Mev[STEP].Name)); // copy orig failure
      Name[strlen(Name)]=0x30+(Now.fail/10);
      Name[strlen(Name)]=0x30+Now.fail%10;
      Name[strlen(Name)]=0x00;




      //:::::: STEP FAILURE :::::::::
      //:::::::::::::::::::::::::::::

      PlaySequence(plr,STEP,Name,1);
      if (!AI[plr]) Tick(2);  //reset dials

      FailEval(plr,Now.code,Now.text,Now.val,Now.xtra);
    } else {   // Step Success

      if (Mev[STEP].loc==28 || Mev[STEP].loc==27) {
         strcpy(Mev[STEP].Name,(plr==0)?"bUC0":"bSC0");
         Mev[STEP].Name[5]=Mev[STEP].E->ID[1];
      }

      if (strncmp(Mev[STEP].E->Name,"DO",2)==0) {
        if (Mev[STEP].loc==2) Data->P[plr].DMod=2;
      }

     //::::::::::::::::::::::::::::::::::
     //::: SUCCESS: Docking klugge ::::::
     //::::::::::::::::::::::::::::::::::
     if (Mev[STEP].Name[0]=='I')
      {
       gork=Data->P[plr].Mission[mpad].Prog;
       if (gork==2) strcpy(Mev[STEP].Name,(plr==0)?"IUM4C1":"ISM4C1");
        else if (gork==3) strcpy(Mev[STEP].Name,(plr==0)?"IUM4C2":"ISM4C2");
         else strcpy(Mev[STEP].Name,(plr==0)?"IUM4C3":"ISM4C3");
      };

     if (Mev[STEP].Name[0]=='e' && Data->P[plr].Mission[mpad].MissionCode==7)
       strcpy(Mev[STEP].Name,(plr==0)?"VUP1":"VSP1");

      //:::::: STEP SUCCESS :::::::::
      //:::::::::::::::::::::::::::::
      // Female step klugge
      // third parameter (0 -> MALE) (2 -> FEMALE)
      //:::::::::::::::::::::::::::::

      gork=0;

      gork = ((MA[0][0].A!=NULL && MA[0][0].A->Sex && EVA[0]==0)
           || (MA[0][1].A!=NULL && MA[0][1].A->Sex && EVA[0]==1)
           || (MA[0][2].A!=NULL && MA[0][2].A->Sex && EVA[0]==2)
           || (MA[0][3].A!=NULL && MA[0][3].A->Sex && EVA[0]==3)
           || (MA[1][0].A!=NULL && MA[1][0].A->Sex && EVA[1]==0)
           || (MA[1][1].A!=NULL && MA[1][1].A->Sex && EVA[1]==1)
           || (MA[1][2].A!=NULL && MA[1][2].A->Sex && EVA[1]==2)
           || (MA[1][3].A!=NULL && MA[1][3].A->Sex && EVA[1]==3));

     //if (!((mcc==9 || mcc==11) && (Mev[STEP].Name[0]=='W')))
       PlaySequence(plr,STEP,Mev[STEP].Name,(gork==1) ? 2 : 0 );  // Play Animations

      VerifyData();
      if (Mev[STEP].sgoto==100) Mev[STEP].trace=0x7F;
      else if (Mev[STEP].sgoto!=0) Mev[STEP].trace=Mev[STEP].sgoto;
      else Mev[STEP].trace=STEP+1;

      if (!(strncmp(Mev[STEP].E->Name,"DO",2)==0 && Mev[STEP].loc==0x02)) {
         Mev[STEP].E->MisSucc++; // set for all but docking power on
      }

	    Mev[STEP].StepInfo=1;

	   // Bottom of success statement
    }

   if (Mev[STEP].loc==0x7f || Mev[STEP].sgoto==100) // force mission end
      Mev[STEP].trace=0x7f;

   if ((mcc==10 || mcc==12 || mcc==13) && STEP==2) Mev[STEP].trace=0x7f;
     if (STEP>30 || STEP<0) {
        delay(20);
     }

   if (Mev[STEP].Prest!=0) {
     pAry[pCnt++]=STEP;
   }

  if (Mev[STEP].sgoto==Mev[STEP].fgoto && Mev[STEP].trace!=0x7f)
     Mev[STEP].trace=Mev[STEP].sgoto;

  if (Mev[STEP].loc==8 && Mev[STEP].StepInfo==1) noDock=1;

  if (Mev[STEP].loc==0 && MANNED[Mev[STEP].pad]>0) InSpace++;
  if (Mev[STEP].loc==4 && MANNED[Mev[STEP].pad]>0) InSpace--;

  // these two lines fix a problem with Lab Missions failing on launch and
  // ending the mission.
  // Yet another in the ongoing series of special cases.
  //

  if (Mev[STEP].trace==0x7f && STEP==3) {
     switch(Mis.Index) {
      case 19: case 22: case 23: case 30:
      case 32: case 35: case 36: case 37:
         InSpace=1;
         Mev[STEP].trace=Mev[STEP].dgoto;
      default: break;
     }
  }
//  if (Mev[STEP].trace==0x7f && InSpace>0) Mev[STEP].trace=STEP+1;

 } while(Mev[STEP].trace!=0x7f);            // End mission
  //end do
  if (!AI[plr] && death==0) delay(1000);

  if ((MA[0][0].A!=NULL && MA[0][0].A->Status==AST_ST_DEAD)
        || (MA[0][1].A!=NULL && MA[0][1].A->Status==AST_ST_DEAD)
        || (MA[0][2].A!=NULL && MA[0][2].A->Status==AST_ST_DEAD)
        || (MA[0][3].A!=NULL && MA[0][3].A->Status==AST_ST_DEAD)
        || (MA[1][0].A!=NULL && MA[1][0].A->Status==AST_ST_DEAD)
        || (MA[1][1].A!=NULL && MA[1][1].A->Status==AST_ST_DEAD)
        || (MA[1][2].A!=NULL && MA[1][2].A->Status==AST_ST_DEAD)
        || (MA[1][3].A!=NULL && MA[1][3].A->Status==AST_ST_DEAD))
  {  // Mission Death
    if (!AI[plr]) {
     if (BIG==0) {
       memset(&pal[64*3],0x00,64*3);  //Specs: 0x08
       gxSetDisplayPalette(pal);
       if (plr==0) RectFill(2,107,140,115,3);
        else RectFill(78,2,241,10,3);
       if (plr==0) RectFill(188,107,294,113,3);
       else RectFill(244,56,314,62,3);
     }
     PlaySequence(plr,STEP,(plr==0)?"UFUN":"SFUN",0);
    }
    if (!AI[plr]) delay(1000);
    death=1;
  }
  else {
   death=0;
  };

  return;
}

/** Draw mission step rectangle
 *
 * The rectangle represents the success or failure rate.
 *
 * \param plr Player data
 * \param lc ??? maybe location of the chart
 * \param safety Safety factor in percent
 * \param val value of the dice checked against safety
 * \param prob is this a problem or not?
 *
 * \return new value of lc
 */
int MCGraph(char plr,int lc,int safety,int val,char prob)
{
    int i;
    TRACE5("->MCGraph(plr, lc %d, safety %d, val %d, prob %c)", lc, safety, val, prob);
    RectFill(lc-2,195,lc,195-safety*22/100,11);
    RectFill(lc-2,195,lc,195-(safety-Mev[STEP].asf)*22/100,6);
    for (i=195;i>195-val*22/100;i--) {
        RectFill(lc-2,195,lc,i,21);
        delay(15);
    }


  if (plr==1 && !AI[plr]) {
      if (val>safety && prob==0) {
        RectFill(lc-2,195,lc,195-val*22/100,9);
        lc=191;
      } else if (val>safety) {
        RectFill(lc-2,195,lc,195-val*22/100,9);
        lc+=5;
      } else {
        if (lc>=241) {
          grSetColor(55);RectFill(189,173,249,196,55);
          for (i=190;i<250;i+=2) {
             grPutPixel(i,178,61);
             grPutPixel(i,184,61);
             grPutPixel(i,190,61);
            }
          RectFill(189,195,191,195-safety*22/100,11);
          RectFill(189,195,191,195-(safety-Mev[STEP].asf)*22/100,6);
          RectFill(189,195,191,195-val*22/100,21);
          if (Mev[STEP].asf>0) RectFill(189,195-safety*22/100,191,195-safety*22/100,11);

          lc=196;
          /* lc > 241 */
        } else {
          lc+=5;
        }
      } /* check safety and problem */
    } else if (plr==0 && !AI[plr]) {
      if (val>safety && prob==0) {
        RectFill(lc-2,195,lc,195-val*22/100,9);
        lc=76;
      } else if (val>safety) {
        RectFill(lc-2,195,lc,195-val*22/100,9);
        lc+=5;
      } else {
        if (lc>=126) {
          RectFill(73,173,133,196,55);
          for (i=73;i<133;i+=2) {
            grPutPixel(i,178,61);
            grPutPixel(i,184,61);
            grPutPixel(i,190,61);
          }
          RectFill(74,195,76,195-safety*22/100,11);
          RectFill(74,195,76,195-(safety-Mev[STEP].asf)*22/100,6);
          RectFill(74,195,76,195-val*22/100,21);
          if (Mev[STEP].asf>0) {
            RectFill(74,195-safety*22/100,76,195-safety*22/100,11);
          }
          lc=81;
        } else {
          lc+=5;
        }
      }
    }
    TRACE1("<-MCGraph()");
    return lc;
}

#define F_ALL 0
#define F_ONE 1

void F_KillCrew(char mode,struct Astros *Victim)
{
  int k=0, p = 0;
  struct Astros *Guy;

  Guy = NULL; /* XXX check uninitialized */

  // Reset Hardware
  if (Victim >= &Data->P[1].Pool[0]) p = 1;

  if ( (Data->Def.Lev1 == 0 && p == 0 ) || ( Data->Def.Lev2 == 0 && p == 1) )
	Mev[STEP].E->Safety/=2;
  else Mev[STEP].E->Safety=Mev[STEP].E->Base;
  Mev[STEP].E->MaxRD = Mev[STEP].E->MSF-1;

  if (mode==F_ALL) {
	for(k=0;k<MANNED[Mev[STEP].pad];k++) {  // should work in news
	  Guy=MA[Mev[STEP].pad][k].A;
	  if (Guy!=NULL) {
		Guy->Status=AST_ST_DEAD;
		Guy->Special=3;
		Guy->RetReas=8;
		Guy->Assign=Guy->Moved=Guy->Crew=Guy->Task=Guy->Una=0;
		tMen++;
		death=1;
	  }
	}
  }
  else if (mode==F_ONE) {  // should work in news
	if (Victim==NULL) return;
	Victim->Status=AST_ST_DEAD;
	Victim->Special=3;
	Victim->RetReas=8;
	Victim->Assign=Victim->Moved=Victim->Crew=Victim->Task=0;

	/* XXX this code was here, but Guy wsa not initialized */
	// Guy->Una=0;

	tMen++;
	//death=1;
  }
}


#define F_RET 0
#define F_INJ 1

void F_IRCrew(char mode,struct Astros *Guy)
{
   if (Guy->Status==AST_ST_DEAD) return;
   if (Guy==NULL) return;
   if (mode==F_RET) {  // should work in news
      Guy->Status=AST_ST_RETIRED;
      Guy->RDelay=1;   // Retire begginning of next season
      Guy->RetReas=9;
      Guy->Assign=Guy->Moved=Guy->Crew=Guy->Task=Guy->Una=0;
      }
   else if (mode==F_INJ) {
      Guy->Status=AST_ST_INJURED;
      Guy->IDelay=3;  // Injured for a year
      Guy->Special=4;
      Guy->Assign=Guy->Moved=Guy->Crew=Guy->Task=Guy->Una=0;
      }
}

int FailEval(char plr,int type,char *text,int val,int xtra)
{
   int FNote=0,temp,k,ctr;
   char PROBLEM=0;
   struct Astros *crw;

   temp = 0; /* XXX check uninitialized */

   if (!(strncmp(Mev[STEP].E->Name,"DO",2)==0 && Mev[STEP].loc==0x02)) {
     Mev[STEP].E->MisFail++;  // set failure for all but docking power on
   }
   Mev[STEP].StepInfo=1003;
   FNote=5;  // Mission Failure

   if (Unm==0) {
     Mev[STEP].trace=0x7f;
     if (type==12) DestroyPad(plr,MPad+Mev[STEP].pad,20,0);
     if (!AI[plr]) temp=FailureMode(plr,FNote,text);

     //Special Case for PhotoRecon with Lunar Probe
     if (Mev[STEP].loc==20 && mcc==8)
        Mev[STEP-1].E->MisFail++;
     return 0;
   };

  Mev[STEP].StepInfo=0;

	switch(type) {

	   case 0: // Failure has no effect on Mission
      case 20:   // don't want to test for crew experience
         FNote=0;
         Mev[STEP].StepInfo=50;
         if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else Mev[STEP].trace=STEP+1;
		   break;


	   case 2:  // End of Mission Failure
        FNote=5;
		   Mev[STEP].StepInfo=1000+Mev[STEP].loc;
        Mev[STEP].trace=0x7f;
		   break;


     case 5:  // Stranded Step  (temp)
	   case 3:  // Kill ALL Crew and END Mission
         if (type==5) FNote=4;
         FNote=8;
         if (InSpace>0 && MANNED[Mev[STEP].pad]==0 && strncmp(Mev[STEP].E->ID,"M2",2)==0) {
           Mev[STEP].pad=other(Mev[STEP].pad);  // for Kicker-C problems
           F_KillCrew(F_ALL,0);
           Mev[STEP].pad=other(Mev[STEP].pad);
         }
		    else F_KillCrew(F_ALL,0);

		    Mev[STEP].StepInfo=4600+Mev[STEP].loc;
         Mev[STEP].trace=0x7F;
		    break;

	   case 4:  // Branch to Alternate Step
         FNote=1;
		   Mev[STEP].StepInfo=1900+Mev[STEP].loc;

         if (Mev[STEP].fgoto==-1) {  // End of Mission Flag
	         if (Mev[STEP].PComp>0) Mev[STEP].PComp=4;
            Mev[STEP].trace=0x7F;  // End of Mission Signal
            FNote=5;
	         }
         else if (Mev[STEP].fgoto!=-2) {  // Alternate Step is other num
	         if (Mev[STEP].PComp>0) Mev[STEP].PComp=4;
            Mev[STEP].trace=Mev[STEP].fgoto;
	         }
         else Mev[STEP].trace=STEP+1;
		   break;

	   case 6:   // Reduce Safety by VAL% temp
         FNote=0;
         Mev[STEP].E->MisSaf-=abs(val);
		   if (Mev[STEP].E->MisSaf<=0)
            Mev[STEP].E->MisSaf=1;
		   Mev[STEP].StepInfo=900+Mev[STEP].loc;
        if (Mev[STEP].fgoto==-1 && Unm==0) Mev[STEP].trace=0x7F;
        else Mev[STEP].trace=STEP+1;
		   break;

	   case 7:   // Reduce Safety by VAL% perm
         FNote=0;
		   Mev[STEP].StepInfo=1700+Mev[STEP].loc;
         if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else Mev[STEP].trace=STEP+1;
		   break;

	   case 9:  // Recheck Step
         FNote=2;
         Mev[STEP].StepInfo=9;
//         Mev[STEP].rnum=random(10000)+1;  // new failure roll
//         Mev[STEP].dice=random(100)+1;    // new die roll
         Mev[STEP].trace=STEP;            // redo step
		   break;

	   case 12:  // Subtract VAL% from Safety, repair Pad for XTRA (launch only)
         FNote=5;
         Mev[STEP].E->MisSaf-=abs(val);
		   if (Mev[STEP].E->MisSaf<=0)
            Mev[STEP].E->MisSaf=1;
		   Mev[STEP].StepInfo=1600+Mev[STEP].loc;

         DestroyPad(plr,MPad+Mev[STEP].pad,abs(xtra),0);  // Destroy Pad

         Mev[STEP].trace=0x7F; // signal end of mission
         break;

      case 13: // Kill Crew, repair Pad for VAL
         FNote=8;
		   F_KillCrew(F_ALL,0);
         DestroyPad(plr,Mev[STEP].pad+MPad,(val==0)?abs(xtra):abs(val),0);  // Destroy Pad
		   Mev[STEP].StepInfo=4500+Mev[STEP].loc;
         Mev[STEP].trace=0x7F;
		   break;

	   case 15:  // Give option to Scrub  1%->20% negative of part
         FNote=3;
         Mev[STEP].E->MisSaf-=random(20)+1;
         if (Mev[STEP].E->MisSaf<=0)
            Mev[STEP].E->MisSaf=1;
		   Mev[STEP].StepInfo=15;
		   break;

	   case 16: // VAL% injury,   XTRA% death
         FNote=0;
		   Mev[STEP].StepInfo=1100+Mev[STEP].loc;

		   for(k=0;k<MANNED[Mev[STEP].pad];k++) {
		      if (random(100)>=val) {
               F_IRCrew(F_INJ,MA[Mev[STEP].pad][k].A);
		         Mev[STEP].StepInfo=2100+Mev[STEP].loc;
               FNote=9;
		         }
		      }; // for

		   ctr=0;

		   for(k=0;k<MANNED[Mev[STEP].pad];k++) {
		      if (Data->P[plr].Pool[temp].Status==AST_ST_RETIRED) {
		         if (random(100)>xtra) {
                  F_KillCrew(F_ONE,MA[Mev[STEP].pad][k].A);
			         Mev[STEP].StepInfo=3100+Mev[STEP].loc;
                  FNote=8;
			         ctr++;
		            }
		         }
		      }

		   if (ctr==MANNED[Mev[STEP].pad]) {
            Mev[STEP].StepInfo=4100+Mev[STEP].loc;
            Mev[STEP].trace=0x7F;
            }
         else if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else Mev[STEP].trace=STEP+1;

        if (Mev[STEP].FName[3]==0x30) Mev[STEP].trace=0x7f;

		   break;



	   case 17: // VAL% survial and XTRA% if injury and retirement
		   Mev[STEP].StepInfo=1300+Mev[STEP].loc;
		   for(k=0;k<MANNED[Mev[STEP].pad];k++) {
		      if (random(100)>=xtra) {
               F_IRCrew(F_RET,MA[Mev[STEP].pad][k].A);
		         Mev[STEP].StepInfo=2300+Mev[STEP].loc;
               FNote=9;
		         }
		      };
         ctr=0;
		   for(k=0;k<MANNED[Mev[STEP].pad];k++) {
		      if (random(100)>=val) {
               F_KillCrew(F_ONE,MA[Mev[STEP].pad][k].A);
		         Mev[STEP].StepInfo=3300+Mev[STEP].loc;
               FNote=8;
               ctr++;
		         }
		      }; // for

		   if (ctr==MANNED[Mev[STEP].pad]) {
            Mev[STEP].StepInfo=4100+Mev[STEP].loc;
            Mev[STEP].trace=0x7F;
            }
         else if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else Mev[STEP].trace=STEP+1;
		   break;

	   case 18:    // set MFlag from VAL, branch if already set
		   if ((MFlag&val) > 0) {
            FNote=1;
            Mev[STEP].StepInfo=1800+Mev[STEP].loc;
            if (Mev[STEP].fgoto==-1) {Mev[STEP].trace=0x7F;FNote=0;}
            else if (Mev[STEP].fgoto!=-2) Mev[STEP].trace=Mev[STEP].fgoto;
            else Mev[STEP].trace=STEP+1;
            }
		   else {
            FNote=0;
            Mev[STEP].StepInfo=18;
            MFlag=MFlag|val;
            if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
            else Mev[STEP].trace=STEP+1;
            };
		   break;

	  case 19:   // Set mission flag and recheck step
		   if ((MFlag&val) > 0) {
            Mev[STEP].StepInfo=1200+Mev[STEP].loc;
            FNote=2;
            }
		   else {
            FNote=2;
            Mev[STEP].StepInfo=19;
            MFlag=MFlag|val;
            };
         Mev[STEP].trace=STEP;            // recheck step
//         Mev[STEP].rnum=random(10000)+1;  // new failure roll
//         Mev[STEP].dice=random(100)+1;    // new die roll
		   break;


	   case 22: // one man % survival :: EVA
		   Mev[STEP].StepInfo=19;
		   if (random(100)>val) {
            FNote=8;
            crw=(EVA[Mev[STEP].pad]!=-1) ? MA[Mev[STEP].pad][EVA[Mev[STEP].pad]].A : MA[other(Mev[STEP].pad)][EVA[other(Mev[STEP].pad)]].A;
            F_KillCrew(F_ONE,crw);

            if (Mev[STEP].Name[6]==0x36) death=1;  // one man lem

		       Mev[STEP].StepInfo=3200+Mev[STEP].loc;
            Mev[STEP].trace=STEP+1;
		      };
		   break;

	   case 23: // VAL% retirement, hardware cut %XTRA perm
         FNote=0;
		   Mev[STEP].StepInfo=23+Mev[STEP].loc;
		   for(k=0;k<MANNED[Mev[STEP].pad];k++) {
		      if (random(100)>=val) {
               FNote=9;
               F_IRCrew(F_RET,MA[Mev[STEP].pad][k].A);
		         Mev[STEP].StepInfo=2400+Mev[STEP].loc;
               ctr++;
		         }
		      };
         //Used to reduce safety

         if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else if (Mev[STEP].fgoto!=-2) Mev[STEP].trace=Mev[STEP].fgoto;
         else Mev[STEP].trace=STEP+1;
		   break;

	   case 24:   // Reduce Safety by VAL% perm :: hardware recovered
         FNote=5;
         Mev[STEP].E->Safety-=random(10);
		   if (Mev[STEP].E->Safety<=0)
            Mev[STEP].E->Safety=1;
		   Mev[STEP].StepInfo=800+Mev[STEP].loc;
         Mev[STEP].trace=0x7F;

		   break;

      case 25:    // Mission Failure recover Minishuttle
         FNote=5;
         Mev[STEP].StepInfo=700;
         Mev[STEP].trace=0x7F;   // End of Mission
         break;

      case 26: // Subtract VAL% from Equip perm and branch to alternate
         FNote=1;
         Mev[STEP].StepInfo=1926;
         Mev[STEP].E->Safety-=random(10);
		   if (Mev[STEP].E->Safety<=0)
            Mev[STEP].E->Safety=1;

         if (Mev[STEP].fgoto==-1) {Mev[STEP].trace=0x7F;FNote=7;}
         else if (Mev[STEP].fgoto!=-2) Mev[STEP].trace=Mev[STEP].fgoto;
         else Mev[STEP].trace=STEP+1;
		   break;


	   case 30:  // Duration Failure
	   Data->P[plr].Mission[MPad+Mev[STEP].pad].Duration= 1; //Original code would also return 1
           durx=-1;  // end durations
         FNote=7;
		   Mev[STEP].StepInfo=1950;
         Mev[STEP].trace=STEP+1;

		   break;

   case 31:  // kill EVA and LM people and branch VAL steps
      //Dock_Skip=1;
      Mev[STEP].trace=Mev[STEP].dgoto;
      Mev[STEP].StepInfo=3100+STEP;

      if (strncmp(Mev[STEP].E->ID,"C4",2)==0) {
         F_KillCrew(F_ALL,0);
         Mev[STEP].StepInfo=4100+STEP;
      }
      else {
        if (JOINT==0) {
          F_KillCrew(F_ONE,MA[0][EVA[0]].A);
          if (LM[0]!=EVA[0]) F_KillCrew(F_ONE,MA[0][LM[0]].A);
        }
        else {
          F_KillCrew(F_ONE,MA[1][EVA[1]].A);
          if (LM[1]!=EVA[1]) F_KillCrew(F_ONE,MA[1][LM[1]].A);
        }
      }
      death=1;
      break;

	   case 33:  // Kill Crew on All Capsules (happens only on dockings)
         if (type==5) FNote=4;
         FNote=8;
        if (MANNED[Mev[STEP].pad]>0) F_KillCrew(F_ALL,0);
        if (MANNED[other(Mev[STEP].pad)]>0)  {
           Mev[STEP].pad=other(Mev[STEP].pad);  // switch pad for a sec
           F_KillCrew(F_ALL,0);
           Mev[STEP].pad=other(Mev[STEP].pad);  // restore current pad
        }
		   Mev[STEP].StepInfo=4600+Mev[STEP].loc;
         Mev[STEP].trace=0x7F;
		   break;

	   case 1: case 8: case 10: case 11: case 14: case 21: case 27:
      case 28: case 29: default:
         FNote=0;
         Mev[STEP].StepInfo=50;
         if (Mev[STEP].fgoto==-1) Mev[STEP].trace=0x7F;
         else Mev[STEP].trace=STEP+1;
		   break; // nothing : continue steps
	};

  if ((Mev[STEP].Name[0]=='A') && MH[Mev[STEP].pad][7]!=NULL)
  {  // boosters involved
     if (MH[Mev[STEP].pad][4]->Safety==MH[Mev[STEP].pad][4]->Base)
       MH[Mev[STEP].pad][7]->Safety=MH[Mev[STEP].pad][7]->Base;
  }

  VerifySF(plr);  // Keep all safety's within the proper ranges

  // check for all astro's that are dead.  End mission if this is the case.
   while(bioskey(1)) bioskey(0);
   key=0;

   if (!AI[plr]) temp=FailureMode(plr,FNote,text);

   if (temp==0 && FNote==3) Mev[STEP].trace=STEP+1;
   else if (FNote==3) {Mev[STEP].StepInfo+=1000;SCRUBS=1;}

    if (SCRUBS==1) {
      if (Mev[STEP].loc==8 && noDock==1) PROBLEM=1;

      if (PROBLEM==0) {
         if (Mev[STEP].fgoto==-1) {  // End of Mission Flag
	         if (Mev[STEP].PComp>0) Mev[STEP].PComp=4;
            Mev[STEP].trace=0x7F;  // End of Mission Signal
            FNote=5;
	         }
         else if (Mev[STEP].fgoto!=-2) {  // Alternate Step is other num
	         if (Mev[STEP].PComp>0) Mev[STEP].PComp=4;
           Mev[STEP].trace=Mev[STEP].fgoto;
	         }
         else Mev[STEP].trace=STEP+1;
      }
    }

  if (strncmp(Mev[STEP].E->ID,"M3",2)==0) death=0;  //what???

  // New death branching code
  if (death==1) {
     if (Mev[STEP].dgoto==0) Mev[STEP].trace=0x7f;
     else if (Mev[STEP].dgoto>0) Mev[STEP].trace=Mev[STEP].dgoto;
  }

   if (type==9 || type==19) {
     Mev[STEP].trace=STEP;
     Mev[STEP].rnum=random(10000)+1;  // new failure roll
     Mev[STEP].dice=random(100)+1;    // new die roll
   }

  death=0;
  return FNote;
}


// EOF
#if MIS_SET

void DebugSetFailure(char *Text)
{
 int fin,k=0,i,j;
 long len[2],length;
 char D[4],A[6];
 GXHEADER local;

 GV(&local,216,84);
 gxGetImage(&local,70,29,285,112,0);

 grSetColor(1);
 ShBox(70,29,285,112);
 InBox(76,34,279,46);RectFill(77,35,278,45,7);
 grSetColor(1);PrintAt(139,42,"FAILURE ENTRY");
 InBox(76,49,279,68);RectFill(77,50,278,67,7);
 InBox(141,76,174,86);RectFill(142,77,173,85,10);

 grSetColor(1);PrintAt(112,83,"RNUM:   ");
 DispNum(0,0,Mev[STEP].rnum);
 InBox(141,91,174,101);RectFill(142,92,173,100,10);
 grSetColor(1);PrintAt(102,98,"NEW:");

 for(i=0;i<5;i++)
  {
   OutBox(187+i*12,74,197+i*12,82);
   sprintf(&D[0],"%d",i);
   grSetColor(11);
   PrintAt(190+i*12,80,&D[0]);
  }
 grSetColor(11);
 for(i=0;i<5;i++)
  {
   OutBox(187+i*12,84,197+i*12,92);
   sprintf(&D[0],"%d",i+5);
   grSetColor(11);
   PrintAt(190+i*12,90,&D[0]);
  }

 OutBox(187,95,245,103);grSetColor(11);PrintAt(202,101,"ENTER");
 grSetColor(11);
 PrintAt(82,57,"ENTER THE NEW FAILURE NUMBER FOR");
 PrintAt(82,64,Text);
 memset(A,0x00,sizeof A);i=0; //reset i

 while(1)
  {
   key=0;
		WaitForMouseUp();
   GetMouse();

   if ((key==K_ENTER ) || (x>=187 && y>=95 && x<=245 && y<=103 && mousebuttons>0))
    {
     InBox(187,95,245,103);
		WaitForMouseUp();

     if (i>0) {
       i=atoi(&A[0]);
       Mev[STEP].rnum=maxx(minn(9999,i),0);
     }
     gxPutImage(&local,gxSET,70,29,0);
     DV(&local);
     key=0;
     return;
    }
	 if (i<4 && key>='0' && key<='9')
	  {
      j=key-0x30;
      if (j<5) InBox(187+j*12,74,197+j*12,82);
       else InBox(187+(j-5)*12,84,197+(j-5)*12,92);
		A[i++]=key;

      if (key>0) delay(150);
      RectFill(142,92,173,100,10);
		grSetColor(1);PrintAt(144,98,&A[0]);
      if (j<5) OutBox(187+j*12,74,197+j*12,82);
       else OutBox(187+(j-5)*12,84,197+(j-5)*12,92);

 	   key=0;
	  }
	 if (i>0 && key==0x08)
	  {

	   A[--i]=0x00;
		grSetColor(1);
      RectFill(142,92,173,100,10);
      PrintAt(144,98,&A[0]);

		key=0;
	  }
    for(j=0;j<5;j++)
     {
      if (x>=187+j*12 && y>=74 && x<=197+j*12 && y<=82 && mousebuttons>0 && i<4)
       {

        InBox(187+j*12,74,197+j*12,82);
        key=j+0x30;
        A[i++]=key;
				WaitForMouseUp();
        if (key>0) delay(150);
        RectFill(142,92,173,100,10);
	     grSetColor(1);PrintAt(150,98,&A[0]);
 	     key=0;
        OutBox(187+j*12,74,197+j*12,82);

       }
     }
    for(j=0;j<5;j++)
     {
      if (x>=187+j*12 && y>=84 && x<=197+j*12 && y<=92 && mousebuttons>0 && i<4)
       {

        InBox(187+j*12,84,197+j*12,92);
        key=j+0x35;
        A[i++]=key;
				WaitForMouseUp();
        if (k>0) delay(150);
        RectFill(142,92,173,100,10);
        grSetColor(1);PrintAt(150,98,&A[0]);
 	     key=0;
        OutBox(187+j*12,84,197+j*12,92);

       }
     }
  }
}

#endif
