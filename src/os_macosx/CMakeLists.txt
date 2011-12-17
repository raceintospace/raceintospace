include_directories(../game)
include_directories(${PROJECT_BINARY_DIR}/config)
include_directories(SYSTEM ${SDL_INCLUDE_DIR})

# Get the OSX frameworks we need
find_library(CoreMIDI_LIBRARY NAMES CoreMIDI REQUIRED)
find_library(CoreAudio_LIBRARY NAMES CoreAudio REQUIRED)
find_library(AudioToolbox_LIBRARY NAMES AudioToolbox REQUIRED)
find_library(AudioUnit_LIBRARY NAMES AudioUnit REQUIRED)

# Sparkle!
#file(DOWNLOAD http://sparkle.andymatuschak.org/files/Sparkle%201.5b6.zip sparkle.zip)

# Set up the bundle
include(CopyNIB.cmake)
CopyNIB(SDLMain.nib ${CMAKE_CURRENT_BINARY_DIR}/SDLMain.nib)

set(BundleResources
  moon.icns
  ${CMAKE_CURRENT_BINARY_DIR}/SDLMain.nib
  )

# Build the .app in the CMake build root
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(app "Race Into Space")

add_executable("${app}" MACOSX_BUNDLE
  SDLMain.m music_osx.c
  ${BundleResources}
  )

target_link_libraries("${app}"
  raceintospace_game ${raceintospace_game_libraries}
  ${CoreMIDI_LIBRARY} ${CoreAudio_LIBRARY} ${AudioToolbox_LIBRARY} ${AudioUnit_LIBRARY}
  )

set_target_properties("${app}" PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/os_macosx/Info.plist.in
  )

set_source_files_properties(${BundleResources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

