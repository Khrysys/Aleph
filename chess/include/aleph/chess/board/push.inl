#pragma once

#include "../board.hpp"

namespace aleph::chess {
    Board Board::push(Move m) const {
        DEBUG_ASSERT(isLegal(m));

        Board next            = *this;
        next._cacheValid      = 0;

        Square from           = m.from();
        Square to             = m.to();

        uint8_t fromIdx       = static_cast<uint8_t>(from);
        uint8_t toIdx         = static_cast<uint8_t>(to);
        uint64_t fromBit      = 1ULL << fromIdx;
        uint64_t toBit        = 1ULL << toIdx;

        bool blackTurn        = isBlackTurn();

        auto& ownBitboards    = blackTurn ? next.blackBitboards : next.whiteBitboards;
        auto& enemyBitboards  = blackTurn ? next.whiteBitboards : next.blackBitboards;

        // --- Determine moving piece type ---
        PieceType movingPiece = get(from).type();
        DEBUG_ASSERT(movingPiece != NONE);

        // --- Clear en passant state ---
        next.metadata &= ~(EN_PASSANT_FILE_MASK | EN_PASSANT_VALID);

        // --- Remove captured piece (if any) ---
        for (int i = 0; i < 6; i++) enemyBitboards[i] &= ~toBit;

        // --- Move the piece ---
        ownBitboards[movingPiece] &= ~fromBit;

        // --- Handle promotion ---
        if (m.hasPromo())
            ownBitboards[m.promo()] |= toBit;
        else
            ownBitboards[movingPiece] |= toBit;

        // --- Handle castling (king moves two squares) ---
        if (movingPiece == KING) {
            int8_t fileDelta = static_cast<int8_t>(to.file()) - static_cast<int8_t>(from.file());
            if (fileDelta == 2) {
                // Kingside — move rook from h-file to f-file
                Square rookFrom(from.rank(), 7);
                Square rookTo(from.rank(), 5);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            } else if (fileDelta == -2) {
                // Queenside — move rook from a-file to d-file
                Square rookFrom(from.rank(), 0);
                Square rookTo(from.rank(), 3);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            }
        }

        // --- Handle en passant capture ---
        if (movingPiece == PAWN) {
            // Double push — set en passant file
            int8_t rankDelta = static_cast<int8_t>(to.rank()) - static_cast<int8_t>(from.rank());
            if (rankDelta == 2 || rankDelta == -2) {
                next.metadata |= EN_PASSANT_VALID;
                next.metadata |= (to.file() & EN_PASSANT_FILE_MASK);
            }

            // En passant capture — remove the captured pawn
            if (isEnPassantValid() && to.file() == getEnPassantFile() &&
                from.file() != getEnPassantFile()) {
                uint8_t capturedRank = blackTurn ? static_cast<uint8_t>(to.rank() + 1)
                                                 : static_cast<uint8_t>(to.rank() - 1);
                Square capturedSq(capturedRank, to.file());
                enemyBitboards[PAWN] &= ~(1ULL << static_cast<uint8_t>(capturedSq));
            }
        }

        // --- Update castling rights ---
        constexpr uint8_t E1 = static_cast<uint8_t>(Square(0, 4));
        constexpr uint8_t E8 = static_cast<uint8_t>(Square(7, 4));
        constexpr uint8_t A1 = static_cast<uint8_t>(Square(0, 0));
        constexpr uint8_t H1 = static_cast<uint8_t>(Square(0, 7));
        constexpr uint8_t A8 = static_cast<uint8_t>(Square(7, 0));
        constexpr uint8_t H8 = static_cast<uint8_t>(Square(7, 7));

        if (fromIdx == E1) next.metadata &= ~(WHITE_KINGSIDE_CASTLE | WHITE_QUEENSIDE_CASTLE);
        if (fromIdx == E8) next.metadata &= ~(BLACK_KINGSIDE_CASTLE | BLACK_QUEENSIDE_CASTLE);
        if (fromIdx == H1 || toIdx == H1) next.metadata &= ~WHITE_KINGSIDE_CASTLE;
        if (fromIdx == A1 || toIdx == A1) next.metadata &= ~WHITE_QUEENSIDE_CASTLE;
        if (fromIdx == H8 || toIdx == H8) next.metadata &= ~BLACK_KINGSIDE_CASTLE;
        if (fromIdx == A8 || toIdx == A8) next.metadata &= ~BLACK_QUEENSIDE_CASTLE;

        // --- Update halfmove clock ---
        bool isCapture = (blackTurn ? getWhiteOccupancy() : getBlackOccupancy()) & toBit;

        if (movingPiece == PAWN || isCapture) {
            next.metadata &= ~HALF_MOVE_CLOCK;
        } else {
            uint32_t clock = getHalfMoveClock() + 1;
            next.metadata  = (next.metadata & ~HALF_MOVE_CLOCK) | ((clock << 9) & HALF_MOVE_CLOCK);
        }

        // --- Flip side to move ---
        next.metadata ^= BLACK_TO_MOVE;

        return next;
    }
}  // namespace aleph::chess