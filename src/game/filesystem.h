#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <list>
#include <string>
#include <boost/shared_ptr.hpp>

#include "display/image.h"

#include "file.h"

class Filesystem
{
protected:
    Filesystem();
    ~Filesystem();

    // this singleton calls the destructor shortly before terminating
    static Filesystem singleton;

public:
    static void init(const char *argv0);
    static bool exists(const std::string &filename);
    static std::list<std::string> enumerate(const std::string &directory);
    static bool unlink(const std::string &filename);

    static boost::shared_ptr<File> open(const std::string &filename);

    static boost::shared_ptr<File> openWrite(const std::string &filename);

    static void readToBuffer(const std::string &filename, void *buffer, uint32_t length, uint32_t offset = 0);
    static boost::shared_ptr<display::PalettizedSurface> readImage(const std::string &filename);
    static void addPath(const char *s);
};

#endif // FILESYSTEM_H
