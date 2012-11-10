# Here's where we build external libraries that we need for the game
# Use CMake to download and compile these projects
cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
include(Global.cmake)


###
# JsonCPP

set (JsonCPP_Version 0.5.0)
set (JsonCPP_URL http://downloads.sourceforge.net/jsoncpp/jsoncpp/${JsonCPP_Version}/jsoncpp-src-${JsonCPP_Version}.tar.gz)
set (JsonCPP_Dir ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp-${JsonCPP_Version})

# JsonCPP uses scons, but it's actually really simple, so instead we ship and
# build with our own jsoncpp-CMakeLists.txt
ExternalProject_Add(jsoncpp_build
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
  URL ${JsonCPP_URL}
  PREFIX ${JsonCPP_Dir}
  PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
  )

add_library(jsoncpp STATIC IMPORTED)
set_property(TARGET jsoncpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(jsoncpp PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libjsoncpp.a)
