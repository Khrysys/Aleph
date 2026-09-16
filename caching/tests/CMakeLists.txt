include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

add_executable(test_caching_compilation test_caching_compilation.cpp)
target_link_libraries(test_caching_compilation PRIVATE Aleph::Caching)