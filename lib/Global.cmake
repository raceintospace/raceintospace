# We install locally-compiled stuff to somewhere inside the CMake build path
# Specifically -- ${CMAKE_BINARY_DIR}/local/{lib,include}
set (LocalPrefix ${CMAKE_BINARY_DIR}/local)

# Make sure we can find our installed files
include_directories(SYSTEM ${LocalPrefix}/include)
link_directories(${LocalPrefix}/lib)
