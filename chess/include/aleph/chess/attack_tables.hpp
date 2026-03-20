#pragma once

#include <array>
#include <cstdint>

#include <aleph/platform.hpp>

#include "piece.hpp"
#include "square.hpp"

namespace aleph::chess {
    namespace detail {
        struct AttackTables {
                std::array<std::array<std::uint64_t, 64>, 12> movement;
                std::array<std::array<std::uint64_t, 64>, 64> between;
        };
    }  // namespace detail
}  // namespace aleph::chess

#include <aleph/chess/generated_tables.hpp>