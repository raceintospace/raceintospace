#ifndef PBM_H
#define PBM_H

#define MAIL_OPPONENT (MAIL == -1 ? -1 : MAIL ^ 1)
#define MAIL_PLAYER (MAIL)
#define MAIL_NEXT (MAIL == -1 ? -1 : MAIL ^ 1)

void ShowPrestigeResults(char plr);
void MailSwitchPlayer();

#endif //PBM_H
