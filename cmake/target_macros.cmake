if(NOT TARGET aleph_definitions)
    add_library(aleph_definitions INTERFACE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(aleph_definitions INTERFACE /Wall)
        if(Aleph_REPRODUCIBLE_BUILDS)
            if(MSVC_TOOLSET_VERSION LESS 120)
                message(WARNING "Reproducible builds were enabled, but the MSVC Toolset "
                "version is less than 120. This version does not have support for `/Brepro`. "
                "Upgrade or set `Aleph_REPRODUCIBLE_BUILDS=OFF`.")
            else()
                target_compile_options(aleph_definitions INTERFACE /Brepro)
                target_link_options(aleph_definitions INTERFACE /Brepro)
            endif()
        endif()
    else()
        # We assume any compiler not shown earlier is going to be a Clang/GCC compatible for flags
        target_compile_options(aleph_definitions INTERFACE -Wall -Wextra -Wpedantic)
        if(Aleph_REPRODUCIBLE_BUILDS)
            target_compile_options(aleph_definitions INTERFACE
                "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
                "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=."
            )
        endif()
    endif()
endif()

function(aleph_add_library)
    set(options INTERFACE STATIC SHARED)
    set(oneValueArgs NAME ALIAS)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 args "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(args_INTERFACE AND args_STATIC)
        message(FATAL_ERROR)
    endif()
    if(args_INTERFACE)
        add_library(${args_NAME} INTERFACE ${args_SOURCES})
        target_include_directories(${args_NAME} INTERFACE "include")
    else()
        if(args_STATIC)
            add_library(${args_NAME} STATIC ${args_SOURCES})
        elseif(args_SHARED)
            add_library(${args_NAME} SHARED ${args_SOURCES})
        else()
            message(FATAL_ERROR ${args})
        endif()
        target_link_libraries(${args_NAME} PRIVATE aleph_definitions)
        target_include_directories(${args_NAME} PUBLIC "include")
    endif()
    if(args_ALIAS)
        add_library(${args_ALIAS} ALIAS ${args_NAME})
    endif()
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
                benchmark::benchmark
        )
    endmacro()
else()
    macro(aleph_add_benchmark benchmark_name package)
    endmacro()
endif()