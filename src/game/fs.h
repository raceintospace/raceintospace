#ifndef _FS_H
#define _FS_H

#include <stdio.h>
#include <physfs.h>
#include <string>

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

/* Physfs provides multiplatform implementation of directory contents listing. */
struct PhysFsEnumerator {
public:
    PhysFsEnumerator(const std::string& name) : m_name(name) {}
    int enumerate();
    virtual PHYSFS_EnumerateCallbackResult onItem(const std::string& origdir, const std::string& fname) = 0;
protected:
    static PHYSFS_EnumerateCallbackResult
        enumerate_callback(void* data, const char* origdir, const char* fname);
private:
    std::string m_name;
};

extern FILE *sOpen(const char *name, const char *mode, int type);
extern char *locate_file(const char *name, int type);
extern FILE *open_gamedat(const char *name);
extern FILE *open_savedat(const char *name, const char *mode);
extern char *slurp_gamedat(const char *name);
extern int create_save_dir(void);
extern int remove_savedat(const char *name);
extern void fix_pathsep(char *path);

#endif /* _FS_H */
