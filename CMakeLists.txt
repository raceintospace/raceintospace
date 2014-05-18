cmake_minimum_required (VERSION 2.8)
project(raceintospace)

enable_testing()

set (raceintospace_VERSION_MAJOR 1)
set (raceintospace_VERSION_MINOR 1)
set (raceintospace_VERSION_RELEASE 0)

# Platform detection
if (WIN32 AND NOT WINDOWS)
  set (WINDOWS TRUE)
endif (WIN32 AND NOT WINDOWS)

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
endif (WINDOWS)

add_subdirectory(lib)
add_subdirectory(src)
install(PROGRAMS icons/raceintospace.desktop DESTINATION /usr/share/applications)
install(FILES icons/raceintospace.xpm DESTINATION /usr/share/pixmaps)
