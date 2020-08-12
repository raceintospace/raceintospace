/*
    Copyright (C) 2007 Krzysztof Kosciuszkiewicz
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

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
/** \file fs.c
 * Implementation of filesystem access functions.
 * 
 */

#include <fs.h>
#include <options.h>
#include <pace.h>
#include <utils.h>
#include <logging.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/** path separator setup */
#ifndef PATHSEP
# if CONFIG_WIN32
#  define PATHSEP '\\'
# else
#  define PATHSEP '/'
# endif
#endif

/** see how do we call mkdir */
#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
   /* MinGW32 */
#  define mkdir(a, b) mkdir(a)
# endif
#else
# if HAVE__MKDIR
   /* plain Windows 32 */
#  define mkdir(a, b) _mkdir(a)
# else
#  error "Don't know how to create a directory on this system."
# endif
#endif

/** see how we get the length of a directory's name */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

LOG_DEFAULT_CATEGORY(filesys)

static DIR *save_dir;

/** used internally to find and open files */
typedef struct file {
	FILE *handle;	/**< standard filehandle */
	char *path;		/**< path to file */
} file;

/** 
 * gamedata & savedata access functions
 */

void
fix_pathsep(char *name)
{
#if 0
	for (; *name; ++name)
		if (*name == '/')
			*name = PATHSEP;
#endif
}

static FILE*
try_fopen(const char *fname, const char *mode)
{
	FILE* fp = NULL;
	assert(fname);
	assert(mode);
	TRACE3("trying to open `%s' (mode %s)", fname, mode);

	fp = fopen(fname, mode);

	/** \todo ENOENT is POSIX, ANSI equivalent for file does not exist??? */
	if (!fp && errno != ENOENT)
	{
		int esave = errno;
		WARNING3("can't access file `%s': %s", fname, strerror(errno));
		errno = esave;
	}

	return fp;
}

/** try to open base/xxx/name for xxx = arg4 ... */
static file
s_open_helper(const char *base, const char *name, const char *mode, ...)
{
	FILE *fh = NULL;
	file f = {NULL, NULL};
	int serrno;
	char *p = NULL;
	char *cooked = xmalloc(1024);
	size_t len = 1024, len2 = 0;
	size_t len_base = strlen(base), len_name = strlen(name);
	va_list ap;

	assert(base);
	assert(name);
	assert(mode);

	va_start(ap, mode);
	for (p = va_arg(ap, char *); p; p = va_arg(ap, char *))
	{
		char *s = NULL;
		int was_upper = 0;
		size_t len_p = strlen(p);

		len2 = len_base + len_name + len_p + 3;
		if (len2 > len)
			cooked = xrealloc(cooked, (len = len2));

		if (strlen(p))
			sprintf(cooked, "%s/%s/%s", base, p, name);
		else 
			sprintf(cooked, "%s/%s", base, name);

		fix_pathsep(cooked);

		fh = try_fopen(cooked, mode);
		if (fh)
			break;

		/* try lowercase version */
		for (s = cooked + len_base + len_p + 2; *s; ++s)
			if (isupper(*s))
			{
				was_upper |= 1;
				*s = tolower(*s);
			}

		if (was_upper)
			fh = try_fopen(cooked, mode);
		if (fh)
			break;
	}
	serrno = errno;
	va_end(ap);
	if (fh)
	{
		f.handle = fh;
		f.path = cooked;
	} else
		free(cooked);
	errno = serrno;
	return f;
}

/** tries to find a file and open it
 * 
 * The function knows about the relative 
 * position of certain filetypes. It retrieves 
 * the position savegamedir and gamedatadir from
 * options.
 * 
 * \param name Name of the file to open
 * \param mode mode to file should be opened in
 * \param type Type of the file eg. FT_SAVE, FT_DATA, ...
 * 
 * \return fileinformation including opened filehandle
 */
static file
try_find_file(const char *name, const char *mode, int type)
{
	file f = {NULL, NULL};
	char *gd = options.dir_gamedata;
	char *sd = options.dir_savegame;
	char *where = "";
	const char *newmode = mode;

	DEBUG2("looking for file `%s'", name);

	/** \note allows write access only to savegame files */
	if (type != FT_SAVE)
	{
		if (strchr(mode, 'w')
				|| strchr(mode, 'a')
				|| strncmp(mode, "r+", 2) == 0)
		{
			char *newmode;
			if (strchr(mode, 'b'))
				newmode = "rb";
			else
				newmode = "r";
			DEBUG3("access mode changed from `%s' to `%s'", mode, newmode);
		}
	}

	switch (type)
	{
		case FT_DATA:
			f = s_open_helper(gd, name, newmode,
					"gamedata",
					NULL);
			where = "game data";
			break;
		case FT_SAVE:
		case FT_SAVE_CHECK:
			f = s_open_helper(sd, name, newmode,
					"",
					NULL);
			where = "savegame";
			break;
		case FT_AUDIO:
			f = s_open_helper(gd, name, newmode,
					"audio/mission",
					"audio/music",
					"audio/news",
					"audio/sounds",
					NULL);
			where = "audio";
			break;
		case FT_VIDEO:
			f = s_open_helper(gd, name, newmode,
					"video/mission",
					"video/news",
					"video/training",
					NULL);
			where = "video";
			break;
		case FT_IMAGE:
			f = s_open_helper(gd, name, newmode,
					"images",
					NULL);
			where = "image";
			break;
		case FT_MIDI:
			f = s_open_helper(gd, name, newmode,
					"audio/midi",
					"midi",
					"audio/music",
					NULL);
			where = "midi";
			break;
		default:
			assert("Unknown FT_* specified");
	}

	if (f.handle == NULL && type != FT_SAVE_CHECK)
	{
		int serrno = errno;
		WARNING3("can't find file `%s' in %s dir(s)", name, where);
		errno = serrno;
	}
	return f;
}

FILE*
sOpen(const char *name, const char *mode, int type)
{
	file f = try_find_file(name, mode, type);
	if (f.path)
	{
		INFO3("opened file `%s' (mode %s)", f.path, mode);
		free(f.path);
	}
	return f.handle;
}

/** Find and open file, if found return full path.
 * Caller is responsible for freeing the memory.
 */
char*
locate_file(const char *name, int type)
{
	file f = try_find_file(name, "rb", type);
	if (f.handle)
	{
		INFO2("found file `%s'", f.path);
		fclose(f.handle);
	}
	return f.path;
}

int
remove_savedat(const char *name)
{
	size_t len_base = strlen(options.dir_savegame) + 1;
	size_t len_name = strlen(name) + 1;
	char *cooked = xmalloc(len_base + len_name);
	int rv = 0;

	sprintf(cooked, "%s/%s", options.dir_savegame, name);
	INFO2("removing save game file `%s'", cooked);
	fix_pathsep(cooked);
	rv = remove(cooked);
	if (rv < 0 && errno != ENOENT)
		WARNING3("failed to remove save game file `%s': %s",
				cooked, strerror(errno));
	free(cooked);
	return rv;
}

FILE *
open_gamedat(const char *name)
{
	return sOpen(name, "rb", FT_DATA);
}

FILE *
open_savedat(const char *name, const char *mode)
{
	return sOpen(name, mode, FT_SAVE_CHECK);
}

char *
slurp_gamedat(const char *name)
{
	FILE *f;
	ssize_t len;
	char *p = NULL;
	size_t buflen = 0;

	f = open_gamedat(name);
	if (!f)
		return NULL;

	len = fread_dyn(&p, &buflen, f);

	if (len < 0)
	{
		CRITICAL2("could not read file `%s'", name);
		exit(EXIT_FAILURE);
	}

	fclose(f);

	return p;
}

/** Create the savegame directory
 * 
 * The directory will be created as defined in options.dir_savegame.
 * 
 * \note The access will be set to 0777 (worldwritable)
 * 
 * \return -1 on error
 * \return 0 on success
 */
int
create_save_dir(void)
{
	if (mkdir(options.dir_savegame, 0777) < 0 && errno != EEXIST) {
		WARNING3("can't create savegame directory `%s': %s",
				options.dir_savegame, strerror(errno));
		return -1;
	}
	return 0;
}

int
first_saved_game(struct ffblk *ffblk)
{
	if (save_dir)
	{
		closedir(save_dir);
		save_dir = NULL;
	}

	if ((save_dir = opendir(options.dir_savegame)) == NULL)
		return (1);

	return (next_saved_game(ffblk));
}

int
next_saved_game(struct ffblk *ffblk)
{
	struct dirent *dp;
	int len;

	memset(ffblk, 0, sizeof *ffblk);

	if (save_dir == NULL)
		return (1);

	while ((dp = readdir(save_dir)) != NULL)
	{
		len = NAMLEN(dp);
		if (len < 4)
			continue;
		if (xstrncasecmp(dp->d_name + len - 4, ".SAV", 4) != 0)
			continue;

		strncpy(ffblk->ff_name, dp->d_name, sizeof ffblk->ff_name);
		ffblk->ff_name[sizeof ffblk->ff_name - 1] = 0;

		return (0);
	}

	return (1);
}

/* vim: set noet ts=4 sw=4 tw=77: */
