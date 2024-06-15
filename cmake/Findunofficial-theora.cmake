find_package(PkgConfig)

find_package(Ogg REQUIRED)

pkg_check_modules(_THEORA QUIET theora)
pkg_check_modules(_THEORAENC QUIET theoraenc)
pkg_check_modules(_THEORADEC QUIET theoradec)

find_path(THEORA_INCLUDE_DIR
    NAMES "theora/theora.h"
    PATHS ${_THEORA_INCLUDE_DIRS})

find_path(THEORAENC_INCLUDE_DIR
    NAMES "theora/theoraenc.h"
    PATHS ${_THEORAENC_INCLUDE_DIRS})

find_path(THEORADEC_INCLUDE_DIR
    NAMES "theora/theoradec.h"
    PATHS ${_THEORADEC_INCLUDE_DIRS})

find_library(THEORA_LIBRARY
    NAMES theora libtheora
    HINTS ${_THEORA_LIBRARY_DIRS})

find_library(THEORAENC_LIBRARY
    NAMES theoraenc libtheoraenc
    HINTS ${_THEORAENC_LIBRARY_DIRS})

find_library(THEORADEC_LIBRARY
    NAMES theoradec libtheoradec
    HINTS ${_THEORADEC_LIBRARY_DIRS})

mark_as_advanced(
    THEORA_INCLUDE_DIR
    THEORAENC_INCLUDE_DIR
    THEORADEC_INCLUDE_DIR
    THEORA_LIBRARY
    THEORAENC_LIBRARY
    THEORADEC_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(unofficial-theora
    REQUIRED_VARS
        THEORA_INCLUDE_DIR THEORAENC_INCLUDE_DIR THEORADEC_INCLUDE_DIR
        THEORA_LIBRARY THEORAENC_LIBRARY THEORADEC_LIBRARY
    VERSION_VAR _THEORA_VERSION)

if(UNOFFICIAL-THEORA_FOUND AND NOT TARGET unofficial::theora::theora)
    add_library(unofficial::theora::theora UNKNOWN IMPORTED)
    set_target_properties(unofficial::theora::theora PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THEORA_INCLUDE_DIR}"
        IMPORTED_LOCATION "${THEORA_LIBRARY}")
endif()

if(UNOFFICIAL-THEORA_FOUND AND NOT TARGET unofficial::theora::theoraenc)
    add_library(unofficial::theora::theoraenc UNKNOWN IMPORTED)
    set_target_properties(unofficial::theora::theoraenc PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THEORAENC_INCLUDE_DIR}"
        IMPORTED_LOCATION "${THEORAENC_LIBRARY}")
endif()

if(UNOFFICIAL-THEORA_FOUND AND NOT TARGET unofficial::theora::theoradec)
    add_library(unofficial::theora::theoradec UNKNOWN IMPORTED)
    set_target_properties(unofficial::theora::theoradec PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THEORADEC_INCLUDE_DIR}"
        IMPORTED_LOCATION "${THEORADEC_LIBRARY}")
endif()