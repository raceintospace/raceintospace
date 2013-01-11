# I would really like to be able to remove this, but we need to convert all
# the users to raceintospace_config.h instead.
add_definitions(-DCONFIG_WIN32)

# Here too. This depends on probing the libsdl that may not have existed
# until after we started the build process.
add_definitions(-DHAVE_SDL_GETENV)

# Make sure we can fine fake_unistd.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform_windows)

set(app "Race Into Space")

add_executable(${app}
  ${game_sources}
  music_vorbis.cpp
  platform_windows/dirent.c
  platform_windows/main.c
  )

target_link_libraries(${app} ${game_libraries})

add_dependencies(${app} libs)
