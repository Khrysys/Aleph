cmake_minimum_required(VERSION 3.20)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE) # Can't believe this isn't a thing in raw CMake yet...
endif()

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# The project name and version must be a literal in the top-level project()
# call. It is read by Conan to populate the conan-side package name and version.
project(Aleph LANGUAGES CXX VERSION 0.0.1.1)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# We need knowledge of GTest and benchmark for the check of Aleph_BUILD_TESTS 
# and Aleph_BUILD_BENCHMARKS
find_package(GTest)
find_package(benchmark)

if(TARGET GTest::gtest_main)
    option(Aleph_BUILD_TESTS 
        "Build tests for the modules within the engine. Has no impact on the regular usage of the engine." 
        ON
    )
else()
    message(STATUS
        "Aleph: GTest::gtest_main target was not found. Tests are unavailable without it."
    )
    set(Aleph_BUILD_TESTS OFF)
endif()

if(DEFINED ENV{LIB_FUZZING_ENGINE})
    option(Aleph_BUILD_FUZZING
        "Build fuzz targets for the modules within the engine. This option adds fuzzing and may add sanitizers, which would impact performance. "
    )
else()
    message(STATUS 
        "Aleph: LIB_FUZZING_ENGINE environment variable not set. Fuzzing is unavailable without it."
    )
    set(Aleph_BUILD_FUZZING OFF)
endif()

if(TARGET benchmark::benchmark)
    option(Aleph_BUILD_BENCHMARKS "Build benchmarks for the modules within the engine. Has no impact on the regular usage of the engine." ON)
else()
    message(STATUS
        "Aleph: benchmark::benchmark target was not found. Benchmarks are unavailable without it."
    )
    set(Aleph_BUILD_BENCHMARKS OFF)
endif()

if(Aleph_BUILD_TESTS AND LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(Aleph_TEST_COVERAGE "Use gcov to output coverage data for the GTest test suite." ON)
else()
    message(STATUS
        "Aleph: For test coverage to be enabled, the compiler must be GCC on Linux, the build type must be Debug, and the Aleph test suite must be built."
    )
    set(Aleph_TEST_COVERAGE OFF)
endif()

option(Aleph_REPRODUCIBLE_BUILD "Attempt to build Aleph bit-reproducibly." ON)
option(Aleph_INSTALL "Add Aleph to the CMake INSTALL target." ON)
option(Aleph_PACKAGE "Build CPack packaging helpers for Aleph." ON)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

if(Aleph_BUILD_TESTS)
    enable_testing()
endif()

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

find_package(Python REQUIRED QUIET COMPONENTS Interpreter)

find_package(fmt REQUIRED QUIET)
find_package(half REQUIRED QUIET)
find_package(libassert REQUIRED QUIET)
find_package(protobuf REQUIRED QUIET)
find_package(quill REQUIRED QUIET)


if(LINUX)
    find_package(libnuma REQUIRED QUIET)
endif()

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

add_subdirectory(caching)
add_subdirectory(chess)
add_subdirectory(platform)

if(Aleph_INSTALL)
    message(STATUS "Aleph: Generating install target.")
    install(FILES
        "CODE_OF_CONDUCT.md"
        "CONTRIBUTING.md"
        "GOVERNANCE.md"
        "SECURITY.md"
        "LICENSE"
        "README.md"
        DESTINATION .
    )
endif()

if(Aleph_PACKAGE)
    message(STATUS "Aleph: Generating packaging target.")
    include(cmake/package.cmake)
endif()
if(Aleph_TEST_COVERAGE)
    message(STATUS "Aleph: Generating test coverage info.")
    include(cmake/coverage.cmake)
endif()