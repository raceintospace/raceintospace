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
/** \file rdplex.c Research and Development Complex
 */

#include <Buzz_inc.h>
#include <externs.h>
#include <options.h>

int call,wh;
GXHEADER but,mans;
extern char HARD1,UNIT1,BUTLOAD;

/* We need to mask number of rolls into R&D history for current turn
 * Max number rolled on 5 dice with mods is 5*7 = 35
 * To keep binary compatibility we use 8 bits for value
 * 35 + 5*40 == 235 < 255, so fits within single byte and is unambiguous.
 */
enum {
	NUM_ROLLS_MULT = 40,
};

/**
 * Extract number of rolls from encoded value.
 * @param encoded
 * @return number of rolls
 */
static inline
uint8_t
decodeNumRolls(uint8_t encoded)
{
	return encoded / NUM_ROLLS_MULT;
}

/**
 * Extract sum of dice rolls from encoded value.
 * @param encoded
 * @return sum of dice rolls
 */
static inline
uint8_t
decodeRollValue(uint8_t encoded)
{
	return encoded % NUM_ROLLS_MULT;
}

/**
 * Encode number of dice and total rolled value.
 * @param nRolls nubmer of dice
 * @param value sum of rolled dice
 * @return encoded value
 */
static inline
uint8_t
encodeRolls(uint8_t nRolls, uint8_t value)
{
	return nRolls * NUM_ROLLS_MULT + value;
}

void SRPrintAt(int x, int y, char *text,char fgd,char bck)
{
   grSetColor(bck);PrintAt(x+1,y+1,text);
   grSetColor(fgd);PrintAt(x,y,text);
   return;
}


void Load_RD_BUT(char player_index)
{
  FILE *fin;
  struct {char na[4];uint32_t size;} Boo;
  if (BUTLOAD==1) return;
  GV(&but,282,61);
  GV(&mans,119,17);

  fin=sOpen("RDBOX.BUT","rb",0);
  fread(&Boo,sizeof Boo,1,fin);
	Swap32bit(Boo.size);

  if (player_index==1)
    while( strncmp("SBUT",&Boo.na[0],4)!=0) {
      fseek(fin,Boo.size,SEEK_CUR);
      fread(&Boo,sizeof Boo,1,fin);
			Swap32bit(Boo.size);
    }
  fread((char *)buffer,Boo.size,1,fin);
  RLED_img((char *)buffer,(char *)but.vptr,Boo.size,but.w,but.h);
  fread(&Boo,sizeof Boo,1,fin);
	Swap32bit(Boo.size);
  fread((char *)buffer,Boo.size,1,fin); fclose(fin);
  RLED_img((char *)buffer,mans.vptr,Boo.size,mans.w,mans.h);
  BUTLOAD=1;
  return;
}

void Del_RD_BUT(void)
{
  BUTLOAD=0;
  DV(&but);
  DV(&mans);
  return;
}

void DrawRD(char player_index)
{
  int i;
  FILE *fin;
  strcpy(IDT,"i009");strcpy(IKEY,"k009");

  FadeOut(2,pal,10,0,0);
  fin=sOpen("VAB.IMG","rb",0);
  fread(&pal[0],768,1,fin); fclose(fin);

  gxClearDisplay(0,0);
  Load_RD_BUT(player_index);
  ShBox(0,0,319,22);ShBox(0,24,319,65);

  ShBox(0,68,158,199); IOBox(3,71,154,85);
  IOBox(3,182,76,196);IOBox(81,182,154,196);

  ShBox(161,68,319,199);
  IOBox(163,155,187,177);IOBox(189,155,213,177);IOBox(215,155,239,177);
  IOBox(241,155,265,177);IOBox(267,155,291,177);IOBox(293,155,317,177);

  IOBox(5,27,77,62);IOBox(81,27,158,62);
  IOBox(162,27,239,62);IOBox(243,27,315,62);

  IOBox(243,3,316,19);
  InBox(3,3,30,19);
  InBox(165,157,185,175);
  IOBox(163,182,317,196);

  OutBox(21,90,136,176);InBox(26,94,131,172);

  LTArrow(24,186);RTArrow(101,186);

  //GradRect(27,95,130,171,player_index*16+128);
  RectFill(27,95,130,171,0);

  grSetColor(1);
  PrintAt(13,80,"VISIT PURCHASING FACILITY");

  for (i=0;i<6;i++)
    gxVirtualDisplay(&mans,i*20,0,166+i*26,158,184+i*26,174,0);

  grSetColor(3);grMoveTo(296,174);grLineTo(314,174);

  gxVirtualDisplay(&but,0,0,8,30,74,59,0);  // Unmanned
  gxVirtualDisplay(&but,68,0,84,30,155,59,0); // Rocket
  gxVirtualDisplay(&but,141,0,165,30,236,59,0);// Manned
  gxVirtualDisplay(&but,214,0,246,30,312,59,0);// Misc

  grSetColor(1);
  DispBig(50,5,"RESEARCH",0,-1);
  grSetColor(11);

  if (Data->Season==0) PrintAt(157,8,"SPRING");
  else PrintAt(162,8,"FALL");
  PrintAt(163,15,"19");
  DispNum(0,0,Data->Year);

  PrintAt(200,8,"CASH:");
  DispMB(201,15,Data->P[player_index].Cash);

  grSetColor(1);
  PrintAt(258,13,"CONTINUE");
  FlagSm(player_index,4,4);
  QueryUnit(1,1,player_index);
  ShowUnit(1,1,player_index);

  return;
} // End of DrawRD


void BButs(char old,char nw)
{

  switch(old) {
    case 1:OutBox(7,29,75,60);
	   gxVirtualDisplay(&but,0,0,8,30,74,59,0);  // Unmanned
	   break;
    case 2:OutBox(83,29,156,60);
	   gxVirtualDisplay(&but,68,0,84,30,155,59,0); // Rocket
	   break;
    case 3:OutBox(164,29,237,60);
	   gxVirtualDisplay(&but,141,0,165,30,236,59,0);// Manned
	   break;
    case 4:OutBox(245,29,313,60);
	   gxVirtualDisplay(&but,214,0,246,30,312,59,0);// Misc
	   break;
  }
  switch(nw) {
    case 1:InBox(7,29,75,60);
	   gxVirtualDisplay(&but,0,31,8,30,74,59,0);  // Unmanned
	   break;
    case 2:InBox(83,29,156,60);
	   gxVirtualDisplay(&but,68,31,84,30,155,59,0); // Rocket
	   break;
    case 3:InBox(164,29,237,60);
	   gxVirtualDisplay(&but,141,31,165,30,236,59,0);// Manned
	   break;
    case 4:InBox(245,29,313,60);
	   gxVirtualDisplay(&but,214,31,246,30,312,59,0);// Misc
	   break;
  }


  return;
}

void
RDButTxt(int cost, int encodedRolls, char playerIndex, char SpDModule) //DM Screen, Nikakd, 10/8/10
{
	RectFill(166, 185, 314, 193, 3);
	grSetColor(1);

	int diceRoll = decodeRollValue(encodedRolls);

 		//DM Screen, Nikakd, 10/8/10
	if (SpDModule==1) {
		PrintAt(184,191,"CANNOT BE RESEARCHED");
		return;
		}

	if (diceRoll == 0)
	{
		PrintAt(169, 191, "RESEARCH PROGRAM FOR ");
		grSetColor(9);
		DispNum(0, 0, cost);
		grSetColor(1);
		PrintAt(0, 0, " MB");
	}
	else
	{
		grSetColor(11);
		PrintAt(192, 191, "R&D ");
		DispNum(0, 0, diceRoll);
		PrintAt(0, 0, "% IMPROVEMENT");
		if (Data->P[playerIndex].RDMods > 0)
			PrintAt(0, 0, "+");
	}
}


char RD(char player_index)
{
  short hardware=1,roll=0,unit=1,buy[4][7],i,j,b;

  b = 0; /* XXX check uninitialized */

  for(i=0;i<4;i++) for(j=0;j<7;j++) buy[i][j]=Data->P[player_index].Buy[i][j];
  hardware=HARD1; unit=UNIT1;
  strcpy(IDT,"i009");strcpy(IKEY,"k009");

  DrawRD(player_index);
  BButs(1,hardware);
  ShowUnit(hardware,unit,player_index);
  RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
  if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
  else {InBox(165,184,315,194);};
  ManSel(decodeNumRolls(buy[hardware-1][unit-1]));
  strcpy(IDT,"i009");strcpy(IKEY,"k009");
  FadeIn(2,pal,10,0,0);
  music_start(M_HARDWARE);
  WaitForMouseUp();
  while (1)
  {
    key=0;GetMouse();
    if (mousebuttons>0 || key>0)
     {if ((x>=283 && y>=90 && x<=302 && y<=100) || key=='F')
	{
	char EqDmg=0;
	switch (hardware-1)
		{
		case 0: EqDmg=Data->P[player_index].Probe[unit-1].Damage!=0?1:0; break;
		case 1: EqDmg=Data->P[player_index].Rocket[unit-1].Damage!=0?1:0; break;
		case 2: EqDmg=Data->P[player_index].Manned[unit-1].Damage!=0?1:0; break;
		case 3: EqDmg=Data->P[player_index].Misc[unit-1].Damage!=0?1:0; break;
		default: break; }
	if (EqDmg) {	InBox(283,90,302,100);
			DamProb(player_index,hardware-1,unit-1);
			DrawRD(player_index);
			BButs(1,hardware);
  			ShowUnit(hardware,unit,player_index);
  			RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
  			if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
  			else {InBox(165,184,315,194);};
  			ManSel(decodeNumRolls(buy[hardware-1][unit-1]));
  			strcpy(IDT,"i009");strcpy(IKEY,"k009");
  			FadeIn(2,pal,10,0,0);
		}
	}
      else
      if ((y>=29 && y<=60 && mousebuttons>0) || (key=='U' || key=='R' || key=='M' || key=='C'))
       {
	    if (((x>=7 && x<=75 && mousebuttons>0) || key=='U') && hardware != 1)  /* Unmanned */
        {
         roll=0;
	      BButs(hardware,1);
	      hardware=1;unit=1;
	      if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	       else {InBox(165,184,315,194);};
	      ManSel(decodeNumRolls(buy[hardware-1][unit-1]));ShowUnit(hardware,unit,player_index);
	      b=Data->P[player_index].Probe[unit-1].RDCost;
	      RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	     }
      else
	   if (((x>=83 && x<=156 && mousebuttons>0) || key=='R') && hardware != 2)  /* Rockets */
	    {
        roll=0;
	     BButs(hardware,2);
	     hardware=2;unit=1;
	     if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	      else {InBox(165,184,315,194);};
	     ManSel(decodeNumRolls(buy[hardware-1][unit-1]));ShowUnit(hardware,unit,player_index);
	     b=Data->P[player_index].Rocket[unit-1].RDCost;
	     RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	    }
      else
	   if (((x>=164 && x<=237 && mousebuttons>0) || key=='C') && hardware != 3)    /* Manned */
	    {
        roll=0;
	     BButs(hardware,3);
	     hardware=3;unit=1;
	     if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	      else {InBox(165,184,315,194);};
	     ManSel(decodeNumRolls(buy[hardware-1][unit-1]));ShowUnit(hardware,unit,player_index);
	     b=Data->P[player_index].Manned[unit-1].RDCost;
	     RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	    }
      else
	   if (((x>=245 && x<=313 && mousebuttons>0) || key=='M') && hardware !=4)     /* Misc */
	     {
         roll=0;
	      BButs(hardware,4);
	      hardware=4;unit=1;
	      if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	        else {InBox(165,184,315,194);};
	      ManSel(decodeNumRolls(buy[hardware-1][unit-1]));ShowUnit(hardware,unit,player_index);
	      b=Data->P[player_index].Misc[unit-1].RDCost;
	      RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	     }
      }
      else
      if (((y>=157 && y<=175 && mousebuttons>0) || (key>='0'&& key<='5')) && buy[hardware-1][unit-1]==0)
      {         /*  R&D Amount */
       if (((x>=165 && x<=185 && mousebuttons>0) || key=='0') && roll!=0) roll=0;
       if (((x>=191 && x<=211 && mousebuttons>0) || key=='1') && roll!=1) roll=1;
       if (((x>=217 && x<=238 && mousebuttons>0) || key=='2') && roll!=2) roll=2;
       if (((x>=243 && x<=263 && mousebuttons>0) || key=='3') && roll!=3) roll=3;
       if (((x>=269 && x<=289 && mousebuttons>0) || key=='4') && roll!=4) roll=4;
       if (((x>=295 && x<=315 && mousebuttons>0) || key=='5') && roll!=5) roll=5;
       if (hardware==1) b=Data->P[player_index].Probe[unit-1].RDCost;
       if (hardware==2) b=Data->P[player_index].Rocket[unit-1].RDCost;
       if (hardware==3) b=Data->P[player_index].Manned[unit-1].RDCost;
       if (hardware==4) b=Data->P[player_index].Misc[unit-1].RDCost;
       RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
       ManSel(roll);
       WaitForMouseUp();
      }
      else
      if ((x>=5 && y>=184 && x<=74 && y<=194 && mousebuttons>0) || key==LT_ARROW)  /* LEFT ARROW */
      {
	    roll=0;
	    InBox(5,184,74,194);
	    WaitForMouseUp();
	    switch(hardware)
	     {
	      case 1: unit= (unit-1 == 0) ? 3 : unit-1;break;
	      case 2: unit= (unit-1 == 0) ? 5 : unit-1;break;
	      case 3: unit= (unit-1 == 0) ? 7 : unit-1;break;
	      case 4: unit= (unit-1 == 0) ? 5 : unit-1;if (player_index==0 && unit==3) unit--;break;  //DM Screen, Nikakd, 10/8/10
         default:break;
	     };
	    RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	    ManSel(decodeNumRolls(buy[hardware-1][unit-1]));
	    if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	      else {InBox(165,184,315,194);};
	    ShowUnit(hardware,unit,player_index);
	    OutBox(5,184,74,194);
      }
      else
      if ((x>=83 && y>=184 && x<=152 && y<=194 && mousebuttons>0) || key==RT_ARROW) /* RIGHT ARROW */
      {
	    roll=0;
	    InBox(83,184,152,194);
	    WaitForMouseUp();
	    switch(hardware)
	     {
	      case 1: unit = (unit+1 > 3) ? 1 : unit+1;break;
	      case 2: unit = (unit+1 > 5) ? 1 : unit+1;break;
	      case 3: unit = (unit+1 > 7) ? 1 : unit+1;break;
	      case 4: unit = (unit+1 > 5) ? 1 : unit+1;if (player_index==0 && unit==3) unit++;break;   //DM Screen, Nikakd, 10/8/10
         default:break;
	     };
	    RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	    ManSel (decodeNumRolls(buy[hardware-1][unit-1]));
	    if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	      else {InBox(165,184,315,194);};
	    ShowUnit(hardware,unit,player_index);
	    RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	    OutBox(83,184,152,194);
      }
      else
      if (((x>=165 && y>=184 && x<=315 && y<=194 && mousebuttons>0) || key=='S') && buy[hardware-1][unit-1]==0
		 && roll !=0)
	  {           // b is the cost per roll
	if (hardware==1)
	 b=Data->P[player_index].Probe[unit-1].RDCost;
	if (hardware==2)
	 b=Data->P[player_index].Rocket[unit-1].RDCost;
	if (hardware==3)
	 b=Data->P[player_index].Manned[unit-1].RDCost;
	if (hardware==4)
	 b=Data->P[player_index].Misc[unit-1].RDCost;
	// Add to the expenditure data

	if ( (b*roll <= Data->P[player_index].Cash) && QueryUnit(hardware,unit,player_index)
	     && MaxChk(hardware,unit,player_index))
	{
	  buy[hardware-1][unit-1]=RDUnit(hardware,unit,roll,player_index);
	  if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
	  else {InBox(165,184,315,194);};
	  Data->P[player_index].Cash-=b*roll;
	  // add the amount to the expenditure budget
	  switch(hardware) {
	    case 1: Data->P[player_index].Spend[0][0] += b*roll;break;
	    case 2: Data->P[player_index].Spend[0][1] += b*roll;break;
	    case 3: Data->P[player_index].Spend[0][2] += b*roll;break;
	    case 4: Data->P[player_index].Spend[0][3] += b*roll;break;
	  }
	  ShowUnit(hardware,unit,player_index);
	  RDButTxt(b*roll,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
	}
	else QueryUnit(hardware,unit,player_index);
      }
     else
      if (((y>=3 && y<=19) && (x>=243 && x<=316 && mousebuttons>0)) || key==K_ENTER)
      {
	    InBox(245,5,314,17);
   	 WaitForMouseUp();
	    for(i=0;i<4;i++) for(j=0;j<7;j++) Data->P[player_index].Buy[i][j]=buy[i][j];
       music_stop();
	    Del_RD_BUT();
	    call=0;
	    HARD1=1;UNIT1=1;
	    return 0;
      }
      else
      if ((x>=5 && y>=73 && x<=152 && y<=83 && mousebuttons>0) || key=='V')
      {
	    InBox(5,73,152,83);
	    HARD1=hardware;UNIT1=unit;
	    for(i=0;i<4;i++) for(j=0;j<7;j++)
         Data->P[player_index].Buy[i][j]=buy[i][j];
       music_stop();
      //DM Screen, Nikakd, 10/8/10 (Removed line)
	    if (call==1) return 1;        // go back through gateway
	    call=1;
	    wh=HPurc(player_index);
	    if (call==0) return 0;
	    hardware=HARD1;unit=UNIT1;
	    call=0;
	    for(i=0;i<4;i++) for(j=0;j<7;j++) buy[i][j]=Data->P[player_index].Buy[i][j];
	    DrawRD(player_index);
      //DM Screen, Nikakd, 10/8/10 (Removed line)
	    BButs(1,hardware);
	    ShowUnit(hardware,unit,player_index);
	    RDButTxt(0,buy[hardware-1][unit-1],player_index,((hardware==4&&unit==5)?1:0));  //DM Screen, Nikakd, 10/8/10
       if (buy[hardware-1][unit-1]==0) QueryUnit(hardware,unit,player_index);
       else {InBox(165,184,315,194);};

	    FadeIn(2,pal,10,0,0);
			music_start(M_HARDWARE);
	    WaitForMouseUp();
      };
    };
    gr_maybe_sync();
  };
}

/**
 * Draw proper outlines on active/inactive research team buttons.
 *
 * @param activeButtonIndex
 */
void ManSel(int activeButtonIndex)
{
    int dx = 26;
    int i;

    for (i = 0; i < 6; ++i)
    {
        if (i == activeButtonIndex)
            InBox(165+i*dx, 157, 185+i*dx, 175);
        else
            OutBox(165+i*dx, 157, 185+i*dx, 175);
    }

    av_need_update_xy(165, 157, 165+i*dx, 175);
}

char QueryUnit(char hardware_index,char unit_index,char player_index)
{
  char enableButton = 0;

  enableButton =
      ((hardware_index==1 && (Data->P[player_index].Probe[unit_index-1].Num != PROGRAM_NOT_STARTED)) ||
      (hardware_index==2 && (Data->P[player_index].Rocket[unit_index-1].Num != PROGRAM_NOT_STARTED)) ||
      (hardware_index==3 && (Data->P[player_index].Manned[unit_index-1].Num != PROGRAM_NOT_STARTED)) ||
      (hardware_index==4 && (unit_index!=5 && Data->P[player_index].Misc[unit_index-1].Num != PROGRAM_NOT_STARTED)));   //DM Screen, Nikakd, 10/8/10

  if (IsHumanPlayer(player_index))
  {
    if (enableButton)
      OutBox(165,184,315,194);
    else
      InBox(165,184,315,194);
  }

  return(enableButton);
}


char MaxChk(char hardware_index,char unit_index,char player_index)
{
  if (hardware_index==1) return(Data->P[player_index].Probe[unit_index-1].Safety < Data->P[player_index].Probe[unit_index-1].MaxRD);
  if (hardware_index==2) return(Data->P[player_index].Rocket[unit_index-1].Safety < Data->P[player_index].Rocket[unit_index-1].MaxRD);
  if (hardware_index==3) return(Data->P[player_index].Manned[unit_index-1].Safety < Data->P[player_index].Manned[unit_index-1].MaxRD);
  if (hardware_index==4) return(Data->P[player_index].Misc[unit_index-1].Safety < Data->P[player_index].Misc[unit_index-1].MaxRD);
  return(0);
}

/**
 * Roll dice and improve R&D value of a given hardware.
 *
 * HACK: need to store number of dices rolled in return value
 *
 * @param[in] hardwareTypeIndex
 * @param[in] hardwareIndex
 * @param[in] nRolls
 * @param[in] playerIndex
 * @return encoded number of dice and sum of rolls
 */
uint8_t
RDUnit(char hardwareTypeIndex, char hardwareIndex, char nRolls, char playerIndex)
{
	int diceRoll = 0;
	int i;
	struct BuzzData* p = &Data->P[playerIndex];
	Equipment* eqArr[4] = {p->Probe, p->Rocket, p->Manned, p->Misc};
	Equipment* eq;

	assert (hardwareTypeIndex >= 1);
	assert (hardwareTypeIndex <= 4);
	assert (hardwareIndex >= 1);
	assert (hardwareIndex <= 8);

	eq = &eqArr[hardwareTypeIndex - 1][hardwareIndex - 1];

	diceRoll = 0;
	for (i = 0; i < nRolls; i++)
		diceRoll += rand() % (6 + p->RDMods) + 1;

	eq->Safety += diceRoll;
	if (eq->Safety > eq->MaxRD)
		eq->Safety = eq->MaxRD;

	return encodeRolls(nRolls, diceRoll);
}

void ShowUnit(char hw,char un,char player_index)
{
  Equipment *PL;
  char qty=1,SCol=0;
  int Unit_Cost,Init_Cost;

  PL = NULL; /* XXX check uninitialized */

  switch(hw) {
   case 1: PL=(Equipment *)&Data->P[player_index].Probe[un-1].Name[0];break;
   case 2: PL=(Equipment *)&Data->P[player_index].Rocket[un-1].Name[0];break;
   case 3: PL=(Equipment *)&Data->P[player_index].Manned[un-1].Name[0];break;
   case 4: PL=(Equipment *)&Data->P[player_index].Misc[un-1].Name[0];break;
   }

  grSetColor(1);

  RectFill(162,69,318,154,3);RectFill(200,9,238,21,3);
  grSetColor(1);
  PrintAt(170,97,"INITIAL COST:");
  PrintAt(170,104,"UNIT COST:");
  PrintAt(170,118,"R&D COST PER TEAM:");
  PrintAt(170,125,"UNIT WEIGHT:");
  PrintAt(170,132,"MAXIMUM PAYLOAD:");
  PrintAt(170,146,"MAXIMUM SAFETY:");
  grSetColor(20);PrintAt(170,139,"MAXIMUM R&D:");

	char EqDmg=0;
	switch (hw-1)
	{
	case 0: EqDmg=Data->P[player_index].Probe[un-1].Damage!=0?1:0; break;
	case 1: EqDmg=Data->P[player_index].Rocket[un-1].Damage!=0?1:0; break;
	case 2: EqDmg=Data->P[player_index].Manned[un-1].Damage!=0?1:0; break;
	case 3: EqDmg=Data->P[player_index].Misc[un-1].Damage!=0?1:0; break;
	default: break; }
	if (EqDmg!=0) {IOBox(281,88,304,102); grSetColor(8); PrintAt(285,97,"F");grSetColor(11); PrintAt(0,0,"IX");}

  SCol=(PL->Num<0) ? 20 : ((PL->Safety<75) ? 8 : 16);
  grSetColor(SCol);PrintAt(170,111,"SAFETY FACTOR:");

  grSetColor(11);
  DispMB(201,15,Data->P[player_index].Cash);

  grSetColor(11);
  if (!(player_index==1 && hw==2 && un==5)) PrintAt(170,80,&PL->Name[0]);
  switch (hw) {
   case 1: switch(un-1){
	 case 0: case 1: PrintAt(0,0," SATELLITE");break;
	 case 2: PrintAt(0,0," PROBE");break;
	 default: break;
	 };break;
   case 2: switch(un-1) {
	 case 0: case 1: case 2: case 3:PrintAt(0,0," ROCKET");break;
	 case 4:
	   if (player_index==0) PrintAt(0,0," STRAP-ON");
	   else PrintAt(170,80,"BOOSTER STAGE");
	   break;
	 default: break;
	 };break;
   case 3: switch(un-1) {
	 case 0: case 1: case 2: PrintAt(0,0," CAPSULE");break;
	 case 3: PrintAt(0,0," MINISHUTTLE");break;
	 case 4: PrintAt(0,0," SPACECRAFT");break;
	 case 5: case 6: PrintAt(0,0," MODULE");break;
	 default: break;
	};break;
   case 4:
      if ((un-1)<3) PrintAt(0,0," BOOSTER");
      if ((un-1)==4) PrintAt(0,0," MODULE");
      break;
   }
   Init_Cost=PL->InitCost;
   Unit_Cost=PL->UnitCost;
   if (Data->P[player_index].TurnOnly==3)
    {
     Init_Cost/=2;Init_Cost=maxx(1,Init_Cost);
     if (Unit_Cost>1) Unit_Cost/=2;
    };
    DispNum(241,97,Init_Cost);
    DispNum(230,104,Unit_Cost);
    DispNum(275,118,PL->RDCost);

    if (hw!=2) DispNum(240,125,PL->UnitWeight);
    else PrintAt(240,125,"N/A");

    grSetColor(20);
    if (PL->MaxRD!=0)	{
    	DispNum(242,139,PL->MaxRD);DispChr('%');
    	if (options.want_debug)	{
    		PrintAt(0,0," / "); DispNum(0,0,PL->MSF); DispChr('%'); //Used to test if MSF was holding the right value
    		}
    	}
    else PrintAt(242,139, "--");
    grSetColor(11);DispNum(254,146,PL->MaxSafety);DispChr('%');

    if (hw!=2) PrintAt(268,132,"N/A"); /* Payload */
    else DispNum(268,132,PL->MaxPay);

    if (PL->Num==-1) {
      grSetColor(8);
      PrintAt(170,90,"NO PROGRAM INITIATED");
      grSetColor(20);
      DispNum(256,111,0);DispChr('%');
    }
    else {
      OnHand(PL->Num);
      grSetColor(SCol);
      DispNum(256,111,PL->Safety);DispChr('%');
	//Display Damaged Equipment
       if (PL->Damage!=0) {
		grSetColor(8);
		PrintAt(280,111,"(");DispNum(283,111,PL->Safety+PL->Damage<1?1:PL->Safety+PL->Damage);DispChr('%');DispChr(')');
		}
      qty=0;
    };
  RectFill(27,95,130,171,0);
  BigHardMe(player_index,27,95,hw-1,un-1,qty,32);

  return;
}

void OnHand(char qty)
{
  if (qty>0) grSetColor(16);
  else grSetColor(9);
  PrintAt(170,90,"UNITS ON HAND:");
  DispNum(251,90,qty);
  return;
}

void DrawHPurc(char player_index)
{
  FILE *fin;

  FadeOut(2,pal,10,0,0);
  fin=sOpen("VAB.IMG","rb",0);
  fread(&pal[0],768,1,fin);
  fclose(fin);

  Load_RD_BUT(player_index);
  gxClearDisplay(0,0);
  ShBox(0,0,319,22);  ShBox(0,24,319,65);
  ShBox(17,68,143,199);
  ShBox(21,71,139,87); ShBox(161,68,319,199);

  ShBox(0,68,158,199); IOBox(3,71,154,85);
  IOBox(3,182,76,196);IOBox(81,182,154,196);

  IOBox(5,27,77,62);IOBox(81,27,158,62);
  IOBox(162,27,239,62);IOBox(243,27,315,62);
  IOBox(163,180,317,197);
  IOBox(243,3,316,19);
  InBox(3,3,30,19);
  FlagSm(player_index,4,4);

  IOBox(264,162,316,176);
  grSetColor(11);PrintAt(280,171,"UNDO");

  OutBox(21,90,136,176);InBox(26,94,131,172);

  grSetColor(1);
  DispBig(35,5,"PURCHASING",0,-1);
  //GradRect(27,95,130,171,player_index*16+128);
  RectFill(27,95,130,171,0);

  LTArrow(24,186);RTArrow(101,186);
  grSetColor(1);
  PrintAt(34,80,"VISIT R&D FACILITY");

  gxVirtualDisplay(&but,0,0,8,30,74,59,0);  // Unmanned
  gxVirtualDisplay(&but,68,0,84,30,155,59,0); // Rocket
  gxVirtualDisplay(&but,141,0,165,30,236,59,0);// Manned
  gxVirtualDisplay(&but,214,0,246,30,312,59,0);// Misc

  grSetColor(11);
  PrintAt(191,190,"PURCHASE EQUIPMENT");

  if (Data->Season==0) PrintAt(158,8,"SPRING");
  else PrintAt(162,8,"FALL");
  PrintAt(163,15,"19");
  DispNum(0,0,Data->Year);

  PrintAt(200,8,"CASH:");
  DispMB(201,15,Data->P[player_index].Cash);

  grSetColor(1);PrintAt(258,13,"CONTINUE");
  ShowUnit(1,1,player_index);
  strcpy(IDT,"i008");strcpy(IKEY,"k008");

  return;
}

char HPurc(char player_index)
{
  short hardware,unit;
  FILE *undo;

  remove_savedat("UNDO.TMP");
  undo=sOpen("UNDO.TMP","wb",1);
  fwrite(Data,sizeof (struct Players),1,undo);
  fclose(undo);

  hardware=HARD1; unit=UNIT1;
  strcpy(IDT,"i008");strcpy(IKEY,"k008");
  DrawHPurc(player_index);
  BButs(1,hardware);
  ShowUnit(hardware,unit,player_index);

  //Specs: undo fix
  //memset(vhptr.vptr,0x00,64000);
  //memcpy(vhptr.vptr,Data,sizeof(struct Players));

  FadeIn(2,pal,10,0,0);
  music_start(M_FILLER);
  WaitForMouseUp();
  while (1)
  {
    key=0;GetMouse();
	if ((x>=283 && y>=90 && x<=302 && y<=100 && mousebuttons>0) || key=='F')
	{
	char EqDmg=0;
	switch (hardware-1)
		{
		case 0: EqDmg=Data->P[player_index].Probe[unit-1].Damage!=0?1:0; break;
		case 1: EqDmg=Data->P[player_index].Rocket[unit-1].Damage!=0?1:0; break;
		case 2: EqDmg=Data->P[player_index].Manned[unit-1].Damage!=0?1:0; break;
		case 3: EqDmg=Data->P[player_index].Misc[unit-1].Damage!=0?1:0; break;
		default: break; }
	if (EqDmg) {	InBox(283,90,302,100);
			DamProb(player_index,hardware-1,unit-1);
			strcpy(IDT,"i008");strcpy(IKEY,"k008");
  			DrawHPurc(player_index);
 			BButs(1,hardware);
  			ShowUnit(hardware,unit,player_index);

			FadeIn(2,pal,10,0,0);
			music_start(M_FILLER);
 			WaitForMouseUp();
		}
	}
    else if ((x>266 && y>164 && x<314 && y<174 && mousebuttons>0) || key=='Z') {
      InBox(266,164,314,174);
			WaitForMouseUp();
  //    memcpy(Data,vhptr.vptr,sizeof(struct Players));
      undo=sOpen("UNDO.TMP","rb",1);
      fread(Data,sizeof (struct Players),1,undo);
      fclose(undo);
      ShowUnit(hardware,unit,player_index);
      OutBox(266,164,314,174);
      key=0;
    }
     if ((y>=29 && y<=60 && mousebuttons>0) || (key=='U' || key=='R' || key=='M' || key=='C'))
      {
	    if (((x>=7 && x<=75 && mousebuttons>0) || key=='U') && hardware != 1)   /* PROBES */
	     {
	      BButs(hardware,1);
	      hardware=1;unit=1;
	      ShowUnit(hardware,unit,player_index);
	     }
      else
	   if (((x>=83 && x<=156 && mousebuttons>0) || key=='R') && hardware != 2)   /* ROCKETS  */
	    {
	     BButs(hardware,2);
	     hardware=2;unit=1;
	     ShowUnit(hardware,unit,player_index);
	    }
      else
	   if (((x>=164 && x<=237 && mousebuttons>0) || key=='C') && hardware != 3) /* MODULES-CAPSULES */
	    {
	     BButs(hardware,3);
	     hardware=3;unit=1;
	     ShowUnit(hardware,unit,player_index);
	    }
      else
	   if (((x>=245 && x<=313 && mousebuttons>0) || key=='M') && hardware !=4)   /* MISC */
	    {
	     BButs(hardware,4);
	     hardware=4;unit=1;
	     ShowUnit(hardware,unit,player_index);
	    }
     }
     else
     if ((x>=5 && y>=184 && x<=74 && y<=194 && mousebuttons>0) || key==LT_ARROW) /* LEFT ARROW */
      {
	    InBox(5,184,74,194);
	    delay(5);
	    WaitForMouseUp();
	    OutBox(5,184,74,194);
	    switch(hardware) {
	     case 1: unit= (unit-1 == 0) ? 3 : unit-1;break;
	     case 2: unit= (unit-1 == 0) ? 5 : unit-1;break;
	     case 3: unit= (unit-1 == 0) ? 7 : unit-1;break;
	     case 4: unit= (unit-1 == 0) ? 5 : unit-1;if (player_index==0 && unit==3) unit--;break;
        default:break;
	    };
	    ShowUnit(hardware,unit,player_index);
      }
      else
      if ((x>=83 && y>=184 && x<=152 && y<=194 && mousebuttons>0) || key==RT_ARROW) /* RIGHT ARROW */
      {
   	 InBox(83,184,152,194);
	    WaitForMouseUp();
   	 OutBox(83,184,152,194);
    	 switch(hardware) {
    	   case 1: unit = (unit+1 > 3 ) ? 1 : unit+1;break;
	      case 2: unit = (unit+1 > 5 ) ? 1 : unit+1;break;
    	   case 3: unit = (unit+1 > 7 ) ? 1 : unit+1;break;
	      case 4: unit = (unit+1 > 5 ) ? 1 : unit+1;if (player_index==0 && unit==3) unit++;break;
         default:break;
   	 };
	    ShowUnit(hardware,unit,player_index);
      }
      else
      if ((y>=182 && y<=195 && x>=166 && x<=314 && mousebuttons>0) || key=='P')  /* PURCHASE */
       {
    	   InBox(165,182,315,195);
   	   WaitForMouseUp();
   	   OutBox(165,182,315,195);
        // NEED DELAY CHECK
        switch(hardware)
         {
          case 1:if (Data->P[player_index].Probe[unit-1].Delay==0) BuyUnit(hardware,unit,player_index);
                  else Help("i135");
                break;
          case 2:if (Data->P[player_index].Rocket[unit-1].Delay==0) BuyUnit(hardware,unit,player_index);
                  else Help("i135");
                break;
          case 3:if (Data->P[player_index].Manned[unit-1].Delay==0) BuyUnit(hardware,unit,player_index);
                  else Help("i135");
                break;
          case 4:if (Data->P[player_index].Misc[unit-1].Delay==0) BuyUnit(hardware,unit,player_index);
                  else Help("i135");
                break;
         default:break;
        }
      }
      else
      if (((y>=3 && y<=19) && (x>=243 && x<=316 && mousebuttons>0)) || key==K_ENTER)
       {
     	  InBox(245,5,314,17);
				WaitForMouseUp();
        music_stop();
        Del_RD_BUT(); call=0;
        HARD1=1;UNIT1=1;
        remove_savedat("UNDO.TMP");
   	  return 0;   // Continue
       }
      else
      if ((x>=5 && y>=73 && x<=152 && y<=83 && mousebuttons>0) || key=='V')         // Gateway to RD
       {
    	 InBox(5,73,152,83);
			 WaitForMouseUp();
    	 HARD1=hardware; UNIT1=unit;
       music_stop();
       remove_savedat("UNDO.TMP");
      //DM Screen, Nikakd, 10/8/10 (Removed line)
    	 if (call==1) return 1;
   	 call=1;
   	 wh=RD(player_index);
   	 if (call==0) return 0;   // Exit
   	 call=0;
   	 hardware=HARD1; unit=UNIT1;
       //DM Screen, Nikakd, 10/8/10 (Removed line)
   	 DrawHPurc(player_index);
   //    memcpy(vhptr.vptr,Data,sizeof(struct Players));
   	 ShowUnit(hardware,unit,player_index);
   	 BButs(1,hardware);

     // Just Added stuff by mike
     undo=sOpen("UNDO.TMP","wb",1);
     fwrite(Data,sizeof (struct Players),1,undo);
     fclose(undo);

   	 FadeIn(2,pal,10,0,0);
       music_start(M_FILLER);
   	 WaitForMouseUp();
      };
  };
}

/**
 * Record purchase of a hardware unit.
 * @param category index of hardware category
 * @param unit index of specific unit in the category
 * @param player_index player doing the purchase
 */

void
BuyUnit(char category, char unit, char player_index)
{
	short n1, n2, n3, n4, n5, n6, n7;
	char new_program = 0;
	int Init_Cost, Unit_Cost;
	Equipment* unit_ptr = NULL;
	Unit_Cost = Init_Cost = 0;

	assert(1 <= category && category <= 4);

	switch (category)
	{
		case 1:
			assert(1 <= unit && unit <= 3);
			unit_ptr = &(Data->P[player_index].Probe[unit - 1]);
			break;
		case 2:
			assert(1 <= unit && unit <= 5);
			unit_ptr = &(Data->P[player_index].Rocket[unit - 1]);
			break;
		case 3:
			assert(1 <= unit && unit <= 7);
			unit_ptr = &(Data->P[player_index].Manned[unit - 1]);
			break;
		case 4:
			assert(1 <= unit && unit <= 5);
			unit_ptr = &(Data->P[player_index].Misc[unit - 1]);
			break;
	}

	Init_Cost = unit_ptr->InitCost;
	Unit_Cost = unit_ptr->UnitCost;

	/* this is the "half-off" sale */
	if (Data->P[player_index].TurnOnly == 3)
	{
		/* make sure changes do not affect items that are already free */
		if (Init_Cost)
			Init_Cost = maxx(1, Init_Cost/2);
		if (Unit_Cost)
			Unit_Cost = maxx(1, Unit_Cost/2);
	}

	if (unit_ptr->Num == -1 && Data->P[player_index].Cash >= Init_Cost)
	{
		Data->P[player_index].Cash -= Init_Cost;
		unit_ptr->Num = 1;
		new_program = 1;
		Data->P[player_index].Spend[0][category - 1] += Init_Cost;
	}
	else if (unit_ptr->Num >= 0 && unit_ptr->Num < 6
			&& Data->P[player_index].Cash >= Unit_Cost)
	{
		Data->P[player_index].Cash -= Unit_Cost;
		unit_ptr->Num += 1;
		Data->P[player_index].Spend[0][category - 1] += Unit_Cost;
	}

	/* compute technology transfer for Probe category */
	if (new_program && category == 1)
	{
		n1 = Data->P[player_index].Probe[0].Safety;
		n2 = Data->P[player_index].Probe[1].Safety;
		n3 = Data->P[player_index].Probe[2].Safety;
		switch (unit - 1)
		{
			case 0:
				if (n2 >= 75)
					Data->P[player_index].Probe[0].Safety = 50;
				if (n3 >= 75)
					Data->P[player_index].Probe[0].Safety = 60;
				break;
			case 1:
				if (n1 >= 75)
					Data->P[player_index].Probe[1].Safety = 45;
				if (n3 >= 75)
					Data->P[player_index].Probe[1].Safety = 50;
				break;
			case 2:
				if (n1 >= 75)
					Data->P[player_index].Probe[2].Safety = 45;
				if (n2 >= 75)
					Data->P[player_index].Probe[2].Safety = 50;
				break;
		}
	}

	/* compute technology transfer for Rocket category */
	if (new_program && category == 2)
	{
		n1 = Data->P[player_index].Rocket[0].Safety; /* One - A	   */
		n2 = Data->P[player_index].Rocket[1].Safety; /* Two - B	   */
		n3 = Data->P[player_index].Rocket[2].Safety; /* Three - C   */
		n4 = Data->P[player_index].Rocket[3].Safety; /* Mega - G    */
		n5 = Data->P[player_index].Rocket[4].Safety; /* Booster - D */
		switch (unit - 1)
		{
			case 0:
				if (n2 >= 75 || n3 >= 75 || n4 >= 75 || n5 >= 75)
					Data->P[player_index].Rocket[0].Safety = 35;
				break;
			case 1:
				if (n1 >= 75 || n5 >= 75)
					Data->P[player_index].Rocket[1].Safety = 25;
				if (n3 >= 75 || n4 >= 75)
					Data->P[player_index].Rocket[1].Safety = 40;
				if ((n1 >= 75 || n5 >= 75) && (n3 >= 75 || n4 >= 75))
					Data->P[player_index].Rocket[1].Safety = 65;
				break;
			case 2:
				if (n1 >= 75 || n5 >= 75)
					Data->P[player_index].Rocket[2].Safety = 15;
				if (n2 >= 75 || n4 >= 75)
					Data->P[player_index].Rocket[2].Safety = 35;
				if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75))
					Data->P[player_index].Rocket[2].Safety = 60;
				break;
			case 3:
				if (n1 >= 75 || n5 >= 75)
					Data->P[player_index].Rocket[3].Safety = 10;
				if (n2 >= 75 || n3 >= 75)
					Data->P[player_index].Rocket[3].Safety = 25;
				if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n3 >= 75))
					Data->P[player_index].Rocket[3].Safety = 35;
				break;
			case 4:
				if (n1 >= 75 || n2 >= 75 || n3 >= 75 || n4 >= 75)
					Data->P[player_index].Rocket[4].Safety = 30;
				break;
		};
	};

	/* compute technology transfer for Manned category */
	if (new_program && category == 3)
	{
		n1 = Data->P[player_index].Manned[0].Safety; /* One - a		   */
		n2 = Data->P[player_index].Manned[1].Safety; /* Two - b		   */
		n3 = Data->P[player_index].Manned[2].Safety; /* Three - c	   */
		n4 = Data->P[player_index].Manned[3].Safety; /* Minishuttle - f */
		n5 = Data->P[player_index].Manned[4].Safety; /* cap/mod - h	   */
		n6 = Data->P[player_index].Manned[5].Safety; /* 2 mod - d	   */
		n7 = Data->P[player_index].Manned[6].Safety; /* 1 mod - e	   */
		switch (unit - 1)
		{
			case 0:
				if (n2 >= 75 || n3 >= 75 || n5 >= 75)
					Data->P[player_index].Manned[0].Safety = 40;
				break;
			case 1:
				if (n1 >= 75)
					Data->P[player_index].Manned[1].Safety = 20;
				if (n3 >= 75 || n5 >= 75)
					Data->P[player_index].Manned[1].Safety = 40;
				break;
			case 2:
				if (n1 >= 75 || n5 >= 75)
					Data->P[player_index].Manned[2].Safety = 20;
				if (n2 >= 75 || n4 >= 75)
					Data->P[player_index].Manned[2].Safety = 30;
				if ((n1 >= 75 || n5 >= 75) && (n2 >= 75 || n4 >= 75))
					Data->P[player_index].Manned[2].Safety = 40;
				break;
			case 3:
				break;
			case 4:
				if (n1 >= 75)
					Data->P[player_index].Manned[4].Safety = 10;
				if (n2 >= 75)
					Data->P[player_index].Manned[4].Safety = 15;
				if (n3 >= 75)
					Data->P[player_index].Manned[4].Safety = 25;
				if ((n1 >= 75 || n2 >= 75 || n3 >= 75) && (n6 >= 75
						|| n7 >= 75))
					Data->P[player_index].Manned[4].Safety = 35;
				break;
			case 5:
				if (n7 >= 75)
					Data->P[player_index].Manned[5].Safety = 30;
				if (n5 >= 75)
					Data->P[player_index].Manned[5].Safety = 40;
				break;
			case 6:
				if (n6 >= 75)
					Data->P[player_index].Manned[6].Safety = 30;
				if (n5 >= 75)
					Data->P[player_index].Manned[6].Safety = 40;
				break;
		}
	}

	/* compute technology transfer for Misc category */
	if (new_program && category == 4)
	{
		n1 = Data->P[player_index].Rocket[0].Safety; /* One - A	   */
		n2 = Data->P[player_index].Rocket[1].Safety; /* Two - B	   */
		n3 = Data->P[player_index].Rocket[2].Safety; /* Three - C   */
		n4 = Data->P[player_index].Rocket[3].Safety; /* Mega - G    */
		n5 = Data->P[player_index].Rocket[4].Safety; /* Booster - D */
		switch (unit - 1)
		{
			case 0:
				if (n2 >= 75)
					Data->P[player_index].Misc[0].Safety = 40;
				break;
			case 1:
				if (n1 >= 75)
					Data->P[player_index].Misc[1].Safety = 35;
				break;
			case 2:
				if (n1 >= 75 || n2 >= 75)
					Data->P[player_index].Misc[2].Safety = 25;
				break;
			default:
				break;
		}
	}

	/* update safety that could have changed */
	if (new_program)
		unit_ptr->Base = unit_ptr->Safety;

	ShowUnit(category, unit, player_index);
	return;
}

/* vim: set noet ts=4 sw=4 tw=77 */
