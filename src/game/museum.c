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
// Museum Main Files

#include <Buzz_inc.h>
#include <externs.h>
extern char Month[12][11]; 
extern char Nums[30][7],AI[2];
extern struct mStr Mis;
#define DELAYCNT 50

struct Astros *abuf;

#if 1
char tame[29][40]={
"ORBITAL SATELLITE",
"LUNAR FLYBY",
"MERCURY FLYBY",
"VENUS FLYBY",
"MARS FLYBY",
"JUPITER FLYBY",
"SATURN FLYBY",
"LUNAR PROBE LANDING",
"DURATION LEVEL F",
"DURATION LEVEL E",
"DURATION LEVEL D",
"DURATION LEVEL C",
"DURATION LEVEL B",
"ONE-PERSON CRAFT",
"TWO-PERSON CRAFT",
"THREE-PERSON CRAFT",
"MINISHUTTLE",
"FOUR-PERSON CRAFT",
"MANNED ORBITAL",
"MANNED LUNAR PASS",
"MANNED LUNAR ORBIT",
"MANNED RESCUE ATTEMPT",
"MANNED LUNAR LANDING",
"ORBITING LAB",
"MANNED DOCKING",
"WOMAN IN SPACE",
"SPACEWALK",
"MANNED SPACE MISSION"
};
#endif

i16 TPoints[2];
extern char Mon[12][4];

#ifdef DEAD_CODE
void BigArrowLt(int,int);
void BigArrowRt(int,int);
void EndRt(int,int);
void EndLt(int,int);
#endif

void Display_ARROW(char num,int x,int y) 
{
    /* Look for explanations in place.c:PatchMe() */
    PatchHdrSmall P;
  int do_fix = 0;
  GXHEADER local,local2;
  FILE *in;
  in=sOpen("ARROWS.BUT","rb",0);
  fseek(in,(num)*(sizeof P),SEEK_CUR);
  fread(&P,sizeof P,1,in);
    SwapPatchHdrSmall(&P);
  fseek(in,P.offset,SEEK_SET);
  if (P.w * P.h != P.size)
  {
      /* fprintf(stderr,
              "Display_ARROW(): w*h != size (%hhd*%hhd == %d != %hd)\n",
              P.w, P.h, P.w*P.h, P.size); */
      if ((P.w+1) * P.h == P.size) {
          /* fprintf(stderr, "Display_ARROW(): P.w++ saves the day!\n"); */
          P.w++;
          do_fix = 1;
      }
      P.size = P.w * P.h;
  }
  GV(&local,P.w,P.h); GV(&local2,P.w,P.h);
  gxGetImage(&local2,x,y,x+P.w-1,y+P.h-1,0);
  fread(local.vptr,P.size,1,in);
  fclose(in);
 // for (j=0;j<P.size;j++) 
 //   if(local.vptr[j]!=0) local2.vptr[j]=local.vptr[j];
  gxPutImage(&local,gxSET,x,y,0);
  DV(&local); DV(&local2);
  return;
}

void Museum(char plr)
{
	int i,tots=7,beg;
  char AName[7][22]={"DIRECTOR RANKING","SPACE HISTORY","MISSION RECORDS","PRESTIGE SUMMARY",
    "HARDWARE EFFICIENCY","ASTRONAUT HISTORY","EXIT THE MUSEUM"};
  char AImg[7]={8,9,10,11,13,14,0};

  if (Data->P[plr].AstroCount==0) {
     memcpy(&AName[5][0],&AName[6][0],22);  // move up Exit
     AImg[5]=AImg[6];
     tots=6;
  }
  else if (plr==1) strncpy(&AName[5][0],"COSMO",5);

  AImg[3]+=plr;
  // FadeOut(2,pal,10,0,0);
  music_start(M_THEME);
  beg=0;

  do {
     if (beg==0) beg=(Data->P[plr].AstroCount>0)? 7 : 6;  // mods for astros
     else {
        FadeOut(2,pal,10,0,0);

        DrawSpaceport(plr);
        PortPal(plr);
        RectFill(166,191,318,198,3);
        grSetColor(0);PrintAt(257,197,"CASH:");DispMB(285,197,Data->P[plr].Cash);
        grSetColor(11);PrintAt(256,196,"CASH:");DispMB(284,196,Data->P[plr].Cash);
        grSetColor(0);if (Data->Season==0) PrintAt(166,197,"SPRING 19");
        else PrintAt(166,197,"FALL 19");DispNum(0,0,Data->Year);
        grSetColor(11);if (Data->Season==0) PrintAt(165,196,"SPRING 19");
        else PrintAt(165,196,"FALL 19");DispNum(0,0,Data->Year);
        
        FadeIn(2,pal,10,0,0);
     }

     strncpy(IDT,(plr==0) ?"i700":"i701",4);
     strncpy(IKEY,(plr==0) ? "k603":"k604",4);
     i=BChoice(plr,tots,&AName[0][0],&AImg[0]);
 
	   switch(i) {
	      case 1: RankMe(plr); break;
	      case 2: strncpy(IDT,"i130",4);strncpy(IKEY,"k031",4);
                ShowSpHist(plr);break;
	      case 3: strncpy(IDT,"i131",4);strncpy(IKEY,"k321",4);
                Records(plr);break;
	      case 4: strncpy(IDT,"i132",4);strncpy(IKEY,"k033",4);
                ShowPrest(plr);break;
	      case 5: strncpy(IDT,"i034",4);ShowHard(plr);break;
	      case 6: strncpy(IDT,(plr==0)? "i133" :"i134",4);
                strncpy(IKEY,(plr==0)? "k035" :"k441",4);
                if (Data->P[plr].AstroCount>0) ShowAstrosHist(plr);
	          break;
	      case 7: default: break;
	   }
     strncpy(IDT,"i000",4);strncpy(IKEY,"k000",4);
  } while (i!=beg);
  music_stop();
	return;
}

void ShowPrest(char plr)
{
   char pos=-1,pos2=-1;
   int i,j;

   for(j=0;j<2;j++)
      for(i=0;i<28;i++)
         TPoints[j]+=(int) Data->Prestige[i].Points[j];
      
  
  FadeOut(2,pal,5,0,0);
  PortPal(plr);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);ShBox(0,24,319,199);
  InBox(4,27,315,196);
  RectFill(5,28,314,195,0);

  //ShBox(6,29,125,101);
  ShBox(6,29,56,101);
  InBox(17,46,44,62);

  InBox(70,42,174,92);
  
  ShBox(179,29,313,101);
  ShBox(58,29,313,101);

  InBox(70,42,174,92);InBox(185,42,304,92);

  //ShBox(136,74,163,90);
  IOBox(243,3,316,19);
  ShBox(6,104,313,194); InBox(10,127,309,191);RectFill(11,128,308,190,0);
  
  ShBox(297,129,307,158);ShBox(297,160,307,189); // Arrows

  DispBig(8,5,"PRESTIGE SUMMARY",0,-1);
  DispBig(14,109,"EVENTS",0,-1);
  grSetColor(11);
  PrintAt(140,120,"1ST:");
  PrintAt(175,120,"2ND:");
  PrintAt(212,120,"SUBS:");
  PrintAt(254,120,"FAIL:");
  grSetColor(1);PrintAt(257,13,"CONTINUE");
  PrintAt(17,39,"FIRST:");
  PrintAt(19,81,"DATE:");
  grSetColor(11);PrintAt(71,37,"# SPACE FIRSTS:");
  PrintAt(187,37,"TOTAL POINTS:");grSetColor(1);
  grSetColor(6);PrintAt(210,99,"USA");PrintAt(90,99,"USA");
  grSetColor(9);PrintAt(261,99,"USSR");PrintAt(135,99,"USSR");
  UPArrow(299,131);DNArrow(299,162);
  DPrest(plr,&pos,&pos2);
  FadeIn(2,pal,5,0,0);
  
	WaitForMouseUp();
  while(1)
  {
      GetMouse();
          // Parse Button actions, note that return is embedded in first pButton
          if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER) {
            InBox(245,5,314,17);
            if (key>0) {delay(300);key=0;};
            WaitForMouseUp();
            OutBox(245,5,314,17); key=0;
            strcpy(IDT,"i000");strcpy(IKEY,"k000");
            return;
	  }
          pButton(297,129,307,158,BackOne(plr,&pos,&pos2),key>>8,72);
          pButton(297,160,307,189,ForOne(plr,&pos,&pos2),key>>8,80);
          Button2(15,129,160,133,Move2(plr,&pos,&pos2,0),key,49);
          Button2(15,136,160,140,Move2(plr,&pos,&pos2,1),key,50);
          Button2(15,143,160,147,Move2(plr,&pos,&pos2,2),key,51);
          Button2(15,150,160,154,Move2(plr,&pos,&pos2,3),key,52);
          Button2(15,157,160,161,Move2(plr,&pos,&pos2,4),key,53);
          Button2(15,164,160,168,Move2(plr,&pos,&pos2,5),key,54);
          Button2(15,171,160,175,Move2(plr,&pos,&pos2,6),key,55);
          Button2(15,178,160,182,Move2(plr,&pos,&pos2,7),key,56);
          Button2(15,185,160,189,Move2(plr,&pos,&pos2,8),key,57);
          key=0;
  };
}

void Move2(char plr, char *pos, char *pos2, char val )
{
   *pos2 = *pos + val;
   if (*pos2>=21) {*pos2+=1;}
   DPrest(plr,pos,pos2);
}

void BackOne(char plr, char *pos,char *pos2)
{
   if (*pos2== -1) return;
   *pos2-=1;
   if (*pos2==21) *pos2-=1;
   if (*pos==21) *pos-=1;
   if(*pos2 < *pos) *pos -=1;
   DPrest(plr,pos,pos2);
   return;
}

void ForOne(char plr, char *pos,char *pos2)
{
   if (*pos2==27) return;
   *pos2+=1;
   if (*pos2==21) *pos2+=1;
   if (*pos==21) *pos+=1;
   if(*pos > 19) ;
   else if (*pos2 > *pos+8) *pos+=1;
   if (*pos>18) *pos=18;
   DPrest(plr,pos,pos2);
   return;
}

void DPrest(char plr,char *pos,char *pos2)
{
   int i,j=0,tmp,tt;

   
   RectFill(12,129,295,190,0);
   RectFill(70,31,175,40,3);RectFill(183,31,300,40,3);
   if (*pos2==-1) {grSetColor(11);PrintAt(71,37,"SPACE FIRSTS:");PrintAt(187,37,"TOTAL POINTS:");}
    else {grSetColor(1);PrintAt(71,37,"ATTEMPTS:");PrintAt(187,37,"POINTS:");}

   grSetColor(2);tt=0;
   for(i=*pos;i<*pos+9+tt;i++,j++) {   // *pos+9+tt
      if (i==21) {
         i++;
         tt=1;
      }
      if(i==*pos2) grSetColor(11);
      if(i==-1) 
         PrintAt(15,133+7*j,"SUMMARY");
      else {
       PrintAt(15,133+7*j,&tame[i][0]);
	 if(i==*pos2) grSetColor(11); else grSetColor(12);
	 DispNum(150,133+7*j,Data->Prestige[i].Add[0]);
	 DispNum(185,133+7*j,Data->Prestige[i].Add[1]);
	 DispNum(228,133+7*j,Data->Prestige[i].Add[2]);
	 DispNum(265,133+7*j,Data->Prestige[i].Add[3]);
      }
      grSetColor(2);
   }
   RectFill(71,43,173,91,6+3*plr);
   RectFill(186,43,303,91,6+3*plr);
   RectFill(10,86,55,92,3);
   grSetColor(3);
   for(i=1;i<3;i++)
	   pline(71,91-16*i,173,91-16*i);
   if(*pos2 != -1) {
      if (Data->Prestige[*pos2].Place == -1 && Data->Prestige[*pos2].Year==0)
        RectFill(18,47,43,61,3);
      else FlagSm(Data->Prestige[*pos2].Place,18,47);

      i = maxx(Data->Prestige[*pos2].Goal[0],Data->Prestige[*pos2].Goal[1]);
      if (i!=0) {
         j=Data->Prestige[*pos2].Goal[0];
         tmp=Data->Prestige[*pos2].Goal[1];

         RectFill(85,(int) 91-48*((float)j/i),111,91,5);
         RectFill(133,(int) 91-48*((float)tmp/i),159,91,8);
         grSetColor(11);
         if (j!=0) DispNum(94,88,j);
         if (tmp!=0) DispNum(142,88,tmp);
         if (Data->Prestige[*pos2].Year!=0)
         {    
           PrintAt(10,91,Mon[Data->Prestige[*pos2].Month]);
           PrintAt(0,0," 19");
           DispNum(0,0,Data->Prestige[*pos2].Year);
         }
                 
      }

      grSetColor(3);
      for(i=0;i<3;i++) pline(186,79-12*i,303,79-12*i);
      i = maxx(abs(Data->Prestige[*pos2].Points[0]),abs(Data->Prestige[*pos2].Points[1]));
      grSetColor(5);pline(205,67,231,67);
      grSetColor(8);pline(258,67,284,67);
      if (i != 0) {
         j=Data->Prestige[*pos2].Points[0];
         tmp=Data->Prestige[*pos2].Points[1];
         RectFill(205,67,231,67-(float)j*24/i,5);
         RectFill(258,67,284,67-(float)tmp*24/i,8);
         grSetColor(11);
         if (j!=0) DispNum(212,(j>0)? 65:73,j);
         if (tmp!=0) DispNum(267,(tmp>0)?65:73,tmp);
      }
   }
   else {
		j=0;tmp=0;
		for(i=0;i<28;i++) {
			j+=(Data->Prestige[i].Place == 0) ? 1 :  0;
			tmp+=(Data->Prestige[i].Place == 1) ? 1 : 0;
		}
      i = maxx(j,tmp);
      if (i != 0) {
         if (j==tmp) RectFill(18,47,43,61,3);
         else if (j>tmp) FlagSm(0,18,47);
         else FlagSm(1,18,47);
         RectFill(85,(int) 91-48*((float)j/i),111,91,5);
         RectFill(133,(int) 91-48*((float)tmp/i),159,91,8);
         grSetColor(11);
         if (j!=0) DispNum(94,88,j);
         if (tmp!=0) DispNum(142,88,tmp);
      }  else RectFill(18,47,43,61,3);

      j=0;tmp=0;
      for(i=0;i<28;i++){
         j+=Data->Prestige[i].Points[0];
         tmp+=Data->Prestige[i].Points[1];
      }
      grSetColor(3);
      for(i=0;i<3;i++)
         pline(186,79-12*i,303,79-12*i);
      i = maxx(abs(j),abs(tmp));
      pline(205,67,231,67);
      pline(258,67,284,67);
      if (i != 0) {
         RectFill(205,67,231,67-(float)j*24/i,5);
         RectFill(258,67,284,67-(float)tmp*24/i,8);
         grSetColor(11);
         if (j!=0) DispNum(212,(j>0)? 65:73,j);
         if (tmp!=0) DispNum(267,(tmp>0)?65:73,tmp);
      }
   }
   
   return;
}

/*  Indexing used for the mission history screen:
 *  To find the index, pos in ShowSpHist, and, *where in the subs, use 
 *  
 *   pos = 2*(year-57) + season
 *   
 *   to go from pos to year and season use:
 *   
 *   year = (pos - season)/2 + 57
 *   
 *   season = where mod 2
 *   
 *   pos is an index: 0 ==> 57,Spring
 *                    1 ==> 57,Fall
 *                    2 ==> 58,Spring, etc...
 */
 
void ShowSpHist(char plr)
{
  int pos;
  
  FadeOut(2,pal,5,0,0);
  PatchMe(0,0,0,0,0,32);
  memset(screen,0x00,64000);
  if ((Data->Year == 57 && Data->Season == 0) || Data->P[plr].PastMis == 0)
     pos = (Data->Year-57)*2+Data->Season;
  else pos = (Data->P[plr].History[Data->P[plr].PastMis-1].MissionYear - 57)*2 +
             ((Data->P[plr].History[Data->P[plr].PastMis-1].Month <=5) ? 0 : 1);
  
  ORBox(0,0,319,22,3); // Draw Inbox around top 
  DispBig(48,5,"MISSION HISTORY",0,-1);
  IOBox(243,3,316,19);
  InBox(3,3,31,19); // USA inbox
  FlagSm(plr,4,4);
  grSetColor(1);
  PrintAt(257,13,"CONTINUE");

  ORBox(0,24,319,199,3);
  IRBox(4,28,315,170,0);
  IRBox(4,174,315,195,0);
  ORBox(7,176,49,193,3);
  ORBox(51,176,93,193,3);
  ORBox(95,176,224,193,3);     //draw the boxes under date
  ORBox(226,176,268,193,3);
  ORBox(270,176,312,193,3);
  Display_ARROW(0,23,178); //left
  Display_ARROW(1,63,178); //left arrow
  Display_ARROW(2,239,178); //right
  Display_ARROW(3,285,178); //right arrow
  DrawMisHist(plr,&pos);
  FadeIn(2,pal,5,0,0);
  
	WaitForMouseUp();
  while(1)
  {
      GetMouse();
      Mission_Data_Buttons(plr,&pos);
      // Parse Button actions, note that return is embedded in first pButton
      if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER)
       {
	     InBox(245,5,314,17);
        if (key>0) {delay(300);key=0;};
	     WaitForMouseUp();
	     OutBox(245,5,314,17);
	     return;
       }
      pButton(7,176,49,193,FullRewind(plr,&pos),key>>8,71); //FullRewind Button etc..
      pButton(51,176,93,193,RewindOne(plr,&pos),key>>8,75);
      pButton(226,176,268,193,FastOne(plr,&pos),key>>8,77);
      pButton(270,176,312,193,FullFast(plr,&pos),key>>8,79);
      key=0;
  };
}

void Mission_Data_Buttons(char plr, int *where)
{
    char index,yr,season,j,temp=0;

    /* Okay, now we have to decide whether there are any missions displayed
       on the screen at this time. If there are any, parse the button. */ 
    if(Data->P[plr].PastMis==0) return;
    index=0;
    season =*where % 2;
    yr=(*where-season)/2 +57;
    while(yr > Data->P[plr].History[index].MissionYear) index++;
    if(Data->P[plr].History[index].MissionYear > yr) return;

  if (season == 1) {
	  while(Data->P[plr].History[index].Month < 6) index++;
	  if(Data->P[plr].History[index].MissionYear > yr) return;
  }

  do{
	  if(Data->P[plr].History[index].Month > 6 && season ==0) break;

	  j = (Data->P[plr].History[index].Month < 6) ? Data->P[plr].History[index].Month 
	    : Data->P[plr].History[index].Month - 6;

	  Button2( 13+49*j,40*(1+temp),62+49*j,40*(2+temp) ,Draw_Mis_Stats(plr,index,where,0),key,0x31+temp);

	  temp++;index++;
  } while(Data->P[plr].History[index].MissionYear == yr);
}

void FastOne(char plr,int *where)
{
   
  int last;

  if(Data->P[plr].PastMis == 0) return ;

  last =(Data->P[plr].History[Data->P[plr].PastMis-1].MissionYear - 57)*2 +
        ((Data->P[plr].History[Data->P[plr].PastMis-1].Month <= 5) ? 0 : 1);

  if (*where >= last) return;
  *where +=1;
  DrawMisHist(plr,where);
  return;
}

void FullRewind(char plr, int *where)
{
    *where=0;
    DrawMisHist(plr,where);
    return;
}

void RewindOne(char plr, int *where)
{
  if(Data->P[plr].PastMis == 0) return;

  if(!(*where == 0)) *where -=1;
  DrawMisHist(plr,where);
  return;
}

void FullFast(char plr, int *where)
{
    if (Data->P[plr].PastMis == 0) return;
    *where=(Data->P[plr].History[Data->P[plr].PastMis-1].MissionYear - 57)*2 +
           ((Data->P[plr].History[Data->P[plr].PastMis-1].Month <=5) ? 0 : 1);
    DrawMisHist(plr,where);
}

void DrawMisHist(char plr,int *where)
{
   char cYr[5],mtext[51];
   char yr,season,i,j,index=0,prog,planet,pmis,temp=0,temp2=11;

   
   //ai klugge

   for(i=0;i<Data->P[plr].PastMis;i++)
    {
     if (Data->P[plr].History[i].MissionCode==55 || Data->P[plr].History[i].MissionCode==56)
      {
       for (j=0;j<4;j++)
        if (Data->P[plr].History[i].Man[0][j]!=-1)
         {
          Data->P[plr].History[i].Man[1][j]=Data->P[plr].History[i].Man[0][j];
          Data->P[plr].History[i].Man[0][j]=-1;
         }
      }
    }
   yr=(*where-(*where % 2))/2 +57;
   season =*where % 2;
   ORBox(95,176,224,193,3);     //draw the boxes under date
   sprintf(cYr,"%d",1900+yr);
   DispBig(103+(yr-57)*4,178,cYr,0,-1);

   RectFill(5,29,314,169,0);
   grSetColor(7+3*plr);
   for(i=0;i<7;i++) pline(13+49*i,29,13+49*i,169);

   grSetColor(12);
   if (*where % 2 ==0) for(i=0;i<6;i++) {
    strncpy(cYr,Month[i],3);cYr[3]=0;
    PrintAt(29+49*i,36,cYr);
   }
   else for(i=0;i<6;i++){
    strncpy(cYr,Month[i+6],3);cYr[3]=0;
    PrintAt(29+49*i,36,cYr);
   }
  

  // What the hell does this do

  while(yr > Data->P[plr].History[index].MissionYear) index++;
  if(Data->P[plr].History[index].MissionYear > yr) {
	  
	  return;
  }

  if (season == 1) {
     while(Data->P[plr].History[index].Month<6 && yr==Data->P[plr].History[index].MissionYear) index++;
     if(Data->P[plr].History[index].MissionYear > yr) {
	      
	      return;
     }
  }

 do {
   if (Data->P[plr].History[index].Month>=6 && season ==0) break;
    j = (Data->P[plr].History[index].Month < 6) ? Data->P[plr].History[index].Month 
	  : Data->P[plr].History[index].Month - 6;



   // first check for joint missions 
   if (Data->P[plr].History[index].Hard[1][0]>0)
    {
	  sprintf(mtext,"%s",Data->P[plr].History[index].MissionName[1]);
	  grSetColor(11);
	  PrintAt(35+49*j-strlen(mtext)/2*5,45+40*temp,mtext);
	  if (Data->P[plr].History[index].Man[0][0]!=-1 && Data->P[plr].History[index].Hard[1][0]!=-1)
      {
	    PatchMe(plr,10+49*j,50+40*temp,Data->P[plr].History[index].Hard[0][0],
		 Data->P[plr].History[index].Patch[0],32);
	   }
	  if (Data->P[plr].History[index].Man[1][0]!=-1 && Data->P[plr].History[index].Hard[1][0]!=-1)
      {
	    PatchMe(plr,42+49*j,50+40*temp,Data->P[plr].History[index].Hard[1][0],
	    Data->P[plr].History[index].Patch[1],32);
      }
     if (Data->P[plr].History[index].Hard[1][0]!=-1 && Data->P[plr].History[index].Hard[1][0]!=-1)
      {
       if (Data->P[plr].History[index].Hard[0][0]!=-1 && Data->P[plr].History[index].Man[0][0]!=-1)
        {
         planet=0;prog=Data->P[plr].History[index].Hard[0][0];
         SmHardMe(plr,44+38*j,50+40*temp,prog,planet,64);
        }
       pmis = Data->P[plr].History[index].MissionCode;
       if (pmis==55 || pmis==56) temp2=0;
        else temp2=11;
       planet=0;prog=Data->P[plr].History[index].Hard[1][0];
       SmHardMe(plr,44+(38+temp2)*j,50+40*temp,prog,planet,64);
      }
      //else
      // {
      //  prog = (Data->P[plr].History[index].Hard[0][0] != -1) ? 
	   //  Data->P[plr].History[index].Hard[0][0] :
	   //  Data->P[plr].History[index].Hard[0][3]+5;
      //  planet=0;
    	//  SmHardMe(plr,44+49*j,50+40*temp,prog,planet,64);  
      // }
    }
     else
      {
       //fix-Handle Joint Missions
       if (Data->P[plr].History[index].Hard[0][0]!=-1 && Data->P[plr].History[index].Man[0][0]!=-1)
         {
          PatchMe(plr,10+49*j,50+40*temp,Data->P[plr].History[index].Hard[0][0],
		    Data->P[plr].History[index].Patch[0],32);
         }
       if (Data->P[plr].History[index].Hard[1][0]!=-1 && Data->P[plr].History[index].Man[1][0]!=-1) prog=Data->P[plr].History[index].Hard[1][0];
        else
         {
          prog = (Data->P[plr].History[index].Hard[0][0] != -1) ? 
	       Data->P[plr].History[index].Hard[0][0] :
	       Data->P[plr].History[index].Hard[0][3]+5;
         }
	    pmis = Data->P[plr].History[index].MissionCode;
	    if (prog==6 && pmis == 9) planet =2;
	     else if(prog == 6 && pmis == 10) planet = 3;
	      else if(prog == 6 && pmis == 11) planet = 1;
	       else if(prog == 6 && pmis == 12) planet = 4;
	        else if(prog == 6 && pmis == 13) planet = 5;
	         else if(pmis == 0) planet = 7;
	          else if(pmis == 1) planet = 7;
	           else if(pmis == 7) planet = 6;
	            else if(pmis == 8) planet = 6;
	             else planet=0;
	    sprintf(mtext,"%s",Data->P[plr].History[index].MissionName[0]);
	    grSetColor(11);
	    PrintAt(35+49*j-strlen(mtext)/2*5,45+40*temp,mtext);
	    SmHardMe(plr,44+49*j,50+40*temp,prog,planet,64);
      } 
   temp++;    
  index++;
 } while (Data->P[plr].History[index].MissionYear == yr);
 
 return;
}

#ifdef DEAD_CODE
void BigArrowLt(int a,int b)
{
 grSetColor(4);
 grMoveTo(a,b);grLineTo(a,b+13);grMoveTo(a+9,b);grLineTo(a+9,b+13);
 grSetColor(2);
 grMoveTo(a+1,b);grLineTo(a+8,b+6);grMoveTo(a+1,b+13);grLineTo(a+8,b+7);
 grMoveTo(a+10,b);grLineTo(a+17,b+6);grMoveTo(a+10,b+13);grLineTo(a+17,b+7);
 return;
}
#endif

#ifdef DEAD_CODE
void BigArrowRt(int a,int b)
{
 grSetColor(4);
 grMoveTo(a,b);grLineTo(a,b+13);grMoveTo(a+9,b);grLineTo(a+9,b+13);
 grSetColor(2);
 grMoveTo(a-1,b);grLineTo(a-8,b+6);grMoveTo(a-1,b+13);grLineTo(a-8,b+7);
 grMoveTo(a+8,b);grLineTo(a+1,b+6);grMoveTo(a+8,b+13);grLineTo(a+1,b+7);
 return;
}
#endif

#ifdef DEAD_CODE
void EndLt(int a,int b)
{
 grSetColor(4);
 grMoveTo(a,b);grLineTo(a,b+11);grMoveTo(a+9,b-1);grLineTo(a+9,b+12);
 grSetColor(2);
 grMoveTo(a,b-1);grLineTo(a-2,b-1);grLineTo(a-2,b+12);grLineTo(a,b+12);

 //grMoveTo(a+8,b-1);grLineTo(a+1,b+5);
 grMoveTo(a+1,b+5);grLineTo(a+8,b-1);
 grMoveTo(a+1,b+6);grLineTo(a+8,b+12);
 //grMoveTo(a+8,b+12);grLineTo(a+1,b+6);
 return;
}
#endif

#ifdef DEAD_CODE
void EndRt(int a,int b)
{
 grSetColor(4);
 grMoveTo(a,b);grLineTo(a,b+13);grMoveTo(a+9,b+1);grLineTo(a+9,b+12);
 grSetColor(2);
 grMoveTo(a+9,b);grLineTo(a+11,b);grLineTo(a+11,b+13);grLineTo(a+9,b+13);
 grMoveTo(a+1,b);grLineTo(a+8,b+6);grMoveTo(a+1,b+13);grLineTo(a+8,b+7);
 return;
}
#endif

void ShowAstrosHist(char plr)
{
  char pos=0,pos2=0,glorf=0;
  GV(&vhptr2,112,55);
  abuf=(struct Astros *) buffer;
  if(Data->P[plr].AstroCount == 0 ) return;
  memcpy(abuf,Data->P[plr].Pool,sizeof(Data->P[plr].Pool));  
  qsort((void *)abuf,Data->P[plr].AstroCount,sizeof(struct Astros),astcomp);
  
  FadeOut(2,pal,5,0,0);
  RectFill(1,39,157,184,3);
  DispBig(68,71,"NO",0,-1);
  DispBig(46,90,"MISSION",0,-1);
  DispBig(27,109,"EXPERIENCE",0,-1);
  gxGetImage(&vhptr2,22,69,133,123,0);
  PatchMe(0,0,0,0,0,32);
  gxClearDisplay(0,0);
  
  ORBox(0,0,319,22,3); // Draw Inbox around top 
  if(plr==0) DispBig(45,4,"ASTRONAUT HISTORY",0,-1); 
  else DispBig(45,4,"COSMONAUT HISTORY",0,-1); 
  IRBox(243,3,316,19,0); // Inbox around cont box
  ORBox(245,5,314,17,3); // box for cont box
  InBox(3,3,31,19); // USA inbox
  if(plr==0) FlagSm(0,4,4);
  else FlagSm(1,4,4); 
  grSetColor(1);
  PrintAt(257,13,"CONTINUE");
  ShBox(0,24,158,199);
  ShBox(161,24,319,199);
  InBox(233,29,314,80);
  InBox(287,104,309,134);
  IRBox(165,175,315,196,0);
  IRBox(6,185,153,197,0);
  ORBox(8,187,151,195,3);
  IRBox(6,26,153,38,0);
  ORBox(8,28,151,36,3);
  grSetColor(11);
  PrintAt(165,32,"NAME:");
  PrintAt(165,60,"TENURE:");
  PrintAt(165,78,"MISSIONS: ");
  PrintAt(165,89,"PRESTIGE: ");
  PrintAt(165,99,"SKILLS:");
  PrintAt(165,149,"SPACE DURATION:");
  PrintAt(165,159,"LOCATION: ");
  grSetColor(6);
  PrintAt(174,107,"CAPSULE PILOT: ");
  PrintAt(174,115,"L.M. PILOT: ");
  PrintAt(174,123,"E.V.A.: ");
  PrintAt(174,131,"DOCKING: ");
  PrintAt(174,139,"ENDURANCE: ");
  ORBox(167,177,202,194,3);
  ORBox(204,177,239,194,3);
  ORBox(241,177,276,194,3);
  ORBox(278,177,313,194,3);

  //IRBox(6,185,153,196,0);
  //ORBox(8,187,151,194,3);
  //RBox(6,27,153,38,0);
  //ORBox(8,29,151,36,3);
  Display_ARROW(0,179,179); //left
  Display_ARROW(1,213,179); //left arrow
  Display_ARROW(2,250,179); //right
  Display_ARROW(3,290,179); //right arrow
  grSetColor(11);
  PrintAt(37,34,"PREVIOUS MISSION");
  PrintAt(47,193,"NEXT MISSION");
  DisplAst(plr,&pos,&pos2);
  DisplAstData(plr,&pos,&pos2);  
  FadeIn(2,pal,5,0,0);
  
	WaitForMouseUp();
  while(1)
  {
      GetMouse();
          // Parse Button actions, note that continue button is not a macro
          if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons > 0) || key==K_ENTER) {
            InBox(245,5,314,17);
            if (key>0) {delay(300);key=0;};
            WaitForMouseUp();
            OutBox(245,5,314,17);
	    DV(&vhptr2);
	    key=0;
            return;
	  }
	  pButton(8,187,151,195,UpAstroData(plr,&pos,&pos2),key>>8,80);
	  pButton(8,28,151,36,DownAstroData(plr,&pos,&pos2),key>>8,72);
	  pButton(167,177,202,194,ShowAstroBack(plr,&pos,&pos2),key>>8,71); //Down to prev Astro
	  pButton(204,177,239,194,ShowAstroDown(plr,&pos,&pos2),key>>8,75);
	  pButton(241,177,276,194,ShowAstroUp(plr,&pos,&pos2),key>>8,77);
	  pButton(278,177,313,194,ShowAstroFor(plr,&pos,&pos2),key>>8,79);
	  if(key>='A' && key<='Z') {
	      glorf=0;
	      while(abuf[glorf].Name[0] < key && glorf< Data->P[plr].AstroCount-1) glorf++;
	      pos=glorf;
	      DisplAst(plr,&pos,&pos2);
	      key=0;
	  }
     key=0;
  };
}

void DisplAst(char plr, char *where, char *where2)
{
  char temp[11]="GROUP \0";
  char Ast_Name[11];
  if(Data->P[plr].AstroCount == 0) return;
	
  RectFill(165,39,230,35,3);
  RectFill(165,49,230,45,3);
  RectFill(172,68,230,63,3);
  RectFill(211,78,229,73,3);
  RectFill(214,89,231,84,3);
  RectFill(250,107,263,102,3);
  RectFill(223,115,241,110,3);
  RectFill(202,123,218,118,3);
  RectFill(216,131,240,126,3);
  RectFill(230,139,254,134,3);
  RectFill(248,149,272,144,3);
  RectFill(216,159,318,154,3);
  RectFill(220,169,280,164,3);
  RectFill(288,105,308,133,3);
  RectFill(239,83,306,97,3);
  grSetColor(1);
  if ((plr==0 && abuf[*where].Missions>=4) || abuf[*where].Hero==1) Display_ARROW(4,289,105);
   else if ((plr==1 && abuf[*where].Missions>=4) || abuf[*where].Hero==1) Display_ARROW(5,290,105);
  if (abuf[*where].Missions>0) Display_ARROW(6,245,83);
  DispNum(214,78,abuf[*where].Missions);
  DispNum(217,89,abuf[*where].Prestige);
  DispNum(250,149,abuf[*where].Days);
    PrintAt(258,149," DAYS");
  DispNum(253,107,abuf[*where].Cap);
  DispNum(227,115,abuf[*where].LM);
  DispNum(204,123,abuf[*where].EVA);
  DispNum(220,131,abuf[*where].Docking);
  DispNum(234,139,abuf[*where].Endurance);
  DispNum(173,68,abuf[*where].Active/2);
  PrintAt(0,0," YEAR");
  if (abuf[*where].Active/2!=1) PrintAt(0,0,"S");
  memset(Ast_Name,0x00,sizeof Ast_Name);
  strncpy(Ast_Name,abuf[*where].Name,10);
  if (abuf[*where].Sex==1) grSetColor(5);  // Show females in blue
  PrintAt(165,39,Ast_Name);  // Displays name of astronaut/cosmonaut
  grSetColor(11);
  strcat(temp,(char *)Nums[abuf[*where].Group]);
  PrintAt(165,49,temp);
  grSetColor(12);
  DispNum(225,169,*where+1);
  PrintAt(0,0," OF ");
  DispNum(0,0,Data->P[plr].AstroCount);
  DispLoc(plr,where);
  DisplAstData(plr,where,where2);  
  GradRect(234,30,313,79,plr);
  AstFaces(plr,234,30,abuf[*where].Face); //30
  
}

void ShowAstroUp(char plr,char *where,char *where2)
{
  if(*where == Data->P[plr].AstroCount - 1) return;
  *where2=0;
  (*where)++;
  DisplAst(plr,where,where2);
}

void ShowAstroDown(char plr, char *where,char *where2)
{
  if(*where == 0) return;
  *where2=0;
  (*where)--;
  DisplAst(plr,where,where2);
}

void ShowAstroBack(char plr, char *where,char *where2)
{
  if(*where == 0) return;
  *where=0;
  *where2=0;
  DisplAst(plr,where,where2);
}

void ShowAstroFor(char plr, char *where,char *where2)
{
  if(*where == Data->P[plr].AstroCount - 1) return;
  *where=Data->P[plr].AstroCount-1;
  *where2=0;
  DisplAst(plr,where,where2);  
}

void DispLoc(char plr,char *where)
{
  int ass;
  
  grSetColor(9);
  ass=abuf[*where].Assign;
  switch(abuf[*where].Status){
  case 0:if (ass==0)
          {
	        if(plr==0) PrintAt(216,159,"ASTRONAUT COMPLEX");
	         else PrintAt(216,159,"COSMONAUT CENTER");
           return;
          }
         PrintAt(216,159,Data->P[plr].Manned[ass-1].Name);PrintAt(0,0," PROGRAM");
         return;
  case 1:if (plr==0) PrintAt(216,159,"ARLINGTON CEMETERY");
          else PrintAt(216,159,"KREMLIN WALL");
         return;
  case 2:PrintAt(216,159,"RETIRED");return;
  case 3:PrintAt(216,159,"INJURED");return;
  case 4: case 5: case 6:PrintAt(216,159,"BASIC TRAINING");return;      
  case 7: case 8: case 9: case 10:
        switch(abuf[*where].Focus)
         {
          case 1:PrintAt(216,159,"ADV. CAPSULE");return;      	
          case 2:PrintAt(216,159,"ADV. L.E.M.");return;      	
          case 3:PrintAt(216,159,"ADV. E.V.A.");return;      	
          case 4:PrintAt(216,159,"ADV. DOCKING");return;      		  	  
          case 5:PrintAt(216,159,"ADV. ENDURANCE");return;      	
         }
   }
}  

void DisplAstData(char plr, char *where,char *where2)
{
  int num=abuf[*where].MissionNum[*where2],num2;
  
  RectFill(1,40,157,182,3);
  if(abuf[*where].Missions == 0){
    gxPutImage(&vhptr2,gxSET,22,69,0);
	 
    return;
  }
  grSetColor(2);
  pline(20,111,138,111);
  grSetColor(4);
  pline(20,113,138,113);


  if (Data->P[plr].History[num].Hard[0][0]!=-1)
    PatchMe(plr,7,41,Data->P[plr].History[num].Hard[0][0],Data->P[plr].History[num].Patch[0],32);
     else PatchMe(plr,7,41,Data->P[plr].History[num].Hard[1][0],Data->P[plr].History[num].Patch[1],32);

  grSetColor(1);
  PrintAt(43,53,"PRESTIGE: ");
  DispNum(93,53,Data->P[plr].History[num].Prestige);
  PrintAt(43,63,"DURATION: ");
  switch(Data->P[plr].History[num].Duration)
   {
    case 1:PrintAt(93,63,"A");break;
    case 2:PrintAt(93,63,"B");break;
    case 3:PrintAt(93,63,"C");break;
    case 4:PrintAt(93,63,"D");break;
    case 5:PrintAt(93,63,"E");break;
    case 6:PrintAt(93,63,"F");break;
    default:break;
   }
  PrintAt(10,83,"DESCRIPTION: ");

  grSetColor(9);
  PrintAt(43,45,&Data->P[plr].History[num].MissionName[0][0]);

  GetMisType(Data->P[plr].History[num].MissionCode);
  PrintAt(10,93,Mis.Abbr);

  if(*where2==abuf[*where].Missions-1){
    RectFill(1,114,157,184,3);
	 
    return;
  }
  num2=abuf[*where].MissionNum[*where2+1];
  if (num2<1 && num2>56) return;

  //astro history patch fix
  if (Data->P[plr].History[num2].Hard[0][0]!=-1)
    PatchMe(plr,7,116,Data->P[plr].History[num2].Hard[0][0],Data->P[plr].History[num2].Patch[0],32);
     else PatchMe(plr,7,116,Data->P[plr].History[num2].Hard[1][0],Data->P[plr].History[num2].Patch[1],32);

  grSetColor(9);
  PrintAt(43,120,&Data->P[plr].History[num2].MissionName[0][0]);

  GetMisType(Data->P[plr].History[num2].MissionCode);
  PrintAt(10,168,Mis.Abbr);

  grSetColor(1);
  PrintAt(43,128,"PRESTIGE: ");
  DispNum(93,128,Data->P[plr].History[num2].Prestige);
  PrintAt(43,138,"DURATION: ");
  switch(Data->P[plr].History[num2].Duration)
  {
   case 1:PrintAt(93,138,"A");break;
   case 2:PrintAt(93,138,"B");break;
   case 3:PrintAt(93,138,"C");break;
   case 4:PrintAt(93,138,"D");break;
   case 5:PrintAt(93,138,"E");break;
   case 6:PrintAt(93,138,"F");break;
   default:break;
  }
  PrintAt(10,158,"DESCRIPTION: ");
  
  return;
}

void DownAstroData(char plr, char *where,char *where2)
{
 if(*where2 == 0) return;
  else *where2-=2;
 DisplAstData(plr,where,where2);
 return;
}

void UpAstroData(char plr,char *where,char *where2)
{
 if ((*where2+2)<=abuf[*where].Missions-1) *where2+=2;
  else return;
// if(*where2 == abuf[*where].Missions-1) return;
//  else *where2+=2;
 DisplAstData(plr,where,where2); 
 return;
}

int astcomp(const void *no1, const void *no2)
{
  return strcmp(((struct Astros *) no1)->Name,((struct Astros *) no2)->Name);
}

// EOF
