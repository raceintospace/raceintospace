#ifndef __PACE_H__
#define __PACE_H__
#include <unistd.h>
#include <SDL.h>

void delay(int millisecs);
void FadeIn(char wh, void *palx, int steps, int val, char mode);
void FadeOut(char wh, void *palx, int steps, int val, char mode);
int PCX_D(void *src, void *dest, unsigned src_size);
int brandom(int limit);
int RLED_img(void *src, void *dest, unsigned int src_size, int w, int h);
char *seq_filename(int seq, int mode);
void idle_loop_secs(double secs);
int getch(void);
void PlayAudio(char *name, char mode);
char DoModem(int sel);
void MesCenter(void);
void StopAudio(char mode);
void StopVoice(void);
void NGetVoice(char plr, char val);
void PlayVoice(void);
void KillVoice(void);
ssize_t load_audio_file(const char *, char **data, size_t *size);
void idle_loop(int ticks);
void play_audio(int sidx, int mode);
void bzdelay(int ticks);
int32_t RLEC(char *src, char *dest, unsigned int src_size);
int RLED(void *src, void *dest, unsigned int src_size);
char MPrefs(char mode);
int bioskey(int wait);

extern struct Prest_Upd MP[3];
extern int show_intro_flag;
extern char *letter_dat;


#endif /* __PACE_H__ */
