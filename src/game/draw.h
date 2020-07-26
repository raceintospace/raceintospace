#ifndef DRAW_H
#define DRAW_H

void draw_string(int x, int y, const char *s);
void draw_string_highlighted(int x, int y, const char *s, unsigned int position);
void draw_heading(int x, int y, const char *txt, char mode, char te);
void draw_number(int xx, int yy, int num);
void draw_megabucks(int x, int y, int val);
void ShBox(int x1, int y1, int x2, int y2);
void draw_up_arrow(int x1, int y1);
void draw_up_arrow_highlight(int x1, int y1);
void draw_right_arrow(int x1, int y1);
void draw_left_arrow(int x1, int y1);
void draw_down_arrow(int x1, int y1);
void draw_down_arrow_highlight(int x1, int y1);
void InBox(int x1, int y1, int x2, int y2);
void OutBox(int x1, int y1, int x2, int y2);
void IOBox(int x1, int y1, int x2, int y2);
void fill_rectangle(int x1, int y1, int x2, int y2, char col);
void GradRect(int x1, int y1, int x2, int y2, char plr);
void draw_small_flag(char plr, int xm, int ym);
void draw_flag(int x, int y, char plr);
void draw_character(char chr);
int TextDisplayLength(const char *str);

#endif // DRAW_H
