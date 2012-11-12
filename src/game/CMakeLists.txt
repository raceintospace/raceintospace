include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/src/game)
include_directories(SYSTEM ${SDL_INCLUDE_DIR})

# Make sure we can access our local includes
include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

set(game_sources
  admin.cpp
  aimast.cpp
  aimis.cpp
  aipur.cpp
  ast0.cpp
  ast1.cpp
  ast2.cpp
  ast3.cpp
  ast4.cpp
  budget.cpp
  crash.cpp
  crew.cpp
  endgame.cpp
  endianness.cpp
  fortify_workaround.cpp
  fs.cpp
  futbub.cpp
  future.cpp
  gamedata.cpp
  game_main.cpp
  gr.cpp
  gx.cpp
  hardef.cpp
  intel.cpp
  intro.cpp
  log4c.cpp
  log_default.cpp
  logging.cpp
  mc.cpp
  mc2.cpp
  mis_c.cpp
  mis_m.cpp
  mmfile.cpp
  museum.cpp
  newmis.cpp
  news.cpp
  news_sup.cpp
  news_suq.cpp
  options.cpp
  pace.cpp
  place.cpp
  port.cpp
  prefs.cpp
  prest.cpp
  radar.cpp
  randomize.cpp
  rdplex.cpp
  records.cpp
  replay.cpp
  review.cpp
  roster.cpp
  roster_group.cpp
  roster_entry.cpp
  rush.cpp
  sdlhelper.cpp
  start.cpp
  utils.cpp
  vab.cpp
  )

set(game_libraries
  ${SDL_LIBRARY} ogg vorbis ${Theora_LIBRARY}
  jsoncpp
  )

set(game_dependencies
  jsoncpp_build
  )

# Defer to the platform-specific CMakeLists for building the actual game target
if (APPLE)
  include(CMakeLists.macosx.txt)
else (APPLE)
  add_library(raceintospace_game ${game_sources} music_vorbis.cpp)
  add_dependencies(raceintospace_game ${game_dependencies})
endif (APPLE)
