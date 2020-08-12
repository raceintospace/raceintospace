#include <Buzz_inc.h>
#include <externs.h>
#include <assert.h>

#ifdef DEAD_CODE
/** Unknown function
 * \deprecated This function is probably depricated
 */
int
grInitMouse (void)
{
	return gxSUCCESS;
}
#endif

#ifdef DEAD_CODE
/** Unknown function
 * \deprecated This function is probably depricated
 */
void
grSetMousePos (int x, int y)
{
}
#endif

int
grGetMouseButtons (void)
{
	int val;

	val = av_mouse_pressed_latched || av_mouse_pressed_cur;
	av_mouse_pressed_latched = 0;
	return (val);
}

int
grGetMousePressedPos (int *xp, int *yp)
{
	*xp = av_mouse_pressed_x / 2;
	*yp = av_mouse_pressed_y / 2;
	return (0);
}

int
grGetMouseCurPos (int *xp, int *yp)
{
	*xp = av_mouse_cur_x / 2;
	*yp = av_mouse_cur_y / 2;
	return (0);
}

static int gr_fg_color; /**< current foreground color to use */
static int gr_bg_color; /**< current background color to use */

void
gr_set_color_map (unsigned char *map)
{
	memcpy (pal, map, 256 * 3);
}

void
gr_sync (void)
{
	av_sync ();
}

void
gr_maybe_sync (void)
{
	if (screen_dirty)
		av_sync ();
}

/** set foreground color
 * 
 * \param color the color code to use
 * 
 * \li 6 = red
 * \li 1 = white
 */
void
grSetColor (int color)
{
	gr_fg_color = color;
}

/** set background color
 * 
 * \param color the color code to use
 */
void
grSetBkColor (int color)
{
	gr_bg_color = color;
}

void
grClearArea (int x1, int y1, int x2, int y2)
{
	int y, t;
	SDL_Rect r;

	assert(0 <= x1 && x1 < MAX_X);
	assert(0 <= x2 && x2 < MAX_X);
	assert(0 <= y1 && y1 < MAX_Y);
	assert(0 <= y2 && y2 < MAX_Y);

	if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
	if (y1 > y2) { t = y1; y1 = y2; y2 = t; }

	for (y = y1; y <= y2; ++y) {
		memset(&screen[y * MAX_X + x1], gr_bg_color, x2-x1+1);
	}

	r.x = x1;
	r.y = y1;
	r.h = y2-y1+1;
	r.w = x2-x1+1;
	av_need_update(&r);
}

static int gr_cur_x, gr_cur_y;

void
grMoveTo (int x, int y)
{
	gr_cur_x = x;
	gr_cur_y = y;
}

void
grPutPixel (int x, int y, int color)
{
	screen[y * MAX_X + x] = color;
}

//#define abs(a) (((a) >= 0) ? (a) : (0))
#define swap(a,b) (t = a, a = b, b = t)
void
grLineTo (int x_arg, int y_arg)
{
	int deltax, deltay;
	int error;
	int ystep;
	int x, y;
	int x0, y0, x1, y1;
	int steep;
	int t;

	x0 = gr_cur_x;
	y0 = gr_cur_y;

	x1 = x_arg;
	y1 = y_arg;

	steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	deltax = x1 - x0;
	deltay = abs (y1 - y0);
	error = 0;

	y = y0;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (x = x0; x <= x1; x++) {
		if (steep) {
			grPutPixel (y, x, gr_fg_color);
		} else {
			grPutPixel (x, y, gr_fg_color);
		}
		error = error + deltay;
		if (2 * error >= deltax) {
			y = y + ystep;
			error = error - deltax;
		}
	}

	gr_cur_x = x_arg;
	gr_cur_y = y_arg;
}

void
grLineRel (int dx, int dy)
{
	grLineTo (gr_cur_x + dx, gr_cur_y + dy);
}

void
grMoveRel (int dx, int dy)
{
	grMoveTo (gr_cur_x + dx, gr_cur_y + dy);
}

void
grDrawRect (int x1, int y1, int x2, int y2, int mode)
{
	SDL_Rect r;
	int t;

	assert (mode == grOUTLINE);

	if (x1 > x2) { t = x1; x1 = x2; x2 = t;}
	if (y1 > y2) { t = y1; y1 = y2; y2 = t;}

	grMoveTo (x1, y1);
	grLineTo (x2, y1);
	grLineTo (x2, y2);
	grLineTo (x1, y2);
	grLineTo (x1, y1);
	r.x = x1; r.y = y1;
	r.w = x2-x1+1; r.h = y2-y1+1;
}

void
grDrawLine (int x1, int y1, int x2, int y2)
{
	grMoveTo (x1, y1);
	grLineTo (x2, y2);
}

int
grGetPixel (int x, int y)
{
	assert(x >= 0 && x < MAX_X);
	assert(y >= 0 && y < MAX_Y);

	return screen[y * MAX_X + x];
}
