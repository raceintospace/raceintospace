#ifndef GR_H
#define GR_H

void grMoveTo(int x, int y);
void grLineTo(int x, int y);
int grGetMouseButtons(void);
void grLineRel(int x, int y);
void grMoveRel(int x, int y);
int grGetMouseCurPos(int *xp, int *yp);
int grGetMousePressedPos(int *xp, int *yp);
void gr_sync(void);

#define grOUTLINE 1

#endif // GR_H
