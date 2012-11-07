#ifndef PLACE_H
#define PLACE_H

int Help(const char *FName);
void Draw_Mis_Stats(char plr, char index, int *where, char mode);
void PatchMe(char plr, int x, int y, char prog, char poff, unsigned char coff);
void BigHardMe(char plr, int x, int y, char hw, char unit, char sh, unsigned char coff);
void AstFaces(char plr, int x, int y, char face);
void SmHardMe(char plr, int x, int y, char prog, char planet, unsigned char coff);
int BChoice(char plr, char qty, char *Name, char *Imx);
int MainMenuChoice();

#endif // PLACE_H