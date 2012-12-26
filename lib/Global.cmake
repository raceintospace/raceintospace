# This file is included elsewhere to provide access to the libraries we need

# We install locally-compiled stuff to somewhere inside the CMake build path
# Specifically -- ${CMAKE_BINARY_DIR}/local/{lib,include}
set (LocalPrefix ${CMAKE_BINARY_DIR}/local)

# Make sure we can find our installed files
include_directories(SYSTEM ${LocalPrefix}/include)
link_directories(${LocalPrefix}/lib)

# Set flags indicating what we should build
if (APPLE)
  set(BUILD_XIPH true)
  set(BUILD_PROTOBUF true)
  set(BUILD_LIBPNG true)
  set(BUILD_PHYSFS true)
  set(BUILD_JSONCPP true)
elseif (WINDOWS)
  set(BUILD_ZLIB true)
  set(BUILD_LIBPNG true)
  set(BUILD_XIPH true)
  set(BUILD_PROTOBUF true)
  set(BUILD_BOOST true)
  set(BUILD_SDL true)
  set(BUILD_PHYSFS true)
  set(BUILD_JSONCPP true)
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

if (BUILD_LIBPNG)
  set(png_LIBRARY libpng15)
  
  # Debug builds of libpng end up with a "d" suffix
  if (CMAKE_BUILD_TYPE EQUAL Debug)
    set(png_LIBRARY ${png_LIBRARY}d)
  endif ()
  
  if (NOT WINDOWS)
    set(png_LIBRARY ${LocalPrefix}/lib/${png_LIBRARY}.a)
  endif ()

else (BUILD_LIBPNG)
  find_library(png_LIBRARY NAMES png REQUIRED)
  find_path(png_INCLUDE_DIR png.h)
endif (BUILD_LIBPNG)

if (BUILD_SDL)
  set(SDL_LIBRARY SDL)
else (BUILD_SDL)
  include(${CMAKE_ROOT}/Modules/FindSDL.cmake)
  include_directories(SYSTEM ${SDL_INCLUDE_DIR})
endif (BUILD_SDL)

if (BUILD_XIPH)
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
endif (BUILD_XIPH)
