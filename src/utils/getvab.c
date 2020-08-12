#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>

typedef unsigned short ui16;

int vflag;

void
usage (void)
{
	fprintf (stderr, "usage: getvab filename\n");
	exit (1);
}

int
PCX_D (void *src_raw,void *dest_raw,unsigned src_size)
{
  char *src = src_raw;
  char *dest = dest_raw;
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

unsigned char cbuf[1000 * 1000];
unsigned char pixels [1000 * 1000];

int
main (int argc, char **argv)
{
	int c;
	char *filename;
	FILE *f;
	FILE *outf;
	int i;
	int pixel;
	unsigned char *up;
	int plr;
	char outname[1000];

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (argc - optind != 1)
		usage ();

	filename = argv[optind];

	if ((f = fopen (filename, "rb")) == NULL) {
		fprintf (stderr, "can't open %s\n", filename);
		exit (1);
	}

	struct bImg {
		unsigned char pal[768];
		ui16 fSize;
	} M;
	
	for (plr = 0; plr < 2; plr++) {
		fread (&M, 1, sizeof M, f);
		fread (&cbuf, M.fSize, 1, f);
		PCX_D (cbuf, pixels, M.fSize);

		sprintf (outname, "vab%d.ppm", plr);
		outf = fopen (outname, "wb");
		fprintf (outf, "P6\n320 200\n255\n");
		for (i = 0; i < 320*200; i++) {
			pixel = pixels[i];
			up = &M.pal[pixel * 3];
			putc (*up++*4, outf);
			putc (*up++*4, outf);
			putc (*up++*4, outf);
		}
		fclose (outf);
	}

	return (0);
}

