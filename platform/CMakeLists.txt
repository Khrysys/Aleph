add_library(aleph_platform INTERFACE
    "include/aleph/platform/allocation/init.hpp"
    "include/aleph/platform/allocation/init.inl"
    "include/aleph/platform/allocation/runtime.hpp"
    "include/aleph/platform/allocation/runtime.inl"
    "include/aleph/platform/allocation.hpp"
    "include/aleph/platform/intrinsics.hpp"
    "include/aleph/platform/log_info.hpp"
    "include/aleph/platform.hpp"
)

# Applies a compiler-specific architecture flag if the given option is enabled.
# GCC/Clang use -m<flag> style; MSVC uses /arch: style.
# Note: BMI2 has no dedicated MSVC flag — it is implied by /arch:AVX2.
macro(aleph_arch_option enabled gccflag winflag)
    if(${enabled})
        target_compile_options(aleph_platform INTERFACE
            $<$<CXX_COMPILER_ID:GNU,Clang>:${gccflag}>
            $<$<CXX_COMPILER_ID:MSVC>:${winflag}>
        )
    endif()
endmacro()

target_include_directories(aleph_platform INTERFACE "include")
target_link_libraries(aleph_platform INTERFACE Boost::headers spdlog::spdlog)

if(Aleph_BUILD_NATIVE_ARCH)
    # -march=native enables all supported extensions on the build machine,
    # including BMI2, AVX2, and POPCNT where available.
    # MSVC has no true native equivalent; /arch:AVX2 is the closest approximation.
    # On future hardware with AVX-512 or later, the MSVC flag must be updated manually.
    target_compile_options(aleph_platform INTERFACE
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

add_library(Aleph::Platform ALIAS aleph_platform)

add_subdirectory(tests)
