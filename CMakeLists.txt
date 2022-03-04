cmake_minimum_required (VERSION 2.8)
project(raceintospace)

enable_testing()

set (raceintospace_VERSION_MAJOR 2)
set (raceintospace_VERSION_MINOR 0)
set (raceintospace_VERSION_PATCH 0)
set (raceintospace_VERSION_EXTRA rc1)
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
  #set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)
  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
          DESTINATION bin
          COMPONENT runtime)
endif (WINDOWS)

option(PBEM "Enable experimental Play-by-EMail feature" OFF)

string(TOLOWER "${CMAKE_BUILD_TYPE}" lc_CMAKE_BUILD_TYPE)
if("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_definitions(-DDEBUG)
endif("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")

add_subdirectory(lib)
add_subdirectory(src)
install(PROGRAMS icons/raceintospace.desktop DESTINATION share/applications)
install(FILES icons/raceintospace.xpm icons/raceintospace.png DESTINATION share/pixmaps)
install(FILES doc/org.raceintospace.Raceintospace.metainfo.xml
	DESTINATION share/metainfo)

if (WINDOWS)
  set(CPACK_GENERATOR NSIS)
else(WINDOWS)
  set(CPACK_GENERATOR TGZ)
endif(WINDOWS)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR ${raceintospace_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${raceintospace_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${raceintospace_VERSION_PATCH})
set(CPACK_PACKAGE_NAME "Race into Space")
set(CPACK_PACKAGE_VENDOR "raceintospace.org")
set(CPACK_INSTALLED_DIRECTORIES ${CMAKE_SOURCE_DIR}/data data)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
#set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/icons/Moon.ico)
include(CPack)

find_program(GIT git DOC "Git program path")

add_custom_target(dist
	COMMAND ${CMAKE_COMMAND} -D ARCHIVE_PREFIX="${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}" -D SOURCE_DIR="${CMAKE_SOURCE_DIR}" -D OUTPUT_DIR="${CMAKE_BINARY_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/make_archive.cmake
)
add_custom_target(run
	COMMAND ${CMAKE_BINARY_DIR}/src/game/raceintospace BARIS_DATA=${CMAKE_SOURCE_DIR}/data
	DEPENDS raceintospace
	)

if("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_definitions(-DDEBUG)
	add_custom_target(gdb
		COMMAND gdb -ex run --args ${CMAKE_BINARY_DIR}/src/game/raceintospace BARIS_DATA=${CMAKE_SOURCE_DIR}/data
		DEPENDS raceintospace
		)
else("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_custom_target(gdb
		COMMENT "Set CMAKE_BUILD_TYPE=Debug for debugging information (e.g., \"cmake -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}\")"
		COMMAND false
		)
endif("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")

add_custom_target(tag
	COMMAND ${GIT} tag -a v${CMAKE_PROJECT_VERSION})

