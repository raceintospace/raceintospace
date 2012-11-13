#ifndef _UTILS_H
#define _UTILS_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "raceintospace_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include "fake_unistd.h"
#endif

void *xmalloc(size_t size);
void *xcalloc(size_t nelems, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *a);
char *xstrcat2(const char *s1, const char *s2);
int xstrcasecmp(const char *a, const char *b);
int xstrncasecmp(const char *a, const char *b, size_t len);
ssize_t fread_dyn(char **destp, size_t *n, FILE *stream);
extern double get_time(void);

#endif /* _UTILS_H */
