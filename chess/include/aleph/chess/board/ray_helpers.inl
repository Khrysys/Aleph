/**
 * @file include/aleph/chess/board/ray_helpers.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>

#include <libassert/assert.hpp>

#include <aleph/platform.hpp>

#include "../attack_tables.hpp"
#include "../square.hpp"

namespace aleph::chess::detail {
    /**
     * Direction used for selecting a ray from `attackTables.rays.
     */
    enum Direction : uint8_t { N = 0, S = 1, E = 2, W = 3, NE = 4, NW = 5, SE = 6, SW = 7 };

    /**
     * Gets the ray from a square in a direction while respecting occupancy. This method only
     * works for directions that would count up in their index (`N`, `E`, `NE`, and `NW`). Other
     * directions should use `rayAttackBackward`.
     */
    [[nodiscard]] constexpr auto rayAttackForward(uint64_t occ, Direction d, Square sq)
        -> std::uint64_t {
        DEBUG_ASSERT(d == N || d == E || d == NE || d == NW);
        std::uint64_t ray      = attackTables.rays[d][sq];
        std::uint64_t blockers = ray & occ;

        auto first             = static_cast<Square>(platform::tzcnt(blockers | (1ULL << 63)));
        std::uint64_t cut      = attackTables.rays[d][first];

        return ray & ~cut;
    }

    /**
     * Gets the ray from a square in a direction while respecting occupancy. This method only
     * works for directions that would count up in their index (`S`, `W`, `SE`, and `SW`). Other
     * directions should use `rayAttackForward`.
     */
    [[nodiscard]] constexpr auto rayAttackBackward(uint64_t occ, Direction d, Square sq)
        -> std::uint64_t {
        DEBUG_ASSERT(d == S || d == W || d == SE || d == SW);
        std::uint64_t ray      = attackTables.rays[d][sq];
        std::uint64_t blockers = ray & occ;

        auto first             = static_cast<Square>(63 - platform::lzcnt(blockers | 1ULL));
        std::uint64_t cut      = attackTables.rays[d][first];

        return ray & ~cut;
    }

    /**
     * Returns all bishop attacks from a given square with respect to the occupancy of the
     * position.
     */
    [[nodiscard]] constexpr auto bishopAttacks(Square sq, uint64_t occ) -> std::uint64_t {
        return rayAttackForward(occ, NE, sq) | rayAttackForward(occ, NW, sq) |
               rayAttackBackward(occ, SE, sq) | rayAttackBackward(occ, SW, sq);
    }

    /** Returns all rook attacks from a given square with respect to the occupancy of the
     * position.
     */
    [[nodiscard]] constexpr auto rookAttacks(Square sq, uint64_t occ) -> std::uint64_t {
        return rayAttackForward(occ, N, sq) | rayAttackBackward(occ, S, sq) |
               rayAttackForward(occ, E, sq) | rayAttackBackward(occ, W, sq);
    }
}  // namespace aleph::chess::detail