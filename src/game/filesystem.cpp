#include <assert.h>
#include <physfs.h>

#include <stdexcept>

#include "display/png_image.h"

#include "raceintospace_config.h"
#include "filesystem.h"

#define throw_error do { \
    const char * error = PHYSFS_getLastError(); \
    if (error == NULL) \
        error = "unknown filesystem error"; \
    throw std::runtime_error(error); \
    } while (0)

#ifdef __linux__
#include <cstring> 
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

/* Function with behaviour like `mkdir -p'  */
static int mkpath(const char *s, mode_t mode){
  if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
    return (0);  
  std::string path = s;
  char *q = strdup(s);
  char *r = dirname(q);
  if (r == NULL) {
     free(q); return -1;
  }
  std::string up = r;
  free(q);
  
  if ((mkpath(up.c_str(), mode) == -1) && (errno != EEXIST))
    return -1;
  
  if ((mkdir(path.c_str(), mode) == -1) && (errno != EEXIST))
    return -1;
  else
    return 0;
}

// Reimplement so that we can use phys 2.0 on linux
static char *my_pref_dir(const char *org, const char *app)
{
    /*
     * We use XDG's base directory spec, even if you're not on Linux.
     *  This isn't strictly correct, but the results are relatively sane
     *  in any case.
     *
     * http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
     */
    const char *envr = getenv("XDG_DATA_HOME");
    const char *append = "/";
    char *retval = NULL;
    size_t len = 0;

    if (!envr)
    {
        /* You end up with "$HOME/.local/share/Game Name 2" */
        envr = PHYSFS_getUserDir();
        append = ".local/share/";
    } /* if */

    len = strlen(envr) + strlen(append) + strlen(app) + 2;
    retval = (char *) malloc(len);
    snprintf(retval, len, "%s%s%s/", envr, append, app);
    mkpath(retval, 0777);
    return retval;
} 
#endif

Filesystem Filesystem::singleton;

Filesystem::Filesystem()
{
    // do nothing
}

Filesystem::~Filesystem()
{
    // shut. down. everything.
    // this can fail, but whatever
    PHYSFS_deinit();
}

void Filesystem::init(const char * argv0)
{
    if (!PHYSFS_isInit()) {
        int success = PHYSFS_init(argv0);
        if (!success)
            throw_error;
        
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
#ifdef __linux__
	const char * prefdir = my_pref_dir("raceintospace.org", "Race Into Space");
#else
        const char * prefdir = PHYSFS_getPrefDir("raceintospace.org", "Race Into Space");
#endif
        if (prefdir == NULL)
            throw_error;
        
        // use this for reading, *before* the expected game data directory, thereby allowing overlays
        success = PHYSFS_mount(prefdir, NULL, 0);
        if (!success)
            throw_error;

        // use this for writing too
        PHYSFS_setWriteDir(prefdir);
        if (!success)
            throw_error;
    }
}

bool Filesystem::exists(const std::string& filename)
{
    if (PHYSFS_exists(filename.c_str())) {
        return true;
    } else {
        return false;
    }
}

void filesystem_enumerate_callback(void *data, const char *directory, const char *filename)
{
    std::list<std::string> *list = (std::list<std::string>*)data;
    std::string full_path(directory);
    full_path.append("/");
    full_path.append(filename);
    list->push_back(full_path);
}
    
std::list<std::string> Filesystem::enumerate(const std::string& directory)
{
    std::list<std::string> list;
    
    // enumerate
    PHYSFS_enumerateFilesCallback(directory.c_str(), filesystem_enumerate_callback, &list);
    
    // sort the result
    list.sort();
    
    return list;
}
    
bool Filesystem::unlink(const std::string& filename)
{
    int failure;
    
    failure = PHYSFS_delete(filename.c_str());
    if (failure)
        return false;
    else
        return true;
}

boost::shared_ptr<File> Filesystem::open(const std::string& filename)
{
    PHYSFS_File * file_handle = PHYSFS_openRead(filename.c_str());
    if (!file_handle)
        throw_error;
    
    boost::shared_ptr<File> file_ptr(new File(file_handle));
    return file_ptr;
}

boost::shared_ptr<File> Filesystem::openWrite(const std::string& filename)
{
    PHYSFS_File * file_handle = PHYSFS_openWrite(filename.c_str());
    if (!file_handle)
        throw_error;
    
    boost::shared_ptr<File> file_ptr(new File(file_handle));
    return file_ptr;
}

void Filesystem::readToBuffer(const std::string& filename, void *buffer, uint32_t length, uint32_t offset)
{
    boost::shared_ptr<File> file_ptr(open(filename));
    
    if (offset)
        file_ptr->seek(offset);

    uint32_t bytes_read = file_ptr->read(buffer, length);
    if (bytes_read < length)
        throw_error;
}

boost::shared_ptr<display::PNGImage> Filesystem::readImage(const std::string& filename)
{
    // open the file
    boost::shared_ptr<File> file_ptr(open(filename));
    
    // get its length, ensuring it's something we're okay with loading from the stack
    uint64_t length = file_ptr->length();
    assert(length < 1024 * 1024);

    // allocate a buffer on the stack and read into it
    uint8_t buffer[length];
    uint64_t bytes_read = file_ptr->read(buffer, length);
    if (bytes_read < length)
        throw_error;
    
    // construct a PNGImage from this buffer
    boost::shared_ptr<display::PNGImage> png_image(new display::PNGImage(buffer, length));
    
    // pass it back to the caller
    return png_image;
}
