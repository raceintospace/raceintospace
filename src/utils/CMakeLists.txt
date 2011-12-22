set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/utils")

include_directories(SYSTEM ${png_INCLUDE_DIR})

foreach(util but2png data_decoder)
  add_executable(${util} ${util}.c)
endforeach(util)

target_link_libraries(but2png ${png_LIBRARY})
