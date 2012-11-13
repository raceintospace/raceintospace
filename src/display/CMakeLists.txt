include_directories(${PROJECT_SOURCE_DIR}/src)

# Make sure we can access our local includes
include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

include_directories(SYSTEM ${png_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${SDL_INCLUDE_DIR})

add_library(
  raceintospace_display STATIC
  graphics.cpp
  palette.cpp
  png_image.cpp
  )

add_dependencies(raceintospace_display libs)
