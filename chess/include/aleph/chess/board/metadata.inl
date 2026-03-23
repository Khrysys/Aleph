/**
 * @file include/aleph/chess/board/metadata.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "../board.hpp"

namespace aleph::chess {

    bool Board::isBlackTurn() const { return (metadata & BLACK_TO_MOVE) != 0U; }

    bool Board::isWhiteTurn() const { return (metadata & BLACK_TO_MOVE) == 0U; }

    bool Board::canWhiteKingsideCastle() const { return (metadata & WHITE_KINGSIDE_CASTLE) != 0U; }

    bool Board::canWhiteQueensideCastle() const {
        return (metadata & WHITE_QUEENSIDE_CASTLE) != 0U;
    }

    bool Board::canBlackKingsideCastle() const { return (metadata & BLACK_KINGSIDE_CASTLE) != 0U; }

    bool Board::canBlackQueensideCastle() const {
        return (metadata & BLACK_QUEENSIDE_CASTLE) != 0U;
    }

    bool Board::isEnPassantValid() const { return (metadata & EN_PASSANT_VALID) != 0U; }

    std::uint8_t Board::getEnPassantFile() const { return (metadata & EN_PASSANT_FILE_MASK); }

    std::uint8_t Board::getHalfMoveClock() const { return (metadata & HALF_MOVE_CLOCK) >> 9; }

    Piece Board::get(Square sq) const {
        uint64_t bit = 1ULL << static_cast<uint8_t>(sq);
        if (!(getOccupancy() & bit)) return Piece(NONE, false);
        for (int i = 0; i < 6; i++) {
            if (whiteBitboards[i] & bit) return Piece(PieceType(i), false);
            if (blackBitboards[i] & bit) return Piece(PieceType(i), true);
        }
        // Unreachable: occupancy check guarantees a piece exists on this square.
        return Piece(NONE, false);
    }

}  // namespace aleph::chess