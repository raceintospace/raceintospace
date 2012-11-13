// Fake unistd.h for Windows

#ifndef FAKE_UNISTD_H
#define FAKE_UNISTD_H

// define ssize_t
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#define snprintf _snprintf

// this really doesn't belong here...
#define __func__ __FUNCTION__

#endif // FAKE_UNISTD_H
