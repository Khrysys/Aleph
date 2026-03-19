cmake_minimum_required(VERSION 3.20)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# The project name and version must be a literal in the top-level project()
# call. It is read by Conan to populate the conan-side package name and version.
project(Aleph LANGUAGES CXX VERSION 0.0.1.0)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

option(Aleph_BUILD_TESTS 
    "Build tests for the modules within the engine. Has no impact on the regular usage of the \
    engine." 
    ON
)

if(DEFINED ENV{LIB_FUZZING_ENGINE})
    option(Aleph_BUILD_FUZZING
        "Build fuzz targets for the modules within the engine. This option adds fuzzing and \
        sanitizers, which may impact performance. "
    )
else()
    message(STATUS 
        "LIB_FUZZING_ENGINE environment variable not set. Fuzzing is unavailable without it."
    )
    set(Aleph_BUILD_FUZZING OFF)
endif()

# Platform architecture changes
option(Aleph_BUILD_NATIVE_ARCH "Build with native architecture flags" ON)
option(Aleph_ENABLE_AVX2  "Enable AVX2"  ON)
option(Aleph_ENABLE_BMI2  "Enable BMI2"  ON)
option(Aleph_ENABLE_POPCNT "Enable POPCNT" ON)

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

find_package(GTest)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

add_subdirectory(chess)
add_subdirectory(platform)
