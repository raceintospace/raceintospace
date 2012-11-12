cmake_minimum_required(VERSION 2.8)

include_directories(SYSTEM "${LocalPrefix}/include")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(theora
  lib/x86/mmxidct.c 
  lib/x86/mmxfrag.c 
  lib/x86/mmxstate.c
  lib/x86/x86state.c
  lib/encoder_disabled.c
  lib/apiwrapper.c
  lib/bitpack.c
  lib/decapiwrapper.c
  lib/decinfo.c
  lib/decode.c
  lib/dequant.c
  lib/fragment.c
  lib/huffdec.c
  lib/idct.c
  lib/info.c
  lib/internal.c
  lib/quant.c
  lib/state.c
  )

install(
  TARGETS theora
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include"
  DESTINATION .
  )
