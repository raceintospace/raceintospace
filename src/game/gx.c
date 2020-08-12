#include <Buzz_inc.h>
#include <limits.h>
#include <assert.h>
#include <externs.h>
#include <av.h>
#include <utils.h>
#include <SDL.h>

extern GXHEADER vhptr;

int
gxVirtualSize (int mode, int w, int h)
{
	return w * h;
}

int
gxCreateVirtual (int mode, GXHEADER *hp,
		 int gxVGA_mode, int w, int h)
{
	assert(hp);
	memset(hp, 0, sizeof *hp);
	hp->w = w;
	hp->h = h;
	hp->vptr = xmalloc(w * h);
	return gxSUCCESS;
}

void
gxDestroyVirtual (GXHEADER *hp)
{
	assert(hp);
	assert(hp->vptr);
	free(hp->vptr);
	hp->vptr = NULL;
}

void
gxGetImage (GXHEADER *hp, int x0, int y0, int x1, int y1, int op)
{
	int w, h, from_idx, to_idx, row;

	assert(op == 0);
	assert(hp);
	assert(0 <= x0 && x0 < MAX_X);
	assert(0 <= x1 && x1 < MAX_X);
	assert(0 <= y0 && y0 < MAX_Y);
	assert(0 <= y1 && y1 < MAX_Y);
	assert(x0 <= x1);
	assert(y0 <= y0);

	w = x1 - x0 + 1;
	h = y1 - y0 + 1;

	assert(w <= hp->w);
	assert(h <= hp->h);

	for (row = 0; row < h; row++) {
		from_idx = (y0 + row) * MAX_X + x0;
		to_idx = row * hp->w;
		memcpy(&hp->vptr[to_idx], &screen[from_idx], w);
	}
}

void
gxPutImage (GXHEADER *hp, int mode, int x, int y, int op2)
{
	int row, col, from_idx, to_idx;
	int clip_x, clip_y;
	SDL_Rect r;

	assert(op2 == 0);
	assert(hp);
	assert(mode == gxSET || mode == gxXOR);
	assert(0 <= x && x < MAX_X);
	assert(0 <= y && y < MAX_Y);

	clip_y = minn(hp->h + y, MAX_Y) - y;
	clip_x = minn(hp->w + x, MAX_X) - x;

	switch (mode) {
	case gxSET:
		for (row = 0; row < clip_y; row++) {
			from_idx = row * hp->w;
			to_idx = (y + row) * MAX_X + x;
			memcpy(&screen[to_idx], &hp->vptr[from_idx], clip_x);
		}
		break;
	case gxXOR:
		for (row = 0; row < clip_y; row++) {
			from_idx = row * hp->w;
			to_idx = (y + row) * MAX_X + x;
			for (col = 0; col < clip_x; col++) {
				screen[to_idx+col] ^= hp->vptr[from_idx+col];
			}
		}
		break;
	}

	r.x = x; r.y = y;
	r.w = clip_x; r.h = clip_y;
	av_need_update(&r);
	screen_dirty = 1;
}

void
gxClearDisplay (int a, int b)
{
	SDL_Rect r = {0, 0, MAX_X, MAX_Y};
	assert(a == 0 && b == 0);

	memset (screen, 0, MAX_X * MAX_Y);
	av_need_update(&r);
	screen_dirty = 1;
}

void
gxVirtualDisplay (GXHEADER *hp,
		  int from_x, int from_y,
		  int to_x0, int to_y0,
		  int to_x1, int to_y1,
		  int always_zero)
{
	int row, from_idx, to_idx;
	int width, height;
	int clip_x, clip_y;
	SDL_Rect r;

	assert(hp);
	assert(always_zero == 0);
	assert(0 <= from_x && from_x < hp->w);
	assert(0 <= from_y && from_y < hp->h);
	assert(to_x0 <= to_x1);
	assert(to_y0 <= to_y1);
	assert(0 <= to_x0 && to_x0 < MAX_X);
	assert(0 <= to_x1 && to_x1 < MAX_X);
	assert(0 <= to_y0 && to_y0 < MAX_Y);
	assert(0 <= to_y1 && to_y1 < MAX_Y);

	width  = to_x1 - to_x0 + 1;
	height = to_y1 - to_y0 + 1;

	clip_y = minn(height + to_y0, MAX_Y) - to_y0;
	clip_x = minn(width  + to_x0, MAX_X) - to_x0;

	for (row = 0; row < clip_y; row++) {
		from_idx = (from_y + row) * hp->w + from_x;
		to_idx = (to_y0 + row) * MAX_X + to_x0;
		memcpy(&screen[to_idx], &hp->vptr[from_idx], clip_x);
	}
	r.x = to_x0; r.y = to_y0;
	r.w = clip_x; r.h = clip_y;
	av_need_update(&r);
	screen_dirty = 1;
}

void
gxDisplayVirtual (int from_x0, int from_y0,
		  int from_x1, int from_y1,
		  int always_zero,
		  GXHEADER *hp,
		  int to_x, int to_y)
{
	int row, from_idx, to_idx;
	int width, height;

	assert(hp);
	assert(always_zero == 0);
	assert(0 <= from_x0 && from_x0 < MAX_X);
	assert(0 <= from_x1 && from_x1 < MAX_X);
	assert(0 <= from_y0 && from_y0 < MAX_Y);
	assert(0 <= from_y1 && from_y1 < MAX_Y);
	assert(from_x0 <= from_x1);
	assert(from_y0 <= from_y0);
	assert(0 <= to_x && to_x < hp->w);
	assert(0 <= to_y && to_y < hp->h);

	width  = from_x1 - from_x0 + 1;
	height = from_y1 - from_y0 + 1;

	assert(width  <= hp->w);
	assert(height <= hp->w);

	for (row = 0; row < height; row++) {
		from_idx = (from_y0 + row) * MAX_X + from_x0;
		to_idx = (to_y + row) * hp->w + to_x;
		memcpy(&hp->vptr[to_idx], &screen[from_idx], width);
	}
}

void
gxSetDisplayPalette (char *pal)
{
}

void
gxVirtualVirtual (GXHEADER *from,
		  int from_x1, int from_y1,
		  int from_x2, int from_y2,
		  GXHEADER *to,
		  int to_x, int to_y,
		  int mode)
{
	int w, h;
	int row;
	int from_idx, to_idx;

	assert(from);
	assert(to);
	assert(0 <= from_x1 && from_x1 < from->w);
	assert(0 <= from_y1 && from_y1 < from->h);
	assert(0 <= from_x2 && from_x2 < from->w);
	assert(0 <= from_y2 && from_y2 < from->h);
	assert(0 <= to_x	&& to_x	   < to->w);
	assert(0 <= to_y	&& to_y	   < to->h);

	w = from_x2 - from_x1 + 1;
	h = from_y2 - from_y1 + 1;

	assert(w <= to->w - to_x);
	assert(h <= to->h - to_y);

	for (row = 0; row < h; row++) {
		from_idx = (from_y1 + row) * from->w + from_x1;
		to_idx = (to_y + row) * to->w + to_x;
		memcpy(&to->vptr[to_idx], &from->vptr[from_idx], w);
	}
}

void
gxClearVirtual (GXHEADER *hp, int a)
{
	assert(hp);
	assert(a == 0);
	memset (hp->vptr, 0, hp->w * hp->h);
}

void
gxVirtualScale (GXHEADER *src, GXHEADER *dest)
{
	int dest_row, dest_col, dest_idx;
	int src_row, src_col, src_idx;

	for (dest_row = 0; dest_row < dest->h; dest_row++) {
		src_row = (dest_row * src->h) / dest->h;

		for (dest_col = 0; dest_col < dest->w; dest_col++) {
			src_col  = (dest_col * src->w) / dest->w;

			src_idx  = src_row	* src->w  + src_col;
			dest_idx = dest_row * dest->w + dest_col;

			dest->vptr[dest_idx] = src->vptr[src_idx];
		}
	}
}

/* vim: set noet ts=4 sw=4 tw=77: */
