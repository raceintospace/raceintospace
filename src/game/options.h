#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct {
    char *dir_savegame;
    char *dir_gamedata;
    unsigned want_audio;
    unsigned want_fullscreen;
    unsigned want_intro;
    unsigned want_cheats;
    unsigned want_debug;
    unsigned feat_shorter_advanced_training;
    unsigned feat_random_nauts;
    unsigned feat_compat_nauts;
    unsigned feat_no_cTraining;
    unsigned feat_no_backup;
    unsigned cheat_no_damage;
    unsigned feat_random_eq;
    unsigned feat_eq_new_name;
    unsigned cheat_altasOnMoon;
    unsigned cheat_addMaxS;
    unsigned boosterSafety;
} game_options;

extern game_options options;
extern int setup_options(int argc, char ** argv);

#endif /* _OPTIONS_H */
