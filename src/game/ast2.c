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

int lenprogname;  // Variable to hold and manipulate length of program name
int skilLev;  // Variable for 'naut's skill level (so 4s don't go to Adv Training, and 3s go to Adv III)

void DrawLimbo(char plr)
{
   int i;
   FadeOut(2,pal,10,0,0);
   
   strcpy(IDT,"i039");strcpy(IKEY,"k039");
   gxClearDisplay(0,0);
   ShBox(0,0,319,22);InBox(3,3,30,19);IOBox(243,3,316,19);
   grSetColor(1);PrintAt(258,13,"CONTINUE");
   if (plr==0) DispBig(36,5,"ASTRONAUT COMPLEX",0,-1);
     else DispBig(40,5,"COSMONAUT CENTER",0,-1);
   ShBox(0,24,158,199);
   InBox(9,51,90,102);
   RectFill(25,129,153,195,0);RectFill(5,129,19,195,0);
   ShBox(6,130,18,161);ShBox(6,163,18,194);
   UPArrow(8,133);DNArrow(8,166);
   ShBox(161,24,319,72);InBox(165,28,315,42);RectFill(166,29,314,41,7);
   ShBox(161,74,319,199);InBox(165,77,315,89);RectFill(166,78,314,88,10);
   RectFill(10,52,89,101,7+plr*3);
   InBox(165,46,315,67);RectFill(166,47,314,66,0);
   ShBox(167,48,239,65);ShBox(241,48,313,65);
   InBox(167,48,239,65);
   InBox(4,128,20,196);  InBox(24,128,154,196);
    // new boxes
    OutBox(0,24,158,47);OutBox(0,105,158,128);
    InBox(9,28,148,42);InBox(9,108,148,123);

   grSetColor(20);
   PrintAt(180,55,"TRANSFER");
   if (plr==0) PrintAt(178,62,"ASTRONAUT");
   else PrintAt(178,62,"COSMONAUT");

   PrintAt(256,55,"FACILITY");PrintAt(254,62,"TRANSFER");

   RectFill(166,78,314,88,10);
   grSetColor(11);PrintAt(185,85,"TRANSFER TO LOCATION");

   for (i=0;i<5;i++) {
      IOBox(165,93+21*i,238,111+21*i);
      IOBox(242,93+21*i,315,111+21*i);
      if (Data->P[plr].Manned[i].Num<0) {
	 InBox(167,95+21*i,236,109+21*i);
	 grSetColor(17);
	 }
      else grSetColor(11);
      lenprogname=(7-strlen(Data->P[plr].Manned[i].Name))*3;
      //lenprogname=(7-lenprogname)*2
      PrintAt(181+lenprogname,101+21*i,Data->P[plr].Manned[i].Name);
      PrintAt(181,107+21*i,"PROGRAM");

      grSetColor(11);
      PrintAt(257,107+21*i,"TRAINING");
      //grMoveTo(254,101+21*i);  This remmed out to manually place the Adv. Training names so they could be centered -Leon

      switch(i) {
	 case 0: PrintAt(258,101+21*i,"CAPSULE");break;
	 case 1: PrintAt(271,101+21*i,"L.M.");break;
	 case 2: PrintAt(268,101+21*i,"E.V.A.");break;
	 case 3: PrintAt(259,101+21*i,"DOCKING");break;
	 case 4: PrintAt(252,101+21*i,"ENDURANCE");break;
	 default: break;
	 }


      }

   grSetColor(11);PrintAt(187,37,"OPERATION SELECTION");

   grSetColor(20);
   //PrintAt(258,13,"CONTINUE");
   PrintAt(15,37,"NAME:");
   PrintAt(17,118,"GROUP:");
   PrintAt(88,118,"TENURE:");
   grSetColor(11);
   PrintAt(102,60,"MOOD:");
   PrintAt(102,68,"CAP:");
   PrintAt(102,76,"L.M.:");   
   PrintAt(102,84,"EVA:");   
   PrintAt(102,92,"DOCK:");   
   PrintAt(102,100,"END:");   
   FlagSm(plr,4,4);
   
   return;
}

void Limbo(char plr)
{
  int i,M[100],BarA,count,now2,tag=0;
  memset(M,-1,sizeof M);
  DrawLimbo(plr);
  music_start((plr==0)?M_ASTTRNG:M_ASSEMBLY);
  
  now2=BarA=count=0;
  ShBox(26,130+BarA*8,152,138+BarA*8);
  for (i=0;i<Data->P[plr].AstroCount;i++)
  if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0) {
	  M[count++]=i;
  };
  DispLeft(plr,BarA,count,now2,&M[0]);
  if (count>0) LimboText(plr,M[now2]);
  FadeIn(2,pal,10,0,0);
  
  while (1)
  {
   key=0;WaitForMouseUp();key=0;
	GetMouse();

   //Mouse ManSelect from being Clicked on
	for (i=0;i<8;i++) {
	   if (x>=27 && y>=(131+i*8) && x<=151 && y<=(137+i*8) && mousebuttons>0 && (now2-BarA+i)<=(count-1)) {
	      now2-=BarA; now2+=i; BarA=i;
	      RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      WaitForMouseUp();
	      
	 }
   }
   // Left Arrow Up
	if ((mousebuttons>0 && x>=6 && y>=130 && x<=18 && y<=161 && count>0)|| key==UP_ARROW) {
	   InBox(6,130,18,161);
      for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
         
	     if (BarA==0 && now2>0) {
	      now2--;
	      RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
 	     
         i=51;
	    }
	   }
      while (mousebuttons==1 || key==UP_ARROW)
       {
        delay(100);
        
  	     if (BarA==0 && now2>0) {
	      now2--;
	      RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
	     if (BarA>0) {
	      RectFill(26,129,153,195,0);
	      BarA--;now2--;
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
	     
        key=0;
        GetMouse();
       }
      //WaitForMouseUp();key=0;
	   OutBox(6,130,18,161);delay(10);
      };

   // Left Arrow Down
	if ((mousebuttons>0 && x>=6 && y>=163 && x<=18 && y<=194 && count>0) || key==DN_ARROW) {
	   InBox(6,163,18,194);
      for (i=0;i<50;i++)
       {
	     key=0;
	     GetMouse();
	     delay(10);
	     if (mousebuttons==0) {
         
  	     if (BarA==7 && (now2 < count-1)) {
	      now2++;
		   RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
	     if (BarA<7 && now2<count-1) {
		   RectFill(26,129,153,195,0);
		   BarA++;now2++;
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   LimboText(plr,M[now2]);
	      };
	     
        i=51;
	    }
	   }
      while (mousebuttons==1 || key==DN_ARROW)
       {
        delay(100);
        
	     if (BarA==7 && (now2 < count-1)) {
	      now2++;
		   RectFill(26,129,153,195,0);
	      ShBox(26,130+BarA*8,152,138+BarA*8);
	      DispLeft(plr,BarA,count,now2,&M[0]);
	      LimboText(plr,M[now2]);
	      };
	     if (BarA<7 && now2<count-1) {
		   RectFill(26,129,153,195,0);
		   BarA++;now2++;
		   ShBox(26,130+BarA*8,152,138+BarA*8);
		   DispLeft(plr,BarA,count,now2,&M[0]);
		   LimboText(plr,M[now2]);
	      };
	     
        key=0;
        GetMouse();
       }
      //key=0;WaitForMouseUp();key=0;
	   OutBox(6,163,18,194);delay(10);
	   };

   // Continue
	if ((mousebuttons>0 && x>=245 && y>=5 && x<=314 && y<=17) || key==K_ENTER ){
      InBox(245,5,314,17);
      WaitForMouseUp();
      if (key>0) delay(150);
      OutBox(245,5,314,17);
      music_stop();
	   return; 
      };

   // Select Transfer Button
      if ((mousebuttons>0 && x>=167 && y>=48 && x<=239 && y<=65) || key==LT_ARROW) {
	 tag=0;
	 InBox(167,48,239,65);OutBox(241,48,313,65);
	 RectFill(166,78,314,88,10);
	 grSetColor(11);PrintAt(185,85,"TRANSFER TO LOCATION");
	 
	 }

   // Select Visit To Button
      if ((mousebuttons>0 && x>=241 && y>=48 && x<=313 && y<=65) || key==RT_ARROW) {
	 tag=1;
	 InBox(241,48,313,65);OutBox(167,48,239,65);
	 RectFill(166,78,314,88,10);
	 grSetColor(11);PrintAt(203,85,"VISIT LOCATION");
	 
	 }

   // Selection Loops
   for (i=0;i<5;i++)
    {
     // Program Transfer
     if ((tag==0 && Data->P[plr].Manned[i].Num>=0) &&
	       ((mousebuttons>0 && x>=167 && y>=(95+21*i) && x<=236 && y<=(109+21*i))
	        || key==0x0030+i)) {
    	 InBox(167,95+21*i,236,109+21*i);
      if (key>0) delay(140);
	    WaitForMouseUp();key=0;
	    OutBox(167,95+21*i,236,109+21*i);
		 Data->P[plr].Pool[M[now2]].Assign=i+1;
		 Data->P[plr].Pool[M[now2]].Una=0;
		 Data->P[plr].Pool[M[now2]].Moved=0;
		 for (i=now2;i<count;i++) M[i]=M[i+1];
		 M[i]=-1;count--;
		 if (count==0) {RectFill(10,52,89,101,7+plr*3);Clear();} 
		 if (now2==count) {
		    if (now2>0) now2--;
		    if (BarA>0) BarA--;
	       };
      
		 RectFill(26,129,153,195,0);
		 ShBox(26,130+BarA*8,152,138+BarA*8);
		 DispLeft(plr,BarA,count,now2,&M[0]);
      
		 if (count>0) LimboText(plr,M[now2]);

	  }

      // Program Visit
      if ((tag==1 && Data->P[plr].Manned[i].Num>=0) &&
	 ((mousebuttons>0 && x>=167 && y>=(95+21*i) && x<=236 && y<=(109+21*i))
	 || key==0x0030+i)) {
	 InBox(167,95+21*i,236,109+21*i);
    if (key>0) delay(140);
	 WaitForMouseUp();key=0;
	 OutBox(167,95+21*i,236,109+21*i);
         music_stop();
	      Programs(plr,i+1);
	      DrawLimbo(plr);
				music_start((plr==0)?M_ASTTRNG:M_ASSEMBLY);
	 
	 InBox(241,48,313,65);OutBox(167,48,239,65);
	 RectFill(166,78,314,88,10);
	 grSetColor(11);PrintAt(203,85,"VISIT LOCATION");
	      RectFill(26,129,153,195,0);
	now2=BarA=count=0;
	 ShBox(26,130+BarA*8,152,138+BarA*8);
	 for (i=0;i<Data->P[plr].AstroCount;i++)
	    if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0)
	       M[count++]=i;
	 DispLeft(plr,BarA,count,now2,&M[0]);
	 if (count>0) LimboText(plr,M[now2]);
      //   ShBox(26,130+BarA*8,152,138+BarA*8);
      //   DispLeft(plr,BarA,count,now2,&M[0]);
      //   LimboText(plr,M[now2]);
	 FadeIn(2,pal,10,0,0);
	 
	 }


      // Training Transfer
      if ((tag==0 && count>0) && ((mousebuttons>0 && x>=244 && y>=(95+21*i) && x<=313 && y<=(109+21*i)) || key==0x0035+i)) {
	 InBox(244,95+21*i,313,109+21*i);
    if (key>0) delay(140);
	 WaitForMouseUp();key=0;
	 OutBox(244,95+21*i,313,109+21*i);
   skilLev=0;  // Figure out relevant skill level bfr sending to Adv Training -Leon
   if (i==0) skilLev=Data->P[plr].Pool[M[now2]].Cap;
   if (i==1) skilLev=Data->P[plr].Pool[M[now2]].LM;
   if (i==2) skilLev=Data->P[plr].Pool[M[now2]].EVA;
   if (i==3) skilLev=Data->P[plr].Pool[M[now2]].Docking;
   if (i==4) skilLev=Data->P[plr].Pool[M[now2]].Endurance;
   grSetColor(2);
   if (Data->P[plr].Pool[M[now2]].TrainingLevel>6) Help("i120");
   else if (skilLev > 3) OutBox(244,95+21*i,313,109+21*i);  // If they have a 4 in that skill, don't send to Adv Training for it
   else if (Data->P[plr].Cash<3) Help("i121");
   else {
		 if (skilLev>2)  // If they have a 3 in that skill, send them directly to Adv III and charge just 2MB
		  {
		   Data->P[plr].Pool[M[now2]].Status=AST_ST_TRAIN_ADV_3;
	   	   Data->P[plr].Cash-=2;
		  }
		 else
		  {
		   Data->P[plr].Pool[M[now2]].Status=AST_ST_TRAIN_ADV_1;
	   	   Data->P[plr].Cash-=3;
		  }
	   	 Data->P[plr].Pool[M[now2]].Focus=i+1;
		 Data->P[plr].Pool[M[now2]].Assign=0;
		 Data->P[plr].Pool[M[now2]].Una=0;
		 Data->P[plr].Pool[M[now2]].Moved=0;
		 for (i=now2;i<count;i++) M[i]=M[i+1];
		 M[i]=-1;count--;
		 if (count==0) {RectFill(10,52,89,101,7+plr*3);Clear();}   
		 if (now2==count) {
		    if (now2>0) now2--;
		    if (BarA>0) BarA--;
	       };
	      RectFill(26,129,153,195,0);
		 ShBox(26,130+BarA*8,152,138+BarA*8);
		 DispLeft(plr,BarA,count,now2,&M[0]);
		 if (count>0) LimboText(plr,M[now2]);
    } // else
	 }  // if adv training

      // Training Visit
      if (tag==1 &&
	 ((mousebuttons>0 && x>=244 && y>=(95+21*i) && x<=313 && y<=(109+21*i)) || key==0x0035+i)) {
	 InBox(244,95+21*i,313,109+21*i);
    if (key>0) delay(140);
	 WaitForMouseUp();key=0;
	 OutBox(244,95+21*i,313,109+21*i);
         music_stop();
	      Train(plr,i+1);
	      DrawLimbo(plr);
         music_start((plr==0)?M_ASTTRNG:M_ASSEMBLY);
	 
	 RectFill(166,78,314,88,10);
	 grSetColor(11);PrintAt(203,85,"VISIT LOCATION");
	 InBox(241,48,313,65);OutBox(167,48,239,65);
	      RectFill(26,129,153,195,0);
	now2=BarA=count=0;
	 ShBox(26,130+BarA*8,152,138+BarA*8);
	 for (i=0;i<Data->P[plr].AstroCount;i++)
	    if (Data->P[plr].Pool[i].Status==AST_ST_ACTIVE && Data->P[plr].Pool[i].Assign==0)
	       M[count++]=i;
	 DispLeft(plr,BarA,count,now2,&M[0]);
	 if (count>0) LimboText(plr,M[now2]);
	 FadeIn(2,pal,10,0,0);
	 
	 }
      }

  };  /* end while */
} /* end Limbo */

void Clear(void)
{
 RectFill(44,31,145,40,3);RectFill(49,112,60,119,3);
 RectFill(123,62,145,77,3);RectFill(125,79,145,85,3);RectFill(128,86,145,93,3);
 RectFill(123,95,145,101,3);RectFill(127,113,141,120,3);
 RectFill(130,54,155,61,3);     
 return;
}

void LimboText(char plr,int astro)
{
   RectFill(44,31,145,40,3);grSetColor(11);
    int col;
    if (Data->P[plr].Pool[astro].Mood>=65) col=16;
    if (Data->P[plr].Pool[astro].Mood<65 && Data->P[plr].Pool[astro].Mood>=40) col=11;
    if (Data->P[plr].Pool[astro].Mood<40 && Data->P[plr].Pool[astro].Mood>=20) col=8;
    if (Data->P[plr].Pool[astro].Mood<20) col=0;
    if (Data->P[plr].Pool[astro].Mood==0) col=3;
    grSetColor(col);  // Print name in green/yellow/red/black depending on mood -Leon
   if (Data->P[plr].Pool[astro].RDelay>0) grSetColor(0);  // Print name in black if 'naut has announced retirement (override mood) -Leon
   PrintAt(46,37,Data->P[plr].Pool[astro].Name);
   grSetColor(11);
   RectFill(49,112,80,119,3);grSetColor(11);
   switch (Data->P[plr].Pool[astro].Group) {
      case 0: PrintAt(53,118,"I");break;
      case 1: PrintAt(53,118,"II");break;
      case 2: PrintAt(53,118,"III");break;
      case 3: PrintAt(53,118,"IV");break;
      case 4: PrintAt(53,118,"V");break;
      }                           
   RectFill(123,62,145,77,3);
   RectFill(125,79,145,85,3);
   RectFill(131,86,145,92,3);
   RectFill(123,95,145,101,3);
   RectFill(130,54,155,61,3);
    grSetColor(col); // Print mood in green/yellow/red/black depending on mood -Leon
   DispNum(132,60,Data->P[plr].Pool[astro].Mood);
    grSetColor(11);
   DispNum(125,68,Data->P[plr].Pool[astro].Cap);
   DispNum(123,76,Data->P[plr].Pool[astro].LM); 
   DispNum(125,84,Data->P[plr].Pool[astro].EVA); 
   DispNum(131,92,Data->P[plr].Pool[astro].Docking); 
   DispNum(125,100,Data->P[plr].Pool[astro].Endurance); 
   RectFill(127,113,141,120,3);grSetColor(11);
   DispNum(130,118,Data->P[plr].Pool[astro].Active);
   AstFaces(plr,10,52,Data->P[plr].Pool[astro].Face);
   return;
}


// EOF
