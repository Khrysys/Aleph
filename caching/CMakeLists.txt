include("${CMAKE_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_library(NAME aleph_caching INTERFACE SOURCES
    "include/aleph/caching/common/types.hpp"
    "include/aleph/caching/mcts/entry.hpp"
    "include/aleph/caching/mcts/tree.hpp"
    "include/aleph/caching/policy/entry.hpp"
    "include/aleph/caching/policy/shard.hpp"
    ALIAS Aleph::Caching
)

target_link_libraries(aleph_caching INTERFACE
    half::half
    Aleph::Chess
    Aleph::Platform
)

add_subdirectory(tests)