/**
 * @file include/aleph/chess/board/push.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include "../board.hpp"

namespace aleph::chess {
    namespace detail {
        inline uint8_t getCastleIndex(uint32_t meta) {
            uint8_t idx = 0;
            if (meta & WHITE_KINGSIDE_CASTLE) idx |= 1;
            if (meta & WHITE_QUEENSIDE_CASTLE) idx |= 2;
            if (meta & BLACK_KINGSIDE_CASTLE) idx |= 4;
            if (meta & BLACK_QUEENSIDE_CASTLE) idx |= 8;
            return idx;
        }
    }  // namespace detail

    Board Board::push(Move m) const {
        DEBUG_ASSERT(isLegal(m));

        Board next            = *this;

        Square from           = m.from();
        Square to             = m.to();

        uint8_t fromIdx       = static_cast<uint8_t>(from);
        uint8_t toIdx         = static_cast<uint8_t>(to);
        uint64_t fromBit      = 1ULL << fromIdx;
        uint64_t toBit        = 1ULL << toIdx;

        bool blackTurn        = isBlackTurn();

        auto& ownBitboards    = blackTurn ? next.blackBitboards : next.whiteBitboards;
        auto& enemyBitboards  = blackTurn ? next.whiteBitboards : next.blackBitboards;

        PieceType movingPiece = get(from).type();
        DEBUG_ASSERT(movingPiece != NONE);

        // Clear en passant state unconditionally — a new en passant square will be set
        // below if this move is a double pawn push.
        next.metadata &= ~(EN_PASSANT_FILE_MASK | EN_PASSANT_VALID);

        // Remove any enemy piece on the destination square.
        for (int i = 0; i < 6; i++) enemyBitboards[i] &= ~toBit;

        // Vacate the origin square and place the piece on the destination.
        ownBitboards[movingPiece] &= ~fromBit;
        if (m.hasPromo())
            // Replace the pawn with the promoted piece type.
            ownBitboards[m.promo()] |= toBit;
        else
            ownBitboards[movingPiece] |= toBit;

        // --- Castling ---
        // Detected contextually: a king moving exactly two files triggers rook relocation.
        // Castling rights in metadata are trusted as authoritative.
        if (movingPiece == KING) {
            int8_t fileDelta = static_cast<int8_t>(to.file()) - static_cast<int8_t>(from.file());
            if (fileDelta == 2) {
                // Kingside — rook moves from h-file to f-file.
                Square rookFrom(from.rank(), 7);
                Square rookTo(from.rank(), 5);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            } else if (fileDelta == -2) {
                // Queenside — rook moves from a-file to d-file.
                Square rookFrom(from.rank(), 0);
                Square rookTo(from.rank(), 3);
                ownBitboards[ROOK] &= ~(1ULL << static_cast<uint8_t>(rookFrom));
                ownBitboards[ROOK] |= (1ULL << static_cast<uint8_t>(rookTo));
            }
        }

        bool isEP = movingPiece == PAWN && isEnPassantValid() && to.file() == getEnPassantFile() &&
                    from.file() != to.file() && !(getOccupancy() & toBit);

        // --- En passant ---
        // Detected contextually: a pawn moving to a different file onto the en passant
        // square triggers removal of the captured pawn from the rank behind the target.
        if (movingPiece == PAWN) {
            int8_t rankDelta = static_cast<int8_t>(to.rank()) - static_cast<int8_t>(from.rank());
            if (rankDelta == 2 || rankDelta == -2) {
                // Double push — record the en passant file for the opponent.
                next.metadata |= EN_PASSANT_VALID;
                next.metadata |= (to.file() & EN_PASSANT_FILE_MASK);
            }

            if (isEP) {
                // En passant capture — remove the captured pawn from the rank it sits on,
                // which is one rank behind the destination from the moving side's perspective.
                uint8_t capturedRank = blackTurn ? static_cast<uint8_t>(to.rank() + 1)
                                                 : static_cast<uint8_t>(to.rank() - 1);
                Square capturedSq(capturedRank, to.file());
                enemyBitboards[PAWN] &= ~(1ULL << static_cast<uint8_t>(capturedSq));

                next._zobristHash    ^= zobrist.pieces[Piece(PAWN, !blackTurn)][capturedSq];
            }
        }

        // --- Castling rights ---
        // Any move from or to a rook's starting square clears the corresponding right,
        // handling both rook moves and captures on those squares in one pass.
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

        // --- Halfmove clock ---
        // Capture detection reads from the original board's occupancy before any pieces
        // were removed, ensuring a correct result even for en passant captures.
        bool isCapture =
            isEP | (((blackTurn ? getWhiteOccupancy() : getBlackOccupancy()) & toBit) != 0);

        if (movingPiece == PAWN || isCapture) {
            next.metadata &= ~HALF_MOVE_CLOCK;
        } else {
            uint32_t clock = getHalfMoveClock() + 1;
            next.metadata  = (next.metadata & ~HALF_MOVE_CLOCK) | ((clock << 9) & HALF_MOVE_CLOCK);
        }

        if (m.hasPromo()) {
            next._zobristHash ^= zobrist.pieces[Piece(m.promo(), blackTurn)][toIdx];
        } else {
            next._zobristHash ^= zobrist.pieces[Piece(movingPiece, blackTurn)][toIdx];
        }

        if (metadata & EN_PASSANT_VALID) {
            uint8_t file       = metadata & EN_PASSANT_FILE_MASK;
            next._zobristHash ^= zobrist.enPassant[file];
        }

        if (metadata & EN_PASSANT_VALID) {
            uint8_t file       = metadata & EN_PASSANT_FILE_MASK;
            next._zobristHash ^= zobrist.enPassant[file];
        }

        uint8_t oldCastle  = detail::getCastleIndex(metadata);
        next._zobristHash ^= zobrist.castling[oldCastle];

        uint8_t newCastle  = detail::getCastleIndex(next.metadata);
        next._zobristHash ^= zobrist.castling[newCastle];

        next.metadata     ^= BLACK_TO_MOVE;
        // Clear Cache
        next.metadata     &= ~CACHED_CHECKERS_VALID;

        // Zobrist Hash
        next._zobristHash ^= zobrist.sideToMove;
        next._zobristHash ^= zobrist.pieces[Piece(movingPiece, blackTurn)][fromIdx];

        return next;
    }
}  // namespace aleph::chess