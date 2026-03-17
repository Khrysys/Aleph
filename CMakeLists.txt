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

option(Aleph_DISABLE_EXECUTOR 
    "Fully disable the executor build. MCTS hooks can be specified programmatically."
    OFF
)
option(Aleph_DEBUG 
    "Force Debug Mode on, even in Release. Will noticeably slow down the engine."
    ON
)
option(Aleph_BUILD_TESTS 
    "Build tests for the modules within the engine. Has no impact on the regular usage of the \
    engine." 
    ON
)

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

find_package(absl REQUIRED)
find_package(boost REQUIRED)

find_package(GTest)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

add_subdirectory(platform)
