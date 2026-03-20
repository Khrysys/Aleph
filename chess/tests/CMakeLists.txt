include(${Aleph_SOURCE_DIR}/cmake/test_macros.cmake)

aleph_add_test(test_move_list Aleph::Chess)
aleph_add_test(test_move Aleph::Chess)
aleph_add_test(test_piece Aleph::Chess)
aleph_add_test(test_square Aleph::Chess)

aleph_add_fuzz_target(fuzz_move Aleph::Chess)