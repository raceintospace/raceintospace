# Probe the system
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(TestBigEndian)

check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(sys/dir.h HAVE_SYS_DIR_H)
check_include_file(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/timeb.h HAVE_SYS_TIMEB_H)
check_include_file(ndir.h HAVE_NDIR_H)
check_include_file(int_types.h HAVE_INTTYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)

# Set some build options
if (APPLE)
  set(SET_SDL_ICON FALSE)
  set(DEFAULT_DATADIR "Contents/Resources")
  set(DEFAULT_SAVEDIR ".")
  set(PLATFORM_PROVIDES_UGLY_CRASH true)

  # this is so we can find Homebrew components
  include_directories(SYSTEM /usr/local/include)
  link_directories(/usr/local/lib)
else (APPLE)
  set(SET_SDL_ICON TRUE)
  set(DEFAULT_DATADIR ".")
  set(DEFAULT_SAVEDIR ".")
endif (APPLE)

# generate the raceintospace_config.h
configure_file(
  "raceintospace_config.h.in"
  "${PROJECT_BINARY_DIR}/config/raceintospace_config.h"
  )

# so we can find raceintospace_config.h
include_directories(${PROJECT_BINARY_DIR}/config)

# pull in src/display/* defines and build rules
include("display/Global.cmake")
add_subdirectory("display")

# pull in src/game/* defines and build rules
add_subdirectory("game")

add_subdirectory("utils")
