find_program(TAR_EXECUTABLE tar REQUIRED)

set(PACKAGE_PATH
    "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
)

message(STATUS "CPACK_TEMPORARY_DIRECTORY=${CPACK_TEMPORARY_DIRECTORY}")
message(STATUS "PACKAGE_PATH=${PACKAGE_PATH}")

execute_process(
    COMMAND "${TAR_EXECUTABLE}" --version
    OUTPUT_VARIABLE TAR_VERSION
    ERROR_VARIABLE TAR_VERSION_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(TAR_VERSION MATCHES "GNU tar")
    set(TAR_IS_GNU TRUE)
    message(STATUS "Aleph: Using GNU Tar executable.")
elseif(TAR_VERSION MATCHES "bsdtar")
    set(TAR_IS_BSD TRUE)
    message(STATUS "Aleph: Using bsdtar executable.")
else()
    message(FATAL_ERROR
        "Unsupported tar implementation:\n"
        "${TAR_VERSION}\n"
        "${TAR_VERSION_ERROR}"
    )
endif()

if(TAR_IS_GNU)
    execute_process(
        COMMAND "${TAR_EXECUTABLE}"
            --sort=name
            --mtime=@0
            --owner=0
            --group=0
            --numeric-owner
            --pax-option=exthdr.name=%d/PaxHeaders/%f,delete=atime,delete=ctime
            -czf "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
            -C "${CPACK_TEMPORARY_DIRECTORY}" 
            .
        RESULT_VARIABLE result
        ERROR_VARIABLE error
    )
elseif(TAR_IS_BSD)
    # Pre sort file input list
    file(GLOB_RECURSE _raw_files
        RELATIVE "${CPACK_TEMPORARY_DIRECTORY}"
        "${CPACK_TEMPORARY_DIRECTORY}/*"
    )

    list(SORT _raw_files)

    set(_file_input "")
    foreach(entry IN LISTS _raw_files)
        list(APPEND _file_input "${entry}")
    endforeach()

    execute_process(
        COMMAND "${TAR_EXECUTABLE}"
            --mtime=@0
            --owner=0
            --group=0
            --numeric-owner
            -czf "${CPACK_TEMPORARY_DIRECTORY}.tar.gz"
            -C "${CPACK_TEMPORARY_DIRECTORY}" 
            ${_file_input}
        RESULT_VARIABLE result
        ERROR_VARIABLE error
    )
    unset(_raw_files)
endif()

if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Failed to create package:\n${error}"
    )
endif()

set(CPACK_EXTERNAL_BUILT_PACKAGES "${PACKAGE_PATH}")
message(STATUS "Created deterministic package: ${PACKAGE_PATH}")