#ifndef FUTBUB_H
#define FUTBUB_H

void Draw_IJ(char w);
void Draw_IJV(char w);
void OrbOut(char a, char b, char c);
void LefEarth(char a, char b);
void OrbIn(char a, char b, char c);
void OrbMid(char a, char b, char c, char d);
void LefOrb(char a, char b, char c, char d);
void Draw_LowS(char a, char b, char c, char x, char y, char z);
void Fly_By(void);
void VenMarMerc(char x);
void Draw_PQR(void);
void Draw_PST(void);
void Draw_GH(char a, char b);
void Q_Patch(void);
void RghtMoon(char x, char y);
void DrawLunPas(char x, char y, char z, char w);
void DrawLefMoon(char x, char y);
void DrawSTUV(char x, char y, char z, char w);
void Draw_HighS(char x, char y, char z);
void DrawMoon(char x, char y, char z, char w, char j, char k, char l);
void LefGap(char x);
void S_Patch(char x);
void DrawZ(void);

extern int Bub_Count;

#endif // FUTBUB_H