cmake_minimum_required(VERSION 2.6)

# Cereal is header-only, so just install them

install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include"
  DESTINATION .
  )
