# CMake script to generate source archive
# OUTPUT_DIR: where to place archive
# SOURCE_DIR: where it git source archive
# ARCHIVE_PREFIX: name of archive without git repo
# ARCHIVE_SUFFIX: optional suffix instead of -git${COMMIT}

if(NOT IS_DIRECTORY ${OUTPUT_DIR})
    message(ERROR "Output directory ${OUTPUT_DIR} does not exist")
endif()
find_program(GIT git)

if(NOT DEFINED ARCHIVE_SUFFIX)
    execute_process(COMMAND ${GIT} rev-list -1 --abbrev-commit HEAD
                    WORKING_DIRECTORY ${SOURCE_DIR}
                    OUTPUT_VARIABLE COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(ARCHIVE_SUFFIX "-git${COMMIT}")
endif(NOT DEFINED ARCHIVE_SUFFIX)
set(ARCHIVE_NAME "${ARCHIVE_PREFIX}${ARCHIVE_SUFFIX}")

message(STATUS "Making archive ${ARCHIVE_NAME}")
execute_process(
    COMMAND ${GIT} archive --prefix="${ARCHIVE_NAME}/" HEAD
    OUTPUT_FILE "${OUTPUT_DIR}/${ARCHIVE_NAME}.tar"
    WORKING_DIRECTORY ${SOURCE_DIR}
)
message(STATUS "Compressing archive ${ARCHIVE_NAME}.tar")
execute_process(
    COMMAND bzip2 -f ${ARCHIVE_NAME}.tar
    WORKING_DIRECTORY ${OUTPUT_DIR}
)
