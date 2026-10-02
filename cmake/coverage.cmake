find_program(LCOV_EXECUTABLE lcov REQUIRED)
find_program(GENHTML_EXECUTABLE genhtml)

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
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage data"
    VERBATIM
)

if(GENHTML_EXECUTABLE)
    add_custom_target(aleph_coverage_html
        COMMAND ${GENHTML_EXECUTABLE}
            ${CMAKE_BINARY_DIR}/coverage.info
            --output-directory ${CMAKE_BINARY_DIR}/coverage_html
            --branch-coverage
            --legend
        DEPENDS aleph_coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating HTML coverage report"
        VERBATIM
    )
endif()