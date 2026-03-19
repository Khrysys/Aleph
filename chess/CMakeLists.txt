set(_generated_header
    ${CMAKE_CURRENT_BINARY_DIR}/include/aleph/chess/generated_tables.hpp
)

add_custom_command(
    OUTPUT ${_generated_header}
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/table_builder.py ${_generated_header}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/table_builder.py
    COMMENT "Building Magic Attack Tables (Python)"
    VERBATIM
)

add_library(aleph_chess INTERFACE 
    "include/aleph/chess/move_list.hpp"
    "include/aleph/chess/move.hpp"
    "include/aleph/chess/piece.hpp"
    "include/aleph/chess/square.hpp"
    "include/aleph/chess.hpp"
)
unset(_generated_header)

target_include_directories(aleph_chess INTERFACE 
    "include" 
    "${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_link_libraries(aleph_chess INTERFACE 
    spdlog::spdlog
    fmt::fmt
    libassert::assert
)

add_library(Aleph::Chess ALIAS aleph_chess)

add_subdirectory(tests)