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
// Programmed by Michael K McCarty & Morgan Roarty
//
// Museum Main Files

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>

extern struct mStr Mis;

extern GXHEADER but;

void Intel(char plr)
{
  char IName[3][22]={"LIBRARY","CIA STATISTICS","EXIT INTELLIGENCE"};
  char IImg[3]={15,17,0};
  int i,beg;

  // FadeOut(2,pal,10,0,0);
	if (plr==1) {
		music_start(M_INTELLEG);
		strncpy((char *)&IName[1],"KGB",3);
	} else {
		music_start(M_INTEL);
	}
  beg=1;

  do {
     if (beg) beg=0;
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
     strncpy(IDT,(plr==0) ?"i704":"i705",4);
     strncpy(IKEY,(plr==0) ? "k605":"k606",4);

	   i=BChoice(plr,3,&IName[0][0],&IImg[0]);
	   switch(i) {
	      case 1: strcpy(IDT,"i024");strcpy(IKEY,"k024");
             Bre(plr);break;
	      case 2: strcpy(IDT,"i026");strcpy(IKEY,"k026");
             IStat(plr);break;
	      default: break;
	   }
  } while (i==1 || i==2);      

  music_stop();
}


void MisIntel(char plr,char acc)
{
 int i=0,mr,j=0,k=0,save[20],lo=0,hi=28,tot=0,nf=0,seg=0;
 char mis,found;
 static char F[3][14] = {
  {6,1,2,3,4,5}, //58 & 59
  {13,6,25,7,9,10,11,12,8,14,15,18,16}, // 60 to 64
  {11,43,38,48,53,54,55,56,42,49,50}  // 65 and up
 };
 static char W[3][16] = {
  {11,7,3,1,1,2,2},  //58 & 59
  {4,15,1,1,3,3,3,2,2,2,2,1,3,1,1}, // 60 to 64
  {5,12,3,3,3,3,2,2,1,1,1,1} // 65 and up
 };
 for (i=0;i<20;i++) save[i]=0;
 if (acc==0)
   {
	switch(Data->Year)
	 {
	  case 58:case 59: for(i=1;i<F[0][0];i++) save[i]=F[0][i];break;
	  case 60:case 61:case 62:case 63:case 64:for(i=1;i<F[1][0];i++) save[i]=F[1][i];break;
	  default:for(i=1;i<F[2][0];i++) save[i]=F[2][i];break;
	 }
	//i=Data->Year-58;
	if (Data->Year<=59) i=0;
	  else if (Data->Year<=64) i=1;
	    else i=2;
	seg=W[i][0];j=random(100);k=2;nf=0;tot=0;
	while (nf==0 && k<W[i][1]) {
	tot=tot+W[i][k]*seg;
	 if (j<=tot) nf=1;
	   else k++;
	};
	nf=0;j=0;tot=0;k=k-2;
	while(nf==0 && j<20)
	{
	  if (tot==k) nf=1;
	    else if (save[j]>=1) tot++;
	  if (nf==0) j++;
	};
     if (j>0) j=j-1;
   }  // end if
  else
   {
	found=0;
	// mission intelligence
	for (i=0;i<20;i++) save[i]=0;
	for (i=0;i<3;i++)
  if (Data->P[abs(plr-1)].Future[i].MissionCode>0)
	 {
	  mis=Data->P[abs(plr-1)].Future[i].MissionCode;
	  save[found]=mis;++found;
	 }
	for (i=0;i<3;i++)
	   if (Data->P[abs(plr-1)].Mission[i].MissionCode>0)
	 {
	  mis=Data->P[abs(plr-1)].Mission[i].MissionCode;
	  save[found]=mis;++found;
	 }
	//}
	for (i=lo;i<hi;i++) if (save[i]>0) j++; // Check if event is good.
	if (j<=1) {MisIntel(plr,0);return;}
	j=random(hi-lo);k=0;
	while( (k < (hi-lo)) && (save[j]==0) )
	  {   // finds candidate
	   j=random(hi-lo);
	   k++;
	  };
  if (k>=20) {MisIntel(plr,0);return;}
 }
 mis=save[j];
 mr=Data->P[plr].PastIntel[0].cur;nf=0;
 for (i=0;i<mr;i++)
   if (Data->P[plr].PastIntel[i].prog==5 && Data->P[plr].PastIntel[i].index==mis)
	 nf=1;
 if (nf==1 || mis>56+plr || mis<0) {HarIntel(plr,0);return;}
 SaveIntel(plr,5,(unsigned char) mis);
}

void XSpec(char plr,char mis,char year)
{
 GetMisType(mis);
 grSetColor(6);PrintAt(17,75,"CLASS: ");grSetColor(9);
 if (Mis.Days>=1) PrintAt(39,81,"MANNED"); else PrintAt(39,81,"UNMANNED");
 grSetColor(6);PrintAt(17,96,"TYPE: ");grSetColor(9);
 if (Mis.Jt==1) PrintAt(0,0,"JOINT LAUNCH"); else PrintAt(0,0,"SINGLE LAUNCH");
 grSetColor(6);PrintAt(17,112,"DOCKING: ");grSetColor(9);
 if (Mis.Doc==1) PrintAt(0,0,"YES"); else PrintAt(0,0,"NO");
 grSetColor(6);PrintAt(17,128,"DURATION: ");grSetColor(9);
 if (Mis.Dur>=1) PrintAt(0,0,"YES"); else PrintAt(0,0,"NO");
 grSetColor(1);
 PrintAt(33,155,"THE ");if (plr==0) PrintAt(0,0,"CIA ");
 else PrintAt(0,0,"KGB ");PrintAt(0,0,"REPORTS THAT THE ");
 if (plr==0) PrintAt(0,0,"SOVIET UNION IS");else PrintAt(0,0,"UNITED STATES IS");
 PrintAt(33,169,"PLANNING A ");
 grSetColor(9);MissionName(mis,93,169,30);grSetColor(1);
 PrintAt(33,183,"SOMETIME IN THE NEXT YEAR.");
 TopSecret(plr,37+Data->P[plr].PastIntel[year].sf);
}

void Special(char p,int ind)
{
 
 grSetColor(6);
 if (ind>=5) PrintAt(17,96,"GROUP: ");
   else PrintAt(17,96,"FACILITY: ");grSetColor(9);
 switch(ind)
  {
   case 3:PrintAt(0,0,"TWO");break;
   case 4:PrintAt(0,0,"THREE");break;
   case 5:PrintAt(0,0,"ONE");break;
   case 6:PrintAt(0,0,"TWO");break;
   default:break;
  }

 if (ind>=5) {grSetColor(6);PrintAt(17,112,"STATUS: ");grSetColor(9);PrintAt(0,0,"TRAINING");}
   else {grSetColor(6);PrintAt(17,112,"STATUS: ");grSetColor(9);PrintAt(0,0,"OPERATIONAL");}

 grSetColor(1);
 PrintAt(33,155,"THE ");if (p==0) PrintAt(0,0,"CIA ");
 else PrintAt(0,0,"KGB ");PrintAt(0,0,"REPORTS THAT THE ");
 if (p==0) PrintAt(0,0,"SOVIET UNION HAS ");else PrintAt(0,0,"UNITED STATES HAS ");
 PrintAt(33,169,"PURCHASED ");PrintAt(0,0,"A NEW");
 grSetColor(9);
 if (ind>=5)
   {
	PrintAt(0,0," GROUP OF ");
	 if (p==0) PrintAt(0,0,"COSMO");
	   else PrintAt(0,0,"ASTRO");PrintAt(0,0,"NAUTS ");
   }

   else PrintAt(0,0," LAUNCH FACILITY ");
 grSetColor(1);
 PrintAt(33,183,"FOR ITS SPACE PROGRAM");
 TopSecret(p,ind);
}


void BackIntel(char p,char year)
{
 int prg,ind,dur=0,xc,yc;
 FILE *fin;
 char code,w;
 
 grSetColor(6);
 prg=Data->P[p].PastIntel[year].prog;
 ind=Data->P[p].PastIntel[year].index;
 grSetColor(6);PrintAt(17,37,"CODE: ");grSetColor(9);
 DispNum(0,0,Data->P[p].PastIntel[year].num);DispChr(Data->P[p].PastIntel[year].code);
 PrintAt(0,0,"-");if (Data->Season==0) PrintAt(0,0,"S");else PrintAt(0,0,"F");
 DispNum(0,0,58+year);
 grSetColor(6);PrintAt(17,51,"CODE NAME: ");grSetColor(1);xc=39;yc=59;
 // CODE NAME GOES HERE
 code=-1;
 if (prg==0) code=ind;
   else if (prg==1) code=ind+7;
	 else if (prg==2) code=ind+12;
	   else if (prg==5) code=Data->P[p].PastIntel[year].sf-1;
 if (code==-1) PrintAt(xc,yc,"TOP SECRET");
   else {
 w=Data->P[p].PastIntel[year].cdex;
 fin=sOpen("CODENAME.DAT","rb",0);     // Get Mission DataFile
 fseek(fin,(code*6*20)+(w*20),SEEK_SET); // Find Mission Type
 fread(&Name,sizeof Name,1,fin);           // Get new Mission Info
 fclose(fin);
 PrintAt(xc,yc,&Name[0]); }
 if (prg==5) {XSpec(p,ind,year);return;}
 grSetColor(6);PrintAt(17,75,"CLASS: ");grSetColor(9);
 switch(prg)
  {
   case 0:if (ind>=5 && ind<=6) {if (p==0) PrintAt(39,82,"COSMO");
		  else PrintAt(39,81,"ASTRO");PrintAt(0,0,"NAUTS");}
		else if (ind>=3 && ind<=4) PrintAt(39,81,"LAUNCH FACILITY");
		  else PrintAt(39,81,"PROBE");break;
   case 1:PrintAt(39,82,"ROCKET");break;
   case 2:if (ind<5) PrintAt(39,81,"CAPSULE");
		else PrintAt(39,81,"LUNAR MODULE");
	  break;
   case 3:if (ind<3) PrintAt(39,81,"KICKER");
		else if (ind==3) PrintAt(39,81,"EVA SUITS");
		 else if (ind==4) PrintAt(39,81,"DOCKING MODULES");
	  break;
   default:break;
  }
 if (prg==0 && ind>=3) {Special(p,ind);return;}
 
 grSetColor(6);PrintAt(17,96,"CREW: ");grSetColor(9);
 if (prg==2)
   {
	if (ind>=0 && ind<=2) DispNum(0,0,ind+1);
	else if (ind==3 || ind==4) DispNum(0,0,ind);
	else DispNum(0,0,ind-4);
	PrintAt(0,0," PERSON");
   } else PrintAt(0,0,"NONE");
 grSetColor(6);PrintAt(17,112,"PROGRAM: ");grSetColor(9);
 switch(prg)
  {
   case 0:PrintAt(0,0,&Data->P[abs(p-1)].Probe[ind].Name[0]);break;
   case 1:PrintAt(0,0,&Data->P[abs(p-1)].Rocket[ind].Name[0]);break;
   case 2:PrintAt(0,0,&Data->P[abs(p-1)].Manned[ind].Name[0]);break;
   case 3:PrintAt(0,0,&Data->P[abs(p-1)].Misc[ind].Name[0]);break;
   default:break;
  }
 grSetColor(6);PrintAt(17,128,"DURATION: ");grSetColor(9);
 switch(prg)
  {
   case 0:dur=Data->P[abs(p-1)].Probe[ind].Duration;break;
   case 1:dur=Data->P[abs(p-1)].Rocket[ind].Duration;break;
   case 2:dur=Data->P[abs(p-1)].Manned[ind].Duration;break;
   case 3:dur=Data->P[abs(p-1)].Misc[ind].Duration;break;
   default:break;
  }
 if (dur==0) PrintAt(0,0,"NONE");
   else {DispNum(0,0,dur);PrintAt(0,0," DAYS");}
 grSetColor(1);
 PrintAt(33,155,"THE ");if (p==0) PrintAt(0,0,"CIA ");
 else PrintAt(0,0,"KGB ");PrintAt(0,0,"REPORTS THAT THE ");
 if (p==0) PrintAt(0,0,"SOVIET UNION IS");else PrintAt(0,0,"UNITED STATES IS");
 PrintAt(33,169,"DEVELOPING THE ");
 grSetColor(9);
 switch(prg)
  {
   case 0:PrintAt(0,0,&Data->P[abs(p-1)].Probe[ind].Name[0]);break;
   case 1:PrintAt(0,0,&Data->P[abs(p-1)].Rocket[ind].Name[0]);break;
   case 2:PrintAt(0,0,&Data->P[abs(p-1)].Manned[ind].Name[0]);break;
   case 3:PrintAt(0,0,&Data->P[abs(p-1)].Misc[ind].Name[0]);break;
   default:break;
  }
 grSetColor(1);
 PrintAt(0,0," AND RATE THE");
 PrintAt(33,183,"RELIABILITY AT ABOUT ");
 DispNum(0,0,Data->P[p].PastIntel[year].sf);PrintAt(0,0," PERCENT.");
 if (prg!=5) TopSecret(p,prg*7+ind);
}

void HarIntel(char p,char acc)
{
 int mr,i,prg=0,ind=0,j=0,k=0,save[28],lo=0,hi=28,tot=0,nf=0,seg=0;

 static char F[10][11] = {
  {7,0,7,8,11,14,15,12,12,12}, //58
  {4,3,5,17,12,12,12,12,12,12}, //59
  {3,16,24,12,12,12,12,12,12,12}, //60
  {9,1,4,6,8,11,14,15,17,24}, //61
  {10,1,2,4,6,8,11,15,16,17,18}, //62
  {5,19,20,9,12,12,12,12,12,12}, //63
  {5,9,10,18,12,12,12,12,12,12}, //64
  {5,9,10,18,12,12,12,12,12,12}, //65
  {5,9,10,18,12,12,12,12,12,12} //66
 };
 static char W[10][15] = {
 {9,8,3,2,1,1,3,1}, // 58
 {7,11,3,1,1,2,1,1,3,1,1}, // 59
 {5,13,1,1,2,2,2,2,3,2,1,2,2}, // 60
 {5,12,2,2,2,2,2,1,3,2,2,2}, // 61
 {5,10,3,3,2,3,2,3,2,2}, // 62
 {3,14,3,3,2,3,2,2,2,2,2,2,2,3}, // 63
 {3,15,3,3,2,3,3,1,2,2,2,2,1,2,3}, // 64
 {3,15,3,3,2,3,3,1,2,2,2,2,1,2,3}, // 65
 {3,15,3,3,2,3,3,1,2,2,2,2,1,2,3} // 66
 };
 for (i=0;i<28;i++) save[i]=0;
 if (acc==0)
   {
	switch(Data->Year)
	 {
	  case 58:for(i=1;i<F[0][0];i++) save[F[0][i]]=1;break;
	  case 59:for(i=1;i<F[0][0];i++) save[F[0][i]]=1;
		  for(i=1;i<F[1][0];i++) save[F[1][i]]=1;break;
	  case 60:for(i=1;i<F[0][0];i++) save[F[0][i]]=1;
		  for(i=1;i<F[1][0];i++) save[F[1][i]]=1;
		  for(i=1;i<F[2][0];i++) save[F[2][i]]=1;break;
	  case 61:for(i=1;i<F[2][0];i++) save[F[2][i]]=1;
		  for(i=1;i<F[3][0];i++) save[F[3][i]]=1;break;
	  case 62:for(i=1;i<F[4][0];i++) save[F[4][i]]=1;break;
	  case 63:for(i=1;i<F[4][0];i++) save[F[4][i]]=1;
		  for(i=1;i<F[5][0];i++) save[F[5][i]]=1;break;
	  case 64:for(i=1;i<F[4][0];i++) save[F[4][i]]=1;
		  for(i=1;i<F[5][0];i++) save[F[5][i]]=1;
		  for(i=1;i<F[6][0];i++) save[F[6][i]]=1;break;
	  case 65:for(i=1;i<F[4][0];i++) save[F[4][i]]=1;
		  for(i=1;i<F[5][0];i++) save[F[5][i]]=1;
		  for(i=1;i<F[7][0];i++) save[F[7][i]]=1;break;
	  default:for(i=1;i<F[4][0];i++) save[F[4][i]]=1;
		  for(i=1;i<F[5][0];i++) save[F[5][i]]=1;
		  for(i=1;i<F[8][0];i++) save[F[8][i]]=1;break;
	 }
	i=Data->Year-58;
	if (Data->Year>=66) i=8;
	seg=W[i][0];j=random(100);k=2;nf=0;tot=0;
	while (nf==0 && k<W[i][1]) {
	tot=tot+W[i][k]*seg;
	 if (j<=tot) nf=1;
	   else k++;
	};
	nf=0;j=0;tot=0;k=k-2;
	while(nf==0 && j<28)
	{
	  if (tot==k) nf=1;
	  else if (save[j]==1) tot++;
	  if (nf==0) j++;
	};
	if (j>0) j=j-1; // adjust

   }
  else
   { // accurate programs pick one
	for (i=0;i<7;i++)
	{
	  if (Data->P[abs(p-1)].Probe[i].Num>=0) save[i]=1;
	  if (Data->P[abs(p-1)].Rocket[i].Num>=0) save[i+7]=1;
	  if (Data->P[abs(p-1)].Manned[i].Num>=0) save[i+14]=1;
	  if (Data->P[abs(p-1)].Misc[i].Num>=0) save[i+21]=1;
	};
  save[3]=save[4]=save[5]=save[6]=save[12]=save[13]=save[26]=save[27]=0;
  if (Data->P[abs(p-1)].LaunchFacility[1]==1) save[3]=1;
  if (Data->P[abs(p-1)].LaunchFacility[2]==1) save[4]=1;
  if (Data->P[abs(p-1)].AstroLevel==0) save[5]=1;
  if (Data->P[abs(p-1)].AstroLevel==1) save[6]=1;
  for (i=lo;i<hi;i++) if (save[i]>0) j++; // Check if event is good.
  if (j<=2) {HarIntel(p,0);return;}
  j=random(hi-lo);k=0;

  while( (k < (hi-lo)) && (save[j]!=1) ) {   // finds candidate
 // DispNum(100,5+k*6,j);
  j=random(hi-lo);
  k++;
  };

  if (k>=28) {HarIntel(p,0);return;}
  } // end else

 if (j>=0 && j<7) {prg=0;ind=j;}
 else if (j>=7 && j<14) {prg=1;ind=j-7;}
 else if (j>=14 && j<21) {prg=2;ind=j-14;}
 else if (j>=21 && j<28) {prg=3;ind=j-21;}

  mr=Data->P[p].PastIntel[0].cur;nf=0;
 for (i=0;i<mr;i++)
   if (Data->P[p].PastIntel[i].prog==prg && Data->P[p].PastIntel[i].index==ind)
	 nf=1;
 if (nf==1 || (prg==1 && ind==5) || (prg==1 && ind==6) || 
   (prg==3 && ind==5) || (prg==3 && ind==6)) {MisIntel(p,0);return;}
 SaveIntel(p,prg,ind);
}


void TopSecret(char plr,char poff)
{
  GXHEADER local,local2;
  unsigned int j;
	SimpleHdr table;
  FILE *in;
  if (poff<56)
    if (plr==1) poff=poff+28;
  in=sOpen("INTEL.BUT","rb",0);
  fread_SimpleHdr(&table,1,in);
  fseek(in,71*sizeof_SimpleHdr,SEEK_SET);
  fread(&pal[0],768,1,in);
  fseek(in,table.offset,SEEK_SET);
  fread(buffer,table.size,1,in);
  GV(&local,157,100); GV(&local2,157,100);
  RLED_img(buffer,local.vptr,table.size,local.w,local.h);
  if (poff!=100)
    {
     fseek(in,(poff+1)*sizeof_SimpleHdr,SEEK_SET);
     fread_SimpleHdr(&table,1,in);
     fseek(in,table.offset,SEEK_SET);
     fread(buffer,table.size,1,in);     
     RLED_img(buffer,local2.vptr,table.size,local.w,local.h);
     for (j=0;j<15700;j++)
	if(local2.vptr[j]!=0) local.vptr[j]=local2.vptr[j];
    }
  
  gxPutImage(&local,gxSET,153,32,0);
  
  fclose(in);
  DV(&local); DV(&local2);
}

void SaveIntel(char p,char prg,char ind)
{
 char Op[61] = {
 0,19,20,20,21,21,21,22,28,24,25,
 23,26,27,21,21,21,21,21,21,21,
 21,21,21,21,21,21,21,21,21,21,
 21,21,21,21,21,21,21,21,21,21,
 21,28,28,28,30,30,30,30,30,30,
 30,30,31,31,31,31,31};
 int mr,sf,j,k;
 char ky;
 if (prg==5 && ind==0) ind=1;
 mr=random(9998)+1;ky=65+random(26);
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].code=ky;
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].num=mr;
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].prog=prg;
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].index=ind;
 j=random(100);if (j<33) k=0;else if(j<66) k=1; else k=2;if (p==1) k=k+3;
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].cdex=k;
 if (prg==5) sf=Op[ind];
   else sf=random(22)+77;
 Data->P[p].PastIntel[Data->P[p].PastIntel[0].cur].sf=sf;
 if (prg!=5) Data->P[p].Table[prg][ind]=sf;
}

#ifdef DEAD_CODE
void Bad(char plr,char pt)
{
 
 if (pt==1){
	ShBox(6,29,130,133);
    InBox(8,31,128,131);
 grSetColor(9);
 PrintAt(17,37,"AGENTS IN THE FIELD");
 PrintAt(17,51,"REPORTED HEAVY");
 PrintAt(17,65,"ACTIVITY AT ");
 if (plr==1) PrintAt(0,0,"THE CAPE"); else PrintAt(0,0,"BAIKONUR");}
 if (pt==0) Data->P[plr].PastIntel[Data->Year-58].prog=-1;
 
}
#endif

void ImpHard(char plr,char hd,char dx)
{ // based on the hardware improve safety factor
 switch(hd)
  {
   case 0:Data->P[plr].Table[0][dx]=Data->P[plr].Probe[dx].MaxRD- random(Data->P[plr].Probe[dx].MaxSafety-Data->P[plr].Probe[dx].MaxRD);break;
   case 1:Data->P[plr].Table[1][dx]=Data->P[plr].Rocket[dx].MaxRD- random(Data->P[plr].Rocket[dx].MaxSafety-Data->P[plr].Rocket[dx].MaxRD);break;
   case 2:Data->P[plr].Table[2][dx]=Data->P[plr].Manned[dx].MaxRD- random(Data->P[plr].Manned[dx].MaxSafety-Data->P[plr].Manned[dx].MaxRD);break;
   case 3:Data->P[plr].Table[3][dx]=Data->P[plr].Misc[dx].MaxRD- random(Data->P[plr].Misc[dx].MaxSafety-Data->P[plr].Misc[dx].MaxRD);break;
   default:break;
  }
}

void UpDateTable(char plr)
{  // based on prestige
 char i,j,p;
 p=other(plr);
 if (Data->P[p].LMpts>0)
  {
   j=random(100);if (j<60) ImpHard(plr,2,5); else ImpHard(plr,2,6);
  }
 for (i=0;i<28;i++)
  {
   if (Data->Prestige[i].Place==p || Data->Prestige[i].mPlace==p)
	 {
	  switch(i) {
	  case 0:ImpHard(plr,0,0);break;
	  case 1:case 2:case 3:case 4:case 5:case 6:ImpHard(plr,0,1);ImpHard(plr,1,1);break;
	  case 7:ImpHard(plr,0,2);ImpHard(plr,1,1);break;
	  case 13: ImpHard(plr,2,0);ImpHard(plr,1,1);break;
	  case 14: ImpHard(plr,2,1);ImpHard(plr,1,2);break;
	  case 15: ImpHard(plr,2,2);ImpHard(plr,1,2);break;
	  case 16: ImpHard(plr,2,3);ImpHard(plr,1,2);break;
	  case 17: ImpHard(plr,2,4);ImpHard(plr,1,3);break;
	  case 18: ImpHard(plr,1,4);break;
	  case 19: ImpHard(plr,1,4);j=random(100); if (j<70) ImpHard(plr,3,1); else ImpHard(plr,3,0);break;
	  case 20: ImpHard(plr,1,4);j=random(100); if (j<70) ImpHard(plr,3,1); else ImpHard(plr,3,0);break;
	  case 24: ImpHard(plr,3,4);break;
     case 26: case 27:ImpHard(plr,3,3);break;
	  default:break;
	 } // switch
	}  // if
   } // for 
}

void IntelPhase(char plr,char pt)
{
 int i,splt,acc,Plr_Level,Acc_Coef;
 if (Data->Year==57 || (Data->Year==58 && Data->Season==0)) return;
 if (pt)EMPTY_BODY;
 splt=random(1000);i=random(1000);
 if (plr==0) Plr_Level=Data->Def.Lev1;
  else Plr_Level=Data->Def.Lev2;
 // stagger accuracy for player levels
 if (Plr_Level==0) Acc_Coef=600;
  else if (Plr_Level==1) Acc_Coef=500;
   else Acc_Coef=400;
 if (i<Acc_Coef) acc=1; else acc=0; // 40% accurate
 if (splt<500) HarIntel(plr,acc);
   else MisIntel(plr,acc);
 Data->P[plr].PastIntel[0].cur++;
 UpDateTable(plr);
}


void DrawBre(char plr)
{
  
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);InBox(3,3,30,19);
  IOBox(242,3,315,19);
  ShBox(0,24,319,199);RectFill(5,28,314,195,0);
  InBox(4,27,315,196);
  ShBox(150,29,313,133);InBox(152,31,310,131);RectFill(153,32,310,131,0);
  ShBox(132,29,148,80);
  ShBox(132,82,148,133);
  ShBox(6,29,130,41);InBox(8,31,128,39); // code
  ShBox(6,43,130,63);InBox(8,45,128,61); // code name
  ShBox(6,65,130,85);InBox(8,67,128,83); // class
  ShBox(6,87,130,101);InBox(8,89,128,99); // crew
  ShBox(6,103,130,118);InBox(8,105,128,116); // program
  ShBox(6,120,130,133);InBox(8,122,128,131); // duration
  IOBox(133,30,147,79);IOBox(133,83,147,132);
  ShBox(6,135,313,194);RectFill(7,136,312,193,9);InBox(11,139,308,190);RectFill(12,140,307,189,7);
  DispBig(33,5,"INTELLIGENCE BRIEFING",1,-1);
  FlagSm(plr,4,4);
  grSetColor(1);PrintAt(256,13,"CONTINUE");
  UPArrow(137,42);DNArrow(137,95);
  
}

void Bre(char plr)
{
 int year=Data->P[plr].PastIntel[0].cur-1;
 DrawBre(plr);
 TopSecret(plr,100); // just the blue background
 BackIntel(plr,year);
 FadeIn(2,pal,10,0,0);
 WaitForMouseUp();
 while (1)
  {
    key=0;GetMouse();
    if (mousebuttons > 0 || key>0)  /* Gameplay */
     {
      if ((x>=135 && y>32 && x<=145 && y<=77 && mousebuttons>0) || key==UP_ARROW)
       {
        InBox(135,32,145,77);
        WaitForMouseUp();
        if (year>=0 && year+1<=Data->P[plr].PastIntel[0].cur-1)
	      {
	       
	       RectFill(45,32,98,38,3);RectFill(38,54,127,60,3); 
	       RectFill(38,76,127,82,3);RectFill(16,91,127,98,3);
	       RectFill(16,107,127,113,3);RectFill(16,123,127,129,3);
	       RectFill(30,148,300,186,7); 
	       year++;BackIntel(plr,year);
	      }
	    OutBox(135,32,145,77);
      }
     else
     if ((x>=135 && y>85 && x<=145 && y<=130 && mousebuttons>0) || key==DN_ARROW)
       {
        InBox(135,85,145,130);
        WaitForMouseUp();
        if (year-1>=0) 
	       {
	        
	        RectFill(45,32,98,38,3);RectFill(38,54,127,60,3); 
	        RectFill(38,76,127,82,3);RectFill(9,90,127,98,3);
	        RectFill(9,106,127,115,3);RectFill(9,123,127,130,3);
	        RectFill(30,148,300,186,7);  
	        year--;BackIntel(plr,year);
	       }
	     OutBox(135,85,145,130);
        }
      else
      if ((x>=244 && y>=5 && x<=313 && y<=17 && mousebuttons>0) || key==K_ENTER)
      {
	    InBox(244,5,313,17);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    OutBox(244,5,313,17);
	    break;  /* Done */
	   };
    }
  };
}

void Load_CIA_BUT(void)
{
  int i;
  FILE *fin;

  fin=sOpen("CIA.BUT","rb",0);
  fread(&pal[0],768,1,fin);
  i=fread(screen,1,MAX_X*MAX_Y,fin);
  PCX_D(screen,(char *)vhptr.vptr,i);
  fclose(fin);
}


void DrawIStat(char plr)
{
 int i;
	
	FadeOut(2,pal,10,0,0);

	Load_CIA_BUT();
	gxClearDisplay(0,0);
	Load_RD_BUT(plr);

	ShBox(0,0,319,199);IOBox(242,3,315,19);
	InBox(3,22,316,167);InBox(3,3,30,19);

	ShBox(0,160,319,199);IOBox(5,162,77,197);
	IOBox(81,162,158,197);IOBox(162,162,239,197);
	IOBox(243,162,315,197);
	GradRect(4,23,315,159,0);
	for (i=4;i<316;i+=2) {
	  grPutPixel(i,57,11);
	  grPutPixel(i,91,11);
	  grPutPixel(i,125,11);
	}
	grSetColor(9);
	DispNum(5,55,75);PrintAt(17,55,"%");
	DispNum(5,89,50);PrintAt(17,89,"%");
	DispNum(5,123,25);PrintAt(17,123,"%");                  
	gxVirtualDisplay(&but,0,0,8,165,74,194,0);  // Unmanned
	gxVirtualDisplay(&but,68,0,84,165,155,194,0); // Rocket
	gxVirtualDisplay(&but,141,0,165,165,236,194,0); // Manned
	gxVirtualDisplay(&but,214,0,246,165,312,194,0); // Misc
	grSetColor(6);
	DispBig(40,5,"INTELLIGENCE STATS",1,-1);
	FlagSm(plr,4,4);
	grSetColor(1);PrintAt(256,13,"CONTINUE");
	FadeIn(2,pal,10,0,0);
	
}

void ReButs(char old,char nw)
{
  
  switch(old) {
    case 0:OutBox(7,164,75,195);
	   gxVirtualDisplay(&but,0,0,8,165,74,194,0);  // Unmanned
	   break;
    case 1:OutBox(83,164,156,195);
	   gxVirtualDisplay(&but,68,0,84,165,155,194,0);  // Rocket
	   break;
    case 2:OutBox(164,164,237,195);
	   gxVirtualDisplay(&but,141,0,165,165,236,194,0); // Manned
	   break;

    case 3:OutBox(245,164,313,195);
	   gxVirtualDisplay(&but,214,0,246,165,312,194,0); // Misc
	   break;
    default:break;
  }
  switch(nw) {
    case 0:InBox(7,164,75,195);
	   gxVirtualDisplay(&but,0,31,8,165,74,194,0);  // Unmanned
	   break;
    case 1:InBox(83,164,156,195);
	   gxVirtualDisplay(&but,68,31,84,165,155,194,0);  // Rocket
	   break;
    case 2:InBox(164,164,237,195);
	   gxVirtualDisplay(&but,141,31,165,165,236,194,0); // Manned
	   break;

    case 3:InBox(245,164,313,195);
	   gxVirtualDisplay(&but,214,31,246,165,312,194,0); // Misc
	   break;
  }
  
}

void IStat(char plr)
{
int place=-1;

  DrawIStat(plr);
  WaitForMouseUp();
  while (1)
  {
	key=0;GetMouse();
    if (mousebuttons > 0 || key>0)  /* Gameplay */
    {
     if (((x>=7 && y>=164 && x<=75 && y<=195 && mousebuttons>0) || key=='U') && place!=0)
      {
	    InBox(7,164,75,195);
	    WaitForMouseUp();
	    OutBox(7,164,75,195);
	    ReButs(place,0);
	    place=0;
	    IInfo(plr,place,0);
	    /* Unmanned */
      };
      if (((x>=83 && y>=164 && x<=156 && y<=195 && mousebuttons>0) || key=='R') && place!=1)
       {
	     InBox(83,164,156,195);
	     WaitForMouseUp();
	     OutBox(83,164,156,195);
	     ReButs(place,1);
	     place=1;
	     IInfo(plr,place,0);
	     /* Rocket */
       };
      if (((x>=164 && y>=164 && x<=237 && y<=195 && mousebuttons>0) || key=='C') && place!=2)
       {
	     InBox(164,164,237,195);
	     WaitForMouseUp();
	     OutBox(164,164,237,195);
	     /* MANNED */
	     ReButs(place,2);
	     place=2;
	     IInfo(plr,place,0);
       };
      if (((x>=245 && y>=164 && x<=313 && y<=195 && mousebuttons>0) || key=='M') && place!=3)
      {
	    InBox(245,164,313,195);
	    WaitForMouseUp();
	    OutBox(245,164,313,195);
	    ReButs(place,3);
	    place=3;
	    IInfo(plr,place,0);
	    /* MISC */
      };
	  if ((x>=244 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER)
      {
	    InBox(244,5,314,17);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    OutBox(244,5,314,17);
	    Del_RD_BUT();
	    break;  /* Done */
      };
    }
  };
}
void DispIt(int x1,int y1,int x2, int y2,int s,int t)
{
  GXHEADER local,local2;
  int i,w,h;
  unsigned char *src,*dest;

    w=x2-x1+1;h=y2-y1+1;
    GV(&local,w,h); GV(&local2,w,h);
    gxClearVirtual(&local,0);gxClearVirtual(&local2,0);
    gxGetImage(&local2,s,t,s+w-1,t+h-1,0);
    gxVirtualVirtual(&vhptr,x1,y1,x2,y2,&local,0,0,gxSET);
    src=local.vptr;dest=local2.vptr;
	for (i=0;i<w*h;i++) {
		if (*src)
			*dest = *src;
		dest++; src++;
	}
	
    gxPutImage(&local2,gxSET,s,t,0);
    
    DV(&local);DV(&local2);
}

void IInfo(char plr,char loc,char w)
{
  int i,sfu,sfs;
  
  if (w==0) {
    GradRect(4,23,315,159,0);
    for (i=4;i<316;i+=2) {
      grPutPixel(i,57,11);
      grPutPixel(i,91,11);
      grPutPixel(i,125,11);
    }
    grSetColor(9);
    DispNum(5,55,75);PrintAt(17,55,"%");
    DispNum(5,89,50);PrintAt(17,89,"%");
    DispNum(5,123,25);PrintAt(17,123,"%");                  
    //gxDisplayVirtual(4,23,315,159,0,&vhptr,4,23);
  } //else gxVirtualDisplay(&vhptr,4,23,4,23,315,159,0);

 grSetColor(1);
  switch(loc) {
    case 1: //PrintAt(137,150,"ROCKETS");
	   for (i=0;i<4;i++)
	    {
	     sfu=-1;sfs=-1;
	     if (plr==0) {if (Data->P[0].Rocket[i].Num>=0) sfu=Data->P[0].Rocket[i].Safety;sfs=Data->P[0].Table[1][i];}
	       else if (plr==1) {if (Data->P[1].Rocket[i].Num>=0) sfs=Data->P[1].Rocket[i].Safety;sfu=Data->P[1].Table[1][i];}
	     switch(i)
	      {
		   case 0:if (sfu>0)
			{RectFill(19,159-sfu*136/100,27,159,6);
			 RectFill(19,159-sfu*136/100,26,158,5);
			 DispIt(101,1,115,57,11,104);};
		      if (sfs>0)
			{RectFill(50,159-sfs*136/100,58,159,9);
			 RectFill(50,159-sfs*136/100,57,158,8);
			 DispIt(125,1,149,85,33,75);};
			  break;
	       case 1:if (sfu>0)
			{
			 RectFill(78,159-sfu*136/100,86,159,6);
			 RectFill(78,159-sfu*136/100,85,158,5);
			 DispIt(115,0,124,68,73,92);};
		      if (sfs>0)
			{
			 RectFill(103,159-sfs*136/100,111,159,9);
			 RectFill(103,159-sfs*136/100,110,158,8);
			 DispIt(151,1,170,95,88,65);};
		      break;
	       case 2:if (sfu>0)
			{
			 RectFill(159,159-sfu*136/100,167,159,6);
			 RectFill(159,159-sfu*136/100,166,158,5);
			 DispIt(172,1,209,133,130,27);};
		      if (sfs>0)
			{
			 RectFill(200,159-sfs*136/100,208,159,9);
			 RectFill(200,159-sfs*136/100,207,158,8);
			 DispIt(211,1,243,133,172,27);};
			 break;
	       case 3:if (sfu>0)
			{
			 RectFill(260,159-sfu*136/100,268,159,6);
			 RectFill(260,159-sfu*136/100,267,158,5);
			 DispIt(245,1,285,137,231,23);};
		       if (sfs>0)
			 {
			  RectFill(302,159-sfs*136/100,310,159,9);
			  RectFill(302,159-sfs*136/100,309,158,8);
			  DispIt(287,1,318,132,274,28);};
		       break;
	       default:break;
		  }
	    };
	    break;
    case 2: //PrintAt(137,150,"CAPSULES");
	   for (i=0;i<5;i++)
	    {
	     sfu=-1;sfs=-1;
	     if (plr==0) {if (Data->P[0].Manned[i].Num>=0) sfu=Data->P[0].Manned[i].Safety;sfs=Data->P[0].Table[2][i];}
	       else if (plr==1) {if (Data->P[1].Manned[i].Num>=0) sfs=Data->P[1].Manned[i].Safety;sfu=Data->P[1].Table[2][i];}
	     switch(i)
	      {
	       case 0:if (sfu>0) {
			 RectFill(13,159-sfu*136/100,21,159,6);
			 RectFill(13,159-sfu*136/100,20,158,5);
			 DispIt(12,91,25,116,11,137);};
		      if (sfs>0) {
			RectFill(41,159-sfs*136/100,49,159,9);
			RectFill(41,159-sfs*136/100,48,158,8);
			DispIt(0,56,26,89,27,123);};
		       break;
	       case 1:
	    if (sfu>0) {
	      RectFill(70,159-sfu*136/100,78,159,6);
	      RectFill(70,159-sfu*136/100,77,158,5);
	      DispIt(27,98,49,127,59,127);};
	    if (sfs>0) {
	      RectFill(97,159-sfs*136/100,105,159,9);
		  RectFill(97,159-sfs*136/100,104,158,8);
	      DispIt(28,62,49,96,84,122);};
		      break;
	       case 2:
	    if (sfu>0) {
	      RectFill(132,159-sfu*136/100,140,159,6);
	      RectFill(132,159-sfu*136/100,139,158,5);
	      DispIt(95,77,117,127,117,106);};
		if (sfs>0) {
	      RectFill(174,159-sfs*136/100,182,159,9);
	      RectFill(174,159-sfs*136/100,181,158,8);
	      DispIt(119,97,170,140,144,113);};
		      break;
	       case 3:
	    if (sfu>0) {
	      RectFill(210,159-sfu*136/100,218,159,6);
	      RectFill(210,159-sfu*136/100,217,158,5);
	      DispIt(3,1,16,54,203,103);};
	    if (sfs>0) {
	      RectFill(232,159-sfs*136/100,240,159,9);
	      RectFill(232,159-sfs*136/100,239,158,8);
	      DispIt(18,1,32,48,223,109);};
		      break;
	       case 4:
	    if (sfu>0) {
	      RectFill(269,159-sfu*136/100,277,159,6);
	      RectFill(269,159-sfu*136/100,276,158,5);
	      DispIt(34,1,65,60,248,97);};
	    if (sfs>0) {
	      RectFill(305,159-sfs*136/100,313,159,9);
	      RectFill(305,159-sfs*136/100,312,158,8);
	      DispIt(67,1,100,60,281,97);};
		      break;
		default:break;
	       }
		  }
	    break;
    case 0: //PrintAt(100,150,"SATELLITES & LM'S");
	    sfu=-1;sfs=-1;
	    if (plr==0) {if (Data->P[0].Manned[6].Num>=0) sfu=Data->P[0].Manned[6].Safety;sfs=Data->P[0].Table[2][6];}
	      else if (plr==1) {if (Data->P[1].Manned[6].Num>=0) sfs=Data->P[1].Manned[6].Safety;sfu=Data->P[1].Table[2][6]; }
	    if (sfu>0) {
	      RectFill(25,159-sfu*136/100,33,159,6);
	      RectFill(25,159-sfu*136/100,32,158,5);
	      DispIt(60,153,88,176,9,132);};
	    if (sfs>0) {
	      RectFill(61,159-sfs*136/100,69,159,9);
	      RectFill(61,159-sfs*136/100,68,158,8);
	      DispIt(31,153,56,182,41,126);}; 
	    sfu=-1;sfs=-1;
	    if (plr==0) {if (Data->P[0].Manned[5].Num>=0) sfu=Data->P[0].Manned[5].Safety;sfs=Data->P[0].Table[2][5];}
	      else if (plr==1) {if (Data->P[1].Manned[5].Num>=0) sfs=Data->P[1].Manned[5].Safety;sfu=Data->P[1].Table[2][5];}
	    if (sfu>0) {
	      RectFill(101,159-sfu*136/100,109,159,6);
	      RectFill(101,159-sfu*136/100,108,158,5);
	      DispIt(1,153,29,182,83,128);
	      };
	    if (sfs>0) {
	      RectFill(132,159-sfs*136/100,140,159,9);
	      RectFill(132,159-sfs*136/100,139,158,8);
	      DispIt(90,151,119,176,112,131);   
	     };
	    for (i=0;i<3;i++)
	       {
		sfu=-1;sfs=-1;
		if (plr==0) {if (Data->P[0].Probe[i].Num>=0) sfu=Data->P[0].Probe[i].Safety;sfs=Data->P[0].Table[0][i];}
		  else if (plr==1) {if (Data->P[1].Probe[i].Num>=0) sfs=Data->P[1].Probe[i].Safety;sfu=Data->P[1].Table[0][i];}
		switch(i)
		  {
		   case 0:
	    if (sfu>0) {
	      RectFill(152,159-sfu*136/100,160,159,6);
	      RectFill(152,159-sfu*136/100,159,158,5);
	      DispIt(58,180,71,196,147,138);};
	    if (sfs>0) {
	      RectFill(173,159-sfs*136/100,181,159,9);
	      RectFill(173,159-sfs*136/100,180,158,8);
	      DispIt(73,180,89,195,165,139);};
		       break;
		   case 1:
	    if (sfu>0) {
	      RectFill(212,159-sfu*136/100,220,159,6);
	      RectFill(212,159-sfu*136/100,219,158,5);
	      DispIt(91,178,115,195,198,139);};
	    if (sfs>0) {
	      RectFill(237,159-sfs*136/100,245,159,9);
	      RectFill(237,159-sfs*136/100,244,158,8);
	      DispIt(153,142,176,166,227,132);};
		       break;
		   case 2:
	    if (sfu>0) {
	      RectFill(272,159-sfu*136/100,280,159,6);
	      RectFill(272,159-sfu*136/100,279,158,5);
	      DispIt(121,142,151,166,253,132);};
	    if (sfs>0) {
	      RectFill(302,159-sfs*136/100,310,159,9);
	      RectFill(302,159-sfs*136/100,309,158,8);
	      DispIt(178,142,201,160,284,138);};
		       break;
		 default:break;
	      }
	     }
	    break;
    case 3: //PrintAt(100,150,"ADDITIONAL PROGRAMS");
	    sfu=-1;sfs=-1;
	    if (plr==0) {if (Data->P[0].Misc[3].Num>=0) sfu=Data->P[0].Misc[3].Safety;sfs=Data->P[0].Table[3][3];}
	      else if (plr==1) {if (Data->P[1].Misc[3].Num>=0) sfs=Data->P[1].Misc[3].Safety;sfu=Data->P[1].Table[3][3];}
		if (sfu>0) {
	      RectFill(19,159-sfu*136/100,27,159,6);
	      RectFill(19,159-sfu*136/100,26,158,5);
	      DispIt(68,65,76,75,17,145);};
	    if (sfs>0) {
	      RectFill(30,159-sfs*136/100,38,159,9);
	      RectFill(30,159-sfs*136/100,37,158,8);
	      DispIt(78,65,86,75,31,145);};

	    sfu=-1;sfs=-1;
	    if (plr==0) {if (Data->P[0].Misc[4].Num>=0) sfu=Data->P[0].Misc[4].Safety;sfs=Data->P[0].Table[3][4];}
	      else if (plr==1) {if (Data->P[1].Misc[4].Num>=0) sfs=Data->P[1].Misc[4].Safety;sfu=Data->P[1].Table[3][4];}
	    if (sfu>0) {
	      RectFill(72,159-sfu*136/100,80,159,6);
	      RectFill(72,159-sfu*136/100,79,158,5);
	      DispIt(88,62,100,75,64,143);};
	    if (sfs>0) {
	      RectFill(91,159-sfs*136/100,99,159,9);
	      RectFill(91,159-sfs*136/100,98,158,8);
	      DispIt(102,66,114,75,84,147);};

	     sfu=-1;sfs=-1;
	     if (plr==0) {if (Data->P[0].Rocket[4].Num>=0) sfu=Data->P[0].Rocket[4].Safety;sfs=Data->P[0].Table[1][4];}
	       else if (plr==1) {if (Data->P[1].Rocket[4].Num>=0) sfs=Data->P[1].Rocket[4].Safety;sfu=Data->P[1].Table[1][4];}

	    if (sfu>0) {
	      RectFill(118,159-sfu*136/100,126,159,6);
	      RectFill(118,159-sfu*136/100,125,158,5);
	      DispIt(1,120,14,151,113,125);};
	    if (sfs>0) {
	      RectFill(143,159-sfs*136/100,151,159,9);
	      RectFill(143,159-sfs*136/100,150,158,8);
	      DispIt(16,130,31,151,134,135);};

	    sfu=-1;sfs=-1;
		if (plr==0) {if (Data->P[0].Misc[0].Num>=0) sfu=Data->P[0].Misc[0].Safety;sfs=Data->P[0].Table[3][0];}
	      else if (plr==1) {if (Data->P[1].Misc[0].Num>=0) sfs=Data->P[1].Misc[0].Safety;sfu=Data->P[1].Table[3][0];}
	    if (sfu>0) {
	      RectFill(173,159-sfu*136/100,181,159,6);
	      RectFill(173,159-sfu*136/100,180,158,5);
	      DispIt(33,140,47,151,165,145);};
	    if (sfs>0) {
	      RectFill(195,159-sfs*136/100,203,159,9);
	      RectFill(195,159-sfs*136/100,202,158,8);
	      DispIt(49,138,61,151,188,143);};

	    sfu=-1;sfs=-1;
	    if (plr==0) {if (Data->P[0].Misc[1].Num>=0) sfu=Data->P[0].Misc[1].Safety;sfs=Data->P[0].Table[3][1];}
	      else if (plr==1) {if (Data->P[1].Misc[1].Num>=0) sfs=Data->P[1].Misc[1].Safety;sfu=Data->P[1].Table[3][1];}
	    if (sfu>0) {
	      RectFill(226,159-sfu*136/100,234,159,6);
	      RectFill(226,159-sfu*136/100,233,158,5);
	      DispIt(63,131,75,151,219,136);};
	    if (sfs>0) {
	      RectFill(246,159-sfs*136/100,254,159,9);
	      RectFill(246,159-sfs*136/100,253,158,8);
	      DispIt(77,129,88,151,240,134);};

      sfu=-1;sfs=-1;
	    if (plr==1) {if (Data->P[1].Misc[2].Num>=0) sfs=Data->P[1].Misc[2].Safety;}
	      else if (plr==0) sfs=Data->P[0].Table[3][2];
	    if (sfs>0){
	      RectFill(296,159-sfs*136/100,304,159,9);
	      RectFill(296,159-sfs*136/100,303,158,8);
	      DispIt(51,77,93,127,266,106);};
	    break;
    default:break;
  };
  
}

/* EOF */
