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
#include <algorithm>
#include <string>

#include <SDL.h>

#include "Buzz_inc.h"
#include "fs.h"
#include "logging.h"
#include "macros.h"
#include "pace.h"
#include "utils.h"

static std::string get_homedir();

/* if paths aren't provided at compile-time, set up defaults */
#ifndef DEFAULT_DATADIR
#  define DEFAULT_DATADIR (".")
#endif

#ifndef DEFAULT_SAVEDIR
#  define DEFAULT_SAVEDIR (get_homedir() + "/." + PACKAGE_TARNAME)
#endif

#if !HAVE_GETENV
#  if HAVE_SDL_GETENV
#    define getenv SDL_getenv
#  else
#    warn I do not know a way to read environment on this system
#    define getenv(a) (NULL)
#  endif
#endif

game_options options{};

LOG_DEFAULT_CATEGORY(config)

static struct Env_Conf{
    const char *name;    /**< name of enviromental variable */
    char **dest;         /**< pointer to the variable holding the string */
    std::string def_val; /**< default value if not provided */
} env_vars[] = {
    {"BARIS_DATA", &options.dir_gamedata, DEFAULT_DATADIR},
    {"BARIS_SAVE", &options.dir_savegame, DEFAULT_SAVEDIR},
};

static const struct Conf_Str{
    const char *name;    /**< name of option */
    const void *dest;    /**< pointer to the variable holding the content */
    const char *format;  /**< scanf format of the data we get */
    int need_alloc;      /**< max memory size to be allocated for value */
    const char *comment; /**< a note to the user */
} config_strings[] = {
    {
        "datadir", &options.dir_gamedata, "%1024[^\n\r]", 1025,
        "Path to directory with game data files (enable only if different from default location)."
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
        "debug_level_main", &options.debug_level_main, "%u", 0,
        "Set to positive values to increase debugging verbosity of specified category."
    },
    {
        "debug_level_multimedia", &options.debug_level_multimedia, "%u", 0, nullptr
    },
    {
        "debug_level_video", &options.debug_level_video, "%u", 0, nullptr
    },
    {
        "debug_level_audio", &options.debug_level_audio, "%u", 0, nullptr
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
static void usage(int fail)
{
    fprintf(stderr, "usage:   raceintospace [options...]\n"
            "options: -i -c -a -f -e -s\n"
            "\t-i disable intro\n"
            "\t-c enable classic mode\n"
            "\t-a disable audio\n"
            "\t-f fullscreen mode\n"
            "\t-e easy mode (enables all cheats)\n"
            "\t-s small scale mode\n"
            "BARIS_DATA={path} to overwrite data folder\n"
		    "BARIS_SAVE={path} to overwrite config and saves folder"
           );
    exit((fail) ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void shift_argv(char** argv, int len, int shift)
{
    assert(shift >= 0);
    assert(len >= 0);

    for (int i = shift; i < len; ++i) {
        argv[i - shift] = argv[i];
        argv[i] = nullptr;
    }
}

static void skip_past_newline(FILE* f)
{
    assert(f);
    fscanf(f, "%*[^\r\n] ");
}

// return true on successful parsing
static bool parse_var_value(FILE* f, Conf_Str& conf_str)
{
    assert(f);

    int need_alloc = conf_str.need_alloc;

    if (need_alloc > 0) {
        /* conf_str.dest points to a pointer */
        void **target = (void **)conf_str.dest;

        char format[128];
        snprintf(format, sizeof(format), " %s%%n", conf_str.format);
        
        *target = xrealloc(*target, need_alloc);
		int chars;
        if (1 > fscanf(f, format, *target, &chars)) {
            return false; // failed to parse
        }

        if (chars < need_alloc) {
            *target = xrealloc(*target, chars + 1);
        }
    } else {
        /* config_strings[].dest points to a value */
        if (1 > fscanf(f, conf_str.format, conf_str.dest)) {
            return false; // failed to parse
        }
    }

    return true;
}

/** read the config file
 * \return -1 if the config file is unavailable
 */
static int read_config_file()
{
    fixpath_options(); // fix slashes
    FILE* f = open_savedat("config", "rt");
    if (!f) {
        LOG_INFO("could not open config file");
        return -1;
    }

    for (fscanf(f," ");!feof(f);skip_past_newline(f)) { // scan 1 line at a time till EOF
        /* skip comments */
        char c[2];
        if (fscanf(f, "%1[#]", c) == 1) { // test if first proper character in the line is '#'
            continue; // if it is, skip the line
        }

        /* get configuration variable name */
        /** \note config variables may be 32 characters of alphas plus dash and underscore */
        char config_word[32 + 1];
        if (fscanf(f, "%32[a-zA-Z_-]", config_word) != 1) {
	        // if we failed to parse it both as a comment and as a variable name - log the problem and skip to the next line
            LOG_NOTICE("expected variable name");
            continue;
        }

        // now we have config variable name - search for it in config_strings
        auto iter = std::find_if(std::begin(config_strings), std::end(config_strings), 
                                [](Conf_Str& conf_str){return strcmp(config_word, config_strings[i].name) == 0;});
        if (iter == std::end(config_strings)) { // not in config_strings? log the problem
            LOG_NOTICE("unknown variable in config file `%s'", config_word);
            continue;
        }
        // now we found which config is being given
        if (!parse_var_value(f, *iter) && !feof(f)) { // if we failed to parse and not due to the EOF - log the problem
            LOG_NOTICE("wrong value type for variable `%s'", config_word);
        }
    }

    inr err = !feof(f);
    fclose(f);
    return -err;
}

static int write_default_config()
{
    create_save_dir();
    FILE* f = open_savedat("config", "wt");

    if (f == nullptr) {
        LOG_WARNING("can't write defaults to file `%s/config': %s\n",
                     options.dir_savegame, strerror(errno));
        return -1;
    } else {
        LOG_NOTICE("writing default config to file `%s/config'",
                    options.dir_savegame);
	}

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
    fprintf(f, "# delete it and the game will create a fresh one next time it opens.\n\n");

    for (int i = 0; i < (int) ARRAY_LENGTH(config_strings); ++i) {
		if (config_strings[i].comment != nullptr)
		{
        	fprintf(f, "\n# %s\n", config_strings[i].comment);
		}
		fprintf(f, "# %s\n", config_strings[i].name);
    }

	int err = ferror(f);
    if (err != 0) {
        LOG_WARNING("error in writing default config: %s", strerror(errno));
    }

    fclose(f);
    return err;
}

// return the location of user's home directory, or empty string if unknown.
static std::string get_homedir()
{
    char* home = getenv("HOME");
    if (home != nullptr) {
        return s;
    }

#if CONFIG_WIN32

	char* path = getenv("HOMEPATH");
    if (path != nullptr) {
        char* path2 = getenv("HOMEDRIVE");
		if (path2 == nullptr) path2 = getenv("HOMESHARE");

        if (path2 != nullptr) {
            std::string drive(path2);
            return drive+path;
        }
    }

	char* profile = getenv("USERPROFILE");
    if (profile != nullptr) {
        return profile;
    }

#endif
    return {"."};
}

static void fixpath_options()
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
	
    options.debug_level_main = 0;
    options.debug_level_multimedia = -1;
    options.debug_level_video = -1;
    options.debug_level_audio = -1;

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

void enable_cheats()
{
    options.cheat_no_damage = 1;
    options.cheat_atlasOnMoon = 1;
    options.cheat_addMaxS = 0;
    options.no_money_cheat = 1;
    // options.cheat_no_fail = 1;
}

};  // End of anonymous namespace


/** read options from envvars, command line and config file
 *
 * \return length of modified argv
 *
 * \todo possibly maintain a list of dirs to search??
 */
int setup_options(int argc, char *argv[])
{
    ResetToDefaultOptions();

    // check envvars
    for (int i = 0; i < (int) ARRAY_LENGTH(env_vars); ++i) {
		char* str = getenv(env_vars[i].name); // search with getenv
        if (str != nullptr) { // if found - use that
            *env_vars[i].dest = xstrdup(str);
			continue;
        } 
        // otherwise use default
		*env_vars[i].dest = xstrdup(env_vars[i].def_val.c_str());
    }

    /* first pass: command line options */
    for (int pos = 1; pos < argc; ++pos) {
        char* str = argv[pos];

        if (str[0] != '-') {
            continue;
        }

        if (str[1] == '\0') {
            continue;
        }

        if (strcmp(str, "--") == 0) {
            shift_argv(argv + pos, argc - pos, 1);
            argc--;
            break;
        }

        /* check what option matches */
        if (strcmp(str, "-h") == 0) {
            usage(0); // exits
        } else if (strcmp(str, "-i") == 0) {
            options.want_intro = 0;
        } else if (strcmp(str, "-c") == 0) {
            options.classic = 1;
        } else if (strcmp(str, "-a") == 0) {
            options.want_audio = 0;
        } else if (strcmp(str, "-f") == 0) {
            options.want_fullscreen = 1;
        } else if (strcmp(str, "-e") == 0) {
            //options.want_cheats = 1;
		} else if (strcmp(str, "-s") == 0) {
            options.want_4xscale = 0;
        } else {
            LOG_ERROR("unknown option %s", str);
            usage(1); // exits
        }

        shift_argv(argv + pos, argc - pos, 1);
        argc--;
        pos--;  /* for loop will advance it again */
    }

    /* second pass: variable assignments */
    for (int pos = 1; pos < argc; ++pos) {
        /** \todo should use PATH_MAX or something similar here */ // ???

        char envvar_name[32 + 1]{};
        int offset = 0;
        if (sscanf(argv[pos], "%32[A-Z_]=%n", envvar_name, &offset) < 1) {
			LOG_NOTICE("unsupported format of command line argument: '%s'", argv[pos]);
            continue;
        }


        auto iter = std::find_if(std::begin(env_vars), std::end(env_vars),
                                 [=](const Env_Conf& env_conf){return strcmp(envvar_name, env_conf.name)==0;})
        if (iter == std::end(env_vars)) {
            LOG_WARNING("unsupported envvar variable `%s'", name);
        } else {
		    free(*iter.dest);
	        char* envvar_value = argv[pos] + offset;
            *iter.dest = xstrdup(envvar_value);
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

    /* now try to read config file, if it exists */
    if (read_config_file() < 0) {
        /* if not, then write default config template */
        write_default_config();
    }
	
    fixpath_options(); // fix slashes
	
    if (options.classic) {
        ResetToClassicOptions();
    }

    /* // currently want_cheats is used for no_fail, uncomment when that's fixed
    if (options.want_cheats) {
        enable_cheats();
    }
    */

    return argc;
}

/* vim: set noet ts=4 sw=4 tw=77: */
