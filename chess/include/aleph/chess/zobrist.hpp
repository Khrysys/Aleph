/**
 * @file include/aleph/chess/zobrist.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstdint>

namespace aleph::chess {
    namespace detail {
        struct Zobrist {
                std::array<std::array<uint64_t, 64>, 12> pieces;

                std::array<uint64_t, 16> castling;
                std::array<uint64_t, 8> enPassant;
                std::uint64_t sideToMove;
        };

        [[nodiscard]] inline constexpr uint64_t splitMix64(uint64_t& key) {
            std::uint64_t z = (key += 0x9e3779b97f4a7c15);
            z               = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z               = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            return z ^ (z >> 31);
        }

        [[nodiscard]] inline consteval Zobrist createZobrist() {
            std::uint64_t key = 0x123456790ABCDEFULL;
            Zobrist zobrist{};
            for (auto i = 0; i < zobrist.pieces.size(); i++) {
                for (auto j = 0; j < zobrist.pieces[0].size(); j++) {
                    zobrist.pieces[i][j] = splitMix64(key);
                }
            }

            for (auto& c : zobrist.castling) {
                c = splitMix64(key);
            }

            for (auto& ep : zobrist.enPassant) {
                ep = splitMix64(key);
            }

            zobrist.sideToMove = splitMix64(key);

            return zobrist;
        }
    }  // namespace detail

    constinit detail::Zobrist zobrist = detail::createZobrist();
}  // namespace aleph::chess