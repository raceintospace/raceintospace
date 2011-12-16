#include <stdio.h>
#include <string.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <dir.h>
#include <ctype.h>
#include <mem.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloc.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <bios.h>
#include <math.h>
#include <sys/stat.h>
#include <process.h>


/*
  Utility function to decrypt old BARIS save games with old BC31 libs

  Quick and dirty, be happy to have it
*/

#define __VERSION__ "0.5"
#define BUFFER_SIZE 32000L

struct SF {
  char ID[4],Name[23],PName[2][20],Country[2],Season,Year;
  unsigned int dSize,fSize;		// these are 16 bits in DOS-land
} FDes;

int
main(int argc, char **argv)
{
  FILE *fin = NULL;
  FILE *fout = NULL;

  unsigned long bytes = 0;
  unsigned char buffer[BUFFER_SIZE];

  printf("BARIS Save game decryption utility %s\n",__VERSION__);

  if (argc != 3)
  {
	printf("Usage: %s <infile> <outfile>\n\n");
	return EXIT_FAILURE;
  }

  printf("Reading BARIS Save game %s\n",argv[1]);
  printf("Writing to %s\n",argv[2]);

  fout=fopen(argv[2],"wb");

  fin=fopen(argv[1],"rb");

  bytes = fread(&FDes,1,sizeof (FDes), fin);
  printf("Signature is %4s\n",FDes.ID);
  FDes.ID[0] = 'P';
  FDes.ID[1] = 'S';
  FDes.ID[2] = 'i';
  FDes.ID[3] = 'R';

  bytes = fwrite(&FDes, 1, sizeof (FDes), fout);

  bytes = fread((char*)buffer, 1, FDes.fSize, fin);

  // No BARIS structs to compare against so trust it
  //if (FDes->dSize==sizeof(struct Players))
  {
	unsigned long i = 0;
	unsigned long left = 0;
	srand(FDes.fSize);		// Randomize based on compressed length

	// Decrypt magic
	for( i = 0; i < FDes.fSize ; i++ )
	  buffer[i] ^= random(256);

	if (strncmp(buffer+1,"BOLO",4)!=0)
	{
		printf("%s is not an encrypted BARIS Save game\n");
		return EXIT_FAILURE;
	}

	bytes = fwrite((char*)buffer,1,FDes.fSize, fout);

	// Copy remainder of save file
	left = BUFFER_SIZE;
	while (left==BUFFER_SIZE) {
	  left=fread((char*)buffer,1,BUFFER_SIZE,fin);
	  bytes = fwrite((char*)buffer,1,left,fout);
	}

	fclose(fin);
	fclose(fout);
  }

  return EXIT_SUCCESS;
}

