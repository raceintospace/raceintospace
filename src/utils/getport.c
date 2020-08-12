#include <Buzz_inc.h>

struct FHead {
   char Text[28];  // File Copyright Notice
   long oMObj;   // Offset to MObj data table
   long oTab;    // Offset to Table of data
   long oPal;    // Offset to Palette
   long oPort;   // Offset to start of Port Images
   long oMse;    // Offset to Mouse Objects
   long oOut;    // Offset to port Outlines
   long oAnim;   // Offset to start of Port Anims
   } PHead;

typedef struct cBoxx {
   i16 x1,y1,x2,y2;
   } BOUND;

typedef struct Img {
   long Size;         // Size of Image (bytes)
   char Comp;         // Type of Compression Used
   i16 Width;         // Width of Image
   i16 Height;        // Height of Image
   i16 PlaceX;        // Where to Place Img:X
   i16 PlaceY;        // Where to Place Img:Y
   } IMG;

typedef struct region {
   char qty;          // number of BOUNDS
   BOUND CD[4];       // boundry coords for mouse location
   char iNum;         // 
   char sNum;         // value for surround box
   char PreDraw;      // Code of Special to Draw first
   } REGION;


typedef struct mobj
   {
   char Name[30];       // Name of region
   char qty;            // Nunber of regions
   char Help[3];       // Help Box Stuff
   REGION Reg[4];       // At Max 4 regions
   } MOBJ;

#define S_QTY 43

MOBJ MObj[35];

long table[S_QTY];

char pal[256 * 3];

char inbuf[1000 * 1000];
unsigned char outbuf[1000 * 1000];

int insize;
int outsize;

int vflag;

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


void
usage (void)
{
	fprintf (stderr, "usage: getport idx\n");
	exit (1);
}

int width, height;

IMG Img;

unsigned char *screen;

void
write_ppm (char *filename, char unsigned *buf, int w, int h)
{
	FILE *outf;
	int row, col;
	int pixel;
	unsigned char *cp;

	if ((outf = fopen (filename, "wb")) == NULL) {
		fprintf (stderr, "can't create %s\n", filename);
		exit (1);
	}

	fprintf (outf, "P6\n%d %d\n255\n", w, h);

	for (row = 0; row < h; row++) {
		for (col = 0; col < w; col++) {
			pixel = buf[row * w + col] & 0xff;

			cp = (unsigned char *)(&pal[pixel * 3]);
			putc (*cp++ * 4, outf);
			putc (*cp++ * 4, outf);
			putc (*cp++ * 4, outf);
		}
	}

	fclose (outf);
}

/* pace */
long fix_width[] = {
	// normal
	80615, // 4/0 VAB in mode 0
	101712, // 8/1 Medical center
	115262, // 16/1 L.M. Program
	113283, // 18/1 XMS-2 program
	112708, // 19/1 Apollo program
	112154, // 20/1 Gemini program
	86035, // 22/1 Research and development
	118301, // 33/1 Tracking station
	82541, // 4/2 vab
	88992, // 22/2 research and development
	92114, // 22/3 research and development

	// predraw 
	79703, // 15-1-1 "SATELLITE PROGRAMS" 
	0
};

int
need_to_fix_width (long table)
{
	int i;
	for (i = 0; fix_width[i]; i++) {
		if (fix_width[i] == table)
			return (1);
	}
	return (0);
}

FILE *fin;

void
do_item (int fm, int idx, int predraw, long off)
{
	char fname[1000];

	printf ("%02d-%d-%d \"%s\" off %ld\n", fm, idx, predraw, MObj[fm].Name, off);

	fseek (fin, off, SEEK_SET);
	fread (&Img, sizeof Img, 1, fin);

	if (need_to_fix_width (off))
		Img.Width++;

	fread (inbuf, Img.Size, 1, fin);
	RLED (inbuf, outbuf, Img.Size);
			
	sprintf (fname, "f%02d-%d-%d.ppm", fm, idx, predraw);
	write_ppm (fname, outbuf, Img.Width, Img.Height);
}

int
main (int argc, char **argv)
{
	int c;
	char *filename;
	int fm;
	int idx;

	filename = "/l/baris/gamedat/usa_port.dat";
	screen = (unsigned_char *)xmalloc(64 * 1024);

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

	if (optind != argc)
		usage ();

	if ((fin = fopen (filename, "rb")) == NULL) {
		fprintf (stderr, "can't open %s\n", filename);
		exit (1);
	}

	fread(&PHead,sizeof PHead,1,fin);
	fread(&MObj[0],sizeof MObj,1,fin);
	fread(&table[0],sizeof table,1,fin);
	fseek(fin,PHead.oPal,SEEK_SET);
	fread(&pal[0],768,1,fin);

	fseek(fin,table[0],SEEK_SET);
	fread(&Img,sizeof Img,1,fin);  // Read in main image Header
	fread((char *)screen,Img.Size,1,fin);  // Read in main image

	write_ppm ("p.ppm", screen, 320, 200);

	for (fm=0;fm<35;fm++) {
		for (idx = 0; idx < 4; idx++) {
			if (MObj[fm].Reg[idx].PreDraw>0)   // PreDrawn Shape
				do_item (fm, idx, 1, table[MObj[fm].Reg[idx].PreDraw]);

			if (MObj[fm].Reg[idx].iNum>0)   // Actual Shape
				do_item(fm, idx, 0, table[MObj[fm].Reg[idx].iNum]);
			
		}
	}

	return (0);
}

