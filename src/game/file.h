#ifndef FILE_H
#define FILE_H

#include <stdint.h>

class File
{
public:
    File(void *handle);
    ~File();

    void close();

    int64_t read(void *buffer, uint64_t length);

    int64_t write(const void *buffer, uint64_t length);
    void flush();

    void seek(uint64_t position);
    uint64_t tell();
    uint64_t length();
    bool eof();

protected:
    void *m_handle;
};

#endif // FILE_H
