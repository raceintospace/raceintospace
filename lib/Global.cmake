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
elseif (WINDOWS)
  set(BUILD_XIPH true)
  set(BUILD_PROTOBUF true)
  set(BUILD_BOOST true)
  set(BUILD_SDL true)
endif (APPLE)

if (BUILD_BOOST)
  # Boost_INCLUDE_DIR can remain unset, since it's already in the path
else (BUILD_BOOST)
  find_package(Boost)
endif (BUILD_BOOST)

if (BUILD_SDL)
else (BUILD_SDL)
  include(${CMAKE_ROOT}/Modules/FindSDL.cmake)
  include(SYSTEM ${SDL_INCLUDE_DIR})
endif (BUILD_SDL)

if (BUILD_XIPH)
  set(Ogg_LIBRARY ogg)
  set(Vorbis_LIBRARY vorbis)
  set(Theora_LIBRARY theora)
  
else (BUILD_XIPH)
  find_library(Ogg_LIBRARY NAMES ogg)
  find_path(Ogg_INCLUDE_DIR ogg/ogg.h)
  add_library(ogg STATIC IMPORTED)
  set_property(TARGET ogg APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(ogg PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Ogg_LIBRARY})

  find_library(Vorbis_LIBRARY NAMES vorbis)
  find_path(Vorbis_INCLUDE_DIR vorbis/codec.h)
  add_library(vorbis STATIC IMPORTED)
  set_property(TARGET vorbis APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(vorbis PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Vorbis_LIBRARY})

  find_library(Theora_LIBRARY NAMES theora)
  find_path(Theora_INCLUDE_DIR theora/theora.h)
  add_library(theora STATIC IMPORTED)
  set_property(TARGET theora APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(theora PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Theora_LIBRARY})
endif (BUILD_XIPH)
