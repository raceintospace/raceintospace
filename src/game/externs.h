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
// Key EXTERN Support File
  extern struct Players *Data;
  extern int x,y,mousebuttons,key;
  extern unsigned char *screen;
  extern unsigned char LOAD,ZOND,QUIT,FADE,XMAS;
  extern char pal[3*256];
  extern char plr[2],Name[20],IDT[5],IKEY[5];
  extern char * buffer;
  extern GXHEADER vhptr,vhptr2;
  extern char * oldpal;
  extern long xMODE;
  extern char *S_Name[];
  extern int cdROM,hDISK;
  extern int screen_dirty;
  extern int evflag; /* news.c, used in news_seq.c */
