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

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>

char MCol[110],sel[25],MaxSel;
int missions;    // Variable for how many missions each 'naut has flown

void Moon(char plr)
{
  int val;
	SimpleHdr table;
  FILE *in;
  GXHEADER local;
  long size;
  strcpy(IDT,"i029");strcpy(IKEY,"k029");
  val=Data->P[plr].Misc[5].Safety;
  FadeOut(2,pal,0,0,0);

  
  gxClearDisplay(0,0);
  ShBox(109,24,222,167);
  InBox(113,27,218,39);RectFill(114,28,217,38,7+3*plr);
  InBox(113,130,218,146);RectFill(114,131,217,145,7+3*other(plr));
  IOBox(113,150,218,164);

  size=(int) ((val-55)/3);
  if (size>13) size=13;

  in=sOpen("MOON.BUT","rb",0);
  fseek(in,sizeof_SimpleHdr * size,SEEK_SET);
  fread_SimpleHdr(&table,1,in);
  fseek(in,table.offset,SEEK_SET);
  GV(&local,104,82);
  fread(&pal[384],384,1,in);  // Individual Palette
  fread(buffer,table.size,1,in);  // Get Image
  fclose(in);
  RLED_img(buffer,local.vptr,table.size,local.w,local.h);
  for (size=0;size<(104*82);size++) local.vptr[size]+=128;
  gxPutImage(&local,gxSET,114,43,0);
  DV(&local);
  InBox(113,42,218,125);
  ShBox(113,42,143,60);RectFill(113,42,142,59,3);InBox(113,42,140,58);  
  grSetColor(11);
  PrintAt(117,35,"PHOTO RECON.");
  PrintAt(118,137,"CURRENT RECON ");
  PrintAt(118,143,"LEVEL IS AT ");
  DispNum(0,0,val);PrintAt(0,0,"%");
  grSetColor(1);PrintAt(144,159,"CONTINUE");
  FlagSm(plr,114,43);
  
  music_start(M_HISTORY);
  FadeIn(2,pal,0,0,0);
  WaitForMouseUp();
  while (1)
  {
	key=0;GetMouse();
	 if ((x>=115 && y>=152 && x<=216 && y<=162 && mousebuttons>0) || key==K_ENTER) {
	   InBox(115,152,216,162);
	   WaitForMouseUp();
	   if (key>0) delay(150);
      music_stop();
	   key=0;return; // Continue
	  }
  }
}



void DispLeft(char plr,int lc,int cnt,int nw,int *ary)
{
  int i,start,num;

  start=nw-lc;
  if (cnt<8) num=cnt; else num=8;
  grSetColor(11);
  for (i=start;i<start+num;i++) {
    if (ary[i]>=0) {
	    grSetColor(11+7*Data->P[plr].Pool[ary[i]].Sex);
	    if (Data->P[plr].Pool[ary[i]].RDelay>0) grSetColor(7); 
	      // Print name in purple if 'naut has announced retirement (black shows poorly here) -Leon
	    PrintAt(28,136+(i-start)*8,&Data->P[plr].Pool[ary[i]].Name[0]);
	    if (Data->P[plr].Pool[ary[i]].Missions>0)
	     {
	      missions=Data->P[plr].Pool[ary[i]].Missions;
	      PrintAt(0,0," (");DispNum(0,0,missions);PrintAt(0,0,")");
	     }
    }
  }
  return;
}

void BarSkill(char plr,int lc,int nw,int *ary)
{
  grSetColor(11);
  if (Data->P[plr].Pool[ary[nw]].Sex==1) grSetColor(18);  // Print in blue if 'naut is female
  grMoveTo(28,136+lc*8);
  // CA LM EV DO EN
  PrintAt(0,0,"CA:");DispNum(0,0,Data->P[plr].Pool[ary[nw]].Cap);
  PrintAt(0,0," LM:");DispNum(0,0,Data->P[plr].Pool[ary[nw]].LM);
  PrintAt(0,0," EV:");DispNum(0,0,Data->P[plr].Pool[ary[nw]].EVA);
  PrintAt(0,0," DO:");DispNum(0,0,Data->P[plr].Pool[ary[nw]].Docking);
  PrintAt(0,0," EN:");DispNum(0,0,Data->P[plr].Pool[ary[nw]].Endurance);
  return;
}


void SatDraw(char plr)
{
	PatchHdrSmall P;
  int i,loc[4];
  FILE *fin;

  GXHEADER local;
  FadeOut(2,pal,0,0,0);
  
  gxClearDisplay(0,0);
  ShBox(1,0,319,22);IOBox(243,3,316,19);InBox(4,3,31,19);

  for (i=0;i<4;i++) {
   ShBox(1+i*80,24,79+i*80,71);ShBox(1+i*80,73,79+i*80,199);
   if (i==2) {
	  InBox(4+i*80,27,76+i*80,56);
	  RectFill(5+i*80,28,75+i*80,55,0);
	  InBox(164,58,236,68); RectFill(165,59,235,67,0);
	  ShBox(166,60,199,66);ShBox(201,60,234,66);
	  }
   else {
	  InBox(4+i*80,27,76+i*80,68);
	  RectFill(5+i*80,28,75+i*80,67,7);
	  }
   }

  loc[0]=(Data->P[plr].Misc[4].Num<0) ? 0 : 1+plr*3;
  loc[1]=(Data->P[plr].Probe[0].Num<0) ? 0 : 2+plr*3;
  loc[2]=(Data->P[plr].Probe[1].Num<0) ? 0 : 0;
  loc[3]=(Data->P[plr].Probe[2].Num<0) ? 0 : 3+plr*3;

  fin=sOpen("SATBLD.BUT","rb",0);
  fread(&pal[0],768,1,fin);

  for (i=0;i<4;i++) {
   
   fseek(fin,(sizeof P)*loc[i]+768,SEEK_SET);
   fread(&P,sizeof P,1,fin);
	SwapPatchHdrSmall(&P);
   GV(&local,P.w,P.h);
   fseek(fin,P.offset,SEEK_SET);
   fread(buffer,P.size,1,fin);
   RLED_img(buffer,local.vptr,P.size,local.w,local.h);
   if (i!=2) gxPutImage(&local,gxSET,5+i*80,28,0);
   else gxVirtualDisplay(&local,0,0,5+i*80,28,75+i*80,55,0);
   DV(&local);
   }
  fclose(fin);

  FlagSm(plr,5,4);
  grSetColor(11);
  grMoveTo(180,63);grLineTo(184,63);
  grMoveTo(215,63);grLineTo(219,63);
  grMoveTo(217,61);grLineTo(217,65);
  DispBig(40,5,"SATELLITE BUILDING",0,-1);
  grSetColor(1);
  PrintAt(258,13,"CONTINUE");
  
  return;
}


void LMDraw(char plr)
{
  char ind=0;
  FadeOut(2,pal,0,0,0);
  
  gxClearDisplay(0,0);
  ShBox(1,0,319,22);IOBox(243,3,316,19);InBox(4,3,31,19);
  ShBox(1,24,319,118);InBox(4,26,316,116);RectFill(5,27,315,115,0); // middle screen
  if (Data->P[plr].Manned[6].Num>=0) ind=4+plr; else ind=0+plr;
  LMPict(ind);ind=0;
  if (Data->P[plr].Manned[5].Num>=0) ind=6+plr; else ind=2+plr;
  LMPict(ind);
  ShBox(110,24,203,36);InBox(112,26,201,34);grSetColor(1);PrintAt(117,32,"CENTRAL HANGAR");
  FlagSm(plr,5,4);
  DispBig(40,5,"LUNAR MODULE",0,-1);
  grSetColor(1);PrintAt(258,13,"CONTINUE");
  
  return;
}

void SatText(char plr)
{
   int i;
   
   grSetColor(11);
   for (i=0;i<4;i++) {
      if (i!=2) {
	 grSetColor(11);
	 switch(i) {
	    case 0: PrintAt(5+i*80,80,"DUR LVL: ");  // Show highest Duration level achieved -Leon
		    switch(Data->P[plr].DurLevel)
		    {
		     case 1:PrintAt(0,0,"A");break;
		     case 2:PrintAt(0,0,"B");break;
		     case 3:PrintAt(0,0,"C");break;
		     case 4:PrintAt(0,0,"D");break;
		     case 5:PrintAt(0,0,"E");break;
		     case 6:PrintAt(0,0,"F");break;
		     default:PrintAt(0,0,"NONE");break;
		    }

		    PrintAt(5+i*80,94,"DOCKING");

		    if (Data->P[plr].Misc[4].Num>=0)
			   DispNum(5+i*80,110,Data->P[plr].Misc[4].Safety);
					else DispNum(5+i*80,110,0);
		    PrintAt(0,0,"%");

		    DispNum(5+i*80,127,Data->P[plr].Misc[4].Steps);

		    DispNum(5+i*80,144,Data->P[plr].Misc[4].Steps-Data->P[plr].Misc[4].Failures);

		    if (Data->P[plr].Misc[4].Steps > 0) DispNum(5+i*80,161,100*(Data->P[plr].Misc[4].Steps-Data->P[plr].Misc[4].Failures)/Data->P[plr].Misc[4].Steps);
		    else DispNum(5+i*80,161,0);

			if (Data->Prestige[24].Place==0) PrintAt(5+i*80,178,"U.S.");
		    else if (Data->Prestige[24].Place==1) PrintAt(5+i*80,178,"SOVIET");
		    else PrintAt(5+i*80,178,"NONE");

		    DispNum(5+i*80,195,Data->Prestige[24].Points[plr]);
		    break;

	    case 1: PrintAt(5+i*80,80,"ORBITAL SAT.");

					if (Data->P[plr].Probe[0].Num>=0)
		       DispNum(5+i*80,110,Data->P[plr].Probe[0].Safety);
		    else DispNum(5+i*80,110,0);
		    PrintAt(0,0,"%");
		   
		    DispNum(5+i*80,127,Data->P[plr].Probe[0].Used);

		    DispNum(5+i*80,144,Data->P[plr].Probe[0].Used-Data->P[plr].Probe[0].Failures);

		    if (Data->P[plr].Probe[0].Used > 0) DispNum(5+i*80,161,100*(Data->P[plr].Probe[0].Used-Data->P[plr].Probe[0].Failures)/Data->P[plr].Probe[0].Used);
		    else DispNum(5+i*80,161,0);

		    if (Data->Prestige[0].Place==0) PrintAt(5+i*80,178,"U.S.");
		    else if (Data->Prestige[0].Place==1) PrintAt(5+i*80,178,"SOVIET");
		    else PrintAt(5+i*80,178,"NONE");

		    DispNum(5+i*80,195,Data->Prestige[0].Points[plr]);
		    break;


	    case 3: PrintAt(5+i*80,80,"LUNAR PROBE");
		    if (Data->P[plr].Probe[2].Num>=0)
		       DispNum(5+i*80,110,Data->P[plr].Probe[2].Safety);
		    else DispNum(5+i*80,110,0);
		    PrintAt(0,0,"%");
		   
 			 DispNum(5+i*80,127,Data->P[plr].Probe[2].Used);

		    DispNum(5+i*80,144,Data->P[plr].Probe[2].Used-Data->P[plr].Probe[2].Failures);

		    if (Data->P[plr].Probe[2].Used > 0) DispNum(5+i*80,161,100*(Data->P[plr].Probe[2].Used-Data->P[plr].Probe[2].Failures)/Data->P[plr].Probe[2].Used);
		    else DispNum(5+i*80,161,0);

		    if (Data->Prestige[7].Place==0) PrintAt(5+i*80,178,"U.S.");
					else if (Data->Prestige[7].Place==1) PrintAt(5+i*80,178,"SOVIET");
		    else PrintAt(5+i*80,178,"NONE");

		    DispNum(5+i*80,195,Data->Prestige[7].Points[plr]);
		    break;

	    }
	 }
    else PlanText(plr,0);

    grSetColor(6+3*plr);
	 PrintAt(5+i*80,103,"SAFETY:");
	 PrintAt(5+i*80,120,"ATTEMPTS:");
	 PrintAt(5+i*80,137,"SUCCESSFUL:");
	 PrintAt(5+i*80,154,"% SUCCESS:");
	 PrintAt(5+i*80,171,"FIRST:");
	 PrintAt(5+i*80,188,"PRESTIGE:");

      }

   
   return;
}

void PlanText(char plr,char plan)
{
  char tx;
  int pUsed,pFails,Find,i;

  Find = 0; /* XXX check uninitialized */

  tx=(Data->P[plr].Probe[1].Num>=0) ? 1 : 0;
  
  RectFill(164,75,78+160,89,3);
  RectFill(162,105,218,113,3);RectFill(162,123,218,131,3);
  RectFill(162,157,218,165,3);
  RectFill(162,173,218,181,3);RectFill(162,190,218,198,3);

  RectFill(162,139,218,145,3);
  if (tx==1) RectFill(165,28,235,55,0);
  grSetColor(11);
  switch (plan) {
   case 0: PrintAt(4+160,80,"LUNAR FLYBY");
	   grSetColor(11);
	   PrintAt(4+160,87,"RECON: ");
	   DispNum(0,0,Data->P[plr].Misc[5].Safety);
	   PrintAt(0,0,"%");Find=7;
      if (tx==1) SmHardMe(plr,190,34,6,6,32);
	   break;
   case 1: PrintAt(4+160,80,"MERCURY FLYBY");
           SmHardMe(plr,190,34,6,1,32);Find=11;
           break;
   case 2: PrintAt(4+160,80,"VENUS FLYBY");
           SmHardMe(plr,190,34,6,2,32);Find=9;
           break;
   case 3: PrintAt(4+160,80,"MARS FLYBY");
           SmHardMe(plr,190,34,6,3,32);Find=10;
           break;
   case 4: PrintAt(4+160,80,"JUPITER FLYBY");
           SmHardMe(plr,190,34,6,4,32);Find=12;
           break;
   case 5: PrintAt(4+160,80,"SATURN FLYBY");
           SmHardMe(plr,190,34,6,5,32);
           Find=13;
           break;
   }

   pUsed=pFails=0;
   for (i=0;i<Data->P[plr].PastMis;i++)
     if (Data->P[plr].History[i].MissionCode==Find) {
        pUsed++;
        if (Data->P[plr].History[i].spResult!=1) pFails++;
     }

   DispNum(5+160,110,Data->P[plr].Probe[1].Safety*tx);PrintAt(0,0,"%");


   DispNum(5+160,127,pUsed);
   DispNum(5+160,144,pUsed-pFails);

   if (pUsed) DispNum(5+160,161,100*(pUsed-pFails)/pUsed);
   else DispNum(5+160,161,0);

   if (Data->Prestige[1+plan].Place==0) PrintAt(5+160,178,"U.S.");
   else if (Data->Prestige[1+plan].Place==1) PrintAt(5+160,178,"SOVIET");
   else PrintAt(5+160,178,"NONE");
   DispNum(5+160,195,Data->Prestige[1+plan].Points[plr]);

   
   return;
}



void LMPict(char poff)
{
  GXHEADER local,local2;
	SimpleHdr table;
  FILE *in;
  in=sOpen("LMER.BUT","rb",0);
  fread_SimpleHdr(&table,1,in);
  fseek(in,8*sizeof_SimpleHdr,SEEK_SET);
  fread(&pal[32*3],672,1,in);
  fseek(in,table.offset,SEEK_SET);
  fread(buffer,table.size,1,in);
  GV(&local,156,89); GV(&local2,156,89);
  RLED_img(buffer,local.vptr,table.size,local.w,local.h);
  fseek(in,(poff)*sizeof_SimpleHdr,SEEK_SET);
  fread_SimpleHdr(&table,1,in);
  fseek(in,table.offset,SEEK_SET);
  fread(buffer,table.size,1,in);
  RLED_img(buffer,local2.vptr,table.size,local2.w,local2.h);
  
  if (poff==0 || poff==1 || poff==4 || poff==5)
	gxPutImage(&local2,gxSET,5,27,0);
	   else gxPutImage(&local2,gxSET,160,27,0);
  
  fclose(in);
  DV(&local); DV(&local2);
  return;
}

void LMBld(char plr)
{
	int i,m;
  LMDraw(plr);
  
  ShBox(1,120,157,168);InBox(4,122,154,166);
  ShBox(160,120,319,168);InBox(163,122,316,166);
  ShBox(1,170,319,194);InBox(41,172,279,192);
  DispBig(71,176,"TOTAL LM POINTS",0,-1);
  memset(Name,0,sizeof Name);
  sprintf(&Name[0],"%d",Data->P[plr].LMpts);
  DispBig(240,176,&Name[0],0,-1);
  for (i=0;i<2;i++)
	{
	 if (i==0) m=172; else m=12;
	 grSetColor(8);PrintAt(m,130,&Data->P[plr].Manned[5+i].Name[0]);grSetColor(9);
	 grSetColor(6);
	 PrintAt(m+48,130,"AVOID FAILURE: ");
         grSetColor(11);
	 if (Data->P[plr].Manned[5+i].SaveCard>0) PrintAt(m+124,130,"YES");
   	   else PrintAt(m+126,130,"NO");
	 grSetColor(6);PrintAt(m,138,"SAFETY FACTOR: ");
    grSetColor(1);DispNum(0,0,(Data->P[plr].Manned[5+i].Num>=0) ? Data->P[plr].Manned[5+i].Safety : 0 );
    PrintAt(0,0,"%");
	 grSetColor(6);PrintAt(m,146,"CURRENT QUANTITY: ");
    grSetColor(1);DispNum(0,0,(Data->P[plr].Manned[5+i].Num>=0) ? Data->P[plr].Manned[5+i].Num : 0 );

	 grSetColor(6);PrintAt(m,154,"NUMBER OF ATTEMPTS: ");grSetColor(1);DispNum(0,0,Data->P[plr].Manned[5+i].Steps);
   grSetColor(6);PrintAt(m,162,"SUCCESSFUL ATTEMPTS: ");grSetColor(1);DispNum(0,0,Data->P[plr].Manned[5+i].Steps-Data->P[plr].Manned[5+i].Failures);
	}
  
  music_start(M_HISTORY);
  FadeIn(2,pal,0,0,0);
  WaitForMouseUp();
  while (1)
  {
	key=0;GetMouse();
	 if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER) {
	   InBox(245,5,314,17);
	   WaitForMouseUp();
	   if (key>0) delay(150);
     music_stop();
	   key=0;return; // Continue
	  }
  }
}

void SatBld(char plr)
{
  int plan;
  plan=0;
  SatDraw(plr);
  SatText(plr);
  strcpy(IDT,"i019");strcpy(IKEY,"k019");
  if (Data->P[plr].Probe[1].Num>=0) PlanText(plr,plan);
  music_start(M_HISTORY);
  FadeIn(2,pal,0,0,0);

  key=0;
  WaitForMouseUp();
  while (1)
  {
	GetMouse();
	 if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER) {
	   InBox(245,5,314,17);
	   WaitForMouseUp();
	   if (key>0) delay(150);
	  // OutBox(245,5,314,17);
      music_stop();
	   key=0;return; // Continue
	  }
	else if (Data->P[plr].Probe[1].Num>=0 && ((x>=166 && y>=60 && x<=199 && y<=66 && mousebuttons>0) || key==LT_ARROW)) { 
	   InBox(166,60,199,66);key=0;
	   WaitForMouseUp();
	  delay(10);key=0;
	  if (plan==0) plan=5; else plan--;
	  PlanText(plr,plan);
	   OutBox(166,60,199,66);
	  }
	else if (Data->P[plr].Probe[1].Num>=0 && ((x>=201 && y>=60 && x<=234 && y<=66 && mousebuttons>0) || key==RT_ARROW)) {
	   InBox(201,60,234,66);key=0;
	   WaitForMouseUp();
	  delay(10);key=0;
	  if (plan>4) plan=0; else plan++;
	  PlanText(plr,plan);
	   OutBox(201,60,234,66);
	  }
  }
}


// EOF
