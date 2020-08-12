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

#include <gamedata.h>
#include <Buzz_inc.h>
#include <externs.h>
#include <macros.h>
#include <av.h>
#include <utils.h>

#define LET_A   0x09
#define LET_M   0x0A
#define LET_O   0x0B
#define LET_V   0x0C
#define LET_R   0x08

#define SPOT_ON 1 /**< turn off until everything else works - pace */
#define BABYSND 1
#define pNOREDRAW 0
#define pREDRAW 1
#define pEXIT 2
#define pQUIT 3
#define pNOFADE 4
#define pNEWMUSIC 5

int put_serial(unsigned char n);

char RUSH,SUSPEND;
extern int oldx,oldy;
extern struct mStr Mis;
extern char Option;

typedef struct portoutlinerestore {
   ui16 loc;
   char val;
} PORTOUTLINE;

PORTOUTLINE *pPortOutlineRestore;

struct FHead {
  char Text[28];  /**< File Copyright Notice */
  long oMObj;     /**< Offset to MObj data table */
  long oTab;      /**< Offset to Table of data */
  long oPal;      /**< Offset to Palette */
  long oPort;     /**< Offset to start of Port Images */
  long oMse;      /**< Offset to Mouse Objects */
  long oOut;      /**< Offset to port Outlines */
  long oAnim;     /**< Offset to start of Port Anims */
   } PHead;

typedef struct cBoxx {
   i16 x1,y1,x2,y2;
   } BOUND;

typedef struct Img {
  long Size;         /**<  Size of Image (bytes) */
  char Comp;         /**<  Type of Compression Used */
  i16 Width;         /**<  Width of Image */
  i16 Height;        /**<  Height of Image */
  i16 PlaceX;        /**<  Where to Place Img:X */
  i16 PlaceY;        /**<  Where to Place Img:Y */
   } IMG;

typedef struct region {
  char qty;          /**< number of BOUNDS */
  BOUND CD[4];       /**< boundry coords for mouse location */
  char iNum;         
  char sNum;         /**< value for surround box */
  char PreDraw;      /**< Code of Special to Draw first */
   } REGION;


typedef struct mobj {
  char Name[30];       /**< Name of region */
  char qty;            /**< Number of regions */
  char Help[3];        /**< Help Box Stuff */
  REGION Reg[4];       /**< At Max 4 regions */
   } MOBJ;

#define S_QTY 43

MOBJ MObj[35];

/** These are the valid hotkeys */
char HotKeyList[]="AIMRPVCQETB\0";

int FCtr;
GXHEADER flaggy;

#define SPOTS 100

/** SPOT structures and data structure variables */
struct mSPOT {        // Main SPOT Header
  uint8_t ID[40];     /**< Copyright notice */
  uint8_t Qty;        /**< Number of Paths */
  uint32_t sOff;      /**< Spot Offsets */
  uint32_t pOff;      /**< Path Offsets */
} MSPOT;

struct sPATH {       // Spot Anim Path Struct
  uint16_t Image;        // Which image to Use
  int16_t  xPut,yPut;    // Where to place this image
  int16_t iHold;         // Repeat this # times
  float Scale;       // Scale object
};

struct sIMG {
  uint8_t w,h;    // Width and Height
};



int16_t sCount,Vab_Spot;        // sCount is the number of steps
FILE *sFin;
SimpleHdr hSPOT;  // Filled by Seek_sOff();
struct sPATH sPath,sPathOld;
struct sIMG sImg,sImgOld;
uint32_t pTable,pLoc;

 
void
Seek_sOff(int where)
{
	fseek(sFin, where * sizeof_SimpleHdr + MSPOT.sOff, SEEK_SET);
	fread_SimpleHdr(&hSPOT, 1, sFin);
	fseek(sFin, hSPOT.offset, SEEK_SET);
}

void Seek_pOff(int where)
{
  fseek(sFin,where*(sizeof pTable)+(MSPOT.pOff),SEEK_SET);
  fread(&pTable,sizeof pTable,1,sFin);
	Swap32bit(pTable);
  fseek(sFin,pTable,SEEK_SET);
}


char PName[20];

#define SPOT_LOAD 0
#define SPOT_STEP 1
#define SPOT_DONE 2
#define SPOT_KILL 3

void SpotCrap(char loc,char mode)
{
  GXHEADER SP1,SP2,SP3;
  int w,h,i;
  int xx;
  static char turnoff=0;

   if (SUSPEND==1) {
     if (turnoff==1) StopVoice();  //Specs: suspend sound babypics 
     return;
    }

   if (sCount==-1  && mode!=SPOT_LOAD) return;

   if (mode==SPOT_LOAD) {   // Open File
      sFin=sOpen("SPOTS.CDR","rb",0);
      fread(&MSPOT,sizeof MSPOT,1,sFin);    // Read Header
			Swap32bit(MSPOT.sOff);
			Swap32bit(MSPOT.pOff);

      Seek_pOff(loc);  // go to correct path
      fread(&PName,sizeof PName,1,sFin);
      fread(&sCount,sizeof sCount,1,sFin);  // get number of paths parts
			Swap16bit(sCount);
      pLoc=ftell(sFin);
      sPath.iHold=1;
      memcpy(vhptr.vptr,screen,MAX_X*MAX_Y);
      sPathOld.xPut=-1;
      SpotCrap(0,SPOT_STEP);
      // All opened up
   }
   else if (mode==SPOT_STEP && sPath.iHold==1 && sCount>0) {  // Play Next Seq
      fseek(sFin,pLoc,SEEK_SET);       // position at next path
      fread(&sPath,sizeof(sPath),1,sFin);  // get the next sPath struct
			
			Swap16bit(sPath.Image);
			Swap16bit(sPath.xPut);
			Swap16bit(sPath.yPut);
			Swap16bit(sPath.iHold);
			SwapFloat(sPath.Scale);

      pLoc=ftell(sFin);                 // Path Update Locations

      Seek_sOff(sPath.Image);          // point to next image
      fread(&sImg,sizeof sImg,1,sFin);    // get image header

      {
	      int expected_w = hSPOT.size / sImg.h;
		  /* DEBUG - FIXING sImg.w */
	      if (sImg.w != expected_w) {
		      sImg.w = expected_w;
	      }
      }

      sImg.w = hSPOT.size / sImg.h;
      GV(&SP1,sImg.w,sImg.h);          // create Virtual buffer
      fread(SP1.vptr,hSPOT.size,1,sFin);  // read image data


      if (sPath.Scale!=1.0) {
	      w=(int) ((float) sImg.w * sPath.Scale);
	      h=(int) ((float) sImg.h * sPath.Scale);
        sImg.w=w;
        sImg.h=h;
        gxVirtualScale(&SP1,&SP2);
      }

      GV(&SP3,sImg.w,sImg.h);  // background buffer

      gxVirtualVirtual(&vhptr,
              minn(sPath.xPut, 319),
              minn(sPath.yPut, 199),
              minn(sPath.xPut+sImg.w-1, 319),
              minn(sPath.yPut+sImg.h-1, 199),
              &SP3,0,0,0);

      if (sPath.Scale!=1.0) {
        xx=hSPOT.size;
        for (i=0;i<xx;i++) {
           if (SP2.vptr[i]==0) SP2.vptr[i]=SP3.vptr[i];
        }
        if (sPathOld.xPut!=-1) gxVirtualDisplay(&vhptr,sPathOld.xPut,sPathOld.yPut,sPathOld.xPut,sPathOld.yPut,sPathOld.xPut+sImgOld.w-1,sPathOld.yPut+sImgOld.h-1,0);
        gxPutImage(&SP2,gxSET,sPath.xPut,sPath.yPut,0);
      }
      else {
        xx=hSPOT.size;
        for (i=0;i<xx;i++) {
           if (SP1.vptr[i]==0) SP1.vptr[i]=SP3.vptr[i];
        }
        if (sPathOld.xPut!=-1)
            gxVirtualDisplay(&vhptr, sPathOld.xPut, sPathOld.yPut, sPathOld.xPut, sPathOld.yPut,
                    minn(sPathOld.xPut+sImgOld.w-1, 319),
                    minn(sPathOld.yPut+sImgOld.h-1, 199),
                    0);
        gxPutImage(&SP1,gxSET,
                minn(sPath.xPut, 319),
                minn(sPath.yPut, 199), 0);
      }
      sPathOld=sPath;
      sImgOld=sImg;

      DV(&SP3);DV(&SP1);
      if (sPath.Scale!=1.0) DV(&SP2);
      sCount--;

   }
   else if (mode==SPOT_STEP && sPath.iHold>1 && sCount>0) sPath.iHold--;
   else if (mode==SPOT_STEP && sPath.iHold==1 && sCount==0) {
      SpotCrap(0,SPOT_DONE);
   }
   else if ((mode==SPOT_DONE || sCount>=0) && sFin!=NULL) {  // Close damn thing down
       fclose(sFin);sFin=NULL;
       sPathOld.xPut=-1;
       sPath.iHold=0;
       sCount=-1;
     #if BABYSND
      if (turnoff==1) {StopAudio(0);turnoff=0;}
     #endif
   }
   else if (mode==SPOT_KILL && sFin!=NULL)
    {
     fclose(sFin);sFin=NULL;
   #if BABYSND
     if (turnoff==1) {StopAudio(0);turnoff=0;}
   #endif
    }
  #if BABYSND
   if ((loc>=0 && loc<=8) || (loc>=15 && loc<=19) || loc==12 || loc==14 || loc==11 || loc==10)
    if (mode==SPOT_LOAD && !IsChannelMute(AV_SOUND_CHANNEL))
     {
      switch(loc) {
       case 1:case 6:PlayAudio("jet.ogg",0);break;
       case 3:case 8:PlayAudio("vcrash.ogg",0);break;
       case 16:PlayAudio("train.ogg",0);break;
       case 4:PlayAudio("crawler.ogg",0);break;
       case 0:case 5:PlayAudio("vthrust.ogg",0);break;
       case 10:PlayAudio("gate.ogg",0);break;
       case 18:PlayAudio("svprops.ogg",0);break;
       case 2:case 7:PlayAudio("heli_00.ogg",0);break;
       case 17:PlayAudio("radarsv.ogg",0);break;
       case 11:PlayAudio("radarus.ogg",0);break;
       case 12:case 14:PlayAudio("lightng.ogg",0);break;
       case 19:PlayAudio("crane.ogg",0);break;
       case 15:PlayAudio("truck.ogg",0);break;
       default:break;
      };
      turnoff=1;
     };
  #endif
 return;
}
   
void WaveFlagSetup(void)
{
  long j;
  FILE *fin;
  GV(&flaggy,230,22);
  fin=sOpen("FLAG.SEQ","rb",0);
  j=fread(vhptr.vptr,1,vhptr.h*vhptr.w,fin);
  fclose(fin);
  RLED_img(vhptr.vptr,flaggy.vptr,j,flaggy.w,flaggy.h);
}

void WaveFlagDel(void)
{
  DV(&flaggy);
  return;                       
}

/* pace */
/**
 * \note this isn't needed now that RLED automatically chops the right column
 * when the data is bigger than needed.  there's still a bug somewhere,
 * so this table is a useful list of funny images to check later
 */
long fix_width[] = {
	// normal
	80615, // 4/0 VAB in mode 0
	101712, // 8/1 Medical center
	115262, // 16/1 L.M. Program
	113283, // 18/1 XMS-2 program
	112708, // 19/1 Apollo program
	112154, // 20/1 Gemini program
	86035, // 22/1 Research and development
	118301, // 33/1 Tracking station
	82541, // 4/2 vab
	88992, // 22/2 research and development
	92114, // 22/3 research and development

	// predraw 
	79703, // 15-1-1 "SATELLITE PROGRAMS" 
	0
};

int
need_to_fix_width (long table)
{
	int i;
	for (i = 0; fix_width[i]; i++) {
		if (fix_width[i] == table)
			return (1);
	}
	return (0);
}

void PortPlace(FILE * fin,long table)
{
  IMG Img;
  GXHEADER local,local2;
  int ctr;

  fseek(fin,table,SEEK_SET);
  fread(&Img,sizeof Img,1,fin);
	Swap32bit(Img.Size);
	Swap16bit(Img.Width);
	Swap16bit(Img.Height);
	Swap16bit(Img.PlaceX);
	Swap16bit(Img.PlaceY);

//  if (need_to_fix_width (table))
//	  Img.Width++;

  GV(&local,Img.Width,Img.Height);
  GV(&local2,Img.Width,Img.Height);
  gxGetImage(&local,Img.PlaceX,Img.PlaceY,Img.PlaceX+Img.Width-1,Img.PlaceY+Img.Height-1,0);
  fread(vhptr.vptr,Img.Size,1,fin);
  RLED_img(vhptr.vptr,local2.vptr,Img.Size,local2.w,local2.h);
  for (ctr=0;ctr<(Img.Width*Img.Height);ctr++)
    if (local2.vptr[ctr]!=0x00) local.vptr[ctr]=local2.vptr[ctr];
  gxPutImage(&local,gxSET,Img.PlaceX,Img.PlaceY,0);  // place image
  DV(&local2);DV(&local);
  return;
}

void PortPal(char plr)
{
  FILE *fin;
  fin=sOpen((plr==0)?"USA_PORT.DAT":"SOV_PORT.DAT","rb",0);
  fread(&PHead,sizeof PHead,1,fin);
	Swap32bit(PHead.oPal);
  fseek(fin,PHead.oPal,SEEK_SET);
  fread(&pal[0],768,1,fin);
  fclose(fin);
  return;
}


void DrawSpaceport(char plr)
{
  long table[S_QTY];
  int i,fm,idx;
  FILE *fin;
  GXHEADER local,local2;
  IMG Img;
  int k,j;

  fin=sOpen((plr==0) ?"USA_PORT.DAT":"SOV_PORT.DAT","rb",0);

  fread(&PHead,sizeof PHead,1,fin);
	Swap32bit(PHead.oMObj);
	Swap32bit(PHead.oTab);
	Swap32bit(PHead.oPal);
	Swap32bit(PHead.oPort);
	Swap32bit(PHead.oMse);
	Swap32bit(PHead.oOut);
	Swap32bit(PHead.oAnim);

  fread(&MObj[0],sizeof MObj,1,fin);

  // Endianness swap
	for (i = 0; i < (int)(sizeof(MObj)/sizeof(MOBJ)); i++)
	{
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 4; k++)
			{
				Swap16bit(MObj[i].Reg[j].CD[k].x1);
				Swap16bit(MObj[i].Reg[j].CD[k].x2);
				Swap16bit(MObj[i].Reg[j].CD[k].y1);
				Swap16bit(MObj[i].Reg[j].CD[k].y2);
			}
		}
	}
	
  fread(&table[0],sizeof table,1,fin);

  // Endianness swap
	for (i = 0; i< S_QTY; i++)
		Swap32bit(table[i]);

  fseek(fin,PHead.oPal,SEEK_SET);
  fread(&pal[0],768,1,fin);

  fseek(fin,table[0],SEEK_SET);
  fread(&Img,sizeof Img,1,fin);  // Read in main image Header
	Swap32bit(Img.Size);
	Swap16bit(Img.Width);
	Swap16bit(Img.Height);
	Swap16bit(Img.PlaceX);
	Swap16bit(Img.PlaceY);
  fread((char *)screen,Img.Size,1,fin);  // Read in main image
  av_need_update_xy(0, 0, MAX_X, MAX_Y);

  UpdatePortOverlays();

  if (xMODE & xMODE_CLOUDS) 
		PortPlace(fin,table[1]); // Clouds

  // Pads
   for (i=0;i<3;i++) {
      Data->P[plr].Port[PORT_LaunchPad_A+i]=1;
      if (Data->P[plr].Mission[i].MissionCode>0) 
				Data->P[plr].Port[PORT_LaunchPad_A+i]=2;
      else if (Data->P[plr].LaunchFacility[i]>1) 
				Data->P[plr].Port[PORT_LaunchPad_A+i]=3;
      else if (Data->P[plr].LaunchFacility[i]<0) // No launch facility
				Data->P[plr].Port[PORT_LaunchPad_A+i]=0;
      }

  if (Vab_Spot==1 && Data->P[plr].Port[PORT_VAB]==2)
   {
    Data->P[plr].Port[PORT_LaunchPad_A]=plr;
   }
  

  if (Data->P[plr].AstroCount>0) {
      PortPlace(fin,table[16-plr*4]); // Draw CPX
      HotKeyList[9]='T';
      HotKeyList[10]='B';
	} else {	// No manned program hotkeys
		HotKeyList[9]='\0';
		HotKeyList[10]='\0';
	}
  if (Data->P[plr].Pool[0].Active>=1) 
      PortPlace(fin,table[17-plr*4]); // Draw TRN

  if (Data->P[plr].Port[PORT_Research]>1) 
		PortPlace(fin,table[13+15*plr]);  // RD Stuff
  if (Data->P[plr].Port[PORT_Research]>2) 
		PortPlace(fin,table[14+15*plr]);
  if (Data->P[plr].Port[PORT_Research]==3) 
		PortPlace(fin,table[15+15*plr]);

  for (fm=0;fm<35;fm++) {
    idx=Data->P[plr].Port[fm];  // Current Port Level for MObj

    if (MObj[fm].Reg[idx].PreDraw>0)   // PreDrawn Shape
      PortPlace(fin,table[MObj[fm].Reg[idx].PreDraw]);

    if (MObj[fm].Reg[idx].iNum>0)   // Actual Shape
      PortPlace(fin,table[MObj[fm].Reg[idx].iNum]);
    }

  fclose(fin);

  ShBox(0,190,319,199);               // Base Box :: larger

  grSetColor(0);PrintAt(257,197,"CASH:");
  DispMB(285,197,Data->P[plr].Cash);
  grSetColor(11);PrintAt(256,196,"CASH:");
  DispMB(284,196,Data->P[plr].Cash);
 
  grSetColor(0);
  if (Data->Season==0) PrintAt(166,197,"SPRING 19");
  else PrintAt(166,197,"FALL 19");
  DispNum(0,0,Data->Year);

  grSetColor(11);
  if (Data->Season==0) PrintAt(165,196,"SPRING 19");
  else PrintAt(165,196,"FALL 19");
  DispNum(0,0,Data->Year);


   // FLAG DRAW
    FCtr=0;
    GV(&local,22,22);GV(&local2,22,22);

    if (plr==0) gxGetImage(&local,49,121,70,142,0);
    else gxGetImage(&local,220,141,241,162,0);

    if (plr==0) gxVirtualVirtual(&flaggy,FCtr*23,0,FCtr*23+21,21,&local2,0,0,gxSET);
    else gxVirtualVirtual(&flaggy,115+FCtr*23,0,115+FCtr*23+21,21,&local2,0,0,gxSET);
    for(i=0;i<(22*22);i++)
      if (local2.vptr[i]==0)
         local2.vptr[i]=local.vptr[i];

    if (plr==0) gxPutImage(&local2,gxSET,49,121,0);
    else gxPutImage(&local2,gxSET,220,141,0);

    DV(&local);DV(&local2);
}

void PortText(int x,int y,char *txt,char col)
{
   RectFill(1,192,160,198,3);
   grSetColor(0);PrintAt(x+1,y+1,txt);
   grSetColor(col);PrintAt(x,y,txt);
}


void UpdatePortOverlays(void)
{
   char i,j;
   for (i=0;i<NUM_PLAYERS;i++) {   // Programs
      for (j=0;j<5;j++) 
         Data->P[i].Port[PORT_Mercury-j]=(Data->P[i].Manned[j].Num>=0)?1:0;

#ifdef DEADCODE
      // Zond thingy -- this was never implemented and available after 6 manned seasons
      //if (i==1 && Data->P[i].Manned[2].Seas>6) Data->P[i].Port[PORT_Zond]=1;
#endif

      if(Data->P[i].Probe[0].Num>=0 || Data->P[i].Probe[1].Num>=0 ||
          Data->P[i].Probe[2].Num>=0 ) Data->P[i].Port[PORT_Satellite]=1;

      if (Data->P[i].Manned[5].Num>=0 || Data->P[i].Manned[6].Num>=0)
         Data->P[i].Port[PORT_LM]=1;

      // Museum
      if (Data->Prestige[18].Goal[i]>0)
         Data->P[i].Port[PORT_Museum]=maxx(Data->P[i].Port[PORT_Museum],1);   // Mus:1
      if (Data->Prestige[1].Goal[i]>0) 
         Data->P[i].Port[PORT_Museum]=maxx(Data->P[i].Port[PORT_Museum],2);   // Mus:2
      if (Data->Prestige[20].Goal[i]>0) 
         Data->P[i].Port[PORT_Museum]=maxx(Data->P[i].Port[PORT_Museum],3);   // Mus:3

      // R&D
      if (Data->P[i].Budget>=85)
         Data->P[i].Port[PORT_Research]=maxx(Data->P[i].Port[PORT_Research],1); // RD:1
      if (Data->P[i].Budget>=112)
         Data->P[i].Port[PORT_Research]=maxx(Data->P[i].Port[PORT_Research],2); // RD:2
      if (Data->P[i].Budget>=150)
         Data->P[i].Port[PORT_Research]=maxx(Data->P[i].Port[PORT_Research],3); // RD:3

      // VAB

      if (Data->Prestige[12].Goal[i]>0)
         Data->P[i].Port[PORT_VAB]=maxx(Data->P[i].Port[PORT_VAB],1);   // VAB:1

       if (Data->P[i].Budget>115)
         Data->P[i].Port[PORT_VAB]=maxx(Data->P[i].Port[PORT_VAB],2);   // VAB:2

      // Admin
      if (Data->P[i].AstroLevel>=2)
         Data->P[i].Port[PORT_Admin]=maxx(Data->P[i].Port[PORT_Admin],1);   // Adm:1
      if (Data->P[i].AstroLevel>=4)
         Data->P[i].Port[PORT_Admin]=maxx(Data->P[i].Port[PORT_Admin],2);   // Adm:2

      if (Data->Prestige[13].Goal[i]>0)
         Data->P[i].Port[PORT_Tracking]=maxx(Data->P[i].Port[PORT_Tracking],1); // Trk:1
      if (Data->Prestige[19].Goal[i]>0)
         Data->P[i].Port[PORT_MissionControl]=maxx(Data->P[i].Port[PORT_MissionControl],1); // MC:1

      if (Data->P[i].AstroCount>0)
         Data->P[i].Port[PORT_AstroComplex]=Data->P[i].Port[PORT_BasicTraining]=1;

      if (Data->P[i].Pool[0].Active>0) {  // Astros
         Data->P[i].Port[PORT_Helipad]=Data->P[i].Port[PORT_Pool]=Data->P[i].Port[PORT_Planetarium]=1;
         Data->P[i].Port[PORT_Centrifuge]=Data->P[i].Port[PORT_MedicalCtr]=Data->P[i].Port[PORT_Airfield]=1;
         }
      }
}

void Master(char plr)
{
  int i,r_value,t_value=0,g_value=0;
  sFin=NULL;
  strcpy(IDT,"i000");strcpy(IKEY,"i000");
  WaveFlagSetup();
  sCount=-1;
  SUSPEND=Vab_Spot=0;

  for (i=0;i<3;i++) {
     GetMisType(Data->P[plr].Mission[i].MissionCode);
     Data->P[plr].Mission[i].Joint=Mis.Jt;
  }

	// Entering screen for the first time so fade out and in.
  FadeOut(2,pal,10,0,0);
	DrawSpaceport(plr);
  FadeIn(2,pal,10,0,0);

  memcpy(vhptr.vptr,screen,MAX_X*MAX_Y);
  av_need_update_xy(0, 0, MAX_X, MAX_Y);

#if SPOT_ON
  if ((Data->P[plr].Pool[0].Active|Data->P[plr].Pool[1].Active|Data->P[plr].Pool[2].Active)>=1)
    g_value=1;
  for(i=0;i<Data->P[plr].AstroCount;i++)
    if (Data->P[plr].Pool[i].Status>=AST_ST_TRAIN_BASIC_2) t_value=1;
  r_value=random(1000);
  if (xMODE & xMODE_CLOUDS)
   {
    if (plr==0 && Data->P[plr].Port[PORT_VAB]==0) SpotCrap(14,SPOT_LOAD);          //USA Storm 
     else if (plr==1) SpotCrap(12,SPOT_LOAD);    //Sov Storm
   }
  else 
    if ((xMODE & xMODE_SPOT_ANIM) && g_value)
    {
     SpotCrap(3+(5*plr),SPOT_LOAD);
     xMODE &= ~xMODE_SPOT_ANIM;
    }
   else if (t_value && g_value) SpotCrap(0+(5*plr),SPOT_LOAD);  //LEM
    else if (r_value<150) {
     if (plr==1 && Data->P[plr].Port[PORT_MedicalCtr]==1) SpotCrap(18,SPOT_LOAD);   
      else SpotCrap(1+(5*plr),SPOT_LOAD);
    }
    else if (r_value>850) SpotCrap(2+(5*plr),SPOT_LOAD);       //Heli
#endif
  
  Port(plr);
  strcpy(IDT,"i000");strcpy(IKEY,"i000");
  WaveFlagDel();
  if (sFin) {fclose(sFin);sFin=NULL;}
}

void GetMse(char plr,char fon)
{
  int i;
  GXHEADER local,local2;
  static double last_wave_step;
  double now;

  now = get_time ();
  if (now - last_wave_step > .125) {
	  last_wave_step = now;

	  if (plr == 0)
		  DoCycle ();

	  if (fon != 1)
		  goto done;
#if SPOT_ON
    SpotCrap(0,SPOT_STEP);
#endif
    FCtr=FCtr%5;
    GV(&local,22,22);GV(&local2,22,22);

    if (plr==0) gxGetImage(&local,49,121,70,142,0);
    else gxGetImage(&local,220,141,241,162,0);
    if (plr==0)
      gxVirtualVirtual(&flaggy,FCtr*23,0,FCtr*23+21,21,&local2,0,0,gxSET);
    else
      gxVirtualVirtual(&flaggy,115+FCtr*23,0,115+FCtr*23+21,21,&local2,0,0,gxSET);
    for(i=0;i<(22*22);i++)
      if (local2.vptr[i]==0)
         local2.vptr[i]=local.vptr[i];

    if (plr==0) gxPutImage(&local2,gxSET,49,121,0);
    else gxPutImage(&local2,gxSET,220,141,0);

    DV(&local);DV(&local2);

  done:
    FCtr++;
  }
  GetMouse_fast();
}

void
DoCycle(void)					// Three ranges of color cycling
{
	int i, tmp1, tmp2, tmp3, j;
    /* hardcoded rectangles with water */
    struct rectangle {
        int x1, y1, x2, y2;
    } r[] = {
        {141, 163, 156, 175},
        {0, 50, 71, 72},
        {168, 16, 294, 32},
        {109, 11, 170, 39},
        {116,114, 157,145}
    };

	j = 384;

	i = 0;
	tmp1 = pal[j + 3 * i + 0];
	tmp2 = pal[j + 3 * i + 1];
	tmp3 = pal[j + 3 * i + 2];
	for (; i < 3; i++)
	{
		pal[j + i * 3 + 0] = pal[j + (i + 1) * 3 + 0];
		pal[j + i * 3 + 1] = pal[j + (i + 1) * 3 + 1];
		pal[j + i * 3 + 2] = pal[j + (i + 1) * 3 + 2];
	};
	pal[j + 3 * i] = tmp1;
	pal[j + 3 * i + 1] = tmp2;
	pal[j + 3 * i + 2] = tmp3;

	i = 4;
	tmp1 = pal[j + 3 * i + 0];
	tmp2 = pal[j + 3 * i + 1];
	tmp3 = pal[j + 3 * i + 2];
	for (; i < 11; i++)
	{
		pal[j + i * 3 + 0] = pal[j + (i + 1) * 3 + 0];
		pal[j + i * 3 + 1] = pal[j + (i + 1) * 3 + 1];
		pal[j + i * 3 + 2] = pal[j + (i + 1) * 3 + 2];
	};
	pal[j + 3 * i] = tmp1;
	pal[j + 3 * i + 1] = tmp2;
	pal[j + 3 * i + 2] = tmp3;

	i = 12;
	tmp1 = pal[j + 3 * i + 0];
	tmp2 = pal[j + 3 * i + 1];
	tmp3 = pal[j + 3 * i + 2];
	for (; i < 15; i++)
	{
		pal[j + i * 3 + 0] = pal[j + (i + 1) * 3 + 0];
		pal[j + i * 3 + 1] = pal[j + (i + 1) * 3 + 1];
		pal[j + i * 3 + 2] = pal[j + (i + 1) * 3 + 2];
	};
	pal[j + 3 * i] = tmp1;
	pal[j + 3 * i + 1] = tmp2;
	pal[j + 3 * i + 2] = tmp3;

	gxSetDisplayPalette(pal);

    for (i = 0; i < (int) ARRAY_LENGTH(r); ++i)
    {
        av_need_update_xy(r[i].x1, r[i].y1, r[i].x2, r[i].y2);
    }
}

/** ???
 * 
 * \param mode ...  0 = ?   1 = copy stored outline ?
 */
void
PortOutLine(unsigned int Count, ui16 * outline, char mode)
{
	int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
	unsigned int i;

	pPortOutlineRestore = xrealloc(pPortOutlineRestore,
		sizeof(PORTOUTLINE) * Count);

	for (i = 0; i < Count; i++)
	{
		if (mode == 1)
		{						   // Save value from the screen 
			pPortOutlineRestore[i].loc = outline[i];	// Offset of the outline into the buffer
			pPortOutlineRestore[i].val = screen[outline[i]];	// Save original pixel value
		}
		else					   // dunno
			outline[i] = pPortOutlineRestore[i].loc;
		screen[outline[i]] = 11;   // Color the outline index 11, which should be Yellow
		min_x = min(min_x, outline[i] % MAX_X);
		min_y = min(min_y, outline[i] / MAX_X);
		max_x = max(max_x, outline[i] % MAX_X);
		max_y = max(max_y, outline[i] / MAX_X);
	}
	if (Count)
		av_need_update_xy(min_x, min_y, max_x, max_y);
}

void
PortRestore(unsigned int Count)
{
	int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
	unsigned int i;
    int loc;

	for (i = 0; i < Count; i++)
	{
        loc = pPortOutlineRestore[i].loc;
		screen[loc] = pPortOutlineRestore[i].val;
		min_x = min(min_x, loc % MAX_X);
		min_y = min(min_y, loc / MAX_X);
		max_x = max(max_x, loc % MAX_X);
		max_y = max(max_y, loc / MAX_X);
	}
	if (Count)
		av_need_update_xy(min_x, min_y, max_x, max_y);
	free(pPortOutlineRestore);
	pPortOutlineRestore = NULL;
}


/** Map a keypress to a spaceport building selection
 */
int MapKey(char plr,int key,int old) 
{
  int val,j,found=0;
  char high=-1,low=-1;
  for (j=0;j<35;j++)
   {
    if (MObj[j].Reg[Data->P[plr].Port[j]].sNum>0)
     {
      if (low==-1) low=j;
      high=j;
     }
   }

  val=old;
  switch(key) {
    case 'A': if (MObj[6].Reg[Data->P[plr].Port[PORT_Admin]].sNum>0) val=6;mousebuttons=1; break;
    case 'I': if (MObj[1].Reg[Data->P[plr].Port[PORT_Pentagon]].sNum>0) val=1;mousebuttons=1; break;
    case 'M': if (MObj[5].Reg[Data->P[plr].Port[PORT_Museum]].sNum>0) val=5;mousebuttons=1; break;
    case 'R': if (MObj[22].Reg[Data->P[plr].Port[PORT_Research]].sNum>0) val=22;mousebuttons=1; break;
    case 'P': if (MObj[2].Reg[Data->P[plr].Port[PORT_Capitol]].sNum>0) val=2;mousebuttons=1; break;
    case 'V': if (MObj[4].Reg[Data->P[plr].Port[PORT_VAB]].sNum>0) val=4;mousebuttons=1; break;
    case 'C': if (MObj[26].Reg[Data->P[plr].Port[PORT_MissionControl]].sNum>0) val=26;mousebuttons=1; break;
    case 'Q': if (MObj[29].Reg[Data->P[plr].Port[PORT_Gate]].sNum>0) val=29;mousebuttons=1; break;
    case 'E': if (MObj[28].Reg[Data->P[plr].Port[PORT_FlagPole]].sNum>0) val=28;mousebuttons=1; break;
    case 'T': if (MObj[7].Reg[Data->P[plr].Port[PORT_AstroComplex]].sNum>0) val=7;mousebuttons=1; break;
    case 'B': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;

#if 0
		// Possibly hotkeys for astronaut training buildings
    case '1': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;
    case '2': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;
    case '3': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;
    case '4': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;
    case 'S': if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum>0) val=9;mousebuttons=1; break;
#endif
    case UP_ARROW:if (old==high) old=0; else old=old+1;
                  found=0;for (j=old;j<high+1;j++)
                   if (MObj[j].Reg[Data->P[plr].Port[j]].sNum>0) {
                     if (found==0) {val=j;found=1;}
                     }
                  break;
    case DN_ARROW:if (old==low) old=34; else old=old-1;
                  found=0;for (j=old;j>low-1;j--)
                   if (MObj[j].Reg[Data->P[plr].Port[j]].sNum>0) {
                     if (found==0) {val=j;found=1;}
                     }
                  break;
    default: val=old;break;
    }

  return val;
}

void Port(char plr)
{
double last_secs;
int i,j,kMode,kEnt,k;
char good, res;
int kPad,pKey,gork;
FILE *fin;
long stable[55];
ui16 Count,*bone;

  strcpy(IDT,"i043");strcpy(IKEY,"k043");
  bone=(ui16 *) buffer;

  fin=sOpen((plr==0)?"USA_PORT.DAT":"SOV_PORT.DAT","rb",0);
  fread(&PHead,sizeof PHead,1,fin);
	Swap32bit(PHead.oMObj);
	Swap32bit(PHead.oTab);
	Swap32bit(PHead.oPal);
	Swap32bit(PHead.oPort);
	Swap32bit(PHead.oMse);
	Swap32bit(PHead.oOut);
	Swap32bit(PHead.oAnim);
  fseek(fin,PHead.oOut,SEEK_SET);
  fread(&stable[0],sizeof stable,1,fin);
	for (i=0; i< 55; i++) {
		Swap32bit(stable[i]);
	}
  if (plr==0 && Data->Year>65) PortText(5,196,"CAPE KENNEDY",12);
  else if (plr==0) PortText(5,196,"THE CAPE",12);
  else PortText(5,196,"BAIKONUR",12);

  pKey=0;
  
  music_start((plr==0)?M_USPORT:M_SVPORT);
  kMode=kPad=kEnt=0;
  last_secs = get_time ();
	i = 0; // this is used to loop through all the selection regions on the port
  while (1)
   {
	   av_block ();
    #if 0
      if (get_time ()- last_secs > 25)   
      {
       SpotCrap(0,SPOT_KILL);
       gork=random(100);
       if (gork<50)
        {
         if (plr==1 && Data->P[plr].Port[PORT_MedicalCtr]==1)
          SpotCrap(18,SPOT_LOAD);
           else SpotCrap(1+(5*plr),SPOT_LOAD);
        }
       else if (plr==1 && gork<90) SpotCrap(10,SPOT_LOAD);
        else if (plr==0 && gork<90) SpotCrap(2+(5*plr),SPOT_LOAD);
         else SpotCrap(1+(5*plr),SPOT_LOAD); //Specs: Default
       last_secs = get_time ();
      }
    #endif
			if (kMode==0)
				i=0;
			else if (kMode==1) 
				kEnt=0;

    do
     {
      #if BABYSND
       UpdateAudio();
      #endif  
      if (pKey==0) {key=0;GetMse(plr,1);}
      else pKey=0;
      if (kPad>0 && kMode==1) key=kPad;
      if (kMode==1 && !(x==319 && y==199)) kMode=0;
      if (key>0) // this was only looking for the low byte
       {
        i=MapKey(plr,key,i);	// Get Port offset for keyboard input
        if (key==K_ESCAPE) {kMode=0;i=0;}
         else {kMode=1;}
        if (MObj[i].Reg[Data->P[plr].Port[i]].sNum>0)
         {
          x=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
          y=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
          kEnt=i;
         }
       }
      if (kMode==1 && kEnt==i)
       {
        x=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
        y=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
       }
      else if (kMode==1 && kEnt!=i)
       {
        x=-1;y=-1;
       }

			for (j=0;j<MObj[(kMode==0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode==0) ? i : kEnt]].qty;j++) 
         if (x>=MObj[(kMode==0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode==0) ? i : kEnt]].CD[j].x1 &&
          y>=MObj[(kMode==0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode==0) ? i : kEnt]].CD[j].y1 &&
          x<=MObj[(kMode==0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode==0) ? i : kEnt]].CD[j].x2 &&
          y<=MObj[(kMode==0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode==0) ? i : kEnt]].CD[j].y2)
         {
          PortText(5,196,MObj[i].Name,11);
          if (MObj[i].Reg[Data->P[plr].Port[i]].sNum>0) {
               fseek(fin,stable[MObj[i].Reg[Data->P[plr].Port[i]].sNum],SEEK_SET);
               fread_uint16_t(&Count,1,fin);
               fread_uint16_t(bone,Count,fin);
               PortOutLine(Count,bone,1);
               strncpy(&IDT[1],MObj[i].Help,3);
              }
          good=0;
					// Search hotkey string for valid selection
          for (k=0;k<(int)strlen(HotKeyList);k++)
           if (HotKeyList[k]==((char)(0x00ff&key))) good=1;

          while (x>=MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x1 &&
             y>=MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y1 &&
             x<=MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x2 &&
             y<=MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y2)
            {
		    av_block ();
          #if BABYSND
           UpdateAudio();
          #endif
             kPad=0;key=0;
             GetMse(plr,1);
              if (kMode==0 && key>0) {x=y=0;pKey=key;};

              if (kMode==1 && !(x==319 && y==199)) {kMode=0;}
               if (kMode==1)
                {
                 x=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                 y=MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
                }

               if (key>0 && kMode==1)  // got a keypress
                if (key!=K_ENTER) // not return
                 {
                  x=-1;y=-1;kPad=key;
                 }

               if (good==1 || (kMode==0 && mousebuttons==1) || (kMode==1 && key==K_ENTER) 
                  || (kMode==0 && key==K_ENTER))
                {
                 PortRestore(Count);Count=0;

                  // || i==33

                  if (!(i==28 || i==29 || i==0 || i==31 
                       || (Data->Year==57 || (Data->Year==58 && Data->Season==0))))
                     {
#if SPOT_ON
                       SpotCrap(0,SPOT_KILL);  // remove spots
#endif
                       music_stop();
                    }
                  else SUSPEND=1;

                  res = PortSel(plr, i);
                  switch(res) {
	                  case pNOREDRAW:
#ifdef DEAD_CODE
                          if (!(i==28 || i==29 || i==0 || i==31 || i==33 
                            || (Data->Year==57 || (Data->Year==58 && Data->Season==0)))) {
                            PreLoadMusic((plr==0)?M_USPORT:M_SVPORT);
#if SPOT_ON
                          memcpy(screen,vhptr.vptr,MAX_X*MAX_Y);
                          av_need_update_xy(0, 0, MAX_X, MAX_Y);
#endif
                          }
#endif

                          PortText(5,196,MObj[i].Name,11);
#ifdef DEAD_CODE
                          if (!(i==28 || i==29 || i==0 || i==31 || i==33 
                             || (Data->Year==57 || (Data->Year==58 && Data->Season==0))))
                            PlayMusic(0);
#endif
	                        break;
	                  case pREDRAW:
	                  case pNEWMUSIC:
                      case pNOFADE:
                          if (res != pNOFADE)
                          {
															music_stop();
                              music_start((plr==0)?M_USPORT:M_SVPORT);
                          }

                          SpotCrap(0,SPOT_KILL);  // remove spots
													// Returning to spaceport so fade between redraws
						if (res == pREDRAW)
                            FadeOut(2,pal,10,0,0);

						DrawSpaceport(plr);

						if (res == pREDRAW)
                            FadeIn(2,pal,10,0,0);

#if SPOT_ON
                          memcpy(vhptr.vptr,screen,MAX_X*MAX_Y);
                          gork=random(100);
                          if (Vab_Spot==1 && Data->P[plr].Port[PORT_VAB]==2) 
                           {
                            Data->P[plr].Port[PORT_LaunchPad_A]=1;
                            if (plr==0) {
                               if (gork<=60) SpotCrap(4,SPOT_LOAD); //Rocket to Pad
                                else SpotCrap(15,SPOT_LOAD); //Rocket&Truck/Door
                              }
                             else if (plr==1) SpotCrap(16,SPOT_LOAD);
                           }
                           else if (Vab_Spot==4 && plr==0 && Data->P[plr].Port[PORT_VAB]==0)
                            {
                             SpotCrap(19,SPOT_LOAD);
                            }
                           else if (Vab_Spot==2 && plr==1)
                            {
                             SpotCrap(10,SPOT_LOAD);
                            }
                           else if (Vab_Spot==3)
                            {
                             if (plr==1) SpotCrap(17,SPOT_LOAD);
                              else if (plr==0) SpotCrap(11,SPOT_LOAD);
                            }
                           else if (gork<30) {
                            if (plr==1 && Data->P[plr].Port[PORT_MedicalCtr]==1)
                             SpotCrap(18,SPOT_LOAD);
                              else SpotCrap(1+(5*plr),SPOT_LOAD);
                            }
                           else if (plr==1 && gork<40) SpotCrap(10,SPOT_LOAD);
                            else if (gork<60) SpotCrap(2+(5*plr),SPOT_LOAD);
#endif              
                          Vab_Spot=0;
#ifdef DEADCODE
											// I'm not sure why we're redrawing the outlines here, 
											//commenting it out for now.  if no problems are seen 
											// with the port outlines then restore this
                       //   if (pPortOutlineRestore) 
												//		PortOutLine(Count,bone,0);
#endif
                          PortText(5,196,MObj[i].Name,11);
                     	  break;
               	   case pEXIT:
                          FadeOut(2,pal,10,0,0);
                          fclose(fin);
                        #if BABYSND
                          if (i==28 || i==29) SUSPEND=0;
                        #endif
                          SpotCrap(0,SPOT_KILL);  // remove spots
                          music_stop();
			  save_game ("AUTOSAVE.SAV");
                          return;
	                  case pQUIT:
               			  FadeOut(2,pal,10,0,0);
                          fclose(fin);
                         #if BABYSND
                           if (i==28 || i==29) SUSPEND=0;
                         #endif
                          SpotCrap(0,SPOT_KILL);  // remove spots
                          music_stop();
               			  return;
	                  } // switch
                  kMode=good=SUSPEND=0;
                 if (MObj[i].Reg[Data->P[plr].Port[i]].sNum>0) {
                    fseek(fin,stable[MObj[i].Reg[Data->P[plr].Port[i]].sNum],SEEK_SET);
                    fread_uint16_t(&Count,1,fin);
                    fread_uint16_t(bone,Count,fin);
                    //pPortOutlineRestore = (PORTOUTLINE *) malloc((sizeof (PORTOUTLINE))*Count);
                    PortOutLine(Count,bone,1);
                    }
	               while(mousebuttons==1) GetMse(plr,1);
                  } // if
                } //while
               if (plr==0 && Data->Year>65) PortText(5,196,"CAPE KENNEDY",12);
               else if (plr==0) PortText(5,196,"THE CAPE",12);
               else PortText(5,196,"BAIKONUR",12);
               PortRestore(Count);Count=0;
               strcpy(IDT,"i043");strcpy(IKEY,"k043");
          } // if
      if (kMode==0 && XMAS==1) i++;if (kMode==1) kEnt++;
   } while ((kMode==0 && i<35 && i>=0) || (kMode==1 && kEnt<35 && kEnt>=0));
  } // while
}


/** This is the code that controls the jumpoff point from the Spaceports to the
 * various areas.  It basically assigns a help message, then makes a call into
 * the module which would have its own event loop
 */
char PortSel(char plr,char loc)
{
  int i,MisOK=0;
  Vab_Spot=0;  // clear the damn thing.

  switch(loc) {
   case PORT_Monument:  Help((plr==0)?"i023":"i022");
            strcpy(IKEY,"k022");
            return pNOREDRAW; // Monuments
    case PORT_Pentagon: 
        if (Data->Year==57 || (Data->Year==58 && Data->Season==0)) {
           Help("i108");
           return pNOREDRAW;
        }
        strcpy(IDT,"i027\0");
        Intel(plr); return pNEWMUSIC;
    case PORT_Capitol: strcpy(IDT,(plr==0)?"i021":"i532");strcpy(IKEY,(plr==0)?"k021":"k532");
            Review(plr); return pREDRAW;
    case PORT_Cemetery: strcpy(IDT,"i020");Hospital(plr,1); return pREDRAW;
    case PORT_VAB: if (Option!=-1) {put_serial(LET_V);put_serial(LET_V);put_serial(LET_V);}
            strcpy(IDT,"i015");MisOK=0;VAB(plr); return pREDRAW;
    case PORT_Museum: if (Option!=-1) {put_serial(LET_M);put_serial(LET_M);put_serial(LET_M);}
            strcpy(IDT,"i027");Museum(plr); return pNEWMUSIC;
    case PORT_Admin: if (Option!=-1) {put_serial(LET_A);put_serial(LET_A);put_serial(LET_A);}
            strcpy(IDT,"i027");Admin(plr);
            if (LOAD==1) return pEXIT;
            else if (QUIT==1) return pQUIT;
            else {if (plr==0) Vab_Spot=4;return pNEWMUSIC;}
    case PORT_AstroComplex: strcpy(IDT,"i039");Limbo(plr);return pREDRAW; // Astro Complex
    case PORT_MedicalCtr: strcpy(IDT,"i041");Hospital(plr,0); return pREDRAW;
    case PORT_BasicTraining: strcpy(IDT,"i038");Train(plr,0);return pREDRAW;
    case PORT_Helipad: strcpy(IDT,"i037");Train(plr,2);return pREDRAW;
    case PORT_Pool: strcpy(IDT,"i037");Train(plr,3);return pREDRAW;
    case PORT_Planetarium: strcpy(IDT,"i037");Train(plr,4);return pREDRAW;
    case PORT_Centrifuge: strcpy(IDT,"i037");Train(plr,5);return pREDRAW;
    case PORT_Airfield: strcpy(IDT,"i037");Train(plr,1);return pREDRAW;
    case PORT_Satellite: strcpy(IDT,"i019");SatBld(plr);return pREDRAW;
    case PORT_LM: strcpy(IDT,"i044");strcpy(IKEY,"k209");LMBld(plr);return pREDRAW; // LM Program
    case PORT_Jupiter: strcpy(IDT,"i036");Programs(plr,5);return pREDRAW;
    case PORT_XMS: strcpy(IDT,"i036");Programs(plr,4);return pREDRAW;
    case PORT_Apollo: strcpy(IDT,"i036");Programs(plr,3);return pREDRAW;
    case PORT_Gemini: strcpy(IDT,"i036");Programs(plr,2);return pREDRAW;
    case PORT_Mercury: strcpy(IDT,"i036");Programs(plr,1);return pREDRAW;
    case PORT_Research: if (Option!=-1) {put_serial(LET_R);put_serial(LET_R);put_serial(LET_R);}
             strcpy(IDT,"i009");RD(plr);if (plr==1) Vab_Spot=2;return pREDRAW;
    case PORT_LaunchPad_A:
    case PORT_LaunchPad_B:
    case PORT_LaunchPad_C: strcpy(IDT,"i028");ShowPad(plr,loc-23);return pREDRAW;
    case PORT_MissionControl: strcpy(IDT,"i018");strcpy(IKEY,"k018");
             MisOK=0;
             for (i=0;i<3;i++) {
               if (Data->P[plr].Mission[i].MissionCode>0 && 
							 		 Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster]==0) 
								 MisOK=10;
               if (Data->P[plr].Mission[i].MissionCode>0) MisOK++;
               }
             if (MisOK>=10) Help("i005");
             else if (MisOK>0) {
               Rush(plr);RUSH=1;
               return pREDRAW;
               }
             else  Help("i104");
             return pNOFADE;

    case PORT_ViewingStand: strcpy(IDT,"i017");strcpy(IKEY,"k017");Viewing(plr);return pREDRAW;
    case PORT_FlagPole: // Flag Pole : End turn
             MisOK=0;
						// Check to see if missions are good to go
             for (i=0;i<3;i++) {
               if (Data->P[plr].Mission[i].MissionCode>0 &&
                   Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster]==0) 
									MisOK=10;
               if (Data->P[plr].Mission[i].MissionCode>0) MisOK++;
              }    

            if (MisOK>=10) {Help("i005");return pNOREDRAW;}
             else if (MisOK==0) {
		         MisOK=0;
		         i=Request(plr,"END TURN",1);
               if (i) return pEXIT;
		         }
             else if (RUSH==1 && MisOK>0 && MisOK<10) {
               MisOK=Help("i004");             // Mission Control
	             if (MisOK>=0) {
                if (Option!=-1) {put_serial(LET_O);put_serial(LET_O);put_serial(LET_O);}
                RUSH=0;return pEXIT;
               };
              }
             else if (RUSH==0) {
               Help("i103");
              }
            return pNOREDRAW;
    case PORT_Gate: // Security Gate : Quit
            QUIT=Request(plr,"QUIT",1);
	          if(QUIT)
             {
              if (Option!=-1) DoModem(2);
              return pQUIT;
             }
	          return pNOREDRAW; 
    case PORT_Moon: strcpy(IDT,"i029");Moon(plr);return pREDRAW; // Moon
    case PORT_SovMonumentAlt: Help("i025");return pNOREDRAW; // Sov Mon #2
    case PORT_Zond: strcpy(IDT,"i036");Programs(plr,3);return pREDRAW; // Zond
    case PORT_Tracking: if (Option!=-1) {MesCenter();Vab_Spot=3;return pREDRAW;} // Tracking
              else {Help("i042");Vab_Spot=3;return pNOFADE;}
    case PORT_SVHQ: return pNOREDRAW; // SV
    default: return pNOREDRAW;
  }
}


char Request(char plr,char *s,char md)
{
  char i;
  GXHEADER local;

  if (md>0) {  // Save Buffer
    GV(&local,196,84);
    gxGetImage(&local,85,52,280,135,0);
  }
  i=strlen(s)>>1;
  grSetColor(0);
  ShBox(85,52,249,135);
  IOBox(170,103,243,130);
  IOBox(91,103,164,130);
  InBox(92,58,243,97);
  grSetColor(1);
  DispBig(111,110,"YES",0,0);
  DispBig(194,110,"NO",0,0);

  grSetColor(11);
  if (md==6)  PrintAt(166-i*7,65,s);
  else DispBig(166-i*10,65,&s[0],0,-1);
  PrintAt(135,94,"ARE YOU SURE?");

  while(1){
     if (md!=6) GetMse(plr,1);
     else GetMouse();
     if (mousebuttons==0) break;
     }
  i=2;
  while(i==2) {
    if (md!=6) GetMse(plr,1);
    else GetMouse();
     if ((x>=172 && y>=105 && x<=241 && y<=128 && mousebuttons!=0)||(key=='N')) {
       InBox(172,105,241,128);i=0;
       delay(50);key=0;
     };
     if ((x>93 && y>=105 && x<=162 && y<=128&& mousebuttons!=0)||(key=='Y')) {
       InBox(93,105,162,128);i=1;
       delay(50);key=0;
     };
     delay(50);
  }; /* End while */

  if (md>0) {
    gxPutImage(&local,gxSET,85,52,0);
    DV(&local);
  }
  return i;
}

char MisReq(char plr)
{
  int i,num=0;
  GXHEADER local;

  GV(&local,184,132);
  gxGetImage(&local,53,29,236,160,0);

  for (i=0;i<3;i++)
    if ((Data->P[plr].Mission[i].MissionCode>0)&&
	(Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster]==0)) num++;
  ShBox(53,29,236,160);
  ShBox(60,34,229,44);
  InBox(60,47,229,120);
  // RectFill(61,48,228,119,0);
  ShBox(63,50,226,117);
  if (num==0) {
    IOBox(60,141,141,155);
    IOBox(148,141,229,155);
    grSetColor(1);
    PrintAt(70,129,"CONFIRM LAUNCH SCHEDULE OR");
    PrintAt(80,136,"CHOOSE TO REVIEW IT.");
    grSetColor(8);PrintAt(85,150,"C");grSetColor(1);PrintAt(0,0,"ONFIRM");
    grSetColor(8);PrintAt(179,150,"R");grSetColor(1);PrintAt(0,0,"EVIEW");
  } else {
    IOBox(60,141,229,155);
    grSetColor(1);
    PrintAt(62,129,"MISSIONS DO NOT ALL HAVE");
    PrintAt(62,136,"ASSIGNMENTS. NO COMMIT POSSIBLE");
    PrintAt(120,150,"REVIEW MISSIONS");
  };

  grSetColor(10);
  PrintAt(94,41,"LAUNCH CONFIRMATION");
  grSetColor(1);
  for (i=0;i<3;i++) {
    PrintAt(68,59+20*i,"PAD ");DispChr(0x41+i);PrintAt(0,0,": ");
    if (Data->P[plr].Mission[i].MissionCode!=0) {
      PrintAt(0,0,&Data->P[plr].Mission[i].Name[0]);
      if (Data->P[plr].Mission[i].Men>0) {
	PrintAt(86,65+20*i,"MANNED MISSION");
      } else {
	PrintAt(86,65+20*i,"UNMANNED MISSION");
      };

      if (Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster]==0) {
	grSetColor(9);
	PrintAt(86,71+20*i,"HARDWARE UNASSIGNED");
      } else {
	PrintAt(86,71+20*i,"HARDWARE ASSIGNED");
      };
      grSetColor(1);
    }
  };

  while(1)
    {if (plr==0) GetMse(plr,1);else GetMouse();if (mousebuttons==0) break;}
  i=2;
  while(i==2) {
   if (plr==0) GetMse(plr,1); else GetMouse();
     if ((x>=62 && y>=143 && x<=139 && y<=153 && mousebuttons!=0 && num==0)
	|| (key=='C' && num==0)) {
       InBox(62,143,139,153);i=1;
       delay(50);key=0;
       Rush(plr);
     };
     if ((x>150 && y>=143 && x<=227 && y<=153 && mousebuttons!=0 && num==0)
       || (key=='R' && num==0)) {
       InBox(150,143,227,153);i=0;
       delay(50);key=0;
     };
     if ((x>62 && y>=143 && x<=227 && y<=153 &&mousebuttons!=0 && num>0)
       || (key=='R' && num>0)) {
       InBox(62,143,227,153);i=0;
       delay(50);key=0;
     };
  }; /* End while */
  gxPutImage(&local,gxSET,53,29,0);
  DV(&local);
  return i;
}

// Editor settings {{{
// ex: ts=4 noet sw=2 
// ex: foldmethod=marker
// }}}




