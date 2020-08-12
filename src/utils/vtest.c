#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <alsa/asoundlib.h>

int xrun_count;

int dsp_fd;

static int buffer_size;
static int period_size;

static void
set_hwparams(snd_pcm_t *handle)
{
        snd_pcm_uframes_t size;
        int dir;
        snd_pcm_hw_params_t *hwparams;
	unsigned int uval;

        snd_pcm_hw_params_malloc(&hwparams);

        snd_pcm_hw_params_any(handle, hwparams);

        snd_pcm_hw_params_set_rate_resample(handle, hwparams, 1);

        snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_U8);
        snd_pcm_hw_params_set_channels(handle, hwparams, 1);

	uval = 11025;
        snd_pcm_hw_params_set_rate_near(handle, hwparams, &uval, 0);

	uval = 500 * 1000; /* usecs */
        snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &uval, &dir);
        snd_pcm_hw_params_get_buffer_size(hwparams, &size);
	buffer_size = size;

	uval = 100 * 1000;
        snd_pcm_hw_params_set_period_time_near(handle, hwparams, &uval, &dir);
        snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
	period_size = size;

        snd_pcm_hw_params(handle, hwparams);

	printf ("buffer_size %d; period_size %d\n", buffer_size, period_size);
}

static void
set_swparams(snd_pcm_t *handle)
{
        snd_pcm_sw_params_t *swparams;

        snd_pcm_sw_params_malloc(&swparams);

        snd_pcm_sw_params_current(handle, swparams);

        /* start the transfer when the buffer is almost full: */
        /* (buffer_size / avail_min) * avail_min */
        snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);

        /* allow the transfer when at least period_size samples can be processed */
        snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);

        /* align all transfers to 1 sample */
        snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);

        snd_pcm_sw_params(handle, swparams);
}

snd_pcm_t *pcm;

void
sound_init (void)
{
	int err;
	char *device;

	device = "plughw:0,0";

	printf ("opening pcm device %s\n", device);
        if ((err = snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
                printf("Playback open error: %s\n", snd_strerror(err));
		exit (1);
        }

	set_hwparams(pcm);
	set_swparams(pcm);
}

int soundbuf_size;
unsigned char *soundbuf;
int soundbuf_used;
int soundbuf_off;
int soundbuf_playing;

/* return 0 when there is nothing left to play */
int
sound_step (void)
{
	int avail, n;

	if (soundbuf_playing == 0)
		return (0);

	if ((avail = soundbuf_used - soundbuf_off) == 0) {
		soundbuf_playing = 0;

		snd_pcm_drain (pcm);
		return (0);
	}

	if (avail > period_size)
		avail = period_size;

	n = snd_pcm_writei (pcm, soundbuf + soundbuf_off, avail);

	if (n < 0) {
		if (n == -EAGAIN) {
			usleep (1);
			return (1);
		}
		if (n == -EPIPE) {
			xrun_count++;
			snd_pcm_prepare (pcm);
			return (1);
		}
		
		printf ("sound write error: %s\n", 
			snd_strerror(n));
		exit (1);
	}

//	printf ("used %d (xrun %d)\n", n, xrun_count);


	soundbuf_off += n;

	return (1);
}

void
sound_play (unsigned char *buf, int size)
{
	unsigned char *start_buf;

	if (size > soundbuf_size) {
		if (soundbuf)
			free (soundbuf);
		soundbuf_size = size;
		if ((soundbuf = malloc (soundbuf_size)) == NULL) {
			fprintf (stderr, "out of memory\n");
			exit (1);
		}
	}

	memcpy (soundbuf, buf, size);

	soundbuf_used = size;
	soundbuf_off = 0;
	soundbuf_playing = 1;

	snd_pcm_start (pcm);
	snd_pcm_prepare (pcm);

	start_buf = malloc (buffer_size);
	memset (start_buf, 0x80, buffer_size);
	snd_pcm_writei (pcm, start_buf, buffer_size);
	free (start_buf);

	sound_step ();
}

double
get_time (void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec + tv.tv_usec / 1e6);
}

void
busywait (double secs)
{
	double start;
	start = get_time ();
	while (get_time () - start < secs)
		;
}

void
usage (void)
{
	fprintf (stderr, "usage: vtest idx\n");
	exit (1);
}

int
main (int argc, char **argv)
{
	int c;
	char *filename;
	int idx;
	FILE *f;
	char *rawbuf;
	FILE *outf;
	int i;

	struct TM {
		long offset;
		long size;
	} ABSnd;

	filename = "/l/baris/gamedat/unews.cdr";

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	idx = atoi (argv[optind++]);

	if (optind != argc)
		usage ();

	if ((f = fopen (filename, "rb")) == NULL) {
		fprintf (stderr, "can't open %s\n", filename);
		exit (1);
	}

	fseek (f, idx * sizeof (ABSnd), SEEK_SET);
	fread (&ABSnd, sizeof ABSnd, 1, f);

	printf ("offset %ld; size %ld\n", ABSnd.offset, ABSnd.size);

	if ((rawbuf = malloc (ABSnd.size)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	fseek (f, ABSnd.offset, SEEK_SET);
	fread (rawbuf, 1, ABSnd.size, f);

	outf = fopen ("x", "wb");
	fwrite (rawbuf, 1, ABSnd.size, outf);
	fclose (outf);

	outf = fopen ("y", "wb");
	for (i = 0; i < ABSnd.size; i++)
		fprintf (outf, "%d\n", rawbuf[i] & 0xff);
	fclose (outf);

	sound_init ();

	usleep (250 * 1000);

	sound_play ((unsigned char *)rawbuf, ABSnd.size);
	while (sound_step ())
		;

	usleep (250 * 1000);

	printf ("total xruns %d\n", xrun_count);

	return (0);
}

