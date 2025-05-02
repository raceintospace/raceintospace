/*
    Copyright (C) 2007 Krzysztof Kosciuszkiewicz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// This file handles Advanced Preferences.

#include "options.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <SDL.h>

#include "Buzz_inc.h"
#include "fs.h"
#include "logging.h"
#include "macros.h"
#include "pace.h"
#include "utils.h"

#define ENVIRON_DATADIR ("BARIS_DATA")
#define ENVIRON_SAVEDIR ("BARIS_SAVE")

/*
#if CONFIG_WIN32
#  define DEFAULT_DATADIR ("c:/" PACKAGE_TARNAME )
#  define DEFAULT_SAVEDIR ("c:/" PACKAGE_TARNAME "/savedat")
#elif CONFIG_LINUX
#  define DEFAULT_DATADIR CONFIG_DATADIR
#  define DEFAULT_SAVEDIR (".")
#elif CONFIG_MACOSX
#  define DEFAULT_DATADIR CONFIG_DATADIR
#  define DEFAULT_SAVEDIR (".")
#endif
*/

/* and provide defaults for unspecified OS */
#ifndef DEFAULT_DATADIR
#  define DEFAULT_DATADIR (".")
#endif

#ifndef DEFAULT_SAVEDIR
#  define DEFAULT_SAVEDIR (".")
#endif

#if !HAVE_GETENV
#  if HAVE_SDL_GETENV
#    define getenv SDL_getenv
#  else
#    warn I do not know a way to read environment on this system
#    define getenv(a) (NULL)
#  endif
#endif

game_options options;

LOG_DEFAULT_CATEGORY(config)

/*set up array for environment vars */
static struct {
    const char *name;
    char **dest;
    const char *def_val;
} env_vars[] = {
    {ENVIRON_DATADIR, &options.dir_gamedata, DEFAULT_DATADIR},
    {ENVIRON_SAVEDIR, &options.dir_savegame, DEFAULT_SAVEDIR},
};

static const struct {
    const char *name; /**< name of option */
    const void *dest; /**< pointer to the variable holding the content */
    const char *format; /**< scanf format of the data we get */
    int need_alloc; /**< max memory size to be allocated for value */
    const char *comment; /**< a note to the user */
} config_strings[] = {
    {
        "datadir", &options.dir_gamedata, "%1024[^\n\r]", 1025,
        "Path to directory with game data files (only if different from default location)."
    },
    {
        "audio", &options.want_audio, "%u", 0,
        "Set to 0 if you don't want audio in the game."
    },
    {
        "nofail",  &options.want_cheats, "%u", 0,
        "Set to 1 if you want every mission step check to succeed."
    },
    {
        "intro", &options.want_intro, "%u", 0,
        "Set to 0 if do not want intro displayed at startup."
    },
    {
        "fullscreen", &options.want_fullscreen, "%u", 0,
        "Set to 1 if you want to display the game at full-screen."
    },
    {
	"xscale",  &options.want_4xscale, "%u", 0,
	"By default now the game is displayed at 4x scale."
	"\n# Set to 0 if you want to display the game at the classic 2x scale."
    },
    {
        "debuglevel", &options.want_debug, "%u", 0,
        "Set to positive values to increase debugging verbosity."
    },
    {
        "game_style", &options.classic, "%u", 0,
        "Set to 1 to play the game in the classic style."
        "\n# This will override other configuration options in-game,"
        " forcing them\n# to their 'classic' setting."
    },
    {
        "short_training", &options.feat_shorter_advanced_training, "%u", 0,
        "Set to non-zero to shorten Advanced Training duration from 4 to 3 seasons (experimental)."
    },
    {
        "female_nauts", &options.feat_female_nauts, "%u", 0,
        "Set to determine when female astronauts may be recruited:"
        "\n#   0  Classic (only after the newscast that requires it)"
        "\n#   1  Always allow recruitment (newscast will still require it)"
    },
    {
        "random_nauts", &options.feat_random_nauts, "%u", 0,
        "Set to non-zero to enable randomization of 'nauts for the AI."
    },  //Naut Randomize, Nikakd, 10/8/10
    {
        "compt_nauts", &options.feat_compat_nauts, "%u", 0,
        "Set the compatibility level of 'nauts (10 is default, 1 complete)."
    }, //Naut Compatibility, Nikakd, 10/8/10
    {
        "no_c_training", &options.feat_no_cTraining, "%u", 0,
        "Crews that have just been assigned are in Training status. By default, newly"
        "\n# assembled crews (i.e., those in Training) are not required to wait a turn"
        "\n# before being assigned to a mission."
        "\n# Set to zero to disable assigning crews in Training to a mission (Classic setting)."
    },   //No Capsule Training, Nikakd, 10/8/10
    {
        "no_backup", &options.feat_no_backup, "%u", 0,
        "Set to zero to require assigning a Backup crew (Classic setting)."
    },   // No Backup crew required -Leon
    {
        "show_recruit_stats", &options.feat_show_recruit_stats, "%u", 0,
        "By default, astronaut/cosmonaut candidate stats are revealed based on"
        "\n# Astronaut difficulty level. Set to 0 to restore the classic setting"
        "\n# where only Capsule and Endurance are shown."
    },   // Depending on difficulty, show recruit's Docking, EVA, LM
    {
        "use_endurance", &options.feat_use_endurance, "%u", 0,
        "By default, crew's endurance is added to duration steps of missions,"
        "\n# and when a newscast has someone retire or be injured, high endurance"
        "\n# makes them less likely to retire."
        "\n# Set to 0 to disable (Classic setting)."

    },
    {
        "cheat_no_damage", &options.cheat_no_damage, "%u", 0,
        "Set to non-zero to disable damaged equipment - will not affect equipment already damaged."
        "\n# ('Damaged' means you have to pay xMB to avoid a y% penalty on its next mission.)"
    },
    {
        "no_money_cheat", &options.no_money_cheat, "%u", 0,
        "Set to 1 to disallow the money cheat in Purchasing (Classic setting)."
        "\n# (By default, in single-player games, pressing $ in Purchasing raises your Cash by 100MB.)"
    },
    /* These two removed because they were never implemented, per #520
        {
            "random_eq", &options.feat_random_eq, "%u", 0,
            "Set to non-zero to enable random equipment model (will break game balance and possibly break the AI)."
        },
        {
            "eq_name_change", &options.feat_eq_new_name, "%u", 0,
            "Set to non-zero to be able to change equipment name when starting a new game."
        },
    */
    {
        "atlasLunar", &options.cheat_atlasOnMoon, "%u", 0,
        "Set to non-zero to enable Atlas/R-7 rockets on lunar missions."
        "\n# (By default, leaving Earth orbit requires at least a Titan/Proton.)"
    },
    {
        "succesRDMax", &options.cheat_addMaxS, "%u", 0,
        "By default, a component used successfully on a mission will gain +1% Max R&D."
        "\n# This lets you research back to where you started if hardware is downgraded"
        "\n# by a newscast, and lets you improve your hardware's Max R&D even if it hasn't"
        "\n# reached that yet."
        "\n# Set to zero to make MaxRD never change (Classic setting)."
    },
    {
        "boosterSafety", &options.boosterSafety, "%u", 0,
        "Determine how Safety of boosted rockets is calculated:"
        "\n#   0: Statistical Safety (default) - Boosted rocket = rocket * booster, e.g. .87 * .85"
        "\n#   1: Minimum Safety - Boosted rocket = rocket or booster, whichever is lower"
        "\n#   2: Average Safety (Classic setting) - Boosted rocket = average of rocket & booster"
    },
};


namespace
{
void ResetToDefaultOptions();
void ResetToClassicOptions();
};


/** prints the minimal usage information to stderr
 *
 * \param fail sets the exit code
 */
static void
usage(int fail)
{
    fprintf(stderr, "usage:   raceintospace [options...]\n"
            "options: -a -i -f -s -v -n\n"
            "\t-v verbose mode\n\t\tadd this several times to get to DEBUG level\n"
            "\t-f fullscreen mode\n"
	    "\t-s 4x scale mode\n"
           );
    exit((fail) ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void
shift_argv(char **argv, int len, int shift)
{
    int i = 0;

    assert(shift >= 0);
    assert(len >= 0);

    for (i = shift; i < len; ++i) {
        argv[i - shift] = argv[i];
        argv[i] = NULL;
    }
}

static int
skip_past_newline(FILE *f)
{
    assert(f);
    return fscanf(f, "%*[^\r\n] ");
}

static int
parse_var_value(FILE *f, int index)
{
    char format[128];
    int need_alloc;
    int res = 0, chars = 0, i = index;

    assert(f);
    assert(i >= 0 && i < (int)ARRAY_LENGTH(config_strings));

    need_alloc = config_strings[i].need_alloc;
    snprintf(format, sizeof(format), " %s%%n", config_strings[i].format);

    if (need_alloc > 0) {
        /* config_strings[].dest points to a pointer */
        void **target = (void **)config_strings[i].dest;

        *target = xrealloc(*target, need_alloc);

        res = fscanf(f, format, *target, &chars);

        if (res < 1) {
            return -1;
        }

        if (chars < need_alloc) {
            *target = xrealloc(*target, chars + 1);
        }
    } else {
        /* config_strings[].dest points to a value */
        const void *target = config_strings[i].dest;

        res = fscanf(f, format, target, &chars);

        if (res < 1) {
            return -1;
        }
    }

    return 0;
}

/** read the config file
 *
 *
 *
 * \return -1 if the config file is unavailable
 */
static int
read_config_file(void)
{
    FILE *f = open_savedat("config", "rt");
    char config_word[32 + 1];
    int err = 0, res = 0, i = 0;
    char c[2];

    if (!f) {
        INFO1("could not open config file");
        return -1;
    }

    while (1) {
        /* skip comments */
        if ((res = fscanf(f, " %1[#]", c)) == 1) {
            goto skip_newline;
        }

        if (res == EOF) {
            break;
        }

        /* get configuration variable name */
        /** \note config variables may be 32 characters of alphas plus dash and underscore */
        res = fscanf(f, "%32[a-zA-Z_-]", config_word);

        /* do we have a match? */
        if (res == 1) {
            for (i = 0; i < (int) ARRAY_LENGTH(config_strings); ++i) {
                if (strcmp(config_word, config_strings[i].name) == 0) {
                    res = parse_var_value(f, i);

                    if (res != 0 && feof(f)) {
                        goto skip_newline;
                    } else if (res != 0) {
                        NOTICE2("wrong value type for variable `%s'",
                                config_word);
                        goto skip_newline;
                    } else {
                        break;
                    }
                }
            }

            /* none matched */
            if (i == (int) ARRAY_LENGTH(config_strings)) {
                NOTICE2("unknown variable in file `%s'",
                        config_word);
                goto skip_newline;
            }
        } else if (res == EOF) {
            break;
        } else {
            NOTICE1("expected variable name");
            goto skip_newline;
        }

skip_newline:

        if (EOF == skip_past_newline(f)) {
            break;
        }
    }

    err = !feof(f);

    fclose(f);

    return -err;
}

static int
write_default_config(void)
{
    int i = 0;
    int err = 0;
    FILE *f = NULL;

    create_save_dir();
    f = open_savedat("config", "wt");

    if (!f) {
        WARNING4("can't write defaults to file `%s/%s': %s\n",
                 options.dir_savegame, "config", strerror(errno));
        return -1;
    } else
        NOTICE3("written defaults to file `%s/%s'",
                options.dir_savegame, "config");

    fprintf(f, "# This is the Advanced Configuration file for %s\n",
            PACKAGE_STRING);
    fprintf(f, "# Comments start with '#' and span the whole line.\n");
    fprintf(f, "# Active (non-comment) lines should look like:\n\n");
    fprintf(f, "# variable_name variable_value\n\n");
    fprintf(f, "# (be sure to remove the #), e.g.:\n");
    fprintf(f, "# female_nauts 1\n");
    fprintf(f, "# (but without the # and space at the beginning)\n\n");
    fprintf(f, "# 'Classic' settings return to how the game worked in BARIS.\n");
    fprintf(f, "# If you should want to return this file to default settings, you can\n");
    fprintf(f, "# delete it and the game will create a fresh one next time it opens.\n\n\n");

    for (i = 0; i < (int) ARRAY_LENGTH(config_strings); ++i) {
        fprintf(f, "# %s\n# %s\n\n",
                config_strings[i].comment, config_strings[i].name);
    }

    err = ferror(f);

    if (err) {
        WARNING2("read error: %s", strerror(errno));
    }

    fclose(f);
    return err;
}

/* return the location of user's home directory, or NULL if unknown.
 * returned string is malloc-ed */
static char *get_homedir(void)
{
    char *s = NULL;

    if ((s = getenv("HOME"))) {
        return xstrdup(s);
    }

#if CONFIG_WIN32

    if ((s = getenv("HOMEPATH"))) {
        char *s2 = NULL;
        std::string path(s);

        if ((s2 = getenv("HOMEDRIVE")) || (s2 = getenv("HOMESHARE"))) {
            std::string drive(s2);
            drive += path;
            return xstrdup(drive.c_str());
        }
    }

    if ((s = getenv("USERPROFILE"))) {
        return xstrdup(s);
    }

#endif
    return NULL;
}

static void
fixpath_options(void)
{
    fix_pathsep(options.dir_savegame);
    fix_pathsep(options.dir_gamedata);
}


namespace
{

/**
 * Set the game options to the RIS default values.
 *
 * These are the default options used if no configuration option
 * is set or no configuration file is found.
 */
void ResetToDefaultOptions()
{
    // Technical aspects
    options.want_audio = 1;
    options.want_intro = 1;
    options.want_cheats = 0;
    options.want_fullscreen = 0;
    options.want_4xscale = 1;
    options.want_debug = 0;

    // Gameplay aspects
    options.classic = 0;
    options.feat_shorter_advanced_training = 0;
    options.feat_female_nauts = 0;
    //Naut Randomize, Nikakd, 10/8/10
    options.feat_random_nauts = 0;
    //Naut Compatibility, Nikakd, 10/8/10
    options.feat_compat_nauts = 10;
    //No Capsule Training, Nikakd, 10/8/10
    options.feat_no_cTraining = 1;
    //No Backup crew required -Leon
    options.feat_no_backup = 1;
    options.no_money_cheat = 0;
    options.feat_show_recruit_stats = 1;
    options.feat_use_endurance = 1;
    options.feat_random_eq = 0;
    options.feat_eq_new_name = 0;
    options.boosterSafety = 0;

    // Cheats
    //Damaged Equipment Cheat, Nikakd, 10/8/10
    options.cheat_no_damage = 0;
    options.cheat_atlasOnMoon = 0;
    options.cheat_addMaxS = 1;
}


/**
 * Tell RIS to mimic the original game behavior.
 *
 * Disables many of the added features and adjustments made over time
 * by the Race Into Space team in favor of the original product.
 * This does not disable cheats.
 */
void ResetToClassicOptions()
{
    options.boosterSafety = 2;
    options.feat_shorter_advanced_training = 0;
    options.feat_female_nauts = 0;
    options.feat_compat_nauts = 10;
    options.feat_no_cTraining = 0;
    options.feat_no_backup = 0;
    options.no_money_cheat = 1;
    options.feat_show_recruit_stats = 0;
    options.feat_use_endurance = 0;

    // These may not be implemented, but disable anyways...
    options.feat_random_nauts = 0;
    options.feat_random_eq = 0;
    options.feat_eq_new_name = 0;
}

};  // End of anonymous namespace


/** read the commandline options
 *
 * \return length of modified argv
 *
 * \todo possibly maintain a list of dirs to search??
 */
int
setup_options(int argc, char *argv[])
{
    char *str = NULL;
    int pos, i;

    /* first set up defaults */
    for (i = 0; i < (int) ARRAY_LENGTH(env_vars); ++i) {
        if ((str = getenv(env_vars[i].name))) {
            *env_vars[i].dest = xstrdup(str);
        } else if (strcmp(env_vars[i].name, ENVIRON_SAVEDIR) == 0
                   && (str = get_homedir())) {
            size_t len = strlen(str) + strlen(PACKAGE_TARNAME) + 3;

            *env_vars[i].dest = (char *)xmalloc(len);
            snprintf(*env_vars[i].dest, len, "%s/.%s",
                     str, PACKAGE_TARNAME);
            free(str);
        } else {
            *env_vars[i].dest = xstrdup(env_vars[i].def_val);
        }
    }

    ResetToDefaultOptions();

    fixpath_options();

    /* now try to read config file, if it exists */
    if (read_config_file() < 0) {
        /* if not, then write default config template */
        write_default_config();
    }

    /* first pass: command line options */
    for (pos = 1; pos < argc; ++pos) {
        str = argv[pos];

        if (str[0] != '-') {
            continue;
        }

        if (!str[1]) {
            continue;
        }

        if (strcmp(str, "--") == 0) {
            shift_argv(argv + pos, argc - pos, 1);
            argc--;
            break;
        }

        /* check what option matches */
        if (strcmp(str, "-h") == 0) {
            usage(0);
        } else if (strcmp(str, "-i") == 0) {
            options.want_intro = 0;
        } else if (strcmp(str, "-n") == 0) {
            options.want_cheats = 1;
        } else if (strcmp(str, "-a") == 0) {
            options.want_audio = 0;
        } else if (strcmp(str, "-f") == 0) {
            options.want_fullscreen = 1;
	} else if (strcmp(str, "-s") == 0) {
            options.want_4xscale = 0;
        } else if (strcmp(str, "-v") == 0) {
            options.want_debug++;
        } else {
            ERROR2("unknown option %s", str);
            usage(1);
        }

        shift_argv(argv + pos, argc - pos, 1);
        argc--;
        pos--;  /* for loop will advance it again */
    }

    /* second pass: variable assignments */
    for (pos = 1; pos < argc; ++pos) {
        /** \todo should use PATH_MAX or something similar here */
        char name[32 + 1], *value;
        int offset = 0;
        int fields = 0;

        fields = sscanf(argv[pos], "%32[A-Z_]=%n", name, &offset);

        /* it is unclear whether %n increments return value */
        if (fields < 1) {
            continue;
        }

        value = argv[pos] + offset;

        for (i = 0; i < (int) ARRAY_LENGTH(env_vars); ++i) {
            if (strcmp(name, env_vars[i].name) == 0) {
                free(*env_vars[i].dest);
                *env_vars[i].dest = xstrdup(value);
                break;
            }
        }

        if (i == (int) ARRAY_LENGTH(env_vars)) {
            WARNING2("unsupported command line variable `%s'", name);
        }

        /* remove matched string from argv */
        shift_argv(argv + pos, argc - pos, 1);

        /*
         * now we have one fewer arg, pos points to the next arg,
         * keep it this way after incrementing it on top of the loop
         */
        pos--;
        argc--;
    }

    fixpath_options();

    if (options.classic) {
        ResetToClassicOptions();
    }

    return argc;
}

/* vim: set noet ts=4 sw=4 tw=77: */
