# This should really build an executable using sources in the platform_windows/ directory.
# It doesn't -- it builds a library instead.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform_windows)

add_library(raceintospace_game ${game_sources} music_vorbis.cpp)
add_dependencies(raceintospace_game libs)
