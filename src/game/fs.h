#ifndef _FS_H
#define _FS_H

#include <race.h>
#include <stdio.h>

/** \file fs.h Definitions for filesystem
 * 
 */

/** types of files - they have different locations in filesystem */
enum {
    FT_DATA = 0,
    FT_SAVE = 1,
    FT_AUDIO,
    FT_VIDEO,
    FT_IMAGE,
    FT_MIDI,
    FT_SAVE_CHECK, /* Ok for open to fail - don't print warning. */
};

/** used for saved games */
struct ffblk {
	char ff_name[64];
	int ff_ftime;
	int ff_fdate;
};

extern FILE* sOpen(const char *name, const char *mode, int type);
extern char* locate_file(const char *name, int type);
extern FILE* open_gamedat(const char *name);
extern FILE* open_savedat(const char *name, const char *mode);
extern char *slurp_gamedat(const char *name);
extern int create_save_dir(void);
extern int remove_savedat(const char *name);
extern int first_saved_game (struct ffblk *ffblk);
extern int next_saved_game (struct ffblk *ffblk);
extern void fix_pathsep(char *path);

#endif /* _FS_H */
