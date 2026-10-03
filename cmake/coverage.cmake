find_program(LCOV_EXECUTABLE lcov REQUIRED)

get_property(ALEPH_TEST_TARGETS GLOBAL PROPERTY ALEPH_TEST_TARGETS)

add_custom_target(aleph_coverage_generation
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
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage data"
    VERBATIM
)

add_custom_target(aleph_coverage_chess
    DEPENDS aleph_coverage_generation
    COMMAND ${LCOV_EXECUTABLE}
        --extract ${CMAKE_BINARY_DIR}/coverage.info
            '${CMAKE_SOURCE_DIR}/chess/include/*'
            '${CMAKE_SOURCE_DIR}/chess/src/*'
        --output-file ${CMAKE_BINARY_DIR}/chess_coverage.info
        --rc branch_coverage=1
    COMMENT "Extracting chess library coverage"
    VERBATIM
)

add_custom_target(aleph_coverage_platform
    DEPENDS aleph_coverage_generation
    COMMAND ${LCOV_EXECUTABLE}
        --extract ${CMAKE_BINARY_DIR}/coverage.info
            '${CMAKE_SOURCE_DIR}/platform/include/*'
            '${CMAKE_SOURCE_DIR}/platform/src/*'
        --output-file ${CMAKE_BINARY_DIR}/platform_coverage.info
        --rc branch_coverage=1
    COMMENT "Extracting platform library coverage"
    VERBATIM
)

add_custom_target(aleph_coverage 
    DEPENDS aleph_coverage_chess aleph_coverage_platform
)