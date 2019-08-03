# Here's where we build external libraries that we need for the game
# Use CMake to download and compile these projects
cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
include(Global.cmake)

# Make a "libs" target that later chunks of code can depend on
add_custom_target(libs)

###
# Boost

if (BUILD_BOOST)
  set (Boost_Version 1.52.0)
  string (REPLACE "." "_" Boost_UnderscoreVersion ${Boost_Version})
  set (Boost_URL http://downloads.sourceforge.net/boost/boost/${Boost_Version}/boost_${Boost_UnderscoreVersion}.tar.gz)
  set (Boost_Dir ${CMAKE_CURRENT_BINARY_DIR}/boost-${Boost_Version})

  ExternalProject_Add(ext_boost
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${Boost_URL}
    PREFIX ${Boost_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/boost/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
    )
  
  add_dependencies(libs ext_boost)
endif (BUILD_BOOST)


###
# zlib

if (BUILD_ZLIB)
  set (zlib_Version 1.2.8)
  set (zlib_URL http://zlib.net/zlib-${zlib_Version}.tar.gz)
  set (zlib_Dir ${CMAKE_CURRENT_BINARY_DIR}/zlib-${zlib_Version})

  # zlib uses CMake, but it doesn't build on Mac OS X:
  #  https://github.com/madler/zlib/pull/26
  ExternalProject_Add(ext_zlib
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${zlib_URL}
    PREFIX ${zlib_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/zlib/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
    )

  add_dependencies(libs ext_zlib)
endif (BUILD_ZLIB)

###
# libpng

if (BUILD_LIBPNG)
  set (libpng_Version 1.5.13)
  set (libpng_URL http://download.sourceforge.net/libpng/libpng-${libpng_Version}.tar.gz)
  set (libpng_Dir ${CMAKE_CURRENT_BINARY_DIR}/libpng-${libpng_Version})

  ExternalProject_Add(ext_libpng
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libpng_URL}
    PREFIX ${libpng_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libpng/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
    )
  
  add_dependencies(libs ext_libpng)
  
  if (BUILD_ZLIB)
    add_dependencies(ext_libpng ext_zlib)
  endif (BUILD_ZLIB)
endif (BUILD_LIBPNG)

###
# SDL

if (BUILD_SDL)
  set (SDL_Version 1.2.15)
  set (SDL_Dir ${CMAKE_CURRENT_BINARY_DIR}/sdl-${SDL_Version})
  
  if (WINDOWS)
    set (SDL_URL http://www.libsdl.org/release/SDL-devel-${SDL_Version}-VC.zip)
    ExternalProject_Add(ext_sdl
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
      URL ${SDL_URL}
      PREFIX ${SDL_Dir}
      PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sdl/CMakeLists.windows.txt <SOURCE_DIR>/CMakeLists.txt
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
      )

  else (WINDOWS)
    message(FATAL_ERROR "I don't know how to build CMake on this platform.")
  endif (WINDOWS)

  add_dependencies(libs ext_sdl)
endif (BUILD_SDL)

###
# JsonCPP

if (BUILD_JSONCPP)
  set (JsonCPP_Version 0.5.0)
  set (JsonCPP_URL http://downloads.sourceforge.net/jsoncpp/jsoncpp/${JsonCPP_Version}/jsoncpp-src-${JsonCPP_Version}.tar.gz)
  set (JsonCPP_Dir ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp-${JsonCPP_Version})

  # JsonCPP uses scons, but it's actually really simple, so instead we ship and
  # build with our own jsoncpp-CMakeLists.txt
  ExternalProject_Add(ext_jsoncpp
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${JsonCPP_URL}
    PREFIX ${JsonCPP_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
    )

  add_dependencies(libs ext_jsoncpp)
endif (BUILD_JSONCPP)

if (BUILD_PHYSFS)
  ###
  # PhysicsFS
  set (physfs_Version 3.0.1)
  set (physfs_URL https://icculus.org/physfs/downloads/physfs-${physfs_Version}.tar.bz2)
  set (physfs_Dir ${CMAKE_CURRENT_BINARY_DIR}/physfs-${physfs_Version})

  # physfs complains about FSPathMakeRef et al being deprecated, and warnings are treated as errors
  # Turn it back into a warning instead
  if (APPLE)
    set (physfs_Flags -Wno-error=deprecated-declarations)
  endif (APPLE)

  ExternalProject_Add(ext_physfs
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${physfs_URL}
    PREFIX ${physfs_Dir}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DPHYSFS_ARCHIVE_7Z=false -DPHYSFS_ARCHIVE_GRP=false -DPHYSFS_ARCHIVE_WAD=false -DPHYSFS_ARCHIVE_HOG=false -DPHYSFS_ARCHIVE_MVL=false -DPHYSFS_ARCHIVE_QPAK=false -DPHYSFS_ARCHIVE_ISO9660=false -DPHYSFS_HAVE_CDROM_SUPPORT=false -DPHYSFS_BUILD_SHARED=false -DPHYSFS_BUILD_TEST=false -DCMAKE_C_FLAGS=${physfs_Flags}
    )

  add_dependencies(libs ext_physfs)

else (BUILD_PHYSFS)
  add_library(physfs STATIC IMPORTED)
  set_property(TARGET physfs APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(physfs PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Physfs_LIBRARY})
endif (BUILD_PHYSFS)


###
# Protocol Buffers

if (BUILD_PROTOBUF)
  set (protobuf_Version 2.6.1)
  set (protobuf_URL https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_Version}/protobuf-${protobuf_Version}.tar.gz)
  set (protobuf_Dir ${CMAKE_CURRENT_BINARY_DIR}/protobuf-${protobuf_Version})

  ExternalProject_Add(ext_protobuf
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${protobuf_URL}
    PREFIX ${protobuf_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DCONFIG_H_IN=${CMAKE_CURRENT_SOURCE_DIR}/protobuf/config.h.in
    )

  add_dependencies(libs ext_protobuf)
endif (BUILD_PROTOBUF)

###
# libogg

if (BUILD_XIPH)
  set (libogg_Version 1.3.0)
  set (libogg_URL http://downloads.xiph.org/releases/ogg/libogg-${libogg_Version}.tar.gz)
  set (libogg_Dir ${CMAKE_CURRENT_BINARY_DIR}/libogg-${libogg_Version})

  ExternalProject_Add(ext_libogg
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libogg_URL}
    PREFIX ${libogg_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libogg/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DCONFIG_TYPES_H_IN=${CMAKE_CURRENT_SOURCE_DIR}/libogg/config_types.h.in
    )

  add_dependencies(libs ext_libogg)

else (BUILD_XIPH)
  add_library(ogg STATIC IMPORTED)
  set_property(TARGET ogg APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(ogg PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Ogg_LIBRARY})

endif (BUILD_XIPH)


###
# libvorbis

if (BUILD_XIPH)
  set (libvorbis_Version 1.3.3)
  set (libvorbis_URL http://downloads.xiph.org/releases/vorbis/libvorbis-${libvorbis_Version}.tar.gz)
  set (libvorbis_Dir ${CMAKE_CURRENT_BINARY_DIR}/libvorbis-${libvorbis_Version})

  ExternalProject_Add(ext_libvorbis
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libvorbis_URL}
    PREFIX ${libvorbis_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DLocalPrefix=${LocalPrefix}
    )
  
  add_dependencies(libs ext_libogg)
  add_dependencies(ext_libvorbis ext_libogg)

else (BUILD_XIPH)
  add_library(vorbis STATIC IMPORTED)
  set_property(TARGET vorbis APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(vorbis PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Vorbis_LIBRARY})

endif (BUILD_XIPH)


###
# libtheora

if (BUILD_XIPH)
  set (libtheora_Version 1.1.1)
  set (libtheora_URL http://downloads.xiph.org/releases/theora/libtheora-${libtheora_Version}.tar.gz)
  set (libtheora_Dir ${CMAKE_CURRENT_BINARY_DIR}/libtheora-${libtheora_Version})

  ExternalProject_Add(ext_libtheora
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libtheora_URL}
    PREFIX ${libtheora_Dir}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libtheora/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DLocalPrefix=${LocalPrefix}
    )
  
  add_dependencies(libs ext_libtheora)
  add_dependencies(ext_libtheora ext_libogg)

else (BUILD_XIPH)
  add_library(theora STATIC IMPORTED)
  set_property(TARGET theora APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(theora PROPERTIES IMPORTED_LOCATION_NOCONFIG ${Theora_LIBRARY})
  
endif (BUILD_XIPH)
