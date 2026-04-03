/**
 * @file include/aleph/chess/zobrist.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstdint>

#include <aleph/platform.hpp>

namespace aleph::chess {
    namespace detail {
        struct Zobrist {
                std::array<std::array<std::uint64_t, 64>, 12> pieces;

                std::array<std::uint64_t, 16> castling;
                std::array<std::uint64_t, 8> enPassant;
                std::uint64_t sideToMove;
        };

        inline constexpr std::uint64_t ZOBRIST_STARTING_KEY = 0x123456790ABCDEFULL;

        [[nodiscard]] inline consteval Zobrist createZobrist() {
            std::uint64_t key = ZOBRIST_STARTING_KEY;
            Zobrist zobrist{};

            for (auto i = 0; i < zobrist.pieces.size(); i++) {
                for (auto j = 0; j < zobrist.pieces[0].size(); j++) {
                    zobrist.pieces[i][j] = platform::splitMix64(key);
                }
            }

            for (auto& c : zobrist.castling) {
                c = platform::splitMix64(key);
            }

            for (auto& ep : zobrist.enPassant) {
                ep = platform::splitMix64(key);
            }

            zobrist.sideToMove = platform::splitMix64(key);

            return zobrist;
        }
    }  // namespace detail

    inline constexpr detail::Zobrist zobrist = detail::createZobrist();
}  // namespace aleph::chess