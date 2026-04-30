find_package(PkgConfig)
pkg_check_modules(_JSONCPP QUIET jsoncpp)

find_path(JSONCPP_INCLUDE_DIR
    NAMES "json/json.h" "jsoncpp/json/json.h"
    PATHS ${_JSONCPP_INCLUDE_DIRS})

find_library(JSONCPP_LIBRARY
    NAMES jsoncpp jsoncpp_static jsoncpp_lib
    HINTS ${_JSONCPP_LIBRARY_DIRS})

mark_as_advanced(
    JSONCPP_INCLUDE_DIR
    JSONCPP_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(jsoncpp
    REQUIRED_VARS JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY
    VERSION_VAR _JSONCPP_VERSION)

if (jsoncpp_FOUND AND NOT TARGET jsoncpp_lib AND NOT TARGET jsoncpp_static)
    add_library(jsoncpp_lib UNKNOWN IMPORTED)
    set_target_properties(jsoncpp_lib PROPERTIES
        IMPORTED_LOCATION "${JSONCPP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_INCLUDE_DIR}")
endif()

if (jsoncpp_FOUND AND NOT TARGET JsonCpp::JsonCpp)
    add_library(JsonCpp::JsonCpp UNKNOWN IMPORTED)
    set_target_properties(JsonCpp::JsonCpp PROPERTIES
        IMPORTED_LOCATION "${JSONCPP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_INCLUDE_DIR}")
endif()
