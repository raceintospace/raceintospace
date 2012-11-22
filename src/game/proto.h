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

// ***************************
//   USEFUL AND RARE DEFINES
// ***************************

#define BUFFER_SIZE 20*1024   /**< Allocated Buffer in Beginning */
#define MAXBUB 30             /**< Maximum Bubbles */
#define pline(a,b,c,d)        {grMoveTo(a,b) ; grLineTo(c,d);}
#define other(a)          abs( (a)-1 )
#define maxx(a,b)         (((a) > (b)) ? (a) : (b))
#define minn(a,b)         (((a) < (b)) ? (a) : (b))
#define MisStep(a,b,c) PrintAt((a),(b),S_Name[(c)])
#define VBlank() do{}while(0)
#define PROGRAM_NOT_STARTED -1  /* Hardware program not started when Num is set to this */

#define isMile(a,b)   Data->Mile[a][b]

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif

#define xMODE_NOFAIL 0x0020
#define xMODE_NOCOPRO 0x00800
#define xMODE_PUSSY 0x0400
#define xMODE_CLOUDS 0x1000
#define xMODE_SPOT_ANIM 0x2000

#define NOFAIL  (xMODE&xMODE_NOFAIL)
#define NOCOPRO (xMODE&xMODE_NOCOPRO)
#define PUSSY   (xMODE&xMODE_PUSSY)

// SPOT ANIMS / CLOUDS RESERVE 0xf000  high nibble

#define UP_ARROW    0x4800
#define DN_ARROW       0x5000
#define RT_ARROW       0x4D00
#define LT_ARROW       0x4B00
#define K_HOME         0x4700
#define K_END          0x4F00
#define K_ENTER     0x000D
#define K_ESCAPE       0x001B
#define K_SPACE     0x0020

#define ME_FB        2
#define VE_FB        3
#define MA_FB        4
#define JU_FB        5
#define SA_FB        6

#define Milestone_OrbitalSatellite    0
#define Milestone_ManInSpace     1
#define Milestone_EarthOrbit  2
#define Milestone_LunarFlyby    3
#define Milestone_LunarPlanetary     4
#define Milestone_LunarPass   5
#define Milestone_LunarOrbit  6
#define Milestone_LunarLanding   7

#define MANSPACE     27
#define EORBIT       18
#define LPASS        19
#define LORBIT       20
#define LLAND        22
#define ORBSAT       0
#define LUNFLY       1
#define PROBELAND    7

#define DUR_F 8
#define DUR_E 9
#define DUR_D 10
#define DUR_C 11
#define DUR_B 12
#define DUR_A -1    // No such experience

#define CAP1 13
#define CAP2 14
#define CAP3 15
#define CAPMS 16
#define CAP4 17

#define OLAB 23
#define DOCK 24
#define WOMAN 25
#define EWALK 26
#define LWALK 40

#define MAX_X   320
#define MAX_Y   200


// EOF
