cmake_minimum_required(VERSION 2.6)

# Do nothing but copy the headers
# Yes, this is lazy, but it avoids invoking yet another build tool

install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/boost"
  DESTINATION include
  PATTERN "*.hpp"
  )
