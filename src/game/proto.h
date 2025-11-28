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
#define pline(a,b,c,d)        {grMoveTo(a,b) ; grLineTo(c,d);}
#define other(a)          abs( (a)-1 )
#define minn(a,b)         (((a) < (b)) ? (a) : (b))
#define MisStep(a,b,c) draw_string((a),(b),S_Name[(c)])
#define PROGRAM_NOT_STARTED -1  /* Hardware program not started when Num is set to this */

#define isMile(a,b)   Data->Mile[a][b]

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif

#define xMODE_NOFAIL 0x0020
#define xMODE_EASYMODE 0x0400
#define xMODE_CLOUDS 0x1000
#define xMODE_SPOT_ANIM 0x2000

#define NOFAIL  (xMODE&xMODE_NOFAIL)
#define EASYMODE   (xMODE&xMODE_EASYMODE)

// SPOT ANIMS / CLOUDS RESERVE 0xf000  high nibble

#define UP_ARROW    0x4800
#define DN_ARROW       0x5000
#define RT_ARROW       0x4D00
#define LT_ARROW       0x4B00
#define K_HOME         0x4700
#define K_END          0x4F00
#define K_PGUP         0x4900
#define K_PGDN         0x5100
#define K_ENTER     0x000D
#define K_ESCAPE       0x001B
#define K_SPACE     0x0020

#define MAX_X   320
#define MAX_Y   200


// EOF
