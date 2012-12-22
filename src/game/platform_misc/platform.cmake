# Here too. This depends on probing the libsdl that may not have existed
# until after we started the build process.
add_definitions(-DHAVE_SDL_GETENV)

# Make sure we can fine fake_unistd.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform_misc)

set(app "raceintospace")

add_executable(${app}
  ${game_sources}
  music_vorbis.cpp
  platform_misc/main.c
  )

target_link_libraries(${app}
  ${game_libraries}
  raceintospace_display ${raceintospace_display_libraries}
  )

add_dependencies(${app} libs)
install(TARGETS raceintospace DESTINATION bin)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/gamedata
  ${PROJECT_SOURCE_DIR}/audio
  ${PROJECT_SOURCE_DIR}/video
  ${PROJECT_SOURCE_DIR}/midi
  ${PROJECT_SOURCE_DIR}/images
  DESTINATION share/raceintospace) 


