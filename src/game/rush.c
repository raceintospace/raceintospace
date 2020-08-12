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

// Function Prototype

char Mon[12][4] = {
   "JAN","FEB","MAR","APR","MAY","JUN",
   "JUL","AUG","SEP","OCT","NOV","DEC"};

int fCsh; //rush cash
extern struct mStr Mis;
extern char pNeg[NUM_PLAYERS][MAX_MISSIONS];

char dg[62][6] =
{  {00,00,00,00,00,00},   // 0
   {00,00,00,00,00,00},   // 1
   {00,00,00,00,00,00},   // 2
   {00,00,00,00,00,00},   // 3
   {02,00,00,00,00,00},   // 4
   {00,00,00,00,00,00},   // 5
   {04,02,00,00,00,00},   // 6
   {00,00,00,00,00,00},   // 7
   {00,00,00,00,00,00},   // 8
   {00,00,00,00,00,00},   // 9
   {00,00,00,00,00,00},   // 10
   {00,00,00,00,00,00},   // 11
   {00,00,00,00,00,00},   // 12
   {00,00,00,00,00,00},   // 13
   {04,02,00,00,00,00},   // 14
   {00,00,00,00,00,00},   // 15
   {00,00,00,00,00,00},   // 16
   {04,02,00,00,00,00},   // 17
   {00,00,00,00,00,00},   // 18
   {00,00,00,00,00,00},   // 19
   {04,00,00,00,00,00},   // 20
   {16,00,00,00,00,00},   // 21
   {19,00,00,00,00,00},   // 22
   {19,00,00,00,00,00},   // 23
   {17,06,04,00,00,00},   // 24
   {04,00,00,00,00,00},   // 25
   {25,06,04,00,00,00},   // 26
   {25,14,06,04,00,00},   // 27
   {25,17,04,00,00,00},   // 28
   {24,26,17,14,06,00},   // 29
   {18,04,00,00,00,00},   // 30
   {19,00,00,00,00,00},   // 31
   {19,00,00,00,00,00},   // 32
   {26,27,25,14,06,00},   // 33
   {31,21,16,00,00,00},   // 34
   {22,37,19,00,00,00},   // 35
   {32,37,22,19,00,00},   // 36
   {19,00,00,00,00,00},   // 37
   {04,00,00,00,00,00},   // 38
   {16,00,00,00,00,00},   // 39
   {38,25,14,06,04,00},   // 40
   {39,16,00,00,00,00},   // 41
   {00,00,00,00,00,00},   // 42
   {04,00,00,00,00,00},   // 43
   {16,00,00,00,00,00},   // 44
   {00,00,00,00,00,00},   // 45
   {43,04,00,00,00,00},   // 46
   {44,16,00,00,00,00},   // 47
   {46,43,38,04,00,00},   // 48
   {38,16,00,00,00,00},   // 49
   {48,46,43,38,04,00},   // 50
   {49,39,16,00,00,00},   // 51
   {47,44,38,16,00,00},   // 52
   {46,43,00,00,00,00},   // 53
   {46,43,00,00,00,00},   // 54
   {47,44,00,00,00,00},   // 55
   {49,00,00,00,00,00},   // 56
   {49,00,00,00,00,00},   // 57
   {00,00,00,00,00,00},   // 58
   {00,00,00,00,00,00},   // 59
   {00,00,00,00,00,00},   // 60
   {00,00,00,00,00,00}    // 61
};


void DrawRush(char plr)
{
  int i,k=0,l=0,JR=0;
  FILE *fin;
  
  FadeOut(2,pal,10,0,0);

 #if 0
  fin=sOpen("CONTROL.IMG","rb",0);
  fread(&pal[0],768,1,fin);
  if (plr==1) {
    fread(&len[0],4,1,fin);
    fseek(fin,len[0],SEEK_CUR);
    fread(&pal[0],768,1,fin);
  }
  fclose(fin);
#endif
  fin=sOpen("LPADS.BUT","rb",0);
  i=fread(screen,1,MAX_X*MAX_Y,fin);fclose(fin);
  RLED_img(screen,vhptr.vptr,i,vhptr.w,vhptr.h);
  gxClearDisplay(0,0);
  JR=0;
  for (l=0;l<3;l++) {
    if (Data->P[plr].Mission[l].Joint==1) JR=1;
    if (Data->P[plr].Mission[l].MissionCode>0 &&
	Data->P[plr].Mission[l].part==0) k++;
  if (Data->P[plr].Mission[l].Rushing==1) Data->P[plr].Cash+=3;
    else if (Data->P[plr].Mission[l].Rushing==2) Data->P[plr].Cash+=6;
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

  ShBox(0,0,319,23);
  IOBox(243,3,316,19);
  InBox(3,3,30,19);
  grSetColor(1);PrintAt(263,13,"ASSIGN");
  FlagSm(plr,4,4);
  for (i=0;i<3;i++) {
    if (Data->P[plr].Mission[i].MissionCode>0 &&
	  Data->P[plr].Mission[i].part==0 ) {
      ShBox(0,25+i*58,80,82+i*58-1);ShBox(83,25+i*58,319,82+i*58-1);

      IOBox(278,30+i*58,314,42+i*58);
      IOBox(278,47+i*58,314,59+i*58);
      IOBox(278,64+i*58,314,76+i*58);

      IOBox(89,39+i*58,266,61+i*58);
      grSetColor(1);PrintAt(89,33+i*58,"SCHEDULE:");
      PrintAt(88,69+i*58,"RUSHING PENALTY: ");
      PrintAt(88,77+i*58,"DOWNGRADE PENALTY: ");
      PrintAt(199,69+i*58,"COST:");
      OutBox(11 ,33+i*58, 69, 74+i*58);
      InBox(20,38+i*58,60,69+i*58);
      gxVirtualDisplay(&vhptr,156*plr,i*30,21,39+i*58,59,68+i*58,0);

      SetRush(Data->P[plr].Mission[i].Rushing,i);
      grSetColor(1);
      DispBig(55,5,"MISSION SCHEDULE",0,-1);

      grSetColor(5);

      GetMisType(Data->P[plr].Mission[i].MissionCode);

      PrintAt(96,48+58*i,Mis.Abbr);
      int MisCod; 
      MisCod=Data->P[plr].Mission[i].MissionCode;
      if ((MisCod>24 && MisCod<32) || MisCod==33 || MisCod==34 || MisCod==35 || MisCod==37 || MisCod==40 || MisCod==41)  
	// Show duration level only on missions with a Duration step - Leon
       {
	switch(Data->P[plr].Mission[i].Duration)
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
      if (Data->P[plr].Mission[i].Name[24]==1)
       {
        grSetColor(9);PrintAt(145,33+i*58,"DOWNGRADED MISSION");
	     PrintAt(193,77+i*58,"-3 PRESTIGE");
       }
      else
       {
        grSetColor(7);PrintAt(145,33+i*58,"ORIGINAL MISSION");
        PrintAt(193,77+i*58,"NO PENALTY");
       }
      grSetColor(11);
      PrintAt(288,38+58*i,&Mon[Data->P[plr].Mission[i].Month-0][0]);
      PrintAt(288,55+58*i,&Mon[Data->P[plr].Mission[i].Month-1][0]);
      PrintAt(288,72+58*i,&Mon[Data->P[plr].Mission[i].Month-2][0]);
    }; /* End if */
  }; /* End for i */
  
  return;
}

void Rush(char plr)
{
int i,R1,R2,R3,oR1,oR2,oR3,dgflag[3]={0,0,0};
char pRush=0;

  R1=R2=R3=oR1=oR2=oR3=0;fCsh=0;
  DrawRush(plr);
  pRush=(Data->P[plr].Cash>=3) ? 1 : 0;
  fCsh=Data->P[plr].Cash;
  grSetColor(1);
  music_start((plr==1)?M_USMIL:M_USSRMIL);
  FadeIn(2,pal,10,0,0);
  WaitForMouseUp();
  while (1)
  {
   key=0;GetMouse();
   if (mousebuttons > 0 || key>0)
    {
     if (((y>=32 && y<=74 && x>=280 && x<=312 && mousebuttons>0) || (key>='1' && key<='3'))
      && pRush && Data->P[plr].Mission[0].MissionCode>0 && Data->P[plr].Mission[0].part!=1)  /* L1: Row One */
      {
      // R1=oR1;
       if (((y>=49 && y<=57 && mousebuttons>0) || key=='2') && oR1!=1 && fCsh<3) Help("i117");
       R1=(((y>=49 && y<=57 && mousebuttons>0) || key=='2') && fCsh>=3) ? 1 : R1;
	    if (((y>=66 && y<=74 && mousebuttons>0) || key=='3') && oR1!=2 && fCsh<6) Help("i117");
       R1=(((y>=66 && y<=74 && mousebuttons>0) || key=='3') && fCsh>=6) ? 2 : R1;
	    R1=((y>=32 && y<=40 && mousebuttons>0) || key=='1') ? 0 : R1;
	    if (oR1!=R1) {ResetRush(oR1,0);SetRush(R1,0);oR1=R1;}
      }
    else
    if (((x>=280 && x<=312 && y>=90 && y<=132 && mousebuttons>0) || (key>='4' && key<='6'))
      && pRush && Data->P[plr].Mission[1].MissionCode>0 && Data->P[plr].Mission[1].part!=1)  /* L2: Row One */
     {
      // R2=oR2;
      if (((y>=107 && y<=115 && mousebuttons>0) || key=='5') && oR2!=1 && fCsh<3) Help("i117");
      R2=(((y>=107 && y<=115 && mousebuttons>0) || key=='5') && fCsh>=3) ? 1 : R2;
   	if (((y>=124 && y<=132 && mousebuttons>0) || key=='6') && oR2!=2 && fCsh<6) Help("i117");
      R2=(((y>=124 && y<=132 && mousebuttons>0) || key=='6') && fCsh>=6) ? 2 : R2;
	   R2=((y>=90 && y<=98 && mousebuttons>0) || key=='4') ? 0 : R2;
   	if (oR2!=R2) {ResetRush(oR2,1);SetRush(R2,1);oR2=R2;}
     }
    else
   if (((x>=280 && x<=312 && y>=148 && y<=190 && mousebuttons>0) || (key>='7' && key<='9'))
     && pRush && Data->P[plr].Mission[2].MissionCode>0 && Data->P[plr].Mission[2].part!=1)  /* L3: Row One */
    {
     // R3=oR3;
     if (((y>=165 && y<=173 && mousebuttons>0) || key=='8') && oR3!=1 && fCsh<3) Help("i117");
     R3=(((y>=165 && y<=173 && mousebuttons>0) || key=='8') && fCsh>=3) ? 1 : R3;
     if (((y>=182 && y<=190 && mousebuttons>0) || key=='9') && oR3!=2 && fCsh<6) Help("i117");
     R3=(((y>=182 && y<=190 && mousebuttons>0) || key=='9') && fCsh>=6) ? 2 : R3;
     R3=((y>=148 && y<=156 && mousebuttons>0) || key=='7') ? 0 : R3;
     if (oR3!=R3) {ResetRush(oR3,2);SetRush(R3,2);oR3=R3;}
    };
   // DOWNGRADE MISSION KEYBOARD
	if (key=='Q' || key=='R' || key=='U')
    {
	  if (key=='Q') i=0; 
	  else if (key=='R') i=1; 
	  else if (key=='U') i=2;
	  else i=0;

     if (Data->P[plr].Mission[i].MissionCode>0 && Data->P[plr].Mission[i].part!=1)    
	   {
	    
	    InBox(91,41+i*58,264,59+i*58);
	    RectFill(144,29+i*58,270,37+i*58,3);
	    if (dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]]!=0)
	     {
	      RectFill(93,43+i*58,262,57+i*58,3);

        grSetColor(5);
        GetMisType(dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]]);

        PrintAt(96,48+58*i,Mis.Abbr);
	if (Mis.Dur>=1)
	{
	 switch(Data->P[plr].Mission[i].Duration)
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

	      //Missions(plr,96,48+58*i,dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]],0);

	      dgflag[i]++;
	      RectFill(191,71+i*58,270,78+i*58,3);
	      grSetColor(9);PrintAt(145,33+i*58,"DOWNGRADED MISSION");
	      PrintAt(193,77+i*58,"-3 PRESTIGE");
	     }
	    else 
	     {
	      dgflag[i]=0;
	      RectFill(93,43+i*58,262,57+i*58,3);
        grSetColor(5);
        GetMisType(Data->P[plr].Mission[i].MissionCode);

        PrintAt(96,48+58*i,Mis.Abbr);
	if (Mis.Dur>=1)
	{
	 switch(Data->P[plr].Mission[i].Duration)
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

	      //Missions(plr,96,48+58*i,Data->P[plr].Mission[i].MissionCode,0);   
	      RectFill(191,71+i*58,270,78+i*58,3);
         if (Data->P[plr].Mission[i].Name[24]==1)
          {
           grSetColor(9);PrintAt(145,33+i*58,"DOWNGRADED MISSION");
	        PrintAt(193,77+i*58,"-3 PRESTIGE");
          }
         else
          {
           grSetColor(7);PrintAt(145,33+i*58,"ORIGINAL MISSION");
           PrintAt(193,77+i*58,"NO PENALTY");
          }
	      //grSetColor(7);
	      //PrintAt(145,33+i*58,"ORIGINAL MISSION");
	      //PrintAt(193,77+i*58,"NO PENALTY");
	     }
	    WaitForMouseUp();
	    OutBox(91,41+i*58,264,59+i*58);
	    
	   }
    };
   for (i=0;i<3;i++)
    {
	  if (x>=91 && x<=264 && y>=41+i*59 && y<=59+i*59 && mousebuttons>0 
	   && Data->P[plr].Mission[i].MissionCode>0
	   && Data->P[plr].Mission[i].part!=1)     // Downgrade
	  {
	   
	   InBox(91,41+i*58,264,59+i*58);
	   RectFill(144,29+i*58,270,37+i*58,3);
	   if (dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]]!=0)
	    {
	     RectFill(93,43+i*58,262,57+i*58,3);
        grSetColor(5);
        GetMisType(dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]]);
        PrintAt(96,48+58*i,Mis.Abbr);
	if (Mis.Dur>=1)
	{
	 switch(Data->P[plr].Mission[i].Duration)
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

	     //Missions(plr,96,48+58*i,dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]],0);
	     dgflag[i]++;
	     RectFill(191,71+i*58,270,78+i*58,3);
	     grSetColor(9);PrintAt(145,33+i*58,"DOWNGRADED MISSION");
	     PrintAt(193,77+i*58,"-3 PRESTIGE");
	    }
	  else 
	    {
	     dgflag[i]=0;
	     RectFill(93,43+i*58,262,57+i*58,3);
        grSetColor(5);
        GetMisType(Data->P[plr].Mission[i].MissionCode);

        PrintAt(96,48+58*i,Mis.Abbr);
	if (Mis.Dur>=1)
	{
	 switch(Data->P[plr].Mission[i].Duration)
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

	     //Missions(plr,96,48+58*i,Data->P[plr].Mission[i].MissionCode,0);   
	     RectFill(191,71+i*58,270,78+i*58,3);
        if (Data->P[plr].Mission[i].Name[24]==1)
         {
          grSetColor(9);PrintAt(145,33+i*58,"DOWNGRADED MISSION");
	       PrintAt(193,77+i*58,"-3 PRESTIGE");
         }
        else
         {
          grSetColor(7);PrintAt(145,33+i*58,"ORIGINAL MISSION");
          PrintAt(193,77+i*58,"NO PENALTY");
         }
	     //grSetColor(7);
	     //PrintAt(145,33+i*58,"ORIGINAL MISSION");
	     //PrintAt(193,77+i*58,"NO PENALTY");
	    }
	  WaitForMouseUp();
	  OutBox(91,41+i*58,264,59+i*58);
	  
	 }
   };
  if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER)     //  CONTINUE
   {
	 InBox(245,5,314,17);
	 WaitForMouseUp();
    if (key>0) delay(150);
	 OutBox(245,5,314,17);delay(10);
	 for (i=0;i<3;i++)
	 {
	  if (Data->P[plr].Mission[i].MissionCode>0)
	   {
	    if (dgflag[i]!=0)
        {
		   Data->P[plr].Mission[i].MissionCode=dg[Data->P[plr].Mission[i].MissionCode][dgflag[i]-1];
         pNeg[plr][i]=1;
        }
	   }
	 }
	 if (Data->P[plr].Mission[1].part==1) R2=R1;
	 if (Data->P[plr].Mission[2].part==1) R3=R2;
	 if (Data->P[plr].Mission[0].MissionCode>0 && Data->P[plr].Cash>=3*R1)
     {
	   Data->P[plr].Cash-=3*R1;
	   Data->P[plr].Mission[0].Month-=R1;
	   Data->P[plr].Mission[0].Rushing=R1;
	  };
	 if (Data->P[plr].Mission[1].MissionCode>0 && Data->P[plr].Cash>=3*R2)
     {
	   Data->P[plr].Cash-=3*R2;
	   Data->P[plr].Mission[1].Month-=R2;
	   Data->P[plr].Mission[1].Rushing=R2;
	  };
	 if (Data->P[plr].Mission[2].MissionCode>0 && Data->P[plr].Cash>=3*R3)
     {
	   Data->P[plr].Cash-=3*R3;
	   Data->P[plr].Mission[2].Month-=R3;
	   Data->P[plr].Mission[2].Rushing=R3;
	  }
    music_stop();
	 return;  // Done
   };
  }
 };
}

void ResetRush(int mode,int val)
{
  
  OutBox(280,32+17*mode+val*58,312,40+17*mode+val*58);
  fCsh+=mode*3;
  
  return;
}

void SetRush(int mode,int val)
{
  
  InBox(280,32+17*mode+val*58,312,40+17*mode+val*58);  
  RectFill(177,63+58*val,192,70+58*val,3);
  RectFill(225,62+58*val,270,70+58*val,3);
  grSetColor(11);DispNum(179,69+58*val,mode*3);DispChr('%');
  grSetColor(9);DispNum(230,69+58*val,mode*3);grSetColor(1);PrintAt(237,69+58*val,"MB");
  fCsh-=mode*3;
  
  return;
}
