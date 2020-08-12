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
/** \file ast3.c Training and Hospital code
 */

#include <Buzz_inc.h>
#include <externs.h>
#include <options.h>


void DrawTrain(char plr,char lvl)
{
 char TrnName[20];
 char *Trner="TRAINING\0";
  
  if (lvl==0) {
   strcpy(IDT,"i038");strcpy(IKEY,"k038");
   }
   else {strcpy(IDT,"i037");strcpy(IKEY,"k037");}
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);
  ShBox(0,24,158,114);
  ShBox(161,24,319,199);
  ShBox(0,116,158,199);
  RectFill(5,129,19,195,0);
  RectFill(25,129,153,195,0);
  ShBox(6,130,18,161);ShBox(6,163,18,194);
  IOBox(243,3,316,19);IOBox(166,179,316,195);
  InBox(3,3,30,19);InBox(202,28,283,79);
  InBox(4,128,20,196);
  InBox(24,128,154,196);
  InBox(3,27,154,110);
  UPArrow(9,133);DNArrow(9,166);
  grSetColor(5);
  if (plr==0) PrintAt(20,124,"ASTRONAUTS");
  else PrintAt(20,124,"COSMONAUTS");
  PrintAt(0,0," IN TRAINING");
  memset(TrnName,0x00,sizeof(TrnName));
  switch(lvl) {
	case 0:strcpy(TrnName,"BASIC ");strcat(TrnName,Trner);
          DispBig(40,5,TrnName,0,-1);break;
	case 1:strcpy(TrnName,"CAPSULE ");strcat(TrnName,Trner);
          DispBig(37,5,TrnName,0,-1);break; 
	case 2:strcpy(TrnName,"LM ");strcat(TrnName,Trner);
          DispBig(40,5,TrnName,0,-1);break; 
	case 3:strcpy(TrnName,"EVA ");strcat(TrnName,Trner);
          DispBig(40,5,TrnName,0,-1);break; 
	case 4:strcpy(TrnName,"DOCKING ");strcat(TrnName,Trner);
          DispBig(37,5,TrnName,0,-1);break; 
	case 5:strcpy(TrnName,"ENDURANCE ");strcat(TrnName,Trner);
          DispBig(37,5,TrnName,0,-1);break; 
	default:break;
  };
  grSetColor(1);
  PrintAt(258,13,"CONTINUE");
  grSetColor(7);PrintAt(169,88,"NAME:");        /* 196,32 */
  grSetColor(1);
  PrintAt(169,97,"GROUP ");
  PrintAt(240,97,"TENURE: ");
  grMoveTo(201,97);

  /*  Data->P[plr].AstroLevel     201,41 */

  grSetColor(7);
  PrintAt(169,115,"STATUS:");
  grSetColor(9);PrintAt(169,133,"SKILL:");
  grSetColor(7);
  PrintAt(192,142,"CAP:"); PrintAt(192,150,"L.M.:");
  PrintAt(192,158,"E.V.A.:"); PrintAt(192,166,"DOCK:");
  PrintAt(192,174,"END:");
  grSetColor(9);PrintAt(181,189,"W");
  grSetColor(1);PrintAt(0,0,"ITHDRAW FROM TRAINING");
  RectFill(203,29,282,78,7+(plr*3));
  FlagSm(plr,4,4);
  
  return;
}

void
TrainText(char plr, int astro, int cnt)
{
	char Fritz[20];

	RectFill(200, 83, 291, 88, 3);
	RectFill(202, 93, 220, 97, 3);
	RectFill(216, 102, 294, 106, 3);
	RectFill(212, 111, 317, 124, 3);
	RectFill(215, 138, 235, 142, 3);
	RectFill(213, 145, 243, 150, 3);
	RectFill(221, 153, 241, 158, 3);
	RectFill(220, 161, 240, 166, 3);
	RectFill(215, 170, 235, 174, 3);
	RectFill(282, 91, 318, 97, 3);
	if (cnt == 0)
		return;
	grSetColor(1);
	PrintAt(200, 88, &Data->P[plr].Pool[astro].Name[0]);
	grMoveTo(212, 115);
	grSetColor(11);
	memset(Fritz, 0x00, sizeof(Fritz));
	if (Data->P[plr].Pool[astro].Status >= AST_ST_TRAIN_BASIC_1
		&& Data->P[plr].Pool[astro].Status <= AST_ST_TRAIN_BASIC_3)
		strncpy(Fritz, "BASIC TRAINING ", 14);

	switch (Data->P[plr].Pool[astro].Status)
	{
		case AST_ST_ACTIVE:
			PrintAt(0, 0, "ACTIVE");
			break;
		case AST_ST_TRAIN_BASIC_1:
			strcat(Fritz, " I");
			PrintAt(0, 0, &Fritz[0]);
			break;
		case AST_ST_TRAIN_BASIC_2:
			strcat(Fritz, " II");
			PrintAt(0, 0, &Fritz[0]);
			break;
		case AST_ST_TRAIN_BASIC_3:
			strcat(Fritz, " III");
			PrintAt(0, 0, &Fritz[0]);
			break;
		case AST_ST_TRAIN_ADV_1:
			PrintAt(0, 0, "ADV TRAINING I");
			break;
		case AST_ST_TRAIN_ADV_2:
			PrintAt(0, 0, "ADV TRAINING II");
			break;
		case AST_ST_TRAIN_ADV_3:
			PrintAt(0, 0, "ADV TRAINING III");
			break;
		case AST_ST_TRAIN_ADV_4:
			if (options.feat_shorter_advanced_training) 
				PrintAt(0, 0, "ADV TRAINING III");
			else	PrintAt(0, 0, "ADV TRAINING IV");
			break;
		default:
			break;
	};
	grSetColor(11);
	grMoveTo(203, 97);
	switch (Data->P[plr].Pool[astro].Group)
	{
		case 0:
			PrintAt(0, 0, "I");
			break;
		case 1:
			PrintAt(0, 0, "II");
			break;
		case 2:
			PrintAt(0, 0, "III");
			break;
		case 3:
			PrintAt(0, 0, "IV");
			break;
	};
	DispNum(282, 97, Data->P[plr].Pool[astro].Active);
	grSetColor(1);
	DispNum(215, 142, Data->P[plr].Pool[astro].Cap);
	DispNum(213, 150, Data->P[plr].Pool[astro].LM);
	DispNum(221, 158, Data->P[plr].Pool[astro].EVA);
	DispNum(220, 166, Data->P[plr].Pool[astro].Docking);
	DispNum(215, 174, Data->P[plr].Pool[astro].Endurance);
	AstFaces(plr, 203, 29, Data->P[plr].Pool[astro].Face);
	return;
}


/* level is as follows
	 0  =  Basic Training
	 1  =  Advanced Capsule
	 2  =  Advanced LM
	 3  =  Advanced EVA
	 4  =  Advanced Docking
	 5  =  Advanced Duration
*/

void Train(char plr,int level)
{
  int now2,BarA,count,i,M[100];
  char temp,Train[10];
  static int m=1;
  for (i=0;i<100;i++) M[i]=-1;
  now2=BarA=count=0;
  DrawTrain(plr,level);

  ShBox(26, 130 + BarA * 8, 152, 138 + BarA * 8);
  for (i = 0; i < Data->P[plr].AstroCount; i++)
  {
      if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_BASIC_1
          && Data->P[plr].Pool[i].Status <= AST_ST_TRAIN_BASIC_3
          && level == 0)
              M[count++] = i;
      if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_ADV_1
          && Data->P[plr].Pool[i].Status <= AST_ST_TRAIN_ADV_4
          && Data->P[plr].Pool[i].Focus == level)
              M[count++] = i;
  };
  
  DispLeft(plr,BarA,count,now2,&M[0]);
  if (count>0) TrainText(plr,M[0],count);
  FadeIn(2,pal,10,0,0);
  if (level==0)
   {
    if (m>4) m=1;
     else m++;
    level=m;
   }
  memset(Train,0x00,sizeof(Train));

	if (level==0)
		music_start((plr==0)?M_ASTTRNG:M_ASSEMBLY);
  else
		music_start(M_DRUMSM);
	
	switch(level) {
	case 1:strcpy(Train,(plr==0)?"UTCP":"STCP");
         Replay(plr,0,4,28,149,82,Train);break;
	case 2:strcpy(Train,(plr==0)?"UTLM":"STLM");
         Replay(plr,0,4,28,149,82,Train);break;
	case 3:strcpy(Train,(plr==0)?"UTEV":"STEV");
         Replay(plr,0,4,28,149,82,Train);break;
	case 4:strcpy(Train,(plr==0)?"UTDO":"STDO");
         Replay(plr,0,4,28,149,82,Train);break;
	case 5:strcpy(Train,(plr==0)?"UTDU":"STDU");
         Replay(plr,0,4,28,149,82,Train);break;
	default:break;
  };
  
  WaitForMouseUp();
  while (1)
  {
	key=0;GetMouse();
	for (i=0;i<8;i++) {  // Right Select Box
	  if (x>=27 && y>=(131+i*8) && x<=151 && y<=(137+i*8) && mousebuttons>0 && (now2-BarA+i)<=(count-1)) { // Left
		 
	 now2-=BarA; now2+=i; BarA=i;
	 RectFill(26,129,153,195,0);
	 ShBox(26,130+BarA*8,152,138+BarA*8);
	 DispLeft(plr,BarA,count,now2,&M[0]);
		 TrainText(plr,M[now2],count);
	 WaitForMouseUp();
	 
	  }
	}

	if (mousebuttons > 0 || key>0)
	{
	 if (((x>=6 && y>=130 && x<=18 && y<=161 && mousebuttons>0) || key==UP_ARROW) && count>0)
     { /* Lft Up */
	   InBox(6,130,18,161);
      for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
         
  	      if (BarA==0)
	       if (now2>0) {
		     now2--;
		     RectFill(26,129,153,195,0);
		     ShBox(26,130+BarA*8,152,138+BarA*8);
		     DispLeft(plr,BarA,count,now2,&M[0]);
		     TrainText(plr,M[now2],count);
	      };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      TrainText(plr,M[now2],count);
	     };
	     
        i=51;
	    }
	   }
      while (mousebuttons==1 || key==UP_ARROW)
       {
        delay(100);
        
	   if (BarA==0)
	    if (now2>0) {
		  now2--;
		  RectFill(26,129,153,195,0);
		  ShBox(26,130+BarA*8,152,138+BarA*8);
		  DispLeft(plr,BarA,count,now2,&M[0]);
		  TrainText(plr,M[now2],count);
	    };
	   if (BarA>0) {
	    RectFill(26,129,153,195,0);
	    BarA--;now2--;
	    ShBox(26,130+BarA*8,152,138+BarA*8);
	    DispLeft(plr,BarA,count,now2,&M[0]);
	    TrainText(plr,M[now2],count);
	   };
	   
        key=0;
        GetMouse();
       }
	   //WaitForMouseUp();
	   OutBox(6,130,18,161);delay(10);
     }
     else
     if (((x>=6 && y>=163 && x<=18 && y<=194 && mousebuttons>0) || key==DN_ARROW) && count>0)
      { /* Lft Dwn */
	    InBox(6,163,18,194);
       for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
        
	    if (BarA==7)
	     if (now2 < count-1) {
		   now2++;
		   RectFill(26,129,153,195,0);
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   TrainText(plr,M[now2],count);
	     };
	    if (BarA<7)
	     if (now2<count-1) {
	      RectFill(26,129,153,195,0);
	      BarA++;now2++;
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   TrainText(plr,M[now2],count);
	     };
	    
        i=51;
	    }
	   }
      while (mousebuttons==1 || key==DN_ARROW)
       {
        delay(100);
        
	    if (BarA==7)
	     if (now2 < count-1) {
		   now2++;
		   RectFill(26,129,153,195,0);
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   TrainText(plr,M[now2],count);
	     };
	    if (BarA<7)
	     if (now2<count-1) {
	      RectFill(26,129,153,195,0);
	      BarA++;now2++;
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   TrainText(plr,M[now2],count);
	     };
	    
        key=0;
        GetMouse();
       }
	    //WaitForMouseUp();
	    OutBox(6,163,18,194);delay(10);
	   }
     else
	  if (((x>=168 && y>=181 && x<=314 && y<=193 && mousebuttons>0) || key=='W') && count>0)
      {
	    InBox(168,181,314,193);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    OutBox(168,181,314,193);
       // Help box are you sure
       temp=0;if (plr==0) temp=Help("i102"); else temp=Help("i109");
       if (temp==1)
        {
	      Data->P[plr].Pool[M[now2]].TrainingLevel=Data->P[plr].Pool[M[now2]].Status;
	      Data->P[plr].Pool[M[now2]].Status=AST_ST_ACTIVE;
	      Data->P[plr].Pool[M[now2]].Assign=0;
	      if (Data->P[plr].Pool[M[now2]].Cap<0) Data->P[plr].Pool[M[now2]].Cap=0;
	      if (Data->P[plr].Pool[M[now2]].LM<0) Data->P[plr].Pool[M[now2]].LM=0;
	      if (Data->P[plr].Pool[M[now2]].EVA<0) Data->P[plr].Pool[M[now2]].EVA=0;
	      if (Data->P[plr].Pool[M[now2]].Docking<0) Data->P[plr].Pool[M[now2]].Docking=0;
	      if (Data->P[plr].Pool[M[now2]].Endurance<0) Data->P[plr].Pool[M[now2]].Endurance=0;
	      for (i=now2;i<count;i++) M[i]=M[i+1];
	      M[i]=-1;count--;
        
	      if (count==0) RectFill(203,29,282,78,7+(plr*3));
	      if (now2==count)
          {
	        if (now2>0) now2--;
	        if (BarA>0) BarA--;
	       };
	      RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      TrainText(plr,M[now2],count);
        
        }
	    
	  } /* end x-y if */
     else
	  if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER)
      {
	    InBox(245,5,314,17);
	    WaitForMouseUp();
       if (key>0) delay(150);
	    OutBox(245,5,314,17);
       music_stop();
	    return;  /* Done */
      }; /* end x-y if */
	} /* end mouse if */
  };  /* end while */
} /* end Limbo */


void Hospital(char plr,int sel)
{
  int now2,BarA,count,i,j,M[100];
  FILE *fin;
  long size;
  if (sel==0) {strcpy(IDT,"i041");strcpy(IKEY,"k041");}
    else {strcpy(IDT,"i020");strcpy(IKEY,"k020");}
  for (i=0;i<100;i++) M[i]=0;
  now2=0;BarA=count=0;
  
  FadeOut(2,pal,10,0,0);

// pal, len, image 
  
  fin=sOpen("CEM.IMG","rb",0);
  fread(&pal[0],768,1,fin);
  fread(&size,4,1,fin);
	Swap32bit(size);
  if (sel==1) {

    if (plr==1) {
      fseek(fin,size,SEEK_CUR);  // place on the Sov cem
      fread(&pal[0],768,1,fin);
      fread(&size,4,1,fin);
			Swap32bit(size);
      }
    }
  if (sel==0) {
    fseek(fin,size,SEEK_CUR);  // Skip past US cem
    fread(&pal[0],768,1,fin);     
    fread(&size,4,1,fin);
		Swap32bit(size);

    fseek(fin,size,SEEK_CUR);  // Place on the US hosp
    fread(&pal[0],768,1,fin);
    fread(&size,4,1,fin);
		Swap32bit(size);

    if (plr==1) {
      fseek(fin,size,SEEK_CUR);  // Skip to Sov hosp
      fread(&pal[0],768,1,fin);
      fread(&size,4,1,fin);
			Swap32bit(size);
      }
    }
  fread(screen,size,1,fin);
  fclose(fin);
  PCX_D(screen,vhptr.vptr,(unsigned int)size);

  gxClearDisplay(0,0);
  ShBox(0,0,319,22);
  ShBox(161,103,319,199);
  ShBox(0,103,158,199);IOBox(243,3,316,19);
  RectFill(5,129,19,195,0);RectFill(25,129,153,195,0);
  ShBox(6,163,18,194);ShBox(6,130,18,161);
  InBox(3,3,30,19);InBox(26,38,107,89);
  InBox(166,107,313,195);
  InBox(9,109,148,123);InBox(4,128,20,196);
  InBox(24,128,154,196);
  UPArrow(9,133);DNArrow(9,166);
  FlagSm(plr,4,4);
  ShBox(0,24,319,101);
  gxVirtualDisplay(&vhptr,0,0,1,25,318,100,0);
  
  gxVirtualDisplay(&vhptr,0,81,167,108,312,194,0);

  grSetColor(1);
  if (plr==0) {
    if (sel==0) {DispBig(40,5,"US HOSPITAL",0,-1);music_start(M_BADNEWS);}
    else {DispBig(37,5,"ARLINGTON CEMETERY",0,-1);music_start(M_USFUN);}
  };
  if (plr==1) {
    if (sel==0) {DispBig(40,5,"SOVIET INFIRMARY",0,-1);music_start(M_INTERLUD);}
    else {DispBig(40,5,"KREMLIN WALL",0,-1);music_start(M_SVFUN);}
  }
  grSetColor(1);
  PrintAt(257,13,"CONTINUE");
  grSetColor(11);
  if (plr==0) PrintAt(25,118,"ASTRONAUT"); else PrintAt(25,118,"COSMONAUT");
  PrintAt(0,0," SELECTION");
  ShBox(26,130+BarA*8,152,138+BarA*8);
  

  j=(sel==0) ? AST_ST_INJURED : AST_ST_DEAD;
  for (i=0;i<Data->P[plr].AstroCount;i++)
    if (Data->P[plr].Pool[i].Status==j) {
      M[count++]=i;
    };
  DispLeft(plr,BarA,count,now2,&M[0]);
  FadeIn(2,pal,10,0,0);
  WaitForMouseUp();
  while (1)
  {
    key=0;GetMouse();
    for (i=0;i<8;i++)
     { 
      if (x>=27 && y>=(131+i*8) && x<=151 && y<=(137+i*8) && mousebuttons>0 && (now2-BarA+i)<=(count-1))
       { 
	     
	     now2-=BarA; now2+=i; BarA=i;
	     RectFill(26,129,153,195,0);
	     ShBox(26,130+BarA*8,152,138+BarA*8);
	     DispLeft(plr,BarA,count,now2,&M[0]);
	     WaitForMouseUp();
	     
	    }
     }  
      if (((x>=6 && y>=130 && x<=18 && y<=161 && mousebuttons>0) || key==UP_ARROW) && count>0)
       { // Up
	     InBox(6,130,18,161);
	     if (BarA==0)
	      if (now2>0) {
	       now2--;
	       RectFill(26,129,153,195,0);
	       ShBox(26,130+BarA*8,152,138+BarA*8);
	       DispLeft(plr,BarA,count,now2,&M[0]);
	      };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	     };
	    
	   // WaitForMouseUp();
	    OutBox(6,130,18,161);delay(10);
      }
    else 
    if (((mousebuttons>0 && x>=6 && y>=163 && x<=18 && y<=194) || key==DN_ARROW) && count>0)
     {  // Down 
	   InBox(6,163,18,194);
	   if (BarA==7)
	    if (now2 < count-1) {
	     now2++;
	     RectFill(26,129,153,195,0);
	     ShBox(26,130+BarA*8,152,138+BarA*8);
	     DispLeft(plr,BarA,count,now2,&M[0]);
	    };
	   if (BarA<7)
	    if (now2<count-1) {
	     RectFill(26,129,153,195,0);
	     BarA++;now2++;
	     ShBox(26,130+BarA*8,152,138+BarA*8);
		  DispLeft(plr,BarA,count,now2,&M[0]);
	    };
	   
	  // WaitForMouseUp();
	   OutBox(6,163,18,194);delay(10);
     };
   if ((mousebuttons>0 && x>=245 && y>=5 && x<=314 && y<=17) || key==K_ENTER)
    {
	  InBox(245,5,314,17);
	  WaitForMouseUp();
    music_stop();
	  return;  /* Done */
    }; /* end x-y if */
  };  /* end while */
}


















