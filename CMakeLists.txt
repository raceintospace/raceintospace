cmake_minimum_required (VERSION 2.8)
project(raceintospace)

enable_testing()

set (raceintospace_VERSION_MAJOR 2)
set (raceintospace_VERSION_MINOR 0)
set (raceintospace_VERSION_PATCH 0)
set (raceintospace_VERSION_EXTRA a3)
set (raceintospace_VERSION
	"${raceintospace_VERSION_MAJOR}.${raceintospace_VERSION_MINOR}.${raceintospace_VERSION_PATCH}${raceintospace_VERSION_EXTRA}")
if (NOT DEFINED CMAKE_PROJECT_VERSION)
# TODO: use project VERSION & cmake 3.0
  set (CMAKE_PROJECT_VERSION "${raceintospace_VERSION}")
endif (NOT DEFINED CMAKE_PROJECT_VERSION)

# Platform detection
if (WIN32 AND NOT WINDOWS)
  set (WINDOWS TRUE)
endif (WIN32 AND NOT WINDOWS)

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
endif (WINDOWS)

# Require C++11
set (CMAKE_CXX_STANDARD 11)

add_subdirectory(lib)
add_subdirectory(src)
install(PROGRAMS icons/raceintospace.desktop DESTINATION share/applications)
install(FILES icons/raceintospace.xpm icons/raceintospace.png DESTINATION share/pixmaps)
install(FILES doc/org.raceintospace.Raceintospace.metainfo.xml
	DESTINATION share/metainfo)

set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
include(CPack)

find_program(GIT git DOC "Git program path")

add_custom_target(dist
	COMMAND ${CMAKE_COMMAND} -D ARCHIVE_PREFIX="${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}" -D SOURCE_DIR="${CMAKE_SOURCE_DIR}" -D OUTPUT_DIR="${CMAKE_BINARY_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/make_archive.cmake
)
add_custom_target(run
	COMMAND ${CMAKE_BINARY_DIR}/src/game/raceintospace BARIS_DATA=${CMAKE_SOURCE_DIR}/data
	DEPENDS raceintospace
)

add_custom_target(tag
	COMMAND ${GIT} tag -a v${CMAKE_PROJECT_VERSION})

