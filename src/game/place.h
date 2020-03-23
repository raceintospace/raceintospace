#ifndef PLACE_H
#define PLACE_H

int Help(const char *FName);
void Draw_Mis_Stats(char plr, char index, int *where, char mode);
void PatchMe(char plr, int x, int y, char prog, char poff);
void BigHardMe(char plr, int x, int y, char hw, char unit, char sh);
void AstFaces(char plr, int x, int y, char face);
void SmHardMe(char plr, int x, int y, char prog, char planet, unsigned char coff);
int BChoice(char plr, char qty, char *Name, char *Imx);
int MainMenuChoice();
bool ScrubMissionQuery(char plr, int pad);

#endif // PLACE_H
