if(NOT TARGET aleph_definitions)
    add_library(aleph_definitions INTERFACE)
    
    # ---------------------------------
    # | Always-on compiler options
    # ---------------------------------
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(aleph_definitions INTERFACE /W4)
    else()
        target_compile_options(aleph_definitions INTERFACE -Wall -Wextra -Wpedantic)
    endif()

    message(STATUS "Aleph: Attempting to make a reproducible build...")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_TOOLSET_VERSION LESS 120)
        message(WARNING "Reproducible builds were enabled, but the MSVC Toolset "
        "version is less than 120. This version does not have support for `/Brepro`. "
        "Upgrade or set `Aleph_REPRODUCIBLE_BUILDS=OFF`.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(aleph_definitions INTERFACE /Brepro)
        target_link_options(aleph_definitions INTERFACE /Brepro)
        message(STATUS "Aleph: Reproducible build enabled.")
    else()
        target_compile_options(aleph_definitions INTERFACE
            "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
            "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=."
        )
    endif()

endif()

function(aleph_add_library library_name)
    string(TOLOWER ${library_name} target_name)
    set(target_name aleph_${target_name})
    set(alias_name Aleph::${library_name})

    set(options INTERFACE STATIC SHARED)
    set(oneValueArgs "")
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(args_INTERFACE AND args_STATIC)
        message(FATAL_ERROR "Library cannot be both INTERFACE and STATIC")
    endif()
    if(args_INTERFACE AND args_SHARED)
        message(FATAL_ERROR "Library cannot be both INTERFACE and SHARED")
    endif()
    if(args_STATIC AND args_SHARED)
        message(FATAL_ERROR "Library cannot be both STATIC and SHARED")
    endif()

    if(NOT args_INTERFACE AND NOT args_STATIC AND NOT args_SHARED)
        message(FATAL_ERROR "Library type not specified for ${target_name}. Must be INTERFACE, STATIC, or SHARED.")
    endif()

    if(args_INTERFACE)
        add_library(${target_name} INTERFACE ${args_SOURCES})
        target_include_directories(${target_name} INTERFACE "include")
        message(STATUS "Aleph: Interface module library declared '${target_name}'")
    else()
        if(args_STATIC)
            add_library(${target_name} STATIC ${args_SOURCES})
            message(STATUS "Aleph: Static module library declared '${target_name}'")
        elseif(args_SHARED)
            add_library(${target_name} SHARED ${args_SOURCES})
            message(STATUS "Aleph: Shared module library declared '${target_name}'")
        endif()
        target_link_libraries(${target_name} PRIVATE aleph_definitions)
        target_include_directories(${target_name} PUBLIC "include")
        if(Aleph_INSTALL)
            install(TARGETS ${target_name})
        endif()
    endif()
    add_library(${alias_name} ALIAS ${target_name})
    message(STATUS "Aleph: Module target declared '${alias_name}'")
endfunction()

if(Aleph_BUILD_TESTS)
    if(NOT COMMAND gtest_discover_tests)
        include(GoogleTest)
    endif()

    macro(aleph_add_test test_name package)
        add_executable(${test_name} ${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE ${package} GTest::gtest_main)
        gtest_discover_tests(${test_name})
    endmacro()
else()
    macro(aleph_add_test test_name package)
    endmacro()
endif()

if(Aleph_BUILD_FUZZING)
    macro(aleph_add_fuzz_target fuzz_name package)
        add_executable(${fuzz_name} ${fuzz_name}.cpp)
        target_link_libraries(${fuzz_name}
            PRIVATE
                ${package}
                $ENV{LIB_FUZZING_ENGINE}
        )
    endmacro()
else()
    macro(aleph_add_fuzz_target fuzz_name package)
    endmacro()
endif()

if(Aleph_BUILD_BENCHMARKS)
    macro(aleph_add_benchmark benchmark_name package)
        add_executable(${benchmark_name} ${benchmark_name}.cpp)
        target_link_libraries(${benchmark_name}
            PRIVATE
                ${package}
                benchmark::benchmark_main
        )
        if(Aleph_INSTALL)
            install(TARGETS ${benchmark_name})
        endif()
    endmacro()
else()
    macro(aleph_add_benchmark benchmark_name package)
    endmacro()
endif()