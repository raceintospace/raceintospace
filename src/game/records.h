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
#define  USSR 1
#define  NOT_SET -1
#define  ORBITAL_SATELLITE  0
#define  MAN_IN_SPACE    1
#define  WOMAN_IN_SPACE      2
#define  PERSON_IN_ORBIT     3
#define  SPACEWALK_MALE      4
#define  SPACEWALK_FEMALE    5
//#define    LUNAR_FLYBY     6
//#define    MERCURY_FLYBY   7
//#define    VENUS_FLYBY     8
//#define    MARS_FLYBY      9
//#define    JUPITER_FLYBY   10
//#define    SATURN_FLYBY    11
#define  LUNAR_PROBE_LANDING     12
//#define    ONE-PERSON_CRAFT    13
//#define    TWO-PERSON_CRAFT    14
//#define    THREE-PERSON_CRAFT      15
//#define    MINISHUTTLE     16
//#define    FOUR-PERSON_CRAFT   17
#define  SPACECRAFT_PRESTIGE_POINTS      18
#define  MANNED_LUNAR_PASS   19
#define  MANNED_LUNAR_ORBIT      20
#define  MANNED_LUNAR_LANDING    21
#define  FEWEST_CASUALTIES   22
#define  MOST_CASUALTIES     23
#define  HIGHEST_SAFETY_AVERAGE      24
#define  LOWEST_SAFETY_AVERAGE   25
#define  FIRST_DOCKING   26
#define  FIRST_MANNED_DOCKING    27
#define  FIRST_ORBITAL_LABORATORY    28
#define  LONGEST_MISSION_DURATION_   29
//#define    MISSION_W/_MOST_PRESTIGE_POINTS     30
#define  FEWEST_MISSIONS_IN_GAME     31
#define  MOST_MISSIONS_IN_GAME   32
#define  MOST_SPACE_MISSIONS_ATTEMPTED   33
#define  MOST_SUCCESSFUL_SPACE_MISSIONS      34
#define  MOST_SPACE_MISSIONS_MALE    35
#define  MOST_SPACE_MISSIONS_FEMALE      36
#define  HIGHEST_PRESTIGE_POINTS_MALE    37
#define  HIGHEST_PRESTIGE_POINTS_FEMALE      38
#define  MOST_DAYS_IN_SPACE_MALE     39
#define  MOST_DAYS_IN_SPACE_FEMALE   40
#define  LONGEST_TERM_OF_DUTY    41
#define  HIGHEST_RATING      42
#define  HIGHEST_AVERAGE_BUDGET      43
#define  LOWEST_AVERAGE_BUDGET   44
#define  GREATEST_PRESTIGE_TOTAL     45
#define  EARLIEST_LOR_LANDING    46
#define  EARLIEST_EOR_LANDING    47
#define  EARLIEST_DIRECT_ASCENT_LANDING      48
#define  EARLIEST_HISTORICAL_LANDING     49
#define  UNITED_STATES   50
#define  SOVIET_UNION    51
#define  LOR     52
#define  EOR     53
#define  DIRECT_ASCENT   54
#define  HISTORICAL      55

extern int Pict[56];

extern char *Record_Names[56];

extern char *Months[12];


#endif // RECORDS_H