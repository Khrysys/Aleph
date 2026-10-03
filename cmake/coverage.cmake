find_program(LCOV_EXECUTABLE lcov REQUIRED)

get_property(ALEPH_TEST_TARGETS GLOBAL PROPERTY ALEPH_TEST_TARGETS)

add_custom_target(aleph_coverage
    DEPENDS ${ALEPH_TEST_TARGETS}
    COMMAND ${LCOV_EXECUTABLE}
        --zerocounters
        --directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-config ${CMAKE_BUILD_TYPE}
        --output-on-failure
    COMMAND ${LCOV_EXECUTABLE}
        --capture
        --directory ${CMAKE_BINARY_DIR}
        --output-file ${CMAKE_BINARY_DIR}/coverage.info 
        --ignore-errors mismatch
        --rc branch_coverage=1
    COMMAND ${LCOV_EXECUTABLE}
        --remove ${CMAKE_BINARY_DIR}/coverage.info
            '/usr/*'
            '*/.conan2/*'
            '*/tests/*'
            '*/benchmarks/*'
            '*/fuzzing/*'
        --output-file ${CMAKE_BINARY_DIR}/coverage.info
        --rc branch_coverage=1
        --ignore-errors unused
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage data"
    VERBATIM
)