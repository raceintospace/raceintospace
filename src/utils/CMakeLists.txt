set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/utils")

find_library(png_LIBRARY NAMES png REQUIRED PATHS /usr/X11/lib)
find_path(png_INCLUDE_DIR png.h PATHS /usr/X11/include)
include_directories(SYSTEM ${png_INCLUDE_DIR})

foreach(util but2png)
  add_executable(${util} ${util}.c)
endforeach(util)

target_link_libraries(but2png ${png_LIBRARY})
