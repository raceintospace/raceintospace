include_directories(${PROJECT_SOURCE_DIR}/src)

# From http://www.cmake.org/pipermail/cmake/2008-September/023963.html
FUNCTION(WRAP_PROTO VAR)
  IF (NOT ARGN)
    MESSAGE(SEND_ERROR "Error: WRAP PROTO called without any proto files")
    RETURN()
  ENDIF(NOT ARGN)

  if(TARGET ext_protobuf)
    set(protoc ${LocalPrefix}/bin/protoc)
    set(deps ext_protobuf)
  else()
    set(protoc protoc)
    set(deps)
  endif()

  SET(INCL)
  SET(${VAR})
  FOREACH(FIL ${ARGN})
    GET_FILENAME_COMPONENT(ABS_FIL ${FIL} ABSOLUTE)
    GET_FILENAME_COMPONENT(FIL_WE ${FIL} NAME_WE)
    LIST(APPEND ${VAR} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
    LIST(APPEND INCL "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

    ADD_CUSTOM_COMMAND(
      OUTPUT ${${VAR}} ${INCL}
      COMMAND ${protoc}
      ARGS --cpp_out  ${CMAKE_CURRENT_BINARY_DIR} --proto_path ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL}
      DEPENDS ${ABS_FIL} ${deps}
      COMMENT "Running protocol buffer compiler on ${FIL}"
      VERBATIM )
  ENDFOREACH(FIL)

  SET(${VAR} ${${VAR}} PARENT_SCOPE)
ENDFUNCTION(WRAP_PROTO)

wrap_proto(raceintospace_proto_impl raceintospace.proto)

add_library(
  raceintospace_protobuf STATIC
  ${raceintospace_proto_impl}
  )

set(raceintospace_protobuf_include_dir ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_dependencies(raceintospace_protobuf libs)
