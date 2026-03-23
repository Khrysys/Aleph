#pragma once

#include <array>
#include <cstdint>

#include "piece.hpp"
#include "square.hpp"

namespace aleph::chess {
    namespace detail {
        struct AttackTables {
                std::array<std::array<uint64_t, 64>, 12> movement;
                std::array<std::array<uint64_t, 64>, 64> between;
        };
    }  // namespace detail
}  // namespace aleph::chess

#include <aleph/chess/generated_tables.inl>