#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int vflag;

int
RLED(void *src_raw, void *dest_raw, unsigned int src_size)
{
	signed char *src = src_raw;
	signed char *dest = dest_raw;
	unsigned short used;
	short count, val;
	short i;

	used = 0;
	while (used < src_size)
	{
		count = src[used++];

		if (count < 0)
		{
			count = -(count - 1);
			val = src[used++];

			if (vflag)
				printf("%6d: repeat %02x\n", count, val & 0xff);

			for (i = 0; i < count; i++)
				*dest++ = val;
		}
		else
		{
			count++;

			if (vflag)
				printf("%6d:", count);

			for (i = 0; i < count; i++)
			{
				if (vflag)
					printf(" %02x", src[used] & 0xff);
				*dest++ = src[used++];
			}

			if (vflag)
				printf("\n");
		}
	}

	return ((void *) dest - (void *) dest_raw);
}

void
usage(void)
{
	fprintf(stderr,
		"usage: mkmovie [-d output_dir] [-b output_basename] inputfile\n");
	exit(EXIT_FAILURE);
}

struct frm
{
	FILE *fin;
	int next_frame_chunks;
	int frame_idx;
	int nframes;
	int frame_rate;
	unsigned char pal[768];
};

struct frm *
frm_open(char *filename)
{
	struct frm *frm;
	FILE *fin;

	if ((fin = fopen(filename, "rb")) == NULL)
		return (NULL);

	if ((frm = calloc(1, sizeof *frm)) == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}

	frm->fin = fin;
	frm->next_frame_chunks = 8;
	frm->frame_idx = 0;

	return (frm);
}

void
frm_close(struct frm *frm)
{
	if (frm)
	{
		if (frm->fin)
			fclose(frm->fin);
		free(frm);
	}
}

int
frm_get2(struct frm *frm, void *pixels_arg, void *map)
{
	unsigned char raw[64 * 1024];
	unsigned char pbuf[64 * 1024];
	unsigned int n;
	int val0, val1;
	unsigned char *pixels;

	n = frm->next_frame_chunks * 2048;

	if (n == 0)
	{
		int count;

		count = 0;
		while (getc(frm->fin) != EOF)
			count++;
		// printf ("trailing bytes %d\n", count);
		return (0);
	}

	if (n > sizeof raw)
		return (-1);

	fread(raw, 1, n, frm->fin);
	val0 = raw[0] | (raw[1] << 8);
	val1 = raw[2];

	if (frm->next_frame_chunks == 8)
	{
		if (frm->frame_idx == 0)
		{
			frm->nframes = val0;
			frm->frame_rate = raw[15998] | (raw[15999] << 8);
			if (frm->frame_rate == 0)
				frm->frame_rate = 8;
			memcpy(frm->pal + 384, raw + 16000, 384);
		}
		pixels = raw;
	}
	else
	{
		int compressed_size = val0;

		RLED(raw + 3, pbuf, compressed_size);
		pixels = pbuf;
	}

	memcpy(pixels_arg, pixels, 160 * 100);
	memcpy(map, frm->pal + 384, 384);

	frm->next_frame_chunks = val1;
	frm->frame_idx++;

	return (1);
}

int
main(int argc, char **argv)
{
	FILE *movief;
	int c;
	char *filename;
	char *dirname = ".";
	char *basename = "frame";
	struct frm *frm;
	int rc;
	unsigned char pixels[64 * 1000], map[768];
	char outfname[100];
	int i, num;
	int pixel;
	unsigned char *up;
	int r, g, b;

	while ((c = getopt(argc, argv, "d:b:")) != EOF)
	{
		switch (c)
		{
			case 'd':
				dirname = optarg;
				break;
			case 'b':
				basename = optarg;
				break;
			default:
				usage();
		}
	}

	if (optind >= argc)
		usage();

	filename = argv[optind++];

	if (optind != argc)
		usage();

	if ((frm = frm_open(filename)) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename);
		exit(EXIT_FAILURE);
	}

	memset(map, 0, sizeof map);
	num = 0;
	while (1)
	{
		if ((rc = frm_get2(frm, pixels, &map[384])) < 0)
		{
			printf("error reading frame\n");
			exit(EXIT_FAILURE);
		}

		if (rc == 0)
			break;
		sprintf(outfname, "%s/%s.%04d.ppm", dirname, basename, num++);

		if ((movief = fopen(outfname, "wb")) == NULL)
		{
			fprintf(stderr, "can't create %s\n", outfname);
			exit(EXIT_FAILURE);
		}

		fprintf(movief, "P6\n160 100\n255\n");

		for (i = 0; i < 160 * 100; i++)
		{
			pixel = pixels[i];
			up = &map[pixel * 3];

			r = up[0] * 4;
			g = up[1] * 4;
			b = up[2] * 4;

			putc(r, movief);
			putc(g, movief);
			putc(b, movief);
		}

		fclose(movief);
	}
	printf("%d frames written, framerate %dfps\n", num, frm->frame_rate);
	frm_close(frm);
	return (EXIT_SUCCESS);
}
