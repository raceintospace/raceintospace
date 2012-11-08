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

#include <string>

/* XXX clean this up */
#include "mmfile.h"
#include "data.h"
// ***************************
//   USEFUL AND RARE DEFINES
// ***************************

#define MR grMoveRel
#define LR grLineRel
#define PP grPutPixel
#define LT grLineTo
#define MT grMoveTo
#define SC grSetColor
#define BUFFER_SIZE 20*1024   /**< Allocated Buffer in Beginning */
#define MINSAFETY 50          /**< Minimum Safety */
#define READYFACTOR 48        /**< Ready Safetyfactor */
#define MAXBUB 30             /**< Maximum Bubbles */
#define pline(a,b,c,d)        {grMoveTo(a,b) ; grLineTo(c,d);}
#define other(a)          abs( (a)-1 )
#define maxx(a,b)         (((a) > (b)) ? (a) : (b))
#define minn(a,b)         (((a) < (b)) ? (a) : (b))
#define MisStep(a,b,c) PrintAt((a),(b),S_Name[(c)])
//#define VBlank() while(!(inp(0x3da)&0x08))  // the all important VBlank
#define VBlank() do{}while(0)
#define PROGRAM_NOT_STARTED -1  /* Hardware program not started when Num is set to this */

#define isMile(a,b)   Data->Mile[a][b]

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif

#define xMODE_HIST 0x0001
#define xMODE_CHEAT 0x0002
#define xMODE_NOFAIL 0x0020
#define xMODE_TOM 0x0040
#define xMODE_DOANIM 0x0080
#define xMODE_NOCOPRO 0x00800
#define xMODE_PUSSY 0x0400
#define xMODE_CLOUDS 0x1000
#define xMODE_SPOT_ANIM 0x2000

#define HIST    (xMODE&xMODE_HIST)
#define CHEAT   (xMODE&xMODE_CHEAT)
#define NOFAIL  (xMODE&xMODE_NOFAIL)
#define TOM     (xMODE&xMODE_TOM)
#define DOANIM  (xMODE&xMODE_DOANIM)
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
#define K_QUIT         0x000F
#define K_ESCAPE       0x001B
#define K_SPACE     0x0020

#define ME_FB        2
#define VE_FB        3
#define MA_FB        4
#define JU_FB        5
#define SA_FB        6

#define MILE_OSAT    0
#define MILE_MAN     1
#define MILE_EORBIT  2
#define MILE_LFLY    3
#define MILE_LPL     4
#define MILE_LPASS   5
#define MILE_LORBIT  6
#define MILE_LLAND   7

#define MANSPACE     27
#define EORBIT       18
#define LPASS        19
#define LORBIT       20
#define LRESCUE      21
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

// ***************************
//   PROTOTYPES FOR ROUTINES
// ***************************

#ifdef DEAD_CODE
// PROTOCOL.C
void Read_CRC(void);
void SaveSide(char side);
void UpdPrestige();
void RecvSide(char side);
//static int timed_get(int n);
//static int build_block(int l, FILE *file);
//static void abort_transfer(void);
int xmit_file(int (*error_handler)(int c, int32_t p, char *s), char *files[]);
//static int getblock(int block, int crc);
int recv_file(int(*error_handler)(int c, int32_t p, char *s), char *path);

// SEL.C
void DFrame(int x1, int y1, int x2, int y2);
void SFrame(int x1, int y1, int x2, int y2);
void SFWin(int x1, int y1, int x2, int y2, char *txt);

// SOUND.C
void GetVoice(char val);
void UpdateVoice(void);
void NGetVoice(char plr, char val);
void NUpdateVoice(void);
char AnimSoundCheck(void);
void U_GetVoice(char val);
void U_UpdateVoice(void);
void KillVoice(void);
void FadeVoiceIn(int maxvolume, unsigned rate);
void FadeVoiceOut(unsigned rate);
void SetVoiceVolume(int percent);
void PlayVoice(void);
void StopVoice(void);
void *load_global_timbre(FILE *GTL, unsigned bank, unsigned patch);
void GetMusic(FILE *fin, int32_t size)  ;
void KillMusic(void);
void FadeMusicIn(int maxvolume, unsigned rate);
void FadeMusicOut(unsigned rate);
void PlayMusic(char mode);
void UpdateMusic(void);
void StopMusic(void);
int board_ID(drvr_desc *desc);
void PreLoadMusic(char val);
void Buzz_SoundDestroy(void);
unsigned int ABXframe(char *scratch, char *dest, int frameno, FILE *fhand);
int GetBuzzEnv(drvr_desc *desc, char *envs);
void PlayAudio(char *name, char mode);
void UpdateAudio(void);
void StopAudio(char mode);
void MuteChannel(int channel, int mute);
int IsChannelMute(int channel);

// SERIAL_F.C
void SaveFirst(char);
void CheckFirst(char);
void UpdateFirst(void);
void SendFirst(void);
void RecvFirst(void);
int e_handler(int c, int32_t p, char *s);
void SaveFirst(char mode);
void SendFirst(void);
void UpdateFirst(void);
void CheckFirst(char Hmode);
void RecvFirst(void);
int port_exist(int port);
void open_port(int port, int inlen);
void close_port(void);
void purge_in(void);
void set_baud(int32_t baud);
int32_t get_baud(void);
int get_bits(void);
int get_parity(void);
int get_stopbits(void);
void set_data_format(int bits, int parity, int stopbit);
void set_port(int32_t baud, int bits, int parity, int stopbit);
int in_ready(void);
int carrier(void);
void set_dtr(int n);

// CDROM.C
int CDAccess(int drive, int track, char mode);
int AquireDrive(void);

// TRACK.C
void Message(int msg, int tr);
void AAset(void);
void DialAt(int x, int y, char *s);
void get_dial(char *old);
void Read_Config(void);
int e_handler(int c, int32_t p, char *s);
void Dnload(void);
void Upload(void);
void ReadMsg(void);
void WriteMsg(void);
void Write_Config(void);
void DrawCenter();
void MesCenter(void);
char MPrefs(char mode);
void Toggle_Cfg(int opt, int old);
void TrackPict(char mode);

// MODEM.CPP
char dial(void);
char CheckCarrier(void);
void Modem_Config(void);
char *get_modem_string(char *s, char *d);
char DoModem(int sel);
void exit_program(void);
void hangup(void);
void modem_control_string(char *s);
#endif

// EOF
