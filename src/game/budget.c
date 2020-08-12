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

#define DELAYCNT 10

int MisCod;  // Variable to store Mission Code (for knowing when to display Duration level)

extern struct mStr Mis;
char olderMiss;

void DrawBudget(char player,char *pStatus)
{
  int i,j,max = 0,k,pscale;
  char name[20],str[10];


  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(0,0,319,47);ShBox(0,49,319,67);ShBox(0,69,158,199);
  ShBox(161,69,319,199);IOBox(165,27,238,43);
  InBox(3,3,74,43);InBox(243,3,314,43);InBox(184,87,313,167);
  InBox(166,181,313,194);InBox(4,73,154,133);InBox(4,138,154,196);
  InBox(133,140,152,152);InBox(133,154,152,166);
  InBox(133,168,152,180);InBox(133,182,152,194);
  Flag(4,4,player);
  grSetColor(1);
  PrintAt(180,37,"CONTINUE"); PrintAt(60,81,"PRESTIGE");
  PrintAt(43,145,"EXPENDITURES"); PrintAt(212,79,"BUDGET TRACK");
  InBox(184,87,313,167);GradRect(185,88,312,166,player);
  GradRect(30,86,140,120,player);GradRect(31,149,124,182,player);
  grSetColor(4);
  // Draw Prestige Box
  Box(30,148,125,183);
  Box(57,85,85,121);Box(85,121,113,85);Box(29,94,141,103);
  Box(29,103,141,112);Box(54,148,77,183);Box(77,148,101,183);
  Box(30,157,125,165);Box(30,165,125,174);InBox(30,148,125,183);
  InBox(29,85,141,121);
  // Draw the Prestige Screen
  k = (player==0) ? 0 : 1;   //max only checks your prestige and guessed
  for(i=0;i<5;i++)           // value for other player
	{
		max = (max > abs(Data->P[player].PrestHist[i][k])) ? max
		   : abs(Data->P[player].PrestHist[i][k]);
	}
  if (player == 0 ) { j=1; k=1;}
  else {j=0;k=1;}
  for(i=0;i<5;i++)
	{
 	  max = (max > abs(Data->P[j].PrestHist[i][k])) ? max
		   : abs(Data->P[j].PrestHist[i][k]);
	}

	if(max<20)
	{
		max=20;
		DispNum(6,114,-10);
		DispNum(6,122,-20);
		DispNum(11,96,10);
		DispNum(11,87,20);
	}
	else
	{
		DispNum(6,122,-max);
		DispNum(6,114,-max/2);
		DispNum(11,96,max/2);
		DispNum(11,87,max);
	}

  pscale=max>>1;		// Half the estimated prestige

  BudPict(player);

	if (player==0) {
		i=0;j=1;grSetColor(5);
	}
	else {
		i=1;j=0;grSetColor(8);
	}

  grMoveTo(30,103-Data->P[i].PrestHist[4][0]*8/pscale);
  grLineTo(57,103-Data->P[i].PrestHist[3][0]*8/pscale);
  grLineTo(85,103-Data->P[i].PrestHist[2][0]*8/pscale);
  grLineTo(113,103-Data->P[i].PrestHist[1][0]*8/pscale);
  grLineTo(140,103-Data->P[i].PrestHist[0][0]*8/pscale);

  if(player==0) {grSetColor(8);}
  else grSetColor(5);

  grMoveTo(30,103-Data->P[j].PrestHist[4][1]*8/pscale);
  grLineTo(57,103-Data->P[j].PrestHist[3][1]*8/pscale);
  grLineTo(85,103-Data->P[j].PrestHist[2][1]*8/pscale);
  grLineTo(113,103-Data->P[j].PrestHist[1][1]*8/pscale);
  grLineTo(140,103-Data->P[j].PrestHist[0][1]*8/pscale);

  grSetColor(5);
  PrintAt(165,89,"200");PrintAt(167,109,"150");
  PrintAt(167,129,"100");PrintAt(171,149,"50");
  PrintAt(164,169,"0 MB");
  grSetColor(1);
  if (Data->Season==0) strcpy(&name[0],"SPRING 19"); else strcpy(&name[0],"FALL 19");
  sprintf(&str[0],"%d",Data->Year);

  strcat(&name[0],&str[0]);
  DispBig(122,5,&name[0],0,-1);
  //DispNum(0,0,Data->Year);

  grSetColor(11);
  PrintAt(177,59,"PROJECTED BUDGET: ");DispMB(0,0,Data->P[player].Budget);
  PrintAt(42,59,"CASH: ");DispMB(0,0,Data->P[player].Cash);

  grSetColor(1);PrintAt(13,105,"0");
  grSetColor(5);
  if (Data->Season==1)
  {
	 DispNum(21,130,Data->Year-2);DispChr('F');
	 DispNum(49,130,Data->Year-1);DispChr('S');
	 DispNum(77,130,Data->Year-1);DispChr('F');
	 DispNum(105,130,Data->Year);DispChr('S');
	 DispNum(133,130,Data->Year);DispChr('F');
  }
  else
  {
	 DispNum(21,130,Data->Year-2);DispChr('S');
	 DispNum(49,130,Data->Year-2);DispChr('F');
	 DispNum(77,130,Data->Year-1);DispChr('S');
	 DispNum(105,130,Data->Year-1);DispChr('F');
	 DispNum(133,130,Data->Year);DispChr('S');
  }
  if (Data->Season==1)
  {
	 DispNum(23,193,Data->Year-2);DispChr('F');
	 DispNum(46,193,Data->Year-1);DispChr('S');
	 DispNum(68,193,Data->Year-1);DispChr('F');
	 DispNum(92,193,Data->Year);DispChr('S');
	 DispNum(114,193,Data->Year);DispChr('F');
  }
  else
  {
	 DispNum(23,193,Data->Year-2);DispChr('S');
	 DispNum(46,193,Data->Year-2);DispChr('F');
	 DispNum(68,193,Data->Year-1);DispChr('S');
	 DispNum(92,193,Data->Year-1);DispChr('F');
	 DispNum(114,193,Data->Year);DispChr('S');
  }
  grSetColor(4);
  grMoveTo(187,147);grLineTo(311,147);
  grMoveTo(187,127);grLineTo(311,127);
  grMoveTo(187,107);grLineTo(311,107);
  for (i=187;i<312;i+=2) {
	 grPutPixel(i,157,4);
	 grPutPixel(i,137,4);
	 grPutPixel(i,117,4);
	 grPutPixel(i,97,4);
  }
  grSetColor(4);
  DispNum(298,174,Data->Year);
  DispNum(271,174,Data->Year-1);
  DispNum(248,174,Data->Year-2);
  DispNum(222,174,Data->Year-3);
  DispNum(194,174,Data->Year-4);

  if (player==0) {
	 for (i=0;i<5;i++) {
		RectFill(197+i*26,164-(Data->P[0].BudgetHistory[Data->Year-57+i]*74)/200,190+i*26,164,6);
		RectFill(206+i*26,164-(Data->P[1].BudgetHistoryF[Data->Year-57+i]*74)/200,199+i*26,164,9);
		RectFill(196+i*26,164-(Data->P[0].BudgetHistory[Data->Year-57+i]*74)/200,190+i*26,163,5);
		RectFill(205+i*26,164-(Data->P[1].BudgetHistoryF[Data->Year-57+i]*74)/200,199+i*26,163,8);
	 }
	 RectFill(170,185,176,189,6);
	 RectFill(170,185,175,188,5);
	 RectFill(297,185,303,189,9);
	 RectFill(297,185,302,188,8);
	 grSetColor(1);
	 PrintAt(181,189,"U.S.A.");
	 PrintAt(262,189,"U.S.S.R.");
  }
  if (player==1) {
	 for (i=0;i<5;i++) {
		RectFill(197+i*26,164-(Data->P[1].BudgetHistory[Data->Year-57+i]*74)/200,190+i*26,164,9);
		RectFill(206+i*26,164-(Data->P[0].BudgetHistoryF[Data->Year-57+i]*74)/200,199+i*26,164,6);
		RectFill(196+i*26,164-(Data->P[1].BudgetHistory[Data->Year-57+i]*74)/200,190+i*26,163,8);
		RectFill(205+i*26,164-(Data->P[0].BudgetHistoryF[Data->Year-57+i]*74)/200,199+i*26,163,5);
	 }
	 RectFill(170,185,176,189,9);
	 RectFill(170,185,175,188,8);
	 RectFill(297,185,303,189,6);
	 RectFill(297,185,302,188,5);
	 grSetColor(1);
	 PrintAt(181,189,"U.S.S.R.");
	 PrintAt(262,189,"U.S.A.");
  }
  DrawPastExp(player,pStatus);
  FadeIn(2,pal,10,0,0);

  return;
}

void BudPict(char poff)
{
	PatchHdrSmall P;
  GXHEADER local;
  unsigned int i,x,y;
  FILE *in;
  in=sOpen("BUDD.BUT","rb",0);
  fseek(in,(poff)*(sizeof P),SEEK_CUR);
  fread(&P,sizeof P,1,in);
	SwapPatchHdrSmall(&P);
  fseek(in,P.offset,SEEK_SET);
  GV(&local,P.w,P.h);
  fread(local.vptr,P.size,1,in);
  //RLED(buffer,local.vptr,P.size);
  gxPutImage(&local,gxSET,245,4,0);
  DV(&local);
  x=134;y=141;
  for (i=2;i<6;i++)
	{
	 y=141+((i-2)*14);
	 fseek(in,(i)*(sizeof P),SEEK_SET);
	 fread(&P,sizeof P,1,in);
	SwapPatchHdrSmall(&P);
	 fseek(in,P.offset,SEEK_SET);
	 GV(&local,P.w,P.h);
	 fread(local.vptr,P.size,1,in);
       //  RLED(buffer,local.vptr,P.size);
	 gxPutImage(&local,gxSET,x,y,0);
   DV(&local);
	}
  fclose(in);
}


void DrawPastExp(char player,char *pStatus)
{
int i,j;
int max=0;
int pScale=25;

  RectFill(31,149,124,182,7+3*player);
  grSetColor(4);
  Box(30,148,125,183);Box(54,148,77,183);Box(77,148,101,183);
  Box(30,157,125,165);Box(30,165,125,174);

  for(j=0;j<5;j++)
	for(i=0;i<4;i++)
	  max = (max > Data->P[player].Spend[j][i]) ? max : Data->P[player].Spend[j][i];

  if(max<=100)
  {
	PrintAt(12,150,"100");PrintAt(12,159,"75");
	PrintAt(12,167,"50");PrintAt(12,176,"25");
	PrintAt(8,185,"0 MB");
  }
  else
  {
	pScale = max>>2;
	DispNum(12,150,max);
	i = (max*3)>>2;DispNum(12,159,i);
	i = max>>1;DispNum(12,167,i);
	i = max>>2;DispNum(12,176,i);
	PrintAt(8,185,"0 MB");
  };

  for(i=0;i<4;i++)
	 {
		if (pStatus[i]==1)
		{
	switch(i) {
	 case 0: grSetColor(11);break;
	 case 1: grSetColor(8);break;
	 case 2: grSetColor(5);break;
	 case 3: grSetColor(16);break;
	 }
		  grMoveTo(31,182-(Data->P[player].Spend[4][i]*8)/pScale);
		  grLineTo(54,182-(Data->P[player].Spend[3][i]*8)/pScale);
		  grLineTo(77,182-(Data->P[player].Spend[2][i]*8)/pScale);
		  grLineTo(101,182-(Data->P[player].Spend[1][i]*8)/pScale);
		  grLineTo(124,182-(Data->P[player].Spend[0][i]*8)/pScale);
		}
	 };
  InBox(30,148,125,183);
}

void Budget(char player)
{
 char pStatus[]={1,1,1,1};
 DrawBudget(player,pStatus);
 strcpy(IDT,"i007\0");strcpy(IKEY,"k007\0");
 WaitForMouseUp();
 while (1)
  {
   key=0;GetMouse();
   if (mousebuttons > 0 || key>0)  /* Game Play */
    {
     if ((x>=166 && y>=29 && x<=236 && y<=41 && mousebuttons>0) || key==K_ENTER)
      {
	    InBox(167,29,236,41);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    OutBox(167,29,236,41);
	    return;  /* Done */
		};
	  if ((x>=133 && y>=140 && x<=152 && y<152 && mousebuttons>0) || key=='U')
		{
		 pStatus[0] = (pStatus[0]==0) ? 1 : 0;

		 if (pStatus[0]==1) InBox(133,140,152,152);
		   else OutBox(133,140,152,152);

		 WaitForMouseUp();
		 DrawPastExp(player,pStatus);
		}
	 if ((x>=133 && y>=154 && x<=152 && y<166 && mousebuttons>0) || key=='R')
		{
		 pStatus[1] = (pStatus[1]==0) ? 1 : 0;

		 if (pStatus[1]==1) InBox(133,154,152,166);
			else OutBox(133,154,152,166);

		 WaitForMouseUp();
		 DrawPastExp(player,pStatus);
		}
	 if ((x>=133 && y>=168 && x<=152 && y<180 && mousebuttons>0) || key=='C')
		{
		 pStatus[2] = (pStatus[2]==0) ? 1 : 0;

		 if (pStatus[2]==1) InBox(133,168,152,180);
			else OutBox(133,168,152,180);

		 WaitForMouseUp();
		 DrawPastExp(player,pStatus);
		}
	 if ((x>=133 && y>=182 && x<=152 && y<194 && mousebuttons>0) || key=='M')
		{
		 pStatus[3] = (pStatus[3]==0) ? 1 : 0;

		 if (pStatus[3]==1) InBox(133,182,152,194);
			else OutBox(133,182,152,194);

		 WaitForMouseUp();
		 DrawPastExp(player,pStatus);
      }
	}
  };
}

void DrawPreviousMissions (char plr)
{
	int i, misnum=0;
	 extern char *Months[12];
	InBox(5,41,314,91);RectFill(6,42,313,90,0);
	i=Data->P[plr].PastMis-olderMiss;
  grSetColor(2);
  while (i > (Data->P[plr].PastMis-olderMiss-3) && i>=0) {

     GetMisType(Data->P[plr].History[i].MissionCode);

     PrintAt(9,49+16*misnum,Data->P[plr].History[i].MissionName[0]);
     PrintAt(9,55+16*misnum,Mis.Abbr);
     MisCod=Data->P[plr].History[i].MissionCode;
     if ((MisCod>24 && MisCod<32) || MisCod==33 || MisCod==34 || MisCod==35 || MisCod==37 || MisCod==40 || MisCod==41)
     {
	switch(Data->P[plr].History[i].Duration)
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

     PrintAt(140,49+16*misnum,"PRESTIGE: ");DispNum(0,0,Data->P[plr].History[i].Prestige);

     PrintAt(230,49+16*misnum,Months[Data->P[plr].History[i].Month]);
     PrintAt(0,0," 19");
     DispNum(0,0,Data->P[plr].History[i].MissionYear);
     i--;misnum++;
  };
  if (misnum<3) {
     grSetColor(1);
     PrintAt(9,49+16*misnum,"NO PAST MISSIONS REMAINING");
  }
}

void DrawViewing(char plr)
{

  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  memset(buffer,0x00,BUFFER_SIZE);
  ShBox(0,0,319,22);InBox(3,3,30,19);FlagSm(plr,4,4);
  IOBox(243,3,316,19);

  ShBox(0,24,319,95);
  InBox(81,28,238,38);RectFill(82,29,237,37,7);
  InBox(5,41,314,91);RectFill(6,42,313,90,0);

  ShBox(0,97,319,199);
  InBox(5,101,314,112);RectFill(6,102,313,111,10);
  InBox(5,114,314,178);RectFill(6,115,313,177,0);
  IOBox(4,182,77,196);IOBox(242,182,315,196);
  LTArrow(24,186);RTArrow(262,186);
  InBox(81,182,238,196);RectFill(82,183,237,195,7);
  ShBox(302,116,312,145);UPArrow(304,118);
  ShBox(302,147,312,176);DNArrow(304,149);

  grSetColor(11);
  PrintAt(113,35,"PREVIOUS MISSIONS");
  PrintAt(106,108,"PAST CURRENT EVENTS");

  grSetColor(1);PrintAt(258,13,"CONTINUE");
  DispBig(45,5,"VIEWING STAND",0,-1);

  DrawPreviousMissions (plr);
  IOBox(4,26,77,40);IOBox(242,26,315,40); InBox(244,28,313,38);
  if (Data->P[plr].PastMis<4) InBox(6,28,75,38);
  LTArrow(24,30);RTArrow(262,30);

  return;
}

void DrawVText(char got)
{
  int xx=10,yy=122,i;
  char *buf;
  buf=buffer;
  grSetColor(1);
  for (i=0;i<got;i++) {
    while(*buf!='x') buf++;
    buf++;
    if (strncmp(&buf[0],"IN THE NEWS TODAY",18)==0) grSetColor(11);
    if (strncmp(&buf[0],"DEVELOPMENTS IN THE WORLD",24)==0) grSetColor(11);
    if (strncmp(&buf[0],"ASTRONAUTS IN THE NEWS",22)==0) grSetColor(11);
    if (strncmp(&buf[0],"ALSO IN THE NEWS",16)==0) grSetColor(12);
    if (strncmp(&buf[0],"IN COSMONAUT NEWS",17)==0) grSetColor(11);
    if (strncmp(&buf[0],"OTHER EVENTS IN THE NEWS",24)==0) grSetColor(12);
    if (strncmp(&buf[0],"PLANETARY",9)==0) grSetColor(11);
    if (strncmp(&buf[0],"CHECK INTEL",11)==0) grSetColor(11);
  }

  for(i=0;i<8;i++) {
    RectFill(6,yy-4,300,yy+1,0);
    grMoveTo(xx,yy);
    if (strncmp(&buf[0],"ASTRONAUTS IN THE NEWS",22)==0) grSetColor(11);
    if (strncmp(&buf[0],"ALSO IN THE NEWS",16)==0) grSetColor(12);
    if (strncmp(&buf[0],"IN COSMONAUT NEWS",17)==0) grSetColor(11);
    if (strncmp(&buf[0],"OTHER EVENTS IN THE NEWS",24)==0) grSetColor(12);
    if (strncmp(&buf[0],"AND THAT'S THE NEWS",19)==0) grSetColor(11);
    if (strncmp(&buf[0],"PLANETARY",9)==0) grSetColor(11);
    if (strncmp(&buf[0],"THIS CONCLUDES OUR NEWS",23)==0) grSetColor(11);
    if (strncmp(&buf[0],"CHECK INTEL",11)==0) grSetColor(11);

    while(buf[0]!='x'&& buf[0]!='\0') {DispChr(buf[0]);buf++;};
    yy+=7;
    buf++;
    if (*buf=='\0') i=9;
  }
  return;
}

int RetFile(char plr,int card)
{
  ONEWS oNews;
  int bline,i;
  FILE *fin;
  memset(buffer,0x00,BUFFER_SIZE);
  fin=sOpen("EVENT.TMP","rb",1);
  fseek(fin,(card+plr*42)*(sizeof (struct oldNews)),SEEK_SET);
  fread(&oNews,sizeof(struct oldNews),1,fin);
  fseek(fin,oNews.offset,SEEK_SET);
  fread(buffer,oNews.size,1,fin);
  fclose(fin);
  bline=0;
  for (i=0;i<(int)strlen(buffer);i++) if (buffer[i]=='x') bline++;
  bline-=8;

  RectFill(82,183,237,195,7);
  grSetColor(11);
  if ((card%2)==1) PrintAt(131,191,"FALL 19");
  else PrintAt(128,191,"SPRING 19");
  DispNum(0,0,(card>>1)+57);

  return bline;
}

void Viewing(char plr)
{
  int ctop,bline=0,oset,maxcard;
  olderMiss=1;
  DrawViewing(plr);
  maxcard=oset=Data->P[plr].eCount-1;
  bline=RetFile(plr,oset);
  ctop=0;
  DrawVText(ctop);
	InBox(244,184,313,194);
  FadeIn(2,pal,10,0,0);
  music_start(M_SOVTYP);
  WaitForMouseUp();
  while (1)
  {
   key=0;GetMouse();
     if (ctop>0 && key==0x4900) {  // Page Up Key
       ctop-=9;
       if (ctop<0) ctop=0;
       DrawVText(ctop);
       bzdelay (DELAYCNT);
     }
     if (ctop<bline && key==0x5100) {  // Page Down Key
       ctop+=9;
       if (ctop>bline) ctop=bline;
       DrawVText(ctop);
       bzdelay (DELAYCNT);
     }
     if (ctop>0 && ((mousebuttons>0 && x>=302 && y>=116 && x<=312 && y<=145) || key==UP_ARROW)) {
	     InBox(302,116,312,145);
       ctop--; DrawVText(ctop);
       bzdelay (DELAYCNT / 2);
       OutBox(302,116,312,145);
     } // UP
     else if (ctop<bline && ((mousebuttons>0 && x>=302 && y>147 && x<=312 && y<=176) || key==DN_ARROW)) {
       InBox(302,147,312,176);
       ctop++; DrawVText(ctop);
       bzdelay (DELAYCNT / 2);
       OutBox(302,147,312,176);
      }  // Down
     else if (oset>0 && ((mousebuttons>0 && x>=6 && y>=184 && x<=75 && y<=194) || key==LT_ARROW))
      {
	    InBox(6,184,75,194);
      oset--;
      bline=RetFile(plr,oset);
      ctop=0; DrawVText(ctop);

	    if (oset!=0) OutBox(6,184,75,194);
      bzdelay (DELAYCNT);
      OutBox(244,184,313,194);

	    // Left Select
      }
     else if (oset<maxcard && ((mousebuttons>0 && x>=244 && y>=184 && x<=313 && y<=194) || key==RT_ARROW)) {
	    InBox(244,184,313,194);
      oset++;
      bline=RetFile(plr,oset);
      ctop=0; DrawVText(ctop);

	    if (oset!=maxcard) OutBox(244,184,313,194);
      bzdelay (DELAYCNT);
      OutBox(6,184,75,194);

      // Right Select
      }
		else if ((mousebuttons>0 && x>=245 && y>=5 && x<=314 && y<=17) || key==K_ENTER || key==K_ESCAPE) {
	     InBox(245,5,314,17);
        if (key>0) delay(150);
	     //OutBox(245,5,314,17);
        music_stop();
	     return;  // Continue
       }
		else if (key=='O' || (mousebuttons>0 && x>=6 && y>=28 && x<=75 && y<=38)) {
			olderMiss++;
			if (olderMiss>Data->P[plr].PastMis-2) olderMiss=Data->P[plr].PastMis-2;
			if (olderMiss<1) olderMiss=1;
			if (olderMiss!=1)OutBox(244,28,313,38); //Boton Newer
			InBox(6,28,75,38); //Botton Older
      		// Debe dibujar la mission
      		DrawPreviousMissions (plr);
      		bzdelay (DELAYCNT);
      		if (olderMiss!=Data->P[plr].PastMis-2 && Data->P[plr].PastMis>3) OutBox(6,28,75,38); //Boton Older
			}
		else if (key=='N' || (mousebuttons>0 && x>=244 && y>=28 && x<=313 && y<=38))  {
			olderMiss--;
			if (olderMiss<1) olderMiss=1;
			if (olderMiss!=Data->P[plr].PastMis-2 && Data->P[plr].PastMis>3) OutBox(6,28,75,38); //Boton Older
			InBox(244,28,313,38); //Botton Newer
      		// Debe dibujar la mission
      		DrawPreviousMissions (plr);
      		bzdelay (DELAYCNT);
      		if (olderMiss!=1)OutBox(244,28,313,38); //Boton Newer
			}
  }
}
/* EOF */
