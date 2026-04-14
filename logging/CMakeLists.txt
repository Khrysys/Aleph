include("${Aleph_SOURCE_DIR}/cmake/target_macros.cmake")

aleph_add_library(aleph_logging INTERFACE SOURCES
    include/aleph/logging/backend.hpp
    include/aleph/logging/frontend.hpp
)

add_executable(test test.cpp)
target_link_libraries(test PRIVATE aleph_logging)