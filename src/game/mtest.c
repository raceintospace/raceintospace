#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void
dump (void *buf, int n)
{
	int i;
	int j;
	int c;

	for (i = 0; i < n; i += 16) {
		printf ("%04x: ", i);
		for (j = 0; j < 16; j++) {
			if (i+j < n)
				printf ("%02x ", ((unsigned char *)buf)[i+j]);
			else
				printf ("   ");
		}
		printf ("  ");
		for (j = 0; j < 16; j++) {
			c = ((unsigned char *)buf)[i+j] & 0x7f;
			if (i+j >= n)
				putchar (' ');
			else if (c < ' ' || c == 0x7f)
				putchar ('.');
			else
				putchar (c);
		}
		printf ("\n");

	}
}


#pragma pack(1)
struct TM {
	char ID[6];
	long offset;
	long size;
};


struct music_dir {
	struct TM dir;
	int nfiles;
	struct TM *files;
};

FILE *musicf;
#define MDIRS 4 
struct music_dir dirs[MDIRS];

int
open_music_cat (char *filename)
{
	int i;
	struct music_dir *dp;

	if ((musicf = fopen (filename, "rb")) == NULL)
		return (-1);

	for (i = 0, dp = dirs; i < MDIRS; i++, dp++)
		fread (&dp->dir, 1, sizeof dp->dir, musicf);
		
	for (i = 0, dp = dirs; i < MDIRS; i++, dp++) {
		printf ("%6.6s 0x%08lx %8ld\n", dp->dir.ID,
			dp->dir.offset, dp->dir.size);

		dp->nfiles = 32;
		dp->files = calloc (dp->nfiles, sizeof *dp->files);
		fseek (musicf, dp->dir.offset, SEEK_SET);
		fread (dp->files, dp->nfiles, sizeof *dp->files, musicf);
	}

	return (0);
}

int
get_music (char *filetype, int idx, void *buf, int bufsize)
{
	int len;
	int i;
	struct music_dir *dp;
	int n;

	len = strlen (filetype);

	for (i = 0, dp = dirs; i < MDIRS; i++, dp++) {
		if (xstrncasecmp (dp->dir.ID, filetype, len) == 0)
			break;
	}

	if (i == MDIRS)
		return (-1);

	if (idx < 0 || idx >= dp->nfiles)
		return (-1);

	n = dp->files[idx].size;
	if (n > bufsize)
		n = bufsize;

	fseek (musicf, dp->dir.offset + dp->files[idx].offset, SEEK_SET);
	n = fread (buf, 1, n, musicf);
	return (n);
}

void
do_display_cat (void)
{
	int i, j;
	struct music_dir *dp;
	struct TM *fp;
	long base, offset;

	base = 0;
	for (i = 0, dp = dirs; i < MDIRS; i++, dp++) {
		for (j = 0, fp = dp->files; j < dp->nfiles; j++, fp++) {
			offset = dp->dir.offset + fp->offset;

			printf ("0x%08lx %8ld %.6s/%.6s\n",
				offset, fp->size,
				dp->dir.ID, fp->ID);
		}
	}
}

int display_cat;

void
usage (void)
{
	fprintf (stderr, "usage: mtest\n");
	exit (1);
}

int
main (int argc, char **argv)
{
	int c;
	char *type = NULL;
	int idx = 0;
	int n;
	char buf[1000 * 1000];

	while ((c = getopt (argc, argv, "c")) != EOF) {
		switch (c) {
		case 'c':
			display_cat = 1;
			break;
		default:
			usage ();
		}
	}

	if (optind < argc)
		type = argv[optind++];

	if (optind < argc)
		idx = atoi (argv[optind++]);

	if (optind != argc)
		usage ();

	open_music_cat ("/l/baris/gamedat/music.cat");

	if (display_cat) {
		do_display_cat ();
		exit (0);
	}

	if (type) {
		n = get_music (type, idx, buf, sizeof buf);
		dump (buf, n);
	}

	return (0);
}


