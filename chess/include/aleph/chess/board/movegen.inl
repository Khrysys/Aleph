/**
 * @file include/aleph/chess/board/movegen.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <aleph/platform.hpp>

#include "../attack_tables.hpp"
#include "../board.hpp"
#include "ray_helpers.inl"

namespace aleph::chess {
    namespace detail {
        /**
         * Returns true if the square at `sqIdx` is attacked by any piece in `attackers`.
         *
         * Used by `isLegalFast` to validate king safety after a simulated move. Takes
         * a pre-constructed attacker bitboard array rather than reading from the board
         * directly, allowing callers to pass post-move enemy bitboards with captured
         * pieces already removed. The `occ` parameter reflects the post-move occupancy
         * and is used to validate slider paths via the between table.
         *
         * `attackersAreBlack` selects the correct pawn attack table — pawn attacks are
         * asymmetric, so the color of the attacking pawns must be known. Pass `!blackTurn`
         * at call sites since attackers are always the enemy of the side to move.
         *
         * Called twice for castling moves: once with original occupancy for the king's
         * origin square, and once with post-move occupancy for the pass-through square.
         * The final king safety check always uses post-move occupancy.
         */
        [[nodiscard]] inline bool isAttackedBy(uint8_t sqIdx, uint64_t occ,
                                               const std::array<uint64_t, 6>& attackers,
                                               bool attackersAreBlack) {
            uint64_t sqBit = 1ULL << sqIdx;

            // Pawn attack tables are asymmetric — index 0..5 are white, 6..11 are black.
            // Piece(PAWN, !attackersAreBlack) exploits the Piece index encoding to select
            // the correct table: white pawns at index 0, black at index 6.
            if (attackTables.movement[Piece(PAWN, !attackersAreBlack)][sqIdx] & attackers[PAWN])
                return true;

            // Knights and kings use symmetric attack tables — lookup from sqIdx directly.
            if (attackTables.movement[KNIGHT][sqIdx] & attackers[KNIGHT]) return true;
            if (attackTables.movement[KING][sqIdx] & attackers[KING]) return true;

            uint64_t bishopLike = bishopAttacks(sqIdx, occ);
            if (bishopLike & (attackers[BISHOP] | attackers[QUEEN])) return true;

            uint64_t rookLike = rookAttacks(sqIdx, occ);
            if (rookLike & (attackers[ROOK] | attackers[QUEEN])) return true;

            return false;
        }
    }  // namespace detail

    MoveList<256> Board::getLegalMoves() const {
        auto pseudoLegal = getPseudoLegalMoves();
        MoveList<256> result{};

        // Under double check only king moves can be legal — filter early to avoid
        // calling isLegalFast on every pseudo-legal move.
        if (platform::popcnt(getCheckers()) == 2) [[unlikely]] {
            auto blackTurn = isBlackTurn();
            auto attackers = blackTurn ? getWhiteBitboards() : getBlackBitboards();
            auto occ       = getOccupancy();
            for (const auto& m : pseudoLegal) {
                if (get(m.from()).type() == KING) [[unlikely]] {
                    auto sq = m.to();
                    if (!detail::isAttackedBy(sq, occ, attackers, !blackTurn)) {
                        result += m;
                    }
                }
            }
            return result;
        }

        for (const auto& m : pseudoLegal)
            if (isLegalFast(m)) result += m;

        return result;
    }

    bool Board::isLegal(Move m) const { return getLegalMoves().contains(m); }

    bool Board::isLegalFast(Move m) const {
        Square from                = m.from();
        Square to                  = m.to();

        uint8_t fromIdx            = static_cast<uint8_t>(from);
        uint8_t toIdx              = static_cast<uint8_t>(to);
        uint64_t fromBit           = 1ULL << fromIdx;
        uint64_t toBit             = 1ULL << toIdx;

        bool blackTurn             = isBlackTurn();

        const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
        const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

        uint64_t ownOcc            = blackTurn ? getBlackOccupancy() : getWhiteOccupancy();
        uint64_t enemyOcc          = blackTurn ? getWhiteOccupancy() : getBlackOccupancy();
        uint64_t occ               = getOccupancy();

        // Own-piece captures are never legal.
        if (ownOcc & toBit) return false;

        PieceType movingPiece = get(from).type();
        if (movingPiece == NONE) return false;

        // Sliders must be aligned with the target square and have a clear path.
        // The movement table gives unblocked rays; the between table gives the
        // squares that must be empty for the move to be geometrically valid.
        if ((movingPiece == BISHOP || movingPiece == ROOK || movingPiece == QUEEN) &&
            attackTables.between[fromIdx][toIdx] & occ) {
            return false;
        }

        // Simulate the post-move board state without constructing a full Board object.
        // The moving piece is removed from its origin and placed on the destination.
        // Any enemy piece on the destination is removed from enemy occupancy.
        uint64_t newOwnOcc     = (ownOcc & ~fromBit) | toBit;
        uint64_t newEnemyOcc   = enemyOcc & ~toBit;

        // En passant additionally removes the captured pawn from the rank it sits on,
        // which is one rank behind the destination from the moving side's perspective.
        uint64_t epCapturedBit = 0;
        if (movingPiece == PAWN && isEnPassantValid()) {
            uint8_t epFile = getEnPassantFile();
            if (to.file() == epFile && from.file() != epFile) {
                uint8_t capturedRank = blackTurn ? static_cast<uint8_t>(to.rank() + 1)
                                                 : static_cast<uint8_t>(to.rank() - 1);
                Square capturedSq(capturedRank, epFile);
                epCapturedBit  = 1ULL << static_cast<uint8_t>(capturedSq);
                newEnemyOcc   &= ~epCapturedBit;
            }
        }

        uint64_t newOcc = newOwnOcc | newEnemyOcc;

        // After a king move the king square is the destination, not the origin.
        uint64_t kingBB = ownBitboards[KING];
        if (movingPiece == KING) kingBB = toBit;
        uint8_t kingSqIdx = static_cast<uint8_t>(platform::tzcnt(kingBB));

        if (movingPiece == KING) {
            int8_t fileDelta = static_cast<int8_t>(to.file()) - static_cast<int8_t>(from.file());
            if (fileDelta == 2 || fileDelta == -2) {
                uint8_t passingFile  = static_cast<uint8_t>(from.file() + (fileDelta > 0 ? 1 : -1));
                uint8_t passingSqIdx = static_cast<uint8_t>(Square(from.rank(), passingFile));

                // The from-square check uses original occupancy since the king has not
                // yet moved. The passing-square check uses newOcc with the king removed
                // from its origin.
                if (detail::isAttackedBy(fromIdx, occ, enemyBitboards, !blackTurn)) return false;
                if (detail::isAttackedBy(passingSqIdx, newOcc, enemyBitboards, !blackTurn))
                    return false;
            }
        }

        // Build the post-move enemy bitboards, removing any piece captured on the
        // destination square and any pawn captured via en passant.
        std::array<uint64_t, 6> enemyBB;
        for (int i = 0; i < 6; i++) enemyBB[i] = enemyBitboards[i] & ~toBit & ~epCapturedBit;

        // Verify the king is not in check on the post-move board.
        if (detail::isAttackedBy(kingSqIdx, newOcc, enemyBB, !blackTurn)) return false;

        return true;
    }

    MoveList<512> Board::getPseudoLegalMoves() const {
        MoveList<512> result{};

        bool blackTurn             = isBlackTurn();

        const auto& ownBitboards   = blackTurn ? blackBitboards : whiteBitboards;
        const auto& enemyBitboards = blackTurn ? whiteBitboards : blackBitboards;

        uint64_t ownOcc            = blackTurn ? getBlackOccupancy() : getWhiteOccupancy();
        uint64_t enemyOcc          = blackTurn ? getWhiteOccupancy() : getBlackOccupancy();
        uint64_t occ               = getOccupancy();

        uint8_t promotionRank      = blackTurn ? 0 : 7;
        uint8_t startingRank       = blackTurn ? 6 : 1;
        int8_t pushDir             = blackTurn ? -1 : 1;

        // Attack table offset: white pieces at indices 0..5, black at 6..11.
        int colorOffset            = blackTurn ? 6 : 0;

        for (int pieceIdx = 0; pieceIdx < 6; pieceIdx++) {
            uint64_t bb = ownBitboards[pieceIdx];

            while (bb) {
                uint8_t fromIdx  = static_cast<uint8_t>(platform::tzcnt(bb));
                bb              &= bb - 1;
                Square from(fromIdx);

                if (pieceIdx == PAWN) {
                    // Single push — only if the destination square is unoccupied.
                    uint8_t pushRank = static_cast<uint8_t>(from.rank() + pushDir);
                    Square pushSq(pushRank, from.file());
                    uint64_t pushBit = 1ULL << static_cast<uint8_t>(pushSq);

                    if (!(occ & pushBit)) {
                        if (pushRank == promotionRank) {
                            // Emit all four promotion variants.
                            result += Move(from, pushSq, QUEEN);
                            result += Move(from, pushSq, ROOK);
                            result += Move(from, pushSq, BISHOP);
                            result += Move(from, pushSq, KNIGHT);
                        } else {
                            result += Move(from, pushSq);

                            // Double push — only from the starting rank and only if
                            // both intermediate and destination squares are unoccupied.
                            if (from.rank() == startingRank) {
                                uint8_t doublePushRank =
                                    static_cast<uint8_t>(from.rank() + pushDir * 2);
                                Square doublePushSq(doublePushRank, from.file());
                                uint64_t doublePushBit = 1ULL << static_cast<uint8_t>(doublePushSq);
                                if (!(occ & doublePushBit)) result += Move(from, doublePushSq);
                            }
                        }
                    }

                    // Diagonal captures — only onto squares occupied by enemy pieces.
                    uint64_t captures =
                        attackTables.movement[colorOffset + PAWN][fromIdx] & enemyOcc;
                    while (captures) {
                        uint8_t toIdx  = static_cast<uint8_t>(platform::tzcnt(captures));
                        captures      &= captures - 1;
                        Square to(toIdx);

                        if (to.rank() == promotionRank) {
                            result += Move(from, to, QUEEN);
                            result += Move(from, to, ROOK);
                            result += Move(from, to, BISHOP);
                            result += Move(from, to, KNIGHT);
                        } else {
                            result += Move(from, to);
                        }
                    }

                    // En passant — the pawn must be on the correct rank and adjacent file.
                    if (isEnPassantValid()) {
                        uint8_t epFile = getEnPassantFile();
                        uint8_t epRank = blackTurn ? 2 : 5;
                        if (from.rank() == static_cast<uint8_t>(epRank - pushDir) &&
                            (from.file() == epFile - 1 || from.file() == epFile + 1)) {
                            result += Move(from, Square(epRank, epFile));
                        }
                    }

                } else if (pieceIdx == KING) {
                    // Normal king moves — all squares in the attack table not occupied by own
                    // pieces.
                    uint64_t moves = attackTables.movement[colorOffset + KING][fromIdx] & ~ownOcc;
                    while (moves) {
                        uint8_t toIdx  = static_cast<uint8_t>(platform::tzcnt(moves));
                        moves         &= moves - 1;
                        result        += Move(from, Square(toIdx));
                    }

                    // Castling — path between king and rook must be clear. Check and
                    // pass-through square validation is deferred to isLegalFast.
                    if (!blackTurn) {
                        if (canWhiteKingsideCastle()) {
                            constexpr uint64_t WK_PATH = (1ULL << 5) | (1ULL << 6);
                            if (!(occ & WK_PATH)) result += Move(from, Square(0, 6));
                        }
                        if (canWhiteQueensideCastle()) {
                            constexpr uint64_t WQ_PATH = (1ULL << 1) | (1ULL << 2) | (1ULL << 3);
                            if (!(occ & WQ_PATH)) result += Move(from, Square(0, 2));
                        }
                    } else {
                        if (canBlackKingsideCastle()) {
                            constexpr uint64_t BK_PATH = (1ULL << 61) | (1ULL << 62);
                            if (!(occ & BK_PATH)) result += Move(from, Square(7, 6));
                        }
                        if (canBlackQueensideCastle()) {
                            constexpr uint64_t BQ_PATH = (1ULL << 57) | (1ULL << 58) | (1ULL << 59);
                            if (!(occ & BQ_PATH)) result += Move(from, Square(7, 2));
                        }
                    }

                } else {
                    // Knights, bishops, rooks, queens — emit all squares in the attack
                    // table not occupied by own pieces. Slider path validity is not checked
                    // here; isLegalFast filters moves that pass through intervening pieces.
                    uint64_t moves =
                        attackTables.movement[colorOffset + pieceIdx][fromIdx] & ~ownOcc;
                    while (moves) {
                        uint8_t toIdx  = static_cast<uint8_t>(platform::tzcnt(moves));
                        moves         &= moves - 1;
                        result        += Move(from, Square(toIdx));
                    }
                }
            }
        }

        return result;
    }
}  // namespace aleph::chess