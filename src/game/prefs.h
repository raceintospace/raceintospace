#ifndef PREFS_H
#define PREFS_H

enum {
    PREFS_ABORTED = 0,
    PREFS_SET
};


void IngamePreferences(int player);
int NewGamePreferences();
int NewPBEMGamePreferences();


#endif // PREFS_H
