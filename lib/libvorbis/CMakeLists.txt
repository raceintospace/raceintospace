cmake_minimum_required(VERSION 2.8)

include_directories(SYSTEM "${LocalPrefix}/include")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(vorbis
  lib/mdct.c
  lib/smallft.c
  lib/block.c
  lib/envelope.c
  lib/window.c
  lib/lsp.c
  lib/lpc.c
  lib/analysis.c
  lib/synthesis.c
  lib/psy.c
  lib/info.c
  lib/floor1.c
  lib/floor0.c
  lib/res0.c
  lib/mapping0.c
  lib/registry.c
  lib/codebook.c
  lib/sharedbook.c
  lib/lookup.c
  lib/bitrate.c
  )

install(
  TARGETS vorbis
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include"
  DESTINATION .
  )
