# This file is included elsewhere to provide access to the libraries we need

# We install locally-compiled stuff to somewhere inside the CMake build path
# Specifically -- ${CMAKE_BINARY_DIR}/local/{lib,include}
set (LocalPrefix ${CMAKE_BINARY_DIR}/local)

# Make sure we can find our installed files
include_directories(SYSTEM ${LocalPrefix}/include)
link_directories(${LocalPrefix}/lib)

# Set flags indicating what we should build
if (APPLE)
  set(BUILD_JSONCPP true)
  set(BUILD_XIPH true)
  set(BUILD_PROTOBUF true)
  set(BUILD_LIBPNG true)
elseif (WINDOWS)
  set(BUILD_JSONCPP true)
  set(BUILD_ZLIB true)
  set(BUILD_LIBPNG true)
  set(BUILD_XIPH true)
  set(BUILD_PROTOBUF true)
  set(BUILD_BOOST true)
  set(BUILD_SDL true)
endif ()

if (BUILD_BOOST)
  # Boost_INCLUDE_DIR can remain unset, since it's already in the path
else (BUILD_BOOST)
  find_package(Boost)
endif (BUILD_BOOST)

if (BUILD_ZLIB)
  set(zlib_LIBRARY z)
else (BUILD_ZLIB)
  find_library(zlib_LIBRARY NAMES z)
  find_path(zlib_INCLUDE_DIR zlib.h)
endif (BUILD_ZLIB)

if (NOT DEFINED BUILD_LIBPNG)
  find_library(png_LIBRARY NAMES png)
  find_path(png_INCLUDE_DIR png.h)
  if (NOT png_LIBRARY OR NOT png_INCLUDE_DIR)
    set(png_LIBRARY png15)
    set(BUILD_LIBPNG ON)
  endif (NOT png_LIBRARY OR NOT png_INCLUDE_DIR)
elseif (BUILD_LIBPNG)
  set(png_LIBRARY png15)
  
else (BUILD_LIBPNG)
  find_library(png_LIBRARY NAMES png REQUIRED)
  find_path(png_INCLUDE_DIR png.h)
endif (NOT DEFINED BUILD_LIBPNG)

if (BUILD_SDL)
  set(SDL_LIBRARY SDL)
else (BUILD_SDL)
  include(${CMAKE_ROOT}/Modules/FindSDL.cmake)
  include_directories(SYSTEM ${SDL_INCLUDE_DIR})
endif (BUILD_SDL)

if (NOT DEFINED BUILD_XIPH)
  find_library(Ogg_LIBRARY NAMES ogg)
  find_path(Ogg_INCLUDE_DIR ogg/ogg.h)

  find_library(Vorbis_LIBRARY NAMES vorbis)
  find_path(Vorbis_INCLUDE_DIR vorbis/codec.h)

  find_library(Theora_LIBRARY NAMES theora)
  find_path(Theora_INCLUDE_DIR theora/theora.h)

  if (NOT Ogg_LIBRARY OR NOT Ogg_INCLUDE_DIR)
    set(Ogg_LIBRARY ogg)
    set(BUILD_XIPH ON)
  endif (NOT Ogg_LIBRARY OR NOT Ogg_INCLUDE_DIR)

  if (NOT Vorbis_LIBRARY OR NOT Vorbis_INCLUDE_DIR)
    set(Vorbis_LIBRARY vorbis)
    set(BUILD_XIPH ON)
  endif (NOT Vorbis_LIBRARY OR NOT Vorbis_INCLUDE_DIR)

  if (NOT Theora_LIBRARY OR NOT Theora_INCLUDE_DIR)
    set(Theora_LIBRARY theora)
    set(BUILD_XIPH ON)
  endif (NOT Theora_LIBRARY OR NOT Theora_INCLUDE_DIR)

elseif (BUILD_XIPH)
  set(Ogg_LIBRARY ogg)
  set(Vorbis_LIBRARY vorbis)
  set(Theora_LIBRARY theora)
  
else (BUILD_XIPH)
  find_library(Ogg_LIBRARY NAMES ogg)
  find_path(Ogg_INCLUDE_DIR ogg/ogg.h)

  find_library(Vorbis_LIBRARY NAMES vorbis)
  find_path(Vorbis_INCLUDE_DIR vorbis/codec.h)

  find_library(Theora_LIBRARY NAMES theora)
  find_path(Theora_INCLUDE_DIR theora/theora.h)
endif (NOT DEFINED BUILD_XIPH)

if (NOT DEFINED BUILD_JSONCPP)
  find_library(JsonCPP_LIBRARY NAMES jsoncpp)
  find_path(JsonCPP_INCLUDE_DIR json/json.h PATH_SUFFIXES jsoncpp)
  if (NOT JsonCPP_LIBRARY OR NOT JsonCPP_INCLUDE_DIR)
    set(BUILD_JSONCPP ON)
    set(JsonCPP_LIBRARY jsoncpp)
    include_directories(SYSTEM ${LocalPrefix}/include/jsoncpp)
  endif ()

elseif (BUILD_JSONCPP)
  set(JsonCPP_LIBRARY jsoncpp)
  include_directories(SYSTEM ${LocalPrefix}/include/jsoncpp)
else (BUILD_JSONCPP)
  find_library(JsonCPP_LIBRARY NAMES jsoncpp)
  find_path(JsonCPP_INCLUDE_DIR json/json.h PATH_SUFFIXES jsoncpp)
endif (NOT DEFINED BUILD_JSONCPP)

if (NOT DEFINED BUILD_PHYSFS)
  find_library(Physfs_LIBRARY NAMES physfs)
  find_path(Physfs_INCLUDE_DIR json/json.h physfs)
  if (NOT Physfs_LIBRARY OR NOT Physfs_INCLUDE_DIR)
    set(BUILD_PHYSFS ON)
    set(Physfs_LIBRARY physfs)
  endif ()

elseif (BUILD_PHYSFS)
  set(Physfs_LIBRARY physfs)

else (BUILD_PHYSFS)
  find_library(Physfs_LIBRARY NAMES physfs)
  find_path(Physfs_INCLUDE_DIR json/json.h physfs)
endif (NOT DEFINED BUILD_PHYSFS)
