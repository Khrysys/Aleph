include("${CMAKE_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_library(Platform STATIC SOURCES 
    "include/aleph/platform/allocation/base.hpp"
    "include/aleph/platform/allocation/string.hpp"
    "include/aleph/platform/allocation.hpp"
    "include/aleph/platform/compiler.hpp"
    "include/aleph/platform/hash.hpp"
    "include/aleph/platform/intrinsics.hpp"
    "include/aleph/platform/threads.hpp"
    "include/aleph/platform.hpp"
    "src/allocation/base.cpp"
    "src/allocation/string.cpp"
    "src/intrinsics.cpp"
    "src/os_dependent.hpp"
    "src/threads.cpp"
)

# Applies a compiler-specific architecture flag if the given option is enabled.
# GCC/Clang use -m<flag> style; MSVC uses /arch: style.
# Note: BMI2 has no dedicated MSVC flag — it is implied by /arch:AVX2.
macro(aleph_arch_option enabled gccflag winflag)
    if(${enabled})
        target_compile_options(aleph_platform PUBLIC
            $<$<CXX_COMPILER_ID:GNU,Clang>:${gccflag}>
            $<$<CXX_COMPILER_ID:MSVC>:${winflag}>
        )
    endif()
endmacro()

target_link_libraries(aleph_platform PUBLIC Boost::headers fmt::fmt libassert::assert)

if(libnuma_FOUND)
    target_link_libraries(aleph_platform PUBLIC libnuma::libnuma)
endif()

if(Aleph_BUILD_NATIVE_ARCH)
    # -march=native enables all supported extensions on the build machine,
    # including BMI2, AVX2, and POPCNT where available.
    # MSVC has no true native equivalent; /arch:AVX2 is the closest approximation.
    # On future hardware with AVX-512 or later, the MSVC flag must be updated manually.
    target_compile_options(aleph_platform PUBLIC
        $<$<CXX_COMPILER_ID:GNU,Clang>:-march=native>
        $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
    )
else()
    # Individual feature flags for cross-compilation or controlled builds.
    # BMI2 flag is empty for MSVC — implied by /arch:AVX2 above if enabled.
    aleph_arch_option(Aleph_ENABLE_AVX2   -mavx2    /arch:AVX2)
    aleph_arch_option(Aleph_ENABLE_BMI2   -mbmi2    "")
    aleph_arch_option(Aleph_ENABLE_POPCNT -mpopcnt  "")
endif()
add_subdirectory(benchmarks)
add_subdirectory(fuzzing)
add_subdirectory(tests)