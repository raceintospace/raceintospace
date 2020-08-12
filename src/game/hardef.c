
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

extern GXHEADER but;
extern char Option, MAIL;

void
DrawHardef(char plr)
{
	int i;

	FadeOut(2, pal, 10, 0, 0);

	Load_CIA_BUT();
	gxClearDisplay(0, 0);
	Load_RD_BUT(plr);

	ShBox(0, 0, 319, 199);
	IOBox(242, 3, 315, 19);
	InBox(3, 22, 316, 167);
	InBox(3, 3, 30, 19);

	ShBox(0, 160, 319, 199);
	IOBox(5, 162, 77, 197);
	IOBox(81, 162, 158, 197);
	IOBox(162, 162, 239, 197);
	IOBox(243, 162, 315, 197);
	GradRect(4, 23, 315, 159, 0);
	for (i = 4; i < 316; i += 2)
	{
		grPutPixel(i, 57, 11);
		grPutPixel(i, 91, 11);
		grPutPixel(i, 125, 11);
	}
	grSetColor(9);
	DispNum(5, 55, 15);
	DispNum(5, 89, 10);
	DispNum(5, 123, 5);
	gxVirtualDisplay(&but, 0, 0, 8, 165, 74, 194, 0);	// Unmanned
	gxVirtualDisplay(&but, 68, 0, 84, 165, 155, 194, 0);	// Rocket
	gxVirtualDisplay(&but, 141, 0, 165, 165, 236, 194, 0);	// Manned
	gxVirtualDisplay(&but, 214, 0, 246, 165, 312, 194, 0);	// Misc
	grSetColor(1);
	DispBig(40, 5, "EFFICIENCY", 1, -1);
	FlagSm(plr, 4, 4);
	grSetColor(1);
	PrintAt(258, 13, "CONTINUE");
	RectFill(149, 2, 157, 10, 9);
	RectFill(149, 2, 156, 9, 8);
	grSetColor(9);
	PrintAt(163, 8, "ATTEMPTS");
	RectFill(149, 12, 157, 20, 6);
	RectFill(149, 12, 156, 19, 5);
	grSetColor(6);
	PrintAt(163, 18, "SUCCESS");
	FadeIn(2, pal, 10, 0, 0);

	return;
}

void
ShowHard(char plr)
{
	int i, place = -1;
	char Cnt = 0;				// switch between screens

	DrawHardef(plr);
	strcpy(IDT, "i034");
	strcpy(IKEY, "k034");
	WaitForMouseUp();
	while (1)
	{
		key = 0;
		GetMouse();
		if (mousebuttons > 0 || key > 0)	/* Game Play */
		{
			if (((x >= 7 && y >= 164 && x <= 75 && y <= 195
						&& mousebuttons > 0) || key == 'U') && place != 0)
			{
				InBox(7, 164, 75, 195);
				WaitForMouseUp();
				OutBox(7, 164, 75, 195);
				ReButs(place, 0);
				place = 0;
				if (Cnt == 0)
					HInfo(plr, place, 0);
				else
					PInfo(plr, place);
				/* UnManned */
			}
			else if ((x >= 3 && y >= 3 && x <= 30 && y <= 19
					&& mousebuttons > 0) || key == 'T')
			{

				FadeOut(2, pal, 10, 0, 0);
				RectFill(33, 1, 239, 21, 3);
				RectFill(4, 23, 315, 159, 0);
				GradRect(4, 23, 315, 159, 0);
				for (i = 4; i < 316; i += 2)
				{
					grPutPixel(i, 57, 11);
					grPutPixel(i, 91, 11);
					grPutPixel(i, 125, 11);
				}
				if (Cnt == 0)
				{
					grSetColor(1);
					DispBig(40, 5, "PRESTIGE POINTS", 1, -1);
					Cnt = 1;
					PInfo(plr, place);
				}
				else
				{
					grSetColor(1);
					DispBig(40, 5, "EFFICIENCY", 1, -1);
					RectFill(149, 2, 157, 10, 9);
					RectFill(149, 2, 156, 9, 8);
					grSetColor(9);
					PrintAt(163, 8, "ATTEMPTS:");
					RectFill(149, 12, 157, 20, 6);
					RectFill(149, 12, 156, 19, 5);
					grSetColor(6);
					PrintAt(163, 18, "SUCCESS");
					Cnt = 0;
					HInfo(plr, place, 0);
				}
				FadeIn(2, pal, 10, 0, 0);

			}
			else if (((x >= 83 && y >= 164 && x <= 156 && y <= 195
						&& mousebuttons > 0) || key == 'R') && place != 1)
			{
				InBox(83, 164, 156, 195);
				WaitForMouseUp();
				OutBox(83, 164, 156, 195);
				ReButs(place, 1);
				place = 1;
				if (Cnt == 0)
					HInfo(plr, place, 0);
				else
					PInfo(plr, place);
				/* Rocket */
			}
			else if (((x >= 164 && y >= 164 && x <= 237 && y <= 195
						&& mousebuttons > 0) || key == 'C') && place != 2)
			{
				InBox(164, 164, 237, 195);
				WaitForMouseUp();
				OutBox(164, 164, 237, 195);
				/* MANNED */
				ReButs(place, 2);
				place = 2;
				if (Cnt == 0)
					HInfo(plr, place, 0);
				else
					PInfo(plr, place);
			}
			else if (((x >= 245 && y >= 164 && x <= 313 && y <= 195
						&& mousebuttons > 0) || key == 'M') && place != 3)
			{
				InBox(245, 164, 313, 195);
				WaitForMouseUp();
				OutBox(245, 164, 313, 195);
				ReButs(place, 3);
				place = 3;
				if (Cnt == 0)
					HInfo(plr, place, 0);
				else
					PInfo(plr, place);
				/* MISC */
			}
			else if ((x >= 244 && y >= 5 && x <= 314 && y <= 17)
				|| key == K_ENTER)
			{
				InBox(244, 5, 314, 17);
				WaitForMouseUp();
				OutBox(244, 5, 314, 17);
				Del_RD_BUT();
				return;			   /* Done */
			};
		}
	};
}

void
HDispIt(int x1, int y1, int x2, int y2, int s, int t)
{
	GXHEADER local, local2;
	int i, w, h;
	unsigned char *src, *dest;

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	GV(&local, w, h);
	GV(&local2, w, h);
	gxClearVirtual(&local, 0);
	gxClearVirtual(&local2, 0);
	gxGetImage(&local2, s, t, s + w - 1, t + h - 1, 0);
	gxVirtualVirtual(&vhptr, x1, y1, x2, y2, &local, 0, 0, gxSET);
	src = local.vptr;
	dest = local2.vptr;
	for (i = 0; i < w * h; i++)
	{
		if (*src != 0x00)
			*dest = *src;
		dest++;
		src++;
	}

	gxPutImage(&local2, gxSET, s, t, 0);

	DV(&local);
	DV(&local2);
	return;
}

void
PInfo(char plr, char loc)
{
	char j, Pt[4][7], stge;
	int i, tot, sfu;
	float ScaleAmt = 0.0;

	GradRect(4, 23, 315, 159, 0);
	for (i = 4; i < 316; i += 2)
	{
		grPutPixel(i, 57, 11);
		grPutPixel(i, 91, 11);
		grPutPixel(i, 125, 11);
	}
	for (i = 0; i < 4; i++)
		for (j = 0; j < 7; j++)
			Pt[i][j] = 0;
	for (i = 0; i < Data->P[plr].PastMis; i++)
	{
		if (Data->P[plr].History[i].Prestige > 0)
		{
			stge = 0;
			stge = Data->P[plr].History[i].Prestige;
			j = 0;
			if (Data->P[plr].History[i].Hard[j][0] > -1)
				Pt[2][Data->P[plr].History[i].Hard[j][0]] += stge;
			if (Data->P[plr].History[i].Hard[j][1] > -1)
				Pt[3][Data->P[plr].History[i].Hard[j][1]] += stge;
			if (Data->P[plr].History[i].Hard[j][2] > -1)
				Pt[2][Data->P[plr].History[i].Hard[j][2]] += stge;
			if (Data->P[plr].History[i].Hard[j][3] > -1)
			{
				if (Data->P[plr].History[i].Hard[j][3] < 4)
					Pt[0][Data->P[plr].History[i].Hard[j][3]] += stge;
				else
					Pt[3][4] += stge;
			}
			if (Data->P[plr].History[i].Hard[j][4] > 0)
			{
				if (Data->P[plr].History[i].Hard[j][4] < 5)
					Pt[1][Data->P[plr].History[i].Hard[j][4] - 1] += stge;
				else
				{
					Pt[1][Data->P[plr].History[i].Hard[j][4] - 5] += stge;
					Pt[1][4] += stge;
				}
			}					   // end-if
		}						   //end-if
	};							   // end-if

	//EVA suit klugge
	Pt[3][3] += Data->Prestige[26].Points[plr];

	// make sure there are no negative vals in Pt
	for (i = 0; i < 4; i++)
		for (j = 0; j < 7; j++)
			Pt[i][j] = maxx(Pt[i][j], 0);
	tot = 0;

	switch (loc)
	{
		case 0:
			if (Pt[2][5] >= Pt[2][6])
				tot = Pt[2][5];
			else
				tot = Pt[2][6];
			for (i = 0; i < 3; i++)
				if (tot <= Pt[0][i])
					tot = Pt[0][i];
			break;
		case 1:
			for (i = 0; i < 4; i++)
				if (tot <= Pt[1][i])
					tot = Pt[1][i];
			break;
		case 2:
			for (i = 0; i < 5; i++)
				if (tot <= Pt[2][i])
					tot = Pt[2][i];
			break;
		case 3:
			if (Pt[3][3] >= Pt[3][4])
				tot = Pt[3][3];
			else
				tot = Pt[3][4];
			if (tot <= Pt[1][4])
				tot = Pt[1][4];
			if (tot <= Pt[3][0])
				tot = Pt[3][0];
			if (tot <= Pt[3][1])
				tot = Pt[3][1];
			if (tot <= Pt[3][2] && plr == 1)
				tot = Pt[3][2];
			break;
		default:
			break;
	}
	if (tot < 20)
		ScaleAmt = 5.0;
	else if (tot < 40)
		ScaleAmt = 10.0;
	else if (tot < 60)
		ScaleAmt = 15.0;
	else if (tot < 80)
		ScaleAmt = 20.0;
	else if (tot < 100)
		ScaleAmt = 25.0;
	else if (tot < 120)
		ScaleAmt = 30.0;
	else if (tot < 140)
		ScaleAmt = 35.0;
	else if (tot < 160)
		ScaleAmt = 40.0;
	else if (tot < 200)
		ScaleAmt = 50.0;

	grSetColor(9);
	DispNum(5, 55, ScaleAmt * 3.0);
	DispNum(5, 89, ScaleAmt * 2.0);
	DispNum(5, 123, ScaleAmt);
	ScaleAmt = 25.0 / ScaleAmt;
	switch (loc)
	{
		case 1:
			for (i = 0; i < 4; i++)
			{
				sfu = -1;
				if (Data->P[plr].Rocket[i].Num >= 0)
					sfu = ScaleAmt * Pt[1][i];
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(22, 159 - sfu * 136 / 100, 60, 159, 6);
							RectFill(22, 159 - sfu * 136 / 100, 59, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(101, 1, 115, 57, 15, 104);
						else if (sfu > 0)
							HDispIt(129, 1, 149, 85, 15, 75);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(86, 159 - sfu * 136 / 100, 124, 159, 6);
							RectFill(86, 159 - sfu * 136 / 100, 123, 158, 5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(115, 0, 124, 68, 82, 92);
						else if (sfu > 0)
							HDispIt(151, 1, 170, 95, 82, 65);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(175, 159 - sfu * 136 / 100, 213, 159,
								6);
							RectFill(175, 159 - sfu * 136 / 100, 212, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(172, 1, 209, 133, 146, 27);
						else if (sfu > 0)
							HDispIt(211, 1, 243, 133, 148, 27);
						break;
					case 3:
						if (sfu > 0)
						{
							RectFill(260, 159 - sfu * 136 / 100, 298, 159,
								6);
							RectFill(260, 159 - sfu * 136 / 100, 297, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(245, 1, 285, 137, 231, 23);
						else if (sfu > 0)
							HDispIt(287, 1, 318, 132, 231, 28);
						break;
					default:
						break;
				}				   // end switch
			};					   // end case 1 'rockets'
			break;
		case 0:
			sfu = -1;
			if (Data->P[plr].Manned[6].Num >= 0)
				sfu = ScaleAmt * Pt[2][6];
			if (sfu > 0)
			{
				RectFill(25, 159 - sfu * 136 / 100, 63, 159, 6);
				RectFill(25, 159 - sfu * 136 / 100, 62, 158, 5);
			};
			if (plr == 0 && sfu > 0)
				HDispIt(60, 153, 88, 176, 7, 133);
			else if (sfu > 0)
				HDispIt(90, 151, 119, 176, 7, 131);
			sfu = -1;
			if (Data->P[plr].Manned[5].Num >= 0)
				sfu = ScaleAmt * Pt[2][5];
			if (sfu > 0)
			{
				RectFill(101, 159 - sfu * 136 / 100, 139, 159, 6);
				RectFill(101, 159 - sfu * 136 / 100, 138, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(1, 153, 29, 182, 83, 126);
			else if (sfu > 0)
				HDispIt(31, 153, 56, 182, 83, 126);
			for (i = 0; i < 3; i++)
			{
				sfu = -1;
				if (Data->P[plr].Probe[i].Num >= 0)
					sfu = ScaleAmt * Pt[0][i];
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(152, 159 - sfu * 136 / 100, 190, 159,
								6);
							RectFill(152, 159 - sfu * 136 / 100, 189, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(58, 180, 71, 196, 147, 138);
						else if (sfu > 0)
							HDispIt(73, 180, 89, 195, 147, 139);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(212, 159 - sfu * 136 / 100, 250, 159,
								6);
							RectFill(212, 159 - sfu * 136 / 100, 249, 158,
								5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(91, 178, 115, 195, 198, 139);
						else if (sfu > 0)
							HDispIt(153, 142, 176, 166, 198, 132);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(272, 159 - sfu * 136 / 100, 310, 159,
								6);
							RectFill(272, 159 - sfu * 136 / 100, 309, 158,
								5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(121, 142, 151, 166, 253, 132);
						else if (sfu > 0)
							HDispIt(178, 142, 201, 160, 253, 138);
						break;
					default:
						break;
				}				   // end switch
			}					   // end case 2 'misc programs + lm'
			break;
		case 2:
			for (i = 0; i < 5; i++)
			{
				sfu = -1;
				if (Data->P[plr].Manned[i].Num >= 0)
					sfu = ScaleAmt * Pt[2][i];
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(16, 159 - sfu * 136 / 100, 54, 159, 6);
							RectFill(16, 159 - sfu * 136 / 100, 53, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(12, 91, 25, 116, 12, 137);
						else if (sfu > 0)
							HDispIt(0, 56, 26, 89, 14, 123);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(77, 159 - sfu * 136 / 100, 115, 159, 6);
							RectFill(77, 159 - sfu * 136 / 100, 114, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(27, 98, 49, 127, 66, 127);
						else if (sfu > 0)
							HDispIt(28, 62, 49, 96, 66, 122);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(142, 159 - sfu * 136 / 100, 180, 159,
								6);
							RectFill(142, 159 - sfu * 136 / 100, 179, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							DispIt(95, 77, 117, 127, 130, 106);
						else if (sfu > 0)
							HDispIt(119, 97, 170, 140, 122, 113);
						break;
					case 3:
						if (sfu > 0)
						{
							RectFill(198, 159 - sfu * 136 / 100, 236, 159,
								6);
							RectFill(198, 159 - sfu * 136 / 100, 235, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(3, 1, 16, 54, 191, 103);
						else if (sfu > 0)
							HDispIt(18, 1, 32, 48, 191, 109);
						break;
					case 4:
						if (sfu > 0)
						{
							RectFill(266, 159 - sfu * 136 / 100, 304, 159,
								6);
							RectFill(266, 159 - sfu * 136 / 100, 303, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(34, 1, 65, 60, 248, 97);
						else if (sfu > 0)
							HDispIt(67, 1, 100, 60, 248, 97);
						break;
					default:
						break;
				}				   // end switch
			}					   // end case 2 'manned' programs
			break;
		case 3:
			sfu = -1;
			if (Data->P[plr].Misc[3].Num >= 0)
				sfu = ScaleAmt * Pt[3][3];
			if (sfu > 0)
			{
				RectFill(19, 159 - sfu * 136 / 100, 57, 159, 6);
				RectFill(19, 159 - sfu * 136 / 100, 56, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(68, 65, 76, 75, 17, 145);
			else if (sfu > 0)
				HDispIt(78, 65, 86, 75, 17, 145);
			sfu = -1;
			if (Data->P[plr].Misc[4].Num >= 0)
				sfu = ScaleAmt * Pt[3][4];
			if (sfu > 0)
			{
				RectFill(72, 159 - sfu * 136 / 100, 110, 159, 6);
				RectFill(72, 159 - sfu * 136 / 100, 109, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(88, 62, 100, 75, 64, 143);
			else if (sfu > 0)
				HDispIt(102, 66, 114, 75, 64, 147);
			sfu = -1;
			if (Data->P[plr].Rocket[4].Num >= 0)
				sfu = ScaleAmt * Pt[1][4];
			if (sfu > 0)
			{
				RectFill(118, 159 - sfu * 136 / 100, 156, 159, 6);
				RectFill(118, 159 - sfu * 136 / 100, 155, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(1, 120, 14, 151, 113, 125);
			else if (sfu > 0)
				HDispIt(16, 130, 31, 151, 113, 135);
			sfu = -1;
			if (Data->P[plr].Misc[0].Num >= 0)
				sfu = ScaleAmt * Pt[3][0];
			if (sfu > 0)
			{
				RectFill(173, 159 - sfu * 136 / 100, 211, 159, 6);
				RectFill(173, 159 - sfu * 136 / 100, 210, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(33, 140, 47, 151, 165, 145);
			else if (sfu > 0)
				HDispIt(49, 138, 61, 151, 165, 143);
			sfu = -1;
			if (Data->P[plr].Misc[1].Num >= 0)
				sfu = ScaleAmt * Pt[3][1];
			if (sfu > 0)
			{
				RectFill(226, 159 - sfu * 136 / 100, 254, 159, 6);
				RectFill(226, 159 - sfu * 136 / 100, 253, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(63, 131, 75, 151, 219, 136);
			else if (sfu > 0)
				HDispIt(77, 129, 88, 151, 219, 134);
			sfu = -1;

			if (Data->P[1].Misc[2].Num >= 0)
				sfu = ScaleAmt * Pt[3][2];
			if (sfu > 0 && plr == 1)
			{
				RectFill(275, 159 - sfu * 136 / 100, 313, 159, 6);
				RectFill(275, 159 - sfu * 136 / 100, 312, 158, 5);
			}
			if (sfu > 0 && plr == 1)
				HDispIt(51, 77, 93, 127, 270, 106);
			break;
		default:
			break;
	}

	return;
}

void
HInfo(char plr, char loc, char w)
{
	int i, sfu, sfs, tot;
	float ov, un, ScaleAmt = 0.0;

	if (w == 0)
	{
		GradRect(4, 23, 315, 159, 0);
		for (i = 4; i < 316; i += 2)
		{
			grPutPixel(i, 57, 11);
			grPutPixel(i, 91, 11);
			grPutPixel(i, 125, 11);
		}
		// determine scale x5
		tot = 0;
		switch (loc)
		{
			case 0:
				if (Data->P[plr].Manned[5].Steps >=
					Data->P[plr].Manned[6].Steps)
					tot = Data->P[plr].Manned[5].Steps;
				else
					tot = Data->P[plr].Manned[6].Steps;
				for (i = 0; i < 3; i++)
					if (tot <= Data->P[plr].Probe[i].Steps)
						tot = Data->P[plr].Probe[i].Steps;
				break;
			case 1:
				for (i = 0; i < 4; i++)
					if (tot <= Data->P[plr].Rocket[i].Steps)
						tot = Data->P[plr].Rocket[i].Steps;
				break;
			case 2:
				for (i = 0; i < 5; i++)
					if (tot <= Data->P[plr].Manned[i].Steps)
						tot = Data->P[plr].Manned[i].Steps;
				break;
			case 3:
				if (Data->P[plr].Misc[3].Steps >= Data->P[plr].Misc[4].Steps)
					tot = Data->P[plr].Misc[3].Steps;
				else
					tot = Data->P[plr].Misc[4].Steps;
				if (tot <= Data->P[plr].Rocket[4].Steps)
					tot = Data->P[plr].Rocket[4].Steps;
				if (tot <= Data->P[plr].Misc[0].Steps)
					tot = Data->P[plr].Misc[0].Steps;
				if (tot <= Data->P[plr].Misc[1].Steps)
					tot = Data->P[plr].Misc[1].Steps;
				if (tot <= Data->P[plr].Misc[2].Steps && plr == 1)
					tot = Data->P[plr].Misc[2].Steps;
				break;
			default:
				break;
		}

		if (tot < 20)
			ScaleAmt = 5.0;
		else if (tot < 40)
			ScaleAmt = 10.0;
		else if (tot < 60)
			ScaleAmt = 15.0;
		else if (tot < 80)
			ScaleAmt = 20.0;
		else if (tot < 100)
			ScaleAmt = 25.0;
		else if (tot < 120)
			ScaleAmt = 30.0;

		grSetColor(9);
		DispNum(5, 55, ScaleAmt * 3.0);
		DispNum(5, 89, ScaleAmt * 2.0);
		DispNum(5, 123, ScaleAmt);
	}
	ScaleAmt = 25.0 / ScaleAmt;
	grSetColor(1);
	switch (loc)
	{
		case 1:				   //PrintAt(137,150,"ROCKETS");
			for (i = 0; i < 4; i++)
			{
				sfu = -1;
				sfs = -1;
				if (Data->P[plr].Rocket[i].Num >= 0)
				{
					sfu = ScaleAmt * Data->P[plr].Rocket[i].Steps;
					if (sfu > 0)
					{
						ov = Data->P[plr].Rocket[i].Steps -
							Data->P[plr].Rocket[i].Failures;
						un = Data->P[plr].Rocket[i].Steps;
						sfs = sfu * (ov / un);
					}
				}
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(22, 159 - sfu * 136 / 100, 60, 159, 9);
							RectFill(22, 159 - sfu * 136 / 100, 59, 158, 8);
						};
						if (sfs > 0)
						{
							RectFill(22, 159 - sfs * 136 / 100, 55, 159, 6);
							RectFill(22, 159 - sfs * 136 / 100, 54, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(101, 1, 115, 57, 15, 104);
						else if (sfu > 0)
							HDispIt(129, 1, 149, 85, 15, 75);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(86, 159 - sfu * 136 / 100, 124, 159, 9);
							RectFill(86, 159 - sfu * 136 / 100, 123, 158, 8);
						}
						if (sfs > 0)
						{
							RectFill(86, 159 - sfs * 136 / 100, 119, 159, 6);
							RectFill(86, 159 - sfs * 136 / 100, 118, 158, 5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(115, 0, 124, 68, 82, 92);
						else if (sfu > 0)
							HDispIt(151, 1, 170, 95, 82, 65);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(175, 159 - sfu * 136 / 100, 213, 159,
								9);
							RectFill(175, 159 - sfu * 136 / 100, 212, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(175, 159 - sfs * 136 / 100, 208, 159,
								6);
							RectFill(175, 159 - sfs * 136 / 100, 207, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(172, 1, 209, 133, 146, 27);
						else if (sfu > 0)
							HDispIt(211, 1, 243, 133, 148, 27);
						break;
					case 3:
						if (sfu > 0)
						{
							RectFill(260, 159 - sfu * 136 / 100, 298, 159,
								9);
							RectFill(260, 159 - sfu * 136 / 100, 297, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(260, 159 - sfs * 136 / 100, 293, 159,
								6);
							RectFill(260, 159 - sfs * 136 / 100, 292, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(245, 1, 285, 137, 231, 23);
						else if (sfu > 0)
							HDispIt(287, 1, 318, 132, 231, 28);
						break;
					default:
						break;
				}				   // end switch
			};					   // end case 1 'rockets'
			break;
		case 2:				   //PrintAt(137,150,"CAPSULES");
			for (i = 0; i < 5; i++)
			{
				sfu = -1;
				sfs = -1;
				if (Data->P[plr].Manned[i].Num >= 0)
				{
					sfu = ScaleAmt * Data->P[plr].Manned[i].Steps;
					if (sfu > 0)
					{
						ov = Data->P[plr].Manned[i].Steps -
							Data->P[plr].Manned[i].Failures;
						un = Data->P[plr].Manned[i].Steps;
						sfs = sfu * (ov / un);
					}
				}
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(16, 159 - sfu * 136 / 100, 54, 159, 9);
							RectFill(16, 159 - sfu * 136 / 100, 53, 158, 8);
						};
						if (sfs > 0)
						{
							RectFill(16, 159 - sfs * 136 / 100, 49, 159, 6);
							RectFill(16, 159 - sfs * 136 / 100, 48, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(12, 91, 25, 116, 12, 137);
						else if (sfu > 0)
							HDispIt(0, 56, 26, 89, 14, 123);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(77, 159 - sfu * 136 / 100, 115, 159, 9);
							RectFill(77, 159 - sfu * 136 / 100, 114, 158, 8);
						};
						if (sfs > 0)
						{
							RectFill(77, 159 - sfs * 136 / 100, 110, 159, 6);
							RectFill(77, 159 - sfs * 136 / 100, 109, 158, 5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(27, 98, 49, 127, 66, 127);
						else if (sfu > 0)
							HDispIt(28, 62, 49, 96, 66, 122);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(142, 159 - sfu * 136 / 100, 180, 159,
								9);
							RectFill(142, 159 - sfu * 136 / 100, 179, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(142, 159 - sfs * 136 / 100, 175, 159,
								6);
							RectFill(142, 159 - sfs * 136 / 100, 174, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							DispIt(95, 77, 117, 127, 130, 106);
						else if (sfu > 0)
							HDispIt(119, 97, 170, 140, 122, 113);
						break;
					case 3:
						if (sfu > 0)
						{
							RectFill(198, 159 - sfu * 136 / 100, 236, 159,
								9);
							RectFill(198, 159 - sfu * 136 / 100, 235, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(198, 159 - sfs * 136 / 100, 231, 159,
								6);
							RectFill(198, 159 - sfs * 136 / 100, 230, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(3, 1, 16, 54, 191, 103);
						else if (sfu > 0)
							HDispIt(18, 1, 32, 48, 191, 109);
						break;
					case 4:
						if (sfu > 0)
						{
							RectFill(266, 159 - sfu * 136 / 100, 304, 159,
								9);
							RectFill(266, 159 - sfu * 136 / 100, 303, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(266, 159 - sfs * 136 / 100, 299, 159,
								6);
							RectFill(266, 159 - sfs * 136 / 100, 298, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(34, 1, 65, 60, 248, 97);
						else if (sfu > 0)
							HDispIt(67, 1, 100, 60, 248, 97);
						break;
					default:
						break;
				}				   // end switch
			}					   // end case 2 'manned' programs
			break;
		case 0:				   //PrintAt(100,150,"SATELLITES & LM'S");
			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Manned[6].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Manned[6].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Manned[6].Steps -
						Data->P[plr].Manned[6].Failures;
					un = Data->P[plr].Manned[6].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(25, 159 - sfu * 136 / 100, 63, 159, 9);
				RectFill(25, 159 - sfu * 136 / 100, 62, 158, 8);
			};
			if (sfs > 0)
			{
				RectFill(25, 159 - sfs * 136 / 100, 58, 159, 6);
				RectFill(25, 159 - sfs * 136 / 100, 57, 158, 5);
			};
			if (plr == 0 && sfu > 0)
				HDispIt(60, 153, 88, 176, 9, 132);
			else if (sfu > 0)
				HDispIt(90, 151, 119, 176, 9, 132);

			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Manned[5].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Manned[5].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Manned[5].Steps -
						Data->P[plr].Manned[5].Failures;
					un = Data->P[plr].Manned[5].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(101, 159 - sfu * 136 / 100, 139, 159, 9);
				RectFill(101, 159 - sfu * 136 / 100, 138, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(101, 159 - sfs * 136 / 100, 134, 159, 6);
				RectFill(101, 159 - sfs * 136 / 100, 133, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(1, 153, 29, 182, 83, 128);
			else if (sfu > 0)
				HDispIt(31, 153, 56, 182, 83, 128);

			for (i = 0; i < 3; i++)
			{
				sfu = -1;
				sfs = -1;
				if (Data->P[plr].Probe[i].Num >= 0)
				{
					sfu = ScaleAmt * Data->P[plr].Probe[i].Steps;
					if (sfu > 0)
					{
						ov = Data->P[plr].Probe[i].Steps -
							Data->P[plr].Probe[i].Failures;
						un = Data->P[plr].Probe[i].Steps;
						sfs = sfu * (ov / un);
					}
				}
				switch (i)
				{
					case 0:
						if (sfu > 0)
						{
							RectFill(152, 159 - sfu * 136 / 100, 190, 159,
								9);
							RectFill(152, 159 - sfu * 136 / 100, 189, 158,
								8);
						};
						if (sfs > 0)
						{
							RectFill(152, 159 - sfs * 136 / 100, 185, 159,
								6);
							RectFill(152, 159 - sfs * 136 / 100, 184, 158,
								5);
						};
						if (plr == 0 && sfu > 0)
							HDispIt(58, 180, 71, 196, 147, 138);
						else if (sfu > 0)
							HDispIt(73, 180, 89, 195, 147, 139);
						break;
					case 1:
						if (sfu > 0)
						{
							RectFill(212, 159 - sfu * 136 / 100, 250, 159,
								9);
							RectFill(212, 159 - sfu * 136 / 100, 249, 158,
								8);
						}
						if (sfs > 0)
						{
							RectFill(212, 159 - sfs * 136 / 100, 245, 159,
								6);
							RectFill(212, 159 - sfs * 136 / 100, 244, 158,
								5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(91, 178, 115, 195, 198, 139);
						else if (sfu > 0)
							HDispIt(153, 142, 176, 166, 198, 132);
						break;
					case 2:
						if (sfu > 0)
						{
							RectFill(272, 159 - sfu * 136 / 100, 310, 159,
								9);
							RectFill(272, 159 - sfu * 136 / 100, 309, 158,
								8);
						}
						if (sfs > 0)
						{
							RectFill(272, 159 - sfs * 136 / 100, 305, 159,
								6);
							RectFill(272, 159 - sfs * 136 / 100, 304, 158,
								5);
						}
						if (plr == 0 && sfu > 0)
							HDispIt(121, 142, 151, 166, 253, 132);
						else if (sfu > 0)
							HDispIt(178, 142, 201, 160, 253, 138);
						break;
					default:
						break;
				}				   // end switch
			}					   // end case 2 'misc programs + lm'
			break;
		case 3:				   //PrintAt(100,150,"ADDITIONAL PROGRAMS");
			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Misc[3].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Misc[3].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Misc[3].Steps -
						Data->P[plr].Misc[3].Failures;
					un = Data->P[plr].Misc[3].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(19, 159 - sfu * 136 / 100, 57, 159, 9);
				RectFill(19, 159 - sfu * 136 / 100, 56, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(19, 159 - sfs * 136 / 100, 52, 159, 6);
				RectFill(19, 159 - sfs * 136 / 100, 51, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(68, 65, 76, 75, 17, 145);
			else if (sfu > 0)
				HDispIt(78, 65, 86, 75, 17, 145);

			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Misc[4].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Misc[4].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Misc[4].Steps -
						Data->P[plr].Misc[4].Failures;
					un = Data->P[plr].Misc[4].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(72, 159 - sfu * 136 / 100, 110, 159, 9);
				RectFill(72, 159 - sfu * 136 / 100, 109, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(72, 159 - sfs * 136 / 100, 105, 159, 6);
				RectFill(72, 159 - sfs * 136 / 100, 104, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(88, 62, 100, 75, 64, 143);
			else if (sfu > 0)
				HDispIt(102, 66, 114, 75, 64, 147);
			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Rocket[4].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Rocket[4].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Rocket[4].Steps -
						Data->P[plr].Rocket[4].Failures;
					un = Data->P[plr].Rocket[4].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(118, 159 - sfu * 136 / 100, 156, 159, 9);
				RectFill(118, 159 - sfu * 136 / 100, 155, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(118, 159 - sfs * 136 / 100, 151, 159, 6);
				RectFill(118, 159 - sfs * 136 / 100, 150, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(1, 120, 14, 151, 113, 125);
			else if (sfu > 0)
				HDispIt(16, 130, 31, 151, 113, 135);
			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Misc[0].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Misc[0].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Misc[0].Steps -
						Data->P[plr].Misc[0].Failures;
					un = Data->P[plr].Misc[0].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(173, 159 - sfu * 136 / 100, 211, 159, 9);
				RectFill(173, 159 - sfu * 136 / 100, 210, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(173, 159 - sfs * 136 / 100, 206, 159, 6);
				RectFill(173, 159 - sfs * 136 / 100, 205, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(33, 140, 47, 151, 165, 145);
			else if (sfu > 0)
				HDispIt(49, 138, 61, 151, 165, 143);
			sfu = -1;
			sfs = -1;
			if (Data->P[plr].Misc[1].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[plr].Misc[1].Steps;
				if (sfu > 0)
				{
					ov = Data->P[plr].Misc[1].Steps -
						Data->P[plr].Misc[1].Failures;
					un = Data->P[plr].Misc[1].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0)
			{
				RectFill(226, 159 - sfu * 136 / 100, 254, 159, 9);
				RectFill(226, 159 - sfu * 136 / 100, 253, 158, 8);
			}
			if (sfs > 0)
			{
				RectFill(226, 159 - sfs * 136 / 100, 249, 159, 6);
				RectFill(226, 159 - sfs * 136 / 100, 248, 158, 5);
			}
			if (plr == 0 && sfu > 0)
				HDispIt(63, 131, 75, 151, 219, 136);
			else if (sfu > 0)
				HDispIt(77, 129, 88, 151, 219, 134);
			sfu = -1;
			sfs = -1;
			if (Data->P[1].Misc[2].Num >= 0)
			{
				sfu = ScaleAmt * Data->P[1].Misc[2].Steps;
				if (sfu > 0)
				{
					ov = Data->P[1].Misc[2].Steps -
						Data->P[1].Misc[2].Failures;
					un = Data->P[1].Misc[2].Steps;
					sfs = sfu * (ov / un);
				}
			}
			if (sfu > 0 && plr == 1)
			{
				RectFill(275, 159 - sfu * 136 / 100, 313, 159, 9);
				RectFill(275, 159 - sfu * 136 / 100, 312, 158, 8);
			}
			if (sfs > 0 && plr == 1)
			{
				RectFill(275, 159 - sfs * 136 / 100, 308, 159, 6);
				RectFill(275, 159 - sfs * 136 / 100, 307, 158, 5);
			}
			if (sfu > 0 && plr == 1)
				HDispIt(51, 77, 93, 127, 270, 106);
			break;
		default:
			break;
	};

	return;
}

void
RankMe(char plr)
{
	int i;

	FadeOut(2, pal, 5, 0, 0);
	PortPal(plr);
	gxClearDisplay(0, 0);
	ShBox(52, 0, 267, 32);
	ShBox(0, 0, 50, 32);
	ShBox(269, 0, 319, 32);
	DispBig(74, 4, "DIRECTOR RANKING", 0, -1);
	ShBox(0, 34, 319, 198);
	RectFill(25, 48, 293, 184, 4);
	RectFill(27, 50, 291, 182, 7);
	RectFill(28, 51, 290, 181, 0);
	RectFill(33, 56, 285, 176, 7);
	RectFill(34, 57, 284, 175, 0);
	grSetColor(7);
	grMoveTo(27, 50);
	grLineTo(33, 56);
	grMoveTo(27, 182);
	grLineTo(33, 176);
	grMoveTo(285, 56);
	grLineTo(291, 50);
	grMoveTo(285, 176);
	grLineTo(291, 182);
	grSetColor(7);
	for (i = 0; i < 22; i++)
	{
		if (i < 11)
		{
			grMoveTo(33, 56 + (i * 12));
			grLineTo(285, 56 + (i * 12));
		}
		grMoveTo(33 + (i * 12), 56);
		grLineTo(33 + (i * 12), 176);
	}
	grSetColor(16);
	grMoveTo(34, 152);
	grLineTo(284, 152);
	DrawRank(plr);
	if (Option == -1 || Option == 0)
	{
		RectFill(43, 66, 49, 70, 6);
		RectFill(43, 66, 48, 69, 5);
		grSetColor(1);
		PrintAt(56, 70, "U.S.A.");
	}
	if (Option == -1 || Option == 1)
	{
		RectFill(43, 78, 49, 82, 9);
		RectFill(43, 78, 48, 81, 8);
		PrintAt(56, 82, "U.S.S.R.");
	}
	FadeIn(2, pal, 5, 0, 0);

	WaitForMouseUp();
	while (1)
	{
		GetMouse();
		if ((x >= 0 && y >= 0 && x <= 319 && y <= 199 && mousebuttons > 0)
			|| key == K_ENTER)
		{
			if (key > 0)
			{
				delay(300);
				key = 0;
			};
			WaitForMouseUp();
			key = 0;
			return;
		}
	};
}

void
DrawRank(char plr)
{
	int i, MaxPz = 0, MaxNg = 0, Px, Py, t1, t2, Cur = 0, OffSet, Year_Inc =
		12, score;
	char Digit[4];

	strcpy(IDT, "i030");
	strcpy(IKEY, "k030");

	//Win=Data->Prestige[22].Place;
	InBox(12, 4, 39, 20);
	InBox(281, 4, 308, 20);
	FlagSm(0, 13, 5);
	FlagSm(1, 282, 5);
	Px = 0;
	Py = 1;
	while (Px != -1 || Py != -1)
	{
		if (Px != -1)
			Cur = Px;
		else if (Py != -1)
			Cur = Py;
		t1 = 0;
		for (i = 0; i < Data->P[Cur].PastMis; i++)
		{
			if (Data->P[Cur].History[i].Prestige < -15)
				Data->P[Cur].History[i].Prestige = 0;
			t1 += Data->P[Cur].History[i].Prestige;
			if (t1 >= 0)
			{
				if (t1 >= MaxPz)
					MaxPz = t1;
			}
			else if (t1 < 0)
			{
				if (t1 <= MaxNg)
					MaxNg = t1;
			}
		}
		if (Cur == Px)
			Px = -1;
		else if (Cur == Py)
			Py = -1;
	}
	OffSet = 0;
	for (i = 0; i < 50; i += 2)
	{
		if (MaxNg != 0)
		{
			if ((MaxNg >= -(i * 10)) && (MaxPz < i * 40))
				if (OffSet == 0)
					OffSet = i * 10;
		}
		else
		{
			if (MaxPz < i * 40)
				if (OffSet == 0)
					OffSet = i * 10;
		}
	}
	grSetColor(6);
	for (i = 0; i < 11; i++)
	{
		DispNum(29 + (24 * i), 43, (i * 2) + 57);
		DispNum(41 + (24 * i), 192, (i * 2) + 58);
	}
	grSetColor(6);
	for (i = 0; i < 6; i++)
	{
		switch (i)
		{
			case 0:
				if (OffSet < 10)
				{
					DispNum(15, 178, -OffSet);
					DispNum(311, 178, -OffSet);
				}
				else if (OffSet < 100)
				{
					DispNum(4, 178, -OffSet);
					DispNum(300, 178, -OffSet);
				}
				else
				{
					DispNum(3, 178, -OffSet);
					DispNum(299, 178, -OffSet);
				}
				break;
			case 1:
				DispNum(10, 154, 0);
				DispNum(306, 154, 0);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				if (OffSet * (i - 1) < 10)
				{
					DispNum(15, 178 - (i * 24), OffSet * (i - 1));
					DispNum(311, 178 - (i * 24), OffSet * (i - 1));
				}
				else if (OffSet * (i - 1) < 100)
				{
					DispNum(6, 178 - (i * 24), OffSet * (i - 1));
					DispNum(302, 178 - (i * 24), OffSet * (i - 1));
				}
				else
				{
					DispNum(3, 178 - (i * 24), OffSet * (i - 1));
					DispNum(299, 178 - (i * 24), OffSet * (i - 1));
				}
				break;
			default:
				break;
		}
	}
	//Win=Data->Prestige[22].Place;
	i = 0;
	t1 = 152;
	t2 = 0;
	grSetColor(5);
	grMoveTo(34, 152);
	//if ((Option==-1 && MAIL==-1) || Option==0 || MAIL==0)

	if (Option == -1 || Option == 0)
		while (i < Data->P[0].PastMis)
		{
			t2 = 0;
			t2 = Data->P[0].History[i].Prestige;
			t1 = t1 - (t2 * ((float) 24 / OffSet));
			grLineTo(34 + Year_Inc * (Data->P[0].History[i].MissionYear -
					57) + Data->P[0].History[i].Month, t1);
			i++;
		}
	i = 0;
	t1 = 152;
	t2 = 0;
	grSetColor(9);
	grMoveTo(34, 152);
	//if ((Option==-1 && MAIL==-1) || Option==1 || MAIL==1)

	if (Option == -1 || Option == 1)
		while (i < Data->P[1].PastMis)
		{
			t2 = 0;
			t2 = Data->P[1].History[i].Prestige;
			t1 = t1 - (t2 * ((float) 24 / OffSet));
			grLineTo(34 + Year_Inc * (Data->P[1].History[i].MissionYear -
					57) + Data->P[1].History[i].Month, t1);
			i++;
		}
	grSetColor(1);
	PrintAt(6, 27, "LEVEL:");
	PrintAt(275, 27, "LEVEL:");
	grSetColor(0);
	DispNum(41, 27, Data->Def.Lev1 + 1);
	grSetColor(0);
	DispNum(310, 27, Data->Def.Lev2 + 1);
	if (Option == -1 || Option == 0)
	{
		InBox(55, 21, 116, 29);
		grSetColor(1);
		PrintAt(57, 27, "SCORE:");
		score = 0;
		score = CalcScore(0, Data->Def.Lev1, Data->Def.Lev2);
		sprintf(&Digit[0], "%d", score);
		PrintAt(92, 27, &Digit[0]);
	}
	if (Option == -1 || Option == 1)
	{
		InBox(203, 21, 264, 29);
		grSetColor(1);
		PrintAt(205, 27, "SCORE:");
		score = CalcScore(1, Data->Def.Lev2, Data->Def.Lev1);
		sprintf(&Digit[0], "%d", score);
		PrintAt(240, 27, &Digit[0]);
	}

}

int
CalcScore(char plr, char lvA, char lvB)
{
	char i;
	int total = 0;

	++lvA;
	++lvB;
	for (i = 0; i < Data->P[plr].PastMis; i++)
	{
		if (Data->P[plr].History[i].Prestige > 0)
		{
			switch (lvA)
			{
				case 1:
					total += Data->P[plr].History[i].Prestige;
					break;
				case 2:
					if (lvB == 1)
						total +=
							2 * Data->P[plr].History[i].Prestige +
							Data->P[plr].History[i].Prestige / 3;
					else if (lvB == 3)
						total +=
							2 * Data->P[plr].History[i].Prestige +
							Data->P[plr].History[i].Prestige / 3;
					else
						total += 2 * Data->P[plr].History[i].Prestige;
					break;
				case 3:
					if (lvB == 1)
						total +=
							2 * Data->P[plr].History[i].Prestige +
							(2 * Data->P[plr].History[i].Prestige) / 3;
					else if (lvB == 2)
						total +=
							3 * Data->P[plr].History[i].Prestige +
							Data->P[plr].History[i].Prestige / 3;
					else
						total += 3 * Data->P[plr].History[i].Prestige;
					break;
				default:
					break;
			}
		}
	}
	if (plr == Data->Prestige[22].Place)
	{
		if (lvA == 1)
		{
			if (Data->Year <= 67)
				total += 50;
			else if (Data->Year <= 69)
				total += 25;
		}
		else if (lvA == 2)
		{
			if (Data->Year <= 67)
				total += 75;
			else if (Data->Year <= 69)
				total += 50;
			else if (Data->Year <= 73)
				total += 25;
		}
		else if (lvA == 3)
		{
			if (Data->Year <= 67)
				total += 150;
			else if (Data->Year <= 69)
				total += 75;
			else if (Data->Year <= 73)
				total += 50;
		}
	}
	return (total);
}

/* EOF */
