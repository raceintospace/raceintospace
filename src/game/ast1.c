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
/** \file ast1.c Astronaut Recruitment
 */

#include <Buzz_inc.h>
#include <externs.h>
#include <options.h>   //Naut Randomize && Naut Compatibilit, Nikakd, 10/8/10

extern char MCol[110],sel[30],MaxSel;
extern struct ManPool *Men;

/** display list of 'naut names
 * 
 */
void DispEight(char now,char loc)
{
  int i,start;
  start=now-loc;
  for (i=start;i<start+8;i++) {
    if (MCol[i]==1) grSetColor(8);
    else grSetColor( 6+(Men[i].Sex+1)*6 );
    PrintAt(189,136+(i-start)*8,&Men[i].Name[0]);
  };
  RectFill(206,48,306,52,3);
  RectFill(221,57,306,61,3);
  RectFill(293,66,301,70,3);
  RectFill(274,98,281,102,3);
  grSetColor(1);
  if (Men[now].Sex==0) PrintAt(206,52,"MR. ");
  else PrintAt(206,52,"MS. ");
  PrintAt(0,0,&Men[now].Name[0]);
  DispNum(294,70,Men[now].Cap);
  DispNum(275,102,Men[now].Endurance);
  return;
} /* End of Disp8 */

void DispEight2(int nw,int lc,int cnt)
{
  int i,start,num;
  start=nw-lc;
  num= (cnt<8) ? cnt : 8;

  grSetColor(11);
  for (i=start;i<start+num;i++) {
    if (sel[i]!=-1) {
      grSetColor( 6+(Men[sel[i]].Sex+1)*6 );
      PrintAt(28,136+(i-start)*8,&Men[ sel[i] ].Name[0]);
    }
   }

   RectFill(45,48,145,52,3);
   RectFill(60,57,145,61,3);
   RectFill(132,66,140,70,3);
   RectFill(113,98,120,102,3);
   RectFill(292,36,310,41,7);
   grSetColor(11);
   DispNum(292,41,MaxSel-cnt);
  if (cnt>0) {
    grSetColor(1);
    if (Men[sel[nw]].Sex==0) PrintAt(45,52,"MR. ");
    else PrintAt(45,52,"MS. ");
    PrintAt(0,0,&Men[sel[nw]].Name[0]);
    DispNum(133,70,Men[sel[nw]].Cap);
    DispNum(114,102,Men[sel[nw]].Endurance);
  };
  return;
}


void DrawAstCheck(char plr)
{
  int i,pos,ad=0;

  pos = 0; /* XXX check uninitialized */

  if (Data->P[plr].AstroDelay>0) ad=1;
  
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(80,44,237,155);
  InBox(87,49,230,103);RectFill(88,50,229,102,7+plr*3);
  IOBox(98,133,150,149);IOBox(166,133,218,149);
  grSetColor(5);
  if (plr==0) PrintAt(99,60,"ASTRONAUT");
  else PrintAt(99,60,"COSMONAUT");
  PrintAt(0,0," RECRUITMENT");
  grSetColor(11);
  PrintAt(100,73,"GROUP ");
  switch (Data->P[plr].AstroLevel) {
    case 0:PrintAt(0,0,"I");pos=ASTRO_POOL_LVL1;break;
    case 1:PrintAt(0,0,"II");pos=ASTRO_POOL_LVL2;break;
    case 2:PrintAt(0,0,"III");pos=ASTRO_POOL_LVL3;break;
    case 3:PrintAt(0,0,"IV");pos=ASTRO_POOL_LVL4;break;
    case 4:PrintAt(0,0,"V");pos=ASTRO_POOL_LVL5;break;
  };
  if (Data->Season==0) PrintAt(160,73,"SPRING 19"); else PrintAt(170,73,"FALL 19");
  DispNum(0,0,Data->Year);
  if (Data->P[plr].AstroLevel==0) i=20; else i=15;
  if (ad==0) {
    if (pos<10) DispNum(110,86,pos); else DispNum(108,86,pos);
    PrintAt(0,0," POSITIONS TO FILL");
    PrintAt(133,97,"COST: ");DispNum(0,0,i);PrintAt(0,0," MB");
  }else {
    if (Data->P[plr].AstroDelay!=1) DispNum(114,86,Data->P[plr].AstroDelay); else DispNum(118,86,Data->P[plr].AstroDelay);
    PrintAt(0,0," SEASON");
    if (Data->P[plr].AstroDelay!=1) PrintAt(0,0,"S");
    PrintAt(0,0," TO WAIT");
    PrintAt(104,97,"FOR THE NEW RECRUITS");
  }
  grSetColor(1);

  if (ad==1) {
    PrintAt(102,113,"  YOU CANNOT RECRUIT");
    if (plr==0) PrintAt(102,122,"ASTRONAUTS THIS TURN");
    else PrintAt(102,122,"COSMONAUTS THIS TURN");
    grSetColor(8);
    PrintAt(114,143,"EXIT");
    PrintAt(182,143,"EXIT");
  };

 if (ad==0) {
  if (Data->P[plr].Cash<i) {
    PrintAt(110,113,"YOU CANNOT AFFORD");
    if (plr==0) PrintAt(103,122,"ASTRONAUTS THIS TURN");
    else PrintAt(103,122,"COSMONAUTS THIS TURN");
    grSetColor(8);
    PrintAt(113,143,"EXIT");
    PrintAt(181,143,"EXIT");
  } else {
    PrintAt(101,113,"DO YOU WISH TO RECRUIT");
    if (plr==0) PrintAt(100,122,"ASTRONAUTS THIS TURN?");
    else PrintAt(100,122,"COSMONAUTS THIS TURN?");
    grSetColor(6);PrintAt(116,143,"Y");grSetColor(1);PrintAt(0,0,"ES");
    grSetColor(6);PrintAt(187,143,"N");grSetColor(1);PrintAt(0,0,"O");
  };
 };
  FlagSm(plr,4,4);
  FadeIn(2,pal,10,0,0);
  
  return;
}

/** Draw selection screen for Astronaut recruitment
 * 
 */
void DrawAstSel(char plr)
{
  
  strcpy(IDT,"i012");
  strcpy(IKEY,"k012");
  FadeOut(2,pal,10,0,0);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);
  ShBox(0,24,158,199); ShBox(161,24,319,199);
  RectFill(5,129,19,195,0); RectFill(166,129,180,195,0);
  RectFill(25,129,153,195,0); RectFill(186,129,314,195,0);
  ShBox(6,130,18,161);  ShBox(6,163,18,194);
  ShBox(167,130,179,161); ShBox(167,163,179,194);
  IOBox(243,3,316,19); IOBox(5,109,153,125);
  IOBox(166,109,315,125);
  InBox(4,128,20,196);  InBox(24,128,154,196);
  InBox(185,128,315,196); InBox(165,128,181,196);
  InBox(3,3,30,19);
  UPArrow(9,133);DNArrow(9,166); //Left
  UPArrow(170,133);DNArrow(170,166); //Right
  grSetColor(1);
  PrintAt(258,13,"CONTINUE");
  DispBig(35,5,"RECRUITMENT",0,-1);
  FlagSm(plr,4,4);

  InBox(3,27,155,44);RectFill(4,28,154,43,7);
  InBox(164,27,316,44);RectFill(165,28,315,43,7);

  grSetColor(1);
  if (plr==0) PrintAt(22,34,"ASTRONAUT SELECTION");
  else PrintAt(22,34,"COSMONAUT SELECTION");
  PrintAt(192,34,"POOL OF APPLICANTS");

  grSetColor(11);
  PrintAt(12,41,"GROUP ");
  switch (Data->P[plr].AstroLevel) {
    case 0:PrintAt(0,0,"I");break;
    case 1:PrintAt(0,0,"II");break;
    case 2:PrintAt(0,0,"III");break;
    case 3:PrintAt(0,0,"IV");break;
    case 4:PrintAt(0,0,"V");break;
  };

  PrintAt(185,41,"REMAINING POSITIONS:");

  if (Data->Season==0) PrintAt(88,41,"SPRING 19"); else PrintAt(98,41,"FALL 19");
  DispNum(0,0,Data->Year);
  grSetColor(9);
  PrintAt(12,52,"NAME:");  PrintAt(173,52,"NAME:");
  PrintAt(12,61,"SERVICE:");PrintAt(173,61,"SERVICE:");
  PrintAt(12,70,"SKILLS:");PrintAt(173,70,"SKILLS:");
  grSetColor(11);
  PrintAt(54,70,"CAPSULE PILOT:");PrintAt(215,70,"CAPSULE PILOT:");
  grSetColor(11);PrintAt(54,78,"L.M. PILOT: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(215,78,"L.M. PILOT: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(54,86,"E.V.A.: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(215,86,"E.V.A.: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(54,94,"DOCKING: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(215,94,"DOCKING: ");grSetColor(1);PrintAt(0,0,"--");
  grSetColor(11);PrintAt(54,102,"ENDURANCE:");PrintAt(215,102,"ENDURANCE:");
  grSetColor(6);
  PrintAt(33,119,"D");grSetColor(1);PrintAt(0,0,"ISMISS APPLICANT");
  grSetColor(6);PrintAt(194,119,"R");grSetColor(1);PrintAt(0,0,"ECRUIT APPLICANT");
  
  return;
}

//Naut Randomize, Nikakd, 10/8/10
void RandomizeNauts() {
  int i;
	for (i=0;i<106;i++) {
		Men[i].Cap = random(5);
		Men[i].LM  = random(5);
		Men[i].EVA = random(5);
		Men[i].Docking = random(5);
		Men[i].Endurance = random(5);
		}
}

void AstSel(char plr)
{
char i,j,k,BarA,BarB,MaxMen,Index,now,now2,max,min,count,fem=0,ksel=0;
FILE *fin;
i=j=k=BarA=BarB=MaxMen=Index=now=now2=max=min=count=0;
music_start(M_DRUMSM);
DrawAstCheck(plr);
  WaitForMouseUp();
  i=0;
  while (i==0)
   {
    key=0;GetMouse();
    if ((x>=100 && y>=135 && x<=148 && y<=147 && mousebuttons>0) || key=='Y')
     {
      
      InBox(100,135,148,147);
      WaitForMouseUp();
      if (Data->P[plr].AstroLevel==0) Index=20; else Index=15;
      if (Data->P[plr].Cash>=Index) i=1; else i=2;
      if (Data->P[plr].AstroDelay>0) i=2;
      if (key>0) delay(150);
      OutBox(100,135,148,147); 
     }
    if ((x>=168 && y>=135 && x<=216 && y<=147 && mousebuttons>0)
      || key=='N' || key==K_ESCAPE || key==K_ENTER)
     {
      
      InBox(168,135,216,147);
      WaitForMouseUp();
      if (key>0) delay(150);
      i=2;OutBox(168,135,216,147);
     }
  };
  if (i==2) {music_stop();return;} /* too poor for astronauts or NO */
  BarA=0;BarB=0;
  DrawAstSel(plr);
  
  ShBox(26,130+BarA*8,152,138+BarA*8); ShBox(187,130+BarB*8,313,138+BarB*8);
  
  memset(sel,-1,sizeof(sel));
  memset(MCol,0x00,sizeof(MCol));
  Men=(struct ManPool *) buffer;
  fin = sOpen("MEN.DAT","rb",1);   /* Open Astronaut Data File  */
  fseek(fin,((sizeof (struct ManPool))*106)*plr,SEEK_SET);
  fread(Men,(sizeof (struct ManPool))*106,1,fin);
  fclose(fin);
  if (options.feat_random_nauts==1) RandomizeNauts();  //Naut Randomize, Nikakd, 10/8/10
  switch(Data->P[plr].AstroLevel) {
    case 0: MaxMen=10;  MaxSel=ASTRO_POOL_LVL1;  Index=0;
	    if (Data->P[plr].Female==1) {MaxMen+=3;fem=1;}
	    Data->P[plr].Cash-=5;
	    Data->P[plr].Spend[0][2]+=5;
	    break;
    case 1: MaxMen=17;  MaxSel=ASTRO_POOL_LVL2;  Index=14;
	    if (Data->P[plr].Female==1) {MaxMen+=3;fem=1;}
	    break;
    case 2: MaxMen=19;  MaxSel=ASTRO_POOL_LVL3;  Index=35;
	    if (Data->P[plr].Female==1) {MaxMen+=3;fem=1;}
	    break;
    case 3: MaxMen=27;  MaxSel=ASTRO_POOL_LVL4;  Index=58;
	    break;
    case 4: MaxMen=19;  MaxSel=ASTRO_POOL_LVL5;  Index=86;
	    break;
  }; /* End of switch */
  Data->P[plr].Cash-=15;
  Data->P[plr].Spend[0][2]+=15;
  now=Index;max=Index+MaxMen;min=Index;
  now2=0;count=0;  /* counter for # selected */
  
  DispEight(now,BarB);
  DispEight2(now2,BarA,count);
  FadeIn(2,pal,10,0,0);
  
  WaitForMouseUp();
  while (1)
  {
   key=0;GetMouse();
   for (i=0;i<8;i++)
    {  // Right Select Box
     if (((x>=188 && y>=(131+i*8) && x<=312 && y<=(137+i*8) && mousebuttons>0)
      || (key==RT_ARROW && ksel==1)) && (now-BarB+i)<=max)
      {
	    
       if (ksel==1) ksel=0;
    	 RectFill(186,129,314,195,0);
    	 now-=BarB; now+=i;BarB=i;
	    ShBox(187,130+BarB*8,313,138+BarB*8);
	    DispEight(now,BarB);
    	 WaitForMouseUp();
    	 
      }
     if (((x>=27 && y>=(131+i*8) && x<=151 && y<=(137+i*8) && mousebuttons>0)
      || (key==LT_ARROW && ksel==0)) && (now2-BarA+i)<=(count-1))
      { // Left Select Box
    	 
       if (ksel==0) ksel=1;
	    RectFill(26,129,153,195,0);
	    now2-=BarA; now2+=i;BarA=i;
	    ShBox(26,130+BarA*8,152,138+BarA*8);
	    DispEight2(now2,BarA,count);
    	 WaitForMouseUp();
    	 
      }
    }
    // else
    if (((x>=6 && y>=130 && x<=18 && y<=161 && mousebuttons>0) || (key==UP_ARROW && ksel==1)) && count>0)
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
	      DispEight2(now2,BarA,count);
	     };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispEight2(now2,BarA,count);
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
	      DispEight2(now2,BarA,count);
	     };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispEight2(now2,BarA,count);
	     };
    	  
        key=0;
        GetMouse();
       }
	    //WaitForMouseUp();
     	 OutBox(6,130,18,161);delay(10);
     }
     else
     if (((x>=6 && y>=163 && x<=18 && y<=194 && mousebuttons>0) || (key==DN_ARROW && ksel==1)) && count>0)
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
	      DispEight2(now2,BarA,count);
	     };
	    if (BarA<7)
	     if (now2<count-1) {
	      RectFill(26,129,153,195,0);
	      BarA++;now2++;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispEight2(now2,BarA,count);
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
	      DispEight2(now2,BarA,count);
	     };
	    if (BarA<7)
	     if (now2<count-1) {
	      RectFill(26,129,153,195,0);
	      BarA++;now2++;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispEight2(now2,BarA,count);
	     };
        
        key=0;
        GetMouse();
       }
        // WaitForMouseUp();
	      OutBox(6,163,18,194);delay(10);
      }
      else
      if ((x>=167 && y>=130 && x<=179 && y<=161 && mousebuttons>0) || (key==UP_ARROW && ksel==0))
       { /* Rt Up */
	     InBox(167,130,179,161);
        for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
         
	      if (BarB==0)
	       if (now > min) {
	        now--;
	        RectFill(186,129,314,195,0);
	        ShBox(187,130+BarB*8,313,138+BarB*8);
	        DispEight(now,BarB);
	       };   
	     if (BarB>0) {
	      RectFill(186,129,314,195,0);
	      BarB--;now--;
	      ShBox(187,130+BarB*8,313,138+BarB*8);
	      DispEight(now,BarB);
	     };
        
         i=51;
	    }
	   }
      while (mousebuttons==1 || key==UP_ARROW)
       {
        delay(100);
        
	      if (BarB==0)
	       if (now > min) {
	        now--;
	        RectFill(186,129,314,195,0);
	        ShBox(187,130+BarB*8,313,138+BarB*8);
	        DispEight(now,BarB);
	       };   
	     if (BarB>0) {
	      RectFill(186,129,314,195,0);
	      BarB--;now--;
	      ShBox(187,130+BarB*8,313,138+BarB*8);
	      DispEight(now,BarB);
	     };
        
        key=0;
        GetMouse();
       }
	     // WaitForMouseUp();
	      OutBox(167,130,179,161);delay(10);
        }
      else
      if ((x>=167 && y>=163 && x<=179 && y<=194 && mousebuttons>0) || (key==DN_ARROW && ksel==0))
       { /* Rt Dwn */
     	  InBox(167,163,179,194);
         for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
         
	      if (BarB==7)
	       if (now <= max) {
	       if (now<max) now++;
	       RectFill(186,129,314,195,0);
	       ShBox(187,130+BarB*8,313,138+BarB*8);
	       DispEight(now,BarB);
	      };
	     if (BarB<7) {
	      RectFill(186,129,314,195,0);
	      BarB++;now++;
	      ShBox(187,130+BarB*8,313,138+BarB*8);
	      DispEight(now,BarB);
	     };
         
          i=51;
	    }
	   }
      while (mousebuttons==1 || key==DN_ARROW)
       {
        delay(100);
        
	      if (BarB==7)
	       if (now <= max) {
	       if (now<max) now++;
	       RectFill(186,129,314,195,0);
	       ShBox(187,130+BarB*8,313,138+BarB*8);
	       DispEight(now,BarB);
	      };
	     if (BarB<7) {
	      RectFill(186,129,314,195,0);
	      BarB++;now++;
	      ShBox(187,130+BarB*8,313,138+BarB*8);
	      DispEight(now,BarB);
	     };
        
        key=0;
        GetMouse();
       }
	    // WaitForMouseUp();
	     OutBox(167,163,179,194);delay(10);
       }
      else
      if ((x>=7 && y>=111 && x<=151 && y<=123 && count>0 && mousebuttons>0) || (key=='D' && count>0))
       { /* Dismiss */
	     InBox(7,111,151,123);
	     count--;
    	  MCol[sel[now2]]=0;
    	  for (i=now2;i<count+1;i++)
	     sel[i]=sel[i+1];
   	  sel[i]=-1;  /* remove astronaut from left */
   	  if (now2==count) {
   	    if (now2>0) now2--;
   	    if (BarA>0) BarA--;
   	  };
   	  RectFill(26,129,153,195,0);
   	  ShBox(26,130+BarA*8,152,138+BarA*8);
   	  DispEight2(now2,BarA,count);
   	  DispEight(now,BarB);
   	  
   	  WaitForMouseUp();
        if (key>0) delay(110);
   	  OutBox(7,111,151,123);
       }
      else
      if ((x>=164 && y>=111 && x<=313 && y<=123 && MCol[now]==0 && mousebuttons>0) || (key=='R' && MCol[now]==0))
       { /* Recruit */
	     InBox(168,111,313,123);
	     if (count<MaxSel) {
	       sel[count]=now; /* move astronaut into left */
	       MCol[now]=1;
	       count++;
	       if (BarB==7)
	         if (now < max) {
	           now++;
	           RectFill(186,129,314,195,0);
	           ShBox(187,130+BarB*8,313,138+BarB*8);
	           DispEight(now,BarB);
	         };
	       if (BarB<7) {
	         RectFill(186,129,314,195,0);
	         BarB++;now++;
	         ShBox(187,130+BarB*8,313,138+BarB*8);
	         DispEight(now,BarB);
	       };

	      RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispEight2(now2,BarA,count);
	   };
	   
	   WaitForMouseUp();
      if (key>0) delay(110);
	   OutBox(168,111,313,123);
     };

      if ((x>=245 && y>=5 && x<=314 && y<=17 && mousebuttons>0) || key==K_ENTER) {   /* Exit - not 'til done */
       if (fem==1) {
         j=0;
         for (i=0;i<count;i++) if (Men[sel[i]].Sex==1) j++;
         if (j>0) fem=0;
         else Help("i100");
         }
       if (fem==0 && count == MaxSel) {
	 InBox(245,5,314,17);
	 WaitForMouseUp();
    if (key>0) delay(150);
	 for (i=0;i<count;i++) {
	   strcpy(&Data->P[plr].Pool[i+Data->P[plr].AstroCount].Name[0],&Men[sel[i]].Name[0]);
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Sex=Men[sel[i]].Sex;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Cap=Men[sel[i]].Cap;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].LM=Men[sel[i]].LM;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].EVA=Men[sel[i]].EVA;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Docking=Men[sel[i]].Docking;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Endurance=Men[sel[i]].Endurance;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Status=AST_ST_TRAIN_BASIC_1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].TrainingLevel=1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Group=Data->P[plr].AstroLevel;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].CR=random(2)+1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].CL=random(2)+1;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Task=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Crew=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Una=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Pool=0;
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Compat=random(options.feat_compat_nauts)+1;  //Naut Compatibility, Nikakd, 10/8/10
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Mood=85+5*random(4);
	   Data->P[plr].Pool[i+Data->P[plr].AstroCount].Face=random(77);
	   if (Data->P[plr].Pool[i+Data->P[plr].AstroCount].Sex==1) 
	     Data->P[plr].Pool[i+Data->P[plr].AstroCount].Face=77+random(8);    
	   k=random(10)+1;
	   if (k>6) {
	     k=random(2);
	     if (k==0) Data->P[plr].Pool[i+Data->P[plr].AstroCount].Endurance--;
	     else Data->P[plr].Pool[i+Data->P[plr].AstroCount].Cap--;
	   };
	   for (j=0;j<3;j++) {
	     k=random(3);
	     switch(k)
	     {
	       case 0: Data->P[plr].Pool[i+Data->P[plr].AstroCount].LM--;
		       break;
	       case 1: Data->P[plr].Pool[i+Data->P[plr].AstroCount].EVA--;
		       break;
	       case 2: Data->P[plr].Pool[i+Data->P[plr].AstroCount].Docking--;
		       break;
	       default: break;
	     }
	   };
	 };
	 Data->P[plr].AstroLevel++;
	 Data->P[plr].AstroCount+=count;
	   switch(Data->P[plr].AstroLevel) {
	     case 1:Data->P[plr].AstroDelay=6;break;
	     case 2: case 3: Data->P[plr].AstroDelay=4;break;
	     case 4: Data->P[plr].AstroDelay=8;break;
	     case 5: Data->P[plr].AstroDelay=99;break;
	  };
	 OutBox(245,5,314,17);
    music_stop();
	 return;  /* Done */
       };
      };
  }
}
