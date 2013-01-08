include_directories(${PROJECT_SOURCE_DIR}/src)

include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

add_library(
  raceintospace_display STATIC
  graphics.cpp
  legacy_surface.cpp
  image.cpp
  palette.cpp
  palettized_surface.cpp
  surface.cpp
  )

add_dependencies(raceintospace_display libs)
