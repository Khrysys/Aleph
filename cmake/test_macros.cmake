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