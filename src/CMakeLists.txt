# Probe the system
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(TestBigEndian)

include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

if (NOT WINDOWS)
  check_function_exists(mkdir HAVE_MKDIR)
  set (HAVE_LRINT true)
else ()
  check_function_exists(_mkdir HAVE__MKDIR)
endif ()

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
elseif (WINDOWS)
  set(SET_SDL_ICON TRUE)
  set(DEFAULT_DATADIR ".")
  set(DEFAULT_SAVEDIR ".")
else()
   set(SET_SDL_ICON TRUE)
   set(DEFAULT_DATADIR "${CMAKE_INSTALL_PREFIX}/share/raceintospace")
   set(DEFAULT_SAVEDIR ".")
  # this is so we can find Homebrew components
  include_directories(SYSTEM /usr/local/include)
  link_directories(/usr/local/lib)
endif ()

# generate the raceintospace_config.h
configure_file(
  "raceintospace_config.h.in"
  "${PROJECT_BINARY_DIR}/config/raceintospace_config.h"
  )

# so we can find raceintospace_config.h
include_directories(${PROJECT_BINARY_DIR}/config)

# build src/display
include("display/Global.cmake")
add_subdirectory("display")

# build src/protobuf
add_subdirectory("protobuf")

# build src/game
add_subdirectory("game")

# build the targets in src/utils too
add_subdirectory("utils")
