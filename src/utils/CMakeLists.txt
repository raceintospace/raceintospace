set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/utils")

include(${PROJECT_SOURCE_DIR}/lib/Global.cmake)

include_directories(SYSTEM ${png_INCLUDE_DIR})

add_executable(but2png EXCLUDE_FROM_ALL but2png.c)
set_target_properties(but2png PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
add_dependencies(but2png libs)
target_link_libraries(but2png ${png_LIBRARY} ${zlib_LIBRARY})

add_executable(data_decoder EXCLUDE_FROM_ALL data_decoder.cpp)
set_target_properties(data_decoder PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
add_dependencies(data_decoder libs)
target_link_libraries(data_decoder jsoncpp)

add_executable(news2png EXCLUDE_FROM_ALL news2png.cpp)
set_target_properties(news2png PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
add_dependencies(news2png libs)
target_link_libraries(news2png ${png_LIBRARY} ${zlib_LIBRARY})
