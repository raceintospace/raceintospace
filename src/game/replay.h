#ifndef REPLAY_H
#define REPLAY_H

#include <string>

void DispBaby(int x, int y, int loc, char neww);
void AbzFrame(char plr, int num, int dx, int dy, int width, int height,
              const char *Type, char mode);
void Replay(char plr, int num, int dx, int dy, int width, int height,
            std::string Type);

#endif // REPLAY_H
