# CXX_STANDARD requires more recent CMake
cmake_minimum_required (VERSION 3.25...3.29)
project(raceintospace VERSION 2.0.0)

enable_testing()

set (raceintospace_VERSION_EXTRA)
set (raceintospace_VERSION_FULL
	"${raceintospace_VERSION}${raceintospace_VERSION_EXTRA}")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Platform detection
if (WIN32 AND NOT WINDOWS)
  set (WINDOWS TRUE)
endif (WIN32 AND NOT WINDOWS)

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
endif (WINDOWS)

# Require C++11
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
option(PBEM "Enable Play-by-EMail feature" ON)

string(TOLOWER "${CMAKE_BUILD_TYPE}" lc_CMAKE_BUILD_TYPE)
if("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_definitions(-DDEBUG)
endif("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")

add_subdirectory(src)

# Distribution
set(CPACK_PACKAGE_NAME "raceintospace")
set(CPACK_PACKAGE_VENDOR "Race Into Space")
set(CPACK_PACKAGE_VERSION "${raceintospace_VERSION_FULL}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_RPM_PACKAGE_SUMMARY "space race strategy simulation game")
set(CPACK_RPM_PACKAGE_DESCRIPTION
"Race into Space is an open-source strategy game where the United
States and the Soviet Union compete for the first manned lunar
landing. It is the computer version of the Liftoff! board game by
Fritz Bronner, originally published by Interplay in 1993. A number of
improvements have been made over the original version.")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_RPM_PACKAGE_SUMMARY}
${CPACK_RPM_PACKAGE_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libsdl1.2debian, libvorbis0a, libvorbisenc2, libpng16-16, libphysfs1, libjsoncpp25")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/applications /usr/share/metainfo /usr/share/pixmaps)


if (WINDOWS)
	configure_file("${PROJECT_SOURCE_DIR}/cmake/NsisCpack.cmake.in"
                   "${PROJECT_BINARY_DIR}/NsisCpack.cmake"
				   @ONLY)
	set(CPACK_PROJECT_CONFIG_FILE
		"${PROJECT_BINARY_DIR}/NsisCpack.cmake")
    set(CPACK_GENERATOR NSIS)
	# HACK: can't figure out how to select all dlls
	# that are required for target **and** its dependencies...
	# need to check IMPORTED_RUNTIME_ARTIFACTS
	install(TARGETS raceintospace DESTINATION .)
	install(CODE "
		file(GLOB dll_files \"$<TARGET_FILE_DIR:raceintospace>/*.dll\")
		file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/.\" TYPE SHARED_LIBRARY FILES \${dll_files})
	")
	install(DIRECTORY data/audio data/gamedata data/images data/video icons DESTINATION .)
	install(FILES COPYING README.md AUTHORS DESTINATION .)
else (WINDOWS)
	install(PROGRAMS icons/raceintospace.desktop DESTINATION share/applications)
	install(FILES icons/raceintospace.xpm icons/raceintospace.png DESTINATION share/pixmaps)
	install(FILES doc/org.raceintospace.Raceintospace.metainfo.xml
		DESTINATION share/metainfo)
        if(NOT DEFINED CPACK_GENERATOR)
                set(CPACK_GENERATOR TGZ)
        endif(NOT DEFINED CPACK_GENERATOR)

endif(WINDOWS)

# Output generated CPack package name to a file so we can read that in CI
configure_file("${PROJECT_SOURCE_DIR}/cmake/CpackWritePackageNames.cmake.in"
			   "${PROJECT_BINARY_DIR}/cmake/CpackWritePackageNames.cmake"
			   @ONLY)
set(CPACK_POST_BUILD_SCRIPTS "${PROJECT_BINARY_DIR}/cmake/CpackWritePackageNames.cmake")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Race into Space Team")
if(NOT WINDOWS)
       if(${CPACK_GENERATOR} MATCHES TGZ)
                INSTALL(PROGRAMS dist/linux-tgz/run.sh DESTINATION .)
       endif(${CPACK_GENERATOR} MATCHES TGZ)
endif(NOT WINDOWS)
include(CPack)

find_program(GIT git DOC "Git program path")

add_custom_target(dist
	COMMAND ${CMAKE_COMMAND} -D ARCHIVE_PREFIX="${CMAKE_PROJECT_NAME}-${raceintospace_VERSION_FULL}" -D SOURCE_DIR="${CMAKE_SOURCE_DIR}" -D OUTPUT_DIR="${CMAKE_BINARY_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/make_archive.cmake
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
	COMMAND ${GIT} tag -a v${raceintospace_VERSION_FULL})
