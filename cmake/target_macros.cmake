macro(aleph_add_library lib_name lib_type alias_name option_type source_files)
    add_library(${lib_name} ${lib_type} ${source_files})
        if(Aleph_REPRODUCIBLE_BUILDS)
            if(MSVC)
                if(MSVC_TOOLSET_VERSION LESS 120)
                    message(WARNING "Reproducible builds were enabled, but the MSVC Toolset "
                    "version is less than 120. This version does not have support for `/Brepro`. "
                    "Upgrade or set `Aleph_REPRODUCIBLE_BUILDS=OFF`.")
                else()
                    target_compile_options(${lib_name} ${option_type} /Brepro /Z7)
                    target_link_options(${lib_name} ${option_type} /Brepro)
                endif()
            else()
                target_compile_options(${lib_name} ${option_type}
                    "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
                    "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=."
                )
            endif()
        endif()
    add_library(${alias_name} ALIAS ${lib_name})
endmacro()

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