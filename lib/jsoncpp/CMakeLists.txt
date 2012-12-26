cmake_minimum_required(VERSION 2.8)

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include)

add_library(jsoncpp
  src/lib_json/json_value.cpp
  src/lib_json/json_reader.cpp
  src/lib_json/json_writer.cpp
  )

install(
  TARGETS jsoncpp
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include/json"
  DESTINATION include/jsoncpp
  )
