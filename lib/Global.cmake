# We install locally-compiled stuff to somewhere inside the CMake build path
# Specifically -- ${CMAKE_BINARY_DIR}/local/{lib,include}
set (LocalPrefix ${CMAKE_BINARY_DIR}/local)

# Make sure we can find our installed files
include_directories(SYSTEM ${LocalPrefix}/include)
link_directories(${LocalPrefix}/lib)

# Set flags indicating what we should build
if (APPLE)
  set(BUILD_XIPH true)
endif (APPLE)

if (BUILD_XIPH)
  set(Ogg_LIBRARY ogg)
  set(Vorbis_LIBRARY vorbis)
  
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
endif (BUILD_XIPH)
