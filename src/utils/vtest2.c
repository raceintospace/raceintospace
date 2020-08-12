/*
    Copyright (C) 2007 Krzysztof Kosciuszkiewicz

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
#include <race.h>
#include <utils.h>
#include <mmfile.h>
#include <int_types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <SDL.h>

#ifndef min
#   define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

void
eperror(const char *str)
{
	perror(str);
	exit(EXIT_FAILURE);
}

void
eprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
usage(const char *progname)
{
	if (strrchr(progname, '/') != NULL)
		progname = strrchr(progname, '/') + 1;
	fprintf(stderr, "usage: %s file.ogg\n", progname);
	exit(EXIT_SUCCESS);
}

struct audiobuf
{
	char *buf;
	int off, bytes, size;
};

static void
audio_cb(void *userdata, Uint8 * stream, int len)
{
	struct audiobuf *abuf = userdata;
	int to_copy = min(len, abuf->bytes);

	memcpy(stream, abuf->buf + abuf->off, to_copy);
	abuf->bytes -= to_copy;
	abuf->off += to_copy;
}

int
main(int argc, char **argv)
{
	char *file = NULL;
	int have_video = 0, have_audio = 0;
	unsigned h = 0, w = 0;
	float fps = 0.0;
	unsigned ch = 0, hz = 0;
	struct audiobuf abuf;
	int end = 0;
	SDL_Surface *display = NULL;
	SDL_Overlay *ovl = NULL;
	SDL_Event event;
	mm_file media;

	if (argc > 1)
		file = argv[1];
	else
		usage(argv[0]);

	if (mm_open(&media, file) <= 0)
		eprintf("No audio or video in `%s'\n", file);

	if (mm_video_info(&media, &w, &h, &fps) >= 0)
	{
		printf("Video data: %dx%d, %gfps\n", w, h, fps);
		have_video = 1;
	}

	if (mm_audio_info(&media, &ch, &hz) >= 0)
	{
		printf("Audio data: %s, %dHz\n", (ch == 1) ? "mono" : "stereo", hz);
		have_audio = 1;
	}

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
		eprintf("Sdl init failed: %s\n", SDL_GetError());

	if (have_video)
	{
		if ((display = SDL_SetVideoMode(w, h, 24,
					SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL)
			eprintf("SDL_SetVideoMode: %s\n", SDL_GetError());

		if ((ovl = SDL_CreateYUVOverlay(w, h,
					SDL_YV12_OVERLAY, display)) == NULL)
			eprintf("SDL_CreateYUVOverlay: %s\n", SDL_GetError());
	}

	if (have_audio)
	{
		int bytes;
		int bps;
		double tdiff;
		SDL_AudioSpec desired;

		desired.channels = ch;
		desired.freq = hz;
		desired.format = AUDIO_U8;
		bps = ch * 1;
		desired.samples = 4096;
		desired.callback = audio_cb;
		desired.userdata = &abuf;
		if (SDL_OpenAudio(&desired, NULL) < 0)
			eprintf("SDL_OpenAudio: %s\n", SDL_GetError());

		abuf.size = 4 * 4096;
		abuf.off = 0;
		abuf.bytes = 0;
		abuf.buf = xmalloc(abuf.size);

		tdiff = get_time();
		while ((bytes = mm_decode_audio(&media,
					abuf.buf + abuf.bytes, abuf.size - abuf.bytes)) > 0)
		{
			abuf.bytes += bytes;
			if (abuf.size - abuf.bytes <= 4096)
				abuf.buf = xrealloc(abuf.buf, abuf.size *= 2);
		};
		if (bytes < 0)
			eperror("convert_audio");

		printf("Decoding: %.3f seconds\n", get_time() - tdiff);
		printf("Audio: %d samples, %.2f seconds\n", abuf.bytes / bps,
			(double) (abuf.bytes) / bps / desired.freq);

		SDL_PauseAudio(0);
	}

	while (!end)
	{
		while (SDL_PollEvent(&event))
			switch (event.type)
			{
				case SDL_QUIT:
					end = 1;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_q
						|| event.key.keysym.sym == SDLK_ESCAPE)
						end = 1;
					break;
				default:
					break;
			}

		if (have_video && !end)
		{
			static double oldt, newt;

			if (mm_decode_video(&media, ovl) > 0)
			{
				SDL_Rect r = { 0, 0, w, h };
				newt = 1 / fps + oldt - get_time();
				if (newt > 0)
					SDL_Delay(newt * 1000);
				SDL_DisplayYUVOverlay(ovl, &r);
				oldt = get_time();
			}
			else
				end = 1;
		}

		if (have_audio && abuf.bytes <= 0)
			end = 1;

		if (!have_video)
			SDL_Delay(100);
	}

	if (have_audio)
	{
		SDL_PauseAudio(1);
		free(abuf.buf);
	}

	if (have_video)
	{
		SDL_FreeYUVOverlay(ovl);
	}

	SDL_Quit();
	mm_close(&media);

	return EXIT_SUCCESS;
}

/* vim: set noet ts=4 sw=4 tw=77: */
