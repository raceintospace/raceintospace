// This file defines how characters are printed on the screen

#include "draw.h"
#include "gr.h"
#include "pace.h"
#include "sdlhelper.h"
#include "filesystem.h"

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/** Print string at specific position
 *
 * The function will print a string at a certain position.
 * If both the x and y coordinate are zero (0) the position is "current position".
 * The area will be marked as "needs update" to the graphics handler.
 *
 * \note Will not print anything if length of string exceeds 100 characters
 *
 * \param x x-coordinate of the start of the string
 * \param y y-coordinate of the start of the string
 * \param s pointer to char-array (string) to print
 *
 */
void draw_string(int x, int y, const char *s)
{
    short i;

    if (x != 0 && y != 0) {
        grMoveTo(x, y);
    }

    if (strlen(s) > 100) {
        return;
    }

    for (i = 0; i < (int)strlen(s); i++) {
        draw_character(s[i]);
    }
}

void draw_string_highlighted(int x, int y, const char *s, unsigned int position)
{
    draw_string(x, y, s);
    grMoveTo(x, y);
    display::graphics.setForegroundColor(9);
    draw_character(s[position]);
    return;
}

/**
 * Draw text using the Header character set.
 *
 * \param x   x-coordinate for displaying the header text.
 * \param y   y-coordinate for displaying the header text.
 * \param txt  The text string to display.
 * \param mode  0 or 1 (Unused).
 * \param te  Highlight the letter at index te (0-based) in red.
 */
void draw_heading(int x, int y, const char *txt, char mode, char te)
{
    int i, k, l, px;
    struct LET {
        char width, img[15][21];
    } letter;
    const int letterSize = sizeof(letter.width) + sizeof(letter.img);
    int c;

    if (txt == NULL) {
        // TODO: This should log an error or throw an exception.
        return;
    }

    y--;

    for (i = 0; i < (int)strlen(txt); i++) {
        if (txt[i] == 0x20) {
            x += 6;
            i++;
        }

        c = toupper(txt[i] & 0xff);

        if (c >= 0x30 && c <= 0x39) {
            px = c - 32;
        } else {
            px = c - 33;
        }

        if (c == '-') {
            px++;
        }

        // Read into letter piecewise to avoid packing issues.
        const char *offset = letter_dat + (letterSize * px);
        memcpy(&letter.width, offset, sizeof(letter.width));
        memcpy(&letter.img, offset + sizeof(letter.width),
               sizeof(letter.img));

        for (k = 0; k < 15; k++) {
            for (l = 0; l < letter.width; l++) {
                if (letter.img[k][l] != 0x03) {
                    if ((letter.img[k][l] == 0x01 || letter.img[k][l] == 0x02) && i == te) {
                        display::graphics.legacyScreen()->setPixel(x + l, y + k, letter.img[k][l] + 7);
                    } else {
                        display::graphics.legacyScreen()->setPixel(x + l, y + k, letter.img[k][l]);
                    }
                }
            }
        }

        x += letter.width - 1;
    }
}

void draw_number(int xx, int yy, int num)
{
    short n0, n1, n2, n3, t;

    if (xx != 0 && yy != 0) {
        grMoveTo(xx, yy);
    }

    t = num;
    num = abs(t);

    if (t < 0) {
        draw_string(0, 0, "-");
    }

    n0 = num / 1000;
    n1 = num / 100 - n0 * 10;
    n2 = num / 10 - n0 * 100 - n1 * 10;
    n3 = num - n0 * 1000 - n1 * 100 - n2 * 10;

    if (n0 != 0) {
        draw_character(n0 + 0x30);
        draw_character(n1 + 0x30);
        draw_character(n2 + 0x30);
        draw_character(n3 + 0x30);
    }

    if (n0 == 0 && n1 != 0) {
        draw_character(n1 + 0x30);
        draw_character(n2 + 0x30);
        draw_character(n3 + 0x30);
    }

    if (n0 == 0 && n1 == 0 && n2 != 0) {
        draw_character(n2 + 0x30);
        draw_character(n3 + 0x30);
    }

    if (n0 == 0 && n1 == 0 && n2 == 0) {
        draw_character(n3 + 0x30);
    }

    return;
}


void draw_megabucks(int x, int y, int val)
{
    draw_number(x, y, val);
    draw_string(0, 0, " MB");
    return;
}

void ShBox(int x1, int y1, int x2, int y2)
{
    fill_rectangle(x1, y1, x2, y2, 3);
    OutBox(x1, y1, x2, y2);
    return;
}

void draw_up_arrow(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1, 25 + y1, 4);
    display::graphics.legacyScreen()->line(3 + x1, 11 + y1, 5 + x1, 11 + y1, 4);

    display::graphics.legacyScreen()->line(1 + x1, y1, 6 + x1, 11 + y1, 2);
    display::graphics.legacyScreen()->line(3 + x1, 12 + y1, 3 + x1, 25 + y1, 2);
    return;
}

void draw_up_arrow_highlight(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1, 25 + y1, 15);
    display::graphics.legacyScreen()->line(3 + x1, 11 + y1, 5 + x1, 11 + y1, 15);

    display::graphics.legacyScreen()->line(1 + x1, y1, 6 + x1, 11 + y1, 11);
    display::graphics.legacyScreen()->line(3 + x1, 12 + y1, 3 + x1, 25 + y1, 11);
    return;
}

void draw_right_arrow(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1 + 31, y1, 4);
    display::graphics.legacyScreen()->line(x1 + 20, y1 + 3, x1 + 20, y1 + 5, 4);

    display::graphics.legacyScreen()->line(x1, y1 + 3, x1 + 19, y1 + 3, 2);
    display::graphics.legacyScreen()->line(x1 + 31, y1 + 1, x1 + 20, y1 + 6, 2);
    return;
}

void draw_left_arrow(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1 + 31, y1, 4);
    display::graphics.legacyScreen()->line(x1 + 11, y1 + 3, x1 + 11, y1 + 5, 4);

    display::graphics.legacyScreen()->line(x1, y1 + 1, x1 + 11, y1 + 6, 2);
    display::graphics.legacyScreen()->line(x1 + 12, y1 + 3, x1 + 31, y1 + 3, 2);
    return;
}

void draw_down_arrow(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1, 25 + y1, 4);
    display::graphics.legacyScreen()->line(3 + x1, 14 + y1, 5 + x1, 14 + y1, 4);

    display::graphics.legacyScreen()->line(3 + x1, y1, 3 + x1, 13 + y1, 2);
    display::graphics.legacyScreen()->line(6 + x1, 14 + y1, 1 + x1, 25 + y1, 2);
    return;
}

void draw_down_arrow_highlight(int x1, int y1)
{
    display::graphics.legacyScreen()->line(x1, y1, x1, 25 + y1, 15);
    display::graphics.legacyScreen()->line(3 + x1, 14 + y1, 5 + x1, 14 + y1, 15);

    display::graphics.legacyScreen()->line(3 + x1, y1, 3 + x1, 13 + y1, 11);
    display::graphics.legacyScreen()->line(6 + x1, 14 + y1, 1 + x1, 25 + y1, 11);

    return;
}

void InBox(int x1, int y1, int x2, int y2)
{
    display::graphics.setForegroundColor(2);
    grMoveTo(x1, y2);
    grLineTo(x2, y2);
    grLineTo(x2, y1);
    display::graphics.setForegroundColor(4);
    grLineTo(x1, y1);
    grLineTo(x1, y2);
}

void OutBox(int x1, int y1, int x2, int y2)
{
    display::graphics.setForegroundColor(4);
    grMoveTo(x1, y2);
    grLineTo(x2, y2);
    grLineTo(x2, y1);
    display::graphics.setForegroundColor(2);
    grMoveTo(x2 - 1, y1);
    grLineTo(x1, y1);
    grLineTo(x1, y2 - 1);
}

void IOBox(int x1, int y1, int x2, int y2)
{
    InBox(x1, y1, x2, y2);
    display::graphics.setForegroundColor(0);
    display::graphics.legacyScreen()->outlineRect(x1 + 1, y1 + 1, x2 - 1, y2 - 1, 0);
    OutBox(x1 + 2, y1 + 2, x2 - 2, y2 - 2);
}

/** draw a rectangle
 *
 * \param x1
 * \param y1
 * \param x2
 * \param y2
 * \param col Color code of the rectangle
 *
 */
void fill_rectangle(int x1, int y1, int x2, int y2, char col)
{
    display::graphics.setBackgroundColor(col);
    display::graphics.legacyScreen()->fillRect(x1, y1, x2, y2, col);
    return;
}

void GradRect(int x1, int y1, int x2, int y2, char plr)
{
    //register int i,j,val;
    //val=3*plr+6;

    fill_rectangle(x1, y1, x2, y2, 7 + 3 * plr);
    //for (j=x1;j<=x2;j+=4)
    //  for (i=y1;i<=y2;i+=4)
    //    screen[j+320*i]=val;
    return;
}

void draw_small_flag(char plr, int xm, int ym)
{
    char fn[64];
    snprintf(fn, sizeof(fn), "images/small_flag.%i.png", (int)plr);
    boost::shared_ptr<display::Surface> flag(Filesystem::readImage(fn));
    display::graphics.screen()->draw(flag, xm, ym);
}

void draw_flag(int x, int y, char plr)
{
    char fn[64];
    snprintf(fn, sizeof(fn), "images/flag.%i.png", (int)plr);
    boost::shared_ptr<display::Surface> flag(Filesystem::readImage(fn));
    display::graphics.screen()->draw(flag, x, y);
}


// Convenience defines, it seems the implementor didn't feel like typing much
#define MR grMoveRel
#define LR grLineRel
#define LT grLineTo
#define MT grMoveTo
#define SC grSetColor

/** Prints a character at current position of graphics handler.
 *
 * \note The function converts all characters to upper case before printing.
 *
 * \param chr Character to be printed
 */
void draw_character(char chr)
{
    switch (toupper(chr)) {
    case 'A':
        LR(0, -3);
        LR(1, -1);
        LR(2, 0);
        LR(1, 1);
        LR(0, 3);
        LR(-1, -1);
        LR(-2, 0);
        MR(5, 1);
        break;

    case 'B':
        LR(0, -4);
        LR(3, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(1, 1);
        LR(-1, 1);
        LR(-2, 0);
        MR(0, -2);
        LR(1, 0);
        MR(4, 2);
        break;

    case 'C':
        MR(4, -4);
        LR(-3, 0);
        LR(-1, 1);
        LR(0, 2);
        LR(1, 1);
        LR(3, 0);
        MR(2, 0);
        break;

    case 'D':
        LR(0, -4);
        LR(3, 0);
        LR(1, 1);
        LR(0, 2);
        LR(-1, 1);
        LR(-2, 0);
        MR(5, 0);
        break;

    case 'E':
        LR(0, -4);
        LR(4, 0);
        MR(0, 4);
        LR(-3, 0);
        MR(0, -2);
        LR(1, 0);
        MR(4, 2);
        break;

    case 'F':
        LR(0, -4);
        LR(4, 0);
        MR(-2, 2);
        LR(-1, 0);
        MR(5, 2);
        break;

    case 'G':
        MR(4, -4);
        LR(-3, 0);
        LR(-1, 1);
        LR(0, 2);
        LR(1, 1);
        LR(3, 0);
        LR(0, -2);
        LR(-1, 0);
        MR(3, 2);
        break;

    case 'H':
        LR(0, -4);
        MR(1, 2);
        LR(2, 0);
        MR(1, -2);
        LR(0, 4);
        MR(2, 0);
        break;

    case 'I':
        LR(2, 0);
        LR(-1, -1);
        LR(0, -2);
        LR(-1, -1);
        LR(2, 0);
        MR(2, 4);
        break;

    case 'J':
        MR(0, -1);
        LR(1, 1);
        LR(2, 0);
        LR(1, -1);
        LR(0, -3);
        MR(2, 4);
        break;

    case 'K':
        LR(0, -4);
        MR(4, 0);
        LR(-2, 2);
        LR(-1, 0);
        MR(1, 0);
        LR(2, 2);
        MR(2, 0);
        break;

    case 'L':
        MR(0, -4);
        LR(0, 4);
        LR(4, 0);
        MR(2, 0);
        break;

    case 'M':
        LR(0, -4);
        LR(2, 2);
        LR(2, -2);
        LR(0, 4);
        MR(2, 0);
        break;

    case 'N':
        LR(0, -4);
        LR(4, 4);
        LR(0, -4);
        MR(2, 4);
        break;

    case 'O':
    case '0':
        MR(0, -1);
        LR(0, -2);
        LR(1, -1);
        LR(2, 0);
        LR(1, 1);
        LR(0, 2);
        LR(-1, 1);
        LR(-2, 0);
        MR(5, 0);
        break;

    case 'P':
        LR(0, -4);
        LR(3, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(-2, 0);
        MR(5, 2);
        break;

    case 'Q':
        MR(0, -1);
        LR(0, -2);
        LR(1, -1);
        LR(2, 0);
        LR(1, 1);
        LR(0, 2);
        LR(-1, 1);
        LR(-2, 0);
        MR(2, -1);
        LR(1, 1);
        MR(2, 0);
        break;

    case 'R':
        LR(0, -4);
        LR(3, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(-2, 0);
        MR(3, 1);
        LR(0, 1);
        MR(2, 0);
        break;

    case 'S':
        LR(3, 0);
        LR(1, -1);
        LR(-1, -1);
        LR(-2, 0);
        LR(-1, -1);
        LR(1, -1);
        LR(3, 0);
        MR(2, 4);
        break;

    case 'T':
        MR(2, 0);
        LR(0, -4);
        LR(-2, 0);
        LR(4, 0);
        MR(2, 4);
        break;

    case 'U':
        MR(0, -4);
        LR(0, 3);
        LR(1, 1);
        LR(2, 0);
        LR(1, -1);
        LR(0, -3);
        MR(2, 4);
        break;

    case 'V':
        MR(0, -4);
        LR(0, 2);
        LR(2, 2);
        LR(2, -2);
        LR(0, -2);
        MR(2, 4);
        break;

    case 'W':
        MR(0, -4);
        LR(0, 4);
        LR(2, -2);
        LR(2, 2);
        LR(0, -4);
        MR(2, 4);
        break;

    case 'X':
        MR(0, -4);
        LR(4, 4);
        MR(0, -4);
        LR(-4, 4);
        MR(6, 0);
        break;

    case 'Y':
        MR(2, 0);
        LR(0, -1);
        LR(-2, -2);
        LR(0, -1);
        MR(4, 0);
        LR(0, 1);
        LR(-2, 2);
        MR(4, 1);
        break;

    case 'Z':
        MR(0, -4);
        LR(4, 0);
        LR(-4, 4);
        LR(4, 0);
        MR(2, 0);
        break;

    case '1':
        LR(2, 0);
        LR(-1, -1);
        LR(0, -3);
        LR(-1, 1);
        MR(4, 3);
        break;

    case '2':
        MR(0, -4);
        LR(3, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(-1, 0);
        LR(-2, 2);
        LR(4, 0);
        MR(2, 0);
        break;

    case '3':
        LR(3, 0);
        LR(1, -1);
        LR(-1, -1);
        LR(-1, 0);
        LR(1, 0);
        LR(1, -1);
        LR(-1, -1);
        LR(-3, 0);
        MR(6, 4);
        break;

    case '4':
        MR(4, -1);
        LR(-4, 0);
        LR(0, -1);
        LR(2, -2);
        LR(1, 0);
        LR(0, 4);
        MR(3, 0);
        break;

    case '5':
        LR(3, 0);
        LR(1, -1);
        LR(-1, -1);
        LR(-2, 0);
        LR(-1, -1);
        LR(0, -1);
        LR(4, 0);
        MR(2, 4);
        break;

    case '6':
        MR(1, -2);
        LR(2, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(-2, 0);
        LR(-1, -1);
        LR(0, -2);
        LR(1, -1);
        LR(2, 0);
        MR(3, 4);
        break;

    case '7':
        MR(0, -4);
        LR(4, 0);
        LR(0, 1);
        LR(-2, 2);
        LR(0, 1);
        MR(4, 0);
        break;

    case '8':
        MR(1, 0);
        LR(2, 0);
        LR(1, -1);
        LR(-1, -1);
        LR(-2, 0);
        LR(-1, -1);
        LR(1, -1);
        LR(2, 0);
        LR(1, 1);
        MR(-4, 2);
        LR(0, 0);
        MR(6, 1);
        break;

    case '9':
        MR(1, 0);
        LR(2, 0);
        LR(1, -1);
        LR(0, -2);
        LR(-1, -1);
        LR(-2, 0);
        LR(-1, 1);
        LR(1, 1);
        LR(2, 0);
        MR(3, 2);
        break;

    case '-':
        MR(0, -2), LR(3, 0);
        MR(2, 2);
        break;

    case '+':
        MR(0, -2);
        LR(4, 0);
        MR(-2, -2);
        LR(0, 4);
        MR(4, 0);
        break;

    case '.':
        LR(0, 0);
        MR(2, 0);
        break;

    case ',':
        MR(0, 1);
        LR(1, -1);
        MR(2, 0);
        break;

    case ':':
        MR(0, -1);
        LR(0, 0);
        MR(0, -2);
        LR(0, 0);
        MR(2, 3);
        break;

    case '&':
        MR(0, -1);
        LR(1, 1);
        LR(1, 0);
        LR(1, -1);
        LR(1, 1);
        LR(-2, -2);
        LR(-1, 0);
        LR(0, -1);
        LR(1, -1);
        LR(1, 1);
        MR(3, 3);
        break;

    case ' ':
        MR(3, 0);
        break;

    case '!':
        LR(0, 0);
        MR(0, -2);
        LR(0, -2);
        MR(2, 4);
        break;

    case '@':
    case '#':
        MR(1, 0);
        LR(0, -4);
        MR(-1, 1);
        LR(4, 0);
        MR(-1, -1);
        LR(0, 4);
        MR(1, -1);
        LR(-4, 0);
        MR(6, 1);
        break;

    case '%':
        LR(4, -4);
        MR(-3, 0);
        LR(-1, 1);
        LR(0, -1);
        LR(4, 4);
        LR(-1, 0);
        LR(1, -1);
        MR(2, 1);
        break;

    case '(':
        MR(1, 0);
        LR(-1, -1);
        LR(0, -2);
        LR(1, -1);
        MR(2, 4);
        break;

    case ')':
        LR(1, -1);
        LR(0, -2);
        LR(-1, -1);
        MR(3, 4);
        break;

    case '/':
        LR(4, -4);
        MR(2, 4);
        break;

    case '<':
        MR(4, -4);
        LR(-2, 2);
        LR(2, 2);
        MR(2, 0);
        break;

    case '>':
        MR(0, -4);
        LR(2, 2);
        LR(-2, 2);
        MR(4, 0);
        break;

    case 0x27:
        MR(0, -4);
        LR(0, 1);
        MR(2, 3);
        break;

    case '*':
        MR(1, 0);
        LR(0, -4);
        MR(-1, 1);
        LR(4, 0);
        MR(-1, -1);
        LR(0, 4);
        MR(1, -1);
        LR(-4, 0);
        MR(6, 1);
        break;

    case '^':
        MR(0, -3);
        LR(1, -1);
        LR(1, 0);
        LR(1, 1);
        MR(0, 3);
        break;

    case '?':
        MR(0, -3);
        LR(1, -1);
        LR(2, 0);
        LR(1, 1);
        LR(-1, 1);
        LR(-1, 0);
        MR(0, 2);
        LR(0, 0);
        MR(4, 0);
        break;

    case 0x14:
        LR(0, -4);
        MR(2, 4);
        break;

    default:
        break;
    }
}


/**
 * Calculate the width a string will occupy on the screen.
 *
 * Determines how much screen width a character string would occupy if
 * sent to the draw_string method.
 *
 * \param   a text string
 * \return  count in pixels.
 */
int TextDisplayLength(const char *str)
{
    unsigned int pixels = 0;
    int count = (int) strlen(str);

    for (int i = 0; i < count; i++) {

        switch (toupper(str[i])) {
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '0':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '&':
        case '@':
        case '#':
        case '%':
        case '/':
        case '<':
        case '*':
        case '?':
            pixels += 6;
            break;

        case '-':
            pixels += 5;
            break;

        case 'I':
        case '1':
        case '>':
            pixels += 4;
            break;

        case ',':
        case ' ':
        case '(':
        case ')':
        case '^':  // 3-pixels, no trailing space
            pixels += 3;
            break;

        case '.':
        case ':':
        case '!':
        case 0x27:
        case 0x14:
            pixels += 2;
            break;

        default:
            // Should a message be logged here?
            break;
        }
    }

    // Account for the pixel space after the last character.
    if (count > 0 && str[count - 1] != '^') {
        pixels -= 1;
    }

    return pixels;
}
