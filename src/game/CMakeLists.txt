include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/src/game)
include_directories(${raceintospace_protobuf_include_dir})
if (JsonCPP_INCLUDE_DIR)
  include_directories(${JsonCPP_INCLUDE_DIR})
endif (JsonCPP_INCLUDE_DIR)

# Make sure we can access our local includes
include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

# Silence some warnings on MSVC
if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif (MSVC)

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
  downgrader.cpp
  draw.cpp
  endgame.cpp
  endianness.cpp
  file.cpp
  filesystem.cpp
  fireworks.cpp
  fortify_workaround.cpp
  fs.cpp
  futbub.cpp
  future.cpp
  gamedata.cpp
  game_main.cpp
  gr.cpp
  hardef.cpp
  hardware.cpp
  hardware_buttons.cpp
  intel.cpp
  intro.cpp
  log4c.cpp
  log_default.cpp
  logging.cpp
  mc.cpp
  mc2.cpp
  mis_c.cpp
  mis_m.cpp
  mission_util.cpp
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
  start.cpp
  state_utils.cpp
  utils.cpp
  vab.cpp
  vehicle.cpp
  sdlhelper.cpp
  )

# Files related only to UI
set (ui_sources
  )

set(game_libraries
  ${SDL_LIBRARY} ogg vorbis theora protobuf
  ${JsonCPP_LIBRARY} physfs ${zlib_LIBRARY}
  raceintospace_display ${raceintospace_display_libraries}
  raceintospace_protobuf
  )

# Defer to the platform-specific CMakeLists for building the actual game target
if (APPLE)
  include(platform_macosx/platform.cmake)
elseif (WINDOWS)
  include(platform_windows/platform.cmake)
else()
  include(platform_misc/platform.cmake)
endif()

# Run this after the platform includes so ${game_sources} will be
# populated with platform-specific files.
# Not using (file GLOB ...) because CMake documentation recommends
# against it (https://cmake.org/cmake/help/v3.14/command/file.html)
set(test_dir ${PROJECT_SOURCE_DIR}/test)
set(test_sources
  music_none.cpp
  ${test_dir}/game/dummy_test.cpp
  ${test_dir}/game/mission_test.cpp
  ${test_dir}/game/downgrade_test.cpp
  ${test_dir}/game/roster_test.cpp
  )

add_executable(game_test ../../test/test_main.cpp ${test_sources} ${game_sources})
set_target_properties(game_test PROPERTIES COMPILE_FLAGS "-DBOOST_TEST_NO_LIB=1")
target_link_libraries(game_test ${game_libraries})
add_test(
  NAME game_test
  COMMAND game_test --catch_system_errors=yes
  )
