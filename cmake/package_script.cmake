find_program(TAR_EXECUTABLE tar REQUIRED)

set(PACKAGE_PATH
    "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
)

execute_process(
    COMMAND "${TAR_EXECUTABLE}" --version
    OUTPUT_VARIABLE TAR_VERSION_OUTPUT
    ERROR_VARIABLE TAR_VERSION_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(TAR_VERSION_OUTPUT MATCHES "GNU tar")
    set(TAR_IS_GNU TRUE)
    message(STATUS "Aleph: Using GNU Tar executable.")
    string(REGEX MATCH "GNU tar[^ \n]* ([0-9]+\\.[0-9]+\\.[0-9]+)"
        TAR_VERSION
        "${TAR_VERSION_OUTPUT}"
    )
    set(TAR_HAS_OWNER true)
    set(TAR_HAS_GROUP true)
    set(TAR_HAS_MTIME true)
elseif(TAR_VERSION_OUTPUT MATCHES "bsdtar")
    set(TAR_IS_BSD TRUE)
    message(STATUS "Aleph: Using bsdtar executable.")
    string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)"
        TAR_VERSION
        "${TAR_VERSION_OUTPUT}"
    )

    if(TAR_VERSION VERSION_GREATER_EQUAL "3.7.3")
        set(TAR_HAS_OWNER true)
        set(TAR_HAS_GROUP true)
    else()
        set(TAR_HAS_OWNER false)
        set(TAR_HAS_GROUP false)
        message(WARNING "${TAR_EXECUTABLE} does not support --owner. Bitwise reproducibility of the archive cannot be guaranteed.")
        message(WARNING "${TAR_EXECUTABLE} does not support --group. Bitwise reproducibility of the archive cannot be guaranteed.")
    endif()
    if(TAR_VERSION VERSION_GREATER_EQUAL "3.8.0")
        set(TAR_HAS_MTIME true)
    else()
        set(TAR_HAS_MTIME false)
        message(WARNING "${TAR_EXECUTABLE} does not support --mtime. Bitwise reproducibility of the archive cannot be guaranteed.")
    endif()

else()
    message(FATAL_ERROR
        "Unsupported tar implementation:\n"
        "${TAR_VERSION_OUTPUT}\n"
        "${TAR_VERSION_ERROR}"
    )
endif()
message(STATUS "Aleph: tar version: ${TAR_VERSION}")

set(TAR_OPTIONS --numeric-owner)
if(TAR_HAS_OWNER)
    list(PREPEND TAR_OPTIONS "--owner=0")
endif()
if(TAR_HAS_GROUP)
    list(PREPEND TAR_OPTIONS "--group=0")
endif()
if(TAR_HAS_MTIME)
    list(PREPEND TAR_OPTIONS "--mtime=@0")
endif()

if(TAR_IS_GNU)
    execute_process(
        COMMAND "${TAR_EXECUTABLE}"
            --sort=name
            --pax-option=exthdr.name=%d/PaxHeaders/%f,delete=atime,delete=ctime
            ${TAR_OPTIONS}
             -czf "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
             -C "${CPACK_TEMPORARY_DIRECTORY}"
            .
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
elseif(TAR_IS_BSD)
    # Pre sort file input list
    file(GLOB_RECURSE _file_input
        LIST_DIRECTORIES false
        RELATIVE "${CPACK_TEMPORARY_DIRECTORY}"
        "${CPACK_TEMPORARY_DIRECTORY}/*"
    )

    list(SORT _file_input)

    execute_process(
        COMMAND "${TAR_EXECUTABLE}"
            ${TAR_OPTIONS}
            -czf "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
            -C "${CPACK_TEMPORARY_DIRECTORY}"
            ${_file_input}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
endif()

if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Failed to create package:\n${output}\n${error}"
    )
endif()

set(CPACK_EXTERNAL_BUILT_PACKAGES "${PACKAGE_PATH}")
message(STATUS "Created deterministic package: ${PACKAGE_PATH}")