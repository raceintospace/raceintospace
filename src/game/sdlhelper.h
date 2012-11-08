#ifndef SDLHELPER_H
#define SDLHELPER_H

int IsChannelMute(int channel);
void NUpdateVoice(void);

extern unsigned char *screen;
extern unsigned char pal[3 * 256];
extern int screen_dirty;

#endif // SDLHELPER_H