#ifndef GR_H
#define GR_H

void grSetColor(int);
void grPutPixel(int x, int y, int val);
void grMoveTo(int x, int y);
void grLineTo(int x, int y);
int grGetMouseButtons(void);
int grGetPixel(int x, int y);
void grDrawLine(int x1, int y1, int x2, int y2);
void grLineRel(int x, int y);
void grMoveRel(int x, int y);
int grGetMouseCurPos(int *xp, int *yp);
int grGetMousePressedPos(int *xp, int *yp);
void grSetBkColor(int col);
void grClearArea(int x1, int y1, int x2, int y2);
void grDrawRect(int x1, int y1, int x2, int y2, int mode);
void gr_maybe_sync(void);
void gr_sync(void);

#define grOUTLINE 1

#endif // GR_H