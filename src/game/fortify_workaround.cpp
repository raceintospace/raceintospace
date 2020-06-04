#include "fortify_workaround.h"

#include <cstdio>


#if _FORTIFY_SOURCE > 0

#undef fread
size_t
fread_fortify_workaround(void *buf,
                         size_t size, size_t n, FILE *stream)
{
    return (fread(buf, size, n, stream));
}

#endif /* _FORTIFY_SOURCE */
