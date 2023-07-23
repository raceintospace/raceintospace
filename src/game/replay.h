#ifndef REPLAY_H
#define REPLAY_H

#include <string>

void DispBaby(int x, int y, int loc, char neww);
void AbzFrame(int plr, int dx, int dy, int width, int height,
              int mission);
void AbzFrame(int plr, int dx, int dy, int width, int height,
              std::string sequence);
void Replay(char plr, int num, int dx, int dy, int width, int height,
            std::string Type);

#endif // REPLAY_H
