#ifndef ADMIN_H
#define ADMIN_H

#include <stdint.h>

void Admin(char plr);
void CacheCrewFile();
void FileAccess(char mode);
int FutureCheck(char plr, char type);
void autosave_game(const char *name);
void BadFileType();
void CheckMSF ();

#endif // ADMIN_H
