#include <assert.h>
#include <physfs.h>

#include <stdexcept>
#include <boost/format.hpp>

#include "display/image.h"

#include "raceintospace_config.h"
#include "filesystem.h"

using boost::format;

#define throw_error do { \
    const char * error = PHYSFS_getLastError(); \
    if (error == NULL) \
        error = "unknown filesystem error"; \
    throw std::runtime_error(error); \
    } while (0)


#define throw_error_with_detail(detail) do { \
    const char * error = PHYSFS_getLastError(); \
    if (error == NULL) \
        error = "unknown filesystem error"; \
    std::string msg = (format("%1%: %2%") % error % detail).str(); \
    throw std::runtime_error(msg); \
    } while (0)

Filesystem Filesystem::singleton;

Filesystem::Filesystem()
{
    // do nothing
}

Filesystem::~Filesystem()
{
    // shut. down. everything.
    // this can fail, but whatever
    // Wrap this in a safety check because it causes errors if called
    // before initialization.
    if (PHYSFS_isInit()) {
        PHYSFS_deinit();
    }
}

void Filesystem::init(const char *argv0)
{
    if (!PHYSFS_isInit()) {
        int success = PHYSFS_init(argv0);

        if (!success) {
            throw_error;
        }

        const std::string basedir(PHYSFS_getBaseDir());

        if (basedir.substr(basedir.length() - 5, 5) == ".app/") {
            // smells like a Mac OS X application bundle
            std::string resource_path = basedir + "Contents/Resources";
            PHYSFS_mount(resource_path.c_str(), NULL, 0);
        } else {
            // er, uh... search in the base directory?
            PHYSFS_mount(basedir.c_str(), NULL, 0);
        }

        // get a platform-specific sensible directory for the app
        const char *prefdir = PHYSFS_getPrefDir("raceintospace.org", "Race Into Space");

        if (prefdir == NULL) {
            throw_error_with_detail(prefdir);
        }

        // use this for reading, *before* the expected game data directory, thereby allowing overlays
        success = PHYSFS_mount(prefdir, NULL, 0);

        if (!success) {
            throw_error_with_detail(prefdir);
        }

        // use this for writing too
        PHYSFS_setWriteDir(prefdir);

        if (!success) {
            throw_error_with_detail(prefdir);
        }
    }
}

void Filesystem::addPath(const char *s)
{
    if (!PHYSFS_isInit()) {
        throw_error;
    }

    int success = PHYSFS_mount(s, NULL, 0);

    if (!success) {
        throw_error_with_detail(s);
    }
}

bool Filesystem::exists(const std::string &filename)
{
    if (PHYSFS_exists(filename.c_str())) {
        return true;
    } else {
        return false;
    }
}

void filesystem_enumerate_callback(void *data, const char *directory, const char *filename)
{
    std::list<std::string> *list = (std::list<std::string> *)data;
    std::string full_path(directory);
    full_path.append("/");
    full_path.append(filename);
    list->push_back(full_path);
}

std::list<std::string> Filesystem::enumerate(const std::string &directory)
{
    std::list<std::string> list;

    // enumerate
    PHYSFS_enumerateFilesCallback(directory.c_str(), filesystem_enumerate_callback, &list);

    // sort the result
    list.sort();

    return list;
}

bool Filesystem::unlink(const std::string &filename)
{
    int failure;

    failure = PHYSFS_delete(filename.c_str());

    if (failure) {
        return false;
    } else {
        return true;
    }
}

boost::shared_ptr<File> Filesystem::open(const std::string &filename)
{
    PHYSFS_File *file_handle = PHYSFS_openRead(filename.c_str());

    if (!file_handle) {
        throw_error_with_detail(filename);
    }

    boost::shared_ptr<File> file_ptr(new File(file_handle));
    return file_ptr;
}

boost::shared_ptr<File> Filesystem::openWrite(const std::string &filename)
{
    PHYSFS_File *file_handle = PHYSFS_openWrite(filename.c_str());

    if (!file_handle) {
        throw_error_with_detail(filename);
    }

    boost::shared_ptr<File> file_ptr(new File(file_handle));
    return file_ptr;
}

void Filesystem::readToBuffer(const std::string &filename, void *buffer, uint32_t length, uint32_t offset)
{
    boost::shared_ptr<File> file_ptr(open(filename));

    if (offset) {
        file_ptr->seek(offset);
    }

    uint32_t bytes_read = file_ptr->read(buffer, length);

    if (bytes_read < length) {
        throw_error_with_detail(filename);
    }
}

boost::shared_ptr<display::PalettizedSurface> Filesystem::readImage(const std::string &filename)
{
    // open the file
    boost::shared_ptr<File> file_ptr(open(filename));

    // get its length, ensuring it's something we're okay with loading from the stack
    uint64_t length = file_ptr->length();
    assert(length < 1024 * 1024);

    // allocate a buffer on the stack and read into it
    uint8_t *buffer = new uint8_t[length];
    uint64_t bytes_read = file_ptr->read(buffer, length);

    if (bytes_read < length) {
        delete[] buffer;
        throw_error_with_detail(filename);
    }

    // construct a PNGImage from this buffer
    boost::shared_ptr<display::PalettizedSurface> image(display::image::readPalettizedPNG(buffer, length));

    delete[] buffer;

    // pass it back to the caller
    return image;
}
