# Locate Westeros simple-shell client protocol support.
#
# Result variables:
#   WesterosSimpleShell_FOUND
#   WesterosSimpleShell_INCLUDE_DIRS
#   WesterosSimpleShell_LIBRARIES
#
# Imported target:
#   WesterosSimpleShell::Client

find_path(WesterosSimpleShell_INCLUDE_DIR
    NAMES simpleshell-client-protocol.h
    HINTS
        ${CMAKE_SYSROOT}
        ${CMAKE_SYSROOT}/usr
        ${CMAKE_SYSROOT}/usr/local
        ${CMAKE_PREFIX_PATH}
        ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES
        include
        include/simpleshell/protocol
        include/westeros/simpleshell/protocol
        simpleshell/protocol
)

find_library(WesterosSimpleShell_CLIENT_LIBRARY
    NAMES westeros_simpleshell_client
    HINTS
        ${CMAKE_SYSROOT}
        ${CMAKE_SYSROOT}/usr
        ${CMAKE_SYSROOT}/usr/local
        ${CMAKE_PREFIX_PATH}
        ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES
        lib
        lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WesterosSimpleShell
    REQUIRED_VARS WesterosSimpleShell_INCLUDE_DIR WesterosSimpleShell_CLIENT_LIBRARY
)

if(WesterosSimpleShell_FOUND)
    set(WesterosSimpleShell_INCLUDE_DIRS ${WesterosSimpleShell_INCLUDE_DIR})
    set(WesterosSimpleShell_LIBRARIES ${WesterosSimpleShell_CLIENT_LIBRARY})

    if(NOT TARGET WesterosSimpleShell::Client)
        add_library(WesterosSimpleShell::Client UNKNOWN IMPORTED)
        set_target_properties(WesterosSimpleShell::Client PROPERTIES
            IMPORTED_LOCATION "${WesterosSimpleShell_CLIENT_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${WesterosSimpleShell_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    WesterosSimpleShell_INCLUDE_DIR
    WesterosSimpleShell_CLIENT_LIBRARY
)
