#ifndef GX_H
#define GX_H

#include "gxdatatypes.h"

//void gxClearDisplay(int, int);
void gxVirtualDisplay(GXHEADER *hp, int a, int b, int c, int d, int e, int f, int g);
void gxVirtualVirtual(GXHEADER *hp, int a, int b, int c, int d, GXHEADER *hp2, int x, int y, int mode);
void gxGetImage(GXHEADER *hp, int a, int b, int c, int d, int e);
void gxClearVirtual(GXHEADER *hp, int a);
void gxPutImage(GXHEADER *hp, int mode, int a, int b, int c);
void gxVirtualScale(GXHEADER *a, GXHEADER *b);
int gxCreateVirtual(GXHEADER *hp, int w, int h);
void gxDestroyVirtual(GXHEADER *hp);
void gxDisplayVirtual(int a, int b, int c, int d, int e, GXHEADER *hp, int f, int g);

#define gxCMM 1
#define gxSET 1 /* mode */
#define gxVGA_13 1 /* 320x200 ... note: non-square pixels */
#define gxXOR 2

#endif // GX_H
