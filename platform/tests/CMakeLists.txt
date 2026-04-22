include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_test(test_allocation_base Aleph::Platform)
aleph_add_test(test_allocation_fifo_queue Aleph::Platform)
aleph_add_test(test_allocation_string Aleph::Platform)