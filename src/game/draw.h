#ifndef DRAW_H
#define DRAW_H

void PrintAt(int x, int y, const char *s);
void PrintAtKey(int x, int y, const char *s, char val);
void DispBig(int x, int y, const char *txt, char mode, char te);
void DispNum(int xx, int yy, int num);
void DispMB(int x, int y, int val);
void ShBox(int x1, int y1, int x2, int y2);
void UPArrow(int x1, int y1);
void RTArrow(int x1, int y1);
void LTArrow(int x1, int y1);
void DNArrow(int x1, int y1);
void InBox(int x1, int y1, int x2, int y2);
void OutBox(int x1, int y1, int x2, int y2);
void IOBox(int x1, int y1, int x2, int y2);
void RectFill(int x1, int y1, int x2, int y2, char col);
void GradRect(int x1, int y1, int x2, int y2, char plr);
void FlagSm(char plr, int xm, int ym);
void Flag(int x, int y, char plr);
void DispChr(char chr);

#endif // DRAW_H
