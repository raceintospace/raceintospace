include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(SYSTEM ${png_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

add_library(
  raceintospace_display STATIC
  palette.cpp
  png_image.cpp
  )
