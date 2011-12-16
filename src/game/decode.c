#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

char inbuf[1000 * 1000];
char outbuf[1000 * 1000];

int insize;
int outsize;

int vflag;

long
RLEC (char *src, char *dest, unsigned int src_size)
{
	short src_i, dest_i, limit;
	short togo;
	
	dest_i = 0;
	src_i = 0;
	while ((togo = src_size - src_i) > 0) {
		int thistime;

		limit = togo - 1;
		if (limit >= 128)
			limit = 128;

		thistime = 0;
		while (thistime < limit && src[src_i] != src[src_i+1]) {
			thistime++;
			src_i++;
		}
		if (thistime) {
			dest[dest_i++] = (thistime-1);
			memcpy (&dest[dest_i], &src[src_i-thistime], thistime);
			dest_i += thistime;
		} else {
			thistime = 2;   /* at least 2 chars to be repeated */
			src_i++; /* advance pointer beyond the first match*/
			while (thistime < limit && src[src_i] == src[src_i+1]){
				thistime++;
				src_i++;
			}
			dest[dest_i++] = (-thistime+1);
			dest[dest_i++] = src[src_i++];
		}
	}
	return (dest_i);
}


int
RLED (void *src_raw, void *dest_raw, unsigned int src_size)
{
	signed char *src = src_raw;
	signed char *dest = dest_raw;
	unsigned short used;
	short count, val;
	short i;

	used = 0;
	while (used < src_size) {
		count = src[used++];
		
		if (count < 0) {
			count = -(count - 1);
			val = src[used++];
			
			if (vflag)
				printf ("%6d: repeat %02x\n", count, val&0xff);

			for (i = 0; i < count; i++)
				*dest++ = val;
		} else {
			count++;

			if (vflag)
				printf ("%6d:", count);

			for (i = 0; i < count; i++) {
				if (vflag)
					printf (" %02x", src[used] & 0xff);
				*dest++ = src[used++];
			}

			if (vflag)
				printf ("\n");
		}
	}

	return ((void *)dest - (void *)dest_raw);
}

int
RLED2 (char *src, char *dest, unsigned int src_size)
{
	short ax, bx, cx;
	char const *si;
	char *di;

	di = dest;
	si = src;
	cx = 0;
	bx = src_size;

loa:
	ax = *si++ & 0xff;
	bx--;
	if (ax & 0x80)
		goto repeat;

	ax = (ax + 1) & 0xff;
	cx = ax;

	do
		*di++ = *si++;
	while (--cx);

	bx -= ax;

	if (bx > 0)
		goto loa;
	goto bot;

repeat:
	ax = -ax;
	ax++;
	cx = ax & 0xff;

	ax = *si++ & 0xff;

lob:
	*di++ = ax;
	if (--cx)
		goto lob;
	bx--;
	if (bx > 0)
		goto loa;

bot: 
	return (di - dest);
}

void
usage (void)
{
	fprintf (stderr, "usage: decode file\n");
	exit (1);
}

int width, height;

int
main (int argc, char **argv)
{
	int c;
	char *filename;
	FILE *f;
	FILE *outf;

	while ((c = getopt (argc, argv, "vw:h:")) != EOF) {
		switch (c) {
		case 'w':
			width = atoi (optarg);
			break;
		case 'h':
			height = atoi (optarg);
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	filename = argv[optind++];

	if (optind != argc)
		usage ();

	if ((f = fopen (filename, "rb")) == NULL) {
		fprintf (stderr, "can't open %s\n", filename);
		exit (1);
	}

	fread (inbuf, 1, 80, f);
	insize = fread (inbuf, 1, sizeof inbuf, f);

	printf ("input size %d\n", insize);

	outsize = RLED (inbuf, outbuf, insize);

	printf ("output size %d\n", outsize);

	if (width && height) {
		if ((outf = fopen ("d.pgm", "wb")) == NULL) {
			fprintf (stderr, "can't create d.pgm\n");
			exit (1);
		}
		fprintf (outf, "P5\n%d %d\n255\n", width, height);
		fwrite (outbuf, 1, width * height, outf);
		fclose (outf);
	}

	return (0);
}

