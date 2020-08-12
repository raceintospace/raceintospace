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
// Review Main Files

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>

extern struct MisAst MA[2][4];
extern char AI[2];

void DrawReview(char plr)
{
int clr,i,cte,P_value;
char Fired_Flag=0,Reset_Flag=0;
  
if (Data->P[plr].PresRev[0]!=0x7F) FadeOut(2,pal,10,0,0);
  PortPal(plr);
  gxClearDisplay(0,0);
  if (Data->P[plr].PresRev[0]==0x7F) {Fired_Flag=1;Data->P[plr].PresRev[0]=16;}
   else if (Data->P[plr].PresRev[0]>=16) {Reset_Flag=1;Data->P[plr].PresRev[0]=15;}
  ShBox(0,0,319,22);ShBox(0,24,319,199);
  FlagSm(plr,4,4);
  RectFill(5,28,314,195,0);
  RectFill(5,122,314,195,0);ShBox(6,123,313,194);
  RectFill(7,124,312,193,9); 
  InBox(11,128,307,189);
  RectFill(12,129,306,188,7);
  ShBox(6,29,177,121);ShBox(179,29,313,121);
  InBox(182,32,309,117);InBox(3,3,30,19);
  IOBox(243,3,316,19);
  if (plr==0) DispBig(40,5,"PRESIDENTIAL REVIEW",0,-1);
  else DispBig(40,5,"POLITBURO REVIEW",0,-1);
  grSetColor(1);
  PrintAt(257,13,"CONTINUE");
  grSetColor(1);PrintAt(59,36,"JOB PERFORMANCE");
  grSetColor(6);PrintAt(8,46,"GOOD");
  grSetColor(1);PrintAt(8,77,"FAIR");
  grSetColor(9);PrintAt(8,109,"POOR");
  grSetColor(1);
  DispNum(154,117,Data->Year-1);
  DispNum(126,117,Data->Year-2);
  DispNum(97,117,Data->Year-3);
  DispNum(70,117,Data->Year-4);
  DispNum(42,117,Data->Year-5);
  RectFill(32,39,172,111,0);
  GradRect(33,39,171,74,0);GradRect(33,75,171,110,0);
  grSetColor(3);
  pline(60,40,60,110);
  pline(88,40,88,110);
  pline(116,40,116,110);
  pline(144,40,144,110);
  pline(33,48,171,48);
  pline(33,57,171,57);
  pline(33,66,171,66);
  pline(33,75,171,75);
  pline(33,84,171,84);
  pline(33,93,171,93);
  pline(33,102,171,102);
  InBox(32,39,172,111);
  for (i=0;i<5;i++) if (Data->P[plr].PresRev[i]>16) Data->P[plr].PresRev[i]=16;
    for(i=0;i<5;i++)
     {
      cte=0;
      if (Data->P[plr].PresRev[i]<8)
       {
	     if (Data->P[plr].PresRev[i]==7) cte=73;
	      else cte=40+Data->P[plr].PresRev[i]*5;
       }
       else
      if (Data->P[plr].PresRev[i]>8)
       {
     	  if (Data->P[plr].PresRev[i]==9) cte=77;
	      else cte=80+(Data->P[plr].PresRev[i]-10)*5;
       };
      if (Data->P[plr].PresRev[i]==8) cte=73;
      RectFill(166-i*28,75,151-i*28,cte,5+((Data->P[plr].PresRev[i] <=8) ? 0 : 3));
      grSetColor(6+((Data->P[plr].PresRev[i] <=8) ? 0 : 3));
      pline(167-i*28,75,167-i*28,cte);
     }
  if (Fired_Flag==1)
   {
    clr=0;
    for (i=0;i<Data->P[plr].AstroCount;i++)
     if (Data->P[plr].Pool[i].Status==AST_ST_DEAD) clr++;
    Data->P[plr].PresRev[0]= (clr>=2) ? 17 : 16; 
   }
  DrawRevText(plr,Data->P[plr].PresRev[0]);
  if (Data->P[plr].PresRev[0]==17) Data->P[plr].PresRev[0]=16;
  P_value=0;
  // 0 pres. 1 v.p.
  if (plr==0) 
    {
     if (Data->P[plr].PresRev[0]<=4 || Data->P[plr].PresRev[0]>=11)
       P_value=0; else P_value=1;
    };
  if (plr==1) 
    {
     if (Data->P[plr].PresRev[0]<=4 || Data->P[plr].PresRev[0]>=12)
       P_value=0; else P_value=1;
    }
  if (plr==0) 
    {
     if (Data->Year<=60) {if (P_value==0) PresPict(0); else PresPict(1);}
       else if (Data->Year>=61 && Data->Year<=63) 
	      {if (P_value==0) PresPict(2); else PresPict(3);}
	 else if (Data->Year>=64 && Data->Year<=68)
		{if (P_value==0) PresPict(4); else PresPict(5);}
	     else if (Data->Year>=69 && Data->Year<=73)
		    {if (P_value==0) PresPict(6); else PresPict(7);}
		else if (Data->Year>=74 && Data->Year<=76)
		       {if (P_value==0) PresPict(8); else PresPict(9);}
		   else if (Data->Year>=77) {if (P_value==0) PresPict(10); else PresPict(11);}
    }
  if (plr==1)
   {
    if (Data->Year<61) {if (P_value==0) PresPict(14); else PresPict(15);}
	else if (Data->Year<=64) {if (P_value==0) PresPict(12); else PresPict(13);}
	  else if (Data->Year>=65) {if (P_value==0) PresPict(16); else PresPict(17);}
   }
  if (Reset_Flag==1) Data->P[plr].PresRev[0]=16;
  FadeIn(2,pal,10,0,0);
  
  return;
}


void Review(char plr)
{
DrawReview(plr);
music_start((plr==0)?M_PRES:M_RD);
WaitForMouseUp();
 while (1)
  {
   key=0;GetMouse();
   if (mousebuttons > 0 || key>0)  /* Game Play */
     {
      if ((x>=245 && y>=5 && x<=314 && y<=17) || key==K_ENTER)
	    {
	     InBox(245,5,314,17);
	     WaitForMouseUp();
       music_stop();
	     return;  /* Done */
       };
     }
  };
}

// Mission Review

void MisRev(char plr,int pres)
{
  if (!AI[plr]) music_start((pres>0) ? M_SUCCESS: M_UNSUCC);
  
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);InBox(3,3,30,19);IOBox(243,3,316,19);
  DispBig(40,5,"MISSION REVIEW",0,-1);
  grSetColor(1);PrintAt(258,13,"CONTINUE");
  FlagSm(plr,4,4);
  
  key=0;
  Draw_Mis_Stats(plr,Data->P[plr].PastMis-1,0,1);
  key=0;
  gxClearDisplay(0,0);
  return;
}


void PresPict(char poff)
{
  GXHEADER local;
	SimpleHdr table;
  FILE *in;
  in=sOpen("PRESR.BUT","rb",0);
  fseek(in,poff*sizeof_SimpleHdr,SEEK_SET);
  fread_SimpleHdr(&table,1,in);
  fseek(in,table.offset,SEEK_SET);
  fread(&pal[96],672,1,in);
  fread(buffer,table.size,1,in);
  fclose(in);
  GV(&local,126,84);
  RLED_img(buffer,local.vptr,table.size,local.w,local.h);
  
  gxPutImage(&local,gxSET,183,33,0);
  
  DV(&local);
  return;
}

void CalcPresRev(void)
{
	i16 *ip;
	int val,max,min,i,j;
	char plr;

	if(Data->Year==57 && Data->Season==0)return;
	// Move PresRev down One
	for(j=0;j<NUM_PLAYERS;j++) 
		for(i=4;i>0;i--)
			Data->P[j].PresRev[i] = Data->P[j].PresRev[i-1];

	Data->P[0].Prestige=Data->P[1].Prestige=0; // Clear Prest when finished
	for(plr=0;plr<NUM_PLAYERS;plr++)
	{
		ip=&Data->P[plr].PresRev[0];

		max = maxx(Data->P[plr].tempPrestige[0],Data->P[plr].tempPrestige[1]);
		min = minn(Data->P[plr].tempPrestige[0],Data->P[plr].tempPrestige[1]);

		val = ((max >= 0 && min >= 0) || (max <= 0 && min <= 0 )) ? max + min : max/2+min;

		*ip = (val < 0 && (*ip<4)) ? *ip+1 : ((val>1 && val<=10) ? *ip-1 
				: ((val >= 11 && val <= 20) ? ((*ip<4) ? *ip-1 : *ip-2) 
					: ((val >= 21) ? ((*ip<4) ? *ip-1 : *ip-3)
						: ((val >= -9 && val <= 0) ? *ip+1 : ((val<=-10) ? 
								((plr==0) ? *ip+Data->Def.Lev1+1: *ip+Data->Def.Lev2+1) : *ip)))));

		*ip = (*ip>16) ? 16 : ((*ip<1) ? 1 : *ip);

		Data->P[plr].tempPrestige[0] = 0;
		Data->P[plr].tempPrestige[1] = 0;
		Data->P[plr].PresRev[0] += (Data->P[plr].PresRev[0] > Data->P[abs(plr-1)].PresRev[0]) ? 1 : 0;
	}

}

void DrawRevText(char plr, int val)
{
  int index=0,length,line=0;
  FILE *fin;
  memset(buffer,0x00,10000);
  fin=sOpen("P_REV.DAT","rb",0);      // Read Mission Structure
  fseek(fin, 204*18*plr+204*val,SEEK_SET);
  fread(buffer,204,1,fin);
  fclose(fin);

  grSetColor(1);

  grMoveTo(20,140);
  do{
     if(buffer[index]=='*') {
       length=0;
       index++;
       line++;
       grMoveTo(20,140+12*line);
     }
     DispChr(buffer[index]);
     length++;
     index++;
  } while(buffer[index]!=0);
}


/* EOF */

