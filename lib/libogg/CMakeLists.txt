cmake_minimum_required(VERSION 2.8)

include(CheckIncludeFile)
check_include_file(dirent.h INCLUDE_INTTYPES_H)
check_include_file(stdint.h INCLUDE_STDINT_H)
check_include_file(sys/types.h INCLUDE_SYS_TYPES_H)

configure_file(${CONFIG_TYPES_H_IN} "${PROJECT_SOURCE_DIR}/include/ogg/config_types.h")

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include)

add_library(ogg
  src/bitwise.c
  src/framing.c
  )

install(
  TARGETS ogg
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include"
  DESTINATION .
  )
