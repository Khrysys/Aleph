include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_benchmark(bench_board Aleph::Chess)
aleph_add_benchmark(bench_perft Aleph::Chess)