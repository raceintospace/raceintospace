#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <SDL.h>

#include <vorbis/vorbisfile.h>

struct music_file {
	struct music_file *next;
	char *name;
	char *buf;
	int size;
};
struct music_file *music_files;

struct music_file *
get_music_file (char *name)
{
	struct music_file *mp;
	FILE *inf;
	int togo, offset;
	OggVorbis_File vf;
	int ret;
	int bs;
	char fullname[1000];
	int chop;

	for (mp = music_files; mp; mp = mp->next) {
		if (xstrcasecmp (name, mp->name) == 0)
			return (mp);
	}

	sprintf (fullname, "/home/pace/ogg/%s.ogg", name);

    fprintf (stderr, "opening music file `%s'", fullname);

	if ((inf = fopen(fullname, "rb")) == NULL) {
		fprintf(stderr, "can't open music file `%s'", fullname);
		return NULL;
	}

	if ((mp = calloc (1, sizeof *mp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	if ((mp->name = strdup (name)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	mp->next = music_files;
	music_files = mp;

	if (ov_open (inf, &vf, NULL, 0) < 0) {
		fprintf(stderr, "ERROR: Failed to open input as vorbis\n");
		goto bad;
	}

	if (ov_info(&vf, 0)->channels != 1) {
		fprintf (stderr, "ERROR: ogg file must be mono\n");
		goto bad;
	}

        mp->size = ov_pcm_total(&vf, 0); /* output size in bytes */

	if ((mp->buf = calloc (1, mp->size)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	togo = mp->size;
	offset = 0;

	while (togo > 0) {
		if ((ret = ov_read (&vf, mp->buf + offset,
				    togo, 0, 1, 0, &bs)) < 0)
			break;

		offset += ret;
		togo -= ret;
	}

	ov_clear(&vf); /* closes inf */

	chop = 2 * 11025;
	if (mp->size > chop)
		mp->size -= chop;

	return (mp);
bad:
	fclose (inf);
	return (mp);
}

SDL_Surface *sur;


double
get_time (void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec + tv.tv_usec / 1e6);
}

void
intr (int sig)
{
	SDL_Quit ();
	exit (0);
}

int have_audio;

SDL_AudioSpec audio_desired, audio_obtained;

struct audio_chunk {
	struct audio_chunk *next;
	unsigned char *data;
	int size;
	int loop;
};

struct audio_chunk *cur_chunk, **cur_chunk_tailp = &cur_chunk;
int cur_offset;

void
audio_callback (void *userdata, Uint8 *stream, int len)
{
	int togo, thistime;

	togo = len;

	while (togo && cur_chunk) {
		thistime = cur_chunk->size - cur_offset;
		if (thistime > togo)
			thistime = togo;
		memcpy (stream, cur_chunk->data + cur_offset, thistime);

		stream += thistime;
		togo -= thistime;
		cur_offset += thistime;

		if (cur_offset >= cur_chunk->size) {
			if (cur_chunk->loop == 0) {
				if ((cur_chunk = cur_chunk->next) == NULL)
					cur_chunk_tailp = &cur_chunk;
			}
			cur_offset = 0;
		}
	}

	memset (stream, audio_obtained.silence, togo);
}

void
play (struct audio_chunk *new_chunk)
{
	if (have_audio == 0)
		return;
	SDL_LockAudio ();
	*cur_chunk_tailp = new_chunk;
	SDL_UnlockAudio ();
}

void
test_news (void)
{
	struct audio_chunk *cp;
	FILE *f;

	cp = calloc (1, sizeof *cp);
	cp->data = malloc (1000 * 1000);

	if ((f = fopen ("/l/baris/gamedat/unews.cdr", "rb")) == NULL) {
		fprintf (stderr, "can't open unews.cdr\n");
		exit (1);
	}

	cp->size = fread (cp->data, 1, 1000 * 1000, f);

	fclose (f);

	play (cp);
}

void
test_music (void)
{
	struct music_file *mp;
	struct audio_chunk *cp;

	if ((mp = get_music_file ("SOVTYP")) == NULL) {
		fprintf (stderr, "can't find music file\n");
		exit (1);
	}

	if (mp->buf == NULL || mp->size == 0) {
		fprintf (stderr, "can't get music\n");
		exit (1);
	}

	cp = calloc (1, sizeof *cp);
	cp->data = (void *)mp->buf;
	cp->size = mp->size;
	cp->loop = 1;
	play (cp);
}

int
main ()
{
	SDL_Rect r;
	SDL_Event ev;

	if (SDL_Init (SDL_INIT_VIDEO) < 0) {
		fprintf (stderr, "SDL_Init error\n");
		exit (1);
	}


	if (SDL_InitSubSystem (SDL_INIT_AUDIO < 0)) {
		printf ("no audio\n");
	} else {
		printf ("audio initialized\n");
		have_audio = 1;
	}

	if ((sur = SDL_SetVideoMode (320, 200, 24, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL) {
		fprintf (stderr, "error in SDL_SetVideoMode\n");
		exit (1);
	}

	SDL_EnableUNICODE (1);

	audio_desired.freq = 11025;
	audio_desired.format = AUDIO_U8;
	audio_desired.channels = 1;
	audio_desired.samples = 8192;
	audio_desired.callback = audio_callback;

	if (SDL_OpenAudio (&audio_desired, &audio_obtained) < 0) {
		fprintf (stderr, "error in SDL_OpenAudio\n");
		exit (1);
	}

	test_music ();
	SDL_PauseAudio (0);

	while (1) {
		while (SDL_PollEvent (&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				exit (0);
				break;
			case SDL_KEYDOWN:
				printf ("got key %d\n", ev.key.keysym.unicode);
				break;

				/* ignore these events */
			case SDL_KEYUP:
			case SDL_ACTIVEEVENT:
			case SDL_MOUSEMOTION:
				break;
			default:
				printf ("got uknown event %d\n", ev.type);
				break;
			}
		}

		r.x = 0; r.y = 0; r.w = 320; r.h = 200;
		SDL_FillRect (sur, &r, 0);

		r.x = 160 + 120 * cos (get_time () * .5 * M_PI);
		r.y = 100;
		r.w = 20;
		r.h = 20;
		SDL_FillRect (sur, &r, 0xff0000);
	
		SDL_Flip (sur);

		usleep (33 * 1000);

	}

	return (0);
}

