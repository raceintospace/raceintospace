set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/utils")

include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

include_directories(SYSTEM ${png_INCLUDE_DIR})

add_executable(but2png EXCLUDE_FROM_ALL but2png.c)
add_dependencies(but2png libs)
target_link_libraries(but2png ${png_LIBRARY})

add_executable(data_decoder EXCLUDE_FROM_ALL data_decoder.cpp)
target_link_libraries(data_decoder jsoncpp)

add_dependencies(data_decoder libs)
