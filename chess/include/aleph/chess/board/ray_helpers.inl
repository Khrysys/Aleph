/**
 * @file include/aleph/chess/board/ray_helpers.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <libassert/assert.h>

namespace aleph::chess {
    namespace detail {
        /**
         * Direction used for selecting a ray from `attackTables.rays.
         */
        enum Direction : uint8_t { N = 0, S = 1, E = 2, W = 3, NE = 4, NW = 5, SE = 6, SW = 7 };

        /**
         * Gets the ray from a square in a direction while respecting occupancy. This method only
         * works for directions that would count up in their index (`N`, `E`, `NE`, and `NW`). Other
         * directions should use `rayAttackBackward`.
         */
        [[nodiscard]] inline constexpr uint64_t rayAttackForward(uint64_t occ, Direction d,
                                                                 Square sq) {
            DEBUG_ASSERT(d == N || d == E || d == NE || d == NW);
            uint64_t ray      = attackTables.rays[d][sq];
            uint64_t blockers = ray & occ;

            uint8_t first     = static_cast<uint8_t>(platform::tzcnt(blockers | (1ULL << 63)));
            uint64_t cut      = attackTables.rays[d][first];

            return ray & ~cut;
        }

        /**
         * Gets the ray from a square in a direction while respecting occupancy. This method only
         * works for directions that would count up in their index (`S`, `W`, `SE`, and `SW`). Other
         * directions should use `rayAttackBackward`.
         */
        [[nodiscard]] inline constexpr uint64_t rayAttackBackward(uint64_t occ, Direction d,
                                                                  Square sq) {
            uint64_t ray      = attackTables.rays[d][sq];
            uint64_t blockers = ray & occ;

            uint8_t first     = static_cast<uint8_t>(63 - platform::lzcnt(blockers | 1ULL));
            uint64_t cut      = attackTables.rays[d][first];

            return ray & ~cut;
        }

        [[nodiscard]] inline constexpr uint64_t bishopAttacks(uint8_t sq, uint64_t occ) {
            return rayAttackForward(occ, NE, sq) | rayAttackForward(occ, NW, sq) |
                   rayAttackBackward(occ, SE, sq) | rayAttackBackward(occ, SW, sq);
        }

        [[nodiscard]] inline constexpr uint64_t rookAttacks(uint8_t sq, uint64_t occ) {
            return rayAttackForward(occ, N, sq) | rayAttackBackward(occ, S, sq) |
                   rayAttackForward(occ, E, sq) | rayAttackBackward(occ, W, sq);
        }
    }  // namespace detail
}  // namespace aleph::chess