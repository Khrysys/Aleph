include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_test(test_allocation Aleph::Platform)

aleph_add_fuzz_target(fuzz_string_arena Aleph::Platform)

aleph_add_benchmark(bench_allocation Aleph::Platform)