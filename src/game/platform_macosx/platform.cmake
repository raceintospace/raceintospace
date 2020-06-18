# Get the OSX frameworks we need
find_library(CoreMIDI_LIBRARY NAMES CoreMIDI REQUIRED)
find_library(CoreAudio_LIBRARY NAMES CoreAudio REQUIRED)
find_library(AudioToolbox_LIBRARY NAMES AudioToolbox REQUIRED)
find_library(AudioUnit_LIBRARY NAMES AudioUnit REQUIRED)
find_library(IOKit_LIBRARY NAMES IOKit REQUIRED)

# Sparkle!
#file(DOWNLOAD http://sparkle.andymatuschak.org/files/Sparkle%201.5b6.zip sparkle.zip)

# Set up the bundle
include(platform_macosx/CopyNIB.cmake)
CopyNIB(platform_macosx/SDLMain.nib ${CMAKE_CURRENT_BINARY_DIR}/SDLMain.nib)
set(BundleResources
  platform_macosx/moon.icns
  ${CMAKE_CURRENT_BINARY_DIR}/SDLMain.nib
  )
set_source_files_properties(${BundleResources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# Copy all these files into the bundle
file(GLOB_RECURSE game_datafiles
  RELATIVE ${PROJECT_SOURCE_DIR}/data/
  ${PROJECT_SOURCE_DIR}/data/*
  )
foreach(datafile ${game_datafiles})
  get_filename_component(parent_dir "${datafile}" PATH)
  set(abspath "${PROJECT_SOURCE_DIR}/data/${datafile}")
  set_source_files_properties("${abspath}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${parent_dir}")
  list(APPEND BundleResources "${PROJECT_SOURCE_DIR}/data/${datafile}")
endforeach(datafile)

# Build the .app in the CMake build root
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# Add the platform-specific source files needed for testing
list(APPEND ui_sources platform_macosx/music_osx.cpp)

set(app "Race Into Space")

add_executable("${app}" MACOSX_BUNDLE
  platform_macosx/SDLMain.m
  ${game_sources}
  ${ui_sources}
  ${BundleResources}
  )

target_link_libraries("${app}"
  ${game_libraries}
  ${CoreMIDI_LIBRARY} ${CoreAudio_LIBRARY} ${AudioToolbox_LIBRARY} ${AudioUnit_LIBRARY} ${IOKit_LIBRARY}
  )

add_dependencies("${app}" libs)

set_target_properties("${app}" PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/game/platform_macosx/Info.plist.in
  MACOSX_BUNDLE_BUNDLE_NAME ${app}
  MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION}
  )
