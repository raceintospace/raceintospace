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
//Equipmet Randomization
//Enabling this option will change game balance, and can possibly break the AI

/* Unit Weight is based in the previous equipment (i.e. Gemini's weight is Mercury's + random number)
	some units have a fixed ranges, loosly based in the basic model
Max Safety is a random number between 80 and 98
Max RD is 2 to 10 lower than Max Safety
Initial Cost is a random number, loosly based in the basic model initial cost
Unit Cost is loosly based in the basic model, but depends on initial cost
RD cost is loosly based in the basic model
*/

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>
#include <assert.h>

/*random_number divides the randomization in 2,
 so there is a lower chance of getting an extreme */
/*random_min is used to give more chances of getting a 1 */

static inline int 
random_aux  (int x,int y)    
{return (random ((y)-(x)+1) + (x)); }

static inline int 
random_number  (int x, int y) 
{return (random_aux ((x)/2,((y)+1)/2) + random_aux(((x)+1)/2,(y)/2)); }

static inline int
random_min  (int n)
{return (maxx (1, random_number (0,n)));}


void RandomizeEq(void) {
	for (int i=0; i<2; i++) { //for each player
		//Satellite
		Data->P[i].Probe[0].UnitWeight 	= random_number (100,500);
		Data->P[i].Probe[0].MaxSafety	= random_number (86,98);
		Data->P[i].Probe[0].MaxRD	= Data->P[i].Probe[0].MaxSafety - random_number (2,10);
		Data->P[i].Probe[0].InitCost    = random_number (3,9);
		Data->P[i].Probe[0].UnitCost	= random_min (Data->P[i].Probe[0].InitCost/2-1);
		Data->P[i].Probe[0].RDCost	= random_min (2);

		//Lunar Satellite
		Data->P[i].Probe[1].UnitWeight 	= Data->P[i].Probe[0].UnitWeight + random_number (200,600);
		Data->P[i].Probe[1].MaxSafety	= random_number (86,98);
		Data->P[i].Probe[1].MaxRD	= Data->P[i].Probe[1].MaxSafety - random_number (2,10);
		Data->P[i].Probe[1].InitCost    = random_number (18,30);
		Data->P[i].Probe[1].UnitCost	= random_number (2,Data->P[i].Probe[1].InitCost/4);
		Data->P[i].Probe[1].RDCost	= random_number (2,4);

		//Lunar Probe
		Data->P[i].Probe[2].UnitWeight 	= Data->P[i].Probe[1].UnitWeight + random_number (300,700);
		Data->P[i].Probe[2].MaxSafety	= random_number (86,98);
		Data->P[i].Probe[2].MaxRD	= Data->P[i].Probe[2].MaxSafety - random_number (2,10);
		Data->P[i].Probe[2].InitCost    = random_number (20,40);
		Data->P[i].Probe[2].UnitCost	= random_number (4,2+Data->P[i].Probe[2].InitCost/5);
		Data->P[i].Probe[2].RDCost	= random_number (2,6);

		//Mercury/Vostok
		Data->P[i].Manned[0].UnitWeight = random_number (400,600);
		Data->P[i].Manned[0].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[0].MaxRD	= Data->P[i].Manned[0].MaxSafety - random_number (2,10);
		Data->P[i].Manned[0].InitCost   = random_number (12,24);
		Data->P[i].Manned[0].UnitCost	= random_min (Data->P[i].Manned[0].InitCost/4);
		Data->P[i].Manned[0].RDCost	= random_min (2);

		//Gemini/Voskhod
		Data->P[i].Manned[1].UnitWeight = Data->P[i].Manned[0].UnitWeight + random_number(500,900);
		Data->P[i].Manned[1].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[1].MaxRD	= Data->P[i].Manned[1].MaxSafety - random_number (2,10);
		Data->P[i].Manned[1].InitCost   = random_number (18,30);
		Data->P[i].Manned[1].UnitCost	= random_number (4,Data->P[i].Manned[1].InitCost/3);
		Data->P[i].Manned[1].RDCost	= random_number (1,3);

		//Apollo/Soyuz
		Data->P[i].Manned[2].UnitWeight = Data->P[i].Manned[1].UnitWeight + random_number (300,500);
		Data->P[i].Manned[2].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[2].MaxRD	= Data->P[i].Manned[2].MaxSafety - random_number (2,10);
		Data->P[i].Manned[2].InitCost   = random_number (28,44);
		Data->P[i].Manned[2].UnitCost	= random_number (8,Data->P[i].Manned[2].InitCost/2-2);
		Data->P[i].Manned[2].RDCost	= random_number (3,7);

		//Minishuttle
		Data->P[i].Manned[3].UnitWeight = Data->P[i].Manned[1].UnitWeight + random_number(200,400);
		Data->P[i].Manned[3].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[3].MaxRD	= Data->P[i].Manned[3].MaxSafety - random_number (2,10);
		Data->P[i].Manned[3].InitCost   = random_number (45,75);
		Data->P[i].Manned[3].UnitCost	= random_number (20,10+Data->P[i].Manned[3].InitCost/2);
		Data->P[i].Manned[3].RDCost	= random_number (5,9);

		//Jupiter/Kvartet
		Data->P[i].Manned[4].UnitWeight = random_number (Data->P[i].Manned[2].UnitWeight*2,Data->P[i].Manned[2].UnitWeight*3.25);
		Data->P[i].Manned[4].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[4].MaxRD	= Data->P[i].Manned[4].MaxSafety - random_number (2,10);
		Data->P[i].Manned[4].InitCost   = random_number (45,75);
		Data->P[i].Manned[4].UnitCost	= random_number (20,10+Data->P[i].Manned[4].InitCost/2);
		Data->P[i].Manned[4].RDCost	= random_number (6,10);

		//Eagle/Duet
		Data->P[i].Manned[5].UnitWeight = random_number (1200,1800);
		Data->P[i].Manned[5].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[5].MaxRD	= Data->P[i].Manned[5].MaxSafety - random_number (2,10);
		Data->P[i].Manned[5].InitCost   = random_number (24,36);
		Data->P[i].Manned[5].UnitCost	= random_number (4,Data->P[i].Manned[5].InitCost/2-3);
		Data->P[i].Manned[5].RDCost	= random_min (4);

		//Cricket/L-3
		Data->P[i].Manned[6].UnitWeight = random_number(800,1200);
		Data->P[i].Manned[6].MaxSafety	= random_number (86,98);
		Data->P[i].Manned[6].MaxRD	= Data->P[i].Manned[6].MaxSafety - random_number (2,10);
		Data->P[i].Manned[6].InitCost   = random_number (36,48);
		Data->P[i].Manned[6].UnitCost	= random_number (4,Data->P[i].Manned[6].InitCost/3-2);
		Data->P[i].Manned[6].RDCost	= random_number (2,6);

		//1-Stage Rocket (Max Payload is at least the satellite and the 1-man capsule)
		Data->P[i].Rocket[0].MaxPay	= maxx(Data->P[i].Manned[0].UnitWeight,Data->P[i].Probe[0].UnitWeight) + random_min (200);
		Data->P[i].Rocket[0].MaxSafety	= random_number (86,98);
		Data->P[i].Rocket[0].MaxRD	= Data->P[i].Rocket[0].MaxSafety - random_number(2,10);
		Data->P[i].Rocket[0].InitCost   = random_number (18,30);
		Data->P[i].Rocket[0].UnitCost	= random_number (2,Data->P[i].Rocket[0].InitCost/6);
		Data->P[i].Rocket[0].RDCost	= random_number (1,3);

		//2-Stage Rocket
		Data->P[i].Rocket[1].MaxPay	= Data->P[i].Rocket[0].MaxPay + random_number (700,1100);
		Data->P[i].Rocket[1].MaxSafety	= random_number (86,98);
		Data->P[i].Rocket[1].MaxRD	= Data->P[i].Rocket[1].MaxSafety - random_number(2,10);
		Data->P[i].Rocket[1].InitCost   = random_number (45,75);
		Data->P[i].Rocket[1].UnitCost	= random_number (6,Data->P[i].Rocket[1].InitCost/3-2);
		Data->P[i].Rocket[1].RDCost	= random_number (2,6);

		//3-Stage Rocket
		Data->P[i].Rocket[2].MaxPay	= Data->P[i].Rocket[1].MaxPay + random_number (1400,2000);
		Data->P[i].Rocket[2].MaxSafety	= random_number (86,98);
		Data->P[i].Rocket[2].MaxRD	= Data->P[i].Rocket[2].MaxSafety - random_number(2,10);
		Data->P[i].Rocket[2].InitCost   = random_number (68,100);
		Data->P[i].Rocket[2].UnitCost	= random_number (14,Data->P[i].Rocket[2].InitCost/4+1);
		Data->P[i].Rocket[2].RDCost	= random_number (4,8);

		//Mega Rocket (Max Payload is at least the 4-man capsule, but maxed at 9999, because otherwise it can show strange symbols)
		Data->P[i].Rocket[3].MaxPay	= minn (Data->P[i].Manned[4].UnitWeight + random_min (Data->P[i].Manned[4].UnitWeight),9999);
		Data->P[i].Rocket[3].MaxSafety	= random_number (86,98);
		Data->P[i].Rocket[3].MaxRD	= Data->P[i].Rocket[3].MaxSafety - random_number(2,10);
		Data->P[i].Rocket[3].InitCost   = random_number (120,170);
		Data->P[i].Rocket[3].UnitCost	= random_number (20,Data->P[i].Rocket[3].InitCost/3-10);
		Data->P[i].Rocket[3].RDCost	= random_number (6,10);

		//Boosters
		Data->P[i].Rocket[4].MaxPay	= random_number (750,1250);
		Data->P[i].Rocket[4].MaxSafety	= random_number (86,98);
		Data->P[i].Rocket[4].MaxRD	= Data->P[i].Rocket[4].MaxSafety - random_number(2,10);
		Data->P[i].Rocket[4].InitCost   = random_number (10,14);
		Data->P[i].Rocket[4].UnitCost	= random_min (Data->P[i].Rocket[4].InitCost/2);
		Data->P[i].Rocket[4].RDCost	= random_min (4);

		//A-Kicker
		Data->P[i].Misc[0].UnitWeight	= random_number (200,400);
		Data->P[i].Misc[0].MaxSafety	= random_number (86,98);
		Data->P[i].Misc[0].MaxRD	= Data->P[i].Misc[0].MaxSafety - random_number(2,10);
		Data->P[i].Misc[0].InitCost 	= random_number (9,15);
		Data->P[i].Misc[0].UnitCost	= random_min (Data->P[i].Misc[0].InitCost/2);
		Data->P[i].Misc[0].RDCost	= random_min (2);

		//B-Kicker (Unit Weight is 2 times A-Kicker weight)
		Data->P[i].Misc[1].UnitWeight	= Data->P[i].Misc[0].UnitWeight*2;
		Data->P[i].Misc[1].MaxSafety	= random_number (86,98);
		Data->P[i].Misc[1].MaxRD	= Data->P[i].Misc[1].MaxSafety - random_number(2,10);
		Data->P[i].Misc[1].InitCost 	= random_number (15,21);
		Data->P[i].Misc[1].UnitCost	= random_number (3,Data->P[i].Misc[1].InitCost/2);
		Data->P[i].Misc[1].RDCost	= 1+random_min (2);

		//C-Kicker
		Data->P[i].Misc[2].UnitWeight	= random_number (2000,3000);
		Data->P[i].Misc[2].MaxSafety	= random_number (86,98);
		Data->P[i].Misc[2].MaxRD	= Data->P[i].Misc[2].MaxSafety - random_number(2,10);
		Data->P[i].Misc[2].InitCost 	= random_number (35,45);
		Data->P[i].Misc[2].UnitCost	= random_number (16,Data->P[i].Misc[2].InitCost/2+4);
		Data->P[i].Misc[2].RDCost	= random_number (3,5);

		//EVA suits (Unit weight, unit cost and RD cost are not randomized)
		Data->P[i].Misc[3].UnitWeight	= 0;
		Data->P[i].Misc[3].MaxSafety	= random_number (86,98);
		Data->P[i].Misc[3].MaxRD	= Data->P[i].Misc[3].MaxSafety - random_number(2,10);
		Data->P[i].Misc[3].InitCost 	= random_number (15,21);
		Data->P[i].Misc[3].UnitCost	= 0;
		Data->P[i].Misc[3].RDCost	= 1;

	}
}


void SetEquipName (char plr) {
	char skip =0;
	RectFill(95,50,227,96,0);
	OutBox (96,51,226,95);
	RectFill(97,52,225,94,3); 
	grSetColor(9);
	PrintAt(103,59, "EQUIPMENT NAME CHANGE");
	PrintAt(123,69, "(ESCAPE TO SKIP)");
	for (int n=0; skip==0 && n<3; n++)
		skip = GetNewName(Data->P[plr].Probe[n].Name);
	for (int n=0; skip==0 && n<7; n++)
		skip = GetNewName(Data->P[plr].Manned[n].Name);
	for (int n=0; skip==0 && n<4; n++)
		skip = GetNewName(Data->P[plr].Rocket[n].Name);
}


char GetNewName (char old_name[20]) {
	char ch=0, num=0, name[20];
	for(int i=0;i<20;i++) name[i]=0x00;

	RectFill(100,74,223,91,3);
	grSetColor(6);PrintAt(162-3*strlen(old_name),80,&old_name[0]);
	InBox (99,84,223,92);
	RectFill(100,85,222,91,0);
	grSetColor(1);PrintAt(162-3*strlen(old_name),90,&old_name[0]);DispChr(0x14);av_sync();

	while (ch != K_ENTER && ch != K_ESCAPE) {
		ch=getch();
		if (ch != (ch&0xff)) ch =0x00;
		if (ch>='a' && ch<='z') ch -=0x20;
		if (ch==0x08 && num>0) name[--num]=0x00;
		else if (num<10 && (isupper(ch) || isdigit(ch) || ch==0x20 || ch=='-'))
			name[num++] = ch;
		RectFill(100,85,222,91,0);
		grSetColor(1); PrintAt(162-3*strlen(name),90,&name[0]); DispChr(0x14); av_sync();
	}
	if (ch == K_ESCAPE) return 1;
	if (ch == K_ENTER) {
		if (name[0]=='\0') return 0;
		else strcpy(&old_name[0],&name[0]);
		}
	delay(100);
	return 0;
}
