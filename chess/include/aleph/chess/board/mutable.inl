/**
 * @file include/aleph/chess/board/mutable.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <functional>
#include <numeric>

#include "../attack_tables.hpp"
#include "../board.hpp"
#include "../piece.hpp"
#include "../square.hpp"

namespace aleph::chess {
    auto Board::getOccupancy() const noexcept -> std::uint64_t {
        return getWhiteOccupancy() | getBlackOccupancy();
    }

    auto Board::getWhiteOccupancy() const noexcept -> std::uint64_t {
        return whiteBitboards[0] | whiteBitboards[1] | whiteBitboards[2] | whiteBitboards[3] |
               whiteBitboards[4] | whiteBitboards[5];
    }

    auto Board::getBlackOccupancy() const noexcept -> std::uint64_t {
        return blackBitboards[0] | blackBitboards[1] | blackBitboards[2] | blackBitboards[3] |
               blackBitboards[4] | blackBitboards[5];
    }

    auto Board::getCheckers() const -> std::uint64_t {
        if ((metadata & CACHED_CHECKERS_VALID) != 0) {
            bool blackTurn             = isBlackTurn();

            const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
            const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

            auto kingSq     = static_cast<std::int8_t>(platform::tzcnt(ownBitboards[KING]));
            std::uint64_t kingSqBit = 1ULL << kingSq;
            std::uint64_t occ       = getOccupancy();

            _checkers               = 0;

            // Pawn attack tables are asymmetric — index 0..5 are white pieces, 6..11 are black.
            // To find enemy pawns that attack the king, use the enemy color's attack table,
            // since a black pawn on sq attacks the squares given by movement[PAWN+6][sq].
            _checkers |=
                (attackTables.movement[Piece(PAWN, blackTurn)][kingSq] & enemyBitboards[PAWN]);
            // Knights
            _checkers |= (attackTables.movement[Piece(KNIGHT, blackTurn)][kingSq] & enemyBitboards[KNIGHT]);

            // Diagonal sliders — bishops and queens
            std::uint64_t sliders = enemyBitboards[BISHOP] | enemyBitboards[QUEEN];
            while (sliders != 0) {
                auto sq  = static_cast<std::uint8_t>(platform::tzcnt(sliders));
                sliders &= sliders - 1;
                if ((attackTables.movement[BISHOP][sq] & kingSqBit &
                     attackTables.between[sq][kingSq] & occ) == 0) {
                    _checkers |= 1ULL << sq;
                }
            }

            // Orthogonal sliders — rooks and queens
            sliders = enemyBitboards[ROOK] | enemyBitboards[QUEEN];
            while (sliders != 0) {
                auto sq  = static_cast<std::uint8_t>(platform::tzcnt(sliders));
                sliders &= sliders - 1;
                if ((attackTables.movement[ROOK][sq] & kingSqBit &
                     attackTables.between[sq][kingSq] & occ) == 0) {
                    _checkers |= 1ULL << sq;
                }
            }
            metadata |= CACHED_CHECKERS_VALID;
        }
        return _checkers;
    }
}  // namespace aleph::chess