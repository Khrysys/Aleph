cmake_minimum_required(VERSION 3.20)

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
        "Build tests for the modules within the engine. Has no impact on the regular usage of the \
        engine." 
        ON
    )
else()
    message(STATUS
        "GTest::gtest_main target was not found. Tests are unavailable without it."
    )
    set(Aleph_BUILD_TESTS OFF)
endif()

if(DEFINED ENV{LIB_FUZZING_ENGINE})
    option(Aleph_BUILD_FUZZING
        "Build fuzz targets for the modules within the engine. This option adds fuzzing and \
        may add sanitizers, which would impact performance. "
    )
else()
    message(STATUS 
        "LIB_FUZZING_ENGINE environment variable not set. Fuzzing is unavailable without it."
    )
    set(Aleph_BUILD_FUZZING OFF)
endif()

if(TARGET benchmark::benchmark)
    option(Aleph_BUILD_BENCHMARKS "Build benchmarks for the modules within the engine. Has no \
        impact on the regular usage of the engine." ON)
else()
    message(STATUS
        "benchmark::benchmark target was not found. Benchmarks are unavailable without it."
    )
    set(Aleph_BUILD_BENCHMARKS OFF)
endif()

# Platform architecture changes
option(Aleph_BUILD_NATIVE_ARCH "Build with native architecture flags" ON)
option(Aleph_ENABLE_AVX2  "Enable AVX2"  ON)
option(Aleph_ENABLE_BMI2  "Enable BMI2"  ON)
option(Aleph_ENABLE_POPCNT "Enable POPCNT" ON)
# Build changes
option(Aleph_REPRODUCIBLE_BUILDS "Enables compiler flags and disables some macros to make builds \
    reproducible. Requires MSVC Toolset 120 or higher." ON)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

if(Aleph_BUILD_TESTS)
    enable_testing()
endif()

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

find_package(Python REQUIRED COMPONENTS Interpreter)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(libassert REQUIRED)
find_package(spdlog REQUIRED)


# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

add_subdirectory(chess)
add_subdirectory(platform)
