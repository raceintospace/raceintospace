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
#include <Buzz_inc.h>
#include <externs.h>

char Month[12][11] = {
    "JANUARY ","FEBRUARY ","MARCH ","APRIL ","MAY ","JUNE ",
    "JULY ","AUGUST ","SEPTEMBER ","OCTOBER ","NOVEMBER ","DECEMBER "};

struct order Order[7] ;
unsigned int colss,VBlank;
extern char AI[2];
extern struct mStr Mis;
extern char dg[62][6];
extern char Option,MAIL,BIG;

/** Function to compare two Missions
 *
 * Orders by date and budget
 *
 * \param
 * \param
 *
 * \return -1 if first date earlier than second
 * \return 0 if date and budget are the same
 * \return 1 if first budget is lower than second
 */
static int
cmp_order(const void *p1, const void *p2)
{
    struct order *o1 = (struct order *) p1;
    struct order *o2 = (struct order *) p2;

    if (o1->date < o2->date)
        return -1;
    else if (o1->date == o2->date)
    {
        if (o1->budget > o2->budget)
            return -1;
        else if  (o1->budget == o2->budget)
            {
            	char whoFirst = random(2);
            	if (whoFirst==1) return 1;
            	else return -1;
            }
            else return 1;
    }
    else return 1;
}

char
OrderMissions(void)
{
	int i, j, k;

	memset(Order, 0x00, sizeof Order);
	// Sort Missions for Proper Order
	k = 0;
	for (i = 0; i < NUM_PLAYERS; i++)
		for (j = 0; j < MAX_MISSIONS; j++)
			if (Data->P[i].Mission[j].MissionCode > 0
				&& Data->P[i].Mission[j].part != 1)
			{
				Order[k].plr = i;
				Order[k].loc = j;
				Order[k].budget = Data->P[i].Budget;
				Order[k].date = Data->P[i].Mission[j].Month;
				k++;
			};
    if (k)
        qsort(Order, k, sizeof(struct order), cmp_order);

	if (MAIL == -1 && Option == -1 && AI[0] == 0 && AI[1] == 0 && k != 0)
		MisOrd(k);
	return k;
}

void MisOrd(char num)
{
  int i,j=0;

  ShBox(63,19,257,173);
  InBox(74,36,246,163);
  grSetColor(36);PrintAt(77,30,"       LAUNCH ORDER");

  for (i=0;i<num;i++) {
    InBox(78,39+21*j,105,55+21*j);
    FlagSm(Order[i].plr,79,40+21*j);
    grSetColor(34);
    PrintAt(110,45+21*j,"SCHEDULED LAUNCH");
	PrintAt(110,52+21*j,"DATE: ");
    grSetColor(1);

    PrintAt(0,0,
      Month[Data->P[Order[i].plr].Mission[Order[i].loc].Month]);

    PrintAt(0,0," 19");DispNum(0,0,Data->Year);
    j++;
  };
  FadeIn(2,pal,10,0,0);

  WaitForMouseUp();
	WaitForKeyOrMouseDown();
	WaitForMouseUp();
	FadeOut(2,pal,10,0,0);
}


void MisAnn(char plr,char pad)
{
  int i,j,bud;
  struct mStr Mis2;
  char k,hold,Digit[4],HelpFlag=0;
  char pad_str[2] = {'A'+pad, '\0'};

  for (i=0;i<768;i++) pal[i]=0;
  gxSetDisplayPalette(pal);
  gxClearDisplay(0,0);

  PortPal(plr);
  ShBox(41,20,281,184);InBox(46,25,276,179);
  InBox(46,25,117,65);Flag(47,26,plr);
  InBox(122,25,276,65);
  grSetColor(9);
  PrintAt(127,33,"SCHEDULED LAUNCH");  //was 154,33
  grSetColor(34);
  PrintAt(127,40,"LAUNCH FACILITY: ");
  grSetColor(1);
  PrintAt(0,0,"PAD ");
  PrintAt(0,0,pad_str);
  grSetColor(34);
  PrintAt(127,47,"DATE: ");
  grSetColor(1);

  PrintAt(0,0,Month[Data->P[plr].Mission[pad].Month]);

  PrintAt(0,0,"19");DispNum(0,0,Data->Year);
  grSetColor(1);

  GetMisType(Data->P[plr].Mission[pad].MissionCode);
  memcpy(&Mis2,&Mis,sizeof Mis);


  if ((Mis.mVab[0]&0x10)==0x10 && Data->P[plr].DMod<=0) {
     i=0;
     while (dg[Mis2.Index][i]!=0 && Mis.Doc==1) {
        GetMisType(dg[Mis2.Index][i]);
        i++;
     }
     if (dg[Mis2.Index][i]==0) Data->P[plr].Mission[pad].MissionCode=4;  // new mission
     else Data->P[plr].Mission[pad].MissionCode=Mis.Index;  // new mission
     GetMisType(Data->P[plr].Mission[pad].MissionCode);
     HelpFlag=1;
  }

  PrintAt(127,54,Mis.Abbr);
      int MisCod;
      MisCod=Data->P[plr].Mission[pad].MissionCode;
      if ((MisCod>24 && MisCod<32) || MisCod==33 || MisCod==34 || MisCod==35 || MisCod==37 || MisCod==40 || MisCod==41)
	// Show duration level only on missions with a Duration step - Leon
       {
	switch(Data->P[plr].Mission[pad].Duration)
	{
	 case 1:PrintAt(0,0,"");break;
	 case 2:PrintAt(0,0," (B)");break;
	 case 3:PrintAt(0,0," (C)");break;
	 case 4:PrintAt(0,0," (D)");break;
	 case 5:PrintAt(0,0," (E)");break;
	 case 6:PrintAt(0,0," (F)");break;
	 default:PrintAt(0,0,"");break;
	}
       }

  IOBox(57,68,118,84);IOBox(131,68,197,84);IOBox(205,68,266,84);
  grSetColor(1);PrintAt(65,78,"CONTINUE");PrintAt(139,78,"PLAY FULL");
  PrintAt(221,78,"SCRUB");
  grSetColor(9);MisCod=Data->P[plr].Mission[i].MissionCode;
  PrintAt(65,78,"C");PrintAt(139,78,"P");PrintAt(221,78,"S");

  //IOBox(85,68,158,84);IOBox(172,68,245,84);
  //grSetColor(1);PrintAt(102,78,"CONTINUE");PrintAt(189,78,"SCRUB");
  //grSetColor(9);
  //PrintAt(102,78,"C");PrintAt(189,78,"S");

  if (Data->P[plr].Mission[pad].Joint==0) PrintAt(128,91,"SINGLE LAUNCH");
    else
     {
      PrintAt(131,91,"JOINT LAUNCH");
      PrintAt(49,101,"PART ONE");
      PrintAt(102,101,"PAD ");
      //DispNum(0,0,pad);
      switch(pad) {
	      case 0: PrintAt(0,0,"A");break;
	      case 1: PrintAt(0,0,"B");break;
	      case 2: PrintAt(0,0,"C");break;
      };

      PrintAt(160,101,"PART TWO");
      PrintAt(213,101,"PAD ");
      //DispNum(0,0,pad+1);
      switch(pad+1) {
	      case 0: PrintAt(0,0,"A");break;
	      case 1: PrintAt(0,0,"B");break;
	      case 2: PrintAt(0,0,"C");break;
      };

     }
for (i=0;i<Data->P[plr].Mission[pad].Joint+1;i++)
  {
  k=0;if (i==0) bud=49; else bud=160;
  for(j=Mission_Capsule; j <= Mission_PrimaryBooster; j++)
   {
    hold = Data->P[plr].Mission[pad+i].Hard[j];
    switch(j)
     {
      case 0:if (hold>-1)
	       {
		grSetColor(7);PrintAt(bud,109+14*k,"CAPSULE: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Manned[hold].Name[0]);
		grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
     Data->P[plr].Manned[hold].Damage != 0? grSetColor(9):grSetColor(1); //Damaged Equipment, Nikakd, 10/8/10
     sprintf(&Digit[0],"%d",Data->P[plr].Manned[hold].Safety+Data->P[plr].Manned[hold].Damage);
     PrintAt(0,0,&Digit[0]);
     PrintAt(0,0,"%");
	 //	PrintAt(144+i*111,116+14*k,"%");
		++k;
	       };
	     break;
      case 1:if (hold>-1)
	       {
		grSetColor(7);PrintAt(bud,109+14*k,"KICKER: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Misc[hold].Name[0]);
     grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
     Data->P[plr].Misc[hold].Damage != 0? grSetColor(9):grSetColor(1);  //Damaged Equipment, Nikakd, 10/8/10
     sprintf(&Digit[0],"%d",Data->P[plr].Misc[hold].Safety+Data->P[plr].Misc[hold].Damage);
     PrintAt(0,0,&Digit[0]);
     PrintAt(0,0,"%");
    // DispNum(0,0,Data->P[plr].Misc[hold].Safety);
	  //	PrintAt(144+i*111,116+14*k,"%");
		++k;
	       };
	      break;
       case 2:if (hold>-1)
		{
		 grSetColor(7);PrintAt(bud,109+14*k,"LM: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Manned[hold].Name[0]);
		 grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
      Data->P[plr].Manned[hold].Damage != 0? grSetColor(9):grSetColor(1);   //Damaged Equipment, Nikakd, 10/8/10
      sprintf(&Digit[0],"%d",Data->P[plr].Manned[hold].Safety+Data->P[plr].Manned[hold].Damage);
      PrintAt(0,0,&Digit[0]);
      PrintAt(0,0,"%");
      //DispNum(0,0,Data->P[plr].Manned[hold].Safety);
		 //PrintAt(144+i*111,116+14*k,"%");
		 ++k;
		};
	       break;
	case 3:if (hold>-1)
		 {
		  if (hold<3)
		    {
		     grSetColor(7);PrintAt(bud,109+14*k,"PROBE: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Probe[hold].Name[0]);
		     grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
          Data->P[plr].Probe[hold].Damage != 0? grSetColor(9):grSetColor(1);   //Damaged Equipment, Nikakd, 10/8/10
          sprintf(&Digit[0],"%d",Data->P[plr].Probe[hold].Safety+Data->P[plr].Probe[hold].Damage);
          PrintAt(0,0,&Digit[0]);
          PrintAt(0,0,"%");
          //DispNum(0,0,Data->P[plr].Probe[hold].Safety);
		     //PrintAt(144+i*111,116+14*k,"%");
		     ++k;
		    }
		   else if (hold==4)
		    {
		     grSetColor(7);PrintAt(bud,109+14*k,"DOCKING: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Misc[hold].Name[0]);
		     grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
          Data->P[plr].Misc[hold].Damage != 0? grSetColor(9):grSetColor(1);   //Damaged Equipment, Nikakd, 10/8/10
          sprintf(&Digit[0],"%d",Data->P[plr].Misc[hold].Safety+Data->P[plr].Misc[hold].Damage);
          PrintAt(0,0,&Digit[0]);
          PrintAt(0,0,"%");
          //DispNum(0,0,Data->P[plr].Misc[hold].Safety);
		     //PrintAt(144+i*111,116+14*k,"%");
		     ++k;
		    }
		 };
	       break;
	case 4:if (hold>-1)
		 {
		  if (hold<5)
		    {
		     grSetColor(7);PrintAt(bud,109+14*k,"ROCKET: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Rocket[hold-1].Name[0]);
		     grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
          Data->P[plr].Rocket[hold-1].Damage != 0? grSetColor(9):grSetColor(1);   //Damaged Equipment, Nikakd, 10/8/10
          sprintf(&Digit[0],"%d",Data->P[plr].Rocket[hold-1].Safety+Data->P[plr].Rocket[hold-1].Damage);
          PrintAt(0,0,&Digit[0]);
          PrintAt(0,0,"%");
          //DispNum(0,0,Data->P[plr].Rocket[hold-1].Safety);
		     //PrintAt(144+i*111,116+14*k,"%");
		     ++k;
		    }
		  else
		   {
		    grSetColor(7);PrintAt(bud,109+14*k,"ROCKET: ");grSetColor(1);PrintAt(0,0,&Data->P[plr].Rocket[hold-5].Name[0]);PrintAt(0,0," W/B");
		    grSetColor(11);PrintAt(bud,116+14*k,"SAFETY FACTOR: ");
         (Data->P[plr].Rocket[hold-5].Damage != 0 || Data->P[plr].Rocket[4].Damage !=0)? grSetColor(9):grSetColor(1);   //Damaged Equipment && Booster's Safety Mod, Nikakd, 10/8/10
         sprintf(&Digit[0],"%d",RocketBoosterSafety(Data->P[plr].Rocket[hold-5].Safety+Data->P[plr].Rocket[hold-5].Damage, Data->P[plr].Rocket[4].Safety+Data->P[plr].Rocket[4].Damage));
         PrintAt(0,0,&Digit[0]);
         PrintAt(0,0,"%");
        // DispNum(0,0,(Data->P[plr].Rocket[hold-5].Safety+Data->P[plr].Rocket[4].Safety)/2);
		   // PrintAt(144+i*111,116+14*k,"%");
		    ++k;
		   }
		 }
	       break;
	default:break;
    }
   }
  }
  FadeIn(2,pal,10,0,0);


  WaitForMouseUp();
  if (HelpFlag) Help("i156");  // Notification of being an Help

  while (1)
  {
	key=0;GetMouse();
	if ((x>=59 && y>=70 && x<=116 && y<=82 && mousebuttons > 0) || key==K_ENTER || key=='C')
	  {
	   InBox(59,70,116,82);
		WaitForMouseUp();
	   OutBox(59,70,116,82);
	   FadeOut(2,pal,10,0,0);BIG=0;return;
	  }
	else if ((x>=133 && y>=70 && x<=195 && y<=82 && mousebuttons > 0) || key==K_ENTER || key=='P')
	  {
	   InBox(133,70,195,82);
		WaitForMouseUp();
	   OutBox(133,70,195,82);
	   FadeOut(2,pal,10,0,0);BIG=1;return;
	  }

	 else
	if ((x>=207 && y>=70 && x<=264 && y<=82 && mousebuttons > 0) || key=='S')
	  {
	   InBox(207,70,264,82);
	   WaitForMouseUp();
	   OutBox(207,70,264,82);
		{
		 ClrMiss(plr,pad);
		}
	   if (Data->P[plr].Mission[pad].MissionCode==0) {FadeOut(2,pal,10,0,0);return;}
	  }
  };
}


void AI_Begin(char plr)
{
  int i;
  FILE *fin;
  long len[2];


  for (i=0;i<768;i++) pal[i]=0;
  gxSetDisplayPalette(pal);

  fin=sOpen("TURN.BUT","rb",0);
  fread(&pal,768,1,fin);
  len[0]=fread(screen,1,MAX_X*MAX_Y,fin);
  fclose(fin);
  RLED_img((char *)screen,vhptr.vptr,(unsigned int)len[0],
	   vhptr.w,vhptr.h);

  gxClearDisplay(0,0); ShBox(0,60,319,80);
  grSetColor(6+plr*3);
  if (plr==0) DispBig(15,64,"DIRECTOR OF THE UNITED STATES",0,-1);
  else DispBig(30,64,"CHIEF DESIGNER OF THE USSR",0,-1);
  grSetColor(11);
  grMoveTo(175,122);
  if (Data->Season==0) PrintAt(0,0,"SPRING 19");
  else PrintAt(0,0,"FALL 19");
  DispNum(0,0,Data->Year);
  gxVirtualDisplay(&vhptr,1+110*plr,1,30,85,30+107,85+93,0);
  grSetColor(11);
  PrintAt(60,58,"COMPUTER TURN:  THINKING...");
  music_start(M_SOVTYP);
  FadeIn(2,pal,10,0,0);
  colss=0;
}

void AI_Done(void)
{
  music_stop();
  FadeOut(2,pal,10,0,0);
  memset(screen,0x00,64000);

}

