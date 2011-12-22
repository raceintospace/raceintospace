# Find our dependencies
include(${CMAKE_ROOT}/Modules/FindSDL.cmake)

find_library(Vorbis_LIBRARY NAMES vorbis)
find_path(Vorbis_INCLUDE_DIR vorbis/codec.h)

find_library(Theora_LIBRARY NAMES theora)
find_path(Theora_INCLUDE_DIR theora/theora.h)

find_library(Ogg_LIBRARY NAMES ogg)
find_path(Ogg_INCLUDE_DIR ogg/ogg.h)

# Probe the system
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(TestBigEndian)

check_function_exists(mkdir HAVE_MKDIR)
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(sys/dir.h HAVE_SYS_DIR_H)
check_include_file(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_file(ndir.h HAVE_NDIR_H)

# Set some build options
if (APPLE)
  set(SET_SDL_ICON FALSE)
  set(DEFAULT_DATADIR "Contents/Resources")
  set(DEFAULT_SAVEDIR ".")
  set(PLATFORM_MUSIC true)
  #set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
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

# pull in src/game/* defines and build rules
include("game/Global.cmake")
add_subdirectory("game")

if (APPLE)
  add_subdirectory("os_macosx")
endif (APPLE)

add_subdirectory("utils")
