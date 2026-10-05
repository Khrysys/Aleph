include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_fuzz_target(fuzz_board_constructor Aleph::Chess)
aleph_add_fuzz_target(fuzz_board_push Aleph::Chess)