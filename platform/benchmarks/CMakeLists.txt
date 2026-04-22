include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_benchmark(bench_allocation Aleph::Platform)
aleph_add_benchmark(bench_allocation_base Aleph::Platform)