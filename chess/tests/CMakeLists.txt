include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_test(test_board Aleph::Chess)
aleph_add_test(test_move_list Aleph::Chess)
aleph_add_test(test_move Aleph::Chess)
aleph_add_test(test_piece Aleph::Chess)
aleph_add_test(test_square Aleph::Chess)

