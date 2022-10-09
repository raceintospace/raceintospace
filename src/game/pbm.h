#ifndef PBM_H
#define PBM_H

#define MAIL (Data->Mail)

// First bit of MAIL: player (0: U.S., 1; Soviet)
// Second bit: turn order (0: first U.S., 1: first Soviet (i.e., inverted))
#define MAIL_PLAYER (MAIL % 2)
#define MAIL_INVERTED (MAIL == -1 ? -1 : (MAIL & 2) >> 1)

#define MAIL_OPPONENT (MAIL == -1 ? -1 : MAIL_PLAYER ^ 1)

// order is 0 1 3 2 0 ...
#define MAIL_NEXT (MAIL == -1 ? -1 : (2 * MAIL + 1 - MAIL / 2) % 4)

void ShowPrestigeResults(char plr);
void MailSwitchPlayer();
void MailSwitchEndgame(void);

#endif //PBM_H
