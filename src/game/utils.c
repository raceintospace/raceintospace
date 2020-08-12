#include <utils.h>
#include <logging.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* taken from ffmpeg project */
#ifdef CONFIG_WIN32
#   include <sys/types.h>
#   include <sys/timeb.h>
#elif defined(CONFIG_OS2)
#   include <string.h>
#   include <sys/time.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/time.h>
#endif

LOG_DEFAULT_CATEGORY(utils)

double
get_time (void)
{
#ifdef CONFIG_WIN32
    /* mingw was complaining */
	struct _timeb tb;

	_ftime(&tb);
	return tb.time + tb.millitm / 1e3;
#else
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
#endif
}

int
xstrcasecmp (const char *a, const char *b)
{
	while (*a) {
		if (tolower (*a & 0xff) != tolower (*b & 0xff))
			break;
		a++;
		b++;
	}
	return (tolower(*a) - tolower(*b));
}

int
xstrncasecmp (const char *a, const char *b, size_t n)
{
	while (n && *a) {
		if (tolower (*a & 0xff) != tolower (*b & 0xff))
			break;
		a++;
		b++;
		n--;
	}
	return (tolower(*a) - tolower(*b));
}

void *
xmalloc (size_t n)
{
	void *p = malloc(n);

	if (!p)
	{
		CRITICAL2("allocation failure: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return (p);
}

void *
xcalloc (size_t a, size_t b)
{
	void *p = calloc(a, b);

	if (!p)
	{
		CRITICAL2("callocation failure: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return (p);
}

/** duplicates a string and returns a pointer to the new string
 */
char *
xstrdup (const char *s)
{
	void *p;

	p = xmalloc (strlen (s) + 1);
	strcpy (p, s);
	return (p);
}

char *
xstrcat2 (const char* s1, const char* s2)
{
	char *s = NULL;
	assert(s1 && s2);
	s = xmalloc(strlen(s1) + strlen(s2) + 1);
	strcpy(s, s1);
	strcat(s, s2);
	return s;
}

void *
xrealloc(void *ptr, size_t size)
{
	void *p = realloc(ptr, size);

	if (!p)
    {
		CRITICAL2("reallocation failure: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return (p);
}

ssize_t
fread_dyn(char **destp, size_t *n, FILE *stream)
{
    const unsigned bsize = 8192;
    size_t total = 0, cnt = 0; 

    assert(destp);
    assert(n);
    assert(stream);

    if (!*destp) 
        *destp = xmalloc(*n = bsize);

    while (1) {
        cnt = fread(*destp+total, 1, *n-total, stream);
        if (cnt != *n-total)
        {
            if (feof(stream))
                return total+cnt;
            else if (ferror(stream))
			{
				CWARNING3(filesys, "read error: %s", strerror(errno));
                return -1;
			}
        }
        total += cnt;

        if (*n <= total)
            *destp = xrealloc(*destp, *n *= 2);
    }
}

/* vim: set noet ts=4 sw=4 tw=77: */
