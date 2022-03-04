# I would really like to be able to remove this, but we need to convert all
# the users to raceintospace_config.h instead.
add_definitions(-DCONFIG_WIN32)

include(${CMAKE_SOURCE_DIR}/lib/Global.cmake)

# Here too. This depends on probing the libsdl that may not have existed
# until after we started the build process.
add_definitions(-DHAVE_SDL_GETENV
				-DNOMINMAX # std::max() fix on windows
				-D_USE_MATH_DEFINES
				)

# Make sure we can find fake_unistd.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform_windows)

# Add the platform-specific source files needed for testing
list(APPEND ui_sources music_vorbis.cpp)

set(app raceintospace)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Race Into Space")
set(CPACK_NSIS_PACKAGE_NAME "Race Into Space")
set(CPACK_NSIS_DISPLAY_NAME "Race Into Space")
set(CPACK_NSIS_URL_INFO_ABOUT "http://raceintospace.org")
set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/src/os_win32/Moon.ico)
# Not sure how to set working icon path
#set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}\icons\Moon.ico)
#set(CPACK_INSTALL_CMAKE_PROJECTS applications data)
set(CPACK_SOURCE_IGNORE_FILES vsbuild)
set(CPACK_PACKAGE_EXECUTABLES raceintospace "Race into Space")

add_executable(raceintospace
  ${game_sources}
  ${ui_sources}
  platform_windows/dirent.c
  platform_windows/main.c
  )

add_library(SDL SHARED IMPORTED)
set_target_properties(SDL PROPERTIES
  IMPORTED_LOCATION "${LocalPrefix}/bin/SDL.dll"
  IMPORTED_IMPLIB "${LocalPrefix}/lib/SDL.lib"
)

install(TARGETS "${app}" DESTINATION bin COMPONENT application)
# Do not know how to make SDL target work. Workaround
install(PROGRAMS "${LocalPrefix}/bin/SDL.dll" DESTINATION bin COMPONENT runtime)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION data COMPONENT data)

target_link_libraries(${app} ${game_libraries} SDL)
set_target_properties(${app} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "BARIS_DATA=${PROJECT_SOURCE_DIR}/data")

add_dependencies(${app} libs)
