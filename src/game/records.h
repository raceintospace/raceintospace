#ifndef RECORDS_H
#define RECORDS_H

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

void Records(char plr);
void UpdateRecords(char Ty);
void SafetyRecords(char plr, int temp);
void MakeRecords(void);

typedef struct pEtype {
    char country;
    char month;
    char yr;
    char program;
    short  tag;
    char type;
    char place;
    char name[20];
    char astro[14];
}  Record_Entry;

extern Record_Entry rec[56][3];

/* The beauty of awk */

#define  USA 0
#define  NOT_SET -1

extern int Pict[56];

extern char *Record_Names[56];

extern char *Months[12];


#endif // RECORDS_H
