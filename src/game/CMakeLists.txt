include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/config)
include_directories(SYSTEM ${SDL_INCLUDE_DIR})

if (PLATFORM_MUSIC)
  # do nothing
else (PLATFORM_MUSIC)
  set(music_vorbis music_vorbis.cpp)
endif (PLATFORM_MUSIC)

add_library(
  raceintospace_game STATIC
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
  ${music_vorbis}
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
  rush.cpp
  sdl.cpp
  start.cpp
  utils.cpp
  vab.cpp
  )
