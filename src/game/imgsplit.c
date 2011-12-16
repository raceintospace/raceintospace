#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
usage (void)
{
	fprintf (stderr, "usage: imgsplit\n");
	exit (1);
}

int
PCX_D (char *src,char *dest,unsigned src_size)
{
  char num;
  char *orig_dest = dest;
  do {
    if ((*src&0xc0)==0xc0) {
      num=*(src++)&0x3f; src_size--;
      while ((num--)>0) {*(dest++)=*src;}
      src++;src_size--;
    } else {
      (*dest++)=*(src++);src_size--;
    };
  }  while(src_size);
  return (dest - orig_dest);
}

int
read_img_frame (FILE *inf, char *rgb)
{
	unsigned char map[256 * 3];
	unsigned int len;
	char compressed[64 * 1024];
	int idx;
	unsigned char *up;
	char ipix[320 * 200];

	len = fread (&map, 1, sizeof map, inf);

	if (len == 0)
		return (-1);

	if (len != sizeof map)
		goto bad;
	
	if (fread (&len, 4, 1, inf) != 1)
		goto bad;

	if (len > sizeof compressed) {
		fprintf (stderr, "frame too big\n");
		goto bad;
	}

	if (fread (compressed, 1, len, inf) != len)
		goto bad;
		
	PCX_D (compressed, ipix, len);

	for (idx = 0; idx < 320 * 200; idx++) {
		up = map + (ipix[idx] & 0xff) * 3;
		*rgb++ = *up++ * 4;
		*rgb++ = *up++ * 4;
		*rgb++ = *up++ * 4;
	}

	return (0);

bad:
	fprintf (stderr, "bad img file\n");
	exit (1);
}

int
main (int argc, char **argv)
{
	int c;
	char *inname;
	FILE *inf;
	int fnum;
	char rgb[320 * 200 * 3];
	char outname[1000];
	FILE *outf;

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	inname = argv[optind++];

	if (optind != argc)
		usage ();

	if ((inf = fopen (inname, "rb")) == NULL) {
		fprintf (stderr, "can't open %s\n", inname);
		exit (1);
	}

	fnum = 0;

	while (1) {
		fnum++;

		if (read_img_frame (inf, rgb) < 0)
			break;
		
		sprintf (outname, "c%02d.ppm", fnum);
		if ((outf = fopen (outname, "wb")) == NULL) {
			fprintf (stderr, "can't create %s\n", outname);
			exit (1);
		}
		fprintf (outf, "P6\n320 200\n255\n");
		fwrite (rgb, 1, 3 * 320 * 200, outf);
		fclose (outf);
	}

	return (0);
}

