# Here's where we build external libraries that we need for the game
# Use CMake to download and compile these projects
cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
include(Global.cmake)


###
# Boost

if (BUILD_BOOST)
  set (Boost_Version 1.52.0)
  string (REPLACE "." "_" Boost_UnderscoreVersion ${Boost_Version})
  set (Boost_URL http://downloads.sourceforge.net/boost/boost/${Boost_Version}/boost_${Boost_UnderscoreVersion}.tar.gz)
  set (Boost_Dir ${CMAKE_CURRENT_BINARY_DIR}/boost-${Boost_Version})

  ExternalProject_Add(boost_build
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${Boost_URL}
    PREFIX ${Boost_Dir}
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/boost/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
    )
endif (BUILD_BOOST)

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
  PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix}
  )

add_library(jsoncpp STATIC IMPORTED)
set_property(TARGET jsoncpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(jsoncpp PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libjsoncpp.a)


###
# PhysicsFS

set (physfs_Version 2.0.3)
set (physfs_URL http://icculus.org/physfs/downloads/physfs-${physfs_Version}.tar.bz2)
set (physfs_Dir ${CMAKE_CURRENT_BINARY_DIR}/physfs-${physfs_Version})

# physfs 2.0.3 complains about FSPathMakeRef et al being deprecated, and warnings are treated as errors
# Turn it back into a warning instead
if (APPLE)
  set (physfs_Flags -Wno-error=deprecated-declarations)
endif (APPLE)

ExternalProject_Add(physfs_build
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
  URL ${physfs_URL}
  PREFIX ${physfs_Dir}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DPHYSFS_ARCHIVE_GRP=false -DPHYSFS_ARCHIVE_WAD=false -DPHYSFS_ARCHIVE_HOG=false -DPHYSFS_ARCHIVE_MVL=false -DPHYSFS_ARCHIVE_QPAK=false -DPHYSFS_HAVE_CDROM_SUPPORT=false -DCMAKE_C_FLAGS=${physfs_Flags}
  )

add_library(physfs STATIC IMPORTED)
set_property(TARGET physfs APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(physfs PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libphysfs.a)


###
# Protocol Buffers

if (BUILD_PROTOBUF)
  set (protobuf_Version 2.4.1)
  set (protobuf_URL http://protobuf.googlecode.com/files/protobuf-${protobuf_Version}.tar.gz)
  set (protobuf_Dir ${CMAKE_CURRENT_BINARY_DIR}/protobuf-${protobuf_Version})

  ExternalProject_Add(protobuf_build
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${protobuf_URL}
    PREFIX ${protobuf_Dir}
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DCONFIG_H_IN=${CMAKE_CURRENT_SOURCE_DIR}/protobuf/config.h.in
    )

endif (BUILD_PROTOBUF)

###
# libogg

if (BUILD_XIPH)
  set (libogg_Version 1.3.0)
  set (libogg_URL http://downloads.xiph.org/releases/ogg/libogg-${libogg_Version}.tar.gz)
  set (libogg_Dir ${CMAKE_CURRENT_BINARY_DIR}/libogg-${libogg_Version})

  ExternalProject_Add(libogg_build
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libogg_URL}
    PREFIX ${libogg_Dir}
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/libogg/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DCONFIG_TYPES_H_IN=${CMAKE_CURRENT_SOURCE_DIR}/libogg/config_types.h.in
    )

  add_library(ogg STATIC IMPORTED)
  set_property(TARGET ogg APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(ogg PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libogg.a)
endif (BUILD_XIPH)


###
# libvorbis

if (BUILD_XIPH)
  set (libvorbis_Version 1.3.3)
  set (libvorbis_URL http://downloads.xiph.org/releases/vorbis/libvorbis-${libvorbis_Version}.tar.gz)
  set (libvorbis_Dir ${CMAKE_CURRENT_BINARY_DIR}/libvorbis-${libvorbis_Version})

  ExternalProject_Add(libvorbis_build
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libvorbis_URL}
    PREFIX ${libvorbis_Dir}
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/libvorbis/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DLocalPrefix=${LocalPrefix}
    )
  
  add_dependencies(libvorbis_build libogg_build)
  
  add_library(vorbis STATIC IMPORTED)
  set_property(TARGET vorbis APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(vorbis PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libvorbis.a)
endif (BUILD_XIPH)


###
# libtheora

if (BUILD_XIPH)
  set (libtheora_Version 1.1.1)
  set (libtheora_URL http://downloads.xiph.org/releases/theora/libtheora-${libtheora_Version}.tar.gz)
  set (libtheora_Dir ${CMAKE_CURRENT_BINARY_DIR}/libtheora-${libtheora_Version})

  ExternalProject_Add(libtheora_build
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    URL ${libtheora_URL}
    PREFIX ${libtheora_Dir}
    PATCH_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/libtheora/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${LocalPrefix} -DLocalPrefix=${LocalPrefix}
    )
  
  add_dependencies(libtheora_build libogg_build)
  
  add_library(theora STATIC IMPORTED)
  set_property(TARGET theora APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(theora PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LocalPrefix}/lib/libtheora.a)
endif (BUILD_XIPH)
