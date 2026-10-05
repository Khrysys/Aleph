/**
 * @file include/aleph/chess/board/metadata.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <stdexcept>

#include "../board.hpp"
#include "../piece.hpp"
#include "../square.hpp"

namespace aleph::chess {

    inline auto Board::isBlackTurn() const -> bool { return (metadata & BLACK_TO_MOVE) != 0U; }

    inline auto Board::isWhiteTurn() const -> bool { return (metadata & BLACK_TO_MOVE) == 0U; }

    inline auto Board::canWhiteKingsideCastle() const -> bool {
        return (metadata & WHITE_KINGSIDE_CASTLE) != 0U;
    }

    inline auto Board::canWhiteQueensideCastle() const -> bool {
        return (metadata & WHITE_QUEENSIDE_CASTLE) != 0U;
    }

    inline auto Board::canBlackKingsideCastle() const -> bool {
        return (metadata & BLACK_KINGSIDE_CASTLE) != 0U;
    }

    inline auto Board::canBlackQueensideCastle() const -> bool {
        return (metadata & BLACK_QUEENSIDE_CASTLE) != 0U;
    }

    inline auto Board::isEnPassantValid() const -> bool {
        return (metadata & EN_PASSANT_VALID) != 0U;
    }

    inline auto Board::getEnPassantFile() const -> std::uint8_t {
        return (metadata & EN_PASSANT_FILE_MASK);
    }

    inline auto Board::getHalfMoveClock() const -> std::uint8_t {
        return (metadata & HALF_MOVE_CLOCK) >> 9;
    }

    inline auto Board::get(Square sq) const -> Piece {
        uint64_t const bit = 1ULL << static_cast<uint8_t>(sq);
        if ((getOccupancy() & bit) == 0) {
            return {NONE, false};
        }

        for (int i = 0; i < 6; i++) {
            if ((whiteBitboards[i] & bit) != 0) {
                return {PieceType(i), false};
            }
            if ((blackBitboards[i] & bit) != 0) {
                return {PieceType(i), true};
            }
        }
        // Unreachable: occupancy check guarantees a piece exists on this square.
        throw std::runtime_error("Occupancy check found a piece that didn't exist!");
    }

}  // namespace aleph::chess