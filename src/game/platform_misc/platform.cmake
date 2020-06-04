# Here too. This depends on probing the libsdl that may not have existed
# until after we started the build process.
add_definitions(-DHAVE_SDL_GETENV)

# Make sure we can find fake_unistd.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform_misc)

# Add the platform-specific source files needed for testing
list(APPEND ui_sources music_vorbis.cpp)

set(app "raceintospace")

add_executable(${app}
  ${game_sources}
  ${ui_sources}
  platform_misc/main.c
  )

target_link_libraries(${app} ${game_libraries})

add_dependencies(${app} libs)
install(TARGETS raceintospace DESTINATION bin)

file(GLOB data_dirs "${PROJECT_SOURCE_DIR}/data/*")
install(DIRECTORY ${data_dirs} DESTINATION share/raceintospace) 
