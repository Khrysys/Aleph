/**
 * @file include/aleph/chess/board/mutable.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <functional>
#include <numeric>

#include "../attack_tables.hpp"
#include "../board.hpp"

namespace aleph::chess {
    std::uint64_t Board::getOccupancy() const { return getWhiteOccupancy() | getBlackOccupancy(); }

    std::uint64_t Board::getWhiteOccupancy() const {
        return whiteBitboards[0] | whiteBitboards[1] | whiteBitboards[2] | whiteBitboards[3] |
               whiteBitboards[4] | whiteBitboards[5];
    }

    std::uint64_t Board::getBlackOccupancy() const {
        return blackBitboards[0] | blackBitboards[1] | blackBitboards[2] | blackBitboards[3] |
               blackBitboards[4] | blackBitboards[5];
    }

    std::uint64_t Board::getCheckers() const {
        if ((metadata & CACHED_CHECKERS_VALID) != 0) {
            bool blackTurn             = isBlackTurn();

            const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
            const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

            uint8_t kingSq             = static_cast<uint8_t>(platform::tzcnt(ownBitboards[KING]));
            uint64_t kingSqBit         = 1ULL << kingSq;
            uint64_t occ               = getOccupancy();

            _checkers                  = 0;

            // Pawn attack tables are asymmetric — index 0..5 are white pieces, 6..11 are black.
            // To find enemy pawns that attack the king, use the enemy color's attack table,
            // since a black pawn on sq attacks the squares given by movement[PAWN+6][sq].
            uint64_t pawns             = enemyBitboards[PAWN];
            while (pawns) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(pawns));
                pawns      &= pawns - 1;
                // Piece(PAWN, !blackTurn) exploits the Piece index encoding (white=0..5,
                // black=6..11) to select the correct pawn attack table: white pawns at index 0,
                // black at index 6.
                if (attackTables.movement[Piece(PAWN, !blackTurn)][sq] & kingSqBit)
                    _checkers |= 1ULL << sq;
            }

            // Knights
            uint64_t knights = enemyBitboards[KNIGHT];
            while (knights) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(knights));
                knights    &= knights - 1;
                if (attackTables.movement[KNIGHT][sq] & kingSqBit) _checkers |= 1ULL << sq;
            }

            // Diagonal sliders — bishops and queens
            uint64_t sliders = enemyBitboards[BISHOP] | enemyBitboards[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[BISHOP][sq] & kingSqBit)
                    if ((attackTables.between[sq][kingSq] & occ) == 0) _checkers |= 1ULL << sq;
            }

            // Orthogonal sliders — rooks and queens
            sliders = enemyBitboards[ROOK] | enemyBitboards[QUEEN];
            while (sliders) {
                uint8_t sq  = static_cast<uint8_t>(platform::tzcnt(sliders));
                sliders    &= sliders - 1;
                if (attackTables.movement[ROOK][sq] & kingSqBit)
                    if ((attackTables.between[sq][kingSq] & occ) == 0) _checkers |= 1ULL << sq;
            }
            metadata |= CACHED_CHECKERS_VALID;
        }
        return _checkers;
    }
}  // namespace aleph::chess