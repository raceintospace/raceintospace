cmake_minimum_required(VERSION 2.8)

set(PACKAGE_VERSION 2.6.1)

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include)

if (WINDOWS)
  # Use the config.h intended for Windows shipped by Google
  include_directories(${PROJECT_SOURCE_DIR}/vsprojects)
else (WINDOWS)
  include(CheckIncludeFile)
  check_include_file(fcntl.h HAVE_FCNTL_H)
  check_include_file(inttypes.h HAVE_INTTYPES_H)
  check_include_file(limits.h HAVE_LIMITS_H)
  check_include_file(stdlib.h HAVE_STDLIB_H)
  check_include_file(stdint.h INCLUDE_STDINT_H)
  check_include_file(unistd.h INCLUDE_UNISTD_H)

  include(CheckFunctionExists)
  check_function_exists(ftruncate HAVE_FTRUNCATE)
  check_function_exists(memcmp HAVE_MEMCMP)
  check_function_exists(memset HAVE_MEMSET)
  check_function_exists(mkdir HAVE_MKDIR)
  check_function_exists(strchr HAVE_STRCHR)
  check_function_exists(strerror HAVE_STRERROR)
  check_function_exists(strtol HAVE_STRTOL)
  check_function_exists(strtod HAVE_STRTOD)

  check_include_file(pthread.h HAVE_PTHREAD)

  set(PACKAGE_NAME protobuf)
  set(PACKAGE_TARNAME protobuf)
  set(PACKAGE_STRING ${PACKAGE_TARNAME}-${PACKAGE_VERSION})
  
  configure_file(${CONFIG_H_IN} "${PROJECT_SOURCE_DIR}/config.h")
endif (WINDOWS)

include_directories(.)
include_directories(SYSTEM src)

add_library(protobuf_lite
  src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
  src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
  src/google/protobuf/stubs/common.cc
  src/google/protobuf/stubs/once.cc
  src/google/protobuf/stubs/stringprintf.cc
  src/google/protobuf/extension_set.cc
  src/google/protobuf/generated_message_util.cc
  src/google/protobuf/message_lite.cc
  src/google/protobuf/repeated_field.cc
  src/google/protobuf/wire_format_lite.cc
  src/google/protobuf/io/coded_stream.cc
  src/google/protobuf/io/zero_copy_stream.cc
  src/google/protobuf/io/zero_copy_stream_impl_lite.cc
  )

add_library(protobuf
  src/google/protobuf/stubs/strutil.cc
  src/google/protobuf/stubs/substitute.cc
  src/google/protobuf/stubs/structurally_valid.cc
  src/google/protobuf/descriptor.cc
  src/google/protobuf/descriptor.pb.cc
  src/google/protobuf/descriptor_database.cc
  src/google/protobuf/dynamic_message.cc
  src/google/protobuf/extension_set_heavy.cc
  src/google/protobuf/generated_message_reflection.cc
  src/google/protobuf/message.cc
  src/google/protobuf/reflection_ops.cc
  src/google/protobuf/service.cc
  src/google/protobuf/text_format.cc
  src/google/protobuf/unknown_field_set.cc
  src/google/protobuf/wire_format.cc
  src/google/protobuf/io/gzip_stream.cc
  src/google/protobuf/io/printer.cc
  src/google/protobuf/io/strtod.cc
  src/google/protobuf/io/tokenizer.cc
  src/google/protobuf/io/zero_copy_stream_impl.cc
  src/google/protobuf/compiler/importer.cc
  src/google/protobuf/compiler/parser.cc
  )

target_link_libraries(protobuf protobuf_lite)

add_executable(protoc
  # libprotoc
  src/google/protobuf/compiler/code_generator.cc
  src/google/protobuf/compiler/command_line_interface.cc
  src/google/protobuf/compiler/plugin.cc
  src/google/protobuf/compiler/plugin.pb.cc
  src/google/protobuf/compiler/subprocess.cc
  src/google/protobuf/compiler/zip_writer.cc
  src/google/protobuf/compiler/cpp/cpp_enum.cc
  src/google/protobuf/compiler/cpp/cpp_enum_field.cc
  src/google/protobuf/compiler/cpp/cpp_extension.cc
  src/google/protobuf/compiler/cpp/cpp_field.cc
  src/google/protobuf/compiler/cpp/cpp_file.cc
  src/google/protobuf/compiler/cpp/cpp_generator.cc
  src/google/protobuf/compiler/cpp/cpp_helpers.cc
  src/google/protobuf/compiler/cpp/cpp_message.cc
  src/google/protobuf/compiler/cpp/cpp_message_field.cc
  src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
  src/google/protobuf/compiler/cpp/cpp_service.cc
  src/google/protobuf/compiler/cpp/cpp_string_field.cc
  src/google/protobuf/compiler/java/java_context.cc
  src/google/protobuf/compiler/java/java_enum.cc
  src/google/protobuf/compiler/java/java_enum_field.cc
  src/google/protobuf/compiler/java/java_extension.cc
  src/google/protobuf/compiler/java/java_field.cc
  src/google/protobuf/compiler/java/java_file.cc
  src/google/protobuf/compiler/java/java_generator.cc
  src/google/protobuf/compiler/java/java_generator_factory.cc
  src/google/protobuf/compiler/java/java_helpers.cc
  src/google/protobuf/compiler/java/java_lazy_message_field.cc
  src/google/protobuf/compiler/java/java_message.cc
  src/google/protobuf/compiler/java/java_message_field.cc
  src/google/protobuf/compiler/java/java_name_resolver.cc
  src/google/protobuf/compiler/java/java_primitive_field.cc
  src/google/protobuf/compiler/java/java_shared_code_generator.cc
  src/google/protobuf/compiler/java/java_service.cc
  src/google/protobuf/compiler/java/java_string_field.cc
  src/google/protobuf/compiler/java/java_doc_comment.cc
  src/google/protobuf/compiler/python/python_generator.cc
  
  # protoc
  src/google/protobuf/compiler/main.cc
  )

target_link_libraries(protoc protobuf)

install(
  TARGETS protobuf_lite protobuf protoc
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/src/google"
  DESTINATION include
  PATTERN "*.h"
  )
