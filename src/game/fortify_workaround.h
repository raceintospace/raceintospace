#ifndef _FORTIFY_WORKAROUND_
#define _FORTIFY_WORKAROUND_

/*
 * there are over 100 places that call fread without checking the
 * return value, and they all make warnings with the new
 * _FORTIFY_SOURCE feature in gcc.  Until someone wants to put error
 * handlers in each place, this workaround will preserve the existing
 * runtime behavor (that is, hope for the best), at the negligible
 * cost of one extra function call per fread, while cleaning up the
 * compiler output so we can notice the real warnings
 */
#if _FORTIFY_SOURCE > 0
#define fread fread_fortify_workaround
#endif

size_t fread_fortify_workaround (void *buf,
				 size_t size, size_t n, FILE *stream);

#endif /* _FORTIFY_WORKAROUND_ */

