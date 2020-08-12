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
/** \file aipur.c AI Purchasing Routines
 */

#include <Buzz_inc.h>
#include <externs.h>
#include <options.h>   //Naut Randomize && Naut Compatibility, Nikakd, 10/8/10

struct ManPool *Men;
char AIsel[25],AIMaxSel,Obs[6]; //indexed 1 thru 5
extern char AI[2],Option;

void DrawStatistics(char Win)
{
 GXHEADER local;
 char AImg[7]={8,9,10,11,13,14,0};
 char Digit[2];
 int starty,qty,i;
 FILE *fin;
 strncpy(IDT,"i145",4);strncpy(IKEY,"k045",4);
 FadeOut(2,pal,10,0,0);
 PortPal(0);
 
 gxClearDisplay(0,0);
 ShBox(35,33,288,159);
 InBox(40,69,111,109);InBox(116,69,283,109);
 InBox(40,114,111,154);InBox(116,114,283,154);
 IOBox(191,40,280,62);
 Flag(41,70,Win);
 Flag(41,115,other(Win));
 DispBig(48,44,"STATISTICS",1,-1);
 DispBig(215,45,"EXIT",1,-1);
 grSetColor(6);
 PrintAt(122,78,"WINNING DIRECTOR: ");
 grSetColor(8);
 if (AI[Win])
  {
   PrintAt(0,0,"COMPUTER");
   sprintf(&Digit[0],"%d",Data->P[Win].Track[3]);
   PrintAt(0,0,&Digit[0]);
  }     
  else if (Win==0) PrintAt(0,0,&Data->P[Data->Def.Plr1].Name[0]);
   else PrintAt(0,0,&Data->P[ Data->Def.Plr2 ].Name[0]);
 grSetColor(6);
 PrintAt(122,125,"LOSING DIRECTOR: ");
 grSetColor(8);
 if (AI[other(Win)])
  {
   PrintAt(0,0,"COMPUTER");
   sprintf(&Digit[0],"%d",Data->P[other(Win)].Track[3]);
   PrintAt(0,0,&Digit[0]);
  }
  else if (Win==0) PrintAt(0,0,&Data->P[Data->Def.Plr2].Name[0]);
   else PrintAt(0,0,&Data->P[ Data->Def.Plr1 ].Name[0]);
 qty=6;starty=118;
 GV(&local,30,19);
 fin=sOpen("PORTBUT.BUT","rb",0);
 OutBox(152,41,183,61); //directors ranking
 for (i=0;i<qty;i++)
  {
   if (i<=4 && AI[Win]==0) OutBox(starty+(i*33),87,31+starty+(i*33),107);
   if (i<=4 && AI[other(Win)]==0) OutBox(starty+(i*33),132,31+starty+(i*33),152);
   fseek(fin,AImg[i]*570,SEEK_SET);
   fread((char *)local.vptr,570,1,fin);
   if (i==0) gxPutImage(&local,gxSET,153,42,0);
    else
     {
      if (AI[Win]==0) gxPutImage(&local,gxSET,starty+((i-1)*33)+1,88,0);
      if (AI[other(Win)]==0) gxPutImage(&local,gxSET,starty+((i-1)*33)+1,133,0);
     }
  }
 fclose(fin);
 DV(&local);
 FadeIn(2,pal,10,0,0);
 
 return;
}

void Stat(char Win)
{
 int j,i,starty;
 DrawStatistics(Win);
 WaitForMouseUp();
 i=0;key=0;starty=118;
  while (i==0)
  {
	key=0;GetMouse();
	if ((x>=193 && y>=42 && x<=278 && y<=60 && mousebuttons>0) || key==K_ENTER)
	  {
	   InBox(193,42,278,60);
	   WaitForMouseUp();
      if (key>0) delay(150);
	   i=1;key=0;
	   OutBox(193,42,278,60);
	  }
   else
   if ((x>=152 && y>=41 && x<=183 && y<=61 && mousebuttons>0) || key=='D')
	  {
	   InBox(152,41,183,61);
	   WaitForMouseUp();
      if (key>0) delay(150);
      RankMe(Win);DrawStatistics(Win);
	   key=0;i=0;
	   OutBox(152,41,183,61);
	  };
   for (j=0;j<5;j++)
   {
    if (AI[Win]==0)
     {
       if ((x>=starty+(j*33) && y>=87 && x<=31+starty+(j*33) && y<=107 && mousebuttons>0) || ((key>='1' && key<='4') || key=='0'))
       {
        if (key>0)
         {
          switch(key)
           {
            case '0':j=0;break;
            case '1':j=1;break;
            case '2':j=2;break;
            case '3':j=3;break;
            case '4':j=4;break;
            default:break;
           }
         }
        InBox(starty+(j*33),87,31+starty+(j*33),107);
	     WaitForMouseUp();
        key=0;strncpy(IKEY,"k999",4);
        switch(j)
         {
	        case 0: strncpy(IDT,"i130",4);strncpy(IKEY,"k031",4);
                  if (Option==-1 || (Option==Win)) ShowSpHist(Win);
                  //ShowSpHist(Win);
                  break;
	        case 1: strncpy(IDT,"i131",4);strncpy(IKEY,"k321",4);
                Records(Win);break;
	        case 2: strncpy(IDT,"i132",4);strncpy(IKEY,"k033",4);
                ShowPrest(Win);break;
	        case 3: strncpy(IDT,"i034",4);ShowHard(Win);break;
	        case 4: strncpy(IDT,(Win==0)? "i133" :"i134",4);
                strncpy(IKEY,(Win==0)? "k035" :"k441",4);
                if (Data->P[Win].AstroCount>0)
                 if (Option==-1 || Option==Win) ShowAstrosHist(Win);
          default:break;
         }
        strncpy(IDT,"i000",4);strncpy(IKEY,"k000",4);
        DrawStatistics(Win);
	     key=0;i=0;
	     OutBox(starty+(j*33),87,31+starty+(j*33),107);
       }
      } // matches AI[Win]==0
     }
    for (j=0;j<5;j++)
     {
     if (AI[other(Win)]==0)
      {
       if ((x>=starty+(j*33) && y>=132 && x<=31+starty+(j*33) && y<=152 && mousebuttons>0) || (key>='5' && key<='9'))
        {
         if (key>0)
          {
           switch(key)
            {
             case '5':j=0;break;
             case '6':j=1;break;
             case '7':j=2;break;
             case '8':j=3;break;
             case '9':j=4;break;
             default:break;
            }
          }
         InBox(starty+(j*33),132,31+starty+(j*33),152);
	      WaitForMouseUp();
         key=0;
         switch(j)
          {
	        case 0:strncpy(IDT,"i130",4);strncpy(IKEY,"k031",4);
                 if (Option==-1 || Option==other(Win)) ShowSpHist(other(Win));
                 //ShowSpHist(other(Win));
                 break;
	        case 1:strncpy(IDT,"i131",4);strncpy(IKEY,"k321",4);
                 Records(other(Win));break;
	        case 2:strncpy(IDT,"i132",4);strncpy(IKEY,"k033",4);
                 ShowPrest(other(Win));break;
	        case 3:strncpy(IDT,"i034",4);strncpy(IKEY,"k999",4);
                 ShowHard(other(Win));break;
	        case 4:strncpy(IDT,(Win==0)?"i133":"i134",4);
                 strncpy(IKEY,(Win==0)?"k035":"k441",4);
                 if (Data->P[other(Win)].AstroCount>0)
                  if (Option==-1 || Option==other(Win))
                   ShowAstrosHist(other(Win));break;
           default:break;
          }
         strncpy(IDT,"i000",4);strncpy(IKEY,"k000",4);
         DrawStatistics(Win);
	      key=0;i=0;
	      OutBox(starty+(j*33),132,31+starty+(j*33),152);
        }
      }
    }
  }  
 return;
}

/** AI Wants to purchase Astronauts
 */
void AIAstroPur(char plr)
{
	int cost;
	int astrosInPool = 0;

	if (Data->P[plr].AstroLevel==0) 
		cost=20;
	else 
		cost=15;

	// Player has no cash, no astronauts
	if (cost>Data->P[plr].Cash) 
		return;

	switch (Data->P[plr].AstroLevel) {
		case 0:astrosInPool=ASTRO_POOL_LVL1;break;
		case 1:astrosInPool=ASTRO_POOL_LVL2;break;
		case 2:astrosInPool=ASTRO_POOL_LVL3;break;
		case 3:astrosInPool=ASTRO_POOL_LVL4;break;
		case 4:astrosInPool=ASTRO_POOL_LVL5;break;
		default:break;
	};
	// Select best astronauts out of number of positions to fill
	SelectBest(plr,astrosInPool);
	return;
}


//Naut Randomize, Nikakd, 10/8/10
void AIRandomizeNauts() {
	for (int i=0;i<106;i++) {
		Men[i].Cap = random(5);
		Men[i].LM  = random(5);
		Men[i].EVA = random(5);
		Men[i].Docking = random(5);
		Men[i].Endurance = random(5);
		}
}


/** Select the best crew for the mission
 */
void SelectBest(char plr,int pos)
{
 int count=0,now,MaxMen = 0,Index,AIMaxSel=0,i,j,k;
 FILE *fin;
 char tot,done;

 for (i=0;i<25;i++) AIsel[i]=0;

 memset(buffer,0x00,5000);
 Men=(struct ManPool *)buffer;
 fin = sOpen("CREW.DAT","rb",0);   
 fseek(fin,((sizeof (struct ManPool))*106)*plr,SEEK_SET);
 fread(Men,(sizeof (struct ManPool)*106),1,fin); 
 fclose(fin);
 if (options.feat_random_nauts==1) AIRandomizeNauts();   //Naut Randomize, Nikakd, 10/8/10
 switch(Data->P[plr].AstroLevel)
  {
	case 0:MaxMen=10;AIMaxSel=ASTRO_POOL_LVL1;Index=0;
		    if (Data->P[plr].Female==1) MaxMen+=3;break;
	case 1:MaxMen=17;AIMaxSel=ASTRO_POOL_LVL2;Index=14;
		    if (Data->P[plr].Female==1) MaxMen+=3;break;
	case 2:MaxMen=19;AIMaxSel=ASTRO_POOL_LVL3;Index=35;
		    if (Data->P[plr].Female==1) MaxMen+=3;break;
	case 3:MaxMen=27;AIMaxSel=ASTRO_POOL_LVL4;Index=58;break;
	case 4:MaxMen=19;AIMaxSel=ASTRO_POOL_LVL5;Index=86;break;
	default: MaxMen = 0; AIMaxSel = 0; Index = 0;break;
  }; 
  now=Index;count=0;done=0;
  for (i=16;i>0;i--)
   {
    done=0;
	 while (count<=AIMaxSel && done==0)
	  {
	   for (j=now;j<now+MaxMen+1;j++)
		 {
		  tot=0;
		  tot= Men[j].Cap + Men[j].LM + Men[j].EVA + Men[j].Docking;
		  if (i==tot) AIsel[count++]=j;
         else
         if (Data->P[plr].Female==1 && Men[j].Sex==1) {
           AIsel[count++]=j;
          }
		 }
	   done=1;
	  }
   };
    for (i=0;i<AIMaxSel;i++)
	  {
	   strcpy(&Data->P[plr].Pool[i+Data->P[plr].AstroCount].Name[0],&Men[AIsel[i]].Name[0]);
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Sex=Men[AIsel[i]].Sex;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Cap=Men[AIsel[i]].Cap;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].LM=Men[AIsel[i]].LM;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].EVA=Men[AIsel[i]].EVA;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Docking=Men[AIsel[i]].Docking;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Endurance=Men[AIsel[i]].Endurance;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Status=AST_ST_ACTIVE;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].oldAssign=-1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].TrainingLevel=1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Group=Data->P[plr].AstroLevel;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].CR=random(2)+1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].CL=random(2)+1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Task=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Crew=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Una=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Pool=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Compat=random(options.feat_compat_nauts)+1;  //Naut Compatibility, Nikakd, 10/8/10
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Mood=100;
      Data->P[plr].Pool[i+Data->P[plr].AstroCount].Face=random(77);
	   if (Data->P[plr].Pool[i+Data->P[plr].AstroCount].Sex==1) 
	     Data->P[plr].Pool[i+Data->P[plr].AstroCount].Face=77+random(8);    
	   k=random(10)+1;
	  }; 
   if (k)EMPTY_BODY;
	Data->P[plr].AstroLevel++;
	Data->P[plr].AstroCount=Data->P[plr].AstroCount+pos;
	switch(Data->P[plr].AstroLevel)
	  {
	   case 1:Data->P[plr].AstroDelay=6;break;
	   case 2: case 3: Data->P[plr].AstroDelay=4;break;
	   case 4:Data->P[plr].AstroDelay=8;break;
	   case 5: Data->P[plr].AstroDelay=99;break;
	   default:break;
	  };
   // remove from the bottom up out of training
   for(i=0;i<Data->P[plr].AstroCount;i++)
	 if (Data->P[plr].Pool[i].Status==AST_ST_TRAIN_BASIC_1)
         Data->P[plr].Pool[i].Status=AST_ST_ACTIVE;
 return;
}

void DumpAstro(char plr,int inx)
{
 int i,j;
 for (i=0;i<Data->P[plr].AstroCount;i++)
	if (Data->P[plr].Pool[i].Assign==inx && Data->P[plr].Pool[i].Prime<1)
	  {
	   Data->P[plr].Pool[i].Assign=0; // back to limbo
	   Data->P[plr].Pool[i].Una=0;
	  }
 for (i=0;i<8;i++)
	{
	 for (j=0;j<4;j++)
	   if (Data->P[plr].Pool[Data->P[plr].Crew[inx][i][j]-1].Prime<1)
		 {
		  Data->P[plr].Crew[inx][i][j]=0;
		  Data->P[plr].Gcnt[inx][i]=0;
		 }
	}
 return;
}

char Skill(char plr,char type)
{
 char m,hgh=0,tst,ind=0;
 for (m=0;m<Data->P[plr].AstroCount;m++)
   {
	if (Data->P[plr].Pool[m].Status==AST_ST_ACTIVE && Data->P[plr].Pool[m].Assign==0 && Data->P[plr].Pool[m].Prime<1)
	  {
	   tst=0;
	   switch(type)
		{
		 case 1:tst=Data->P[plr].Pool[m].Cap+Data->P[plr].Pool[m].EVA;break;
		 case 2:tst=Data->P[plr].Pool[m].LM+Data->P[plr].Pool[m].EVA;break;
		 case 3:tst=Data->P[plr].Pool[m].Docking;break;
		 case 4:tst=Data->P[plr].Pool[m].Cap;break;
		 case 5:tst=Data->P[plr].Pool[m].EVA;break;
		 default:break;
		}
	   if (tst>=hgh)
		 {
		  ind=m;
		  hgh=tst;
		 }
	  }
	}
 return(ind);
}

void TransAstro(char plr,int inx)
                                    // indexed 1 thru 5
{
 int i,j,w,count=0,max,found,flt1,flt2,bug=1;
 while (bug==1) { count=0;max=0;found=0;flt1=0;flt2=0;
 if (inx==5) max=4;
  else if (inx==4) max=3;
	else max=inx;
 if (Data->P[plr].AstroCount==0) return;
 for (i=0;i<Data->P[plr].AstroCount;i++)
    {
     Data->P[plr].Pool[i].Mood=100;
     if (Data->P[plr].Pool[i].Status==AST_ST_RETIRED || Data->P[plr].Pool[i].Status==AST_ST_DEAD)
      {
       Data->P[plr].Pool[i].Status=AST_ST_ACTIVE;
       Data->P[plr].Pool[i].Assign=0;
       Data->P[plr].Pool[i].Una=0;
       Data->P[plr].Pool[i].Prime=0;
       Data->P[plr].Pool[i].oldAssign=-1;
       Data->P[plr].Pool[i].Crew=0;
      }
     Data->P[plr].Pool[i].RetReas=0;
     Data->P[plr].Pool[i].Hero=0; //clear hero flag
   if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0 && Data->P[plr].Pool[i].Prime<1)
	 ++count;
    }
 if (count<max*2) {CheckAdv(plr);return;}
 found=0;flt1=-1;flt2=-1;
 for (i=0;i<8;i++) {
   if (Data->P[plr].Gcnt[inx][i]==0) {
	 if (flt1==-1) flt1=i;
	   else if (flt2==-1) flt2=i;
   }
 }
 if (flt1==-1 || flt2==-1) {CheckAdv(plr);return;}
 for (i=0;i<2;i++)
   {
	for (j=1;j<=max;j++)
	  {
	   w=0;found=0;
	   while (w < Data->P[plr].AstroCount && found==0)
		 {
		  if (Data->P[plr].Pool[w].Status==AST_ST_ACTIVE && Data->P[plr].Pool[w].Assign==0 && Data->P[plr].Pool[w].Prime<1)
			{
			 // based on [j] an program and position pick best skill
			 switch(inx)
			  {
			   case 1:w=Skill(plr,1);
					  break;
			   case 2:if (j==1) w=Skill(plr,1);
						else w=Skill(plr,2);
					  break;
			   case 3:if (j==1) w=Skill(plr,1);
						else if (j==2) w=Skill(plr,2);
						  else w=Skill(plr,3);
					  break;
			   case 4:if (j==1) w=Skill(plr,4);
						else if (j==2) w=Skill(plr,2);
						  else w=Skill(plr,3);
					  break;
			   case 5:if (j==1) w=Skill(plr,4);
						else if (j==2) w=Skill(plr,4);
						  else if (j==3) w=Skill(plr,5);
						  else w=Skill(plr,5);break;
			   default:break;
			  }
			 if (i==0)
			   {
				Data->P[plr].Pool[w].Assign=inx;
				Data->P[plr].Pool[w].Una=1;
				Data->P[plr].Crew[inx][flt1][j-1]=w+1;
				found=1;
			   }
			 else
			   {
				Data->P[plr].Pool[w].Assign=inx;
				Data->P[plr].Pool[w].Una=1;
				Data->P[plr].Crew[inx][flt2][j-1]=w+1;
				found=1;
			   }
		    } // end if
		  ++w;
		 } // end while
	  }
	Data->P[plr].Gcnt[inx][flt1]=max;
	Data->P[plr].Gcnt[inx][flt2]=max;
   }
 } // end while
 return;
}

void CheckAdv(char plr)
{
 int i,count;
 count=0;
 for (i=0;i<Data->P[plr].AstroCount;i++)
   if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0)
	 ++count;
 if (count<=3)
   {
	for (i=0;i<Data->P[plr].AstroCount;i++)
	   if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0)
	 {
	  Data->P[plr].Pool[i].Focus=random(4)+1;
	  if (Data->P[plr].Pool[i].Focus>0)
	   {
		Data->P[plr].Cash-=3;
		Data->P[plr].Pool[i].Assign=0;
		Data->P[plr].Pool[i].Status=AST_ST_TRAIN_ADV_1;
	   }
	 }
   }
 return;
}


#ifdef DEAD_CODE
/** Remove unhappy astro's
 */
void RemoveUnhappy(char plr)
{
	int i,l,astroClass=0,fltCrew=0;
    int foundClass = 0, foundCrew = 0;

	for (i=0;i<Data->P[plr].AstroCount;i++)
	{
		if (Data->P[plr].Pool[i].Mood < ASTRONAUT_MOOD_THRESHOLD)
			if (Data->P[plr].Pool[i].Assign!=0 && Data->P[plr].Pool[i].Status==AST_ST_ACTIVE)
			{
				Data->P[plr].Pool[i].Assign=0; // back to limbo
				Data->P[plr].Pool[i].Una=0;

                // find the crew astronaut is in
				for (astroClass=0; astroClass<ASTRONAUT_POOLS; astroClass++)
				{
					for (fltCrew=0; fltCrew<ASTRONAUT_CREW_MAX; fltCrew++)
					{
						for (l=0; l<ASTRONAUT_FLT_CREW_MAX; l++)
						{
							if (Data->P[plr].Crew[astroClass][fltCrew][l]==i) 
                            {
                                foundClass = astroClass;
                                foundCrew = fltCrew;
								goto endLoop;
                            }
						}
					}
				}

            endLoop:
				for (l=0; l<ASTRONAUT_FLT_CREW_MAX; l++)
				{
					Data->P[plr].Pool[Data->P[plr].Crew[foundClass][foundCrew][l]].Assign=0;
					Data->P[plr].Pool[Data->P[plr].Crew[foundClass][foundCrew][l]].Una=0;
					Data->P[plr].Crew[foundClass][foundCrew][l]=0;
				}
			}
	}
	return;
}
#endif

void RDafford(char plr,int class,int index)
{
 i16 b=0,roll=0,ok=0;
 if (class==0) b+=Data->P[plr].Probe[index].RDCost;
 if (class==1) b+=Data->P[plr].Rocket[index].RDCost;
 if (class==2) b+=Data->P[plr].Manned[index].RDCost;
 if (class==3) b+=Data->P[plr].Misc[index].RDCost;

 if (class==0) roll+=Data->P[plr].Probe[index].MaxRD - Data->P[plr].Probe[index].Safety;
 if (class==1) roll+=Data->P[plr].Rocket[index].MaxRD - Data->P[plr].Rocket[index].Safety;
 if (class==2) roll+=Data->P[plr].Manned[index].MaxRD - Data->P[plr].Manned[index].Safety;
 if (class==3) roll+=Data->P[plr].Misc[index].MaxRD - Data->P[plr].Misc[index].Safety;

 roll= (roll*10) / 35; ok=0;

 if (roll>5) roll=5;
 if (Data->P[plr].Buy[class][index]>0) return;
   while (ok==0 && roll!=0)
	 {
	  if ( (b*roll <= Data->P[plr].Cash) && QueryUnit(class,index,plr)==1
		&& MaxChk(class+1,index+1,plr))
		{
		 Data->P[plr].Buy[class][index]=RDUnit(class+1,index+1,roll,plr);
		 Data->P[plr].Cash= (Data->P[plr].Cash - (b*roll));
		 ok=1; // jump out of the loop
		}
	   else roll-=1;
	 }
 return;
}

#ifdef DEAD_CODE
int CheckMax(char plr,int m)
{
 switch(m)
  {
   case 1: if (Data->P[plr].Probe[0].Num<1 || Data->P[plr].Probe[0].Safety<90) return(1);
			 else return(0);
   case 2: if (Data->P[plr].Probe[1].Num<1 || Data->P[plr].Probe[1].Safety<90) return(1);
			 else return(0);
   case 3: if (Data->P[plr].Probe[2].Num<1 || Data->P[plr].Probe[2].Safety<90) return(1);
			 else return(0);
   case 4: if (Data->P[plr].Manned[0].Num<1 || Data->P[plr].Manned[0].Safety<90) return(1);
			 else return(0);
   case 5: if (Data->P[plr].Manned[1].Num<1 || Data->P[plr].Manned[1].Safety<90) return(1);
			 else return(0);
   default:return 0;
  }
}
#endif

#ifdef DEAD_CODE
char SF(char plr,char m)
{
 char num=0;
 switch(m)
  {
   case 1:num=Data->P[plr].Probe[0].Safety;break;
   case 2:num=Data->P[plr].Probe[1].Safety;break;
   case 3:num=Data->P[plr].Probe[2].Safety;break;
   case 4:num=Data->P[plr].Manned[0].Safety;break;
   case 5:num=Data->P[plr].Manned[1].Safety;break;
   default:break;
  }
 return(num);
}
#endif

#ifdef DEAD_CODE
char SN(char plr,char m)
{
 char num=0;
 switch(m)
  {
   case 1:num=Data->P[plr].Probe[0].Num;break;
   case 2:num=Data->P[plr].Probe[1].Num;break;
   case 3:num=Data->P[plr].Probe[2].Num;break;
   case 4:num=Data->P[plr].Manned[0].Num;break;
   case 5:num=Data->P[plr].Manned[1].Num;break;
   default:break;
  }
 return(num);
}
#endif

void AIPur(char plr)
{
 if (Data->P[plr].AIStat==0) Data->P[plr].AIStat=1;
 if (Data->P[plr].Track[0]==0) Data->P[plr].Track[0]=1;
 if (Data->P[plr].Probe[0].Num<=Data->P[plr].Rocket[0].Num)
  {
	if (GenPur(plr,0,0)) RDafford(plr,0,0);
	 else RDafford(plr,0,0);
	if (GenPur(plr,1,0)) RDafford(plr,1,0);
	 else RDafford(plr,1,0);
  }
  else
	{
	 if (GenPur(plr,1,0)) RDafford(plr,1,0);
	  else RDafford(plr,1,0);
	 if (GenPur(plr,0,0)) RDafford(plr,0,0);
	  else RDafford(plr,0,0);
	}
 return;
}

int GenPur(char plr,int hardware_index,int unit_index)
{
 char RT_value=0,newf,n1,n2,n3,n4,n5,n6,n7;

 newf=0; // reinitialize
 //special case DM before Kickers
 if (hardware_index==3 && unit_index<=1 && Data->P[plr].Misc[4].Num == PROGRAM_NOT_STARTED)
  {
   hardware_index=3;unit_index=4;
  };
 switch(hardware_index)
 {
  case 0: if (Data->P[plr].Probe[unit_index].Num<2)
		{ // Probe Programs
		 if (Data->P[plr].Probe[unit_index].Num == PROGRAM_NOT_STARTED)
		   {
			if (Data->P[plr].Probe[unit_index].InitCost < Data->P[plr].Cash)
			  {
			   Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Probe[unit_index].InitCost;
			   Data->P[plr].Probe[unit_index].Num = 1;
			   RT_value=1;newf=1;
			  }
		   }
		 else
			{
			 if (Data->P[plr].Probe[unit_index].UnitCost < Data->P[plr].Cash)
			   {
				Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Probe[unit_index].UnitCost;
				Data->P[plr].Probe[unit_index].Num = Data->P[plr].Probe[unit_index].Num + 1;
				RT_value=1;
			   }
          else {
             RT_value=1;++Data->P[plr].Probe[unit_index].Num;
            }
			}
	   }; // end case 1
	 break;
  case 1:if (Data->P[plr].Rocket[unit_index].Num<2)
		    { // Rocket Programs Purchasing
	        if (Data->P[plr].Rocket[unit_index].Num == PROGRAM_NOT_STARTED)
		      {
		       if (Data->P[plr].Rocket[unit_index].InitCost < Data->P[plr].Cash)
		        { 
		         Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Rocket[unit_index].InitCost;
		         if (Data->P[plr].Rocket[unit_index].Num == PROGRAM_NOT_STARTED) Data->P[plr].Rocket[unit_index].Num=1;
		          else ++Data->P[plr].Rocket[unit_index].Num;
		         RT_value=1;newf=1;
		        }
		      }
	    	  else
		      {
		       if (Data->P[plr].Rocket[unit_index].Num==1 && (Data->P[plr].Rocket[unit_index].Safety<Data->P[plr].Rocket[unit_index].MaxRD-15))
			     {
			      RDafford(plr,1,unit_index);
               Data->P[plr].Buy[1][unit_index]=0;
			      RT_value=1;
			     }
             else
		       if (Data->P[plr].Rocket[unit_index].Num>=0)
			     {
			      if (Data->P[plr].Rocket[unit_index].UnitCost < Data->P[plr].Cash)
			       {
				     Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Rocket[unit_index].UnitCost;
				     ++Data->P[plr].Rocket[unit_index].Num;
				     RT_value=1;
			       }
               else
                { 
                 RT_value=1;++Data->P[plr].Rocket[unit_index].Num;
                }
			     }
		      }
	   }; // end case 2
	 break;
   case 2:if (unit_index==3 && Data->P[plr].Manned[3].Num==1) return(1);
			else
			  if (Data->P[plr].Manned[unit_index].Num<2)
				{ // Manned Programs
				 if (Data->P[plr].Manned[unit_index].Num == PROGRAM_NOT_STARTED)
				   {
					if (Data->P[plr].Manned[unit_index].InitCost < Data->P[plr].Cash)
					  {
					   Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Manned[unit_index].InitCost;
					   if (Data->P[plr].Manned[unit_index].Num == PROGRAM_NOT_STARTED) Data->P[plr].Manned[unit_index].Num=1;
						 else ++Data->P[plr].Manned[unit_index].Num;
					   RT_value=1;newf=1;
					  }
				   }
				 else
				  {
				   if (Data->P[plr].Manned[unit_index].Num==1 && (Data->P[plr].Manned[unit_index].Safety<Data->P[plr].Manned[unit_index].MaxRD-15))
					 {
					  RDafford(plr,2,unit_index);
                 Data->P[plr].Buy[2][unit_index]=0;
					  RT_value=1;
					 }
              else
				  if (Data->P[plr].Manned[unit_index].Num>=0)
					{
					 if (Data->P[plr].Manned[unit_index].UnitCost < Data->P[plr].Cash)
					   {
						Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Manned[unit_index].UnitCost;
						++Data->P[plr].Manned[unit_index].Num;
						RT_value=1;
					   }
                else {
                  RT_value=1;++Data->P[plr].Manned[unit_index].Num;
                  }
					}
				 }
	   }; // end case 3
	 break;
   case 3: if (Data->P[plr].Misc[unit_index].Num<2) { // Misc Programs
		  if (unit_index==3 && Data->P[plr].Misc[unit_index].Num==1) return(1);
		if (Data->P[plr].Misc[unit_index].Num == PROGRAM_NOT_STARTED)
		  {
		   if (Data->P[plr].Misc[unit_index].InitCost < Data->P[plr].Cash)
			 {
			  Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Misc[unit_index].InitCost;
			  if (Data->P[plr].Misc[unit_index].Num == PROGRAM_NOT_STARTED) Data->P[plr].Misc[unit_index].Num=1;
			else ++Data->P[plr].Misc[unit_index].Num;
			 RT_value=1;newf=1;
			 }
		 }
		else
		  if (Data->P[plr].Misc[unit_index].Num>=0)
			{
			 if (Data->P[plr].Misc[unit_index].UnitCost < Data->P[plr].Cash)
			   {
				Data->P[plr].Cash = Data->P[plr].Cash - Data->P[plr].Misc[unit_index].UnitCost;
				++Data->P[plr].Misc[unit_index].Num;
				RT_value=1;
			   }
          else {
            RT_value=1;++Data->P[plr].Misc[unit_index].Num;
            }
		   }
	   }; // end case 4
	 break;
  default:break;
   } // end switch

  // starting bonuses and cost bonuses
  if (hardware_index==0 && newf==1) {
	n1=Data->P[plr].Probe[0].Safety;
	n2=Data->P[plr].Probe[1].Safety;
	n3=Data->P[plr].Probe[2].Safety;
	switch(unit_index) {
	  case 0: if (n2>=75) Data->P[plr].Probe[0].Safety=50;
		  if (n3>=75) Data->P[plr].Probe[0].Safety=60;
		  break;
	  case 1: if (n1>=75) Data->P[plr].Probe[1].Safety=45;
		  if (n3>=75) Data->P[plr].Probe[1].Safety=50;
		  break;
	  case 2: if (n1>=75) Data->P[plr].Probe[2].Safety=45;
		  if (n2>=75) Data->P[plr].Probe[2].Safety=50;
		  break;
	};
	Data->P[plr].Probe[unit_index].Base=Data->P[plr].Probe[unit_index].Safety;
  };
  if (hardware_index==1 && newf==1) {
	n1=Data->P[plr].Rocket[0].Safety; /* One - A     */
	n2=Data->P[plr].Rocket[1].Safety; /* Two - B     */
	n3=Data->P[plr].Rocket[2].Safety; /* Three - C   */
	n4=Data->P[plr].Rocket[3].Safety; /* Mega - G    */
	n5=Data->P[plr].Rocket[4].Safety; /* Booster - D */
	switch(unit_index) {
      case 0: if (n2>=75 || n3>=75 || n4>=75 || n5>=75)
		Data->P[plr].Rocket[0].Safety=35;
		  break;
      case 1: if (n1>=75 || n5>=75) Data->P[plr].Rocket[1].Safety=25;
	      if (n3>=75 || n4>=75) Data->P[plr].Rocket[1].Safety=40;
	      if ((n1>=75 || n5>=75) && (n3>=75 || n4>=75))
		Data->P[plr].Rocket[1].Safety=65;
	      break;
	  case 2: if (n1>=75 || n5>=75) Data->P[plr].Rocket[2].Safety=15;
	      if (n2>=75 || n4>=75) Data->P[plr].Rocket[2].Safety=35;
	      if ((n1>=75 || n5>=75) && (n2>=75 || n4>=75))
		Data->P[plr].Rocket[2].Safety=60;
	      break;
      case 3: if (n1>=75 || n5>=75) Data->P[plr].Rocket[3].Safety=10;
	      if (n2>=75 || n3>=75) Data->P[plr].Rocket[3].Safety=25;
	      if ((n1>=75 || n5>=75) && (n2>=75 || n3>=75))
		Data->P[plr].Rocket[3].Safety=35;
	      break;
      case 4: if (n1>=75 || n2>=75 || n3>=75 || n4>=75)
		Data->P[plr].Rocket[4].Safety=30;
	      break;
    };
	Data->P[plr].Rocket[unit_index].Base=Data->P[plr].Rocket[unit_index].Safety;
  };
  if (hardware_index==2 && newf==1) {
	n1=Data->P[plr].Manned[0].Safety; /* One - a         */
	n2=Data->P[plr].Manned[1].Safety; /* Two - b         */
	n3=Data->P[plr].Manned[2].Safety; /* Three - c       */
	n4=Data->P[plr].Manned[3].Safety; /* Minishuttle - f */
	n5=Data->P[plr].Manned[4].Safety; /* cap/mod - h     */
	n6=Data->P[plr].Manned[5].Safety; /* 2 mod - d       */
	n7=Data->P[plr].Manned[6].Safety; /* 1 mod - e       */
	switch(unit_index) {
	  case 0: if (n2>=75 || n3>=75 || n5>=75)
		Data->P[plr].Manned[0].Safety=40;
		  break;
	  case 1: if (n1>=75) Data->P[plr].Manned[1].Safety=20;
		  if (n3>=75 || n5>=75) Data->P[plr].Manned[1].Safety=40;
		  break;
	  case 2: if (n1>=75 || n5>=75) Data->P[plr].Manned[2].Safety=20;
		  if (n2>=75 || n4>=75) Data->P[plr].Manned[2].Safety=30;
		  if ((n1>=75 || n5>=75) && (n2>=75 || n4>=75))
		Data->P[plr].Manned[2].Safety=40;
		  break;
	  case 3: break;
	  case 4: if (n1>=75) Data->P[plr].Manned[4].Safety=10;
		  if (n2>=75) Data->P[plr].Manned[4].Safety=15;
		  if (n3>=75) Data->P[plr].Manned[4].Safety=25;
		  if ((n1>=75 || n2>=75 || n3>=75) && (n6>=75 || n7>=75))
		 Data->P[plr].Manned[4].Safety=35;
		  break;
	  case 5: if (n7>=75) Data->P[plr].Manned[5].Safety=30;
		  if (n5>=75) Data->P[plr].Manned[5].Safety=40;
		  break;
	  case 6: if (n6>=75) Data->P[plr].Manned[6].Safety=30;
		  if (n5>=75) Data->P[plr].Manned[6].Safety=40;
		  break;
	};
	Data->P[plr].Manned[unit_index].Base=Data->P[plr].Manned[unit_index].Safety;

  };

  if (hardware_index==3 && newf==1) {
	n1=Data->P[plr].Rocket[0].Safety; /* One - A     */
	n2=Data->P[plr].Rocket[1].Safety; /* Two - B     */
	n3=Data->P[plr].Rocket[2].Safety; /* Three - C   */
	n4=Data->P[plr].Rocket[3].Safety; /* Mega - G    */
	n5=Data->P[plr].Rocket[4].Safety; /* Booster - D */
	switch(unit_index) {
	  case 0: if (n2>=75)
		Data->P[plr].Misc[0].Safety=40;
		  break;
	  case 1: if (n1>=75) Data->P[plr].Misc[1].Safety=35;
		  break;
	  case 2: if (n1>=75 || n2>=75) Data->P[plr].Misc[2].Safety=25;
		  break;
	  default: break;
	};
	Data->P[plr].Misc[unit_index].Base=Data->P[plr].Misc[unit_index].Safety;

  };
 return(RT_value);
}



/* EOF */
