#include <assert.h>
#include <physfs.h>

#include <stdexcept>

#include "file.h"

#define m_phys_handle ((PHYSFS_File*)m_handle)

#define throw_error do { \
    const char * error = PHYSFS_getLastError(); \
    if (error == NULL) \
        error = "unknown filesystem error"; \
    throw std::runtime_error(error); \
    } while (0)

File::File(void *handle)
    : m_handle(handle)
{
    assert(m_handle);
}

File::~File()
{
    close();
}

void File::close()
{
    int success;

    if (m_handle) {
        success = PHYSFS_close(m_phys_handle);

        if (success) {
            m_handle = NULL;
        }
    }
}

int64_t File::read(void *buffer, uint64_t length)
{
    int64_t result = PHYSFS_readBytes(m_phys_handle, buffer, length);

    if (result < 0) {
        throw_error;    // only explode on total failure
    }

    return result;
}

int64_t File::write(const void *buffer, uint64_t length)
{
    int64_t result = PHYSFS_writeBytes(m_phys_handle, buffer, length);

    if (result < length) {
        throw_error;    // is this too drastic?
    }

    return result;
}

void File::flush()
{
    int success = PHYSFS_flush(m_phys_handle);

    if (!success) {
        throw_error;
    }
}

void File::seek(uint64_t position)
{
    int success = PHYSFS_seek(m_phys_handle, position);

    if (!success) {
        throw_error;
    }
}

uint64_t File::tell()
{
    int64_t position = PHYSFS_tell(m_phys_handle);

    if (position < 0) {
        throw_error;
    }

    return position;
}

uint64_t File::length()
{
    int64_t length = PHYSFS_fileLength(m_phys_handle);

    if (length < 0) {
        throw_error;
    }

    return length;
}

bool File::eof()
{
    if (PHYSFS_eof(m_phys_handle)) {
        return true;
    } else {
        return false;
    }
}
