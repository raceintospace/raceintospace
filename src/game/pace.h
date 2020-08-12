#ifndef __PACE_H__
#define __PACE_H__
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include <av.h>

typedef struct {
	unsigned char *vptr;
	int x1, y1, x2, y2;
	int w, h;
} GXHEADER;

typedef struct {
	int dummy;
} drvr_desc;

ssize_t load_audio_file(const char *, char **data, size_t *size);

#define grTRACK 1
#define grCARROW 2

unsigned char XMAS;

int grInitMouse (void);
void grSetColor (int);
void grMoveTo (int x, int y);
void grLineTo (int x, int y);
void grPutPixel (int x, int y, int val);
void grSetMousePos (int x, int y);
int grGetPixel (int x, int y);
int grGetMouseCurPos (int *xp, int *yp);
int grGetMousePressedPos (int *xp, int *yp);
int grGetMouseButtons (void);
void grDrawLine (int x1, int y1, int x2, int y2);
void grSetBkColor (int col);
void grClearArea (int x1, int y1, int x2, int y2);
#define grOUTLINE 1
void grDrawRect (int x1, int y1, int x2, int y2, int mode);
void grLineRel (int x, int y);
void grMoveRel (int x, int y);

#define grLBUTTON 1
#define grRBUTTON 2

void gxClearDisplay (int, int);
void gxGetImage (GXHEADER *hp, int a, int b, int c, int d, int e);


#define gxSET 1 /* mode */
#define gxXOR 2
void gxPutImage (GXHEADER *hp, int mode, int a, int b, int c);

void gxVirtualDisplay (GXHEADER *hp, int a, int b, int c,
		       int d, int e, int f, int g);
void gxDisplayVirtual (int a, int b, int c, int d, int e,
		       GXHEADER *hp, int f, int g);
void gxClearVirtual (GXHEADER *hp, int a);
void gxSetDisplayPalette (char *pal);
void gxVirtualVirtual (GXHEADER *hp, int a, int b, int c, int d,
		       GXHEADER *hp2, int x, int y, int mode);
#define gxVGA_13 1 /* 320x200 ... note: non-square pixels */
int gxVirtualSize (int mode, int w, int h);
#define gxCMM 1
int gxCreateVirtual (int mode, GXHEADER *hp, int gxVGA_mode, int w, int h);
void gxDestroyVirtual (GXHEADER *hp);
void gxVirtualScale (GXHEADER *a, GXHEADER *b);

#define gxSUCCESS 0

void delay (int millisecs);
int brandom (int limit);
int bioskey (int wait);
int inp (int port);

#define EMPTY_BODY {}

typedef struct {
	int dummy;
} *HTIMER;

int getdisk (void);
void getcurdir (int drive, char *buf);

struct diskfree_t {
	int dummy;
};

//#define RED 2
//#define WHITE 3
//#define YELLOW 4
//#define BLACK 5

void randomize (void);

void SMove (void *p, int x, int y);
void LMove (void *p);

int getch (void);

double get_time (void);

int show_intro_flag;

char *letter_dat;

void gr_sync (void);
void gr_maybe_sync (void);

void gr_set_color_map (unsigned char *map);

int RLED_img (void *src, void *dest, unsigned int src_size, int w, int h);

void idle_loop (int ticks);
void idle_loop_secs (double secs);

char *seq_filename (int seq, int mode);
void play_audio (int sidx, int mode);

void bzdelay (int ticks);
void GetMouse_fast (void);

void dbg (char const *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif /* __PACE_H__ */
