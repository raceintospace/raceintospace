include_directories(${PROJECT_SOURCE_DIR}/src)

include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

add_library(
  raceintospace_display STATIC
  graphics.cpp
  palette.cpp
  png_image.cpp
  )

add_dependencies(raceintospace_display libs)
