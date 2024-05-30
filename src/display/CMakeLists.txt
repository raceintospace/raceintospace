# include_directories(${PROJECT_SOURCE_DIR}/src)

# include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

find_package(SDL REQUIRED)
find_package(Boost REQUIRED)
find_package(PNG REQUIRED)
add_library(
  raceintospace_display STATIC
  graphics.cpp
  legacy_surface.cpp
  image.cpp
  palette.cpp
  palettized_surface.cpp
  surface.cpp
  )
target_link_libraries(raceintospace_display PUBLIC SDL::SDL)
target_link_libraries(raceintospace_display PRIVATE PNG::PNG)
target_include_directories(raceintospace_display PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(raceintospace_display PUBLIC ${PROJECT_SOURCE_DIR}/src)

# add_dependencies(raceintospace_display)
