include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/src/game)

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
  file.cpp
  filesystem.cpp
  fortify_workaround.cpp
  fs.cpp
  futbub.cpp
  future.cpp
  gamedata.cpp
  game_main.cpp
  gr.cpp
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
  ${SDL_LIBRARY} ogg vorbis theora
  jsoncpp physfs ${zlib_LIBRARY}
  )

# Defer to the platform-specific CMakeLists for building the actual game target
if (APPLE)
  include(platform_macosx/platform.cmake)
elseif (WINDOWS)
  include(platform_windows/platform.cmake)
else()
  include(platform_misc/platform.cmake)
endif()
